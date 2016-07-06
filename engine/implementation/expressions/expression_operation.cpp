#include "expression_operation.h"
#include "expression_literal.h"

#include <cassert>

namespace dm
{

OperationExpression::OperationExpression(
   TExpressionPtr&& child) :
      Base(),
      m_operation(OperationType::Negation),
      m_children()
{
   m_children.push_back(std::move(child));
}

OperationExpression::OperationExpression(
   OperationType operation, TExpressionPtrVector&& children) :
      Base(),
      m_operation(operation), 
      m_children(std::move(children))
{
   assert
   (
      (OperationType::Negation == m_operation && m_children.size() == 1) ||
      (OperationType::Negation != m_operation && m_children.size() > 1) 
   );
}

OperationExpression::OperationExpression(const OperationExpression& rhs):
   Base(),
   m_operation(rhs.m_operation),
   m_children()
{
   m_children.reserve(rhs.m_children.size());
   for (const auto& child : rhs.m_children)
   {
      m_children.push_back(child->Clone());
   }   
}

OperationExpression::OperationExpression(
   const OperationExpression& rhs, const TExpressionPtrVector& actual_params) :
      Base(),
      m_operation(rhs.m_operation),
      m_children()
{
   m_children.reserve(rhs.m_children.size());
   for (const auto& child : rhs.m_children)
   {
      m_children.push_back(std::move(child->CloneWithSubstitution(actual_params)));
   }   
}

OperationType OperationExpression::GetOperation() const
{
   return m_operation;
}

void OperationExpression::SetOperation(OperationType operation)
{
   assert(operation != OperationType::None);
   m_operation = operation;
}

long OperationExpression::GetChildCount() const
{
   return m_children.size();
}

const TExpressionPtr& OperationExpression::GetChild(long index) const
{
   assert(index >=0 && index < (long)m_children.size());
   return m_children[index];
}

TExpressionPtr& OperationExpression::GetChild(long index)
{
   assert(index >=0 && index < (long)m_children.size());
   return m_children[index];
}

void OperationExpression::AddChild(TExpressionPtr&& expression)
{
   m_children.push_back(std::move(expression));
}

void OperationExpression::InsertChild(long index, TExpressionPtr&& expression)
{
   assert(index >=0 && index <= (long)m_children.size());
   m_children.insert(m_children.begin() + index, std::move(expression));
}

void OperationExpression::InsertChildren(long index, TExpressionPtrVector&& expressions)
{
   assert(index >=0 && index <= (long)m_children.size());
   m_children.reserve(m_children.size() + expressions.size());
   for (auto& expression : expressions)
   {
      m_children.insert(m_children.begin() + index, std::move(expression));
      ++index;
   }
   expressions.clear();
}

void OperationExpression::RemoveChild(long index)
{
   assert(index >=0 && index < (long)m_children.size());
   m_children.erase(m_children.begin() + index);
}

void OperationExpression::RemoveChildren(long indexFrom, long indexTo)
{
   assert(indexFrom >=0 && indexFrom < (long)m_children.size());
   assert(indexTo >=0 && indexTo <= (long)m_children.size());
   m_children.erase(m_children.begin() + indexFrom, m_children.begin() + indexTo);
}

std::string OperationExpression::ToString() const
{
   std::string result;
      
   if (OperationType::Negation == m_operation)
   {
      result += OperationTypeToString(m_operation);
      result += m_children.at(0)->ToString();
   }
   else
   {   
      std::string operation_str = 
         std::string(1, ' ') + OperationTypeToString(m_operation) + std::string(1, ' ');

      result += "(";

      bool first = true;
      for (const auto& child : m_children)
      {
         if (first)
         {
            first = false;
         }
         else
         {
            result += operation_str;
         }
         result += child->ToString();
      }

      result += ")";
   }

   return result;
}

TExpressionPtr OperationExpression::Clone() const
{
   return TExpressionPtr(new OperationExpression(*this));
}

TExpressionPtr OperationExpression::CloneWithSubstitution(
   const TExpressionPtrVector& actual_params) const
{
   return TExpressionPtr(new OperationExpression(*this, actual_params));
}

} // namespace dm
