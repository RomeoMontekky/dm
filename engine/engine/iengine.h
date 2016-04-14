#pragma once

#include "istringable.h"
#include "impexp.h"

#include <memory>

namespace dm
{

class ENGINE_API IEngine
{
public:
   IEngine();
   virtual ~IEngine();

   virtual const IStringable& Process(const char* str, long len = -1) = 0;
};

using TIEnginePtr = std::unique_ptr<IEngine>;

ENGINE_API TIEnginePtr CreateEngine();

} // namespace dm

