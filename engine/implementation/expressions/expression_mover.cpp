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

TExpressionPtrVector MoveChildExpressions(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ChildExpressionsMover mover;
   expression->Accept(mover);
   return std::move(mover.GetExpressions());
}

} // namespace dm