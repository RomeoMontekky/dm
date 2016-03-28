#include <engine/iengine.h>
#include <engine/iexception.h>

#include <string>
#include <iostream>

int main()
{
   dm::TIEnginePtr engine = dm::CreateEngine();

   std::cerr << "Enter commands to interact with the engine. Enter 'exit' to quit the program." << std::endl;

   std::string str;
   while (true)
   {
      std::getline(std::cin, str);
      if (str == "exit")
      {
         break;
      }

      try
      {
         const dm::IStringable& ret = engine->Process(str.c_str(), str.length());
         std::cout << ret.ToString() << std::endl;
      }
      catch (const dm::IException& ex)
      {
         std::cout << "Error: " << ex.GetDescription() << std::endl;
      }
   }

   return 0;
}

