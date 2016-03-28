#include "function_base.h"
#include "../common/exception.h"

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

}; // namespace dm