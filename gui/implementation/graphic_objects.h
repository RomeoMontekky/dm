#pragma one

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
   virtual bool ProcessMouseClick(long x, long y);
   virtual bool ProcessMouseMove(long x, long y);

protected:
   Gdiplus::RectF m_boundary;
};

class Text : public Base
{
public:
   Text(const wchar_t* font_name, unsigned long font_size,
        unsigned long font_style, Gdiplus::Color font_color);
   
   bool SetText(const char* text);
   const std::wstring& GetText() const;
   
   // GraphicObject overrides
   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) override;
   virtual void Draw(Gdiplus::Graphics* graphics) const override;

   // Own virtual methods
   virtual const wchar_t* GetFontName() const;
   virtual unsigned long GetFontSize() const;
   virtual unsigned long GetFontStyle() const;
   virtual Gdiplus::Color GetFontColor() const;
   
private:
   std::wstring m_text;
   const wchar_t* m_font_name;
   unsigned long m_font_size;
   unsigned long m_font_style;
   Gdiplus::Color m_font_color;
};

class ClickableText : public Text
{
public:
   ClickableText(const wchar_t* font_name, unsigned long font_size,
                 unsigned long font_style, Gdiplus::Color font_color, bool is_clickable = true);

   bool SetClickable(bool is_clickable);

   // Base and Text overrides
   virtual bool ProcessMouseClick(long x, long y) override;
   virtual bool ProcessMouseMove(long x, long y) override;
   virtual unsigned long GetFontStyle() const override;

private:
   bool m_is_clickable;
   bool m_is_clickable_view;
};

class Group : public Base
{
public:
   Group(Gdiplus::REAL indent_before_x = 0, Gdiplus::REAL indent_before_y = 0);

   enum class GluingType { Right, Bottom };

   bool SetObjectCount(unsigned long count);
   unsigned long GetObjectCount() const;
   
   void SetObject(unsigned long index, std::unique_ptr<Base>&& object,
                  GluingType gluing_type, Gdiplus::REAL indent_after = 0);
   const Base* GetObject(unsigned long index) const;
   Base* GetObject(unsigned long index);

   // Base overrides
   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) override;
   virtual void Draw(Gdiplus::Graphics* graphics) const override;
   virtual bool ProcessMouseClick(long x, long y) override;
   virtual bool ProcessMouseMove(long x, long y) override;

protected:
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
