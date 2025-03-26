cmake_minimum_required(VERSION 3.10)
project(bahasa)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add source files
add_executable(bahasa
    src/main.cpp
    src/lexer/Lexer.cpp
    src/lexer/Parser.cpp
    src/ast/AST.cpp
    src/codegen/Codegen.cpp
    src/ast/ASTPrinter.cpp
)

# Link against LLVM libraries
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(bahasa ${llvm_libs})

# Include source directories
target_include_directories(bahasa PRIVATE ${PROJECT_SOURCE_DIR}/src) 