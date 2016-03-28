#include "named_object.h"

namespace dm
{

NamedObject::NamedObject() :
   m_name()
{
}

NamedObject::NamedObject(const StringPtrLen& name) :
   m_name(name)
{
}

NamedObject::NamedObject(const char* name) :
   m_name(name)
{
}

const std::string& NamedObject::GetName() const
{
   return m_name;
}

}; // namespace dm