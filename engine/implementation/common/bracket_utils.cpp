#include "bracket_utils.h"
#include "exception.h"

#include <cstring>
#include <cassert>

namespace dm
{

namespace
{

const char g_char_br_opened = '(';
const char g_char_br_closed = ')';
const char g_char_comma     = ',';

} // namespace

////////// BracketsBalancer //////////

BracketsBalancer::BracketsBalancer() : 
   m_balance(0), m_possible_trimmings(-1)
{
}

bool BracketsBalancer::ProcessChar(char ch)
{
   if (g_char_br_opened == ch)
   {
      // First opened brackets will be ignored due to the first part of condition,
      // so the whole condition will trigger only when at least one closed bracket
      // is occured before.

      if (m_possible_trimmings >= 0 && m_balance < m_possible_trimmings)
      {
         m_possible_trimmings = m_balance;
      }

      ++m_balance;
   }
   else if (g_char_br_closed == ch) 
   {
      // Last closed brackets will be ignored due to -1 check, so the variable
      // will be initialized by bracket balance of the first closed bracked.

      if (-1 == m_possible_trimmings)
      {
         m_possible_trimmings = m_balance;
      }

      if (--m_balance < 0)
      {
         Error("Closing bracket can't be before an opening one.");
      }
   }
   else
   {
      return false;
   }

   return true;
}

void BracketsBalancer::ProcessEnding()
{
   if (m_balance != 0)
   {
      Error("Closing bracket is missing.");
   }
}

long BracketsBalancer::GetBalance() const
{
   return m_balance;
}

long BracketsBalancer::GetPossibleTrimmings() const
{
   return m_possible_trimmings;
}

///////// BracketsContent ///////////

BracketsContent::BracketsContent() :
   m_content()
{
}

StringPtrLen BracketsContent::Parse(const StringPtrLen& str)
{
   m_content.Reset();

   const char* bracket_opened = str.Find(g_char_br_opened);
   // If opened bracket is absent, imply this fact as if
   // the whole obtained string is the name.
   if (nullptr == bracket_opened)
   {
      return str;
   }

   if (str.At(str.Len() - 1) != g_char_br_closed)
   {
      Error("Extra characters are detected after closing bracket.");
   }

   m_content = str.Right(bracket_opened + 1);
   m_content.RemoveRight(1);

   return str.Left(bracket_opened);
}

bool BracketsContent::GetPart(StringPtrLen& part)
{
   part.Reset();

   if (nullptr == m_content.Ptr())
   {
      return false;
   }

   const char* comma = FindWithZeroBalance(m_content, g_char_comma);
   if (comma != nullptr)
   {
      part = m_content.Left(comma);
      m_content = m_content.Right(comma + 1);
   }
   else
   {
      part = m_content;
      m_content.Reset();
   }

   return true;
}

/////////// Utilities /////////////

void CheckBracketBalance(const StringPtrLen& str)
{
   BracketsBalancer balancer;

   const char* curr = str.Begin();
   const char* const end = str.End();
   for (; curr != end; ++curr)
   {
      balancer.ProcessChar(*curr);
   }

   balancer.ProcessEnding();
}

static long CalculatePossibleTrimmings(const StringPtrLen& str)
{
   BracketsBalancer balancer;

   const char* curr = str.Begin();
   const char* const end = str.End();
   for (; curr != end; ++curr)
   {
      balancer.ProcessChar(*curr);
   }

   return balancer.GetPossibleTrimmings();
}

void TrimBrackets(StringPtrLen& str)
{
   str.Trim();

   long possible_trimmings = CalculatePossibleTrimmings(str);
   for (; possible_trimmings > 0; --possible_trimmings)
   {
      assert(str.Len() > 1);      
      if (g_char_br_opened == str.At(0) && 
          g_char_br_closed == str.At(str.Len() - 1))
      {
          str.RemoveLeft(1);
          str.RemoveRight(1);
          str.Trim();
      }
      else
      {
         break;
      }
   }
}

const char* FindWithZeroBalance(const StringPtrLen& str, const char* sub)
{
   StringPtrLen tail = str;
   const long sub_len = std::strlen(sub);

   BracketsBalancer balancer;
   while (tail.Len() >= sub_len)
   {
      if (!balancer.ProcessChar(tail.At(0)) &&
           balancer.GetBalance() == 0 && tail.StartsWith(sub))
      {
         return tail.Ptr();
      }
      tail.RemoveLeft(1);
   }
   return nullptr;
}

const char* FindWithZeroBalance(const StringPtrLen& str, char ch)
{
   assert(ch != g_char_br_opened && ch != g_char_br_closed);

   BracketsBalancer balancer;

   const char* curr = str.Begin();
   const char* const end = str.End();
   for (; curr != end; ++curr)
   {
      if (!balancer.ProcessChar(*curr) && balancer.GetBalance() == 0 && (*curr == ch))
      {
         return curr;
      }
   }
   return nullptr;
}

} // namespace dm
