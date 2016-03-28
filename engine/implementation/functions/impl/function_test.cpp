#include "../function_base.h"
#include "../function_registrator.h"

#include <sstream>
#include <cassert>

namespace dm
{

namespace
{

class FunctionTest : public Function
{
public:
   FunctionTest();

   virtual TFunctionOutputPtr Call(VariableManager& viriable_mgr, const TStringPtrLenVector& params) override;
};

FunctionTest::FunctionTest() : Function("test", 1)
{
}

TFunctionOutputPtr FunctionTest::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.size() == GetParameterCount());
   
   variable_mgr; // To avoid warning

   auto output = std::make_unique<FunctionOutput>();

   std::stringstream sstr;
   sstr << "Test function called with parameter '" << params.at(0) << "'.";
   output->AddLine(sstr.str());

   return output;
}

}; // namespace

REGISTER_FUNCTION(FunctionTest);

}; // namespace dm
