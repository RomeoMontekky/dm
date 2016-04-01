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

bool IsOperationCommutative(OperationType operation);
bool IsOperationAssociative(OperationType operation);

LiteralType PerformOperation(OperationType operation, const LiteralType values[], long amount);

const char* OperationTypeToString(OperationType operation);
OperationType StartsWithOperation(const StringPtrLen& str);

}; // namespace dm
