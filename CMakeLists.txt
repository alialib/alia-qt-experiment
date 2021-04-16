project(alia-qt-experiment)
cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)

# Download/include CMake/Conan integration.
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
                  TLS_VERIFY ON)
endif()
include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

# Invoke Conan.
conan_cmake_run(
    REQUIRES
        qt/5.15.2
    BASIC_SETUP
    BUILD missing
    IMPORTS "bin, *.dll -> ./bin"
    IMPORTS "lib, *.dylib* -> ./bin"
    OPTIONS
    #     qt:openssl=False
    #     qt:with_pcre2=False
    #     qt:with_sqlite3=False
    #     qt:with_pq=False
    #     qt:with_odbc=False
    #     qt:with_openal=False
         qt:shared=True)

# Download alia.hpp and set it up as a library.
# TODO: Improve this!
set(alia_hpp_url
    https://github.com/alialib/alia/releases/download/0.8.0/alia.hpp)
file(DOWNLOAD ${alia_hpp_url}
    ${CMAKE_CURRENT_BINARY_DIR}/alia.hpp)
file(DOWNLOAD ${alia_hpp_url}
    ${CMAKE_CURRENT_BINARY_DIR}/alia.cpp)
add_library(alia ${CMAKE_CURRENT_BINARY_DIR}/alia.cpp)
target_compile_definitions(alia PRIVATE -DALIA_IMPLEMENTATION)
target_include_directories(alia PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

add_executable(sandbox WIN32 sandbox.cpp adaptor.cpp)

message("${CONAN_LIB_DIRS}")

target_link_libraries(sandbox ${CONAN_LIBS} alia)
target_link_directories(sandbox PUBLIC ${CONAN_LIB_DIRS})
target_compile_definitions(
    sandbox PUBLIC ${CONAN_DEFINES} ${CONAN_COMPILE_DEFINITIONS})
target_include_directories(sandbox PUBLIC
    ${CONAN_INCLUDE_DIRS})
