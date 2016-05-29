#pragma once

#include "expression_base.h"
#include "../common/named_entity.h"

namespace dm
{

class VariableDeclaration;

class ParamRefExpression : public TypedExpression<ExpressionType::ParamRef>
{
   using Base = TypedExpression<ExpressionType::ParamRef>;

public:
   ParamRefExpression(const VariableDeclaration& variable, long index);

   const NamedEntity& GetParamRef() const;
   long GetParamIndex() const;

   // IStringable
   virtual std::string ToString() const override;

   // Expression
   virtual TExpressionPtr Clone() const override;
   virtual TExpressionPtr CloneWithSubstitution(const TExpressionPtrVector& actual_params) const override;
   virtual void Accept(ExpressionVisitor& visitor) override;
   virtual void Accept(ConstExpressionVisitor& visitor) const override;

protected:
   // Expression
   virtual bool IsEqualToTheSameType(const Expression& rhs) const override;

private:
   ParamRefExpression(const ParamRefExpression& rhs) = default;
   ParamRefExpression& operator=(const ParamRefExpression& rhs) = delete;

private:
   const VariableDeclaration& m_variable;
   long m_index;
};

} // namespace dm
