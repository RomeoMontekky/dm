#include "window.h"
#include "window_class.h"
#include "sticker.h"

#include <gdiplus.h>

class GdiplusInitializer
{
public:
   GdiplusInitializer() : m_gdiplusToken(0)
   {
      Gdiplus::GdiplusStartupInput gdiplusStartupInput;
      Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
   }

   ~GdiplusInitializer()
   {
      Gdiplus::GdiplusShutdown(m_gdiplusToken);
   }

private:
   ULONG_PTR m_gdiplusToken;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   GdiplusInitializer gdi_initializer;

   wc::MainWindow main_window;
   main_window.Create("Main Window", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 800, 300, 300, 300);
   ::ShowWindow(main_window.GetHandle(), nCmdShow);

   Sticker sticker;
   sticker.Create(nullptr, WS_CHILD|WS_VISIBLE|WS_DLGFRAME, 0, 0, 70, 22, main_window.GetHandle());
   
   sticker.SetRedraw(false);
   {
      sticker.SetSectionCount(2);
      {
         auto& section = sticker.GetSection(0);
         section.SetTitle("Section 1");
         section.SetItemCount(2);
         section.SetItem(0, "21.09", "12:45", "Send");
         section.SetItem(1, "21.09", "13:00", "Received");
      }
      {
         auto& section = sticker.GetSection(1);
         section.SetTitle("Section 2");
         section.SetItemCount(2);
         section.SetItem(0, "22.09", "09:00", "Send");
         section.SetItem(1, "22.09", "09:15", "Received");
      }
   }
   sticker.SetRedraw(true);

   MSG msg;
   // Main message loop.
   while (GetMessage(&msg, nullptr, 0, 0) != 0)
   {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
   }

   return msg.wParam;
}
