#include "sticker.h"

#include <string>

ISection::~ISection()
{
   // no code
}

namespace
{

class Section : public ISection
{
public:
   Section(Sticker& sticker);

   const std::string& GetName() const;
   const std::string& GetHeaderDescription() const;
   const std::string& GetFooterPrefex() const;
   const std::string& GetFooterDescription() const;

   struct Item
   {
      std::string m_date;
      std::string m_time;
      std::string m_description;
   };

   long GetItemCount() const;
   const Item& GetItem(long index) const;

   // ISection overrides
   virtual void SetName(const char* name) override;
   virtual void SetHeader(const char* description) override;
   virtual void SetFooter(const char* prefix, const char* description) override;
   virtual void SetItemCount(long count) override;
   virtual void SetItem(long index, const char* date, const char* time, const char* description) override;

private:
   std::string m_name;
   std::string m_header_description;
   std::string m_footer_prefix;
   std::string m_footer_description;

   std::vector<Item> m_items;

   Sticker& m_sticker;
};

Section::Section(Sticker& sticker) :
   m_name(),
   m_header_description(),
   m_footer_prefix(),
   m_footer_description(),
   m_items(),
   m_sticker(sticker)
{
   // no code
}

const std::string& Section::GetName() const
{
   return m_name;
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

long Section::GetItemCount() const
{
   return static_cast<long>(m_items.size());
}

const Section::Item& Section::GetItem(long index) const
{
   return m_items.at(index);
}

void Section::SetName(const char* name)
{
   if (m_name != name)
   {
      m_name = name;
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

void Section::SetItemCount(long count)
{
   if ((long)m_items.size() != count)
   {
      m_items.resize(count);
      m_sticker.SetDirty();
   }
   m_sticker.Update();
}

void Section::SetItem(long index, const char* date, const char* time, const char* description)
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
   m_is_dirty(false),
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

   Gdiplus::Graphics graphics(hdc);

   if (m_is_dirty)
   {
      m_bitmap.reset(new Gdiplus::Bitmap(rect.right - rect.left, rect.bottom - rect.top, &graphics));
      std::unique_ptr<Gdiplus::Graphics> bitmap_graphics(Gdiplus::Graphics::FromImage(m_bitmap.get()));

      Gdiplus::Pen pen(Gdiplus::Color(0, 0, 0), 2);
      bitmap_graphics->DrawEllipse(&pen, (INT)rect.left, rect.top, rect.right, rect.bottom);

      // Draw into bitmap using bitmap_graphics
      m_is_dirty = false;
   }

   graphics.DrawImage(m_bitmap.get(), 0, 0);
}

/*
class DMMainWindow : public wc::MainWindow
{
protected:
   LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
   {
      switch (uMsg)
      {
         case WM_PAINT:
         {
            PAINTSTRUCT ps;
            //HDC hdc = ::GetDC(GetHandle());
            HDC hdc = ::BeginPaint(GetHandle(), &ps);
            OnPaint(hdc);
            ::EndPaint(GetHandle(), &ps);
            //::ReleaseDC(GetHandle(), hdc);
            return 0;
         }
      }
      return MainWindow::WindowProc(uMsg, wParam, lParam);
   }

private:
   void OnPaint(HDC dc);
};

void DMMainWindow::OnPaint(HDC hdc)
{
   Gdiplus::Graphics graphics(hdc);

   RECT rect;
   ::GetClientRect(GetHandle(), &rect);

   Gdiplus::LinearGradientBrush brush(
      Gdiplus::Point(0, 0),
      Gdiplus::Point(400, 0),
      Gdiplus::Color(255, 255, 0, 0),   // opaque red
      Gdiplus::Color(255, 0, 0, 255));  // opaque blue

   Gdiplus::Rect gdi_rect(rect.left, rect.top, rect.right, rect.bottom);
   graphics.FillRectangle(&brush, gdi_rect);
}

*/

