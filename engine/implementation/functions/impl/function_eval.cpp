#include "../function_base.h"
#include "../function_registrator.h"
#include "../../common/exception.h"

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

   auto variable = CheckAndGetConstVariable(variable_mgr, params[0]);

   auto output = std::make_unique<FunctionOutput>();

   // TODO: Implement
   output->AddLine("Eval function called");

   return output;
}

}; // namespace

REGISTER_FUNCTION(FunctionImpl);

}; // namespace dm
