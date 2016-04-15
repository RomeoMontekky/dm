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
   
   long GetChildCount() const;
   DetermierExpressionVisitor& GetChild(long index);

   // ConstExpressionVisitor
   virtual void Visit(const LiteralExpression& expression) override;
   virtual void Visit(const ParamRefExpression& expression) override;
   virtual void Visit(const OperationExpression& expression) override;

   bool operator ==(const DetermierExpressionVisitor& rhs) const;

private:
   LiteralType m_literal;
   long m_param_index;
   OperationType m_operation;
   std::vector<DetermierExpressionVisitor> m_children;
};

DetermierExpressionVisitor::DetermierExpressionVisitor() :
    m_literal(LiteralType::None),
    m_param_index(-1),
    m_operation(OperationType::None),
    m_children()
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

long DetermierExpressionVisitor::GetChildCount() const
{
   return m_children.size();
}

DetermierExpressionVisitor& DetermierExpressionVisitor::GetChild(long index)
{
   assert(index >= 0 && index < (long)m_children.size());
   return m_children[index];
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
   m_children.resize(expression.GetChildCount());

   const long child_count = expression.GetChildCount();
   for (long index = 0; index < child_count; ++index)
   {
      expression.GetChild(index)->Accept(m_children[index]);
   }
}

bool DetermierExpressionVisitor::operator ==(const DetermierExpressionVisitor& rhs) const
{
   return
   (
      (m_literal == rhs.m_literal) &&
      (m_param_index == rhs.m_param_index) &&
      (m_operation == rhs.m_operation) &&
      (m_children == rhs.m_children)
   );
}

} // namespace

void EvaluateExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   DetermierExpressionVisitor determier;
   expression->Accept(determier);
}

} // namespace dm
