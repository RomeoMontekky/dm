#pragma once

namespace dm
{

class LiteralExpression;
class ParamRefExpression;
class OperationExpression;

class ExpressionVisitor
{
public:
   ExpressionVisitor();
   virtual ~ExpressionVisitor();

   virtual void Visit(LiteralExpression& expression);
   virtual void Visit(ParamRefExpression& expression);
   virtual void Visit(OperationExpression& expression);
};

class ConstExpressionVisitor
{
public:
   ConstExpressionVisitor();
   virtual ~ConstExpressionVisitor();

   virtual void Visit(const LiteralExpression& expression);
   virtual void Visit(const ParamRefExpression& expression);
   virtual void Visit(const OperationExpression& expression);
};


}; // namespace dm