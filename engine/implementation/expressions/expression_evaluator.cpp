#include "expression_evaluator.h"
#include "expression_visitor.h"

#include "expression_simplifier.h"
#include "expression_normalizer.h"
#include "expression_mover.h"
#include "expressions.h"

#include <map>
#include <cassert>

namespace dm
{

namespace
{

class ExpressionEvaluator : private ExpressionVisitor
{
public:
   ExpressionEvaluator();
   ExpressionEvaluator(ExpressionEvaluator&& rhs) = default;
   ExpressionEvaluator& operator =(ExpressionEvaluator&& rhs) = default;
   
   void Evaluate(TExpressionPtr& expression);
   
   bool operator ==(const ExpressionEvaluator& rhs) const;
   bool operator !=(const ExpressionEvaluator& rhs) const;
   
private:   
   void Reset();

   // ExpressionVisitor
   virtual void Visit(LiteralExpression& expression) override;
   virtual void Visit(ParamRefExpression& expression) override;
   virtual void Visit(OperationExpression& expression) override;

private:
   void EvaluateOperation(OperationExpression& expression);

   void EvaluateNegation(OperationExpression& expression);
   void EvaluateConjunction(OperationExpression& expression);
   void EvaluateDisjunction(OperationExpression& expression);
   void EvaluateImplication(OperationExpression& expression);
   void EvaluateEquality(OperationExpression& expression);
   void EvaluatePlus(OperationExpression& expression);

private:
   // Following set of methods is used to re-use common rules of
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

   // This method is used by implication evaluation.
   void InPlaceNormalization(OperationExpression& expression);

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
   std::vector<ExpressionEvaluator> m_children;
};

ExpressionEvaluator::ExpressionEvaluator()
{
   Reset();
}

void ExpressionEvaluator::Evaluate(TExpressionPtr& expression)
{
   while (true)
   {
      Reset();
      
      expression->Accept(*this);
      
      if (m_evaluated_expression.get() != nullptr)
      {
         expression = std::move(m_evaluated_expression);
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
}

bool ExpressionEvaluator::operator ==(const ExpressionEvaluator& rhs) const
{
   // Following values isn't checked intentionally:
   //    1. m_is_normalization_needed
   //    2. m_evaluated_expression
   
   return
   (
      (m_literal == rhs.m_literal) &&
      (m_param_index == rhs.m_param_index) &&
      (m_operation == rhs.m_operation) &&
      (m_children == rhs.m_children)
   );
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
      m_children[index].Evaluate(expression.GetChild(index));
   }

   EvaluateOperation(expression);
}

void ExpressionEvaluator::EvaluateOperation(OperationExpression& expression)
{
   using base_map = std::map
   <
      OperationType,
      void(ExpressionEvaluator::*)(OperationExpression& expression)
   >;

   static const class operation_to_function_map : public base_map
   {
   public:
      operation_to_function_map() : base_map()
      {
         emplace(OperationType::Negation,    EvaluateNegation);
         emplace(OperationType::Conjunction, EvaluateConjunction);
         emplace(OperationType::Disjunction, EvaluateDisjunction);
         emplace(OperationType::Implication, EvaluateImplication);
         emplace(OperationType::Equality,    EvaluateEquality);
         emplace(OperationType::Plus,        EvaluatePlus);
      }
   }
   op_to_func;

   auto method = op_to_func.at(expression.GetOperation());
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
   //    1. !!x => x

   if (m_children[0].m_operation == OperationType::Negation)
   {
      MoveChildExpression(m_evaluated_expression, expression.GetChild(0));
   }
}

void ExpressionEvaluator::EvaluateConjunction(OperationExpression& expression)
{
   assert(m_children.size() > 1);

   // We have following rules:
   //    1. (x  & 0) = 0
   //    2. (x  & 1) = x
   //    3. (x  & x) = x
   //    4. (!x & x) = 0

   // According to rule 1, evaluate expression to literal 0 if it exist.
   if (RemoveAllIfLiteralExists(expression, LiteralType::False))
   {
      return;
   }

   // According to rule 4, evaluate expression to literal 0
   // if there exist pair x and !x.
   if (RemoveAllIfNegNotNegExists(expression, LiteralType::False))
   {
      return;
   }

   // According to rule 2, remove literal 1 if exist.
   RemoveLiteralIfExists(expression, LiteralType::True);

   // According to rule 3, remove all duplicates.
   RemoveDuplicates(expression);
}

void ExpressionEvaluator::EvaluateDisjunction(OperationExpression& expression)
{
   assert(m_children.size() > 1);

   // We have following rules:
   //    1. (x  | 1) = 1
   //    2. (x  | 0) = x
   //    3. (x  | x) = x
   //    4. (!x | x) = 1

   // According to rule 1, evaluate expression to literal 1 if it exist.
   if (RemoveAllIfLiteralExists(expression, LiteralType::True))
   {
      return;
   }

   // According to rule 4, evaluate expression to literal 1
   // if there exist pair x and !x.
   if (RemoveAllIfNegNotNegExists(expression, LiteralType::True))
   {
      return;
   }

   // According to rule 2, remove literal 0 if exist.
   RemoveLiteralIfExists(expression, LiteralType::False);

   // According to rule 3, remove all duplicates.
   RemoveDuplicates(expression);
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
   //    9. ( x ->  y -> x) =>  x - TODO

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

         if (LiteralType::False == m_children[0].m_literal ||
             m_children[0] == m_children[1])
         {
            m_children.erase(m_children.begin(), m_children.begin() + 2);
            expression.RemoveChildren(0, 2);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 4 perform transformation (!x -> 0) => x
         else if (OperationType::Negation == m_children[0].m_operation &&
                  LiteralType::False == m_children[1].m_literal)
         {
            m_children[0] = std::move(m_children[0].m_children[0]);
            MoveChildExpressionInplace(expression.GetChild(0));
            m_children.erase(m_children.begin() + 1);
            expression.RemoveChild(1);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
         // According to rule 6, we can remove first operand in case of (!x ->  x)
         // According to rule 7, we can remove first operand in case of ( x -> !x)

         else if ((OperationType::Negation == m_children[0].m_operation &&
                   m_children[0].m_children[0] == m_children[1]) ||
                  (OperationType::Negation == m_children[1].m_operation &&
                   m_children[1].m_children[0] == m_children[0]))
         {
            m_children.erase(m_children.begin());
            expression.RemoveChild(0);
            InPlaceNormalization(expression);
            was_evaluated = true;
         }
      } // while (m_children.size() > 1 && was_evaluated)

      // Other rules require at least 3 operands.
      if (m_children.size() < 3)
      {
         break;
      }

      // According to rule 8, we can remove two zero literals if they stay one after one.
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

      // It's possible following situation: x -> y -> (x -> y) -> z.
      // This expression we can group in the following way: (x -> y) -> (x -> y) -> z.
      // Then use rules 5 and 1 and evaluate this to z.

      for (long i = m_children.size() - 1; i > 1; --i)
      {
         if (OperationType::Implication == m_children[i].m_operation &&
             m_children[i].m_children.size() == i)
         {
            long j = 0;
            for (; j < i; ++j)
            {
               if (m_children[j] != m_children[i].m_children[j])
               {
                  break;
               }
            }

            // If all are equal
            if (j == i)
            {
               m_children.erase(m_children.begin(), m_children.begin() + i + 1);
               expression.RemoveChildren(0, i + 1);
            }

            InPlaceNormalization(expression);
            was_evaluated = true;
            break;
         }
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

   // According to rule 3 and 1, remove duplicates
   // and assign leteral 1 if all operands were removed.
   if (AbsorbDuplicates(expression, LiteralType::True))
   {
      return;
   }

   // According to rule 5 absorb operands that equal up to negation operation,
   // count resulting 0 literals and reduce/add to the end.
   if (AbsorbNegNotNegs(expression, LiteralType::False, LiteralType::True))
   {
      return;
   }
   
   // According to rule 4 and 2, reduce even amount of negations
   // and reduce the only remaining negation (if exists) with leteral 0.
   AbsorbNegations(expression, LiteralType::False);
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

   // According to rule 3 and 1, remove duplicates
   // and assign leteral 0 if all operands were removed.
   if (AbsorbDuplicates(expression, LiteralType::False))
   {
      return;
   }
   
   // According to rule 5 absorb operands that equal up to negation operation,
   // count resulting 1 literals and reduce/add to the end.
   if (AbsorbNegNotNegs(expression, LiteralType::True, LiteralType::False))
   {
      return;
   }
   
   // According to rule 4 and 2, reduce even amount of negations
   // and reduce the only remaining negation (if exists) with leteral 1.
   AbsorbNegations(expression, LiteralType::True);
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
         if ((OperationType::Negation == m_children[i].m_operation &&
              m_children[i].m_children[0] == m_children[j]) ||
             (OperationType::Negation == m_children[j].m_operation &&
              m_children[j].m_children[0] == m_children[i]))
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
      m_children.resize(m_children.size() - 1);
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
      if (OperationType::Negation == m_children[index].m_operation)
      {
         if (-1 == prev_negation)
         {
            prev_negation = index;
         }
         else
         {
            {
               auto temp = std::move(m_children[prev_negation].m_children[0]);
               m_children[prev_negation] = std::move(temp);
            }
            {
               auto temp = std::move(m_children[index].m_children[0]);
               m_children[index] = std::move(temp);
            }
            MoveChildExpressionInplace(expression.GetChild(prev_negation));
            MoveChildExpressionInplace(expression.GetChild(index));
            prev_negation = -1;
            m_is_normalization_needed = true;
         }
      }
   }

   if (prev_negation != -1 && eq_to_neg_literal == m_children.back().m_literal)
   {
      {
         auto temp = std::move(m_children[prev_negation].m_children[0]);
         m_children[prev_negation] = std::move(temp);
      }
      MoveChildExpressionInplace(expression.GetChild(prev_negation));
      m_children.resize(m_children.size() - 1);
      expression.RemoveChild(m_children.size());
      m_is_normalization_needed = true;
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
         if ((OperationType::Negation == m_children[i].m_operation &&
              m_children[i].m_children[0] == m_children[j]) ||
             (OperationType::Negation == m_children[j].m_operation &&
              m_children[j].m_children[0] == m_children[i]))
         {
            ++absorption_count;
            m_children.erase(m_children.begin() + j);
            m_children.erase(m_children.begin() + i);
            expression.RemoveChild(j);
            expression.RemoveChild(i);
            child_count -= 2;
            continue;
         }
         ++j;
      }
      ++i;
   }

   // If absorption count is odd
   if ((absorption_count & 1) == 1)
   {
      if (child_count > 0 &&
          m_children.back().m_literal == eq_to_neg_literal)
      {
         m_children.pop_back();
         expression.RemoveChild(child_count - 1);
         --child_count;
      }
      else
      {
         m_children.resize(m_children.size() + 1);
         m_children.back().m_literal = eq_to_neg_literal;
         expression.AddChild(std::make_unique<LiteralExpression>(eq_to_neg_literal));
         ++child_count;
      }
   }

   if (0 == child_count)
   {
      m_evaluated_expression = std::make_unique<LiteralExpression>(remaining_literal);
      return true;
   }

   return false;
}

void ExpressionEvaluator::InPlaceNormalization(OperationExpression& expression)
{
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

} // namespace

void EvaluateExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);
   ExpressionEvaluator evaluator;
   evaluator.Evaluate(expression);
}

} // namespace dm
