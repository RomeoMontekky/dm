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

FunctionImpl::FunctionImpl() : Function("display_all", 0)
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.empty());

   auto output = std::make_unique<FunctionOutput>();

   for (auto variable = variable_mgr.GetFirstVariable(); 
        variable != nullptr; variable = variable_mgr.GetNextVariable())
   {
      output->AddLine(variable->ToString());
   }

   return output;
}

}; // namespace

REGISTER_FUNCTION(FunctionImpl);

}; // namespace dm
