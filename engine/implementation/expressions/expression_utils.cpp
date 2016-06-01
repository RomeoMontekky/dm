#include "expression_utils.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

LiteralType GetLiteral(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   if (expr->GetType() == ExpressionType::Literal)
   {
      return static_cast<const LiteralExpression*>(expr.get())->GetLiteral();
   }
   return LiteralType::None;
}

OperationType GetOperation(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   if (expr->GetType() == ExpressionType::Operation)
   {
      return static_cast<const OperationExpression*>(expr.get())->GetOperation();
   }
   return OperationType::None;
}

OperationExpression& CastToOperation(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Operation);
   return static_cast<OperationExpression&>(*expr.get());
}

const OperationExpression& CastToOperation(const TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Operation);
   return static_cast<const OperationExpression&>(*expr.get());
}

void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expr)
{
   auto& expression = CastToOperation(expr);

   target.clear();

   const long child_count = expression.GetChildCount();
   target.reserve(child_count);
   for (long index = 0; index < child_count; ++index)
   {
      target.push_back(std::move(expression.GetChild(index)));
   }
}

void MoveChildExpression(TExpressionPtr& target, TExpressionPtr& expr, long child_index)
{
   TExpressionPtrVector moved_expressions;
   MoveChildExpressions(moved_expressions, expr);
   assert(child_index >= 0 && child_index < moved_expressions.size());
   target = std::move(moved_expressions[child_index]);
}

void MoveChildExpressionInplace(TExpressionPtr& expr, long child_index)
{
   MoveChildExpression(expr, expr, child_index);
}

} // namespace dm
