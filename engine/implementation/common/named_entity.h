#pragma once

#include "string_utils.h"

#include <string>
#include <vector>

namespace dm
{

class NamedEntity
{
public:
   NamedEntity();
   NamedEntity(const StringPtrLen& name);
   NamedEntity(const char* name);

   const std::string& GetName() const;

private:
   std::string m_name;
};

using TNamedEntityVector = std::vector<NamedEntity>;

} // namespace dm