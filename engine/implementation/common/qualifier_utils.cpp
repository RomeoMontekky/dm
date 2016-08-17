#include "qualifier_utils.h"
#include "function_utils.h"
#include "literals.h"
#include "exception.h"

#include <cctype>
#include <cassert>

namespace dm
{

namespace
{

bool IsQualifier(const StringPtrLen& str)
{
   assert(str.Len() > 0);
   
   auto curr = str.Begin();
   const auto end = str.End();
   
   // The first character can be alphabetic or '_'
   if (!std::isalpha(*curr) && *curr != '_')
   {
      return false;
   }

   // Other characters can be alphabetic, numeric or '_'
   for (++curr; curr != end; ++curr) 
   {
      if (!std::isalnum(*curr) && (*curr != '_'))
      {
         return false;
      }
   }

   return true;
}

bool IsReservedWord(const StringPtrLen& str)
{
   // Expand each time a new reserved word appears.
   return (IsLiteralToken(str) || IsCallToken(str));
}

} // namespace

void CheckQualifier(const StringPtrLen& str, const char* error_prefix)
{
   if (0 == str.Len())
   {
      Error(error_prefix, " can't be empty.");
   }

   if (!IsQualifier(str))
   {
      Error(error_prefix, " '", str, "' is not a qualifier.");
   }

   if (IsReservedWord(str))
   {
      Error(error_prefix, " '", str, "' can't be reserved word.");
   }
}

} // namespace dm