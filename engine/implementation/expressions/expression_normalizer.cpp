#include "expression_normalizer.h"
#include "expressions.h"
#include "expression_utils.h"

#include <cassert>

namespace dm
{

void NormalizeExpression(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);

   if (expr->GetType() != ExpressionType::Operation)
   {
      return;
   }

   auto& expression = CastToOperation(expr);
   const auto operation = expression.GetOperation();

   if (OperationType::Negation == operation)
   {
      // Do not normalize negation.
      // Removing of double negation is implied in operation evaluation.
      return;
   }

   const auto are_operands_movable = AreOperandsMovable(operation);

   for (long index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      TExpressionPtr& child = expression.GetChild(index);

      // Recursive call must be done before actual normalization.
      NormalizeExpression(child);

      // The child expression can be normalized if following is true:
      //    - it is either first child, or current operation is associative;
      //    - it is operation expression and operation is the same as in the current expression;
      if ((are_operands_movable || 0 == index) && (GetOperation(child) == operation))
      {
         MoveChildExpressionsUp(expression, index);
      }
   }
}

} // namespace dm
