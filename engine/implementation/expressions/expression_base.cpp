#include "expression_base.h"

namespace dm
{

///////////// IStringable //////////////

IStringable::IStringable()
{
}

IStringable::~IStringable()
{
}

///////////// Expression //////////////

Expression::Expression()
{
}

Expression::Expression(const Expression& rhs)
{
}

bool Expression::IsEqualTo(const Expression& rhs)
{
   if (GetType() != rhs.GetType())
   {
      return false;
   }
   return IsEqualToTheSameType(rhs);
}

bool IsEqual(const TExpressionPtr& left, const TExpressionPtr& right)
{
   return left->IsEqualTo(*right.get());
}

} // namespace dm
