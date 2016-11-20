#pragma once

#include <windows.h>

namespace wc
{

class WindowClass;

class Window
{
   friend class WindowClass;

   Window(const Window& rhs) = delete;
   Window& operator=(const Window& rhs) = delete;

public:
   Window();
   virtual ~Window();

   Window(Window&& rhs);
   Window& operator=(Window&& rhs);

   void Attach(HWND wnd);
   void Detach();
   HWND GetHandle() const;

   bool Create(const WindowClass& window_class, const TCHAR* caption, 
               DWORD style, int x, int y, int width, int heigh, HWND parent = nullptr);
   bool Create(const TCHAR* caption, 
               DWORD style, int x, int y, int width, int heigh, HWND parent = nullptr);

protected:
   virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

   LRESULT DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
   HWND m_wnd;
};

class MainWindow : public Window
{
public:
   MainWindow();
   virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};

} // namespace wc
