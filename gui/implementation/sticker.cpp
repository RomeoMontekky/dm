#include "sticker.h"
#include "sticker_objects.h"

// For GET_X_LPARAM
#include <windowsx.h>

#include <cassert>

namespace
{

//////////// Utilities /////////////

inline std::unique_ptr<Gdiplus::Graphics> GetGraphics(
   const std::unique_ptr<Gdiplus::Bitmap>& image)
{
   std::unique_ptr<Gdiplus::Graphics> graphics(Gdiplus::Graphics::FromImage(image.get()));
   graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
   return graphics;
}

inline void InvalidateRectF(HWND wnd, const Gdiplus::RectF& rectf)
{
   const RECT rect =
   {
      static_cast<LONG>(rectf.X - 0.5),
      static_cast<LONG>(rectf.Y - 0.5),
      static_cast<LONG>(rectf.GetRight() + 0.5),
      static_cast<LONG>(rectf.GetBottom() + 0.5)
   };
   ::InvalidateRect(wnd, &rect, FALSE);
}

} // namespace

ISection::~ISection()
{
   // no code
}

IStickerCallback::~IStickerCallback()
{
   // no code
}

/////////////// class Sticker /////////////////

Sticker::Sticker() : wc::Window(),
   m_is_dirty(true),
   m_is_redraw(true),
   m_is_mouse_tracking(false),
   m_memory_image(),
   m_callback(),
   m_object(new SGO::StickerObject(*this))
{
   // no code
}

Sticker::~Sticker()
{
   // no code
}

void Sticker::SetDirty()
{
   m_is_dirty = true;
}

void Sticker::SetRedraw(bool is_redraw)
{
   m_is_redraw = is_redraw;
   Update();
}

void Sticker::Update()
{
   if (m_is_redraw && m_is_dirty)
   {
      ::InvalidateRect(GetHandle(), nullptr, FALSE);
   }
}

void Sticker::SetSectionCount(unsigned long count)
{
   m_object->SetSectionCount(count);
}

ISection& Sticker::GetSection(unsigned long index)
{
   return m_object->GetSection(index);
}

void Sticker::SetCallback(std::unique_ptr<IStickerCallback>&& callback)
{
   m_callback = std::move(callback);
}

LRESULT Sticker::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_CREATE:
      {
         RECT client_rect;
         ::GetClientRect(GetHandle(), &client_rect);
         m_object->Initialize(client_rect);
         break;
      }
      case WM_LBUTTONUP:
      {
         OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
         return TRUE;
      }
      case WM_MOUSEMOVE:
      {
         OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
         return FALSE;
      }
      case WM_MOUSEHOVER:
      {
         OnMouseHover(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
         return FALSE;
      }
      case WM_MOUSELEAVE:
      {
         OnMouseLeave();
         return FALSE;
      }
      case WM_ERASEBKGND:
      {
         return TRUE;
      }
      case WM_PAINT:
      {
         PAINTSTRUCT ps;
         HDC hdc = ::BeginPaint(GetHandle(), &ps);
         OnPaint(hdc);
         ::EndPaint(GetHandle(), &ps);
         return 0;
      }
   }
   return Window::WindowProc(uMsg, wParam, lParam);
}

void Sticker::OnLButtonUp(long x, long y)
{
   if (!m_memory_image)
   {
      return;
   }

   if (m_object->ProcessClick(x, y) == BGO::Object::ClickType::ClickDoneNeedResize)
   {
      auto memory_graphics = GetGraphics(m_memory_image);
      
      m_object->RecalculateBoundary(0, 0, memory_graphics.get());
      const auto& object_boundary = m_object->GetBoundary();

      RECT window_rect;
      ::GetWindowRect(GetHandle(), &window_rect);
      ::MapWindowPoints(nullptr, ::GetParent(GetHandle()), (LPPOINT)(&window_rect), 2);

      ::SetWindowPos(GetHandle(), nullptr,
                     window_rect.left,
                     window_rect.top,
                     static_cast<int>(object_boundary.Width + 6.5),
                     static_cast<int>(object_boundary.Height + 6.5),
                     SWP_NOZORDER);
      
      ::InvalidateRect(GetHandle(), nullptr, TRUE);   
   }
}

void Sticker::OnMouseMove(long x, long y)
{
   if (!m_is_mouse_tracking)
   {
      TRACKMOUSEEVENT tracking_data;
      tracking_data.cbSize = sizeof(tracking_data);
      tracking_data.dwFlags = TME_HOVER | TME_LEAVE;
      tracking_data.hwndTrack = GetHandle();
      tracking_data.dwHoverTime = 1; 
      
      ::TrackMouseEvent(&tracking_data);
      
      m_is_mouse_tracking = true;
   }
}

void Sticker::OnMouseHover(long x, long y)
{
   ProcessHover(x, y);
   m_is_mouse_tracking = false;
}

void Sticker::OnMouseLeave()
{
   ProcessHover(-1, -1);
   m_is_mouse_tracking = false;
}

void Sticker::OnPaint(HDC hdc)
{
   RECT client_rect;
   ::GetClientRect(GetHandle(), &client_rect);
   
   const auto client_width = client_rect.right - client_rect.left;
   const auto client_height = client_rect.bottom - client_rect.top;

   Gdiplus::Graphics graphics(hdc);

   if (m_is_dirty || !m_memory_image ||
       client_width != m_memory_image->GetWidth() || client_height != m_memory_image->GetHeight())
   {
      m_memory_image.reset(new Gdiplus::Bitmap(client_width, client_height, &graphics));
      auto memory_graphics = GetGraphics(m_memory_image);

      if (m_is_dirty)
      {
         m_object->RecalculateBoundary(0, 0, memory_graphics.get());
         m_is_dirty = false;
      }

      m_object->Draw(memory_graphics.get());
   }
            
   graphics.DrawImage(m_memory_image.get(), 0, 0);
}

void Sticker::ProcessHover(long x, long y)
{
   if (!m_memory_image)
   {
      return;
   }

   BGO::TObjectPtrVector invalidated_objects;
   m_object->ProcessHover(x, y, invalidated_objects);

   if (!invalidated_objects.empty())
   {
      auto graphics = GetGraphics(m_memory_image);

      Gdiplus::RectF invalidated_rect;
      for (auto index = 0UL; index < invalidated_objects.size(); ++index)
      {
         const auto object = invalidated_objects[index];
         const auto& boundary = object->GetBoundary();
         if (0UL == index)
         {
            invalidated_rect = boundary;
         }
         else
         {
            Gdiplus::RectF::Union(invalidated_rect, invalidated_rect, boundary);
         }
         object->Draw(graphics.get());
      }

      ::InvalidateRectF(GetHandle(), invalidated_rect);
   }
}
