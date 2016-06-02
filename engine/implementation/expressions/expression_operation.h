#pragma once

#include "expression_base.h"
#include "../common/operations.h"

namespace dm
{

class OperationExpression : public TypedExpression<ExpressionType::Operation>
{
   using Base = TypedExpression<ExpressionType::Operation>;

public:
   OperationExpression(TExpressionPtr&& child);
   OperationExpression(OperationType operation, TExpressionPtrVector&& children);

   OperationType GetOperation() const;
   long GetChildCount() const;
   const TExpressionPtr& GetChild(long index) const;
   TExpressionPtr& GetChild(long index);

   void AddChild(TExpressionPtr&& expression);
   void InsertChild(long index, TExpressionPtr&& expression);
   void InsertChildren(long index, TExpressionPtrVector&& expressions);
   void RemoveChild(long index);
   void RemoveChildren(long indexFrom, long indexTo);
   
   // Checks that first size children have one-to-one accordance with
   // first size children of specified operation expression.
   bool AreFirstChildrenEqual(const OperationExpression& rhs, long size) const;

   // IStringable
   virtual std::string ToString() const override;

   // Expression
   virtual TExpressionPtr Clone() const override;
   virtual TExpressionPtr CloneWithSubstitution(const TExpressionPtrVector& actual_params) const override;

protected:
   // Expression
   virtual bool IsEqualToTheSameType(const Expression& rhs) const override;

private:
   OperationExpression(const OperationExpression& rhs);
   OperationExpression(const OperationExpression& rhs, const TExpressionPtrVector& actual_params);
   OperationExpression& operator=(const OperationExpression& rhs) = delete;

private:
   OperationType m_operation;
   TExpressionPtrVector m_children;
};

} // namespace dm
