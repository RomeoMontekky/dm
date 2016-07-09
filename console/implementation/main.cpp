#include <engine/iengine.h>
#include <engine/iexception.h>

#include <string>
#include <iostream>
#include <fstream>

int ProcessStream(std::istream& stream)
{
   auto engine = dm::CreateEngine();

   std::string str;
   while (!stream.eof())
   {
      std::getline(stream, str);
      if (str == "exit")
      {
         break;
      }

      try
      {
         const auto& ret = engine->Process(str.c_str(), str.length());
         auto output = ret.ToString();
         if (!output.empty())
         {
            std::cout << ret.ToString() << std::endl;
         }
      }
      catch (const dm::IException& ex)
      {
         std::cout << "Error: " << ex.GetDescription() << std::endl;
      }
   }

   return 0;
}

int main(int argc, char* argv[])
{
   if (argc > 2)
   {
      std::cerr << "Wrong number of parameters." << std::endl;
      return 1;
   }

   if (1 == argc)
   {
      std::cout << "DM Console utility. Copyright (c) 2016 Roman Lapitsky." << std::endl 
                << std::endl
                << "Enter commands to interact with the engine. Enter 'exit' to quit the program." << std::endl
                << std::endl;
      return ProcessStream(std::cin);
   }

   // Process commands from file
   std::ifstream ifstr(argv[1]);
   if (!ifstr.is_open())
   {
      std::cerr << "Cannot open file '" << argv[1] << "'." << std::endl;
      return 2;
   }

   return ProcessStream(ifstr);
}
