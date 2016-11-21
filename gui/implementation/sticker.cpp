#include "sticker.h"

#include <string>

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

} // namespace

/////////////// Sticker /////////////////

Sticker::Sticker() :
   wc::Window(),
   m_state(StateType::Minimized),
   m_memory_face(),
   m_is_dirty(true),
   m_is_redraw(true),
   m_sections()
{
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

void Sticker::OnPaint(HDC hdc)
{
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

      Gdiplus::Pen pen(Gdiplus::Color(0, 0, 0), 1);
      memory_graphics->DrawEllipse(&pen, (INT)rect.left + 1, rect.top + 1, rect.right - 5, rect.bottom - 5);

      m_is_dirty = false;
   }

   graphics.DrawImage(m_memory_face.get(), 0, 0);
}
