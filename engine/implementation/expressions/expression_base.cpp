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

Expression::Expression(const Expression& rhs) = default;

} // namespace dm
