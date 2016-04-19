#include "../function_base.h"
#include "../function_registrator.h"

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

FunctionImpl::FunctionImpl() : Function("remove_all", 0)
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.empty());
   variable_mgr.RemoveAllVariables();
   return std::make_unique<FunctionOutput>("All variables were removed.");
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
