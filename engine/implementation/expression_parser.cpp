#include "expression_parser.h"
#include "common/exception.h"
#include "common/bracket_utils.h"
#include "common/string_utils.h"
#include "common/qualifier_utils.h"

#include "expressions/expression_simplifier.h"
#include "expressions/expression_normalizer.h"
#include "expressions/expressions.h"

#include <string>
#include <cstring>
#include <cassert>

namespace dm
{

namespace
{

const char g_token_assignment[] = ":=";

} // namespace

ExpressionParser::ExpressionParser(const VariableManager& variable_mgr) :
   m_variable_mgr(variable_mgr), m_curr_variable(nullptr)
{
}

TVariablePtr ExpressionParser::Parse(StringPtrLen str)
{
   CheckBracketBalance(str);

   TVariablePtr variable;
   
   const char* assignment = FindWithZeroBalance(str, g_token_assignment);
   if (assignment != nullptr)
   {
      variable = ParseVariableDeclaration(str.Left(assignment));
      str = str.Right(assignment + std::strlen(g_token_assignment));
   }
   else
   {
      // Unnamed variable
      variable = std::make_unique<Variable>();
   }
    
   m_curr_variable = variable.get();
   TExpressionPtr expression = ParseExpression(str);
   m_curr_variable = nullptr;

   NormalizeExpression(expression);
   SimplifyExpression(expression);
   
   variable->SetExpression(std::move(expression));

   return variable;
}

TVariablePtr ExpressionParser::ParseVariableDeclaration(StringPtrLen str) const
{
   str.TrimRight();

   BracketsContent content;
   str = content.Parse(str);

   str.Trim();
   CheckQualifier(str, "Variable name");

   if (m_variable_mgr.FindVariable(str) != nullptr)
   {
      Error("Variable '", str, "' is already declared.");
   }

   TVariablePtr variable = std::make_unique<Variable>(str);

   StringPtrLen param;
   while (content.GetPart(param))
   {
      param.Trim();
      CheckQualifier(param, "Parameter name");
      variable->AddParameter(param);
   }

   return variable;
}

TExpressionPtr ExpressionParser::ParseExpression(StringPtrLen str) const
{
   TrimBrackets(str);
   if (0 == str.Len())
   {
      Error("Empty expression is not allowed.");
   }

   TExpressionPtr recursive_expr;
   if (recursive_expr = ParseOperationExpression(str))
   {
      return recursive_expr;
   }
   else if (recursive_expr = ParseLiteralExpression(str))
   {
      return recursive_expr;
   }
   else if (recursive_expr = ParseParameterizedVariableExpression(str))
   {
      return recursive_expr;
   }

   CheckQualifier(str, "Parameter or not parameterized variable name");

   if (recursive_expr = ParseParameterExpression(str))
   {
      return recursive_expr;
   }
   else if (recursive_expr = ParseNotParameterizedVariableExpression(str))
   {
      return recursive_expr;
   }

   Error("Usage of undefined parameter or not parameterized variable name '", str, "'.");

   // To suppress a warning
   return TExpressionPtr();
}

TExpressionPtr ExpressionParser::ParseOperationExpression(StringPtrLen str) const
{
   BracketsBalancer balancer;
   OperationType max_operation = OperationType::None;
   long max_operation_amount = -1;
   StringPtrLen tail = str;

   // Find operation with zero bracket balance and with maximum value.
   // Maximum value means minimal arithmetic priority.
   for (; tail.Len() > 0; tail.RemoveLeft(1))
   {
      if (!balancer.ProcessChar(tail.At(0)) && balancer.GetBalance() == 0)
      {
         OperationType operation = StartsWithOperation(tail); // TODO: Skip the whole operation symbols
         if (operation != OperationType::None)
         {
            if (operation > max_operation)
            {
               max_operation = operation;
               max_operation_amount = 1;
            }
            else if (operation == max_operation)
            {
               ++max_operation_amount;
            }
         }
      }
   }

   if (OperationType::None == max_operation)
   {
      // No operations with zero balance
      return TExpressionPtr();
   }

   assert(balancer.GetBalance() == 0);

   const char* const max_operation_str = OperationTypeToString(max_operation);
   const size_t max_operation_str_len = std::strlen(max_operation_str);

   if (OperationType::Negation == max_operation)
   {
      if (!str.StartsWith(max_operation_str))
      {
         Error("Incorrect usage of unary operation '", max_operation_str, "'.");
      }

      str.RemoveLeft(max_operation_str_len);
      TExpressionPtr child_expression = ParseExpression(str);
      return std::make_unique<OperationExpression>(std::move(child_expression));
   }

   TExpressionPtrVector children_expressions;
   children_expressions.reserve(max_operation_amount + 1);

   tail = str;
   while (tail.Len() > 0)
   {
      if (!balancer.ProcessChar(tail.At(0)) && balancer.GetBalance() == 0)
      {
         if (tail.StartsWith(max_operation_str))
         {
            TExpressionPtr child_expression = ParseExpression(str.Left(tail.Ptr()));
            children_expressions.push_back(std::move(child_expression));
            tail.RemoveLeft(max_operation_str_len);
            str = tail;
            continue;
         }
      }
      tail.RemoveLeft(1);
   }

   TExpressionPtr child_expression = ParseExpression(str);
   children_expressions.push_back(std::move(child_expression));

   return std::make_unique<OperationExpression>(max_operation, std::move(children_expressions));
}

TExpressionPtr ExpressionParser::ParseLiteralExpression(StringPtrLen str) const
{
   LiteralType literal = StringToLiteralType(str);
   if (LiteralType::None == literal)
   {
      return TExpressionPtr();
   }
   return std::make_unique<LiteralExpression>(literal);
}

TExpressionPtr ExpressionParser::ParseParameterizedVariableExpression(StringPtrLen str) const
{
   BracketsContent content;
   StringPtrLen name = content.Parse(str);
   if (str.Len() == name.Len())
   {
      return TExpressionPtr();
   }

   name.Trim();
   CheckQualifier(name, "Variable name");

   auto variable = m_variable_mgr.FindVariable(name);
   if (nullptr == variable)
   {
      Error("Usage of undefined variable '", name, "'.");
   }

   TExpressionPtrVector actual_params;
   actual_params.reserve(5);

   StringPtrLen param;
   while (content.GetPart(param))
   {
      TExpressionPtr param_expr = ParseExpression(param);
      actual_params.push_back(std::move(param_expr));
   }

   if (actual_params.size() != variable->GetParameterCount())
   {
      Error("Incorrect amount of parameters during usage of variable '", variable->GetName(), 
         "'. Expected amount - ", variable->GetParameterCount(), ", actual amount - ", actual_params.size(), ".");
   }

   return variable->GetExpression()->CloneWithSubstitution(actual_params);
}

TExpressionPtr ExpressionParser::ParseParameterExpression(StringPtrLen str) const
{
   long param_index = m_curr_variable->FindParameter(str);
   if (-1 == param_index)
   {
      return TExpressionPtr();
   }
   return std::make_unique<ParamRefExpression>(*m_curr_variable, param_index);
}

TExpressionPtr ExpressionParser::ParseNotParameterizedVariableExpression(StringPtrLen str) const
{
   auto variable = m_variable_mgr.FindVariable(str);
   if (nullptr == variable)
   {
      return TExpressionPtr();
   }

   if (variable->GetParameterCount() > 0)
   {
      Error("Parameters are missing during usage of variable '", variable->GetName(), "'.");
   }

   return variable->GetExpression()->Clone();
}

} // namespace dm
