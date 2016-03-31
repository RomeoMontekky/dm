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

FunctionTest::FunctionTest() : Function("test")
{
}

TFunctionOutputPtr FunctionTest::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   variable_mgr; // To avoid warning

   auto output = std::make_unique<FunctionOutput>();

   {
      std::stringstream sstr;
      sstr << "Test function called with " << params.size() << " parameters.";
      output->AddLine(sstr.str());
   }

   for (long index = 0; index < (long)params.size(); ++index)
   {
      std::stringstream sstr;
      sstr << "   Parameter [" << index << "] = '" << params.at(index) << "'";
      output->AddLine(sstr.str());
   }

   return output;
}

}; // namespace

REGISTER_FUNCTION(FunctionTest);

}; // namespace dm
