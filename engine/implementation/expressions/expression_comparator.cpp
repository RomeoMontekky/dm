#include "expression_comparator.h"
#include "expression_visitor.h"
#include "expressions.h"

#include "../common/local_array.h"

namespace dm
{

namespace
{

class ComparatorExpressionVisitor : public ConstExpressionVisitor
{
public:
   ComparatorExpressionVisitor();

   // ConstExpressionVisitor
   virtual void Visit(const LiteralExpression& expression) override;
   virtual void Visit(const ParamRefExpression& expression) override;
   virtual void Visit(const OperationExpression& expression) override;

   bool operator ==(const ComparatorExpressionVisitor& rhs) const;

private:
   LiteralType m_literal;
   long m_param_index;
   OperationType m_operation;
   std::vector<ComparatorExpressionVisitor> m_child_comparators;
};

ComparatorExpressionVisitor::ComparatorExpressionVisitor() :
    m_literal(LiteralType::None),
    m_param_index(-1),
    m_operation(OperationType::None),
    m_child_comparators()
{
}

void ComparatorExpressionVisitor::Visit(const LiteralExpression& expression)
{
   m_literal = expression.GetLiteral();
}

void ComparatorExpressionVisitor::Visit(const ParamRefExpression& expression)
{
   m_param_index = expression.GetParamIndex();
}

void ComparatorExpressionVisitor::Visit(const OperationExpression& expression)
{
   m_operation = expression.GetOperation();
   m_child_comparators.resize(expression.GetChildCount());

   const long child_count = expression.GetChildCount();
   for (long index = 0; index < child_count; ++index)
   {
      expression.GetChild(index)->Accept(m_child_comparators[index]);
   }
}

bool ComparatorExpressionVisitor::operator ==(const ComparatorExpressionVisitor& rhs) const
{
   return
   (
      m_literal == rhs.m_literal &&
      m_param_index == rhs.m_param_index &&
      m_operation == rhs.m_operation &&
      m_child_comparators == rhs.m_child_comparators
   );
}

} // namespace

void CalculateComparisonMatrix(bool comp_matrix[], const TExpressionPtrVector& expressions)
{
   const long expression_count = expressions.size();
   LOCAL_ARRAY(ComparatorExpressionVisitor, comparators, expression_count);

   for (long index = 0; index < expression_count; ++index)
   {
      expressions[index]->Accept(comparators[index]);
   }

   for (long i = 0; i < expression_count; ++i)
      for (long j = 0; j < expression_count; ++j)
         comp_matrix[i*expression_count + j] = (comparators[i] == comparators[j]);
}

} // namespace dm
