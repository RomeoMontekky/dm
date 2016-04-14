#include "engine.h"

namespace dm
{

IEngine::IEngine()
{
}

IEngine::~IEngine()
{
}

Engine::Engine() :
   m_variable_mgr(), m_parser(m_variable_mgr), m_caller(m_variable_mgr),
   m_last_unnamed_var(), m_last_function_output()
{
}

const IStringable& Engine::Process(const char* str, long len)
{
   StringPtrLen str_obj(str, len);
   
   str_obj.RemoveComment();

   if (str_obj.HasNoData())
   {
      static FunctionOutput empty_output;
      return empty_output;
   }

   if (IsFunctionCall(str_obj))
   {
      m_last_function_output = m_caller.ParseAndCall(str_obj);
      return *m_last_function_output;
   }

   TVariablePtr variable = m_parser.Parse(str_obj);
   if (variable->GetName().empty())
   {
      m_last_unnamed_var = std::move(variable);
      return *m_last_unnamed_var.get();
   }

   return m_variable_mgr.AddVariable(std::move(variable));
}

TIEnginePtr CreateEngine()
{
   return std::make_unique<Engine>();
}

}; // namespace dm
