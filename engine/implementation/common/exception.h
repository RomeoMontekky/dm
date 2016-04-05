#pragma once

#include <engine/iexception.h>

#include <string>
#include <sstream>

namespace dm
{

class Exception : public IException
{
public:
   Exception(const std::string& error);
   virtual const char* GetDescription() const override;

private:
   std::string m_error;
};

void FormatError(std::stringstream& stream);

template <typename FirstParamType, typename ...OtherParamsType>
void FormatString(
   std::stringstream& stream, 
   const FirstParamType& first_param, 
   const OtherParamsType&... other_params)
{
   stream << first_param;
   FormatString(stream, other_params...);
}

template <typename ...Args>
void Error(const Args&... args)
{
   std::stringstream stream;
   FormatString(stream, args...);
   throw Exception(stream.str());
}

}; // namespace dm