#include "../function_base.h"
#include "../function_registrator.h"
#include "../../common/exception.h"
#include "../../expressions/expression_calculator.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
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
   header += variable->GetName();
   header += g_char_filler;
   header += g_char_vert_line;

   return header;
}

std::string ConstructRow(const Variable* variable, const LiteralType param_values[], LiteralType func_value)
{
   assert(variable != nullptr);

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
   row << std::setw(variable->GetName().size()) << LiteralTypeToString(func_value);
   row << g_char_filler;
   row << g_char_vert_line;

   return row.str();
}

class FunctionTable : public Function
{
public:
   FunctionTable();

   virtual TFunctionOutputPtr Call(VariableManager& viriable_mgr, const TStringPtrLenVector& params) override;
};

FunctionTable::FunctionTable() : Function("table", 1)
{
}

TFunctionOutputPtr FunctionTable::Call(VariableManager& variable_mgr, const TStringPtrLenVector& params)
{
   assert(params.size() == GetParameterCount());

   auto variable = variable_mgr.FindVariable(params[0]);
   if (nullptr == variable)
   {
      Error("Usage of undefined variable '", params[0], "'.");
   }

   const auto header = ConstructHeader(variable);
   const std::string horizontal_line(header.size(), g_char_horz_line);

   auto output = std::make_unique<FunctionOutput>();

   output->AddLine(horizontal_line);
   output->AddLine(header);
   output->AddLine(horizontal_line);

   // One element on each variable's parameter + one element
   // to hold carry-flag from highest bit-element.
   const long all_count = variable->GetParameterCount() + 1;

#ifdef _MSC_VER
   std::unique_ptr<LiteralType[]> all_values_ptr = std::make_unique<LiteralType[]>(all_count);
   LiteralType * const all_values = all_values_ptr.get();
#else
   LiteralType all_values[all_count];
#endif

   std::fill(all_values, all_values + all_count, LiteralType::False);

   // Element with carry-flag is skipped
   const LiteralType* const param_values = all_values + 1;

   // Checking of carry-flag
   while (LiteralType::False == *all_values)
   {
      const LiteralType func_value = CalculateExpression(variable->GetExpression(), param_values);

      output->AddLine(ConstructRow(variable, param_values, func_value));

      long i = all_count - 1;
      for (; i > 0 && LiteralType::True == all_values[i]; all_values[i--] = LiteralType::False);
      all_values[i] = LiteralType::True;
   }

   output->AddLine(horizontal_line);
   
   return output;
}

}; // namespace

REGISTER_FUNCTION(FunctionTable);

}; // namespace dm
