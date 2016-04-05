#include "function_base.h"
#include "../common/exception.h"
#include "../common/qulifier_utils.h"

#include <cassert>

namespace dm
{

Function::Function(const char* name, long param_count) :
   NamedObject(name), m_param_count(param_count)
{
}

Function::~Function()
{
}

long Function::GetParameterCount() const
{
   return m_param_count;
}

const Variable* Function::CheckAndGetConstVariable(
   const VariableManager& variable_mgr, const TStringPtrLenVector& params, long param_index, bool must_exist = true)
{
   CheckQualifier(params, param_index);
   
   assert(param_index >= 0 && param_index < params.size());
   const auto& param = params[param_index];
   auto variable = variable_mgr.FindVariable(param);

   if (must_exist && nullptr == variable)
   {
      Error(GetParameterReportingString(param), " must be an existing variable name");
   }

   if (!must_exist && nullptr != variable)
   {
      Error(GetParameterReportingString(param), " must not be an existing variable name");
   }

   return variable;
}

StringPtrLen Function::CheckQualifier(const TStringPtrLenVector& params, long param_index)
{
   assert(param_index >= 0 && param_index < params.size());
   const auto& param = params[param_index];
   ::CheckQualifier(param, GetParameterReportingString(param));
}

std::string Function::GetParameterReportingString(const StringPtrLen& param_value)
{
   std::stringstream stream;
   stream << "Parameter '" << param_value << "' of function '" << GetName() << '\'';
   return stream.str();
}

}; // namespace dm