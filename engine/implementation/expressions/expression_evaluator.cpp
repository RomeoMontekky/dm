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

   void Reset();
   
   LiteralType GetLiteral() const;
   long GetParamIndex() const;
   OperationType GetOperation() const;
   
   long GetChildCount() const;
   ExpressionEvaluator& GetChild(long index);

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

LiteralType ExpressionEvaluator::GetLiteral() const
{
   return m_literal;
}

long ExpressionEvaluator::GetParamIndex() const
{
   return m_param_index;
}

OperationType ExpressionEvaluator::GetOperation() const
{
   return m_operation;
}

long ExpressionEvaluator::GetChildCount() const
{
   return m_children.size();
}

TExpressionPtr& ExpressionEvaluator::GetEvaluatedExpression()
{
   return m_evaluated_expression;
}

ExpressionEvaluator& ExpressionEvaluator::GetChild(long index)
{
   assert(index >= 0 && index < (long)m_children.size());
   return m_children[index];
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
   m_children.resize(expression.GetChildCount());

   const long child_count = expression.GetChildCount();
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
}

void ExpressionEvaluator::EvaluateNegation(OperationExpression& expression)
{
   assert(m_children.size() == 1);

   // Removing double negation
   if (m_children[0].GetOperation() == OperationType::Negation)
   {
      auto moved_children = MoveChildExpressions(expression.GetChild(0));
      assert(moved_children.size() == 1);
      m_evaluated_expression = std::move(moved_children[0]);
   }
}

void ExpressionEvaluator::EvaluateConjunction(OperationExpression& expression)
{
   int child_count = m_children.size();
   assert(child_count > 1);

   // If there exist operand "0", then all expression is "0"
   if (m_children[child_count - 1].GetLiteral() == LiteralType::False)
   {
      m_evaluated_expression = std::move(expression.GetChild(child_count - 1));
      return;
   }

   // If there exist operand "1", it should be removed.
   if (m_children[child_count - 1].GetLiteral() == LiteralType::True)
   {
      m_children.resize(child_count - 1);
      expression.RemoveChild(child_count - 1);
      --child_count;
   }

   // If there are two equal operands, one of them should be removed.
   for (long i = 0; i < child_count - 1; ++i)
      for (long j = i + 1; j < child_count; ++j)
         if (m_children[i] == m_children[j])
         {
            m_children.erase(m_children.cbegin() + j);
            expression.RemoveChild(j);
            --child_count;
            --j;
         }
}

void ExpressionEvaluator::EvaluateDisjunction(OperationExpression& expression)
{
   int child_count = m_children.size();
   assert(child_count > 1);

   // If there exist operand "1", then all expression is "1"
   if (m_children[child_count - 1].GetLiteral() == LiteralType::True)
   {
      m_evaluated_expression = std::move(expression.GetChild(child_count - 1));
      return;
   }

   // If there exist operand "0", it should be removed.
   if (m_children[child_count - 1].GetLiteral() == LiteralType::False)
   {
      m_children.resize(child_count - 1);
      expression.RemoveChild(child_count - 1);
      --child_count;
   }

   // If there are two equal operands, one of them should be removed.
   for (long i = 0; i < child_count - 1; ++i)
      for (long j = i + 1; j < child_count; ++j)
         if (m_children[i] == m_children[j])
         {
            m_children.erase(m_children.cbegin() + j);
            expression.RemoveChild(j);
            --child_count;
            --j;
         }
}

void ExpressionEvaluator::EvaluateImplication(OperationExpression& expression)
{

}

void ExpressionEvaluator::EvaluateEquality(OperationExpression& expression)
{

}

void ExpressionEvaluator::EvaluatePlus(OperationExpression& expression)
{

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
