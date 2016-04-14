#include "function_output.h"

namespace dm
{

FunctionOutput::FunctionOutput() :
   m_output()
{
}

FunctionOutput::FunctionOutput(const std::string& line) :
   m_output()
{
   AddLine(line);
}

FunctionOutput::FunctionOutput(const char* line) :
   m_output()
{
   AddLine(line);
}

void FunctionOutput::AddLine(const std::string& line)
{
   if (!m_output.empty())
   {
      m_output += '\n';
   }
   m_output += line;
}

void FunctionOutput::AddLine(const char* line)
{
   if (!m_output.empty())
   {
      m_output += '\n';
   }
   m_output += line;
}

std::string FunctionOutput::ToString() const
{
   return m_output;
}

} // namespace dm