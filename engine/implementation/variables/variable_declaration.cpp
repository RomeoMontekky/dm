#include "variable_declaration.h"
#include "../common/exception.h"

#include <cassert>

namespace dm
{

VariableDeclaration::VariableDeclaration() : // unnamed variable declaration
   NamedEntity(), m_parameters()
{
}

VariableDeclaration::VariableDeclaration(const StringPtrLen& name) :
   NamedEntity(name), m_parameters()
{
}

VariableDeclaration::VariableDeclaration(
   const StringPtrLen& name, const VariableDeclaration& rhs) :
      NamedEntity(name), m_parameters(rhs.m_parameters)
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
   for (auto index = 0L; index < (long)m_parameters.size(); ++index)
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

const NamedEntity& VariableDeclaration::GetParameter(long index) const
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
   
         auto is_first = true;
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

} // namespace dm
