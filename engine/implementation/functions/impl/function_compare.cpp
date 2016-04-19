#include "../function_base.h"
#include "../function_registrator.h"
#include "../../common/combinations.h"
#include "../../expressions/expression_calculator.h"

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

FunctionImpl::FunctionImpl() : Function("compare", 2)
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.size() == GetParameterCount());

   auto variable1 = CheckAndGetConstVariable(variable_mgr, params[0]);
   auto variable2 = CheckAndGetConstVariable(variable_mgr, params[1]);
   
   std::stringstream stream;
   stream << "Variables '" << variable1->GetName() << "' and '" << variable2->GetName() << "' are ";

   if (variable1->GetParameterCount() != variable2->GetParameterCount())
   {
      stream << "not equal. Different number of parameters.";
      return std::make_unique<FunctionOutput>(stream.str());
   }
   
   const long param_count = variable1->GetParameterCount();
   
   CombinationGenerator generator(param_count);
   const LiteralType* param_values = generator.GenerateFirst();
   while (param_values != nullptr)
   {
      const LiteralType result1 = CalculateExpression(variable1->GetExpression().get(), param_values);
      const LiteralType result2 = CalculateExpression(variable2->GetExpression().get(), param_values);
      
      if (result1 != result2)
      {
         stream << "not equal. Different results on parameter combination (";
         bool is_first = true;
         for (long index = 0; index < param_count; ++index)
         {
            if (is_first)
            {
               is_first = false;
            }
            else
            {
               stream << ", ";
            }
            stream << LiteralTypeToString(param_values[index]);
         }
         stream << ").";
         return std::make_unique<FunctionOutput>(stream.str());
      }
      
      param_values = generator.GenerateNext();
   }
   
   stream << "equal.";
   return std::make_unique<FunctionOutput>(stream.str());
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
