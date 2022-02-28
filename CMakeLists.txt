cmake_minimum_required(VERSION 3.22.1)

project("Reduced C Compiler"
    VERSION 0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()

### Mini Stack Machine ###
add_executable(msm MiniStackMachine/src/msm.c)
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/MiniStackMachine/Debug-x64")
set_target_properties(msm
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/MiniStackMachine/Debug-x64"
)

### Argtable3 ###
add_library(argtable3 SHARED ReducedCCompiler/vendor/argtable3/argtable3.c)
target_include_directories(argtable3 PRIVATE ReducedCCompiler/vendor)
set_target_properties(argtable3
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/ReducedCCompiler/Debug-x64"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/ReducedCCompiler/Debug-x64"
)

### Reduced C Compiler ###
add_executable(rcc
    ReducedCCompiler/src/code_generation.c
    ReducedCCompiler/src/main.c
    ReducedCCompiler/src/semantic_analysis.c
    ReducedCCompiler/src/syntactic_analysis.c
    ReducedCCompiler/src/syntactic_node.c
    ReducedCCompiler/src/token.c
)
target_include_directories(rcc PRIVATE ReducedCCompiler/vendor)
if (MSVC)
    target_compile_definitions(rcc PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(rcc PRIVATE /W4 /WX)
else()
    target_compile_options(rcc PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
target_link_libraries(rcc PRIVATE argtable3)
if (NOT WIN32)
    target_link_libraries(rcc PRIVATE m)
endif()
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/ReducedCCompiler/Debug-x64")
set_target_properties(rcc
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/ReducedCCompiler/Debug-x64"
)

### Tests ###
if (NOT DEFINED NO_TESTS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_custom_target(test ${Python3_EXECUTABLE} ReducedCCompiler_Test.py
        DEPENDS rcc msm
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/ReducedCCompiler-Test"
    )
    add_custom_target(extratest ${Python3_EXECUTABLE} ReducedCCompiler_Test.py extratest
        DEPENDS rcc msm
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/ReducedCCompiler-Test"
    )
    file(GLOB_RECURSE FILES_TO_CLEAN "ReducedCCompiler-Test/*.txt" "ReducedCCompiler-Test/*.msm")
    file(GLOB DIR_TO_CLEAN "ReducedCCompiler-Test/*__pycache__" "ReducedCCompiler-Test/*/__pycache__")
    set_property(TARGET test extratest
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${FILES_TO_CLEAN}" "${DIR_TO_CLEAN}"
    )
endif()
