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
   const Variable* FindVariable(const StringPtrLen& name) const;
   void RemoveVariable(const StringPtrLen& name);
   void RemoveAllVariables();

private:
   std::map<std::string, TVariablePtr> m_variables;
};

}; // namespace dm
