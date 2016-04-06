#include "expression_param_ref.h"
#include "expression_visitor.h"
#include "../variables/variable_declaration.h"

namespace dm
{

ParamRefExpression::ParamRefExpression(const VariableDeclaration& variable, long index) :
    Expression(), m_variable(variable), m_index(index)
{
}

const NamedEntity& ParamRefExpression::GetParamRef() const
{
   return m_variable.GetParameter(m_index);
}

long ParamRefExpression::GetParamIndex() const
{
   return m_index;
}

std::string ParamRefExpression::ToString() const
{
   return m_variable.GetParameter(m_index).GetName();
}

TExpressionPtr ParamRefExpression::Clone() const
{
   return TExpressionPtr(new ParamRefExpression(*this));
}

TExpressionPtr ParamRefExpression::CloneWithSubstitution(
   const TExpressionPtrVector& actual_params) const
{
   // Copy substituted subtree without subtitution
   return actual_params.at(m_index)->Clone();
}

void ParamRefExpression::Accept(ExpressionVisitor& visitor)
{
   visitor.Visit(*this);
}

void ParamRefExpression::Accept(ConstExpressionVisitor& visitor) const
{
   visitor.Visit(*this);
}

}; // namespace dm