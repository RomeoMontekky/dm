#include "expression_simplifier.h"
#include "expression_utils.h"
#include "expressions.h"

#include "../common/local_array.h"

#include <algorithm>
#include <cassert>

namespace dm
{

namespace
{

LiteralType SimplifyExpressionImpl(TExpressionPtr& expr)
{
   const auto type = expr->GetType();

   if (ExpressionType::Literal == type)
   {
      return CastToLiteral(expr).GetLiteral();
   }
   else if (ExpressionType::ParamRef == type)
   {
      return LiteralType::None;
   }

   // Case of (ExpressionType::Operation == type)

   auto& expression = CastToOperation(expr);
   const auto operation = expression.GetOperation();
   const auto are_operands_movable = AreOperandsMovable(operation);

   auto non_actual_values_count = 0L;
   auto first_actual_values_count = 0L;
   auto child_count = expression.GetChildCount();
   LOCAL_ARRAY(LiteralType, child_values, child_count);

   for (auto index = 0L; index < child_count; ++index)
   {
      child_values[index] = SimplifyExpressionImpl(expression.GetChild(index));

      if (LiteralType::None == child_values[index])
      {
         ++non_actual_values_count;
      }
      else if (0 == non_actual_values_count)
      {
         ++first_actual_values_count;
      }
   }

   auto value = LiteralType::None;

   const auto actual_values_count = child_count - non_actual_values_count;
   if (0 == actual_values_count)
   {
      // Nothing to simplify
      return value;
   }

   if (0 == non_actual_values_count)
   {
      // If all child expressions have actual values, then we can
      // simplify the whole operation expression to a calculated value.
      value = PerformOperation(operation, child_values, child_count);
   }
   else if (!are_operands_movable || first_actual_values_count == actual_values_count)
   {
      if (first_actual_values_count > 1)
      {
         // In this case we can't move operands, so we can simplify only first actual values.
         // Another case is when operands with actual values are all already at the biginning.
         expression.RemoveChildren(0, first_actual_values_count);

         const LiteralType part_value = PerformOperation(operation, child_values, first_actual_values_count);
         auto part_value_expression = std::make_unique<LiteralExpression>(part_value);

         if (are_operands_movable)
         {
            expression.AddChild(std::move(part_value_expression));
            return value;
         }
         else
         {
            expression.InsertChild(0, std::move(part_value_expression));
         }

         child_values[0] = part_value;
         std::copy(child_values + first_actual_values_count, child_values + child_count, child_values + 1);

         child_count -= first_actual_values_count - 1;
      }
      // first_actual_values_count == 1
      else if (are_operands_movable)
      {
         // Just move the single literal to the end (simpifying this, if necessary)
         TExpressionPtr actual_expression = (LiteralType::None != GetLiteral(expression.GetChild(0))) ?
            std::move(expression.GetChild(0)) : std::make_unique<LiteralExpression>(child_values[0]);
         
         expression.RemoveChild(0);
         expression.AddChild(std::move(actual_expression));

         return value;
      }

      // If we are here, then it is possible that there exist expressions
      // with actual values, but not raw. Let's simplify them.
      for (auto index = 0L; index < child_count; ++index)
      {
         auto& child = expression.GetChild(index);

         if (LiteralType::None != child_values[index] &&
             LiteralType::None == GetLiteral(child))
         {
            child = std::make_unique<LiteralExpression>(child_values[index]);
         }
      }
   }
   else if (are_operands_movable)
   {
      if (actual_values_count > 1)
      {
         // In this case we can perform operations with operands in any order, so we must do following:
         //    1. Collect all actual values in a single array;
         //    2. Calculate the result of operation over this array;
         //    3. Remove all expressions that correspond to actual values;
         //    4. Add one literal expression which will hold the calculated result.

         LOCAL_ARRAY(LiteralType, actual_values, actual_values_count);
         std::remove_copy(child_values, child_values + child_count, actual_values, LiteralType::None);

         for (auto index = child_count - 1; index >= 0; --index)
         {
            if (child_values[index] != LiteralType::None)
            {
               expression.RemoveChild(index);
            }
         }

         const auto value = PerformOperation(operation, actual_values, actual_values_count);
         expression.AddChild(std::make_unique<LiteralExpression>(value));
      }
      else // actual_values_count == 1
      {
         // In this case just move expression that corresponds to the actual value to the end.
         // If it is raw - this is enough to move it.
         // Otherwise we need to create new literal expression to simplify it and delete the old one.
         
         auto actual_index = 0L;
         for (; child_values[actual_index] == LiteralType::None; ++actual_index);
         assert(actual_index < child_count);
         
         auto& child = expression.GetChild(actual_index);
         if (LiteralType::None == GetLiteral(child))
         {
            // Move with simplification.
            expression.RemoveChild(actual_index);
            expression.AddChild(std::make_unique<LiteralExpression>(child_values[actual_index]));
         }
         else
         {
            // If actual_index == child_count - 1, then actual expression is already on its place.
            if (actual_index < child_count - 1)
            {
               auto actual_expression = std::move(child);
               expression.RemoveChild(actual_index);
               expression.AddChild(std::move(actual_expression));
            }
         }
      }
   }

   return value;
}

} // namespace

void SimplifyExpression(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);

   const auto value = SimplifyExpressionImpl(expr);

   // SimplifyExpressionImpl simplifies only child expressions of each operation
   // expression, so the root can be still not simplifed. Let's correct this if so.
   if (LiteralType::None != value && LiteralType::None == GetLiteral(expr))
   {
      expr = std::make_unique<LiteralExpression>(value);
   }
}

} // namespace dm
