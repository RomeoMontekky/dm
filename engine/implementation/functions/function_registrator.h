#pragma once

#include "function_manager.h"

namespace dm
{

template <typename FunctionClass>
class FunctionRegistrator
{
public:
   FunctionRegistrator()
   {
      FunctionManager::GetInstance().AddFunction(std::make_unique<FunctionClass>());
   }
};

}; // namespace dm

#define REGISTER_FUNCTION(FUNCTION_CLASS) \
   static const FunctionRegistrator<FUNCTION_CLASS> g_registrator;
