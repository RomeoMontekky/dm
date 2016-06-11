#pragma once

#include "literals.h"
#include "string_utils.h"

namespace dm
{

// Operations are in order of their priorities

enum class OperationType
{
   None = -1,
   Negation,
   Conjunction,
   Disjunction,
   Implication,
   Equality,
   Plus
};

LiteralType PerformOperation(OperationType operation, const LiteralType values[], long amount);

// Actually it means that operation is commutative and associative.
bool AreOperandsMovable(OperationType operation);

bool AreOperationsMutuallyReverse(OperationType operation1, OperationType operation2);

OperationType GetOppositeOperation(OperationType operation);

const char* OperationTypeToString(OperationType operation);
OperationType StartsWithOperation(const StringPtrLen& str);

} // namespace dm
