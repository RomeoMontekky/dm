#pragma once

#include <engine/iengine.h>

#include "variables/variable_manager.h"
#include "functions/function_output.h"
#include "common/noncopyable.h"
#include "expression_parser.h"
#include "function_caller.h"

#include <memory>

namespace dm
{

class Engine : public IEngine, public NonCopyable
{
public:
   Engine();

   // IEngine
   virtual const IStringable& Process(const char* str, long len = -1) override;

private:
   VariableManager m_variable_mgr;
   ExpressionParser m_parser;
   FunctionCaller m_caller;

   TVariablePtr m_last_unnamed_var;
   TFunctionOutputPtr m_last_function_output;
};

} // namespace dm