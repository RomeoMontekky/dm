#pragma once

#include "expression_base.h"

namespace dm
{

void MoveChildExpressions(TExpressionPtrVector& target, TExpressionPtr& expression);
void MoveChildExpression(TExpressionPtr& target, TExpressionPtr& expression);
void MoveChildExpressionInplace(TExpressionPtr& expression);

} // namespace dm
