#include "expression_calculator.h"
#include "expression_visitor.h"
#include "expressions.h"

#include "../common/local_array.h"

#include <cassert>

namespace dm
{

namespace
{

class ExpressionCalculator : public ConstExpressionVisitor
{
public:
   ExpressionCalculator(const LiteralType param_values[]);

   LiteralType GetValue() const;

   // ExpressionVisitor
   virtual void Visit(const LiteralExpression& expression) override;
   virtual void Visit(const ParamRefExpression& expression) override;
   virtual void Visit(const OperationExpression& expression) override;

private:
   const LiteralType* const m_param_values;
   LiteralType m_value;
};

ExpressionCalculator::ExpressionCalculator(const LiteralType param_values[]) :
   m_param_values(param_values), m_value(LiteralType::None)
{
}

LiteralType ExpressionCalculator::GetValue() const
{
   return m_value;
}

void ExpressionCalculator::Visit(const LiteralExpression& expression)
{
   m_value = expression.GetLiteral();
}

void ExpressionCalculator::Visit(const ParamRefExpression& expression)
{
   m_value = m_param_values[expression.GetParamIndex()];
}

void ExpressionCalculator::Visit(const OperationExpression& expression)
{
   const long child_count = expression.GetChildCount();

   LOCAL_ARRAY(LiteralType, child_values, child_count);
   for (long index = 0; index < child_count; ++index)
   {
      ExpressionCalculator child_visitor(m_param_values);
      expression.GetChild(index)->Accept(child_visitor);
      child_values[index] = child_visitor.GetValue();
   }

   m_value = PerformOperation(expression.GetOperation(), child_values, child_count);
}

} // namespace

LiteralType CalculateExpression(const Expression* expression, const LiteralType param_values[])
{
   ExpressionCalculator calculator(param_values);
   expression->Accept(calculator);
   return calculator.GetValue();
}

} // namespace dm
