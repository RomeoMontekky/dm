#include "expression_simplifier.h"
#include "expression_visitor.h"
#include "expressions.h"

#include "../common/local_array.h"

#include <algorithm>
#include <cassert>

namespace dm
{

namespace
{

class ExpressionSimplifierVisitor : public ExpressionVisitor
{
public:
   ExpressionSimplifierVisitor();

   LiteralType GetValue() const;
   bool GetIsRaw() const;

   // ExpressionVisitor
   virtual void Visit(LiteralExpression& expression) override;
   virtual void Visit(OperationExpression& expression) override;

private:
   // Literal value of the subtree (raw or calculated).
   // If calculation isn't possible, then it has LiteralType::None value.
   LiteralType m_value;
   // Whether literal value is raw or it was calculated.
   bool m_is_raw;   
};

ExpressionSimplifierVisitor::ExpressionSimplifierVisitor() :
   m_value(LiteralType::None), m_is_raw(false)
{
}

LiteralType ExpressionSimplifierVisitor::GetValue() const
{
   return m_value;
}

bool ExpressionSimplifierVisitor::GetIsRaw() const
{
   return m_is_raw;
}

void ExpressionSimplifierVisitor::Visit(LiteralExpression& expression)
{
   m_value = expression.GetLiteral();
   m_is_raw = true;
}

void ExpressionSimplifierVisitor::Visit(OperationExpression& expression)
{
   long child_count = expression.GetChildCount();

   LOCAL_ARRAY(LiteralType, child_values, child_count);
   LOCAL_ARRAY(bool, child_is_raws, child_count);

   long non_actual_values_count = 0;
   long first_actual_values_count = 0;

   for (long index = 0; index < child_count; ++index)
   {
      ExpressionSimplifierVisitor child_visitor;
      expression.GetChild(index)->Accept(child_visitor);

      child_values[index] = child_visitor.GetValue();
      child_is_raws[index] = child_visitor.GetIsRaw();

      if (LiteralType::None == child_values[index])
      {
         ++non_actual_values_count;
      }
      else if (0 == non_actual_values_count)
      {
         ++first_actual_values_count;
      }
   }

   const long actual_values_count = child_count - non_actual_values_count;

   const bool is_children_movable = 
      IsOperationCommutative(expression.GetOperation()) && 
      IsOperationAssociative(expression.GetOperation());

   if (0 == non_actual_values_count)
   {
      // If all child expressions have actual values, then we can
      // simplify the whole operation expression to a calculated value.

      m_value = PerformOperation(expression.GetOperation(), child_values, child_count);

      return;
   }
   else if ((!is_children_movable || first_actual_values_count == actual_values_count) && 
             first_actual_values_count > 1)
   {
      // In this case we cann't move operands, so we can simplify only first actual values.
      // Another case is when operands with actual values are all already at the biginning.

      for (long counter = first_actual_values_count; counter > 0; --counter)
      {
         expression.RemoveChild(0);
      }

      const LiteralType value = PerformOperation(expression.GetOperation(), child_values, first_actual_values_count);
      auto value_expression = std::make_unique<LiteralExpression>(value);

      if (is_children_movable)
      {
         expression.AddChild(std::move(value_expression));
         return;
      }
      else
      {
         expression.InsertChild(0, std::move(value_expression));
      }

      child_values[0] = value;
      child_is_raws[0] = true;

      std::copy(child_values + first_actual_values_count, child_values + child_count, child_values + 1);
      std::copy(child_is_raws + first_actual_values_count, child_is_raws + child_count, child_is_raws + 1);

      child_count -= first_actual_values_count - 1;
   }
   else if (is_children_movable && actual_values_count > 1)
   {
      // In this case we can perform operations with operands in any order, so we must do following:
      //    1. Collect all actual values in a single array;
      //    2. Calculate the result of operation over this array;
      //    3. Remove all expressions that correspond to actual values;
      //    4. Add one literal expression which will hold the calculated result.

      LOCAL_ARRAY(LiteralType, actual_values, actual_values_count);
      std::remove_copy(child_values, child_values + child_count, actual_values, LiteralType::None);

      long index = 0;
      while (index < child_count)
      {
         if (child_values[index] != LiteralType::None)
         {
            expression.RemoveChild(index);
            std::copy(child_values + index + 1, child_values + child_count, child_values + index);
            --child_count;
         }
         else
         {
            ++index;
         }
      }

      const LiteralType value = PerformOperation(expression.GetOperation(), actual_values, actual_values_count);
      expression.AddChild(std::make_unique<LiteralExpression>(value));

      return;
   }

   // If we are here, then it is possible that there exist expressions
   // with actual values, but not raw. Let's simplify them.

   for (long index = 0; index < child_count; ++index)
   {
      if (child_values[index] != LiteralType::None && !child_is_raws[index])
      {
         expression.GetChild(index) = std::make_unique<LiteralExpression>(child_values[index]);
      }
   }

   // Leave default values of m_value and m_is_raw
}

}; // namespace

void SimplifyExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);

   ExpressionSimplifierVisitor visitor;
   expression->Accept(visitor);

   // The visitor simplifies only child expressions of each operation expression, 
   // so the root can be still not simplifed. Let's correct this if so.
   if (LiteralType::None != visitor.GetValue() && !visitor.GetIsRaw())
   {
      expression = std::make_unique<LiteralExpression>(visitor.GetValue());
   }
}

}; // namespace dm