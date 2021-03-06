#pragma once

#include "function_output.h"
#include "../variables/variable_manager.h"
#include "../common/named_entity.h"
#include "../common/string_utils.h"

#include <memory>
#include <vector>

namespace dm
{

class Function;
using TFunctionPtr = std::unique_ptr<Function>;

class Function : public NamedEntity
{
public:
   Function(const char* name, long param_count = -1);
   virtual ~Function();

   long GetParameterCount() const;

   virtual TFunctionOutputPtr Call(VariableManager& variable_mgr, const TStringPtrLenVector& params) = 0;

protected:
   void CheckNonEmptyParameters(const TStringPtrLenVector& params);
   void CheckQualifier(const StringPtrLen& param);

   const Variable* CheckAndGetConstVariable(
      const VariableManager& variable_mgr, const StringPtrLen& param, bool must_exist = true);
   Variable* CheckAndGetVariable(
      VariableManager& variable_mgr, const StringPtrLen& param, bool must_exist = true);

private:
   std::string GetParameterReportingString(const StringPtrLen& param_value);

private:
   long m_param_count;
};

} // namespace dm
