#pragma once

#include "impexp.h"

namespace dm
{

class ENGINE_API IException
{
public:
   IException();
   virtual ~IException();

   virtual const char* GetDescription() const = 0;
};

} // namespace dm