#include "sticker.h"
#include "graphic_objects.h"

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

} // namespace

// Sticker graphic objects
namespace GraphicObjects
{

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
   
private:
   enum Indexes { idxDate, idxTime, idxDesc, idxLast };
};

Item::Item() : Group()
{
   Group::SetObjectCount(idxLast);
   Group::SetObject(idxDate, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
   Group::SetObject(idxTime, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
   Group::SetObject(idxDesc, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
}

bool Item::SetDate(const char* text)
{
   return static_cast<Text*>(Group::GetObject(idxDate))->SetText(text);
}

bool Item::SetTime(const char* text)
{
   return static_cast<Text*>(Group::GetObject(idxTime))->SetText(text);
}

bool Item::SetDescription(const char* text)
{
   return static_cast<Text*>(Group::GetObject(idxDesc))->SetText(text);
}

///////////// class Footer ////////////////

class Footer : public Group
{
public:
   Footer();

   bool SetPrefix(const char* text);
   bool SetDescription(const char* text);
   
private:
   enum Indexes { idxPrefix, idxDesc, idxLast };
};

Footer::Footer()
{
   Group::SetObjectCount(idxLast);
   Group::SetObject(idxPrefix, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
   Group::SetObject(idxDesc, std::make_unique<Text>(), Group::GluingType::Right, g_indent_horz);
}

bool Footer::SetPrefix(const char* text)
{
   return static_cast<Text*>(Group::GetObject(idxPrefix))->SetText(text);
}

bool Footer::SetDescription(const char* text)
{
   return static_cast<Text*>(Group::GetObject(idxDesc))->SetText(text);
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
   enum Indexes { idxTitle, idxHeader, idxItems, idxFooter, idxLast };
   Sticker& m_sticker;
};

Section::Section(Sticker& sticker) :
   m_sticker(sticker)
{
   Group::SetObjectCount(idxLast);
   Group::SetObject(idxTitle, std::make_unique<Text>(), Group::GluingType::Bottom, g_indent_vert);
   Group::SetObject(idxHeader, std::make_unique<Header>(), Group::GluingType::Bottom, g_indent_vert);
   Group::SetObject(idxItems, std::make_unique<Group>(), Group::GluingType::Bottom, g_indent_vert);
   Group::SetObject(idxFooter, std::make_unique<Footer>(), Group::GluingType::Bottom, g_indent_vert);
}

const std::wstring& Section::GetTitle() const
{
   return static_cast<const Text*>(Group::GetObject(idxTitle))->GetText();
}

void Section::SetTitle(const char* title)
{
   if (static_cast<Text*>(Group::GetObject(idxTitle))->SetText(title))
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
   if (static_cast<Header*>(Group::GetObject(idxHeader))->SetText(description))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetFooter(const char* prefix, const char* description)
{
   auto footer = static_cast<Footer*>(Group::GetObject(idxFooter));

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
   auto items = static_cast<Group*>(Group::GetObject(idxItems));

   if (items->SetObjectCount(count))
   {
      m_sticker.SetDirty();
   }
   
   m_sticker.Update();
}

void Section::SetItem(unsigned long index, const char* date, const char* time, const char* description)
{
   auto items = static_cast<Group*>(Group::GetObject(idxItems));

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

////////// class StickerObject /////////////

class Sticker::StickerObject : public GraphicObjects::Group
{
public:
   StickerObject(Sticker& sticker);
   
   bool SetSectionCount(unsigned long count);
   GraphicObjects::Section& GetSection(unsigned long index);
   
private:
   enum Indexes { idxSections, idxEtc, idxLast };
   
   enum class StateType { Minimized, Opened, Expanded };
   StateType m_state;
   Sticker& m_sticker;
};

Sticker::StickerObject::StickerObject(Sticker& sticker) : 
   Group(), m_state(StateType::Minimized), m_sticker(sticker)
{
   Group::SetObjectCount(idxLast);
   Group::SetObject(idxSections, std::make_unique<GraphicObjects::Group>(), GraphicObjects::Group::GluingType::Bottom, g_indent_vert);
   Group::SetObject(idxEtc, std::make_unique<GraphicObjects::Text>(), GraphicObjects::Group::GluingType::Bottom, g_indent_vert);
}

bool Sticker::StickerObject::SetSectionCount(unsigned long count)
{
   auto sections = static_cast<Group*>(Group::GetObject(idxSections));
   if (sections->SetObjectCount(count))
   {
      m_sticker.SetDirty();
   }
   m_sticker.Update();
}

GraphicObjects::Section& Sticker::StickerObject::GetSection(unsigned long index)
{
   auto sections = static_cast<Group*>(Group::GetObject(idxSections));
   auto section = static_cast<GraphicObjects::Section*>(sections->GetObject(index));
   if (nullptr == section)
   {
      auto section_ptr = std::make_unique<GraphicObjects::Section>(m_sticker);
      section = section_ptr.get();
      sections->SetObject(index, std::move(section_ptr), Group::GluingType::Bottom, g_indent_vert);
   }
   return *section;
}

/////////////// class Sticker /////////////////

Sticker::Sticker() : wc::Window(),
   m_object(new StickerObject()),
   m_memory_face(),
   m_callback(),
   m_is_dirty(true),
   m_is_redraw(true),
   m_sections()
{
   ::SetRectEmpty(&m_minimized_window_rect);
   ::SetRectEmpty(&m_window_rect);
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
