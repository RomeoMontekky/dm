#include "expression_normalizer.h"
#include "expression_visitor.h"
#include "expressions.h"

#include <cassert>

namespace dm
{

namespace
{

class ExpressionNormalizerVisitor : public ExpressionVisitor
{
public:
   ExpressionNormalizerVisitor();

   // ExpressionVisitor
   virtual void Visit(LiteralExpression& expression) override;
   virtual void Visit(ParamRefExpression& expression) override;
   virtual void Visit(OperationExpression& expression) override;
};

ExpressionNormalizerVisitor::ExpressionNormalizerVisitor()
{
}

void ExpressionNormalizerVisitor::Visit(LiteralExpression& expression)
{
}

void ExpressionNormalizerVisitor::Visit(ParamRefExpression&)
{
}

void ExpressionNormalizerVisitor::Visit(OperationExpression& expression)
{
}

}; // namespace

void NormalizeExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);

   ExpressionNormalizerVisitor visitor;
   expression->Accept(visitor);
}

}; // namespace dm