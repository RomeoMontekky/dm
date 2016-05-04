#include "../function_base.h"
#include "../function_registrator.h"
#include "../../expressions/expression_evaluator.h"

#include <cassert>

namespace dm
{

namespace
{

class FunctionImpl : public Function
{
public:
   FunctionImpl();

   virtual TFunctionOutputPtr Call(VariableManager& viriable_mgr, const TStringPtrLenVector& params) override;
};

FunctionImpl::FunctionImpl() : Function("eval", 1)
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.size() == GetParameterCount());
   auto variable = CheckAndGetVariable(variable_mgr, params[0]);

#ifndef NDEBUG
   TStringPtrLenVector copy_params;
   copy_params.reserve(2);
   std::string original_name = std::string(params[0]) + "_original";
   copy_params.emplace_back(original_name.c_str(), original_name.size());
   copy_params.push_back(params[0]);
   FunctionManager::GetInstance().FindFunction(StringPtrLen("copy", -1))->Call(variable_mgr, copy_params);
#endif

   EvaluateExpression(variable->GetExpression());

#ifndef NDEBUG
   TFunctionOutputPtr result = FunctionManager::GetInstance().
                              FindFunction(StringPtrLen("compare", -1))->Call(variable_mgr, copy_params);
   std::string result_str = result->ToString();
   if (result_str.substr(result_str.size() - 6) != "equal.")
   {
      auto output = std::make_unique<FunctionOutput>();
      output->AddLine(variable->ToString());
      output->AddLine(result_str);
      return output;
   }
#endif

   return std::make_unique<FunctionOutput>(variable->ToString());
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
