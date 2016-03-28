#pragma once

#include "function_base.h"
#include "../common/noncopyable.h"

#include <memory>

namespace dm
{

class FunctionManager : public NonCopyable
{
public:
   static FunctionManager& GetInstance();

   void AddFunction(TFunctionPtr&& function);
   Function* FindFunction(const StringPtrLen& name) const;

private:
   FunctionManager();

private:
   std::map<std::string, TFunctionPtr> m_functions;
};

}; // namespace dm