#pragma once

#include "string_utils.h"

namespace dm
{

class BracketsBalancer
{
public:
   BracketsBalancer();

   // Returns whether the input char was processed or not
   bool ProcessChar(char ch);
   void ProcessEnding();

   long GetBalance() const;
   long GetPossibleTrimmings() const;

private:
   long m_balance;
   long m_possible_trimmings;
};

class BracketsContent
{
public:
   BracketsContent();

   // If parsing is successful, then returns string before
   // opening bracket. Otherwise returns the whole string.
   StringPtrLen Parse(const StringPtrLen& str);

   bool GetPart(StringPtrLen& part);

private:
   StringPtrLen m_content;
};

/////// Utilities ///////

void CheckBracketBalance(const StringPtrLen& str);
void TrimBrackets(StringPtrLen& str);

const char* FindWithZeroBalance(const StringPtrLen& str, const char* sub);
const char* FindWithZeroBalance(const StringPtrLen& str, char ch);

} // namespace dm
