#pragma once

#include <engine/istringable.h>

#include "expression_base.h"
#include "../common/named_object.h"

namespace dm
{

class ParamRefExpression : public Expression
{
public:
   ParamRefExpression(const NamedObject& ref, long index);

   const NamedObject& GetParamRef() const;
   long GetParamIndex() const;

   // IStringable
   virtual std::string ToString() const override;

   // Expression
   virtual TExpressionPtr Clone() const override;
   virtual TExpressionPtr CloneWithSubstitution(const TExpressionPtrVector& actual_params) const override;
   virtual void Accept(ExpressionVisitor& visitor) override;
   virtual void Accept(ConstExpressionVisitor& visitor) const override;

private:
   ParamRefExpression(const ParamRefExpression& rhs) = default;
   ParamRefExpression& operator=(const ParamRefExpression& rhs) = delete;

private:
   const NamedObject& m_ref;
   long m_index;
};

}; // namespace dm