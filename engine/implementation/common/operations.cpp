#include "operations.h"

#include <cassert>

namespace dm
{

namespace
{

const char g_token_negation[]    = "!";
const char g_token_disjunction[] = "|";
const char g_token_conjunction[] = "&";
const char g_token_implication[] = "->";
const char g_token_equality[]    = "=";
const char g_token_plus[]        = "+";

LiteralType Negation(LiteralType value)
{
   assert(LiteralType::None != value);

   return (LiteralType::True == value) ?
      LiteralType::False : LiteralType::True;
}

LiteralType Conjunction(LiteralType value1, LiteralType value2)
{
   assert(LiteralType::None != value1);
   assert(LiteralType::None != value2);

   return (LiteralType::True == value1 && LiteralType::True == value2) ?
      LiteralType::True : LiteralType::False;
}

LiteralType Disjunction(LiteralType value1, LiteralType value2)
{
   assert(LiteralType::None != value1);
   assert(LiteralType::None != value2);

   return (LiteralType::True == value1 || LiteralType::True == value2) ?
      LiteralType::True : LiteralType::False;
}

LiteralType Implication(LiteralType value1, LiteralType value2)
{
   assert(LiteralType::None != value1);
   assert(LiteralType::None != value2);

   return (LiteralType::False == value1 || LiteralType::True == value2) ?
      LiteralType::True : LiteralType::False;
}

LiteralType Equality(LiteralType value1, LiteralType value2)
{
   assert(LiteralType::None != value1);
   assert(LiteralType::None != value2);

   return (value1 == value2) ?
      LiteralType::True : LiteralType::False;
}

LiteralType Plus(LiteralType value1, LiteralType value2)
{
   assert(LiteralType::None != value1);
   assert(LiteralType::None != value2);

   return (value1 != value2) ? 
      LiteralType::True : LiteralType::False;
}

} // namespace

bool IsOperationCommutative(OperationType operation)
{
   // Each time you add new operation to OperationType
   // check the condition in IsOperationCommutative.
   //
   // After that you can add new operation to assert
   // condition manully. 
   //
   // Such an approach will guarantee that new operation
   // will not be forgotten to be checked for commutativity.

   assert
   (
      OperationType::Negation == operation ||
      OperationType::Conjunction == operation ||
      OperationType::Disjunction == operation ||
      OperationType::Implication == operation ||
      OperationType::Equality == operation ||
      OperationType::Plus == operation
   );

   return (// OperationType::Negation == operation is not commutative
              OperationType::Conjunction == operation ||
              OperationType::Disjunction == operation ||
           // OperationType::Implication is not commutative
              OperationType::Equality  == operation ||
              OperationType::Plus  == operation);

}

bool IsOperationAssociative(OperationType operation)
{
   // Each time you add new operation to OperationType
   // check the condition in IsOperationAssociative.
   //
   // After that you can add new operation to assert
   // condition manully. 
   //
   // Such an approach will guarantee that new operation
   // will not be forgotten to be checked for associativity.

   assert
   (
      OperationType::Negation == operation ||
      OperationType::Conjunction == operation ||
      OperationType::Disjunction == operation ||
      OperationType::Implication == operation ||
      OperationType::Equality == operation ||
      OperationType::Plus == operation
   );

   return (// OperationType::Negation == operation is not associative
              OperationType::Conjunction == operation ||
              OperationType::Disjunction == operation ||
           // OperationType::Implication is not associative
              OperationType::Equality  == operation ||
              OperationType::Plus  == operation);
}

LiteralType PerformOperation(OperationType operation, const LiteralType values[], long amount)
{
   LiteralType result = LiteralType::None;
   
   if (OperationType::Negation == operation)
   {
      assert(1 == amount);
      result = Negation(values[0]);
   }
   else
   {
      assert(amount > 1);

      using TOperationFunctionPtr = LiteralType(*)(LiteralType, LiteralType);

      static TOperationFunctionPtr functions[] =
      {
         nullptr,
         Conjunction,
         Disjunction,
         Implication,
         Equality,
         Plus
      };

      auto func = functions[static_cast<int>(operation)];
      result = values[0];
      for (long i = 1; i < amount; ++i)
      {
         result = func(result, values[i]);
      }
   }

   return result;
}

const char* OperationTypeToString(OperationType operation)
{
   static const char* operation_tokens[] =
   {
      g_token_negation,
      g_token_conjunction,
      g_token_disjunction,
      g_token_implication,
      g_token_equality,
      g_token_plus
   };

   return operation_tokens[static_cast<int>(operation)];
}

OperationType StartsWithOperation(const StringPtrLen& str)
{
   OperationType operation = OperationType::None;

   if (str.StartsWith(g_token_negation))
   {
      operation = OperationType::Negation;
   }
   else if (str.StartsWith(g_token_conjunction))
   {
      operation = OperationType::Conjunction;
   }
   else if (str.StartsWith(g_token_disjunction))
   {
      operation = OperationType::Disjunction;
   }
   else if (str.StartsWith(g_token_implication))
   {
      operation = OperationType::Implication;
   }
   else if (str.StartsWith(g_token_equality))
   {
      operation = OperationType::Equality;
   }
   else if (str.StartsWith(g_token_plus))
   {
      operation = OperationType::Plus;
   }

   return operation;
}

} // namespace dm
