#include "expression_literal.h"

namespace dm
{

LiteralExpression::LiteralExpression(LiteralType literal) :
   Base(), m_literal(literal)
{
}

LiteralType LiteralExpression::GetLiteral() const
{
   return m_literal;
}

std::string LiteralExpression::ToString() const
{
   return LiteralTypeToString(m_literal);
}

TExpressionPtr LiteralExpression::Clone() const
{  
   return TExpressionPtr(new LiteralExpression(*this));
}

TExpressionPtr LiteralExpression::CloneWithSubstitution(const TExpressionPtrVector&) const
{
   return TExpressionPtr(new LiteralExpression(*this));
}

} // namespace dm
