#pragma once

#include <engine/istringable.h>

#include <vector>
#include <memory>

namespace dm
{

class Expression;
class ExpressionVisitor;
class ConstExpressionVisitor;

using TExpressionPtr = std::unique_ptr<Expression>;
using TExpressionPtrVector = std::vector<TExpressionPtr>;

class Expression : public IStringable
{
public:
   Expression();
   Expression(const Expression& rhs) = default;
   Expression& operator=(const Expression& rhs) = delete;

   // Clones expression tree as is.
   virtual TExpressionPtr Clone() const = 0;
   // Clones expression tree, substiting params with actual values
   virtual TExpressionPtr CloneWithSubstitution(const TExpressionPtrVector& actual_params) const = 0;
   // Visitors support
   virtual void Accept(ExpressionVisitor& visitor) = 0;
   virtual void Accept(ConstExpressionVisitor& visitor) const = 0;
};

}; // namespace dm