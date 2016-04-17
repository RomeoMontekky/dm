#include "expression_evaluator.h"
#include "expression_visitor.h"
#include "expressions.h"
#include "expression_mover.h"

#include <map>
#include <cassert>

namespace dm
{

namespace
{

class ExpressionEvaluator : public ExpressionVisitor
{
public:
   ExpressionEvaluator();
   ExpressionEvaluator(ExpressionEvaluator&& rhs) = default;
   ExpressionEvaluator& operator =(ExpressionEvaluator&& rhs) = default;

   void Reset();
   TExpressionPtr& GetEvaluatedExpression();

   // ConstExpressionVisitor
   virtual void Visit(LiteralExpression& expression) override;
   virtual void Visit(ParamRefExpression& expression) override;
   virtual void Visit(OperationExpression& expression) override;

   bool operator ==(const ExpressionEvaluator& rhs) const;

private:
   void EvaluateNegation(OperationExpression& expression);
   void EvaluateConjunction(OperationExpression& expression);
   void EvaluateDisjunction(OperationExpression& expression);
   void EvaluateImplication(OperationExpression& expression);
   void EvaluateEquality(OperationExpression& expression);
   void EvaluatePlus(OperationExpression& expression);

private:
   bool RemoveAllIfLiteralExists(OperationExpression& expression, LiteralType literal);
   void RemoveLiteralIfExists(OperationExpression& expression, LiteralType literal);
   void RemoveDuplicates(OperationExpression& expression);
   bool AbsorbDuplicates(OperationExpression& expression, LiteralType remaining_literal);
   void AbsorbNegations(OperationExpression& expression, LiteralType eq_to_neg_literal);

private:
   // Following three fields hold values for three possible variants
   // of visited expressions.
   LiteralType m_literal;
   long m_param_index;
   OperationType m_operation;

   // Contains visitors for child expressions if current visited is operation expression.
   std::vector<ExpressionEvaluator> m_children;

   // Will be filled by evaluated new expression if the whole
   // operation expression was evaluated to some simple form.
   TExpressionPtr m_evaluated_expression;
};

ExpressionEvaluator::ExpressionEvaluator() :
    m_literal(LiteralType::None),
    m_param_index(-1),
    m_operation(OperationType::None),
    m_children(),
    m_evaluated_expression()
{
}

void ExpressionEvaluator::Reset()
{
   m_literal = LiteralType::None;
   m_param_index = -1;
   m_operation = OperationType::None;
   m_children.clear();
   m_evaluated_expression.reset();
}

TExpressionPtr& ExpressionEvaluator::GetEvaluatedExpression()
{
   return m_evaluated_expression;
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
      auto& child_expression = expression.GetChild(index);
      child_expression->Accept(m_children[index]);
      auto& evaluated_expression = m_children[index].GetEvaluatedExpression();
      if (evaluated_expression.get() != nullptr)
      {
         child_expression = std::move(evaluated_expression);
         m_children[index].Reset();
         child_expression->Accept(m_children[index]);
      }
   }

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

bool ExpressionEvaluator::operator ==(const ExpressionEvaluator& rhs) const
{
   return
   (
      (m_literal == rhs.m_literal) &&
      (m_param_index == rhs.m_param_index) &&
      (m_operation == rhs.m_operation) &&
      (m_children == rhs.m_children)
   );
}

void ExpressionEvaluator::EvaluateNegation(OperationExpression& expression)
{
   assert(m_children.size() == 1);

   // Removing double negation
   if (m_children[0].m_operation == OperationType::Negation)
   {
      auto moved_children = MoveChildExpressions(expression.GetChild(0));
      assert(moved_children.size() == 1);
      m_evaluated_expression = std::move(moved_children[0]);
   }
}

void ExpressionEvaluator::EvaluateConjunction(OperationExpression& expression)
{
   assert(m_children.size() > 1);
   if (!RemoveAllIfLiteralExists(expression, LiteralType::False))
   {
      RemoveLiteralIfExists(expression, LiteralType::True);
      RemoveDuplicates(expression);
   }
}

void ExpressionEvaluator::EvaluateDisjunction(OperationExpression& expression)
{
   assert(m_children.size() > 1);
   if (!RemoveAllIfLiteralExists(expression, LiteralType::True))
   {
      RemoveLiteralIfExists(expression, LiteralType::False);
      RemoveDuplicates(expression);
   }
}

void ExpressionEvaluator::EvaluateImplication(OperationExpression& expression)
{
   assert(m_children.size() > 1);

   // As implication is not commutative/associative, we are able
   // to evaluate operation expression only from left to right.
   // This is why we may not reuse utility methods that are available
   // in other EvaluateXXX methods.

   // We have following rules of evaluation:
   //   1. ( 1 ->  x) => x
   //   2. ( 0 ->  x) => 1
   //   3. ( x ->  1) => 1
   //   4. (!x ->  0) => x
   //   5. ( x ->  x) => 1
   //   6. (!x ->  x) => x
   //   7. ( x -> !x) => !x

   // According to rules 3 and 1, if there exist "1" operand,
   // we can remove all operands to the left, including this "1" operand.

   long child_count = m_children.size();
   for (long index = child_count - 1; index >= 0; --index)
   {
      if (LiteralType::True == m_children[index].m_literal)
      {
         m_children.erase(m_children.cbegin(), m_children.cbegin() + index + 1);
         expression.RemoveChildren(0, index + 1);
         child_count -= index + 1;
      }
   }

   while (child_count > 1)
   {
      // According to rules 2 and 1, we can remove subexpression
      // (0 -> x) if it is at the beginning of the expression.
      if (LiteralType::False = m_children[0].m_literal)
      {
         m_children.erase(m_children.cbein(), m_children.cbein() + 2);
         expression.RemoveChildren(0, 2);
         child_count -= 2;
      }
      // TODO: Appliying of the rule 4 and others
   }
}

void ExpressionEvaluator::EvaluateEquality(OperationExpression& expression)
{
   assert(m_children.size() > 1);
   RemoveLiteralIfExists(expression, LiteralType::True);
   if (!AbsorbDuplicates(expression, LiteralType::True))
   {
      AbsorbNegations(expression, LiteralType::False);
      AbsorbDuplicates(expression, LiteralType::True);
   }
}

void ExpressionEvaluator::EvaluatePlus(OperationExpression& expression)
{
   assert(m_children.size() > 1);
   RemoveLiteralIfExists(expression, LiteralType::False);
   if (!AbsorbDuplicates(expression, LiteralType::False))
   {
      AbsorbNegations(expression, LiteralType::True);
      AbsorbDuplicates(expression, LiteralType::False);
   }
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
            m_children.erase(m_children.cbegin() + j);
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
            m_children.erase(m_children.cbegin() + j);
            m_children.erase(m_children.cbegin() + i);
            expression.RemoveChild(j);
            expression.RemoveChild(i);
            --j;
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
            m_children[prev_negation] =
               std::move(m_children[prev_negation].m_children[0]);
            m_children[index] =
               std::move(m_children[index].m_children[0]);
            expression.GetChild(prev_negation) =
               std::move(MoveChildExpressions(expression.GetChild(prev_negation))[0]);
            expression.GetChild(index) =
               std::move(MoveChildExpressions(expression.GetChild(index))[0]);
            prev_negation = -1;
         }
      }
   }

   if (prev_negation != -1 && eq_to_neg_literal == m_children.back().m_literal)
   {
      m_children[prev_negation] =
         std::move(m_children[prev_negation].m_children[0]);
      expression.GetChild(prev_negation) =
         std::move(MoveChildExpressions(expression.GetChild(prev_negation))[0]);
      m_children.resize(m_children.size() - 1);
      expression.RemoveChild(m_children.size());
   }
}

} // namespace

void EvaluateExpression(TExpressionPtr& expression)
{
   assert(expression.get() != nullptr);

   ExpressionEvaluator evaluator;
   expression->Accept(evaluator);

   // The visitor evaluates only child expressions of each operation expression,
   // so the root can be still not evaluated. Let's correct this if so.
   auto& evaluated_expression = evaluator.GetEvaluatedExpression();
   if (evaluated_expression.get() != nullptr)
   {
      expression = std::move(evaluated_expression);
   }
}

} // namespace dm
