#include "named_entity.h"

namespace dm
{

NamedEntity::NamedEntity() :
   m_name()
{
}

NamedEntity::NamedEntity(const StringPtrLen& name) :
   m_name(name)
{
}

NamedEntity::NamedEntity(const char* name) :
   m_name(name)
{
}

const std::string& NamedEntity::GetName() const
{
   return m_name;
}

} // namespace dm