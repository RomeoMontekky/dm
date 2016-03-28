#pragma once

#include "string_utils.h"

#include <string>
#include <vector>

namespace dm
{

class NamedObject
{
public:
   NamedObject();
   NamedObject(const StringPtrLen& name);
   NamedObject(const char* name);

   const std::string& GetName() const;

private:
   std::string m_name;
};

using TNamedObjectVector = std::vector<NamedObject>;

}; // namespace dm