#pragma once

#include "expression_base.h"
#include "../common/literals.h"

namespace dm
{

class LiteralExpression : public Expression
{
public:
   LiteralExpression(LiteralType literal);

   LiteralType GetLiteral() const;

   // IStringable
   virtual std::string ToString() const override;

   // Expression
   virtual TExpressionPtr Clone() const override;
   virtual TExpressionPtr CloneWithSubstitution(const TExpressionPtrVector& actual_params) const override;
   virtual void Accept(ExpressionVisitor& visitor) override;
   virtual void Accept(ConstExpressionVisitor& visitor) const override;

private:
   LiteralExpression(const LiteralExpression& rhs) = default;
   LiteralExpression& operator=(const LiteralExpression& rhs) = delete;

private:
   LiteralType m_literal;
};

} // namespace dm