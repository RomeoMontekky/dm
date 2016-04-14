#include "../function_base.h"
#include "../function_registrator.h"
#include "../../expressions/expression_evaluator.h"

#include <sstream>
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
   EvaluateExpression(variable->GetExpression());
   return std::make_unique<FunctionOutput>(variable->ToString());
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
