cmake_minimum_required(VERSION 3.20.0)

project("Reduced C Compiler"
    VERSION 0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
message("Build type: ${CMAKE_BUILD_TYPE}")

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()

### Mini Stack Machine ###
add_executable(msm MiniStackMachine/src/msm.c)

### Reduced C Compiler ###
add_executable(rcc
    ReducedCCompiler/src/code_generation.c
    ReducedCCompiler/src/main.c
    ReducedCCompiler/src/semantic_analysis.c
    ReducedCCompiler/src/syntactic_analysis.c
    ReducedCCompiler/src/syntactic_node.c
    ReducedCCompiler/src/token.c
    ReducedCCompiler/src/token.h
    # Argtable
    ReducedCCompiler/vendor/argtable3/argtable3.c
)
target_include_directories(rcc PRIVATE ReducedCCompiler/vendor)
if (MSVC)
    target_compile_definitions(rcc PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(rcc PRIVATE /W4 /WX)
else()
    target_compile_options(rcc PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
if (NOT WIN32)
    target_link_libraries(rcc PRIVATE m)
endif()

### Tests ###
if (NOT DEFINED NO_TESTS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_custom_target(test ${Python3_EXECUTABLE} ReducedCCompiler_Test.py
        DEPENDS rcc msm
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../ReducedCCompiler-Test"
    )
    add_custom_target(extratest ${Python3_EXECUTABLE} ReducedCCompiler_Test.py extra
        DEPENDS rcc msm
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../ReducedCCompiler-Test"
    )
    # file(GLOB_RECURSE FILES_TO_CLEAN "ReducedCCompiler-Test/*.txt" "ReducedCCompiler-Test/*.msm")
    # file(GLOB DIR_TO_CLEAN "ReducedCCompiler-Test/*__pycache__" "ReducedCCompiler-Test/*/__pycache__")
    # set_property(TARGET test extratest
    #     APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${FILES_TO_CLEAN}" "${DIR_TO_CLEAN}"
    # )
endif()
