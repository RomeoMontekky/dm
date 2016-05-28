#include "expression_calculator.h"
#include "expressions.h"

#include "../common/local_array.h"

#include <cassert>

namespace dm
{

LiteralType CalculateExpression(const TExpressionPtr& expr, const LiteralType param_values[])
{
   assert(expr.get() != nullptr);

   LiteralType value = LiteralType::None;
   switch (expr->GetType())
   {
      case ExpressionType::Literal:
      {
         value = static_cast<const LiteralExpression*>(expr.get())->GetLiteral();
         break;
      }

      case ExpressionType::ParamRef:
      {
         value = param_values[static_cast<const ParamRefExpression*>(expr.get())->GetParamIndex()];
         break;
      }

      case ExpressionType::Operation:
      {
         auto expression = static_cast<const OperationExpression*>(expr.get());
         const long child_count = expression->GetChildCount();

         LOCAL_ARRAY(LiteralType, child_values, child_count);
         for (long index = child_count - 1; index >= 0; --index)
         {
            child_values[index] = CalculateExpression(expression->GetChild(index), param_values);
         }

         value = PerformOperation(expression->GetOperation(), child_values, child_count);

         break;
      }

      default:
      {
         assert(!"Unknown type of expression");
      }
   }

   return value;
}

} // namespace dm
