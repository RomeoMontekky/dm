#include "expression_evaluator.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

namespace
{

///////////// DetermierExpressionVisitor ////////////

class DetermierExpressionVisitor : public ConstExpressionVisitor
{
public:
   DetermierExpressionVisitor();
   
   LiteralType GetLiteral() const;
   long GetParamIndex() const;
   OperationType GetOperation() const;

   // ConstExpressionVisitor
   virtual void Visit(const LiteralExpression& expression) override;
   virtual void Visit(const ParamRefExpression& expression) override;
   virtual void Visit(const OperationExpression& expression) override;

   bool operator ==(const DetermierExpressionVisitor& rhs) const;

private:
   LiteralType m_literal;
   long m_param_index;
   OperationType m_operation;
   std::vector<DetermierExpressionVisitor> m_child_comparators;
};

DetermierExpressionVisitor::DetermierExpressionVisitor() :
    m_literal(LiteralType::None),
    m_param_index(-1),
    m_operation(OperationType::None),
    m_child_comparators()
{
}

LiteralType DetermierExpressionVisitor::GetLiteral() const
{
   return m_literal;
}

long DetermierExpressionVisitor::GetParamIndex() const
{
   return m_param_index;
}

OperationType DetermierExpressionVisitor::GetOperation() const
{
   return m_operation;
}

void DetermierExpressionVisitor::Visit(const LiteralExpression& expression)
{
   m_literal = expression.GetLiteral();
}

void DetermierExpressionVisitor::Visit(const ParamRefExpression& expression)
{
   m_param_index = expression.GetParamIndex();
}

void DetermierExpressionVisitor::Visit(const OperationExpression& expression)
{
   m_operation = expression.GetOperation();
   m_child_comparators.resize(expression.GetChildCount());

   const long child_count = expression.GetChildCount();
   for (long index = 0; index < child_count; ++index)
   {
      expression.GetChild(index)->Accept(m_child_comparators[index]);
   }
}

bool DetermierExpressionVisitor::operator ==(const DetermierExpressionVisitor& rhs) const
{
   return
   (
      (m_literal == rhs.m_literal) &&
      (m_param_index == rhs.m_param_index) &&
      (m_operation == rhs.m_operation) &&
      (m_child_comparators == rhs.m_child_comparators)
   );
}

void EvaluateExpression(TExpressionPtr& expression, DetermierExpressionVisitor& determier)
{
   
}

} // namespace

void EvaluateExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   
   DetermierExpressionVisitor determier;
   expression->Accept(determier);
   
   EvaluateExpression(expression, determier);
}

} // namespace dm
