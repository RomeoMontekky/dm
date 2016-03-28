#pragma once

#include "variables/variable.h"
#include "variables/variable_manager.h"
#include "expressions/expression_base.h"
#include "common/string_utils.h"

namespace dm
{

class ExpressionParser
{
public:
   ExpressionParser(const VariableManager& variable_mgr);

   TVariablePtr Parse(StringPtrLen str);

private:
   TVariablePtr ParseVariableDeclaration(StringPtrLen str) const;

   TExpressionPtr ParseExpression(StringPtrLen str) const;
   TExpressionPtr ParseOperationExpression(StringPtrLen str) const;
   TExpressionPtr ParseLiteralExpression(StringPtrLen str) const;
   TExpressionPtr ParseParameterizedVariableExpression(StringPtrLen str) const;
   TExpressionPtr ParseParameterExpression(StringPtrLen str) const;
   TExpressionPtr ParseNotParameterizedVariableExpression(StringPtrLen str) const;

private:
   const VariableManager& m_variable_mgr;
   const VariableDeclaration* m_curr_variable;
};

}; // namespace dm