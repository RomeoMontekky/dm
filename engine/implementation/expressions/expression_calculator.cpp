#include "expression_calculator.h"
#include "expression_visitor.h"
#include "expressions.h"

#include "../common/local_array.h"

#include <cassert>

namespace dm
{

namespace
{

class ExpressionCalculatorVisitor : public ConstExpressionVisitor
{
public:
   ExpressionCalculatorVisitor(const LiteralType param_values[]);

   LiteralType GetValue() const;

   // ExpressionVisitor
   virtual void Visit(const LiteralExpression& expression) override;
   virtual void Visit(const ParamRefExpression& expression) override;
   virtual void Visit(const OperationExpression& expression) override;

private:
   const LiteralType* const m_param_values;
   LiteralType m_value;
};

ExpressionCalculatorVisitor::ExpressionCalculatorVisitor(const LiteralType param_values[]) :
   m_param_values(param_values), m_value(LiteralType::None)
{
}

LiteralType ExpressionCalculatorVisitor::GetValue() const
{
   return m_value;
}

void ExpressionCalculatorVisitor::Visit(const LiteralExpression& expression)
{
   m_value = expression.GetLiteral();
}

void ExpressionCalculatorVisitor::Visit(const ParamRefExpression& expression)
{
   m_value = m_param_values[expression.GetParamIndex()];
}

void ExpressionCalculatorVisitor::Visit(const OperationExpression& expression)
{
   const long child_count = expression.GetChildCount();

   LOCAL_ARRAY(LiteralType, child_values, child_count);
   for (long index = 0; index < child_count; ++index)
   {
      ExpressionCalculatorVisitor child_visitor(m_param_values);
      expression.GetChild(index)->Accept(child_visitor);
      child_values[index] = child_visitor.GetValue();
   }

   m_value = PerformOperation(expression.GetOperation(), child_values, child_count);
}

}; // namespace

LiteralType CalculateExpression(const Expression* expression, const LiteralType param_values[])
{
   ExpressionCalculatorVisitor visitor(param_values);
   expression->Accept(visitor);
   return visitor.GetValue();
}

}; // namespace dm