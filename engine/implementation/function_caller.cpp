#include "function_caller.h"
#include "functions/function_manager.h"
#include "common/exception.h"
#include "common/bracket_utils.h"
#include "common/string_utils.h"
#include "common/qualifier_utils.h"

#include <string>
#include <vector>
#include <cstring>
#include <cassert>

namespace dm
{

FunctionCaller::FunctionCaller(VariableManager& variable_mgr) :
   m_variable_mgr(variable_mgr)
{
}

TFunctionOutputPtr FunctionCaller::ParseAndCall(StringPtrLen str)
{
   TrimFunctionCall(str);

   CheckBracketBalance(str);

   BracketsContent content;
   str = content.Parse(str);

   str.Trim();
   CheckQualifier(str, "Function name");

   auto function = FunctionManager::GetInstance().FindFunction(str);
   if (nullptr == function)
   {
      Error("Call of undefined function '", str, "'.");
   }

   TStringPtrLenVector params;

   StringPtrLen param;
   while (content.GetPart(param))
   {
      param.Trim();
      params.push_back(param);
   }

   if (function->GetParameterCount() != -1 && params.size() != function->GetParameterCount())
   {
      Error("Incorrect amount of parameters during call of function '", function->GetName(), 
            "'. Expected amount - ", function->GetParameterCount(), ", actual amount - ", params.size(), ".");
   }

   return function->Call(m_variable_mgr, params);
}

}; // namespace dm