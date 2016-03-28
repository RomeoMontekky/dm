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
   std::string ret;

   if (!GetName().empty())
   {
      ret += GetName();
      ret += " := ";
   }
   
   assert(m_expression.get() != nullptr);
   ret += m_expression->ToString();

   // TODO: Add parameters

   return ret;
}

}; // namespace dm