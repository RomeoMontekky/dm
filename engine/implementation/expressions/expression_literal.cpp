#include "expression_literal.h"
#include "expression_visitor.h"

namespace dm
{

LiteralExpression::LiteralExpression(LiteralType literal) :
   Expression(), m_literal(literal)
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

void LiteralExpression::Accept(ExpressionVisitor& visitor)
{
   visitor.Visit(*this);
}

void LiteralExpression::Accept(ConstExpressionVisitor& visitor) const
{
   visitor.Visit(*this);
}

}; // namespace dm