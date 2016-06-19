#include "expression_evaluator.h"
#include "expression_utils.h"
#include "expressions.h"

#include "../common/local_array.h"

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
   bool Evaluate(TExpressionPtr& expr);
   
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
   bool MakeNegationRepresentative(OperationExpression& expression);
   bool RemoveAllIfLiteralExists(OperationExpression& expression, LiteralType literal);
   bool RemoveAllIfNegNotNegExists(OperationExpression& expression, LiteralType remaining_literal);
   void RemoveLiteralIfExists(OperationExpression& expression, LiteralType literal);
   void RemoveDuplicates(OperationExpression& expression);
   bool AbsorbDuplicates(OperationExpression& expression, LiteralType remaining_literal);
   void RemoveNegations(OperationExpression& expression, LiteralType eq_to_neg_literal);
   bool CanBeGroupedAsNegNotNeg(const OperationExpression& expression);
   void ApplyDeMorganLawsForChildren(OperationExpression& expression);
   bool ApplyDeMorganLawsForOperation(OperationExpression& expression);

   // Appies absorption/gluing laws while it is possible.
   void ApplyAbsorptionGluingLaws(OperationExpression& expression, LiteralType remaining_literal);
   // Single absorption/gluing laws applying. If any rule was applied, true is returned.
   bool ApplyAbsorptionGluingLawsOnce(OperationExpression& expression);

   static void InPlaceSimplification(OperationExpression& expression, long child_index);

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
   static bool IsNegationEquivalent(const OperationExpression& expression);
   static bool IsShortNegationEquivalent(const TExpressionPtr& expr);
   static bool IsShortNegationEquivalent(const OperationExpression& expression);

   static bool IsShortNegationEquivalentWithChildOperation(
      const TExpressionPtr& expr, OperationType operation);
   static bool IsShortNegationEquivalentWithChildOperation(
      const OperationExpression& expression, OperationType operation);

   static bool CheckNegNotNeg(const TExpressionPtr& neg_expr, const TExpressionPtr& expr);
   static void ExtractFromUnderNegationEquivalent(TExpressionPtr& expr);
   static void CoverWithNegationEquivalent(TExpressionPtr& expr);
   static void RevertNegations(TExpressionPtr& expr);

   static bool IsEqual(const TExpressionPtr& left, const TExpressionPtr& right);
   static bool IsEqualUpToMutuallyReverseOperations(
      const OperationExpression& left, const OperationExpression& right);

   static bool IsEqualToAnyChild(
      const TExpressionPtr& expr, const OperationExpression& expression);

   static bool AreFirstChildrenEqual(const OperationExpression& left,
                                     const OperationExpression& right, long amount);

   // Checks whether first enclosing_amount children of enclosing operation are included
   // in first enveloping_amount children of enveloping operation.
   // If skip_index is not -1, then skip_index-th child is to be skipped during
   // analyzing enveloping children.
   static bool AreFirstChildrenIncludedInFirstChildren(
      const OperationExpression& enclosing, long enclosing_amount,
      const OperationExpression& enveloping, long enveloping_amount, long skip_index = -1);

   // Checks whether children of expression1 differs from children of expression2 by a single
   // item. If it is true, diff_index1 and diff_index2 are filled with differed indexes.
   static bool AreChildrenDifferByOne(const OperationExpression& expression1,
                                      const OperationExpression& expression2,
                                      long& diff_index1, long& diff_index2);

private:
   // Will be filled by new evaluated expression if the whole
   // operation expression was evaluated to some simple form.
   TExpressionPtr m_evaluated_expression;
};

ExpressionEvaluator::ExpressionEvaluator() : m_evaluated_expression()
{
}

bool ExpressionEvaluator::Evaluate(TExpressionPtr& expr)
{
   if (expr->GetType() != ExpressionType::Operation)
   {
      return false;
   }

   auto& expression = CastToOperation(expr);
   const auto operation = expression.GetOperation();
   const auto are_operands_movable = AreOperandsMovable(operation);

   for (auto index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      auto& child = expression.GetChild(index);

      if (Evaluate(child) &&
          OperationType::Negation != operation && GetOperation(child) == operation)
      {
         // In-place simplification
         if (are_operands_movable)
         {
            InPlaceSimplification(expression, index);
         }

         // In-place normalization
         if ((are_operands_movable || 0 == index))
         {
            MoveChildExpressionsUp(expression, index);
         }
      }
   }

   EvaluateOperation(expression);

   if (m_evaluated_expression.get() != nullptr)
   {
      expr = std::move(m_evaluated_expression);
      return true;
   }

   return false;
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

   auto method = methods[static_cast<int>(expression.GetOperation())];
   (this->*method)(expression);

   // If the only operand is remained after the evaluation, then the whole operation
   // is to be evaluated to this operand.
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
      return;
   }

   MakeNegationRepresentative(expression);
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

   // According to rule 2, remove literal 1 if exist.
   RemoveLiteralIfExists(expression, LiteralType::True);
   
   // Evaluation of child negations, according to De Morgan's laws.
   ApplyDeMorganLawsForChildren(expression);

   // Apply absorption/gluing rules while it is possible.
   ApplyAbsorptionGluingLaws(expression, LiteralType::False);

   // According to rule 4, evaluate expression to literal 0
   // if there exists x and !x.
   if (RemoveAllIfNegNotNegExists(expression, LiteralType::False))
   {
      return;
   }

   // According to rule 3, remove all duplicates.
   RemoveDuplicates(expression);
   
   // According to rule 4, evaluate expression to literal 0
   // if there exists !x and grouped subset that equals to x.
   if (CanBeGroupedAsNegNotNeg(expression))
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(LiteralType::False);
      return;
   }

   // Transformation of the whole operation, according to De Morgan's laws.
   ApplyDeMorganLawsForOperation(expression);
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

   // According to rule 2, remove literal 0 if exist.
   RemoveLiteralIfExists(expression, LiteralType::False);

   // Evaluation of child negations, according to De Morgan's laws.
   ApplyDeMorganLawsForChildren(expression);

   // Apply absorption/gluing rules while it is possible.
   ApplyAbsorptionGluingLaws(expression, LiteralType::False);

   // According to rule 4, evaluate expression to literal 1
   // if there exists x and !x.
   if (RemoveAllIfNegNotNegExists(expression, LiteralType::True))
   {
      return;
   }

   // According to rule 3, remove all duplicates.
   RemoveDuplicates(expression);
   
   // According to rule 4, evaluate expression to literal 1
   // if there exists !x and grouped subset that equals to x.
   if (CanBeGroupedAsNegNotNeg(expression))
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(LiteralType::True);
      return;
   }

   // Transformation of the whole operation, according to De Morgan's laws.
   ApplyDeMorganLawsForOperation(expression);
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

   for (auto index = expression.GetChildCount() - 1; index >= 0; --index)
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
      for (auto index = expression.GetChildCount() - 2; index >= 0; --index)
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
   
   MakeNegationRepresentative(expression);
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
   if (AbsorbDuplicates(expression, LiteralType::True))
   {
      return;
   }
   
   MakeNegationRepresentative(expression);
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
   if (AbsorbDuplicates(expression, LiteralType::False))
   {
      return;
   }

   MakeNegationRepresentative(expression);
}

bool ExpressionEvaluator::MakeNegationRepresentative(OperationExpression& expression)
{
   if (IsShortNegationEquivalent(expression))
   {
      auto& child = expression.GetChild(0);

      // It can't be negation equivalent, since otherwise it would have
      // been evaluated before.
      assert(!IsNegationEquivalent(child));

      if (OperationType::Negation != expression.GetOperation() ||
          OperationType::None != GetOperation(child))
      {
         // Make negation representaive if it is necessary.
         CoverWithNegationEquivalent(child);
         m_evaluated_expression = std::move(child);
         return true;
      }
   }

   return false;
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
   for (int i = expression.GetChildCount() - 1; i > 0; --i)
   {
      for (int j = i - 1; j >= 0; --j)
      {
         if (CheckNegNotNeg(expression.GetChild(i), expression.GetChild(j)) ||
             CheckNegNotNeg(expression.GetChild(j), expression.GetChild(i)))
         {
            m_evaluated_expression = std::make_unique<LiteralExpression>(remaining_literal);
            return true;
         }
      }
   }

   return false;
}

void ExpressionEvaluator::RemoveLiteralIfExists(
   OperationExpression& expression, LiteralType literal)
{
   const auto last_index = expression.GetChildCount() - 1;
   if (literal == GetLiteral(expression.GetChild(last_index)))
   {
      expression.RemoveChild(last_index);
   }
}

void ExpressionEvaluator::RemoveDuplicates(OperationExpression& expression)
{
   for (auto i = expression.GetChildCount() - 1; i > 0; --i)
   {
      for (auto j = i - 1; j >= 0; --j)
      {
         if (IsEqual(expression.GetChild(i), expression.GetChild(j)))
         {
            expression.RemoveChild(i);
            break;
         }
      }
   }
}

bool ExpressionEvaluator::AbsorbDuplicates(
   OperationExpression& expression, LiteralType remaining_literal)
{
   for (auto i = expression.GetChildCount() - 1; i > 0; --i)
   {
      for (auto j = i - 1; j >= 0; --j)
      {
         if (IsEqual(expression.GetChild(i), expression.GetChild(j)))
         {
            expression.RemoveChild(i--);
            expression.RemoveChild(j);
            break;
         }
      }
   }

   if (0 == expression.GetChildCount())
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(remaining_literal);
      return true;
   }

   return false;
}

void ExpressionEvaluator::RemoveNegations(OperationExpression& expression, LiteralType eq_to_neg_literal)
{
   bool is_negation_lonely = false;

   for (auto index = expression.GetChildCount() - 1; index >= 0; --index)
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
      const auto last_index = expression.GetChildCount() - 1;
      if (GetLiteral(expression.GetChild(last_index)) == eq_to_neg_literal)
      {
         expression.RemoveChild(last_index);
      }
      else
      {
         expression.AddChild(std::make_unique<LiteralExpression>(eq_to_neg_literal));
      }
   }
}

bool ExpressionEvaluator::CanBeGroupedAsNegNotNeg(const OperationExpression& expression)
{
   // Checks whether there exists some negated child, that (under negation) contains child
   // operands, that all have equivalents between children of the current operation.

   // Examples:
   //    !(x & y) & x & y & z
   //    ((x | y)->0) | x | y | z

   // Additional variant of grouping is using De Morgan's law.

   // Example:
   //    ((x & y) | !x | !y)  =/ De Morgan /=> (!(!x | !y) | !x | !y) = 1

   const auto child_count = expression.GetChildCount();
   if (child_count < 3)
   {
      // Two operarands cannot be grouped, because it means that operation under
      // negation must have a single child. The last fact is possible for the only
      // expression (!x * x), but this expression were to be already reduced before.
      return false;
   }

   const auto opposite_operation = GetOppositeOperation(expression.GetOperation());
   assert(OperationType::None != opposite_operation);

   for (auto index = child_count -1; index >= 0; --index)
   {
      const auto& child = expression.GetChild(index);
      
      // If current child is a negation equivalent and it encapsulates the same operation
      // as current one, then we need to check whether other children are the same as
      // children of the negated child.
      if (IsShortNegationEquivalentWithChildOperation(child, expression.GetOperation()))
      {
         const auto& child_to_check = CastToOperation(CastToOperation(child).GetChild(0));
         if (AreFirstChildrenIncludedInFirstChildren(
                child_to_check, child_to_check.GetChildCount(),
                expression, expression.GetChildCount(), index))
         {
            return true;
         }
      }
      // If child operation is opposite, try to group after reverting children negations.
      else if (GetOperation(child) == opposite_operation)
      {
         auto cloned_child = child->Clone();
         RevertNegations(cloned_child);
         const auto& cloned_child_expression = CastToOperation(cloned_child);
         if (AreFirstChildrenIncludedInFirstChildren(
                cloned_child_expression, cloned_child_expression.GetChildCount(),
                expression, expression.GetChildCount(), index))
         {
            return true;
         }
      }
   }

   return false;
}

void ExpressionEvaluator::ApplyDeMorganLawsForChildren(OperationExpression& expression)
{
   // Method uses De Morgan's laws for each child negation of the operation.
   //    1. !(x & y) => !x | !y
   //    2. !(x | y) => !x & !y
   
   const auto opposite_operation = GetOppositeOperation(expression.GetOperation());
   assert(OperationType::None != opposite_operation);

   for (auto index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      auto& child = expression.GetChild(index);
      if (IsShortNegationEquivalentWithChildOperation(child, opposite_operation))
      {
          ExtractFromUnderNegationEquivalent(child);
          RevertNegations(child);
          MoveChildExpressionsUp(expression, index);
      }
   }
}

bool ExpressionEvaluator::ApplyDeMorganLawsForOperation(OperationExpression& expression)
{
   // Method uses De Morgan's laws for the whole operation.
   //    1. (!x & !y) => !(x | y)
   //    2. (!x | !y) => !(x & y)

   // It is possible to have the only child in case of removing other
   // operarands on the previous phases of evaluation. Let's return if so.
   if (expression.GetChildCount() < 2)
   {
      return false;
   }

   const auto child_count = expression.GetChildCount();
   const auto opposite_operation = GetOppositeOperation(expression.GetOperation());
   assert(OperationType::None != opposite_operation);
             
   auto negation_count = 0L;
   for (auto index = child_count - 1; index >= 0; --index)
   {
      if (IsNegationEquivalent(expression.GetChild(index)))
      {
         ++negation_count;
      }
   }

   negation_count <<= 1;

   if ((OperationType::Conjunction == opposite_operation && negation_count <  child_count) ||
       (OperationType::Disjunction == opposite_operation && negation_count <= child_count))
   {
      // Don't do transformation if amount of negated operands less (or equal) then non-negated ones.
      
      // As conjunction is transformed to disjunction and vise versa, we need to decide what to do
      // if amounts of negations and non-negations are equal to stabilize the algorithm. Evidently
      // one of these operations should be picked out as "preferable". Let it be conjunction.
      // So if amounts of negations and non-negations are equal, skip transormation if operation is
      // conjunction and don't skip otherwise.
      
      return false;
   }
   
   // Re-create expression in order to set another operation and obtain TExpressionPtr type as result.
   TExpressionPtrVector moved_expressions;
   MoveChildExpressions(moved_expressions, expression);
   TExpressionPtr new_expr = std::make_unique<OperationExpression>(opposite_operation, std::move(moved_expressions));
   
   RevertNegations(new_expr);

   // Add negation to new expression according to the rule.
   m_evaluated_expression = std::make_unique<OperationExpression>(std::move(new_expr));
   
   return true;
}

void ExpressionEvaluator::ApplyAbsorptionGluingLaws(
   OperationExpression& expression, LiteralType remaining_literal)
{
   while (ApplyAbsorptionGluingLawsOnce(expression));
}

bool ExpressionEvaluator::ApplyAbsorptionGluingLawsOnce(OperationExpression& expression)
{
   // Absorptions laws are:
   //    1. (x & y) | x => x
   //    2. (x | y) & x => x

   // Two sub-cases are available for both rules. Let's consider them on the first rule.
   //    1. simple case: x is not conjunction:
   //          (x & y) | y => x
   //    2. complex case: x is conjunction (u1 & ... & un)
   //          (u1 & ... & un & y) | (u1 & .. & un) => (u1 & ... & un)

   // Gluing laws are:
   //    1. (x & y) | (x & !y) = x
   //    2. (x | y) & (x | !y) = x

   auto is_modified = false;

   const auto opposite_operation = GetOppositeOperation(expression.GetOperation());
   assert(OperationType::None != opposite_operation);

   const auto last_index = expression.GetChildCount() - 1;
   for (auto index1 = last_index ; index1 > 0; --index1)
   {
      auto& child1_expr = expression.GetChild(index1);
      const auto is_opposite_operation1 = (GetOperation(child1_expr) == opposite_operation);

      for (auto index2 = index1 - 1; index2 >= 0; --index2)
      {
         auto& child2_expr = expression.GetChild(index2);
         const auto is_opposite_operation2 = (GetOperation(child2_expr) == opposite_operation);

         if (is_opposite_operation1)
         {
            auto& child1_expression = CastToOperation(child1_expr);

            // Complex case for absorption and gluing.
            if (is_opposite_operation2)
            {
               auto& child2_expression = CastToOperation(child2_expr);

               const auto child1_count = child1_expression.GetChildCount();
               const auto child2_count = child2_expression.GetChildCount();

               if (child1_count < child2_count)
               {
                  if (AreFirstChildrenIncludedInFirstChildren(
                        child1_expression, child1_count,
                        child2_expression, child2_count))
                  {
                     expression.RemoveChild(index2);
                     is_modified = true;
                     --index1;
                  }
               }
               else if (child1_count > child2_count)
               {

                  if (AreFirstChildrenIncludedInFirstChildren(
                        child2_expression, child2_count,
                        child1_expression, child1_count))
                  {
                     expression.RemoveChild(index1);
                     is_modified = true;
                     break;
                  }
               }
               else // (child1_count == child2_count)
               {
                  // Application of gluing law.
                  auto diff_index1 = -1L, diff_index2 = -1L;
                  if (AreChildrenDifferByOne(child1_expression, child2_expression,
                                             diff_index1, diff_index2))
                  {
                     auto& diff1_expr = child1_expression.GetChild(diff_index1);
                     auto& diff2_expr = child2_expression.GetChild(diff_index2);

                     if (CheckNegNotNeg(diff1_expr, diff2_expr) ||
                         CheckNegNotNeg(diff2_expr, diff1_expr))
                     {
                        child2_expression.RemoveChild(diff_index2);
                        if (2 == child2_count)
                        {
                           // In-place normalization
                           child2_expr = std::move(child2_expression.GetChild(0));
                        }
                        expression.RemoveChild(index1);
                        is_modified = true;
                        break;
                     }
                  }
               }
            }
            // Simple case for absorption.
            else if (IsEqualToAnyChild(child2_expr, child1_expression))
            {
               expression.RemoveChild(index1);
               is_modified = true;
               break;
            }
         }
         // Simple case for absorption (backward indexes)
         else if (is_opposite_operation2 && IsEqualToAnyChild(child1_expr, CastToOperation(child2_expr)))
         {
            expression.RemoveChild(index2);
            is_modified = true;
            --index1;
         }
      }
   }

   return is_modified;
}

void ExpressionEvaluator::InPlaceSimplification(OperationExpression& expression, long child_index)
{
   // Simplification is necessary only if child operation has a literal at the end.
   // Because the literal can be only the last operand (since all operations are
   // already simplified), the algorithm is not so difficult as the full simplification.

   auto& child_expression = CastToOperation(expression.GetChild(child_index));

   const auto last_index = expression.GetChildCount() - 1;
   const auto last_child_index = child_expression.GetChildCount() - 1;
   const auto last_literal = GetLiteral(expression.GetChild(last_index));
   const auto last_child_literal = GetLiteral(child_expression.GetChild(last_child_index));

   if (LiteralType::None != last_child_literal &&
       LiteralType::None != last_literal)
   {
      // If both literals exist, calculate result of operation and replace literal operand
      // of the parent expression with new operand with calculated result inside.
      const LiteralType literals[] = { last_child_literal, last_literal };
      auto result = PerformOperation(expression.GetOperation(), literals, 2);
      child_expression.RemoveChild(last_child_index);
      expression.GetChild(last_index) = std::make_unique<LiteralExpression>(result);
   }
   else if (LiteralType::None != last_child_literal)
   {
      // If literal exists only in child operation, just move it to the end
      // of the parent operation.
      auto last_child_expr = std::move(child_expression.GetChild(last_child_index));
      child_expression.RemoveChild(last_child_index);
      expression.AddChild(std::move(last_child_expr));
   }
}

void ExpressionEvaluator::InPlaceNormalization(OperationExpression& expression, long child_index)
{
   if (expression.GetOperation() == GetOperation(expression.GetChild(child_index)))
   {
      MoveChildExpressionsUp(expression, child_index);
   }
}

void ExpressionEvaluator::InPlaceNormalization(OperationExpression& expression)
{
   // The method is able to be called by implication evaluation even in case of empty 
   // children vector (it is done for logic simplification), so we must check emptiness manually.
   if (expression.GetChildCount() > 0)
   {
      InPlaceNormalization(expression, 0);
   }
}

bool ExpressionEvaluator::RemoveBeginningIfEqualToChild(
   OperationExpression& expression, long operands_between, bool include_child, bool negated_child)
{
   for (auto index = expression.GetChildCount() - 1; index > 1 + operands_between; --index)
   {
      auto& child_expr = expression.GetChild(index);
      if (child_expr->GetType() != ExpressionType::Operation)
      {
         continue;
      }

      auto& child_expression = CastToOperation(child_expr);
      const auto child_operation = child_expression.GetOperation();
      const auto amount_to_check = index - operands_between;
      auto implication_correction = 0L;
      
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
          AreFirstChildrenEqual(*child_to_check, expression, amount_to_check))
      {
         const auto right_bound = include_child ? (index + 1) : index;
         expression.RemoveChildren(0, right_bound);
         InPlaceNormalization(expression);
         return true;
      }
   }

   return false;
}

bool ExpressionEvaluator::IsNegationEquivalent(const TExpressionPtr& expr)
{
   if (expr->GetType() != ExpressionType::Operation)
   {
      return false;
   }

   return IsNegationEquivalent(CastToOperation(expr));
}

bool ExpressionEvaluator::IsNegationEquivalent(const OperationExpression& expression)
{
   // We have following rules of negation equivalents:
   //    1. !x
   //    2.  x -> 0
   //    3.  x = 0
   //    4.  x + 1

   const auto operation = expression.GetOperation();
   const auto child_count = expression.GetChildCount();

   if (OperationType::Negation == operation)
   {
      return true;
   }
   
   if (child_count < 2)
   {
      return false;
   }

   return (OperationType::Implication == operation &&
             (LiteralType::False == GetLiteral(expression.GetChild(child_count - 1)))) ||
          (OperationType::Equality == operation &&
             (LiteralType::False == GetLiteral(expression.GetChild(child_count - 1)))) ||
          (OperationType::Plus == operation &&
             (LiteralType::True == GetLiteral(expression.GetChild(child_count - 1))));
}

bool ExpressionEvaluator::IsShortNegationEquivalent(const TExpressionPtr& expr)
{
   if (expr->GetType() != ExpressionType::Operation)
   {
      return false;
   }

   return IsShortNegationEquivalent(CastToOperation(expr));
}

bool ExpressionEvaluator::IsShortNegationEquivalent(const OperationExpression& expression)
{
   // Short negation equivalent is such a negation equivalent that will turn into
   // single operand expression if negation is removed.
   return IsNegationEquivalent(expression) && (expression.GetChildCount() < 3);
}

bool ExpressionEvaluator::IsShortNegationEquivalentWithChildOperation(
   const TExpressionPtr& expr, OperationType operation)
{
   if (expr->GetType() != ExpressionType::Operation)
   {
      return false;
   }

   return IsShortNegationEquivalentWithChildOperation(CastToOperation(expr), operation);
}

bool ExpressionEvaluator::IsShortNegationEquivalentWithChildOperation(
   const OperationExpression& expression, OperationType operation)
{
   return IsShortNegationEquivalent(expression) &&
          GetOperation(expression.GetChild(0)) == operation;
}

bool ExpressionEvaluator::CheckNegNotNeg(
   const TExpressionPtr& neg_expr, const TExpressionPtr& expr)
{
   if (!IsNegationEquivalent(neg_expr))
   {
      return false;
   }

   // We have 2 cases that must be checked:
   //    1. Simple case, e.g. (x + 1) and x.
   //    2. Complex case, e.g. (x + y + 1) and (x + y)

   // We can cast without checking GetType, because IsNegationEquivalent
   // will return false in case of non-operation expression.
   const auto& neg_expression = CastToOperation(neg_expr);

   // 1. Simple case.
   if (neg_expression.GetChildCount() < 3 &&
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
           AreFirstChildrenEqual(neg_expression, expression, expression.GetChildCount()));
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

void ExpressionEvaluator::CoverWithNegationEquivalent(TExpressionPtr& expr)
{
   assert(!IsNegationEquivalent(expr));

   // Let's use following rules to make negation more presentable.
   //    1. ( x -> 0), in case of implication
   //    2. ( x  = 0), in case of equality
   //    3. ( x  + 1), in case of plus
   //    4. (!x),      otherwise

   const auto child_operation = GetOperation(expr);

   if (OperationType::Implication == child_operation ||
       OperationType::Equality == child_operation)
   {
      CastToOperation(expr).AddChild(std::make_unique<LiteralExpression>(LiteralType::False));
   }
   else if (OperationType::Plus == child_operation)
   {
      CastToOperation(expr).AddChild(std::make_unique<LiteralExpression>(LiteralType::True));
   }
   else
   {
      expr = std::make_unique<OperationExpression>(std::move(expr));
   }
}

void ExpressionEvaluator::RevertNegations(TExpressionPtr& expr)
{
   auto& expression = CastToOperation(expr);
   for (auto index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      auto& child_expr = expression.GetChild(index);
      if (IsNegationEquivalent(child_expr))
      {
         ExtractFromUnderNegationEquivalent(child_expr);
         InPlaceNormalization(expression, index);
      }
      else
      {
         CoverWithNegationEquivalent(child_expr);
      }
   }
}

bool ExpressionEvaluator::IsEqual(const TExpressionPtr& left, const TExpressionPtr& right)
{
   const auto type = left->GetType();

   if (type != right->GetType())
   {
      return false;
   }

   switch (type)
   {
      case ExpressionType::Literal:
         return (CastToLiteral(left).GetLiteral() == CastToLiteral(right).GetLiteral());

      case ExpressionType::ParamRef:
         return (CastToParamRef(left).GetParamIndex() == CastToParamRef(right).GetParamIndex());

      case ExpressionType::Operation:
      {
         auto& left_operation = CastToOperation(left);
         auto& right_operation = CastToOperation(right);

         if (left_operation.GetOperation() == right_operation.GetOperation())
         {
            return (left_operation.GetChildCount() == right_operation.GetChildCount() &&
                    AreFirstChildrenEqual(left_operation, right_operation,
                                          left_operation.GetChildCount()));
         }

         // Equality is still possible, if operations are mutually reverse, for example:
         //    (x + y) equals (x = y = 0)
         return IsEqualUpToMutuallyReverseOperations(left_operation, right_operation);
      }
   }

   assert(!"Unknown expression type.");

   return false;
}

bool ExpressionEvaluator::IsEqualUpToMutuallyReverseOperations(
   const OperationExpression& left, const OperationExpression& right)
{
   if (!AreOperationsMutuallyReverse(left.GetOperation(), right.GetOperation()))
   {
      return false;
   }

   const auto left_count = left.GetChildCount();
   const auto right_count = right.GetChildCount();

   const auto diff = (left_count - right_count);
   // Amounts of child operands are to be distinguished from each other by 1.
   if ((diff != 1) && (diff != -1))
   {
      return false;
   }

   if ((1 == diff && LiteralType::None == GetLiteral(left.GetChild(left_count - 1))) ||
       (1 != diff && LiteralType::None == GetLiteral(right.GetChild(right_count - 1))))
   {
       return false;
   }

   return AreFirstChildrenEqual(left, right, (1 == diff) ? right_count : left_count);
}

bool ExpressionEvaluator::IsEqualToAnyChild(
   const TExpressionPtr& expr, const OperationExpression& expression)
{
   for (auto index = expression.GetChildCount() - 1; index >= 0; --index)
   {
      if (IsEqual(expr, expression.GetChild(index)))
      {
         return true;
      }
   }
   return false;
}

bool ExpressionEvaluator::AreFirstChildrenEqual(
   const OperationExpression& left, const OperationExpression& right, long amount)
{
   return AreFirstChildrenIncludedInFirstChildren(left, amount, right, amount);
}

bool ExpressionEvaluator::AreFirstChildrenIncludedInFirstChildren(
   const OperationExpression& enclosing, long enclosing_amount,
   const OperationExpression& enveloping, long enveloping_amount, long skip_index)
{
   assert(enclosing_amount <= enveloping_amount);
   assert(enclosing_amount <= enclosing.GetChildCount());
   assert(enveloping_amount <= enveloping.GetChildCount());

   if (!AreOperandsMovable(enclosing.GetOperation()))
   {
      // If operands are not movable, then gaps are not available.
      if (enclosing_amount != enveloping_amount)
      {
         return false;
      }
      
      // Just use sequential pairwaise comparison.
      for (auto index = enclosing_amount - 1; index >= 0; --index)
      {
         if (!IsEqual(enclosing.GetChild(index), enveloping.GetChild(index)))
         {
            return false;
         }
      }

      return true;
   }

   // If operands are movable, it's not enough just to use sequential pairwise comparison.
   // We need to check whether two sets of child operands contain the same operands up
   // to a permutation.

   // The array contains information about whether i-th operand of "enveloping" was linked
   // to some operand of "enclosing", during conformity detection.
   LOCAL_ARRAY(bool, child_linked_flags, enveloping_amount);
   std::fill_n(child_linked_flags, enveloping_amount, false);

   // Let's establish one-to-one corresponce between elements of "enclosing" and "enveloping",
   // using child_linked_flags to mark element of "enveloping" as linked.
   for (auto index1 = enclosing_amount - 1, index2 = 0L; index1 >= 0; --index1)
   {
      for (index2 = enveloping_amount - 1; index2 >= 0; --index2)
      {
         if (!child_linked_flags[index2] && index2 != skip_index &&
             IsEqual(enclosing.GetChild(index1), enveloping.GetChild(index2)))
         {
            child_linked_flags[index2] = true;
            break;
         }
      }

      if (-1 == index2)
      {
         // No pair for "enclosing" child.
         return false;
      }
   }

   // Full conformity is detected.
   return true;
}

bool ExpressionEvaluator::AreChildrenDifferByOne(
   const OperationExpression& expression1, const OperationExpression& expression2,
   long& diff_index1, long& diff_index2)
{
   assert(expression1.GetOperation() == expression2.GetOperation());
   assert(expression1.GetChildCount() == expression2.GetChildCount());
   assert(AreOperandsMovable(expression1.GetOperation()));

   diff_index1 = -1;
   diff_index2 = -1;

   const auto child_count = expression1.GetChildCount();

   LOCAL_ARRAY(bool, child_linked_flags, child_count);
   std::fill_n(child_linked_flags, child_count, false);

   for (auto index1 = child_count - 1, index2 = 0L; index1 >= 0; --index1)
   {
      for (index2 = child_count - 1; index2 >= 0; --index2)
      {
         if (!child_linked_flags[index2] &&
             IsEqual(expression1.GetChild(index1), expression2.GetChild(index2)))
         {
            child_linked_flags[index2] = true;
            break;
         }
      }

      if (-1 == index2)
      {
         // No pair for "expression1" child.
         if (-1 == diff_index1)
         {
            diff_index1 = index1;
         }
         else
         {
            return false;
         }
      }
   }

   if (-1 == diff_index1)
   {
      // Fully equal sets of children is not our case.
      return false;
   }

   // Find unset element of child_linked_flags and set its index to diff_index2.
   auto it = std::find(child_linked_flags, child_linked_flags + child_count, false);
   assert(it != child_linked_flags + child_count);
   diff_index2 = (it - child_linked_flags);

   return true;
}

} // namespace

void EvaluateExpression(TExpressionPtr& expr)
{
   assert(expr.get() != nullptr);
   ExpressionEvaluator evaluator;
   evaluator.Evaluate(expr);
}

} // namespace dm
