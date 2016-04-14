#include "function_manager.h"

#include <cassert>

namespace dm
{

FunctionManager& FunctionManager::GetInstance()
{
   static FunctionManager manager;
   return manager;
}

FunctionManager::FunctionManager()
{
}

void FunctionManager::AddFunction(TFunctionPtr&& function)
{
   auto ret = m_functions.insert(
      std::make_pair(function->GetName(), std::move(function)));
   assert(ret.second);
}

Function* FunctionManager::FindFunction(const StringPtrLen& name) const
{
   auto it = m_functions.find(name);
   if (it != m_functions.end())
   {
      return (*it).second.get();
   }
   return nullptr;
}

} // namespace dm