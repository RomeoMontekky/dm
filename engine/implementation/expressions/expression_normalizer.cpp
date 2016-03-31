#include "expression_normalizer.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

namespace
{

/////////// ExpressionLiniarizerVisitor /////////////

class ExpressionLiniarizerVisitor : public ExpressionVisitor
{
public:
   ExpressionLiniarizerVisitor();

   TExpressionPtrVector& GetExpressions();

   // ExpressionVisitor
   virtual void Visit(OperationExpression& expression) override;

private:
   TExpressionPtrVector m_expressions;
};

ExpressionLiniarizerVisitor::ExpressionLiniarizerVisitor() :
   m_expressions()
{
}

TExpressionPtrVector& ExpressionLiniarizerVisitor::GetExpressions()
{
   return m_expressions;
}

void ExpressionLiniarizerVisitor::Visit(OperationExpression& expression)
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
      ExpressionNormalizerVisitor normalizer_visitor;
      expression.GetChild(index)->Accept(normalizer_visitor);

      // The child expression can be linearized if following is true:
      //    - it is either first child, or current operation is associative;
      //    - it is operation expression and operation is the same as in the current expression;

      if ((is_associative || 0 == index) && 
          (normalizer_visitor.GetOperation() == expression.GetOperation()))
      {
         ExpressionLiniarizerVisitor liniarizer_visitor;
         expression.GetChild(index)->Accept(liniarizer_visitor);

         if (!liniarizer_visitor.GetExpressions().empty())
         {
            const long expressions_count = liniarizer_visitor.GetExpressions().size();
            expression.RemoveChild(index);
            expression.InsertChildren(index, std::move(liniarizer_visitor.GetExpressions()));
            index += expressions_count - 1;
            child_count += expressions_count - 1;
         }
      }
   }
}

}; // namespace

void NormalizeExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ExpressionNormalizerVisitor visitor;
   expression->Accept(visitor);
}

}; // namespace dm