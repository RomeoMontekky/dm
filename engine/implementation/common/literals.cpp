#include "literals.h"
#include "string_utils.h"

#include <cassert>

namespace dm
{

const char g_token_0[]     = "0";
const char g_token_1[]     = "1";
const char g_token_false[] = "false";
const char g_token_true[]  = "true";

bool IsLiteralToken(const StringPtrLen& str)
{
   return (str.Equals(g_token_false) || str.Equals(g_token_true));
}

LiteralType StringToLiteralType(const StringPtrLen& str)
{
   LiteralType literal = LiteralType::None;
   if (str.Equals(g_token_0) || str.Equals(g_token_false))
   {
      literal = LiteralType::False;
   }
   else if (str.Equals(g_token_1) || str.Equals(g_token_true))
   {
      literal = LiteralType::True;
   }
   return literal;
}

const char* LiteralTypeToString(LiteralType literal)
{
   assert(literal != LiteralType::None);
   return (literal == LiteralType::True ? g_token_1 : g_token_0);
}

} // namespace dm