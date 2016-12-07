#include "sticker_objects.h"

#include <cassert>

// Sticker graphic objects namespace
namespace SGO
{

namespace
{

////////////////// Constants //////////////////

const wchar_t g_tahoma_name[] = L"Tahoma";
const auto g_indent_vert = 3;
const auto g_indent_horz = 3;

namespace Colors
{
   const Gdiplus::Color black(0x00, 0x00, 0x00);
   const Gdiplus::Color grey_light(0xCC, 0xCC, 0xCC);
   const Gdiplus::Color grey_very_light(0xF5, 0xF5, 0xF5);
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

/////////// class ItemDate //////////

ItemDate::ItemDate() :
   BGO::Text(Colors::grey_very_light, g_tahoma_name, 9, Gdiplus::FontStyleRegular, Colors::grey_dark_with_blue)
{}

/////////// class ItemTime //////////

ItemTime::ItemTime() :
   BGO::Text(Colors::grey_very_light, g_tahoma_name, 8, Gdiplus::FontStyleRegular, Colors::grey_light)
{}

/////////// class ItemDesctiption //////////

ItemDescription::ItemDescription() :
   BGO::ClickableText(Colors::grey_very_light, g_tahoma_name, 9, Gdiplus::FontStyleRegular, Colors::grey_dark_with_blue)
{}

/////////// class SectionItem //////////

SectionItem::SectionItem() : BGO::Group()
{
   BGO::Group::SetObjectCount(idxLast);
   BGO::Group::SetObject(idxDate, std::make_unique<ItemDate>(), BGO::Group::GluingType::Right, g_indent_horz);
   BGO::Group::SetObject(idxTime, std::make_unique<ItemTime>(), BGO::Group::GluingType::Right, g_indent_horz);
   BGO::Group::SetObject(idxDesc, std::make_unique<ItemDescription>(), BGO::Group::GluingType::Right, g_indent_horz);
}

bool SectionItem::SetDate(const char* text)
{
   return static_cast<BGO::Text*>(BGO::Group::GetObject(idxDate))->SetText(text);
}

bool SectionItem::SetTime(const char* text)
{
   return static_cast<BGO::Text*>(BGO::Group::GetObject(idxTime))->SetText(text);
}

bool SectionItem::SetDescription(const char* text)
{
   return static_cast<BGO::Text*>(BGO::Group::GetObject(idxDesc))->SetText(text);
}

/////////// class SectionHeader //////////

SectionHeader::SectionHeader() :
   BGO::Text(Colors::grey_very_light, g_tahoma_name, 9, Gdiplus::FontStyleRegular, Colors::grey_dark)
{}

/////////// class FooterPrefix //////////

FooterPrefix::FooterPrefix() :
   BGO::Text(Colors::grey_very_light, g_tahoma_name, 9, Gdiplus::FontStyleRegular, Colors::grey_dark)
{}

/////////// class FooterDescription //////////

FooterDescription::FooterDescription() :
   BGO::Text(Colors::grey_very_light, g_tahoma_name, 9, Gdiplus::FontStyleRegular, Colors::black)
{}

///////////// class SectionFooter ////////////////

SectionFooter::SectionFooter() : BGO::Group()
{
   BGO::Group::SetObjectCount(idxLast);
   BGO::Group::SetObject(idxPrefix, std::make_unique<FooterPrefix>(), BGO::Group::GluingType::Right, g_indent_horz);
   BGO::Group::SetObject(idxDesc, std::make_unique<FooterDescription>(), BGO::Group::GluingType::Right, g_indent_horz);
}

bool SectionFooter::SetPrefix(const char* text)
{
   return static_cast<BGO::Text*>(BGO::Group::GetObject(idxPrefix))->SetText(text);
}

bool SectionFooter::SetDescription(const char* text)
{
   return static_cast<BGO::Text*>(BGO::Group::GetObject(idxDesc))->SetText(text);
}

/////////// class SectionTitle //////////

SectionTitle::SectionTitle() :
   BGO::CollapsibleText(Colors::grey_very_light, g_tahoma_name, 9,
                        Gdiplus::FontStyleBold, Colors::red_dark, 8, Colors::grey_dark)
{}

///////////// class Section ////////////////

Section::Section(Sticker& sticker) : BGO::Group(), m_sticker(sticker)
{
   BGO::Group::SetObjectCount(idxLast);
   BGO::Group::SetObject(idxTitle, std::make_unique<SectionTitle>(), BGO::Group::GluingType::Bottom, g_indent_vert);
   BGO::Group::SetObject(idxHeader, std::make_unique<SectionHeader>(), BGO::Group::GluingType::Bottom, g_indent_vert);
   BGO::Group::SetObject(idxItems, std::make_unique<BGO::Group>(), BGO::Group::GluingType::Bottom, g_indent_vert);
   BGO::Group::SetObject(idxFooter, std::make_unique<SectionFooter>(), BGO::Group::GluingType::Bottom, g_indent_vert);
}

void Section::SetCollapsed(bool is_collapsed)
{
   static_cast<SGO::SectionTitle*>(BGO::Group::GetObject(idxTitle))->SetCollapsed(is_collapsed);
}

void Section::RecalculateTitleBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics)
{
   BGO::Group::GetObject(idxTitle)->RecalculateBoundary(x, y, graphics);
}

void Section::DrawTitle(Gdiplus::Graphics* graphics) const
{
   BGO::Group::GetObject(idxTitle)->Draw(graphics);
}

void Section::SetTitle(const char* title)
{
   if (static_cast<BGO::Text*>(BGO::Group::GetObject(idxTitle))->SetText(title))
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
   if (static_cast<SectionHeader*>(BGO::Group::GetObject(idxHeader))->SetText(description))
   {
      m_sticker.SetDirty();
   }
   m_sticker.Update();
}

void Section::SetFooter(const char* prefix, const char* description)
{
   auto footer = static_cast<SectionFooter*>(BGO::Group::GetObject(idxFooter));
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
   auto items = static_cast<BGO::Group*>(BGO::Group::GetObject(idxItems));
   if (items->SetObjectCount(count))
   {
      m_sticker.SetDirty();
   }
   m_sticker.Update();
}

void Section::SetItem(unsigned long index, const char* date, const char* time, const char* description)
{
   auto items = static_cast<BGO::Group*>(BGO::Group::GetObject(idxItems));

   auto item = static_cast<SectionItem*>(items->GetObject(index));
   if (nullptr == item)
   {
      auto item_ptr = std::make_unique<SectionItem>();
      item = item_ptr.get();
      items->SetObject(index, std::move(item_ptr), BGO::Group::GluingType::Bottom, g_indent_vert);
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

////////// class Sections /////////////

Sections::Sections(Sticker& sticker) : m_sticker(sticker)
{
   // no code
}

void Sections::SetSectionCount(unsigned long count)
{
   if (BGO::Group::SetObjectCount(count))
   {
      m_sticker.SetDirty();
   }
   m_sticker.Update();
}

unsigned long Sections::GetSectionCount() const
{
   return BGO::Group::GetObjectCount();     
}

const Section& Sections::GetSection(unsigned long index) const
{
   auto section = static_cast<const Section*>(BGO::Group::GetObject(index));
   assert(section != nullptr);
   return *section;
}

Section& Sections::GetSection(unsigned long index)
{
   auto section = static_cast<Section*>(BGO::Group::GetObject(index));
   if (nullptr == section)
   {
      auto section_ptr = std::make_unique<Section>(m_sticker);
      section = section_ptr.get();
      section->SetCollapsed(index > 0);
      BGO::Group::SetObject(index, std::move(section_ptr), BGO::Group::GluingType::Bottom, g_indent_vert);
   }
   return *section;
}

void Sections::CollapseAllExcludingFirst()
{
   const auto count = GetSectionCount();
   assert(count > 0);

   GetSection(0).SetCollapsed(false);
   for (auto index = 1UL; index < count; ++index)
   {
      GetSection(index).SetCollapsed(true);
   }
}

////////// class StickerGraphicObject /////////////

StickerObject::StickerObject(Sticker& sticker) :
   BGO::Group(), m_minimized_boundary(), m_state(StateType::Minimized), m_sticker(sticker)
{
   BGO::Group::SetObjectCount(idxLast);
   BGO::Group::SetObject(idxSections, std::make_unique<Sections>(sticker), BGO::Group::GluingType::Bottom, g_indent_vert);
   //Group::SetObject(idxEtc, std::make_unique<BGO::Text>(), BGO::Group::GluingType::Bottom, g_indent_vert);
}

void StickerObject::Initialize(const RECT& boundary)
{
   m_minimized_boundary.X = boundary.left;
   m_minimized_boundary.Y = boundary.top;
   m_minimized_boundary.Width = boundary.right - boundary.left;
   m_minimized_boundary.Height = boundary.bottom - boundary.top;
}

StickerObject::StateType StickerObject::GetState() const
{
   return m_state;
}

void StickerObject::SetSectionCount(unsigned long count)
{
   static_cast<Sections*>(BGO::Group::GetObject(idxSections))->SetSectionCount(count);
}

unsigned long StickerObject::GetSectionCount() const
{
   return static_cast<const Sections*>(BGO::Group::GetObject(idxSections))->GetSectionCount();
}

const Section& StickerObject::GetSection(unsigned long index) const
{
   return static_cast<const Sections*>(BGO::Group::GetObject(idxSections))->GetSection(index);
}

Section& StickerObject::GetSection(unsigned long index)
{
   return static_cast<Sections*>(BGO::Group::GetObject(idxSections))->GetSection(index);
}

void StickerObject::RecalculateBoundary(Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::Graphics* graphics)
{
   if (StateType::Minimized == m_state)
   {
      GetSection(0).RecalculateTitleBoundary(x, y, graphics);
      m_boundary = m_minimized_boundary;
   }
   else
   {
      BGO::Group::RecalculateBoundary(x, y, graphics);
   }
}

void StickerObject::Draw(Gdiplus::Graphics* graphics) const
{
   Gdiplus::SolidBrush back_brush(Colors::grey_very_light);
   graphics->FillRectangle(&back_brush, GetBoundary());

   if (StateType::Minimized == m_state)
   {
      GetSection(0).DrawTitle(graphics);
   }
   else
   {
      BGO::Group::Draw(graphics);
   }
}

BGO::Object::ClickType StickerObject::ProcessClick(long x, long y, BGO::TULongVector* group_indexes)
{
   if (StateType::Minimized == m_state)
   {
      m_state = StateType::Opened;
      return ClickType::ClickDoneNeedResize;
   }
   else
   {
      return BGO::Group::ProcessClick(x, y, group_indexes);
   }
}

} // namespace SGO
