cmake_minimum_required (VERSION 3.1)
project (mj-sdr)
# The version number.
set (mj-sdr_VERSION_MAJOR 0)
set (mj-sdr_VERSION_MINOR 1)

file(GLOB mj-sdr_SRC
    "src/*.h"
    "src/*.cpp"
)

set (CMAKE_CXX_STANDARD 11)
 
# add the executable
add_executable(mj-sdr ${mj-sdr_SRC})
target_link_libraries(mj-sdr rtlsdr pthread)
