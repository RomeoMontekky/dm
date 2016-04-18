#include "../function_base.h"
#include "../function_registrator.h"
#include "../../common/combinations.h"
#include "../../expressions/expression_calculator.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace dm
{

namespace
{

const char g_char_vert_line = '|';
const char g_char_horz_line = '-';
const char g_char_filler    = ' ';

std::string ConstructHeader(const Variable* variable)
{
   assert(variable != nullptr);

   std::string header;

   const long param_count = variable->GetParameterCount();
   for (long index = 0; index < param_count; ++index)
   {
      header += g_char_vert_line;
      header += g_char_filler;
      header += variable->GetParameter(index).GetName();
      header += g_char_filler;
   }

   header += g_char_vert_line;
   header += g_char_vert_line;
   header += g_char_filler;
   header += variable->VariableDeclaration::ToString();
   header += g_char_filler;
   header += g_char_vert_line;

   return header;
}

std::string ConstructRow(const Variable* variable, const LiteralType param_values[], LiteralType func_value)
{
   assert(variable != nullptr);
   const auto declaration = variable->VariableDeclaration::ToString();

   std::stringstream row;

   const long param_count = variable->GetParameterCount();
   for (long index = 0; index < param_count; ++index)
   {
      row << g_char_vert_line;
      row << g_char_filler;
      row << std::setw(variable->GetParameter(index).GetName().size()) 
          << LiteralTypeToString(param_values[index]);
      row << g_char_filler;
   }

   row << g_char_vert_line;
   row << g_char_vert_line;
   row << g_char_filler;
   row << std::setw(declaration.size()) << LiteralTypeToString(func_value);
   row << g_char_filler;
   row << g_char_vert_line;

   return row.str();
}

class FunctionImpl : public Function
{
public:
   FunctionImpl();

   virtual TFunctionOutputPtr Call(VariableManager& viriable_mgr, const TStringPtrLenVector& params) override;
};

FunctionImpl::FunctionImpl() : Function("table", 1)
{
}

TFunctionOutputPtr FunctionImpl::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.size() == GetParameterCount());

   auto variable = CheckAndGetConstVariable(variable_mgr, params[0]);

   const auto header = ConstructHeader(variable);
   const std::string horizontal_line(header.size(), g_char_horz_line);

   auto output = std::make_unique<FunctionOutput>();

   output->AddLine(horizontal_line);
   output->AddLine(header);
   output->AddLine(horizontal_line);

   CombinationGenerator generator(variable->GetParameterCount());
   const LiteralType* param_values = generator.GenerateFirst();
   while (param_values != nullptr)
   {
      const LiteralType func_value = CalculateExpression(variable->GetExpression().get(), param_values);
      output->AddLine(ConstructRow(variable, param_values, func_value));
      param_values = generator.GenerateNext();
   }

   output->AddLine(horizontal_line);
   
   return output;
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
