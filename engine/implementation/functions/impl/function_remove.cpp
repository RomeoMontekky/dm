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

FunctionImpl::FunctionImpl() : Function("remove")
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
      variable_mgr.RemoveVariable(param);
      std::stringstream stream;
      stream << "Variable '" << param << "' was removed.";
      output->AddLine(stream.str());
   }

   return output;
}

}; // namespace

REGISTER_FUNCTION(FunctionImpl);

}; // namespace dm
