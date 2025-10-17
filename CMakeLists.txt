cmake_minimum_required(VERSION 3.16)
project(embedded_alice VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
  abseil
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20250127.1
)
FetchContent_MakeAvailable(abseil)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

file(GLOB DSP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/dsp/*.c
)

add_library(dsp STATIC ${DSP_SOURCES})

target_include_directories(dsp PUBLIC
    ${CMAKE_SOURCE_DIR}/src     # contains dsp/ folder
)

add_executable(embedded_alice
  src/main.cc
)
target_include_directories(embedded_alice PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(embedded_alice PRIVATE dsp absl::flags absl::flags_parse absl::log)
target_compile_options(embedded_alice PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_subdirectory(tests)
