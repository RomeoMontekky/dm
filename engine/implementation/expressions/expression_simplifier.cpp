#include "expression_simplifier.h"
#include "expression_visitor.h"
#include "expressions.h"

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
   virtual void Visit(ParamRefExpression& expression) override;
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

void ExpressionSimplifierVisitor::Visit(ParamRefExpression&)
{
   // Leave default values
}

void ExpressionSimplifierVisitor::Visit(OperationExpression& expression)
{
   const long child_count = expression.GetChildCount();

#ifdef _MSC_VER
   std::unique_ptr<LiteralType[]> children_values_ptr = std::make_unique<LiteralType[]>(child_count);
   LiteralType * const children_values = children_values_ptr.get();
#else
   LiteralType children_values[child_count];
#endif

#ifdef _MSC_VER
   std::unique_ptr<bool[]> children_is_raws_ptr = std::make_unique<bool[]>(child_count);
   bool * const children_is_raws = children_is_raws_ptr.get();
#else
   bool children_is_raws[child_count];
#endif

   bool is_all_actual_values = true;
   for (long index = 0; index < child_count; ++index)
   {
      ExpressionSimplifierVisitor child_visitor;
      expression.GetChild(index)->Accept(child_visitor);

      children_values[index] = child_visitor.GetValue();
      children_is_raws[index] = child_visitor.GetIsRaw();

      if (LiteralType::None == child_visitor.m_value)
      {
         is_all_actual_values = false;
      }
   }

   // If all children expressions have actual values, then we can
   // simplify the whole operation expression to a calculated value.
   if (is_all_actual_values)
   {
      m_value = PerformOperation(expression.GetOperation(), children_values, child_count);
   }
   // TODO: Partial simplification
   //else if ()
   //{
   //}
   else
   {
      // If not all child expressions have actual values, then we need to
      // simplify at least those who have and is not yet simplified.
      for (long index = 0; index < child_count; ++index)
      {
         if (children_values[index] != LiteralType::None && !children_is_raws[index])
         {
            expression.GetChild(index) = std::make_unique<LiteralExpression>(children_values[index]);
         }
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