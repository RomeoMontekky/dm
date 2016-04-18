#pragma once

#include "literals.h"
#include "noncopyable.h"

#include <memory>

namespace dm
{

class CombinationGenerator : public NonCopyable
{
public:
   CombinationGenerator(long dimension);

   const LiteralType* GenerateFirst();
   const LiteralType* GenerateNext();

private:
   long m_dimension;
   std::unique_ptr<LiteralType[]> m_combination;
};

} // namespace dm
