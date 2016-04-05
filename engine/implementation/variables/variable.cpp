#include "variable.h"

#include <cassert>

namespace dm
{

Variable::Variable() :
   VariableDeclaration(), m_expression()
{
}

Variable::Variable(const StringPtrLen& name) :
   VariableDeclaration(name), m_expression()
{
}

Variable::Variable(const StringPtrLen& name, const Variable& rhs) :
   VariableDeclaration(rhs), m_expression()
{
   // TODO: Copy expression, changing param-refs to new variable
}

void Variable::SetExpression(TExpressionPtr&& expression)
{
   assert(expression.get() != nullptr);
   m_expression = std::move(expression);
}

const Expression* Variable::GetExpression() const
{
   return m_expression.get();
}

std::string Variable::ToString() const
{
   assert(m_expression.get() != nullptr);
   
   std::string ret;

   ret += VariableDeclaration::ToString();
   ret += " := ";
   ret += m_expression->ToString();

   return ret;
}

}; // namespace dm