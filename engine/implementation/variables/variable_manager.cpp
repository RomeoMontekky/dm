#include "variable_manager.h"

#include <cassert>

namespace dm
{

VariableManager::VariableManager() : m_variables()
{
}

const Variable& VariableManager::AddVariable(TVariablePtr&& variable)
{
   auto ret = m_variables.insert(
      std::make_pair(variable->GetName(), std::move(variable)));
   assert(ret.second);
   return *((*ret.first).second);
}

const Variable* VariableManager::FindVariable(const StringPtrLen& name) const
{
   auto it = m_variables.find(name);
   if (it != m_variables.end())
   {
      return (*it).second.get();
   }
   return nullptr;
}

void VariableManager::RemoveVariable(const StringPtrLen& name)
{
   auto ret = m_variables.erase(name);
   assert(1 == ret);
}

}; // namespace dm