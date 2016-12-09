#include "window.h"
#include "window_class.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   wc::MainWindow main_window;
   main_window.Create("Main Window", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 800, 300, 300, 300);
   ::ShowWindow(main_window.GetHandle(), nCmdShow);

   MSG msg;
   // Main message loop.
   while (GetMessage(&msg, nullptr, 0, 0) != 0)
   {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
   }

   return msg.wParam;
}
