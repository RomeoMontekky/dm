#include "variable.h"
#include "../expressions/expression_param_ref.h"

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
   const long param_count = GetParameterCount();
   
   TExpressionPtrVector replace_params;
   replace_params.reserve(param_count);
   for (long index = 0; i < param_count; ++i)
   {
      replace_params.push_back(
         std::unique_ptr<LiteralExpression>(GetParameter(index), index));
   }

   m_expression = rhs.m_expression->CloneWithSubstitution(replace_params);
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