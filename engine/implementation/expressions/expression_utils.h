#pragma once

#include "expression_base.h"

#include "../common/literals.h"
#include "../common/operations.h"

namespace dm
{

LiteralType GetLiteral(const TExpressionPtr& expression);
OperationType GetOperation(const TExpressionPtr& expression);

// Set of functions to move child expressions from the operation expression.
// There is possibility to move them to:
//    - target vector.
//    - target expression (the only child is moved, other will be destroyed).
//    - original expression with overwriting.
void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expression);
void MoveChildExpression(TExpressionPtr& target, TExpressionPtr& expression, long child_index = 0);
void MoveChildExpressionInplace(TExpressionPtr& expression, long child_index = 0);

// Removes child expression from the operation expression.
//    If child_index =  n, removes n-th child from the beginning.
//    If child_index = -n. removes n-th child from the end.
void RemoveChildExpression(TExpressionPtr& expression, long child_index);

// Adds child expression to the end of the operation expression.
void AddChildExpression(TExpressionPtr& expression, TExpression&& child);

} // namespace dm
