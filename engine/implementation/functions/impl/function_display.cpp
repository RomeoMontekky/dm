#include "../function_base.h"
#include "../function_registrator.h"

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

FunctionImpl::FunctionImpl() : Function("display")
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   CheckNonEmptyParameters(params);
   for (const auto& param : params)
   {
      CheckAndGetConstVariable(variable_mgr, param);
   }

   auto output = std::make_unique<FunctionOutput>();

   for (const auto& param : params)
   {
      auto variable = variable_mgr.FindVariable(param);
      assert(variable != nullptr);
      output->AddLine(variable->ToString());
   }

   return output;
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
