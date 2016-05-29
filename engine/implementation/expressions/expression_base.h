#pragma once

#include <engine/istringable.h>

#include <vector>
#include <memory>

namespace dm
{

enum class ExpressionType
{
   Literal,
   ParamRef,
   Operation
};

class Expression;
class ExpressionVisitor;
class ConstExpressionVisitor;

using TExpressionPtr = std::unique_ptr<Expression>;
using TExpressionPtrVector = std::vector<TExpressionPtr>;

class Expression : public IStringable
{
public:
   Expression();
   Expression(const Expression& rhs);
   Expression& operator=(const Expression& rhs) = delete;

   // Returns expression type.
   virtual ExpressionType GetType() const = 0;
   // Clones expression tree as is.
   virtual TExpressionPtr Clone() const = 0;
   // Clones expression tree, substiting params with actual values
   virtual TExpressionPtr CloneWithSubstitution(const TExpressionPtrVector& actual_params) const = 0;
   // Visitors support
   virtual void Accept(ExpressionVisitor& visitor) = 0;
   virtual void Accept(ConstExpressionVisitor& visitor) const = 0;

   // Check whether the expression equals to another one.
   bool IsEqualTo(const Expression& rhs);

protected:
   // Check whether the expression equals to another one with the same type.
   virtual bool IsEqualToTheSameType(const Expression& rhs) const = 0;
};

template <ExpressionType type>
class TypedExpression : public Expression
{
public:
   virtual ExpressionType GetType() const override
   {
      return type;
   }
};

} // namespace dm
