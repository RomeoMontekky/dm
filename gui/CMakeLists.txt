cmake_minimum_required(VERSION 3.0)

include("../common.cmake")

set(BINARY_NAME "gui")

set(CPP_FILES 
   "implementation/main.cpp"
   "implementation/window.cpp"
   "implementation/window_class.cpp"
)

set(HEADER_FILES
   "implementation/window.h"
   "implementation/window_class.h"
)

include_directories("../engine")
link_directories("../engine")

add_executable(${BINARY_NAME} ${CPP_FILES} ${HEADER_FILES})

target_link_libraries(${BINARY_NAME} "engine" "gdiplus")

# From "common.cmake"
set_options_and_post_build_steps()

# Tell linker that entry point is WinMain.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--subsystem,windows")