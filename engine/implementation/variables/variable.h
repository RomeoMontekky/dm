#pragma once

#include "variable_declaration.h"
#include "../expressions/expression_base.h"

#include <memory>

namespace dm
{

class Variable : public VariableDeclaration
{
public:
   // Unnamed variable
   Variable(); 

   Variable(const StringPtrLen& name);

   void SetExpression(TExpressionPtr&& expression);
   const Expression* GetExpression() const;

   // IStringable
   virtual std::string ToString() const override;

private:
   TExpressionPtr m_expression;
};

using TVariablePtr = std::unique_ptr<Variable>;

}; // namespace dm