#pragma once

#include "variables/variable_manager.h"
#include "functions/function_base.h"
#include "common/function_utils.h"

namespace dm
{

class FunctionCaller
{
public:
   FunctionCaller(VariableManager& variable_mgr);

   TFunctionOutputPtr ParseAndCall(StringPtrLen str);

private:
   VariableManager& m_variable_mgr;
};

}; // namespace dm