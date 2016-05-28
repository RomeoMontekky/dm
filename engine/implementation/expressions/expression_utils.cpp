#include "expression_utils.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

LiteralType GetLiteral(const TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);

   if (expression->GetType() == ExpressionType::Literal)
   {
      const LiteralExpression* literal_expression =
         static_cast<const LiteralExpression*>(expression.get());
      return literal_expression->GetLiteral();
   }

   return LiteralType::None;
}

OperationType GetOperation(const TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);

   if (expression->GetType() == ExpressionType::Operation)
   {
      const OperationExpression* operation_expression =
         static_cast<const OperationExpression*>(expression.get());
      return operation_expression->GetOperation();
   }

   return OperationType::None;
}

void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   assert(expression->GetType() == ExpressionType::Operation);

   OperationExpression* operation_expression =
      static_cast<OperationExpression*>(expression.get());

   target.clear();

   const long child_count = operation_expression->GetChildCount();
   target.reserve(child_count);
   for (long index = 0; index < child_count; ++index)
   {
      target.push_back(std::move(operation_expression->GetChild(index)));
   }
}

void MoveChildExpression(TExpressionPtr& target, TExpressionPtr& expression, long child_index)
{
   TExpressionPtrVector moved_expressions;
   MoveChildExpressions(moved_expressions, expression);
   assert(child_index >= 0 && child_index < moved_expressions.size());
   target = std::move(moved_expressions[child_index]);
}

void MoveChildExpressionInplace(TExpressionPtr& expression, long child_index)
{
   MoveChildExpression(expression, expression, child_index);
}

void RemoveChildExpression(TExpressionPtr& expression, long child_index)
{
   assert(expression.get() != nullptr);
   assert(expression->GetType() == ExpressionType::Operation);

   OperationExpression* operation_expression =
      static_cast<OperationExpression*>(expression.get());

   if (child_index < 0)
   {
      child_index = operation_expression->GetChildCount() + child_index;
   }

   operation_expression->RemoveChild(child_index);
}

} // namespace dm
