#pragma once

#include "expression_base.h"

namespace dm
{

void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expression);
void MoveChildExpression(TExpressionPtr& target, TExpressionPtr& expression, long child_index = 0);
void MoveChildExpressionInplace(TExpressionPtr& expression, long child_index = 0);

void RemoveChildExpression(TExpressionPtr& expression, long child_index);

} // namespace dm
