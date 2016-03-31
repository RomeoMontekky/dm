#pragma once

#include <memory>

#ifdef _MSC_VER
   #define LOCAL_ARRAY(TYPE, NAME, SIZE) \
      std::unique_ptr<TYPE[]> NAME##_ptr = std::make_unique<TYPE[]>(SIZE); \
      TYPE* const NAME = NAME##_ptr.get()
#else
   #define LOCAL_ARRAY(TYPE, NAME, SIZE) \
      TYPE NAME[SIZE]
#endif
