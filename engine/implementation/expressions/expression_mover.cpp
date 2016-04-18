#include "expression_mover.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

namespace
{

class ChildExpressionsMover : public ExpressionVisitor
{
public:
   ChildExpressionsMover();

   TExpressionPtrVector& GetExpressions();

   // ExpressionVisitor
   virtual void Visit(OperationExpression& expression) override;

private:
   TExpressionPtrVector m_expressions;
};

ChildExpressionsMover::ChildExpressionsMover() :
   m_expressions()
{
}

TExpressionPtrVector& ChildExpressionsMover::GetExpressions()
{
   return m_expressions;
}

void ChildExpressionsMover::Visit(OperationExpression& expression)
{
   const long child_count = expression.GetChildCount();
   m_expressions.reserve(child_count);
   for (long index = 0; index < child_count; ++index)
   {
      m_expressions.push_back(std::move(expression.GetChild(index)));
   }
}

} // namespace

void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ChildExpressionsMover mover;
   expression->Accept(mover);
   target = std::move(mover.GetExpressions());
}

void MoveChildExpression(TExpressionPtr& target, TExpressionPtr& expression, long child_index)
{
   TExpressionPtrVector moved_expressions;
   MoveChildExpressions(moved_expressions, expression);
   assert(child_index >= 0 && child_index < moved_expressions.size());
   target = std::move(moved_expressions[0]);
}

void MoveChildExpressionInplace(TExpressionPtr& expression, long child_index)
{
   MoveChildExpression(expression, expression, child_index);
}

} // namespace dm
