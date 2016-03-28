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

   virtual void Visit(LiteralExpression& expression) = 0;
   virtual void Visit(ParamRefExpression& expression) = 0;
   virtual void Visit(OperationExpression& expression) = 0;
};

class ConstExpressionVisitor
{
public:
   ConstExpressionVisitor();
   virtual ~ConstExpressionVisitor();

   virtual void Visit(const LiteralExpression& expression) = 0;
   virtual void Visit(const ParamRefExpression& expression) = 0;
   virtual void Visit(const OperationExpression& expression) = 0;
};


}; // namespace dm