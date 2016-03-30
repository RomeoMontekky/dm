#pragma once

#include "expression_base.h"
#include "../common/operations.h"

namespace dm
{

class OperationExpression : public Expression
{
public:
   OperationExpression(TExpressionPtr&& child);
   OperationExpression(OperationType operation, TExpressionPtrVector&& children);

   OperationType GetOperation() const;
   long GetChildCount() const;
   const TExpressionPtr& GetChild(long index) const;
   TExpressionPtr& GetChild(long index);

   void RemoveChild(long index);
   void InsertChild(long index, TExpressionPtr&& expression);
   void InsertChidren(long index, TExpressionPtrVector&& expressions);

   // IStringable
   virtual std::string ToString() const override;

   // Expression
   virtual TExpressionPtr Clone() const override;
   virtual TExpressionPtr CloneWithSubstitution(const TExpressionPtrVector& actual_params) const override;
   virtual void Accept(ExpressionVisitor& visitor) override;
   virtual void Accept(ConstExpressionVisitor& visitor) const override;

private:
   OperationExpression(const OperationExpression& rhs);
   OperationExpression(const OperationExpression& rhs, const TExpressionPtrVector& actual_params);
   OperationExpression& operator=(const OperationExpression& rhs) = delete;

private:
   OperationType m_operation;
   TExpressionPtrVector m_children;
};

}; // namespace dm