#include "expression_evaluator.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

namespace
{

class ExpressionEvaluatorVisitor : public ExpressionVisitor
{
public:
   ExpressionEvaluatorVisitor();

   // ExpressionVisitor
   virtual void Visit(OperationExpression& expression) override;
};

ExpressionEvaluatorVisitor::ExpressionEvaluatorVisitor()
{
}

void ExpressionEvaluatorVisitor::Visit(OperationExpression& expression)
{
}

}; // namespace

void EvaluateExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ExpressionEvaluatorVisitor visitor;
   expression->Accept(visitor);
}

}; // namespace dm
