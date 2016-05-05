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
   TStringPtrLenVector nested_params;
   nested_params.reserve(2);
   std::string original_name = std::string(params[0]) + "_original";
   nested_params.emplace_back(original_name.c_str(), original_name.size());
   nested_params.push_back(params[0]);
   
   auto copy_function = FunctionManager::GetInstance().FindFunction("copy");
   assert(copy_function != nullptr);
   copy_function->Call(variable_mgr, nested_params);
#endif

   EvaluateExpression(variable->GetExpression());

#ifndef NDEBUG
   auto compare_function = FunctionManager::GetInstance().FindFunction("compare");
   assert(compare_function != nullptr);
   TFunctionOutputPtr result = compare_function->Call(variable_mgr, nested_params);
   const std::string result_str = result->ToString();
   
   nested_params.resize(1);
   
   auto remove_function = FunctionManager::GetInstance().FindFunction("remove");
   assert(remove_function != nullptr);
   remove_function->Call(variable_mgr, nested_params);
   
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
