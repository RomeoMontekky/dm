#pragma one

#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include <vector>
#include <string>

namespace BGO
{

class Object;
using TObjectPtrVector = std::vector<Object*>;
using TULongVector = std::vector<unsigned long>;

class Object
{
   Object(const Object& rhs) = delete;

public:
   Object();
   virtual ~Object();

   const Gdiplus::RectF& GetBoundary() const;

   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) = 0;
   virtual void Draw(Gdiplus::Graphics* graphics) const = 0;
   virtual bool ProcessClick(long x, long y, TULongVector& group_indexes);
   virtual void ProcessHover(long x, long y, TObjectPtrVector& invalidated_objects);

protected:
   Gdiplus::RectF m_boundary;
};

class ObjectWithBackground : public Object
{
public:
   ObjectWithBackground(const Gdiplus::Color& back_color);

   // Object overrides
   virtual void Draw(Gdiplus::Graphics* graphics) const override;

private:
   Gdiplus::Color m_back_color;
};

class Text : public ObjectWithBackground
{
public:
   Text(const Gdiplus::Color& back_color,
        const wchar_t* font_name, unsigned long font_size,
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
   ClickableText(const Gdiplus::Color& back_color,
                 const wchar_t* font_name, unsigned long font_size,
                 unsigned long font_style, Gdiplus::Color font_color, bool is_clickable = true);

   bool SetClickable(bool is_clickable);

   // Base and Text overrides
   virtual bool ProcessClick(long x, long y, TULongVector& group_indexes) override;
   virtual void ProcessHover(long x, long y, TObjectPtrVector& invalidated_objects) override;
   virtual unsigned long GetFontStyle() const override;

private:
   bool m_is_clickable;
   bool m_is_clickable_view;
};

class Group : public Object
{
public:
   Group(Gdiplus::REAL indent_before_x = 0, Gdiplus::REAL indent_before_y = 0);

   enum class GluingType { Right, Bottom };

   bool SetObjectCount(unsigned long count);
   unsigned long GetObjectCount() const;
   
   void SetObject(unsigned long index, std::unique_ptr<Object>&& object,
                  GluingType gluing_type, Gdiplus::REAL indent_after = 0);
   const Object* GetObject(unsigned long index) const;
   Object* GetObject(unsigned long index);

   // Base overrides
   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) override;
   virtual void Draw(Gdiplus::Graphics* graphics) const override;
   virtual bool ProcessClick(long x, long y, TULongVector& group_indexes) override;
   virtual void ProcessHover(long x, long y, TObjectPtrVector& invalidated_objects) override;

protected:
   Gdiplus::REAL m_indent_before_x;
   Gdiplus::REAL m_indent_before_y;

   struct ObjectInfo
   {
      std::unique_ptr<Object> m_object;
      GluingType m_gluing_type;
      Gdiplus::REAL m_indent_after;
   };

   std::vector<ObjectInfo> m_object_infos;
};

} // namespace BGO
