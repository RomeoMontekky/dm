#include "window_class.h"
#include "window.h"

#include <cassert>

namespace wc
{

WindowClass::WindowClass(const TCHAR* class_name, UINT style, const TCHAR* base_class_name)
{
   WNDCLASSEX::cbSize = sizeof(WNDCLASSEX);
   WNDCLASSEX::style = 0;
   WNDCLASSEX::cbClsExtra = 0;
   WNDCLASSEX::cbWndExtra = 0;
   WNDCLASSEX::hInstance = nullptr;
   WNDCLASSEX::hIcon = nullptr;
   WNDCLASSEX::hCursor = nullptr;
   WNDCLASSEX::hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
   WNDCLASSEX::lpszMenuName = nullptr;
   WNDCLASSEX::hIconSm = nullptr;

   // If base_class_name is specified, load all fields from this class.
   WNDPROC oldProc = ::DefWindowProc;
   if (base_class_name && ::GetClassInfoEx(nullptr, base_class_name, this))
   {
      oldProc = WNDCLASSEX::lpfnWndProc;
   }

   // Register class with DefWindowProc to avoid calling of Window::WindowProc during
   // temporary window creation (see below). Required Window::WindowProc will be set
   // later, using descriptor of the temporary window.
   WNDCLASSEX::lpfnWndProc = ::DefWindowProc;
   WNDCLASSEX::style |= style;
   WNDCLASSEX::lpszClassName = class_name;
   WNDCLASSEX::hInstance = ::GetModuleHandle(nullptr);
   // Four bytes in extra-data to hold old pointer to window procedure.
   WNDCLASSEX::cbClsExtra = sizeof(LONG);
   WNDCLASSEX::hCursor = ::LoadCursor( nullptr, IDC_ARROW );
   WNDCLASSEX::hbrBackground = (HBRUSH)::GetStockObject( WHITE_BRUSH );

   if (::RegisterClassEx(this) == 0)
   {
      assert(!"Error during registring window class.");
      return;
   }

   // Win API doesn't allow to set class's extra-data using ATOM, returned from
   // RegisterClassEx, so we are forced to create temprory invisible window and set
   // extra-data through it. The window is destroyed as soon as data is set.
   // Also we set here Window::WindowProc, which could not be set at the beginning
   // (see explanation above).
   auto temp_window = ::CreateWindowEx(
      0, class_name, "", 0, 0, 0, 0, 0, nullptr, nullptr, WNDCLASSEX::hInstance, nullptr);
   {
      ::SetClassLongPtr(temp_window, 0, reinterpret_cast<LONG>(oldProc));
      ::SetClassLongPtr(temp_window, GCLP_WNDPROC, reinterpret_cast<LONG>(Window::StaticWindowProc));
   }
   ::DestroyWindow(temp_window);
}

} // namespace wc
