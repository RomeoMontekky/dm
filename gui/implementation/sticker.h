#pragma once

#include "window.h"

#include <gdiplus.h>

#include <vector>
#include <memory>

class ISection
{
public:
   virtual void SetName(const char* name) = 0;
   virtual void SetHeader(const char* description) = 0;
   virtual void SetFooter(const char* prefix, const char* description) = 0;
   virtual void SetItemCount(long count) = 0;
   virtual void SetItem(long index, const char* date, const char* time, const char* description) = 0;

   virtual ~ISection();
};

class Sticker : public wc::Window
{
public:
   Sticker();

   void SetDirty();
   void SetRedraw(bool is_redraw);
   void Update();

   void SetSectionCount(long count);
   ISection& GetSection(long index);

protected:
   virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

   void OnPaint(HDC hdc);

private:
   enum class StateType { Minimized, Opened, Expanded } m_state;

   std::unique_ptr<Gdiplus::Bitmap> m_bitmap;

   bool m_is_dirty;
   bool m_is_redraw;
   std::vector< std::unique_ptr<ISection> > m_sections;
};
