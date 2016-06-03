#pragma once

#include "expression_base.h"

#include "../common/literals.h"
#include "../common/operations.h"

namespace dm
{

LiteralType GetLiteral(const TExpressionPtr& expr);
OperationType GetOperation(const TExpressionPtr& expr);

// Forward declarations of expressions.
class LiteralExpression;
class ParamRefExpression;
class OperationExpression;

// Expression casts
const LiteralExpression& CastToLiteral(const TExpressionPtr& expr);
const ParamRefExpression& CastToParamRef(const TExpressionPtr& expr);
const OperationExpression& CastToOperation(const TExpressionPtr& expr);
OperationExpression& CastToOperation(TExpressionPtr& expr);

// Following functions allow to move child expressions from the operation expression.
void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expr);
void MoveChildExpressions(TExpressionPtrVector& target, OperationExpression& expression);

} // namespace dm
