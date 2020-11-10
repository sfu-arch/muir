cmake_minimum_required(VERSION 3.10)
project(xketch-generator)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}")

set(PACKAGE_NAME dandelion-gen)
set(PACKAGE_VERSION 0.1)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "amiralis@sfu.ca")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++14 -fno-rtti -Wall -fvisibility=hidden -fdiagnostics-color=always")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++14 -g -fno-rtti -Wall \
    -fno-omit-frame-pointer -Wno-unused-function -fdiagnostics-color=always")

if(APPLE)
    set(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup")
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

############## EXTERNAL PROJECT ##################
# Global settings
set(GLOBAL_OUTPUT_PATH   ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)

# 3rdparty Libraries
#include(CMakeLists.txt.in)

############## LLVM CONFIGURATION #################

# LLVM_DIR must be set to the prefix of /share/llvm/cmake via commandline
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# We incorporate the CMake features provided by LLVM:
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

option(LLVM_ENABLE_CXX1Y  "Enable C++14" ON)
option(LLVM_INCLUDE_TOOLS "Generate build targets for the LLVM tools." ON)
option(LLVM_BUILD_TOOLS
  "Build the LLVM tools. If OFF, just generate build targets." ON
)

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Binaries    ${LLVM_BINARY_DIR}/bin
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

# Now set the LLVM header and library paths:
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
add_definitions(${LLVM_DEFINITIONS})

############## LLVM CONFIGURATION #################

option(TAPIR "Tapir Cilk extension" OFF)
if(TAPIR)
    add_definitions(-DTAPIR)
endif()

############## FINAL PROJECT CONFIG #################

# And the project header and library paths
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${LIBRARY_OUTPUT_PATH})
set(CMAKE_TEMP_LIBRARY_PATH "${PROJECT_BINARY_DIR}/lib")

# TODO: Add install path to the list....


#configure_file("${CMAKE_SOURCE_DIR}/tests" "${CMAKE_CURRENT_BINARY_DIR}/" @COPYONLY)
file(COPY ${CMAKE_SOURCE_DIR}/tests DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
configure_file("${CMAKE_SOURCE_DIR}/tests/c/Makefile" "${CMAKE_CURRENT_BINARY_DIR}/tests/c/Makefile" @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/tests/c/debug/Makefile" "${CMAKE_CURRENT_BINARY_DIR}/tests/c/debug/Makefile" @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/tests/bc/Machsuite/common.mk" "${CMAKE_CURRENT_BINARY_DIR}/tests/bc/Machsuite/common.mk" @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/scripts/colors.sh" "${CMAKE_CURRENT_BINARY_DIR}/scripts/colors.sh" @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/scripts/config.json" "${CMAKE_CURRENT_BINARY_DIR}/scripts" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/scripts/setup.sh"
            "${CMAKE_CURRENT_BINARY_DIR}/scripts/setup.sh" @ONLY)


add_subdirectory(lib)
add_subdirectory(tools)
