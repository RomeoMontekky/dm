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

FunctionImpl::FunctionImpl() : Function("remove", 1)
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.size() == GetParameterCount());

   CheckAndGetConstVariable(variable_mgr, params, 0);
   viriable_mgr.RemoveVariable(params[0]);

   std::stringstream stream;
   stream << "Variable '" << params[0] << "' was removed";
   
   return std::make_unique<FunctionOutput>(stream.str());
}

}; // namespace

REGISTER_FUNCTION(FunctionImpl);

}; // namespace dm
