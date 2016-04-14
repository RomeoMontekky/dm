#pragma once

#include "string_utils.h"

namespace dm
{

bool IsCallToken(const StringPtrLen& str);
bool IsFunctionCall(const StringPtrLen& str);
void TrimFunctionCall(StringPtrLen& str);

} // namespace dm