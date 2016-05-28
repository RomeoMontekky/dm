#pragma once

#include "expression_base.h"
#include "../common/literals.h"

namespace dm
{

LiteralType CalculateExpression(const TExpressionPtr& expr, const LiteralType param_values[]);

} // namespace dm
