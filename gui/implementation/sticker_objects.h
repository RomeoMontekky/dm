#pragma once

#include "graphic_objects.h"
#include "sticker.h"

// Sticker graphic objects namespace
namespace SGO
{

class ItemDate : public BGO::Text
{
public:
   ItemDate();
};

class ItemTime : public BGO::Text
{
public:
   ItemTime();
};

class ItemDescription : public BGO::ClickableText
{
public:
   ItemDescription();
};

class SectionItem : public BGO::Group
{
public:
   SectionItem();

   bool SetDate(const char* text);
   bool SetTime(const char* text);
   bool SetDescription(const char* text);

private:
   enum Indexes { idxDate, idxTime, idxDesc, idxLast };
};

// TODO: Change text filling
class SectionHeader : public BGO::Text
{
public:
   SectionHeader();
};

class FooterPrefix : public BGO::Text
{
public:
   FooterPrefix();
};

class FooterDescription : public BGO::Text
{
public:
   FooterDescription();
};

class SectionFooter : public BGO::Group
{
public:
   SectionFooter();

   bool SetPrefix(const char* text);
   bool SetDescription(const char* text);

private:
   enum Indexes { idxPrefix, idxDesc, idxLast };
};

// TODO: Finish behavior implementation
class SectionTitle : public BGO::Text
{
public:
   SectionTitle();
};

class Section : public BGO::Group, public ISection
{
public:
   Section(Sticker& sticker);

   void RecalculateTitleBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics);
   void DrawTitle(Gdiplus::Graphics* graphics) const;

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

class StickerObject : public BGO::Group
{
public:
   StickerObject(Sticker& sticker);

   void SetMinimizedBoundary(const RECT& boundary);

   enum class StateType { Minimized, Opened, Expanded };
   StateType GetState() const;

   bool SetSectionCount(unsigned long count);
   unsigned long GetSectionCount() const;

   const Section& GetSection(unsigned long index) const;
   Section& GetSection(unsigned long index);

   // Group overrides
   virtual void RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics) override;
   virtual void Draw(Gdiplus::Graphics* graphics) const override;
   virtual const Object* ProcessMouseClick(long x, long y) override;

private:
   enum Indexes { idxSections, /*idxEtc,*/ idxLast };

   Gdiplus::RectF m_minimized_boundary;
   StateType m_state;
   Sticker& m_sticker;
};

} // namespace SGO


