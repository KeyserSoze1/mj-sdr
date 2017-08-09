cmake_minimum_required (VERSION 3.1)
project (mj-sdr)
include (ExternalProject)

# The version number.
set (mj-sdr_VERSION_MAJOR 0)
set (mj-sdr_VERSION_MINOR 1)

file(GLOB mj-sdr_SRC
    "src/*.cpp"
)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set (RTLSDR_PREFIX "${CMAKE_BINARY_DIR}/rtlsdr-prefix")
ExternalProject_Add (
    rtlsdr_external
    GIT_REPOSITORY "https://github.com/osmocom/rtl-sdr.git"
    GIT_TAG "master"
    PREFIX ${RTLSDR_PREFIX}

    INSTALL_COMMAND ""
)

find_library(LIBUSB_LIBRARY usb-1.0)
add_library (rtlsdr STATIC IMPORTED)
set_property (TARGET rtlsdr PROPERTY IMPORTED_LOCATION "${RTLSDR_PREFIX}/src/rtlsdr_external-build/src/librtlsdr.a")
add_dependencies (rtlsdr rtlsdr_external)
include_directories("${RTLSDR_PREFIX}/src/rtlsdr_external/include")
set (RTLSDR_LIBRARY rtlsdr)
 
# add the executable
add_executable(mj-sdr ${mj-sdr_SRC})
target_link_libraries(mj-sdr ${RTLSDR_LIBRARY} ${LIBUSB_LIBRARY} pthread)
