#include "expression_evaluator.h"
#include "expression_utils.h"
#include "expressions.h"

#include <algorithm>
#include <cassert>

namespace dm
{

namespace
{
   

class ExpressionEvaluator
{
public:
   ExpressionEvaluator();
   
   // Returns information about whether current expression
   // has been fully evaluated to a certain other expression.
   void Evaluate(TExpressionPtr& expr);
   
private:
   void EvaluateOperation(OperationExpression& expression);

   // Following methods are called from EvaluateOperation, using pointer.
   void EvaluateNegation(OperationExpression& expression);
   void EvaluateConjunction(OperationExpression& expression);
   void EvaluateDisjunction(OperationExpression& expression);
   void EvaluateImplication(OperationExpression& expression);
   void EvaluateEquality(OperationExpression& expression);
   void EvaluatePlus(OperationExpression& expression);

private:
   // Following set of methods is intended to re-use common rules of
   // evaluation for associative/commutative operations.
   // If a method returns true it means it set m_evaluated_expression
   // member, so the calling side must return control up as soon as possible.
   bool RemoveAllIfLiteralExists(OperationExpression& expression, LiteralType literal);
   bool RemoveAllIfNegNotNegExists(OperationExpression& expression, LiteralType remaining_literal);
   void RemoveLiteralIfExists(OperationExpression& expression, LiteralType literal);
   void RemoveDuplicates(OperationExpression& expression);
   bool AbsorbDuplicates(OperationExpression& expression, LiteralType remaining_literal);
   void RemoveNegations(OperationExpression& expression, LiteralType eq_to_neg_literal);
   void DeMorganTransfromation(OperationExpression& expression);
   bool CanBeGroupedAsNegNotNeg(const OperationExpression& expression);

   // In-place normatlization for equality/plus
   static void InPlaceNormalization(OperationExpression& expression, long child_index);
   // In-place normatlization for implication
   static void InPlaceNormalization(OperationExpression& expression);

   // This method is used by implication evaluation.
   bool RemoveBeginningIfEqualToChild(OperationExpression& expression,
                                      long operands_between, bool include_child, 
                                      bool negated_child = false);

   // Utility
   static bool IsNegationEquivalent(const TExpressionPtr& expr);
   static bool CheckNegNotNeg(const TExpressionPtr& neg_expr, const TExpressionPtr& expr);
   static void ExtractFromUnderNegationEquivalent(TExpressionPtr& expr);

private:
   // Will be filled by new evaluated expression if the whole
   // operation expression was evaluated to some simple form.
   TExpressionPtr m_evaluated_expression;
};

ExpressionEvaluator::ExpressionEvaluator() : m_evaluated_expression()
{
}

void ExpressionEvaluator::Evaluate(TExpressionPtr& expr)
{
   if (expr->GetType() != ExpressionType::Operation)
   {
      return;
   }
   
   auto& expression = CastToOperation(expr);
   
   for (long index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      Evaluate(expression.GetChild(index));
   }
   
   EvaluateOperation(expression);
   
   if (m_evaluated_expression.get() != nullptr)
   {
      expr = std::move(m_evaluated_expression);
   } 
}

void ExpressionEvaluator::EvaluateOperation(OperationExpression& expression)
{
   using TEvaluateMetodPtr = void(ExpressionEvaluator::*)(OperationExpression& expression);

   static TEvaluateMetodPtr methods[] =
   {
      EvaluateNegation,
      EvaluateConjunction,
      EvaluateDisjunction,
      EvaluateImplication,
      EvaluateEquality,
      EvaluatePlus
   };

   auto method = methods[static_cast<long>(expression.GetOperation())];
   (this->*method)(expression);

   // If after evaluation the only operand is remained, then
   // current expression can be evaluated to this operand.
   if (expression.GetOperation() != OperationType::Negation && expression.GetChildCount() == 1)
   {
      // Evaluated expression shouln't be set already.
      assert(m_evaluated_expression.get() == nullptr);
      m_evaluated_expression = std::move(expression.GetChild(0));
   }
}

void ExpressionEvaluator::EvaluateNegation(OperationExpression& expression)
{
   assert(expression.GetChildCount() == 1);

   // We have the only rule:
   //    1. !!x  => x

   // The nested negation can be treated as negation equivalent.
   auto& child = expression.GetChild(0);
   if (IsNegationEquivalent(child))
   {
      ExtractFromUnderNegationEquivalent(child);
      m_evaluated_expression = std::move(child);
   }
}

void ExpressionEvaluator::EvaluateConjunction(OperationExpression& expression)
{
   assert(expression.GetChildCount() > 1);

   // We have following rules:
   //    1. ( x & 0) = 0
   //    2. ( x & 1) = x
   //    3. ( x & x) = x
   //    4. (!x & x) = 0

   // According to rule 1, evaluate expression to literal 0 if it exist.
   if (RemoveAllIfLiteralExists(expression, LiteralType::False))
   {
      return;
   }

   // According to rule 4, evaluate expression to literal 0
   // if there exists x and !x.
   if (RemoveAllIfNegNotNegExists(expression, LiteralType::False))
   {
      return;
   }

   // According to rule 2, remove literal 1 if exist.
   RemoveLiteralIfExists(expression, LiteralType::True);

   // According to rule 3, remove all duplicates.
   RemoveDuplicates(expression);
   
   // According to rule 4, evaluate expression to literal 0
   // if there exists !x and grouped subset that equals to x.
   if (CanBeGroupedAsNegNotNeg(expression))
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(LiteralType::False);
   }
}

void ExpressionEvaluator::EvaluateDisjunction(OperationExpression& expression)
{
   assert(expression.GetChildCount() > 1);

   // We have following rules:
   //    1. ( x | 1) = 1
   //    2. ( x | 0) = x
   //    3. ( x | x) = x
   //    4. (!x | x) = 1

   // According to rule 1, evaluate expression to literal 1 if it exist.
   if (RemoveAllIfLiteralExists(expression, LiteralType::True))
   {
      return;
   }

   // According to rule 4, evaluate expression to literal 1
   // if there exists x and !x.
   if (RemoveAllIfNegNotNegExists(expression, LiteralType::True))
   {
      return;
   }

   // According to rule 2, remove literal 0 if exist.
   RemoveLiteralIfExists(expression, LiteralType::False);

   // According to rule 3, remove all duplicates.
   RemoveDuplicates(expression);
   
   // According to rule 4, evaluate expression to literal 1
   // if there exists !x and grouped subset that equals to x.
   if (CanBeGroupedAsNegNotNeg(expression))
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(LiteralType::True);
   }
}

void ExpressionEvaluator::EvaluateImplication(OperationExpression& expression)
{
   assert(expression.GetChildCount() > 1);

   // As implication is not commutative/associative, we are able
   // to evaluate operation expression only from left to right.
   // This is why we may not reuse utility methods that are available
   // in other EvaluateXXX methods.

   // We have following rules:
   //    1. ( 1 ->  x)      =>  x
   //    2. ( 0 ->  x)      =>  1
   //    3. ( x ->  1)      =>  1
   //    4. (!x ->  0)      =>  x
   //    5. ( x ->  x)      =>  1
   //    6. (!x ->  x)      =>  x
   //    7. ( x -> !x)      => !x
   //    8. ( x ->  0 -> 0) =>  x
   //    9. ( x ->  y -> x) =>  x

   // According to rules 3 and 1, if there exist "1" operand,
   // we can remove all operands to the left, including this "1" operand.

   for (long index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      if (LiteralType::True == GetLiteral(expression.GetChild(index)))
      {
         expression.RemoveChildren(0, index + 1);
         InPlaceNormalization(expression);
         break;
      }
   }

   bool was_evaluated = true;

   do
   {
      while (expression.GetChildCount() > 1 && was_evaluated)
      {
         was_evaluated = false;

         // According to rules 2 and 1, we can remove subexpression (0 -> x)
         // if it is at the beginning of the expression.
         // According to rules 5 and 1, we can remove subexpression (x -> x)
         // if it is at the beginning of the expression
         if (LiteralType::False == GetLiteral(expression.GetChild(0)) ||
             IsEqual(expression.GetChild(0), expression.GetChild(1)))
         {
            expression.RemoveChildren(0, 2);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 4 perform transformation (!x -> 0) => x
         else if (IsNegationEquivalent(expression.GetChild(0)) &&
                  LiteralType::False == GetLiteral(expression.GetChild(1)))
         {
            ExtractFromUnderNegationEquivalent(expression.GetChild(0));
            expression.RemoveChild(1);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 6, we can remove first operand in case of (!x ->  x)
         // According to rule 7, we can remove first operand in case of ( x -> !x)
         else if (CheckNegNotNeg(expression.GetChild(0), expression.GetChild(1)) ||
                  CheckNegNotNeg(expression.GetChild(1), expression.GetChild(0)))
         {
            expression.RemoveChild(0);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 9, we can remove first two operands in case of (x -> y -> x)
         else if (expression.GetChildCount() > 2 && IsEqual(expression.GetChild(0), expression.GetChild(2)))
         {
            expression.RemoveChildren(0, 2);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
      } // while (expression.GetChildCount() > 1 && was_evaluated)

      // Other rules require at least 3 operands.
      if (expression.GetChildCount() < 3)
      {
         break;
      }

      // According to rule 8, we can remove two zero literals
      // if they stay one after another.
      for (long index = expression.GetChildCount() - 2; index >= 0; --index)
      {
         if (LiteralType::False == GetLiteral(expression.GetChild(index)) &&
             LiteralType::False == GetLiteral(expression.GetChild(index + 1)))
         {
            expression.RemoveChildren(index, index + 2);
            was_evaluated = true;
         }
      }

      // Now try to evaluate the expression, using grouping of some amount
      // of first operands and applying basic rules.

      // Expression (x1 -> ... -> xn -> (x1 -> ... -> xn) -> z)
      // can be grouped and the whole expression can be evaluated to (z),
      // using rule 5 and 1.
      if (RemoveBeginningIfEqualToChild(expression, 0, true))
      {
         was_evaluated = true;
      }

      // Expression (x1 -> .. -> xn -> !(x1 -> ... -> xn))
      // can be grouped and the whole expression can be evaluated to
      // !(x1 -> ... -> xn), using rule 6.
      if (RemoveBeginningIfEqualToChild(expression, 0, false, true))
      {
         was_evaluated = true;
      }
      
      // Expression (x1 -> .. -> xn -> y -> (x1 -> ... -> xn))
      // can be grouped and the whole expression can be evaluated to
      // (x1 -> ... -> xn), using rule 9.
      if (RemoveBeginningIfEqualToChild(expression, 1, false))
      {
         was_evaluated = true;
      }
   }
   while (was_evaluated);

   if (expression.GetChildCount() == 0)
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(LiteralType::True);
   }
}

void ExpressionEvaluator::EvaluateEquality(OperationExpression& expression)
{
   assert(expression.GetChildCount() > 1);

   // We have following rules:
   //    1. ( x =  1 ) => x
   //    2. (!x =  0 ) => x
   //    3. ( x =  x ) => 1
   //    4. (!x = !y ) => (x = y)

   // According to rule 1, remove literal 1.
   RemoveLiteralIfExists(expression, LiteralType::True);

   // According to rule 4 and 2, reduce even amount of negations
   // and reduce the only remaining negation (if exists) with literal 0.
   RemoveNegations(expression, LiteralType::False);

   // According to rule 3 and 1, remove duplicates
   // and assign leteral 1 if all operands were removed.
   AbsorbDuplicates(expression, LiteralType::True);
}

void ExpressionEvaluator::EvaluatePlus(OperationExpression& expression)
{
   assert(expression.GetChildCount() > 1);

   // We have following rules:
   //    1. ( x +  0 ) => x
   //    2. (!x +  1 ) => x
   //    3. ( x +  x ) => 0
   //    4. (!x + !y ) => (x + y)

   // According to rule 1, remove literal 0.
   RemoveLiteralIfExists(expression, LiteralType::False);

   // According to rule 4 and 2, reduce even amount of negations
   // and reduce the only remaining negation (if exists) with literal 1.
   RemoveNegations(expression, LiteralType::True);

   // According to rule 3 and 1, remove duplicates
   // and assign leteral 0 if all operands were removed.
   AbsorbDuplicates(expression, LiteralType::False);
}

bool ExpressionEvaluator::RemoveAllIfLiteralExists(
   OperationExpression& expression, LiteralType literal)
{
   auto& last_child = expression.GetChild(expression.GetChildCount() - 1);
   if (GetLiteral(last_child) == literal)
   {
      m_evaluated_expression = std::move(last_child);
      return true;
   }
   return false;
}

bool ExpressionEvaluator::RemoveAllIfNegNotNegExists(
   OperationExpression& expression, LiteralType remaining_literal)
{
   const long child_count = expression.GetChildCount();

   for (int i = expression.GetChildCount() - 1; i > 0; --i)
      for (int j = i - 1; j >= 0; --j)
         if (CheckNegNotNeg(expression.GetChild(i), expression.GetChild(j)) ||
             CheckNegNotNeg(expression.GetChild(j), expression.GetChild(i)))
         {
            m_evaluated_expression = std::make_unique<LiteralExpression>(remaining_literal);
            return true;
         }

   return false;
}

void ExpressionEvaluator::RemoveLiteralIfExists(
   OperationExpression& expression, LiteralType literal)
{
   const long child_count = expression.GetChildCount();
   if (literal == GetLiteral(expression.GetChild(child_count - 1)))
   {
      expression.RemoveChild(child_count - 1);
   }
}

void ExpressionEvaluator::RemoveDuplicates(OperationExpression& expression)
{
   long child_count = expression.GetChildCount();

   for (long i = 0; i < child_count - 1; ++i)
   {
      long j = i + 1;
      while (j < child_count)
      {
         if (IsEqual(expression.GetChild(i), expression.GetChild(j)))
         {
            expression.RemoveChild(j);
            --child_count;
            continue;
         }
         ++j;
      }
   }
}

bool ExpressionEvaluator::AbsorbDuplicates(
   OperationExpression& expression, LiteralType remaining_literal)
{
   long child_count = expression.GetChildCount();

   long i = 0;
   while (i < child_count - 1)
   {
      long j = i + 1;
      while (j < child_count)
      {
         if (IsEqual(expression.GetChild(i), expression.GetChild(j)))
         {
            expression.RemoveChild(j);
            expression.RemoveChild(i);
            j = i + 1;
            child_count -= 2;
            continue;
         }
         ++j;
      }
      ++i;
   }

   if (0 == child_count)
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(remaining_literal);
      return true;
   }

   return false;
}

void ExpressionEvaluator::RemoveNegations(OperationExpression& expression, LiteralType eq_to_neg_literal)
{
   bool is_negation_lonely = false;

   for (long index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      if (IsNegationEquivalent(expression.GetChild(index)))
      {
         ExtractFromUnderNegationEquivalent(expression.GetChild(index));
         InPlaceNormalization(expression, index);
         is_negation_lonely = !is_negation_lonely;
      }
   }

   if (is_negation_lonely)
   {
      const long child_count = expression.GetChildCount();
      if (GetLiteral(expression.GetChild(child_count - 1)) == eq_to_neg_literal)
      {
         expression.RemoveChild(child_count - 1);
      }
      else
      {
         expression.AddChild(std::make_unique<LiteralExpression>(eq_to_neg_literal));
      }
   }
}

void ExpressionEvaluator::DeMorganTransfromation(OperationExpression& expression)
{
   // De Morgan laws are following:
   //    1. (!x & !y) => !(x | y)
   //    1. (!x | !y) => !(x & y)
   
   auto operation = expression.GetOperation();

   assert(OperationType::Conjunction == operation ||
          OperationType::Disjunction == operation);

   long negation_count = 0;
   for (long index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      if (IsNegationEquivalent(expression.GetChild(index)))
      {
         ++negation_count;
      }
   }

   if (OperationType::Disjunction == expression.GetOperation())
   {
      // It is done to avoid the endless call of De Morgan's transormations.
      // In case of equal amount of negated/non-negated operands, increment
      // of negation_count will allow to call transofmation from disjunction
      // to conjunction, but reverse transormation will not be called.
      ++negation_count;
   }

   if (negation_count < (expression.GetChildCount() >> 1))
   {
      // Don't do De Morgan's transformation if amount of negated operands
      // less then non-negated ones.
      return;
   }

   operation = (OperationType::Conjunction == operation) ?
      OperationType::Disjunction : OperationType::Conjunction;
   expression.SetOperation(operation);

   for (long index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      auto& child_expr = expression.GetChild(index);
      const auto child_operation = GetOperation(child_expr);
      
      if (IsNegationEquivalent(child_expr))
      {
         ExtractFromUnderNegationEquivalent(child_expr);
         InPlaceNormalization(expression, index);
      }
      else
      {
         // Add negation. Let's use following rules to make it presentable.
         //    1. ( x -> 0), in case of implication
         //    2. ( x  = 0), in case of equality
         //    3. ( x  + 1), in case of plus
         //    4. (!x),      otherwise
         
         if (OperationType::Implication == child_operation ||
             OperationType::Equality == child_operation)
         {
            CastToOperation(child_expr).AddChild(std::make_unique<LiteralExpression>(LiteralType::False));
         }
         else if (OperationType::Plus == child_operation)
         {
            CastToOperation(child_expr).AddChild(std::make_unique<LiteralExpression>(LiteralType::True));
         }
         else
         {
            child_expr = std::make_unique<OperationExpression>(std::move(child_expr));
         }
      }
   }

   // TODO: Add negation to the whole operation
}

bool ExpressionEvaluator::CanBeGroupedAsNegNotNeg(const OperationExpression& expression)
{
   // Checks whether there exists some negated child, that (under negation)
   // contains child operands, that all have equivalents between
   // children of the current operation.
   
   // Examples:
   //    !(x & y) & x & y & z
   //    ((x | y)->0) | x | y | z
   
   const long child_count = expression.GetChildCount();
   
   if (child_count < 3)
   {
      // Two operarands cannot be grouped, because it means that operation under
      // negation must have a single child. The last fact is possible for the only
      // expression (!x * x), but this expression were to be already reduced before.
      return false;
   }
   
   for (long index = 0, i = 0, j = 0; index < child_count; ++index)
   {
      const auto& child_expr = expression.GetChild(index);
      
      // If current child is a negation equivalent and it encapsulates the same operation
      // as current one, then we need to check whether other children are the same as
      // children of the negated child.
      
      // Checking of the fact, that amount of child operands of the 'child' is less
      // than 3, needs additional clarification. Let's explain this on examples.
      
      // Example 1:
      //    (x = y = 0) = x = y
      // Before evaluation start it had to be normalized to:
      //     x = y = 0 = x = y
      // So it's impossible situation.

      // Example 2:
      //    (x -> y -> 0) = x = y
      // This expression could not be normalized before evaluation, so it's possible. But it
      // can't be grouped because removing of negation from bracket's content gives (x -> y),
      // and this operation isn't equality.

      // Example 3:
      //   ((x = y) -> 0) = x = y
      // This expression could not be normalized, so is possible like to the previous one.
      // But after negation removing it will give (x = y), so remained operarands of the
      // female operation are really can be grouped to have the same set.

      // Let's make a line. The grouping is possible only if operands' amount of the
      // negation equivalent is less, then 3. This condition is OK for binary operations,
      // and also fits raw negation operation, that is ought to have a single child operand.
      
      if (IsNegationEquivalent(child_expr))
      {
         // We can do casting without checking return value of GetType becuase IsNegationEquivalent
         // will return false for a non-operation expression.
         const auto& child_expression = CastToOperation(child_expr);

         if (child_expression.GetChildCount() < 3 &&
             GetOperation(child_expression.GetChild(0)) == expression.GetOperation())
         {
            const auto& child_to_check = CastToOperation(child_expression.GetChild(0));
            const long child_to_check_count = child_to_check.GetChildCount();

            for (i = 0; i < child_to_check_count; ++i)
            {
               for (j = 0; j < child_count; ++j)
               {
                  if (j != index && IsEqual(expression.GetChild(j), child_to_check.GetChild(i)))
                  {
                     // Break if equal child is found.
                     break;
                  }
               }

               // If counter "j" reached the end of the cycle, then child expression that
               // equals to the children_to_check.GetChild(i) was not found.
               if (child_count == j)
               {
                  break;
               }
            }

            // If counter "i" reached the end of the cycle then there exists equivalent
            // for each child under the negation.
            if (child_to_check_count == i)
            {
               return true;
            }
         }
      }
   }

   return false;
}

void ExpressionEvaluator::InPlaceNormalization(OperationExpression& expression, long child_index)
{
   if (expression.GetOperation() == GetOperation(expression.GetChild(child_index)))
   {
      TExpressionPtrVector moved_children;
      MoveChildExpressions(moved_children, expression.GetChild(child_index));
      expression.RemoveChild(child_index);
      expression.InsertChildren(child_index, std::move(moved_children));
   }
}

void ExpressionEvaluator::InPlaceNormalization(OperationExpression& expression)
{
   // The method is able to be called by implication evaluation even
   // in case of empty children vector (it is done for logic simplification),
   // so we must check emptiness manually.
   if (expression.GetChildCount() > 0)
   {
      InPlaceNormalization(expression, 0);
   }
}

bool ExpressionEvaluator::RemoveBeginningIfEqualToChild(
   OperationExpression& expression, long operands_between, bool include_child, bool negated_child)
{
   for (long index = expression.GetChildCount() - 1; index > 1 + operands_between; --index)
   {
      auto& child_expr = expression.GetChild(index);
      if (child_expr->GetType() != ExpressionType::Operation)
      {
         continue;
      }

      auto& child_expression = CastToOperation(child_expr);
      const auto child_operation = child_expression.GetOperation();
      const long amount_to_check = index - operands_between;
      long implication_correction = 0;
      
      OperationExpression* child_to_check = nullptr;
      if (negated_child)
      {
         if (OperationType::Negation == child_operation &&
             OperationType::Implication == GetOperation(child_expression.GetChild(0)))
         {
            child_to_check = &CastToOperation(child_expression.GetChild(0));
         }
         else if (OperationType::Implication == child_operation &&
                  LiteralType::False == GetLiteral(
                     child_expression.GetChild(child_expression.GetChildCount() - 1)))
         {
            child_to_check = &child_expression;
            implication_correction = 1;
         }
      }
      else if (OperationType::Implication == child_operation)
      {
         child_to_check = &child_expression;
      }
      
      if (child_to_check != nullptr &&
          child_to_check->GetChildCount() - implication_correction == amount_to_check &&
          child_to_check->AreFirstChildrenEqual(expression, amount_to_check))
      {
         const long right_bound = include_child ? (index + 1) : index;
         expression.RemoveChildren(0, right_bound);
         InPlaceNormalization(expression);
         return true;
      }
   }

   return false;
}

bool ExpressionEvaluator::IsNegationEquivalent(const TExpressionPtr& expr)
{
   // We have following rules of negation equivalents:
   //    1. !x
   //    2.  x -> 0
   //    3.  x = 0
   //    4.  x + 1

   if (expr->GetType() != ExpressionType::Operation)
   {
      return false;
   }

   const auto& expression = CastToOperation(expr);
   const auto operation = expression.GetOperation();
   const long child_count = expression.GetChildCount();

   return (OperationType::Negation == operation) ||
          (OperationType::Implication == operation &&
             (LiteralType::False == GetLiteral(expression.GetChild(child_count - 1)))) ||
          (OperationType::Equality == operation &&
             (LiteralType::False == GetLiteral(expression.GetChild(child_count - 1)))) ||
          (OperationType::Plus == operation &&
             (LiteralType::True == GetLiteral(expression.GetChild(child_count - 1))));
}

bool ExpressionEvaluator::CheckNegNotNeg(
   const TExpressionPtr& neg_expr, const TExpressionPtr& expr)
{
   if (!IsNegationEquivalent(neg_expr))
   {
      return false;
   }

   // We can cast without checking GetType, because IsNegationEquivalent
   // will return false in case of non-operation expression.
   const auto& neg_expression = CastToOperation(neg_expr);

   switch (neg_expression.GetOperation())
   {
      case OperationType::Negation:
         return (IsEqual(neg_expression.GetChild(0), expr));
         
      case OperationType::Implication:
      case OperationType::Equality:
      case OperationType::Plus:
         break;
         
      default:
         // Other operations can't form negation equivalent.
         return false;
   }
   
   // We have 2 cases that must be checked:
   //    1. Simple case, e.g. (x + 1) and x.
   //    2. Complex case, e.g. (x + y + 1) and (x + y)

   // 1. Simple case.
   if (neg_expression.GetChildCount() == 2 &&
       IsEqual(neg_expression.GetChild(0), expr))
   {
      return true;
   }

   if (expr->GetType() != ExpressionType::Operation)
   {
      return false;
   }

   const auto& expression = CastToOperation(expr);

   // 2. Complex case.
   return (neg_expression.GetOperation() == expression.GetOperation() &&
           neg_expression.GetChildCount() - 1 == expression.GetChildCount() &&
           neg_expression.AreFirstChildrenEqual(expression, expression.GetChildCount()));
}

void ExpressionEvaluator::ExtractFromUnderNegationEquivalent(TExpressionPtr& expr)
{
   assert(IsNegationEquivalent(expr));

   // We can cast without checking GetType, because IsNegationEquivalent
   // will return false in case of non-operation expression.
   auto& expression = CastToOperation(expr);
   
   // It will be performed for following operations:
   //    - OperationType::Implication
   //    - OperationType::Equality
   //    - OperationType::Plus
   if (expression.GetChildCount() > 1)
   {
      expression.RemoveChild(expression.GetChildCount() - 1);
   }
   
   // If will be performed for previous operations if the only child is remained
   // after literal removing.
   // Also the condition will be triggered for OperationType::Negation.
   if (expression.GetChildCount() == 1)
   {
      expr = std::move(expression.GetChild(0));
   }
}

} // namespace

void EvaluateExpression(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   ExpressionEvaluator evaluator;
   evaluator.Evaluate(expr);
}

} // namespace dm
