cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "spectra")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${PROJECT_NAME})

if(NOT DEFINED SPECTRA_BUILD_PROFILE)
    set(SPECTRA_BUILD_PROFILE "debug")
endif()
add_compile_definitions(SPECTRA_FW_BUILD_PROFILE_CSTR=\"${SPECTRA_BUILD_PROFILE}\")

if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wall -Wextra -Wformat -Wformat-security)
endif()
