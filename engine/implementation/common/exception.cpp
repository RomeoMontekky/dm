#include "exception.h"

namespace dm
{

IException::IException()
{
}

IException::~IException()
{
}

Exception::Exception(const std::string& error) : 
   IException(), m_error(error)
{
}

const char* Exception::GetDescription() const
{
   return m_error.c_str();
}

void FormatString(std::stringstream& stream)
{
}

}; // namespace dm