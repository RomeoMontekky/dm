#include "expression_utils.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

const LiteralExpression& CastToLiteral(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Literal);
   return static_cast<const LiteralExpression&>(*expr.get());
}

LiteralExpression& CastToLiteral(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Literal);
   return static_cast<LiteralExpression&>(*expr.get());
}

const ParamRefExpression& CastToParamRef(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::ParamRef);
   return static_cast<const ParamRefExpression&>(*expr.get());
}

ParamRefExpression& CastToParamRef(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::ParamRef);
   return static_cast<ParamRefExpression&>(*expr.get());
}

const OperationExpression& CastToOperation(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Operation);
   return static_cast<const OperationExpression&>(*expr.get());
}

OperationExpression& CastToOperation(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Operation);
   return static_cast<OperationExpression&>(*expr.get());
}

LiteralType GetLiteral(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   return (expr->GetType() == ExpressionType::Literal) ?
      CastToLiteral(expr).GetLiteral() : LiteralType::None;
}

long GetParamIndex(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   return (expr->GetType() == ExpressionType::ParamRef) ?
      CastToParamRef(expr).GetParamIndex() : -1;
}

OperationType GetOperation(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   return (expr->GetType() == ExpressionType::Operation) ?
      CastToOperation(expr).GetOperation() : OperationType::None;
}

void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expr)
{
   MoveChildExpressions(target, CastToOperation(expr));
}

void MoveChildExpressions(TExpressionPtrVector& target, OperationExpression& expression)
{
   target.clear();

   const auto child_count = expression.GetChildCount();
   target.reserve(child_count);
   for (auto index = 0L; index < child_count; ++index)
   {
      target.push_back(std::move(expression.GetChild(index)));
   }
}

void MoveChildExpressionsUp(OperationExpression& expression, long child_index)
{
   TExpressionPtrVector moved_children;
   MoveChildExpressions(moved_children, expression.GetChild(child_index));
   expression.RemoveChild(child_index);
   expression.InsertChildren(child_index, std::move(moved_children));
}

} // namespace dm
