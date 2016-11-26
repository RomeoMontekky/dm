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
   
////////////////// Constants //////////////////
   
const auto g_indent_vert = 3;
const auto g_indent_horz = 3;
const wchar_t g_tahoma_name[] = L"Tahoma";

namespace Colors
{
   const Gdiplus::Color black(0x00, 0x00, 0x00);
   const Gdiplus::Color grey_light(0xCC, 0xCC, 0xCC);
   const Gdiplus::Color grey_dark(0x99, 0x99, 0x99);
   const Gdiplus::Color grey_dark_with_blue(0x89, 0x91, 0xA9);
   const Gdiplus::Color green_dark(0x72, 0xBE, 0x44);
   const Gdiplus::Color blue_dark(0x00, 0x55, 0xBB);
   const Gdiplus::Color red_dark(0xD9, 0x47, 0x00);
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
   TextInfo(/*const Gdiplus::Font& font, const Gdiplus::SolidBrush& brush*/);
   
   bool SetText(const char* text);
   const std::wstring& GetText() const;
   
   void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics);
   const Gdiplus::RectF& GetBoundary() const;

   void Draw(Gdiplus::Graphics* graphics) const;
   
private:
   std::wstring m_text;
   Gdiplus::RectF m_boundary;
   //const Gdiplus::Font& m_font;
   //const Gdiplus::Brush& m_brush;
};

TextInfo::TextInfo(/*const Gdiplus::Font& font, const Gdiplus::SolidBrush& brush*/) :
   m_text(), m_boundary()/*, m_font(font), m_brush(brush)*/
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

void TextInfo::RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics)
{
   const Gdiplus::Font font(g_tahoma_name, 9, Gdiplus::FontStyleRegular);
   //const Gdiplus::SolidBrush black(Gdiplus::Color(0x00, 0x00, 0x00));
   Gdiplus::RectF origin_rect(x, y, 0, 0);
   //auto status = graphics->MeasureString(m_text.c_str(), m_text.size(), &m_font, origin_rect, &m_boundary);
   auto status = graphics->MeasureString(m_text.c_str(), m_text.size(), &font, origin_rect, &m_boundary);
}

const Gdiplus::RectF& TextInfo::GetBoundary() const
{
   return m_boundary;
}

void TextInfo::Draw(Gdiplus::Graphics* graphics) const
{
   const Gdiplus::Font font(g_tahoma_name, 9, Gdiplus::FontStyleRegular);
   const Gdiplus::SolidBrush black(Gdiplus::Color(0x00, 0x00, 0x00));
   auto status = graphics->DrawString(m_text.c_str(), m_text.size(), &font, m_boundary, nullptr, &black);
}

///////////// class Section ////////////////

class Section : public ISection
{
public:
   Section(Sticker& sticker);

   const std::wstring& GetTitle() const;

   void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics);
   const Gdiplus::RectF& GetBoundary() const;

   void Draw(Gdiplus::Graphics* graphics) const;

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

   struct Item
   {
      Item();

      void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics);
      const Gdiplus::RectF& GetBoundary() const;

      void Draw(Gdiplus::Graphics* graphics) const;

      TextInfo m_date;
      TextInfo m_time;
      TextInfo m_description;
      Gdiplus::RectF m_boundary;
   };

   std::vector<Item> m_items;
   Gdiplus::RectF m_boundary;
   Sticker& m_sticker;
};

Section::Item::Item() :
/*
   m_date(Fonts::tahoma_9_regular, Brushes::grey_dark_with_blue),
   m_time(Fonts::tahoma_8_regular, Brushes::grey_light),
   m_description(Fonts::tahoma_9_regular, Brushes::grey_dark_with_blue),*/
   m_boundary()
{
}

void Section::Item::RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics)
{
   m_boundary.X = x;
   m_boundary.Y = y;
   m_boundary.Width = 0;
   m_boundary.Height = 0;

   m_date.RecalculateBoundary(m_boundary.GetRight(), m_boundary.GetTop(), graphics);
   Gdiplus::RectF::Union(m_boundary, m_boundary, m_date.GetBoundary());

   m_time.RecalculateBoundary(m_boundary.GetRight() + g_indent_horz, m_boundary.GetTop(), graphics);
   Gdiplus::RectF::Union(m_boundary, m_boundary, m_time.GetBoundary());

   m_description.RecalculateBoundary(m_boundary.GetRight() + g_indent_horz, m_boundary.GetTop(), graphics);
   Gdiplus::RectF::Union(m_boundary, m_boundary, m_description.GetBoundary());
}

const Gdiplus::RectF& Section::Item::GetBoundary() const
{
   return m_boundary;
}

void Section::Item::Draw(Gdiplus::Graphics* graphics) const
{
   m_date.Draw(graphics);
   m_time.Draw(graphics);
   m_description.Draw(graphics);
}

Section::Section(Sticker& sticker) :
/*
   m_title(Fonts::tahoma_9_bold, Brushes::red_dark),
   m_owner_name(Fonts::tahoma_9_regular, Brushes::black),
   m_header_description(Fonts::tahoma_9_regular, Brushes::grey_dark),
   m_footer_prefix(Fonts::tahoma_9_regular, Brushes::grey_dark),
   m_footer_description(Fonts::tahoma_9_regular, Brushes::black),
   m_items(),
   m_boundary(),*/
   m_sticker(sticker)
{
}

const std::wstring& Section::GetTitle() const
{
   return m_title.GetText();
}

void Section::RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics)
{
   m_boundary.X = x;
   m_boundary.Y = y;
   m_boundary.Width = 0;
   m_boundary.Height = 0;

   m_title.RecalculateBoundary(m_boundary.GetLeft(), m_boundary.GetBottom() + g_indent_vert, graphics);
   Gdiplus::RectF::Union(m_boundary, m_boundary, m_title.GetBoundary());

   m_header_description.RecalculateBoundary(m_boundary.GetLeft(), m_boundary.GetBottom() + g_indent_vert, graphics);
   Gdiplus::RectF::Union(m_boundary, m_boundary, m_header_description.GetBoundary());

   for (auto& item : m_items)
   {
      item.RecalculateBoundary(m_boundary.GetLeft(), m_boundary.GetBottom() + g_indent_vert, graphics);
      Gdiplus::RectF::Union(m_boundary, m_boundary, item.GetBoundary());
   }

   m_footer_prefix.RecalculateBoundary(m_boundary.GetLeft(), m_boundary.GetBottom() + g_indent_vert, graphics);
   m_footer_description.RecalculateBoundary(m_boundary.GetLeft() + m_footer_prefix.GetBoundary().GetRight() + g_indent_horz,
                                            m_boundary.GetBottom() + g_indent_vert, graphics);
   Gdiplus::RectF::Union(m_boundary, m_boundary, m_footer_prefix.GetBoundary());
   Gdiplus::RectF::Union(m_boundary, m_boundary, m_footer_description.GetBoundary());
}

const Gdiplus::RectF& Section::GetBoundary() const
{
   return m_boundary;
}

void Section::Draw(Gdiplus::Graphics* graphics) const
{
   m_title.Draw(graphics);
   m_header_description.Draw(graphics);
   for (auto& item : m_items)
   {
      item.Draw(graphics);
   }
   m_footer_prefix.Draw(graphics);
   m_footer_description.Draw(graphics);
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

         ::SetWindowPos(GetHandle(), nullptr,
                        m_window_rect.left,
                        m_window_rect.top,
                        m_window_rect.right - m_window_rect.left,
                        m_window_rect.bottom - m_window_rect.top,
                        SWP_NOZORDER);

         ::InvalidateRect(GetHandle(), nullptr, TRUE);
         
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

      memory_graphics->Clear(Gdiplus::Color(230, 230, 230));
      
      if (StateType::Minimized == m_state)
      {
         // TODO: Rework
         const Gdiplus::Font font(L"Tahoma", 9, Gdiplus::FontStyleBold);
         const Gdiplus::RectF rectf(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
         const Gdiplus::SolidBrush brush(Gdiplus::Color(0x72, 0xBE, 0x44));

         Gdiplus::StringFormat format;
         format.SetAlignment(Gdiplus::StringAlignmentCenter);

         memory_graphics->DrawString(
            static_cast<Section*>(m_sections[0].get())->GetTitle().c_str(),
            -1, &font, rectf, &format, &brush);
      }
      else
      {
         for (unsigned long index = 0; index < m_sections.size(); ++index)
         {
            // Show only first 3 sections in opened state
            if (StateType::Opened == m_state && index > 2)
            {
               break;
            }
            auto* section = static_cast<Section*>(m_sections[index].get());
            section->Draw(memory_graphics.get());
         }

         // TODO: Draw "next" footer.
      }
   }
            
   m_is_dirty = false;

   graphics.DrawImage(m_memory_face.get(), 0, 0);
}

void Sticker::SetState(StateType state)
{
   assert(!m_sections.empty());

   m_state = state;

   // Recalculate all rectangles
   if (StateType::Minimized == m_state)
   {
      m_window_rect = m_minimized_window_rect;
      return;
   }
   
   assert(m_memory_face != nullptr);
   std::unique_ptr<Gdiplus::Graphics> memory_graphics(Gdiplus::Graphics::FromImage(m_memory_face.get()));
   memory_graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
   
   Gdiplus::RectF boundary(0, 0, 0, 0);
   for (unsigned long index = 0; index < m_sections.size(); ++index)
   {
      // Show only first 3 sections in opened state
      if (StateType::Opened == m_state && index > 2)
      {
         break;
      }
      
      auto* section = static_cast<Section*>(m_sections[index].get());
      section->RecalculateBoundary(boundary.GetLeft(), boundary.GetBottom(), memory_graphics.get());
      Gdiplus::RectF::Union(boundary, boundary, section->GetBoundary());
   }

   // TODO: Draw "next" footer.

   // Take into account client edge
   boundary.Inflate(3, 3);

   ::SetRect(&m_window_rect,
             m_minimized_window_rect.left,
             m_minimized_window_rect.top,
             m_minimized_window_rect.left + static_cast<LONG>(boundary.Width + 0.5),
             m_minimized_window_rect.top + static_cast<LONG>(boundary.Height + 0.5));
}
