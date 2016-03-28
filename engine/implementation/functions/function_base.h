#pragma once

#include "function_output.h"
#include "../variables/variable_manager.h"
#include "../common/named_object.h"
#include "../common/string_utils.h"

#include <memory>
#include <vector>

namespace dm
{

class Function : public NamedObject
{
public:
   Function(const char* name, long param_count = -1);
   virtual ~Function();

   long GetParameterCount() const;

   virtual TFunctionOutputPtr Call(VariableManager& variable_mgr, const TStringPtrLenVector& params) = 0;

private:
   long m_param_count;
};

using TFunctionPtr = std::unique_ptr<Function>;

}; // namespace dm