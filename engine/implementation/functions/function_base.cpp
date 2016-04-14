#include "function_base.h"
#include "../common/exception.h"
#include "../common/qualifier_utils.h"

#include <cassert>

namespace dm
{

Function::Function(const char* name, long param_count) :
   NamedEntity(name), m_param_count(param_count)
{
}

Function::~Function()
{
}

long Function::GetParameterCount() const
{
   return m_param_count;
}

void Function::CheckNonEmptyParameters(const TStringPtrLenVector& params)
{
   if (params.empty())
   {
      Error("Function '", GetName(), "' can't have empty list of parameters.");
   }
}

void Function::CheckQualifier(const StringPtrLen& param)
{
   dm::CheckQualifier(param, GetParameterReportingString(param).c_str());
}

const Variable* Function::CheckAndGetConstVariable(
   const VariableManager& variable_mgr, const StringPtrLen& param, bool must_exist)
{
   CheckQualifier(param);
   
   auto variable = variable_mgr.FindVariable(param);
   if (must_exist && nullptr == variable)
   {
      Error(GetParameterReportingString(param), " must be an existing variable name.");
   }

   if (!must_exist && nullptr != variable)
   {
      Error(GetParameterReportingString(param), " must not be an existing variable name.");
   }

   return variable;
}

Variable* Function::CheckAndGetVariable(
   VariableManager& variable_mgr, const StringPtrLen& param, bool must_exist)
{
   return const_cast<Variable*>(CheckAndGetConstVariable(variable_mgr, param, must_exist));
}

std::string Function::GetParameterReportingString(const StringPtrLen& param)
{
   std::stringstream stream;
   stream << "Parameter '" << param << "' of function '" << GetName() << '\'';
   return stream.str();
}

} // namespace dm
