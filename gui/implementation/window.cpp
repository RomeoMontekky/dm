#include "window.h"
#include "window_class.h"

#include <cassert>

namespace wc
{

static const TCHAR g_window_ptr_property[] = TEXT("WindowPtr");

Window::Window() : m_wnd(nullptr)
{
   // no code
}

Window::Window(Window&& rhs) : m_wnd(nullptr)
{
   m_wnd = rhs.m_wnd;
   rhs.m_wnd = nullptr;
}

Window::~Window()
{
   // no code
}

Window& Window::operator=(Window&& rhs)
{
   if (&rhs != this)
   {
      m_wnd = rhs.m_wnd;
      rhs.m_wnd = nullptr;
   }
   return *this;
}

void Window::Attach(HWND wnd)
{
   m_wnd = wnd;   
}

void Window::Detach()
{
   m_wnd = nullptr;
}

HWND Window::GetHandle() const
{
   return m_wnd;
}

bool Window::Create(
   const WindowClass& window_class, const TCHAR* caption, 
   DWORD style, int x, int y, int width, int heigh, HWND parent)
{
   assert(nullptr == m_wnd);

   m_wnd = ::CreateWindow(window_class.lpszClassName, caption, style,
                          x, y, width, heigh, parent, nullptr,
                          ::GetModuleHandle(nullptr), this);

   if (nullptr == m_wnd)
   {
      return false;
   }

   // Subclassing can be useful in future.
   // auto wnd_proc = ::GetWindowLongPtr(m_wnd, GWLP_WNDPROC);
   // if (wnd_proc != reinterpret_cast<LONG_PTR>(Window::StaticWindowProc))
   // {
   //    SubclassWindow(...);
   // }

   return true;
}

bool Window::Create(
   const TCHAR* caption, 
   DWORD style, int x, int y, int width, int heigh, HWND parent)
{
   static const WindowClass window_class("WindowClass");
   return Create(window_class, caption, style, x, y, width, heigh, parent);
}

LRESULT Window::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   return DefWindowProc(uMsg, wParam, lParam);
}

LRESULT Window::DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   auto base_window_proc = ::GetClassLongPtr(m_wnd, 0);
   return ::CallWindowProc(reinterpret_cast<WNDPROC>(base_window_proc),
                           m_wnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   Window* window = nullptr;

   if (WM_NCCREATE == uMsg)
   {
      auto create_data = reinterpret_cast<CREATESTRUCT*>(lParam);
      window = reinterpret_cast<Window*>(create_data->lpCreateParams);

      assert(window != nullptr);
      assert(window->GetHandle() == nullptr);
      window->Attach(hwnd);

      ::SetProp(hwnd, g_window_ptr_property, reinterpret_cast<HANDLE>(window));
   }
   else
   {
      window = reinterpret_cast<Window*>(::GetProp(hwnd, g_window_ptr_property));
   }

   // Some messages can be send before WM_NCCREATE.
   // It is enough to have just default handling for such a cases.
   if (nullptr == window)
   {
      return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
   }

   assert(window->GetHandle() == hwnd);

   return window->WindowProc(uMsg, wParam, lParam);
}

LRESULT MainWindow::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (WM_CLOSE == uMsg)
   {
      ::PostQuitMessage(0);
   }
   return Window::WindowProc(uMsg, wParam, lParam);
}

MainWindow::MainWindow() : Window()
{
   // no code
}

} // namespace wc
