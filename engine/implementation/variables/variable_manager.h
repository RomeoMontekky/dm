#pragma once

#include "variable.h"
#include "../common/noncopyable.h"

#include <map>

namespace dm
{

class VariableManager : public NonCopyable
{
public:
   VariableManager();

   const Variable& AddVariable(TVariablePtr&& variable);
   void RemoveVariable(const StringPtrLen& name);
   void RemoveAllVariables();

   const Variable* FindVariable(const StringPtrLen& name) const;
   Variable* FindVariable(const StringPtrLen& name);

   const Variable* GetFirstVariable() const;
   const Variable* GetNextVariable() const;

private:
   using TVariablePtrMap = std::map<std::string, TVariablePtr>;

   TVariablePtrMap m_variables;
   mutable TVariablePtrMap::const_iterator m_curr_iterator;
};

} // namespace dm
