#include "expression_normalizer.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

namespace
{

/////////// ChildrenAbsorberVisitor /////////////

class ChildrenAbsorberVisitor : public ExpressionVisitor
{
public:
   ChildrenAbsorberVisitor();

   TExpressionPtrVector& GetExpressions();

   // ExpressionVisitor
   virtual void Visit(OperationExpression& expression) override;

private:
   TExpressionPtrVector m_expressions;
};

ChildrenAbsorberVisitor::ChildrenAbsorberVisitor() :
   m_expressions()
{
}

TExpressionPtrVector& ChildrenAbsorberVisitor::GetExpressions()
{
   return m_expressions;
}

void ChildrenAbsorberVisitor::Visit(OperationExpression& expression)
{
   const long child_count = expression.GetChildCount();
   m_expressions.reserve(child_count);
   for (long index = 0; index < child_count; ++index)
   {
      m_expressions.push_back(std::move(expression.GetChild(index)));
   }
}

/////////// ExpressionNormalizerVisitor /////////////

class ExpressionNormalizerVisitor : public ExpressionVisitor
{
public:
   ExpressionNormalizerVisitor();

   OperationType GetOperation() const;

   // ExpressionVisitor
   virtual void Visit(OperationExpression& expression) override;

private:
   // Operation which in each operation expression node is the subtree.
   OperationType m_operation;
};

ExpressionNormalizerVisitor::ExpressionNormalizerVisitor() :
   m_operation(OperationType::None)
{
}

OperationType ExpressionNormalizerVisitor::GetOperation() const
{
   return m_operation;
}

void ExpressionNormalizerVisitor::Visit(OperationExpression& expression)
{
   m_operation = expression.GetOperation();

   long child_count = expression.GetChildCount();
   const bool is_associative = IsOperationAssociative(expression.GetOperation());

   for (long index = 0; index < child_count; ++index)
   {
      ExpressionNormalizerVisitor child_visitor;
      expression.GetChild(index)->Accept(child_visitor);

      // The child expression can be normalized if following is true:
      //    - it is either first child, or current operation is associative;
      //    - it is operation expression and operation is the same as in the current expression;

      if ((is_associative || 0 == index) && 
          (child_visitor.GetOperation() == expression.GetOperation()))
      {
         ChildrenAbsorberVisitor absorber;
         expression.GetChild(index)->Accept(absorber);
         auto& expressions = absorber.GetExpressions();

         if (!expressions.empty())
         {
            const long expressions_count = expressions.size();
            expression.RemoveChild(index);
            expression.InsertChildren(index, std::move(expressions));
            index += expressions_count - 1;
            child_count += expressions_count - 1;
         }
      }
   }
}

} // namespace

void NormalizeExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ExpressionNormalizerVisitor visitor;
   expression->Accept(visitor);
}

} // namespace dm