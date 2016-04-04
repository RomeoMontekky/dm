#include "../function_base.h"
#include "../function_registrator.h"
#include "../../common/exception.h"

#include <sstream>
#include <cassert>

namespace dm
{

namespace
{

class FunctionEval : public Function
{
public:
   FunctionEval();

   virtual TFunctionOutputPtr Call(VariableManager& viriable_mgr, const TStringPtrLenVector& params) override;
};

FunctionEval::FunctionEval() : Function("eval", 1)
{
}

TFunctionOutputPtr FunctionEval::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.size() == GetParameterCount());

   auto variable = variable_mgr.FindVariable(params[0]);
   if (nullptr == variable)
   {
      Error("Usage of undefined variable '", params[0], "'.");
   }

   auto output = std::make_unique<FunctionOutput>();

   // TODO: Implement
   output->AddLine("Eval function called");

   return output;
}

}; // namespace

REGISTER_FUNCTION(FunctionEval);

}; // namespace dm
