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

FunctionImpl::FunctionImpl() : Function("print")
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   variable_mgr; // To avoid warning

   auto output = std::make_unique<FunctionOutput>();

   for (const auto& param : params)
   {
      output->AddLine(param);
   }

   return output;
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
