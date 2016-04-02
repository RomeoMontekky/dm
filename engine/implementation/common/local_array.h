#pragma once

#ifdef _MSC_VER
   #include <malloc.h>
#endif

#ifdef _MSC_VER
   #define LOCAL_ARRAY(TYPE, NAME, SIZE) TYPE* const NAME = (TYPE*)_alloca(SIZE * sizeof(TYPE))
#else
   #define LOCAL_ARRAY(TYPE, NAME, SIZE) TYPE NAME[SIZE]
#endif
