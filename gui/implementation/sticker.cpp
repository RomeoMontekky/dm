#include "sticker.h"

#include <string>
#include <cstring>
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
   
const wchar_t g_tahoma_name[] = L"Tahoma";

namespace Brushes
{
   const Gdiplus::SolidBrush black(Gdiplus::Color(0x00, 0x00, 0x00));
   const Gdiplus::SolidBrush light_grey(Gdiplus::Color(0xCC, 0xCC, 0xCC));
   const Gdiplus::SolidBrush dark_grey(Gdiplus::Color(0x99, 0x99, 0x99)); 
   const Gdiplus::SolidBrush dark_grey_with_blue(Gdiplus::Color(0x89, 0x91, 0xA9));
   const Gdiplus::SolidBrush dark_green(Gdiplus::Color(0x72, 0xBE, 0x44));
   const Gdiplus::SolidBrush dark_blue(Gdiplus::Color(0x00, 0x55, 0xBB));
   const Gdiplus::SolidBrush dark_red(Gdiplus::Color(0xD9, 0x47, 0x00));
}

namespace Fonts
{
   const Gdiplus::Font tahoma_9_bold(g_tahoma_name, 9, Gdiplus::FontStyleBold);
   const Gdiplus::Font tahoma_9_regular(g_tahoma_name, 9, Gdiplus::FontStyleRegular);
   const Gdiplus::Font tahoma_8_regular(g_tahoma_name, 8, Gdiplus::FontStyleRegular);
}
   
std::wstring AsciiToWide(const char* input)
{
   if (nullptr == input || '\0' == *input)
   {
      return std::wstring();
   }
   
   const auto input_size = std::strlen(input);
   auto output_size = ::MultiByteToWideChar(CP_ACP, 0, input, input_size, nullptr, 0);
   assert(output_size > 0);
   
   std::unique_ptr<wchar_t[]> output(new wchar_t[output_size]);
   auto ret = ::MultiByteToWideChar(CP_ACP, 0, input, input_size, output.get(), output_size);
   assert(ret != 0);
   
   return std::wstring(output.get(), output.get() + output_size);
}

///////////// class TextInfo /////////////
   
class TextInfo
{
public:
   TextInfo(const Gdiplus::Font& font, const Gdiplus::SolidBrush& brush);
   
   bool SetText(const char* text);
   const std::wstring& GetText() const;
   
   void RecalculateBoundary(int x, int y, Gdiplus::Graphics* graphics);
   
private:
   std::wstring m_text;
   Gdiplus::RectF m_boundary;
   const Gdiplus::Font& m_font;
   const Gdiplus::Brush& m_brush;
};

TextInfo::TextInfo(const Gdiplus::Font& font, const Gdiplus::SolidBrush& brush) : 
   m_text(), m_boundary(), m_font(font), m_brush(brush)
{
   // no code
}

bool TextInfo::SetText(const char* text)
{
   const auto wide_text = ::AsciiToWide(text);
   if (m_text != wide_text)
   {
      m_text = wide_text;
      return true;
   }
   return false;
}

const std::wstring& TextInfo::GetText() const
{
   return m_text;
}

void TextInfo::RecalculateBoundary(int x, int y, Gdiplus::Graphics* graphics)
{
   Gdiplus::RectF origin_rect(x, y, 0, 0);
   graphics->MeasureString(m_text.c_str(), m_text.size(), &m_font, origin_rect, &m_boundary);
}

///////////// class Section ////////////////

class Section : public ISection
{
public:
   Section(Sticker& sticker);

   const std::wstring& GetTitle() const;
   const std::wstring& GetOwnerName() const;
   const std::wstring& GetHeaderDescription() const;
   const std::wstring& GetFooterPrefex() const;
   const std::wstring& GetFooterDescription() const;

   struct Item
   {
      Item();

      TextInfo m_date;
      TextInfo m_time;
      TextInfo m_description;
   };

   unsigned long GetItemCount() const;
   const Item& GetItem(unsigned long index) const;
   
   void RecalculateBoundary(int x, int y, Gdiplus::Graphics* graphics);

   // ISection overrides
   virtual void SetTitle(const char* title) override;
   virtual void SetOwnerName(const char* owner_name) override;
   virtual void SetHeader(const char* description) override;
   virtual void SetFooter(const char* prefix, const char* description) override;
   virtual void SetItemCount(unsigned long count) override;
   virtual void SetItem(unsigned long index, const char* date, const char* time, const char* description) override;

private:
   TextInfo m_title;
   TextInfo m_owner_name;
   TextInfo m_header_description;
   TextInfo m_footer_prefix;
   TextInfo m_footer_description;

   std::vector<Item> m_items;
   
   RECT m_boundary;

   Sticker& m_sticker;
};

Section::Item::Item() :
   m_date(Fonts::tahoma_9_regular, Brushes::dark_grey_with_blue),
   m_time(Fonts::tahoma_8_regular, Brushes::dark_grey_with_blue),
   m_description(Fonts::tahoma_9_regular, Brushes::dark_grey_with_blue)
{
}

Section::Section(Sticker& sticker) :
   m_title(Fonts::tahoma_9_bold, Brushes::dark_red),
   m_owner_name(Fonts::tahoma_9_regular, Brushes::black),
   m_header_description(Fonts::tahoma_9_bold, Brushes::dark_red),
   m_footer_prefix(Fonts::tahoma_9_bold, Brushes::dark_red),
   m_footer_description(Fonts::tahoma_9_bold, Brushes::dark_red),
   m_items(),
   m_boundary(),
   m_sticker(sticker)
{
   ::SetRectEmpty(&m_boundary);
}

const std::wstring& Section::GetTitle() const
{
   return m_title.GetText();
}

const std::wstring& Section::GetOwnerName() const
{
   return m_owner_name.GetText();
}

const std::wstring& Section::GetHeaderDescription() const
{
   return m_header_description.GetText();
}

const std::wstring& Section::GetFooterPrefex() const
{
   return m_footer_prefix.GetText();
}

const std::wstring& Section::GetFooterDescription() const
{
   return m_footer_description.GetText();
}

unsigned long Section::GetItemCount() const
{
   return m_items.size();
}

const Section::Item& Section::GetItem(unsigned long index) const
{
   return m_items.at(index);
}

void Section::RecalculateBoundary(int x, int y, Gdiplus::Graphics* graphics)
{
   // TODO: Init boundary to {x, y, 0, 0}

   m_title.RecalculateBoundary(x, y, graphics);

   // TODO: Recalculate another items and unite boundaries.
}

void Section::SetTitle(const char* title)
{
   if (m_title.SetText(title))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetOwnerName(const char* owner_name)
{
   if (m_owner_name.SetText(owner_name))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetHeader(const char* description)
{
   if (m_header_description.SetText(description))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetFooter(const char* prefix, const char* description)
{
   if (m_footer_prefix.SetText(prefix))
   {
      m_sticker.SetDirty();
   }
   
   if (m_footer_description.SetText(description))
   {
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
   
   if (item.m_date.SetText(date))
   {
      m_sticker.SetDirty();
   }
   
   if (item.m_time.SetText(time))
   {
      m_sticker.SetDirty();
   }
   
   if (item.m_description.SetText(description))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

} // namespace

/////////////// class Sticker /////////////////

Sticker::Sticker() : wc::Window(),
   m_state(StateType::Minimized),
   m_memory_face(),
   m_callback(),
   m_is_dirty(true),
   m_is_redraw(true),
   m_sections()
{
   ::SetRectEmpty(&m_minimized_window_rect);
   ::SetRectEmpty(&m_window_rect);
   ::SetRectEmpty(&footer_rect);
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
               static_cast<Section*>(m_sections[0].get())->GetTitle().c_str(), 
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
   if (StateType::Minimized == state)
   {
      m_window_rect = m_minimized_window_rect;
      return;
   }
   
   assert(!m_sections.empty());
   
   int x = 0;
   int y = 0;
   
   std::unique_ptr<Gdiplus::Graphics> memory_graphics(Gdiplus::Graphics::FromImage(m_memory_face.get()));
   memory_graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
   
   for (unsigned long index = 0; index < m_sections.size(); ++index)
   {
      // Show only first 3 sections in opened state
      if (StateType::Opened == state && index > 2)
      {
         break;
      }
      
      auto* section = static_cast<Section*>(m_sections[index].get());
      section->RecalculateBoundary(x, y, memory_graphics.get());
      
      // TODO:
   }
}
