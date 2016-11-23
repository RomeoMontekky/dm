#include "sticker.h"

#include <string>
#include <cassert>

ISection::~ISection()
{
   // no code
}

IStickerCallback::~IStickerCallback()
{
   // no code
}

namespace
{

class Section : public ISection
{
public:
   Section(Sticker& sticker);

   const std::string& GetTitle() const;
   const std::string& GetOwnerName() const;
   const std::string& GetHeaderDescription() const;
   const std::string& GetFooterPrefex() const;
   const std::string& GetFooterDescription() const;

   struct Item
   {
      std::string m_date;
      std::string m_time;
      std::string m_description;
   };

   unsigned long GetItemCount() const;
   const Item& GetItem(unsigned long index) const;

   // ISection overrides
   virtual void SetTitle(const char* title) override;
   virtual void SetOwnerName(const char* owner_name) override;
   virtual void SetHeader(const char* description) override;
   virtual void SetFooter(const char* prefix, const char* description) override;
   virtual void SetItemCount(unsigned long count) override;
   virtual void SetItem(unsigned long index, const char* date, const char* time, const char* description) override;

private:
   std::string m_title;
   std::string m_owner_name;
   std::string m_header_description;
   std::string m_footer_prefix;
   std::string m_footer_description;

   std::vector<Item> m_items;

   Sticker& m_sticker;
};

Section::Section(Sticker& sticker) :
   m_title(),
   m_owner_name(),
   m_header_description(),
   m_footer_prefix(),
   m_footer_description(),
   m_items(),
   m_sticker(sticker)
{
   // no code
}

const std::string& Section::GetTitle() const
{
   return m_title;
}

const std::string& Section::GetOwnerName() const
{
   return m_owner_name;
}

const std::string& Section::GetHeaderDescription() const
{
   return m_header_description;
}

const std::string& Section::GetFooterPrefex() const
{
   return m_footer_prefix;
}

const std::string& Section::GetFooterDescription() const
{
   return m_footer_description;
}

unsigned long Section::GetItemCount() const
{
   return m_items.size();
}

const Section::Item& Section::GetItem(unsigned long index) const
{
   return m_items.at(index);
}

void Section::SetTitle(const char* title)
{
   if (m_title != title)
   {
      m_title = title;
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetOwnerName(const char* owner_name)
{
   if (m_owner_name != owner_name)
   {
      m_owner_name = owner_name;
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetHeader(const char* description)
{
   if (m_header_description != description)
   {
      m_header_description = description;
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetFooter(const char* prefix, const char* description)
{
   if (m_footer_prefix != prefix)
   {
      m_footer_prefix = prefix;
      m_sticker.SetDirty();
   }
   
   if (m_footer_description != description)
   {
      m_footer_description = description;
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetItemCount(unsigned long count)
{
   if (m_items.size() != count)
   {
      m_items.resize(count);
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetItem(unsigned long index, const char* date, const char* time, const char* description)
{
   auto& item = m_items.at(index);
   if (item.m_date != date)
   {
      item.m_date = date;
      m_sticker.SetDirty();
   }
   
   if (item.m_time != time)
   {
      item.m_time = time;
      m_sticker.SetDirty();
   }
   
   if (item.m_description != description)
   {
      item.m_description = description;
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

std::wstring AsciiToWide(const std::string& input)
{
   auto output_buffer_size = ::MultiByteToWideChar(CP_ACP, 0, input.c_str(), input.size(), nullptr, 0);
   assert(output_buffer_size > 0);
   std::unique_ptr<wchar_t[]> output_buffer(new wchar_t[output_buffer_size]);
   auto ret = ::MultiByteToWideChar(CP_ACP, 0, input.c_str(), input.size(), output_buffer.get(), output_buffer_size);
   assert(ret != 0);
   return std::wstring(output_buffer.get(), output_buffer.get() + output_buffer_size);
}

} // namespace

/////////////// Sticker /////////////////

Sticker::Sticker() : wc::Window(),
   m_state(StateType::Minimized),
   m_memory_face(),
   m_callback(),
   m_is_dirty(true),
   m_is_redraw(true),
   m_sections()
{
   memset(&m_minimized_window_rect, 0, sizeof(m_minimized_window_rect));
   memset(&m_window_rect, 0, sizeof(m_window_rect));
   memset(&footer_rect, 0, sizeof(footer_rect));
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
   m_sections.resize(count);
}

ISection& Sticker::GetSection(unsigned long index)
{
   auto& section = m_sections.at(index);
   if (!section)
   {
      section.reset(new Section(*this));
   }
   return *section.get();
}

void Sticker::SetCallback(IStickerCallback* callback)
{
   m_callback.reset(callback);
}

LRESULT Sticker::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_LBUTTONUP:
      {
         OnMouseClick();
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

void Sticker::OnMouseClick()
{
   switch (m_state)
   {
      case StateType::Minimized:
      {
         ::GetWindowRect(GetHandle(), &m_minimized_window_rect);
         ::MapWindowPoints(nullptr, ::GetParent(GetHandle()), (LPPOINT)&m_minimized_window_rect, 2);
         
         SetState(StateType::Opened);

         //::SetWindowPos(GetHandle(), GetHandle(),  )
         
         break;
      }
   }
}

void Sticker::OnPaint(HDC hdc)
{
   if (m_sections.empty())
   {
      return;
   }
   
   RECT rect;
   ::GetClientRect(GetHandle(), &rect);
   
   const auto rect_width = rect.right - rect.left;
   const auto rect_height = rect.bottom - rect.top;

   Gdiplus::Graphics graphics(hdc);

   if (m_is_dirty || !m_memory_face ||
       rect_width != m_memory_face->GetWidth() || rect_height != m_memory_face->GetHeight())
   {
      m_memory_face.reset(new Gdiplus::Bitmap(rect_width, rect_height, &graphics));
      std::unique_ptr<Gdiplus::Graphics> memory_graphics(Gdiplus::Graphics::FromImage(m_memory_face.get()));
      
      memory_graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
      
      switch (m_state)
      {
         case StateType::Minimized:
         {      
            const Gdiplus::Font font(L"Tahoma", 9, Gdiplus::FontStyleBold);
            const Gdiplus::RectF rectf(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
            const Gdiplus::SolidBrush brush(Gdiplus::Color(0x72, 0xBE, 0x44));
            
            Gdiplus::StringFormat format;
            format.SetAlignment(Gdiplus::StringAlignmentCenter);
            
            memory_graphics->DrawString(
               AsciiToWide(static_cast<Section*>(m_sections[0].get())->GetTitle()).c_str(), 
               -1, &font, rectf, &format, &brush);
            
            break;
         }
      }
   }
            
   m_is_dirty = false;

   graphics.DrawImage(m_memory_face.get(), 0, 0);
}

void Sticker::SetState(StateType state)
{
   m_state = state;

   // Recalculate all rectangles

   for (auto& isection : m_sections)
   {
      auto* section = static_cast<Section*>(isection.get());
      // TODO:
   }
}
