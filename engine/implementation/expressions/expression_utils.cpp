#include "expression_utils.h"
#include "expression_visitor.h"
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

void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Operation);

   OperationExpression* expression = static_cast<OperationExpression*>(expr.get());

   target.clear();

   const long child_count = expression->GetChildCount();
   target.reserve(child_count);
   for (long index = 0; index < child_count; ++index)
   {
      target.push_back(std::move(expression->GetChild(index)));
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

void RemoveChildExpression(TExpressionPtr& expr, long child_index)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Operation);

   OperationExpression* expression = static_cast<OperationExpression*>(expr.get());

   if (child_index < 0)
   {
      child_index = expression->GetChildCount() + child_index;
   }

   expression->RemoveChild(child_index);
}

void AddChildExpression(TExpressionPtr& expr, TExpressionPtr&& child)
{
   assert(expr.get() != nullptr);
   assert(expr->GetType() == ExpressionType::Operation);
   static_cast<OperationExpression*>(expr.get())->AddChild(std::move(child));
}

} // namespace dm
