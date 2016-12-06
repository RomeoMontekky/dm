#pragma once

#include "window.h"

#include <gdiplus.h>

#include <vector>
#include <memory>

class ISection
{
public:
   virtual void SetTitle(const char* title) = 0;
   virtual void SetOwnerName(const char* owner_name) = 0;
   virtual void SetHeader(const char* description) = 0;
   virtual void SetFooter(const char* prefix, const char* description) = 0;
   virtual void SetItemCount(unsigned long count) = 0;
   virtual void SetItem(unsigned long index, const char* date, const char* time, const char* description) = 0;

   virtual ~ISection();
};

class IStickerCallback
{
public:
   virtual void OnHeaderClick(unsigned long section_index) = 0;
   virtual void OnItemClick(unsigned long section_index, unsigned long item_index) = 0;
   virtual void OnFooterClick(unsigned long section_index) = 0;
   
   virtual ~IStickerCallback();
};

namespace SGO
{
   // Forward declaration to use in PIMPL.
   class StickerObject;
}

class Sticker : public wc::Window
{
public:
   Sticker();
   ~Sticker();

   void SetDirty();
   void SetRedraw(bool is_redraw);
   void Update();

   void SetSectionCount(unsigned long count);
   ISection& GetSection(unsigned long index);
   
   void SetCallback(std::unique_ptr<IStickerCallback>&& callback);

protected:
   virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   
private:
   void OnLButtonUp(long x, long y);
   void OnMouseMove(long x, long y);
   void OnMouseHover(long x, long y);
   void OnMouseLeave();
   void OnPaint(HDC hdc);
   
   void ProcessHover(long x, long y);

private:
   bool m_is_dirty;
   bool m_is_redraw;
   bool m_is_mouse_tracking;
   
   std::unique_ptr<Gdiplus::Bitmap> m_memory_image;
   std::unique_ptr<IStickerCallback> m_callback;
   std::unique_ptr<SGO::StickerObject> m_object;
};
