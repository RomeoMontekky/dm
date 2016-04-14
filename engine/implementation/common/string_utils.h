#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace dm
{

struct StringPtrLen
{
public:
   StringPtrLen();
   StringPtrLen(const char* ptr, long len);

   operator std::string() const;

   const char* Ptr() const;
   long Len() const;
   const char* Begin() const;
   const char* End() const;
   char At(long index) const;
   
   StringPtrLen Left(const char* boundary) const;
   StringPtrLen Right(const char* boundary) const;

   void Assign(const char* ptr, long len);
   void Reset();
   void RemoveLeft(long count);
   void RemoveRight(long count);

   void TrimLeft();
   void TrimRight();
   void Trim();

   void RemoveComment();

   // Variable str is null terminated
   bool StartsWith(const char* str) const;
   bool Equals(const char* str) const;

   const char* Find(char ch) const;
   const char* FindBackward(char ch) const;
   
   bool HasNoData() const;

private:
   const char* m_ptr;
   long m_len;
};

using TStringPtrLenVector = std::vector<StringPtrLen>;

std::ostream& operator<<(std::ostream& os, const StringPtrLen& str);

} // namespace dm

