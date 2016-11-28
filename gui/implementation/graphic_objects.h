﻿#pragma one

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <vector>
#include <string>

namespace GraphicObjects
{

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

} // namespace GraphicObjects
