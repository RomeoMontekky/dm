#pragma once

#include <engine/istringable.h>
#include "../common/named_entity.h"

#include <memory>

namespace dm
{

class VariableDeclaration : public NamedEntity, public IStringable
{
public:
   // Unnamed variable declaration
   VariableDeclaration(); 
   VariableDeclaration(const StringPtrLen& name);
   VariableDeclaration(const StringPtrLen& name, const VariableDeclaration& rhs);

   void AddParameter(const StringPtrLen& name);
   long FindParameter(const StringPtrLen& name) const;

   long GetParameterCount() const;
   const NamedEntity& GetParameter(long index) const;

   // IStringable
   virtual std::string ToString() const override;

protected:
   TNamedEntityVector m_parameters;
};

}; // namespace dm