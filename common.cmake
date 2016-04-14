cmake_minimum_required(VERSION 3.0)

macro(set_options_and_post_build_steps)
   if (MINGW)
      if (CMAKE_BUILD_TYPE STREQUAL "Release")
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++14")
         add_custom_command(
            TARGET ${BINARY_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} ARGS "--strip-unneeded" "$<TARGET_FILE:${BINARY_NAME}>")
      else()
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -std=c++14")
      endif()
   else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS")
   endif()
   
   add_custom_command(
      TARGET ${BINARY_NAME} POST_BUILD 
      COMMAND ${CMAKE_COMMAND} -E "make_directory" ARGS "${CMAKE_SOURCE_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E "copy" ARGS "$<TARGET_FILE:${BINARY_NAME}>" "${CMAKE_SOURCE_DIR}/bin")
endmacro(set_options_and_post_build_steps)
   
   