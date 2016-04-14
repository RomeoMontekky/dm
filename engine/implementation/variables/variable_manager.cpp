#include "variable_manager.h"

#include <cassert>

namespace dm
{

VariableManager::VariableManager() : 
   m_variables(), m_curr_iterator(m_variables.cend())
{
}

const Variable& VariableManager::AddVariable(TVariablePtr&& variable)
{
   auto ret = m_variables.insert(
      std::make_pair(variable->GetName(), std::move(variable)));
   assert(ret.second);
   return *((*ret.first).second);
}

void VariableManager::RemoveVariable(const StringPtrLen& name)
{
   auto ret = m_variables.erase(name);
   assert(1 == ret);
}

void VariableManager::RemoveAllVariables()
{
   m_variables.clear();
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

Variable* VariableManager::FindVariable(const StringPtrLen& name)
{
   return const_cast<Variable*>(((const VariableManager*)this)->FindVariable(name));
}

const Variable* VariableManager::GetFirstVariable() const
{
   if (m_variables.empty())
   {
      return nullptr;
   }

   m_curr_iterator = m_variables.cbegin();
   return (*m_curr_iterator).second.get();
}

const Variable* VariableManager::GetNextVariable() const
{
   if ((  m_curr_iterator == m_variables.cend()) ||
       (++m_curr_iterator == m_variables.cend()))
   {
      return nullptr;
   }

   return (*m_curr_iterator).second.get();
}

} // namespace dm
