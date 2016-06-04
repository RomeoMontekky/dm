#pragma once

#include "expression_base.h"

#include "../common/literals.h"
#include "../common/operations.h"

namespace dm
{

class LiteralExpression;
class ParamRefExpression;
class OperationExpression;

// Expression casts
const LiteralExpression& CastToLiteral(const TExpressionPtr& expr);
LiteralExpression& CastToLiteral(TExpressionPtr& expr);
const ParamRefExpression& CastToParamRef(const TExpressionPtr& expr);
ParamRefExpression& CastToParamRef(TExpressionPtr& expr);
const OperationExpression& CastToOperation(const TExpressionPtr& expr);
OperationExpression& CastToOperation(TExpressionPtr& expr);

// Helpers
LiteralType GetLiteral(const TExpressionPtr& expr);
long GetParamIndex(const TExpressionPtr& expr);
OperationType GetOperation(const TExpressionPtr& expr);

// Move child expressions from an operation expression to the target.
void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expr);
void MoveChildExpressions(TExpressionPtrVector& target, OperationExpression& expression);

// Move child expressions from child_index-th child to its place in the parent expression.
void MoveChildExpressionsUp(OperationExpression& expression, long child_index);

} // namespace dm
