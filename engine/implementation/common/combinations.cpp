#include "combinations.h"

#include <algorithm>

namespace dm
{

CombinationGenerator::CombinationGenerator(long dimension) :
   m_dimension(dimension), m_combination()
{
}

const LiteralType* CombinationGenerator::GenerateFirst()
{
   // Array has got additional element to hold carry-flag.
   // It will be stored at element with index 0.
   const auto count = m_dimension + 1;

   if (m_combination.get() == nullptr)
   {
      m_combination = std::make_unique<LiteralType[]>(count);
   }

   std::fill_n(m_combination.get(), count, LiteralType::False);

   return m_combination.get() + 1;
}

const LiteralType* CombinationGenerator::GenerateNext()
{
   // Generate next combination using algorithm of binary increment,
   // implying combination array elements as bits in some binary number representation.

   auto i = m_dimension;
   for (; i > 0 && LiteralType::True == m_combination[i]; m_combination[i--] = LiteralType::False);
   m_combination[i] = LiteralType::True;
   
   // Check of carry-flag
   if (LiteralType::True == m_combination[0])
   {
      return nullptr;
   }

   return m_combination.get() + 1;
}

} // namespace dm
