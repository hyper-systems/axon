#
# Copyright (c) 2021 Hyper Collective LTD
#
cmake_minimum_required(VERSION 3.13.1)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)

# set custom `boards` directory
set(BOARD_ROOT ${PROJECT_ROOT_DIR}/)

# add extenal libs modules
set(ZEPHYR_EXTRA_MODULES
   # add Hyper Drivers module
   ${PROJECT_ROOT_DIR}/drivers
   # add external libs module
   ${PROJECT_ROOT_DIR}/lib
)

# Find Zephyr. This also loads Zephyr's build system.
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(axon)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
target_sources(app PRIVATE ${SRC_FILES})
target_include_directories(app PUBLIC ${PROJECT_SOURCE_DIR}/src/)

add_subdirectory(${PROJECT_SOURCE_DIR}/hyper_device)

# add HYPER_FIRMWARE_VER macro
execute_process(
   COMMAND ${PROJECT_ROOT_DIR}/utils/get_commit_hash.sh
   WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
   OUTPUT_VARIABLE GIT_HASH
   OUTPUT_STRIP_TRAILING_WHITESPACE
   )
target_compile_definitions(app PRIVATE -DHYPER_FIRMWARE_VER="${GIT_HASH}")