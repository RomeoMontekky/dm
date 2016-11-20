#pragma once

#include <windows.h>

namespace wc
{

class WindowClass : public WNDCLASSEX
{
   WindowClass(const WindowClass& rhs) = delete;

public:
   WindowClass(const TCHAR* class_name, UINT style = 0, const TCHAR* base_class_name = nullptr);
};

} // namespace wc
