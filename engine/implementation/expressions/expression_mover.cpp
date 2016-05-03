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

class ChildExpressionRemover : public ExpressionVisitor
{
public:
   ChildExpressionRemover(long index);

   // ExpressionVisitor
   virtual void Visit(OperationExpression& expression) override;

private:
   long m_child_index;
};

ChildExpressionRemover::ChildExpressionRemover(long child_index) :
   m_child_index(child_index)
{
}

void ChildExpressionRemover::Visit(OperationExpression& expression)
{
   if (m_child_index < 0)
   {
      m_child_index = expression.GetChildCount() + m_child_index;
   }
   expression.RemoveChild(m_child_index);
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
   target = std::move(moved_expressions[child_index]);
}

void MoveChildExpressionInplace(TExpressionPtr& expression, long child_index)
{
   MoveChildExpression(expression, expression, child_index);
}

void RemoveChildExpression(TExpressionPtr& expression, long child_index)
{
   assert(expression.get() != nullptr);
   ChildExpressionRemover remover(child_index);
   expression->Accept(remover);
}

} // namespace dm
