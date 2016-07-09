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

   const auto param_count = variable->GetParameterCount();
   for (auto index = 0L; index < param_count; ++index)
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

std::string ConstructRow(const Variable* variable, const LiteralType param_values[], LiteralType result)
{
   assert(variable != nullptr);
   const auto declaration = variable->VariableDeclaration::ToString();

   std::stringstream row;

   const auto param_count = variable->GetParameterCount();
   for (auto index = 0L; index < param_count; ++index)
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
   row << std::setw(declaration.size()) << LiteralTypeToString(result);
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

   for (auto param_values = generator.GenerateFirst();
        param_values != nullptr;
        param_values = generator.GenerateNext())
   {
      const auto result = CalculateExpression(variable->GetExpression(), param_values);
      output->AddLine(ConstructRow(variable, param_values, result));
   }

   output->AddLine(horizontal_line);
   
   return output;
}

} // namespace

REGISTER_FUNCTION(FunctionImpl);

} // namespace dm
