#include "window.h"
#include "window_class.h"

#include <gdiplus.h>

class DMMainWindow : public wc::MainWindow
{
public:
   DMMainWindow();
   virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   void OnPaint(HDC hdc);
};

DMMainWindow::DMMainWindow() : MainWindow()
{}

LRESULT DMMainWindow::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (WM_PAINT == uMsg)
   {
      PAINTSTRUCT ps;
      HDC hdc = ::BeginPaint(GetHandle(), &ps);
      OnPaint(hdc);
      ::EndPaint(GetHandle(), &ps);
      return 0;
   }
   return MainWindow::WindowProc(uMsg, wParam, lParam);
}

void DMMainWindow::OnPaint(HDC hdc)
{
   Gdiplus::Graphics graphics(hdc);

   Gdiplus::RectF rect;
   rect.X = 10;
   rect.Y = 10;
   rect.Width = 1;
   rect.Height = 1;

   const Gdiplus::SolidBrush brush(Gdiplus::Color(128, 128, 128));
   graphics.FillRectangle(&brush, rect);

   rect.X = 20;
   const Gdiplus::Pen pen(Gdiplus::Color(0, 0, 0));
   graphics.DrawRectangle(&pen, rect);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   ULONG_PTR gdiplusToken = 0;
   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

   DMMainWindow main_window;
   main_window.Create("Main Window", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 800, 300, 300, 300);
   ::ShowWindow(main_window.GetHandle(), nCmdShow);

   MSG msg;
   // Main message loop.
   while (GetMessage(&msg, nullptr, 0, 0) != 0)
   {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
   }

   if (gdiplusToken != 0)
   {
      Gdiplus::GdiplusShutdown(gdiplusToken);
   }

   return msg.wParam;
}
