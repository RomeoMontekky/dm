#pragma once

#include <engine/istringable.h>

#include <memory>
#include <string>

namespace dm
{

class FunctionOutput;
using TFunctionOutputPtr = std::unique_ptr<FunctionOutput>;

class FunctionOutput : public IStringable
{
public:
   FunctionOutput();
   FunctionOutput(const std::string& line);
   FunctionOutput(const char* line);

   void AddLine(const std::string& line);
   void AddLine(const char* line);

   // IStringable
   virtual std::string ToString() const override;

private:
   std::string m_output;
};

} // namespace dm