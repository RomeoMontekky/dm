#include "graphic_objects.h"

#include <cstring>
#include <cassert>

namespace
{

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

///////////// class Text /////////////
   
Text::Text(const wchar_t* font_name, unsigned long font_size,
           unsigned long font_style, Gdiplus::Color font_color) :
   Base(), m_font_name(font_name), m_font_size(font_size),
   m_font_style(font_style), m_font_color(font_color)
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
   Gdiplus::RectF origin_rect(x, y, 0, 0);
   Gdiplus::Font font(GetFontName(), GetFontSize(), GetFontStyle());
   graphics->MeasureString(m_text.c_str(), m_text.size(), &font, origin_rect, &m_boundary);
}

void Text::Draw(Gdiplus::Graphics* graphics) const
{
   Gdiplus::Font font(GetFontName(), GetFontSize(), GetFontStyle());
   Gdiplus::SolidBrush brush(GetFontColor());
   graphics->DrawString(m_text.c_str(), m_text.size(), &font, m_boundary, nullptr, &brush);
}

const wchar_t* Text::GetFontName() const
{
   return m_font_name;
}

unsigned long Text::GetFontSize() const
{
   return m_font_size;
}

unsigned long Text::GetFontStyle() const
{
   return m_font_style;
}

Gdiplus::Color Text::GetFontColor() const
{
   return m_font_color;
}

///////////// class Group ////////////////

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

const Base* Group::GetObject(unsigned long index) const
{
   return m_object_infos.at(index).m_object.get();
}

Base* Group::GetObject(unsigned long index)
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
         indent_x + ((GluingType::Right == object_info.m_gluing_type) ? m_boundary.GetRight() : m_boundary.GetLeft()),
         indent_y + ((GluingType::Bottom == object_info.m_gluing_type) ? m_boundary.GetBottom() : m_boundary.GetTop()),
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

} // namespace GraphicObjects
