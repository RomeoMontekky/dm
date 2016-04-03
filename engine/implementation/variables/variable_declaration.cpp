#include "variable_declaration.h"
#include "../common/exception.h"

#include <cassert>

namespace dm
{

VariableDeclaration::VariableDeclaration() : // unnamed variable declaration
   NamedObject()
{
}

VariableDeclaration::VariableDeclaration(const StringPtrLen& name) :
   NamedObject(name)
{
}

void VariableDeclaration::AddParameter(const StringPtrLen& name)
{
   if (FindParameter(name) >= 0)
   {
      Error("Duplicate parameter '", name, 
            "' occured in declaration of variable '", GetName(), "'.");
   }
   m_parameters.emplace_back(name);
}

long VariableDeclaration::FindParameter(const StringPtrLen& name) const
{
   std::string parameter_name = name;
   for (size_t index = 0; index < m_parameters.size(); ++index)
   {
      if (m_parameters[index].GetName() == parameter_name)
      {
         return index;
      }
   }
   return -1;
}

long VariableDeclaration::GetParameterCount() const
{
   return m_parameters.size();
}

const NamedObject& VariableDeclaration::GetParameter(long index) const
{
   assert(index >= 0 && index < (long)m_parameters.size());
   return m_parameters.at(index);
}


std::string VariableDeclaration::ToString() const
{
   std::string ret;

   if (!GetName().empty())
   {
      ret += GetName();

      if (!m_parameters.empty())
      {
         ret += '(';
   
         bool is_first = true;
         for (const auto& parameter : m_parameters)
         {
            if (is_first)
            {
               is_first = false;
            }
            else
            {
               ret += ", ";
            }
            ret += parameter.GetName();
         }
   
         ret += ")";
      }
   }
   
   return ret;
}

}; // namespace dm