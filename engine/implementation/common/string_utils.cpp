#include "string_utils.h"

#include <cstring>
#include <cctype>
#include <cassert>

namespace dm
{

StringPtrLen::StringPtrLen() :
   m_ptr(nullptr), m_len(-1)
{
}

StringPtrLen::StringPtrLen(const char* ptr, long len) :
   m_ptr(ptr), m_len(len)   
{
   if (-1 == m_len)
   {
      m_len = std::strlen(ptr);
   }
}

StringPtrLen::operator std::string() const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   return std::string(m_ptr, m_len);
}

const char* StringPtrLen::Ptr() const
{
   return m_ptr;
}

long StringPtrLen::Len() const
{
   return m_len;
}

const char* StringPtrLen::Begin() const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   return m_ptr;
}

const char* StringPtrLen::End() const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   return (m_ptr + m_len);
}

char StringPtrLen::At(long index) const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);
   assert(index >=0 && index < m_len);

   return *(m_ptr + index);
}

StringPtrLen StringPtrLen::Left(const char* boundary) const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);
   assert(boundary >= m_ptr);

   return StringPtrLen(m_ptr, boundary - m_ptr);
}

StringPtrLen StringPtrLen::Right(const char* boundary) const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);
   assert(boundary >= m_ptr);

   return StringPtrLen(boundary, m_len + m_ptr - boundary);
}

void StringPtrLen::Assign(const char* ptr, long len)
{
   m_ptr = ptr;
   m_len = len;

   assert(m_ptr != nullptr);
   assert(m_len != -1);
}

void StringPtrLen::Reset()
{
   m_ptr = nullptr;
   m_len = -1;
}

void StringPtrLen::RemoveLeft(long count)
{
   assert(m_ptr != nullptr);
   assert(count >= 0);
   assert(m_len >= count);

   m_ptr += count;
   m_len -= count;
}

void StringPtrLen::RemoveRight(long count)
{
   assert(m_ptr != nullptr);
   assert(count >= 0);
   assert(m_len >= count);

   m_len -= count;
}

void StringPtrLen::TrimLeft()
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   while (std::isspace(*m_ptr))
   {
      ++m_ptr;
      --m_len;
   }
}

void StringPtrLen::TrimRight()
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   const char* last = m_ptr + m_len - 1;
   while (last >= m_ptr && std::isspace(*last))
   {
      --last;
      --m_len;
   }
}

void StringPtrLen::Trim()
{
   TrimRight();
   TrimLeft();
}

bool StringPtrLen::StartsWith(const char* str) const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   const char* ptr = m_ptr;
   const char* const end = m_ptr + m_len;
   for (; ptr != end && *str != '\0'; ++ptr, ++str)
   {
      if (*ptr != *str)
      {
         return false;
      }
   }

   return (*str == '\0');
}

bool StringPtrLen::Equals(const char* str) const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   const char* ptr = m_ptr;
   const char* const end = m_ptr + m_len;
   for (; ptr != end && *str != '\0'; ++ptr, ++str)
   {
      if (*ptr != *str)
      {
         return false;
      }
   }

   return (ptr == end && *str == '\0');
}

const char* StringPtrLen::Find(char ch) const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   const char* ptr = m_ptr;
   const char* const end = m_ptr + m_len;
   for (; ptr != end; ++ptr)
   {
      if (*ptr == ch)
      {
         return ptr;
      }
   }

   return nullptr;
}

const char* StringPtrLen::FindBackward(char ch) const
{
   assert(m_ptr != nullptr);
   assert(m_len != -1);

   const char* last = m_ptr + m_len - 1;
   for (; last >= m_ptr; --last)
   {
      if (*last == ch)
      {
         return last;
      }
   }

   return nullptr;
}

std::ostream& operator<<(std::ostream& os, const StringPtrLen& str)
{
   return os.write(str.Ptr(), str.Len());
}

}; // namespace dm