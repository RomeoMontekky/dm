#include "expression_normalizer.h"
#include "expression_visitor.h"
#include "expressions.h"
#include "expression_mover.h"

#include <cassert>

namespace dm
{

namespace
{

class ExpressionNormalizer : public ExpressionVisitor
{
public:
   ExpressionNormalizer();

   OperationType GetOperation() const;

   // ExpressionVisitor
   virtual void Visit(OperationExpression& expression) override;

private:
   // Operation which in each operation expression node is the subtree.
   OperationType m_operation;
};

ExpressionNormalizer::ExpressionNormalizer() :
   m_operation(OperationType::None)
{
}

OperationType ExpressionNormalizer::GetOperation() const
{
   return m_operation;
}

void ExpressionNormalizer::Visit(OperationExpression& expression)
{
   m_operation = expression.GetOperation();

   if (OperationType::Negation == m_operation)
   {
      // Do not normalize negation.
      // Removing of double negation is implied in operation evaluation.
      return;
   }

   long child_count = expression.GetChildCount();
   const bool is_associative = IsOperationAssociative(expression.GetOperation());

   for (long index = 0; index < child_count; ++index)
   {
      ExpressionNormalizer child_normalizer;
      expression.GetChild(index)->Accept(child_normalizer);

      // The child expression can be normalized if following is true:
      //    - it is either first child, or current operation is associative;
      //    - it is operation expression and operation is the same as in the current expression;

      if ((is_associative || 0 == index) && 
          (child_normalizer.GetOperation() == expression.GetOperation()))
      {
         auto moved_children = MoveChildExpressions(expression.GetChild(index));
         assert(!moved_children.empty());

         const long moved_children_count = moved_children.size();
         expression.RemoveChild(index);
         expression.InsertChildren(index, std::move(moved_children));
         index += moved_children_count - 1;
         child_count += moved_children_count - 1;
      }
   }
}

} // namespace

void NormalizeExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ExpressionNormalizer normalizer;
   expression->Accept(normalizer);
}

} // namespace dm
