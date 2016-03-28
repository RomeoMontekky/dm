#pragma once

#include "impexp.h"

#include <string>
#include <memory>

namespace dm
{

class ENGINE_API IStringable
{
public:
   IStringable();
   virtual ~IStringable();

   virtual std::string ToString() const = 0;
};

}; // namespace dm