#include "expression_visitor.h"

namespace dm
{

///////////// ExpressionVisitor ///////////////

ExpressionVisitor::ExpressionVisitor()
{
}

ExpressionVisitor::~ExpressionVisitor()
{
}

void ExpressionVisitor::Visit(LiteralExpression& expression)
{
}

void ExpressionVisitor::Visit(ParamRefExpression& expression)
{
}

void ExpressionVisitor::Visit(OperationExpression& expression)
{
}

////////// ConstExpressionVisitor ////////////

ConstExpressionVisitor::ConstExpressionVisitor()
{
}

ConstExpressionVisitor::~ConstExpressionVisitor()
{
}

void ConstExpressionVisitor::Visit(const LiteralExpression& expression)
{
}

void ConstExpressionVisitor::Visit(const ParamRefExpression& expression)
{
}

void ConstExpressionVisitor::Visit(const OperationExpression& expression)
{
}

} // namespace dm