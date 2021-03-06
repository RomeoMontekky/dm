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
   VariableDeclaration(name, rhs), m_expression()
{
   const auto param_count = GetParameterCount();

   assert(param_count == rhs.GetParameterCount());

   TExpressionPtrVector replace_params;
   replace_params.reserve(param_count);
   for (auto index = 0L; index < param_count; ++index)
   {
      replace_params.push_back(
         std::make_unique<ParamRefExpression>(*this, index));
   }

   m_expression = rhs.GetExpression()->CloneWithSubstitution(replace_params);
}

void Variable::SetExpression(TExpressionPtr&& expression)
{
   assert(expression.get() != nullptr);
   m_expression = std::move(expression);
}

const TExpressionPtr& Variable::GetExpression() const
{
   return m_expression;
}

TExpressionPtr& Variable::GetExpression()
{
   return m_expression;
}

std::string Variable::ToString() const
{
   assert(m_expression.get() != nullptr);
   
   std::string ret;

   if (!GetName().empty())
   {
      ret += VariableDeclaration::ToString();
      ret += " := ";
   }
   ret += m_expression->ToString();

   return ret;
}

} // namespace dm
