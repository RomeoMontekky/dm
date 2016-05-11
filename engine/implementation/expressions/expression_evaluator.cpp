#include "expression_evaluator.h"
#include "expression_visitor.h"

#include "expression_simplifier.h"
#include "expression_normalizer.h"
#include "expression_mover.h"
#include "expressions.h"

#include "../common/local_array.h"

#include <algorithm>
#include <cassert>

namespace dm
{

namespace
{
   
class ExpressionEvaluator;
using TExpressionEvaluatorVector = std::vector<ExpressionEvaluator>;

class ExpressionEvaluator : private ExpressionVisitor
{
public:
   ExpressionEvaluator();
   ExpressionEvaluator(ExpressionEvaluator&& rhs) = default;
   ExpressionEvaluator& operator =(ExpressionEvaluator&& rhs) = default;
   
   // Returns information about whether current expression
   // has been fully evaluated to a certain other expression.
   bool Evaluate(TExpressionPtr& expression);
   
   bool operator ==(const ExpressionEvaluator& rhs) const;
   bool operator !=(const ExpressionEvaluator& rhs) const;
   
private:   
   void Reset();

   // ExpressionVisitor
   virtual void Visit(LiteralExpression& expression) override;
   virtual void Visit(ParamRefExpression& expression) override;
   virtual void Visit(OperationExpression& expression) override;

private:
   bool EvaluateNestedNegationEquivalents(OperationExpression& expression);
   void EvaluateOperation(OperationExpression& expression);

   // Following set of methods are called from EvaluateOperation, using pointer.
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
   void AbsorbNegations(OperationExpression& expression, LiteralType eq_to_neg_literal);
   bool AbsorbNegNotNegs(OperationExpression& expression,
                         LiteralType eq_to_neg_literal, LiteralType remaining_literal);
   bool AbsorbNegNotNegsGrouped(OperationExpression& expression,
                                LiteralType eq_to_neg_literal, LiteralType remaining_literal);
   bool CorrectLiteralByAbsorptionCount(OperationExpression& expression, long absorption_count,
                                        LiteralType eq_to_neg_literal, LiteralType remaining_literal);
   bool CanBeGroupedAsNegNotNeg(OperationExpression& expression, std::vector<long>* indexes = nullptr);

   // These methods are used by implication evaluation.
   void InPlaceNormalization(OperationExpression& expression);
   bool RemoveBeginningIfEqualToChild(OperationExpression& expression,
                                      long operands_between, bool include_child, 
                                      bool negated_child = false);

   // Utility
   static bool IsNegationEquivalent(const ExpressionEvaluator& value);
   static bool CheckNegNotNeg(const ExpressionEvaluator& negated_value,
                              const ExpressionEvaluator& value);
   static void ExtractFromUnderNegationEquivalent(ExpressionEvaluator& value,
                                                  TExpressionPtr& expression);

   // Checks that first size elements of vectors vec1 and vec2 have
   // one-to-one accordance each with another.
   static bool AreEvaluatorSetsEqual(const TExpressionEvaluatorVector& vec1,
                                     const TExpressionEvaluatorVector& vec2, long size);

private:
   // Following three fields hold values for three possible variants
   // of visited expressions.
   LiteralType m_literal;
   long m_param_index;
   OperationType m_operation;
   
   // Is normalization/simplification needed on high level.
   bool m_is_normalization_needed;

   // Will be filled by new evaluated expression if the whole
   // operation expression was evaluated to a some simple form.
   TExpressionPtr m_evaluated_expression;
   
   // Contains visitors for child expressions if current visited is operation expression.
   TExpressionEvaluatorVector m_children;
};

ExpressionEvaluator::ExpressionEvaluator()
{
   Reset();
}

bool ExpressionEvaluator::Evaluate(TExpressionPtr& expression)
{
   bool was_evaluated_to_expression = false;

   while (true)
   {
      Reset();
      
      expression->Accept(*this);
      
      if (m_evaluated_expression.get() != nullptr)
      {
         expression = std::move(m_evaluated_expression);
         was_evaluated_to_expression = true;
      } 
      else if (m_is_normalization_needed)
      {
         NormalizeExpression(expression);
         SimplifyExpression(expression);
      }
      else
      {
         break;
      }
   }

   return was_evaluated_to_expression;
}

bool ExpressionEvaluator::operator ==(const ExpressionEvaluator& rhs) const
{
   // Following values isn't checked intentionally:
   //    1. m_is_normalization_needed
   //    2. m_evaluated_expression
   
   if ((m_literal != rhs.m_literal) ||
       (m_param_index != rhs.m_param_index) ||
       (m_operation != rhs.m_operation))
   {
      return false;
   }
   
   if (OperationType::None == m_operation)
   {
      return true;
   }
   
   if (!AreOperandsMovable(m_operation))
   {
      return (m_children == rhs.m_children);
   }
   
   // If operands are movable, it's not enough just to use comparison of vectors.
   // We need to check whether two vectors contain the same set of operands
   // up to a permutation.
   
   // If sizes aren't equal, then conformity is definitly absent.
   if (m_children.size() != rhs.m_children.size())
   {
      return false;
   }
   
   return AreEvaluatorSetsEqual(m_children, rhs.m_children, m_children.size());
}

bool ExpressionEvaluator::operator !=(const ExpressionEvaluator& rhs) const
{
   return !(*this == rhs);
}

void ExpressionEvaluator::Reset()
{
   m_literal = LiteralType::None;
   m_param_index = -1;
   m_operation = OperationType::None;
   m_is_normalization_needed = false;
   m_children.clear();
   m_evaluated_expression.reset();
}

void ExpressionEvaluator::Visit(LiteralExpression& expression)
{
   m_literal = expression.GetLiteral();
}

void ExpressionEvaluator::Visit(ParamRefExpression& expression)
{
   m_param_index = expression.GetParamIndex();
}

void ExpressionEvaluator::Visit(OperationExpression& expression)
{
   m_operation = expression.GetOperation();

   const long child_count = expression.GetChildCount();
   m_children.resize(child_count);
   for (long index = 0; index < child_count; ++index)
   {
      auto& child = m_children[index];

      if (child.Evaluate(expression.GetChild(index)) &&
          child.m_operation == expression.GetOperation())
      {
         m_is_normalization_needed = true;
      }
   }

   EvaluateOperation(expression);
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

   // If after evaluation the only operand is remained, then
   // current expression can be evaluated to this operand.
   if (expression.GetOperation() != OperationType::Negation &&
       expression.GetChildCount() == 1)
   {
      // Evaluated expression shouln't be set already.
      assert(m_evaluated_expression.get() == nullptr);
      m_evaluated_expression = std::move(expression.GetChild(0));
   }
}

void ExpressionEvaluator::EvaluateNegation(OperationExpression& expression)
{
   assert(m_children.size() == 1);

   // We have the only rule:
   //    1. !!x       => x

   // The nested negation can be treated as negation equivalent.
   if (IsNegationEquivalent(m_children.front()))
   {
      ExtractFromUnderNegationEquivalent(m_children.front(), expression.GetChild(0));
      m_evaluated_expression = std::move(expression.GetChild(0));
   }
}

void ExpressionEvaluator::EvaluateConjunction(OperationExpression& expression)
{
   assert(m_children.size() > 1);

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
   assert(m_children.size() > 1);

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
   assert(m_children.size() > 1);

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

   for (long index = m_children.size() - 1; index >= 0; --index)
   {
      if (LiteralType::True == m_children[index].m_literal)
      {
         m_children.erase(m_children.begin(), m_children.begin() + index + 1);
         expression.RemoveChildren(0, index + 1);
         InPlaceNormalization(expression);
         break;
      }
   }

   bool was_evaluated = true;

   do
   {
      while (m_children.size() > 1 && was_evaluated)
      {
         was_evaluated = false;

         // According to rules 2 and 1, we can remove subexpression (0 -> x)
         // if it is at the beginning of the expression.
         // According to rules 5 and 1, we can remove subexpression (x -> x)
         // if it is at the beginning of the expression
         if (LiteralType::False == m_children.front().m_literal ||
             m_children[0] == m_children[1])
         {
            m_children.erase(m_children.begin(), m_children.begin() + 2);
            expression.RemoveChildren(0, 2);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 4 perform transformation (!x -> 0) => x
         else if (IsNegationEquivalent(m_children.front()) &&
                  LiteralType::False == m_children[1].m_literal)
         {
            ExtractFromUnderNegationEquivalent(m_children.front(), expression.GetChild(0));
            m_children.erase(m_children.begin() + 1);
            expression.RemoveChild(1);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 6, we can remove first operand in case of (!x ->  x)
         // According to rule 7, we can remove first operand in case of ( x -> !x)
         else if (CheckNegNotNeg(m_children[0], m_children[1]) ||
                  CheckNegNotNeg(m_children[1], m_children[0]))
         {
            m_children.erase(m_children.begin());
            expression.RemoveChild(0);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 9, we can remove first two operands in case of (x -> y -> x)
         else if (m_children.size() > 2 && m_children[0] == m_children[2])
         {
            m_children.erase(m_children.begin(), m_children.begin() + 2);
            expression.RemoveChildren(0, 2);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
      } // while (m_children.size() > 1 && was_evaluated)

      // Other rules require at least 3 operands.
      if (m_children.size() < 3)
      {
         break;
      }

      // According to rule 8, we can remove two zero literals
      // if they stay one after another.
      for (long index = m_children.size() - 2; index >= 0; --index)
      {
         if (LiteralType::False == m_children[index].m_literal &&
             LiteralType::False == m_children[index + 1].m_literal)
         {
            m_children.erase(m_children.begin() + index, m_children.begin() + index + 2);
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

   if (m_children.empty())
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(LiteralType::True);
   }
}

void ExpressionEvaluator::EvaluateEquality(OperationExpression& expression)
{
   assert(m_children.size() > 1);

   // We have following rules:
   //    1. ( x =  1 ) => x
   //    2. (!x =  0 ) => x
   //    3. ( x =  x ) => 1
   //    4. (!x = !y ) => (x = y)
   //    5. ( x = !x ) => 0

   // According to rule 1, remove literal 1.
   RemoveLiteralIfExists(expression, LiteralType::True);

   // According to rule 5 absorb operands that equal each another up to
   // negation, count resulting 0 literals and reduce/add to the end.
   if (AbsorbNegNotNegs(expression, LiteralType::False, LiteralType::True))
   {
      return;
   }
   
   // According to rule 4 and 2, reduce even amount of negations
   // and reduce the only remaining negation (if exists) with literal 0.
   AbsorbNegations(expression, LiteralType::False);

   // According to rule 3 and 1, remove duplicates
   // and assign leteral 1 if all operands were removed.
   AbsorbDuplicates(expression, LiteralType::True);
   
   // The same as AbsorbNegNotNegs, but with grouping.
   AbsorbNegNotNegsGrouped(expression, LiteralType::False, LiteralType::True);
}

void ExpressionEvaluator::EvaluatePlus(OperationExpression& expression)
{
   assert(m_children.size() > 1);

   // We have following rules:
   //    1. ( x +  0 ) => x
   //    2. (!x +  1 ) => x
   //    3. ( x +  x ) => 0
   //    4. (!x + !y ) => (x + y)
   //    5. ( x + !x ) => 1

   // According to rule 1, remove literal 0.
   RemoveLiteralIfExists(expression, LiteralType::False);

   // According to rule 5 absorb operands that equal up to negation operation,
   // count resulting 1 literals and reduce/add to the end.
   if (AbsorbNegNotNegs(expression, LiteralType::True, LiteralType::False))
   {
      return;
   }
   
   // According to rule 4 and 2, reduce even amount of negations
   // and reduce the only remaining negation (if exists) with literal 1.
   AbsorbNegations(expression, LiteralType::True);

   // According to rule 3 and 1, remove duplicates
   // and assign leteral 0 if all operands were removed.
   AbsorbDuplicates(expression, LiteralType::False);
   
   // The same as AbsorbNegNotNegs, but with grouping.
   AbsorbNegNotNegsGrouped(expression, LiteralType::True, LiteralType::False);
}

bool ExpressionEvaluator::RemoveAllIfLiteralExists(
   OperationExpression& expression, LiteralType literal)
{
   if (literal == m_children.back().m_literal)
   {
      m_evaluated_expression = std::move(
         expression.GetChild(m_children.size() - 1));
      return true;
   }
   return false;
}

bool ExpressionEvaluator::RemoveAllIfNegNotNegExists(
   OperationExpression& expression, LiteralType remaining_literal)
{
   const long child_count = m_children.size();

   for (int i = 0; i < child_count - 1; ++i)
      for (int j = i + 1; j < child_count; ++j)
         if (CheckNegNotNeg(m_children[i], m_children[j]) ||
             CheckNegNotNeg(m_children[j], m_children[i]))
         {
            m_evaluated_expression =
               std::make_unique<LiteralExpression>(remaining_literal);
            return true;
         }

   return false;
}

void ExpressionEvaluator::RemoveLiteralIfExists(
   OperationExpression& expression, LiteralType literal)
{
   if (literal == m_children.back().m_literal)
   {
      m_children.pop_back();
      expression.RemoveChild(m_children.size());
   }
}

void ExpressionEvaluator::RemoveDuplicates(OperationExpression& expression)
{
   int child_count = m_children.size();

   for (long i = 0; i < child_count - 1; ++i)
   {
      long j = i + 1;
      while (j < child_count)
      {
         if (m_children[i] == m_children[j])
         {
            m_children.erase(m_children.begin() + j);
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
   int child_count = m_children.size();

   long i = 0;
   while (i < child_count - 1)
   {
      long j = i + 1;
      while (j < child_count)
      {
         if (m_children[i] == m_children[j])
         {
            m_children.erase(m_children.begin() + j);
            m_children.erase(m_children.begin() + i);
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

void ExpressionEvaluator::AbsorbNegations(OperationExpression& expression, LiteralType eq_to_neg_literal)
{
   const long child_count = m_children.size();

   long prev_negation = -1;
   for (long index = 0; index < child_count; ++index)
   {
      if (IsNegationEquivalent(m_children[index]))
      {
         if (-1 == prev_negation)
         {
            prev_negation = index;
         }
         else
         {
            ExtractFromUnderNegationEquivalent(
               m_children[prev_negation], expression.GetChild(prev_negation));
            ExtractFromUnderNegationEquivalent(
               m_children[index], expression.GetChild(index));

            if (m_children[prev_negation].m_operation == expression.GetOperation() ||
                m_children[index].m_operation == expression.GetOperation())
            {
               m_is_normalization_needed = true;
            }

            prev_negation = -1;
         }
      }
   }

   if (prev_negation != -1)
   {
      ExtractFromUnderNegationEquivalent(
         m_children[prev_negation], expression.GetChild(prev_negation));

      if (eq_to_neg_literal == m_children.back().m_literal)
      {
         m_children.pop_back();
         expression.RemoveChild(m_children.size());
      }
      else
      {
         m_children.resize(m_children.size() + 1);
         m_children.back().m_literal = eq_to_neg_literal;
         expression.AddChild(std::make_unique<LiteralExpression>(eq_to_neg_literal));
      }

      if (m_children[prev_negation].m_operation == expression.GetOperation())
      {
         m_is_normalization_needed = true;
      }
   }
}

bool ExpressionEvaluator::AbsorbNegNotNegs(
   OperationExpression& expression, LiteralType eq_to_neg_literal, LiteralType remaining_literal)
{
   int child_count = m_children.size();

   int absorption_count = 0;

   long i = 0;
   while (i < child_count - 1)
   {
      long j = i + 1;
      while (j < child_count)
      {
         if (CheckNegNotNeg(m_children[i], m_children[j]) ||
             CheckNegNotNeg(m_children[j], m_children[i]))
         {
            ++absorption_count;
            m_children.erase(m_children.begin() + j);
            m_children.erase(m_children.begin() + i);
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

   return CorrectLiteralByAbsorptionCount(expression, absorption_count, eq_to_neg_literal, remaining_literal);
}

bool ExpressionEvaluator::AbsorbNegNotNegsGrouped(
   OperationExpression& expression, LiteralType eq_to_neg_literal, LiteralType remaining_literal)
{
   int absorption_count = 0;
   
   std::vector<long> indexes;
   while (!m_children.empty() && CanBeGroupedAsNegNotNeg(expression, &indexes))
   {
      ++absorption_count;
      
      if (m_children.size() == indexes.size())
      {
         // Small optimization for case of the whole removing
         m_children.clear();
         expression.RemoveChildren(0, expression.GetChildCount());
      }
      else
      {
         // Removing is done in reverse order to avoid shifting elements in affected vectors.
         for (auto it = indexes.crbegin(); it != indexes.crend(); ++it)
         {
            m_children.erase(m_children.begin() + *it);
            expression.RemoveChild(*it);
         }
      }
   }
   
   return CorrectLiteralByAbsorptionCount(expression, absorption_count, eq_to_neg_literal, remaining_literal);
}

bool ExpressionEvaluator::CorrectLiteralByAbsorptionCount(
   OperationExpression& expression, long absorption_count, LiteralType eq_to_neg_literal, LiteralType remaining_literal)
{
   // If absorption count is odd
   if ((absorption_count & 1) == 1)
   {
      if (!m_children.empty() &&
           m_children.back().m_literal == eq_to_neg_literal)
      {
         m_children.pop_back();
         expression.RemoveChild(expression.GetChildCount() - 1);
      }
      else
      {
         m_children.resize(m_children.size() + 1);
         m_children.back().m_literal = eq_to_neg_literal;
         expression.AddChild(std::make_unique<LiteralExpression>(eq_to_neg_literal));
      }
   }

   if (m_children.empty())
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(remaining_literal);
      return true;
   }
   
   return false;
}

bool ExpressionEvaluator::CanBeGroupedAsNegNotNeg(
   OperationExpression& expression, std::vector<long>* indexes)
{
   // Checks whether there exists some negated child, that (under negation)
   // contains child operands, that all have equivalents between
   // children of the current operation.
   
   // Examples:
   //    !(x & y) & x & y & z
   //    ((x + y)->0) + x + y + z
   
   const long child_count = m_children.size();
   
   if (child_count < 3)
   {
      // Two operarands cannot be grouped, because it means that operation under
      // negation must have a single child, that is possible for negation only,
      // but nested negations must be reduced during recursive evalution call.
      return false;
   }
   
   if (indexes != nullptr)
   {
      indexes->reserve(child_count - 1);
   }
   
   for (long index = 0, i = 0, j = 0; index < child_count; ++index)
   {
      const auto& child = m_children[index];
      
      if (indexes != nullptr)
      {
         indexes->clear();
         indexes->push_back(index);
      }
      
      // If current child is negation equivalent and there it contains the same
      // operation as current operation, then check whether other children are
      // the same as children of the negated child.
      
      // Checking of the fact, that child operands amount of the 'child' is less
      // than 3, needs additional clarification. Removing of negation from the
      // operation that has got more then 2 child operands will give this operation,
      // but this operations will not be same as the current operation (otherwise
      // it would be normalized).
      
      if (IsNegationEquivalent(child) && 
          child.m_children.size() < 3 &&
          child.m_children.front().m_operation == expression.GetOperation())
      {
         const auto& children_to_check = child.m_children.front().m_children;
         const long children_to_check_count = children_to_check.size();
         
         for (i = 0; i < children_to_check_count; ++i)
         {
            for (j = 0; j < child_count; ++j)
            {
               if (j != index && m_children[j] == children_to_check[i])
               {
                  // Break if equal child is found.
                  break;
               }
            }
            
            // Equal child is not found.
            if (child_count == j)
            {
               break;
            }
            else if (indexes != nullptr)
            {
               indexes->push_back(j);
            }
         }
         
         // There are exist equivalent for each child under negation.
         if (children_to_check_count == i)
         {
            if (indexes != nullptr)
            {
               std::sort(indexes->begin(), indexes->end());
            }
            return true;
         }
      }
   }
   
   if (indexes != nullptr)
   {
      indexes->clear();
   }   
   
   return false;
}

void ExpressionEvaluator::InPlaceNormalization(OperationExpression& expression)
{
   // InPlaceNormalization can be done for implication only.
   // For other operations full normalization/simplification must be performed.
   assert(expression.GetOperation() == OperationType::Implication);

   if (!m_children.empty() && OperationType::Implication == m_children[0].m_operation)
   {
      TExpressionPtrVector moved_children;
      MoveChildExpressions(moved_children, expression.GetChild(0));
      expression.RemoveChild(0);
      expression.InsertChildren(0, std::move(moved_children));

      auto moved_children_evaluators = std::move(m_children[0].m_children);
      m_children.erase(m_children.begin());
      m_children.reserve(m_children.size() + moved_children_evaluators.size());
      for (long index = moved_children_evaluators.size() - 1; index >= 0; --index)
      {
         m_children.emplace(m_children.begin(),
            std::move(moved_children_evaluators[index]));
      }
   }
}

bool ExpressionEvaluator::RemoveBeginningIfEqualToChild(
   OperationExpression& expression, long operands_between, bool include_child, bool negated_child)
{
   for (long i = m_children.size() - 1; i > 1 + operands_between; --i)
   {
      const auto& curr_child = m_children[i];
      const long amount_to_check = i - operands_between;
      long implication_correction = 0;
      
      const decltype(m_children)* children_to_check = nullptr;
      if (negated_child)
      {
         if (OperationType::Negation == curr_child.m_operation &&
             OperationType::Implication == curr_child.m_children[0].m_operation)
         {
            children_to_check = &curr_child.m_children[0].m_children;
         }
         else if (OperationType::Implication == curr_child.m_operation &&
                  LiteralType::False == curr_child.m_children.back().m_literal)
         {
            children_to_check = &curr_child.m_children;
            implication_correction = 1;
         }
      }
      else
      {
         if (OperationType::Implication == curr_child.m_operation)
         {
            children_to_check = &curr_child.m_children;
         }
      }
      
      if (children_to_check != nullptr && 
          children_to_check->size() - implication_correction == amount_to_check &&
          std::equal(m_children.begin(), m_children.begin() + amount_to_check,
                     children_to_check->begin(), children_to_check->begin() + amount_to_check))
      {
         const long right_bound = include_child ? (i + 1) : i;
         m_children.erase(m_children.begin(), m_children.begin() + right_bound);
         expression.RemoveChildren(0, right_bound);
         InPlaceNormalization(expression);
         return true;
      }
   }

   return false;
}

bool ExpressionEvaluator::IsNegationEquivalent(const ExpressionEvaluator& value)
{
   // We have following rules of negation equivalents:
   //    1. !x
   //    2.  x -> 0
   //    3.  x = 0
   //    4.  x + 1

   return (OperationType::Negation == value.m_operation) ||
          (OperationType::Implication == value.m_operation &&
             (LiteralType::False == value.m_children.back().m_literal)) ||
          (OperationType::Equality == value.m_operation &&
             (LiteralType::False == value.m_children.back().m_literal)) ||
          (OperationType::Plus == value.m_operation &&
             (LiteralType::True == value.m_children.back().m_literal));
}

bool ExpressionEvaluator::CheckNegNotNeg(
   const ExpressionEvaluator& negated_value, const ExpressionEvaluator& value)
{
   if (!IsNegationEquivalent(negated_value))
   {
      return false;
   }

   switch (negated_value.m_operation)
   {
      case OperationType::Negation:
         return (negated_value.m_children.front() == value);
         
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
   if (negated_value.m_children.size() == 2 &&
       negated_value.m_children.front() == value)
   {
      return true;
   }

   // 2. Complex case.
   if (negated_value.m_operation == value.m_operation &&
       negated_value.m_children.size() - 1 == value.m_children.size())
   {
      // There is 2 sub-cases:
      //    - if operation is commutative, we must compare children as sets.
      //    - if operation isn't commutative, we must compare children as vectors.

      if (AreOperandsMovable(value.m_operation))
      {
         return AreEvaluatorSetsEqual(
            negated_value.m_children, value.m_children, value.m_children.size());
      }
      else
      {
         return std::equal(
            negated_value.m_children.begin(), negated_value.m_children.begin() + value.m_children.size(),
            value.m_children.begin(), value.m_children.begin() + value.m_children.size());
      }
   }

   return false;
}

void ExpressionEvaluator::ExtractFromUnderNegationEquivalent(
   ExpressionEvaluator& value, TExpressionPtr& expression)
{
   assert(IsNegationEquivalent(value));
   
   // It will be performed for following operations:
   //    - OperationType::Implication
   //    - OperationType::Equality
   //    - OperationType::Plus
   if (value.m_children.size() > 1)
   {
      value.m_children.pop_back();
      RemoveChildExpression(expression, -1);
   }
   
   // If will be performed for previous operations if the only child is remained
   // after literal removing.
   // Also the condition will be triggered for OperationType::Negation.
   if (value.m_children.size() == 1)
   {
      value = std::move(value.m_children.front());
      MoveChildExpressionInplace(expression);
   }
}

bool ExpressionEvaluator::AreEvaluatorSetsEqual(
   const TExpressionEvaluatorVector& vec1, const TExpressionEvaluatorVector& vec2, long size)
{
   assert(size <= vec1.size());
   assert(size <= vec2.size());
   
   // Contains information about whether i-th element of vec2 was
   // linked to some element of m_children, during conformity detection.
   LOCAL_ARRAY(bool, child_linked_flags, size);
   std::fill_n(child_linked_flags, size, false);

   // Let's establish one-to-one corresponce between elements of vec1 and vec2,
   // using child_linked_flags to mark element of vec2 as linked.

   for (long i = 0, j; i < size; ++i)
   {
      for (j = 0; j < size; ++j)
      {
         if ((false == child_linked_flags[j]) && 
             (vec1[i] == vec2[j]))
         {
            child_linked_flags[j] = true;
            break;
         }
      }
      
      if (size == j)
      {
         // No equal pair for m_children[i].
         return false;
      }
   }
   
   // Full conformity is detected.
   return true;
}

} // namespace

void EvaluateExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ExpressionEvaluator evaluator;
   evaluator.Evaluate(expression);
}

} // namespace dm
