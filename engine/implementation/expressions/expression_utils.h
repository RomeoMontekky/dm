#pragma once

#include "expression_base.h"

#include "../common/literals.h"
#include "../common/operations.h"

namespace dm
{

// Forward declaration.
class OperationExpression;

LiteralType GetLiteral(const TExpressionPtr& expr);
OperationType GetOperation(const TExpressionPtr& expr);

OperationExpression& CastToOperation(TExpressionPtr& expr);
const OperationExpression& CastToOperation(const TExpressionPtr& expr);

// Set of functions to move child expressions from the operation expression.
// There is possibility to move them to:
//    - target vector.
//    - target expression (the only child is moved, other will be destroyed).
//    - original expression with overwriting.
void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expr);
void MoveChildExpressions(TExpressionPtrVector& target, OperationExpression& expression);
void MoveChildExpression(TExpressionPtr& target, TExpressionPtr& expr, long child_index = 0);
void MoveChildExpressionInplace(TExpressionPtr& expr, long child_index = 0);

} // namespace dm
