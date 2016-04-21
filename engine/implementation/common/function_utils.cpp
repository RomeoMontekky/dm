#include "function_utils.h"

#include <cstring>
#include <cctype>
#include <cassert>

namespace dm
{

namespace
{

const char g_token_call[] = "call";

} // namespace

bool IsCallToken(const StringPtrLen& str)
{
   return str.Equals(g_token_call);
}

bool IsFunctionCall(const StringPtrLen& str)
{
   StringPtrLen str_copy = str;
   str_copy.TrimLeft();

   if (!str_copy.StartsWith(g_token_call))
   {
      return false;
   }

   str_copy.RemoveLeft(std::strlen(g_token_call));

   return (str_copy.Len() == 0 || std::isspace(str_copy.At(0)));
}

void TrimFunctionCall(StringPtrLen& str)
{
   assert(IsFunctionCall(str));

   str.TrimLeft();
   str.RemoveLeft(std::strlen(g_token_call));
   str.TrimLeft();
}

} // namespace dm
