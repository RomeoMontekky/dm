cmake_minimum_required(VERSION 3.0)

include("../common.cmake")

set(BINARY_NAME "console")

set(CPP_FILES 
   "implementation/main.cpp"
)

include_directories("../engine")
link_directories("../engine")

add_executable(${BINARY_NAME} ${CPP_FILES})

target_link_libraries(${BINARY_NAME} "engine")

# From "common.cmake"
set_options_and_post_build_steps()