#include "sticker.h"

#include <vector>
#include <string>
#include <memory>
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

} // namespace

namespace GraphicObjects
{

///////////// class GraphicObject ////////

class Base
{
   Base(const Base& rhs) = delete;

public:
   Base();
   virtual ~Base();

   const Gdiplus::RectF& GetBoundary() const;

   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) = 0;
   virtual void Draw(Gdiplus::Graphics* graphics) const = 0;

protected:
   Gdiplus::RectF m_boundary;
};

Base::Base() : m_boundary()
{
   // no code
}

Base::~Base()
{
   // no code
}

const Gdiplus::RectF& Base::GetBoundary() const
{
   return m_boundary;
}

///////////// class TextInfo /////////////
   
class Text : public Base
{
public:
   Text(/*const Gdiplus::Font& font, const Gdiplus::SolidBrush& brush*/);
   
   bool SetText(const char* text);
   const std::wstring& GetText() const;
   
   // GraphicObject overrides
   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) override;
   virtual void Draw(Gdiplus::Graphics* graphics) const override;
   
private:
   std::wstring m_text;
   //const Gdiplus::Font& m_font;
   //const Gdiplus::Brush& m_brush;
};

Text::Text(/*const Gdiplus::Font& font, const Gdiplus::SolidBrush& brush*/) :
   m_text() /*, m_font(font), m_brush(brush)*/
{
   // no code
}

bool Text::SetText(const char* text)
{
   const auto wide_text = ::AsciiToWide(text);
   if (m_text != wide_text)
   {
      m_text = wide_text;
      return true;
   }
   return false;
}

const std::wstring& Text::GetText() const
{
   return m_text;
}

void Text::RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics)
{
   const Gdiplus::Font font(g_tahoma_name, 9, Gdiplus::FontStyleRegular);
   //const Gdiplus::SolidBrush black(Gdiplus::Color(0x00, 0x00, 0x00));
   Gdiplus::RectF origin_rect(x, y, 0, 0);
   //auto status = graphics->MeasureString(m_text.c_str(), m_text.size(), &m_font, origin_rect, &m_boundary);
   auto status = graphics->MeasureString(m_text.c_str(), m_text.size(), &font, origin_rect, &m_boundary);
}

void Text::Draw(Gdiplus::Graphics* graphics) const
{
   const Gdiplus::Font font(g_tahoma_name, 9, Gdiplus::FontStyleRegular);
   const Gdiplus::SolidBrush black(Gdiplus::Color(0x00, 0x00, 0x00));
   auto status = graphics->DrawString(m_text.c_str(), m_text.size(), &font, m_boundary, nullptr, &black);
}

///////////// class Group ////////////////

class Group : public Base
{
public:
   Group(Gdiplus::REAL indent_before_x = 0, Gdiplus::REAL indent_before_y = 0);

   enum class GluingType { Right, Bottom };

   bool SetObjectCount(unsigned long count);
   void SetObject(unsigned long index, std::unique_ptr<Base>&& object,
                  GluingType gluing_type, Gdiplus::REAL indent_after = 0);
   Base* GetObject(unsigned long index);
   const Base* GetObject(unsigned long index) const;

   // Base overrides
   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) override;
   virtual void Draw(Gdiplus::Graphics* graphics) const override;

private:
   Gdiplus::REAL m_indent_before_x;
   Gdiplus::REAL m_indent_before_y;

   struct ObjectInfo
   {
      std::unique_ptr<Base> m_object;
      GluingType m_gluing_type;
      Gdiplus::REAL m_indent_after;
   };

   std::vector<ObjectInfo> m_object_infos;
};

Group::Group(Gdiplus::REAL indent_before_x, Gdiplus::REAL indent_before_y) :
   m_indent_before_x(indent_before_x), m_indent_before_y(indent_before_y), m_object_infos()
{
   m_object_infos.reserve(10);
}

bool Group::SetObjectCount(unsigned long count)
{
   if (m_object_infos.size() == count)
   {
      return false;
   }
   m_object_infos.resize(count);
   return true;
}

void Group::SetObject(unsigned long index, std::unique_ptr<Base>&& object,
                      GluingType gluing_type, Gdiplus::REAL indent_after)
{
   auto& object_info = m_object_infos.at(index);
   object_info.m_object = std::move(object);
   object_info.m_gluing_type = gluing_type;
   object_info.m_indent_after = indent_after;
}

Base* Group::GetObject(unsigned long index)
{
   return m_object_infos.at(index).m_object.get();
}

const Base* Group::GetObject(unsigned long index) const
{
   return m_object_infos.at(index).m_object.get();
}

void Group::RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics)
{
   m_boundary.X = x;
   m_boundary.Y = y;
   m_boundary.Width = 0;
   m_boundary.Height = 0;

   Gdiplus::REAL indent_x = m_indent_before_x;
   Gdiplus::REAL indent_y = m_indent_before_y;

   for (auto& object_info : m_object_infos)
   {
      assert(object_info.m_object);

      object_info.m_object->RecalculateBoundary
      (
         indent_x + (GluingType::Right == object_info.m_gluing_type) ? m_boundary.GetRight() : m_boundary.GetLeft(),
         indent_y + (GluingType::Bottom == object_info.m_gluing_type) ? m_boundary.GetBottom() : m_boundary.GetTop(),
         graphics
      );

      if (GluingType::Right == object_info.m_gluing_type)
      {
         indent_x = object_info.m_indent_after;
         indent_y = 0;
      }
      else
      {
         indent_x = 0;
         indent_y = object_info.m_indent_after;
      }

      Gdiplus::RectF::Union(m_boundary, m_boundary, object_info.m_object->GetBoundary());
   }

}

void Group::Draw(Gdiplus::Graphics* graphics) const
{
   for (auto& object_info : m_object_infos)
   {
      object_info.m_object->Draw(graphics);
   }
}

/////////// class Header //////////

// TODO: Create class and change text filling
using Header = Text;

/////////// class Item //////////

class Item : public Group
{
public:
   Item();

   bool SetDate(const char* text);
   bool SetTime(const char* text);
   bool SetDescription(const char* text);
};

Item::Item() : Group()
{
   Group::SetObjectCount(3);
   Group::SetObject(0, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
   Group::SetObject(1, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
   Group::SetObject(2, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
}

bool Item::SetDate(const char* text)
{
   return static_cast<Text*>(Group::GetObject(0))->SetText(text);
}

bool Item::SetTime(const char* text)
{
   return static_cast<Text*>(Group::GetObject(1))->SetText(text);
}

bool Item::SetDescription(const char* text)
{
   return static_cast<Text*>(Group::GetObject(2))->SetText(text);
}

///////////// class Footer ////////////////

class Footer : public Group
{
public:
   Footer();

   bool SetPrefix(const char* text);
   bool SetDescription(const char* text);
};

Footer::Footer()
{
   Group::SetObjectCount(2);
   Group::SetObject(0, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
   Group::SetObject(1, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
}

bool Footer::SetPrefix(const char* text)
{
   return static_cast<Text*>(Group::GetObject(0))->SetText(text);
}

bool Footer::SetDescription(const char* text)
{
   return static_cast<Text*>(Group::GetObject(1))->SetText(text);
}

///////////// class Section ////////////////

class Section : public ISection, public Group
{
public:
   Section(Sticker& sticker);

   const std::wstring& GetTitle() const;

   // ISection overrides
   virtual void SetTitle(const char* title) override;
   virtual void SetOwnerName(const char* owner_name) override;
   virtual void SetHeader(const char* description) override;
   virtual void SetFooter(const char* prefix, const char* description) override;
   virtual void SetItemCount(unsigned long count) override;
   virtual void SetItem(unsigned long index, const char* date, const char* time, const char* description) override;

private:
/*
   GraphicObjects::Text m_title;
   GraphicObjects::Text m_owner_name;
   GraphicObjects::Text m_header_description;
   GraphicObjects::Text m_footer_prefix;
   GraphicObjects::Text m_footer_description;

   GraphicObjects::Group m_items;

   Gdiplus::RectF m_boundary;
*/

   Sticker& m_sticker;
};

Section::Section(Sticker& sticker) :
/*
   m_title(Fonts::tahoma_9_bold, Brushes::red_dark),
   m_owner_name(Fonts::tahoma_9_regular, Brushes::black),
   m_header_description(Fonts::tahoma_9_regular, Brushes::grey_dark),
   m_footer_prefix(Fonts::tahoma_9_regular, Brushes::grey_dark),
   m_footer_description(Fonts::tahoma_9_regular, Brushes::black),
   m_items(),
   m_boundary(),
   m_items(),*/
   m_sticker(sticker)
{
   Group::SetObjectCount(4);
   Group::SetObject(0, std::make_unique<Text>(), Group::GluingType::Bottom, g_indent_vert);
   Group::SetObject(1, std::make_unique<Header>(), Group::GluingType::Bottom, g_indent_vert);
   Group::SetObject(2, std::make_unique<Group>(), Group::GluingType::Bottom, g_indent_vert);
   Group::SetObject(3, std::make_unique<Footer>(), Group::GluingType::Bottom, g_indent_vert);
}

const std::wstring& Section::GetTitle() const
{
   return static_cast<const Text*>(Group::GetObject(0))->GetText();
}

/*
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
*/
/*
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
*/

void Section::SetTitle(const char* title)
{
   if (static_cast<Text*>(Group::GetObject(0))->SetText(title))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetOwnerName(const char* owner_name)
{
/*
   if (m_owner_name.SetText(owner_name))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();*/
}

void Section::SetHeader(const char* description)
{
   if (static_cast<Header*>(Group::GetObject(1))->SetText(description))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetFooter(const char* prefix, const char* description)
{
   auto footer = static_cast<Footer*>(Group::GetObject(3));

   if (footer->SetPrefix(prefix))
   {
      m_sticker.SetDirty();
   }
   
   if (footer->SetDescription(description))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetItemCount(unsigned long count)
{
   auto items = static_cast<Group*>(Group::GetObject(2));

   if (items->SetObjectCount(count))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetItem(unsigned long index, const char* date, const char* time, const char* description)
{
   auto items = static_cast<Group*>(Group::GetObject(2));

   auto item = static_cast<Item*>(items->GetObject(index));
   if (nullptr == item)
   {
      auto item_ptr = std::make_unique<Item>();
      item = item_ptr.get();
      items->SetObject(index, std::move(item_ptr), Group::GluingType::Bottom, g_indent_vert);
   }

   if (item->SetDate(date))
   {
      m_sticker.SetDirty();
   }
   
   if (item->SetTime(time))
   {
      m_sticker.SetDirty();
   }
   
   if (item->SetDescription(description))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

} // namespace GraphicObjects

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
      section.reset(new GraphicObjects::Section(*this));
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
            static_cast<GraphicObjects::Section*>(m_sections[0].get())->GetTitle().c_str(),
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
            auto* section = static_cast<GraphicObjects::Section*>(m_sections[index].get());
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
      
      auto* section = static_cast<GraphicObjects::Section*>(m_sections[index].get());
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
