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
   sticker.Create(nullptr, WS_CHILD|WS_VISIBLE|WS_DLGFRAME, 0, 0, 100, 35, main_window.GetHandle());

   MSG msg;
   // Main message loop.
   while (GetMessage(&msg, nullptr, 0, 0) != 0)
   {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
   }

   return msg.wParam;
}
