#pragma once

#include <engine/istringable.h>

#include "../common/named_object.h"

#include <memory>

namespace dm
{

class VariableDeclaration : public NamedObject, public IStringable
{
public:
   // Unnamed variable declaration
   VariableDeclaration(); 

   VariableDeclaration(const StringPtrLen& name);

   void AddParameter(const StringPtrLen& name);
   long FindParameter(const StringPtrLen& name) const;
   long GetParameterCount() const;
   const NamedObject& GetParameter(long index) const;

private:
   TNamedObjectVector m_parameters;
};

}; // namespace dm