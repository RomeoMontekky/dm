#include "expression_evaluator.h"
#include "expression_visitor.h"
#include "expressions.h"
#include "expression_mover.h"

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
   // Following three fields hold values for three possible variants
   // of visited expressions.
   LiteralType m_literal;
   long m_param_index;
   OperationType m_operation;

   // Contains visitors for child expressions if current visited operation expression.
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

   // TODO: Evaluation depending on operation.
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
}

} // namespace dm
