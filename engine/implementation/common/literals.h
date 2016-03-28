#pragma once

#include "string_utils.h"

namespace dm
{

enum class LiteralType
{
   None = -1,
   False,
   True
};

bool IsLiteralToken(const StringPtrLen& str);

LiteralType StringToLiteralType(const StringPtrLen& str);
const char* LiteralTypeToString(LiteralType literal);

}; // namespace dm