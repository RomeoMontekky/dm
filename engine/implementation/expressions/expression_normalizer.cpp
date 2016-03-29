#include "expression_normalizer.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

namespace
{

/////////// ExpressionNormalizerVisitor /////////////

class ExpressionNormalizerVisitor : public ExpressionVisitor
{
public:
   ExpressionNormalizerVisitor();

   OperationType GetOperation() const;
   bool GetIsLeaf() const;

   // ExpressionVisitor
   virtual void Visit(LiteralExpression& expression) override;
   virtual void Visit(ParamRefExpression& expression) override;
   virtual void Visit(OperationExpression& expression) override;

private:
   // Operation which in each operation expression node is the subtree.
   OperationType m_operation;
   // Is current expression is a leaf of the subtree.
   bool m_is_leaf;
};

ExpressionNormalizerVisitor::ExpressionNormalizerVisitor() :
   m_operation(OperationType::None), m_is_leaf(false)
{
}

OperationType ExpressionNormalizerVisitor::GetOperation() const
{
   return m_operation;
}

bool ExpressionNormalizerVisitor::GetIsLeaf() const
{
   return m_is_leaf;
}


void ExpressionNormalizerVisitor::Visit(LiteralExpression& expression)
{
   m_is_leaf = true;
}

void ExpressionNormalizerVisitor::Visit(ParamRefExpression&)
{
   m_is_leaf = true;
}

void ExpressionNormalizerVisitor::Visit(OperationExpression& expression)
{
   const long child_count = expression.GetChildCount();
   const bool is_associative = IsOperationAssociative(expression.GetOperation());

   for (long index = 0; index < child_count; ++index)
   {
      ExpressionNormalizerVisitor child_visitor;
      expression.GetChild(index)->Accept(child_visitor);

      // An expression can be marked as potentionally normalizable if:
      //    - All children are leaves or operations
      //      with the same operation type, as current expression.
      // If the current operation is not associative additional condition should be checked:
      //    - Only the first child can be not leaf.

      // Here is negated and simplified condtion:

      if (
            !child_visitor.GetIsLeaf() && 
            (
               (!is_associative && index != 0) || 
               child_visitor.GetOperation() == expression.GetOperation()
            )
         )
      {
         return;
      }
   }

   m_operation = expression.GetOperation();
}

/////////// ExpressionLiniarizerVisitor /////////////

}; // namespace

void NormalizeExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ExpressionNormalizerVisitor visitor;
   expression->Accept(visitor);
}

}; // namespace dm