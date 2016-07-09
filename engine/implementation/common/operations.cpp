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

LiteralType PerformOperation(OperationType operation, const LiteralType values[], long amount)
{
   assert(OperationType::None != operation);

   auto result = LiteralType::None;

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
         nullptr,     // OperationType::Negation   
         Conjunction, // OperationType::Conjunction
         Disjunction, // OperationType::Disjunction
         Implication, // OperationType::Implication
         Equality,    // OperationType::Equality   
         Plus         // OperationType::Plus       
      };

      auto func = functions[static_cast<int>(operation)];
      result = values[0];
      for (auto i = 1L; i < amount; ++i)
      {
         result = func(result, values[i]);
      }
   }

   return result;
}

bool AreOperandsMovable(OperationType operation)
{
   assert(OperationType::None != operation);

   static bool movability_flags[]
   {
      false, // OperationType::Negation
      true,  // OperationType::Conjunction
      true,  // OperationType::Disjunction
      false, // OperationType::Implication
      true,  // OperationType::Equality
      true   // OperationType::Plus
   };

   return movability_flags[static_cast<int>(operation)];
}

bool AreOperationsMutuallyReverse(OperationType operation1, OperationType operation2)
{
   assert(OperationType::None != operation1);
   assert(OperationType::None != operation2);

   return (OperationType::Equality == operation1 && OperationType::Plus == operation2) ||
          (OperationType::Equality == operation2 && OperationType::Plus == operation1);
}

OperationType GetOppositeOperation(OperationType operation)
{
   assert(OperationType::None != operation);

   static OperationType opposite_operations[] =
   {
      OperationType::None,        // OperationType::Negation   
      OperationType::Disjunction, // OperationType::Conjunction
      OperationType::Conjunction, // OperationType::Disjunction
      OperationType::None,        // OperationType::Implication
      OperationType::None,        // OperationType::Equality   
      OperationType::None,        // OperationType::Plus       
   };

   return opposite_operations[static_cast<int>(operation)];
}

const char* OperationTypeToString(OperationType operation)
{
   assert(OperationType::None != operation);

   static const char* operation_tokens[] =
   {
      g_token_negation,    // OperationType::Negation   
      g_token_conjunction, // OperationType::Conjunction
      g_token_disjunction, // OperationType::Disjunction
      g_token_implication, // OperationType::Implication
      g_token_equality,    // OperationType::Equality   
      g_token_plus         // OperationType::Plus       
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
