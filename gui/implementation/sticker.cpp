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
         // Save initilial window rect into the variable to have ability to restore it at any time.
         m_object->SetMinimizedBoundary(client_rect);
      }
      case WM_LBUTTONUP:
      {
         OnMouseClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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

void Sticker::OnMouseClick(long x, long y)
{
   if (!m_memory_image)
   {
      return;
   }

   const auto old_state = m_object->GetState();
   m_object->ProcessMouseClick(x, y);
   const auto new_state = m_object->GetState();
   
   if (new_state != old_state)
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
