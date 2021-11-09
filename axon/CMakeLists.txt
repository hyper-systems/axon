#
# Copyright (c) 2021 Hyper Collective LTD
#
cmake_minimum_required(VERSION 3.13.1)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)

# set custom `boards` directory
set(BOARD_ROOT ${PROJECT_ROOT_DIR}/)

list(INSERT OVERLAY_CONFIG 0 ${PROJECT_ROOT_DIR}/ncs/nrf/samples/openthread/common/overlay-ot-defaults.conf)

# Find Zephyr. This also loads Zephyr's build system.
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(axon)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
target_sources(app PRIVATE ${SRC_FILES})
target_include_directories(app PUBLIC ${PROJECT_SOURCE_DIR}/src/)

add_subdirectory(${PROJECT_SOURCE_DIR}/hyper_device)
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/subsys/net/ip)

# add external libs
add_subdirectory(${PROJECT_ROOT_DIR}/lib external_libs)

# add drivers dir
add_subdirectory(${PROJECT_ROOT_DIR}/drivers common_drivers)

# add HYPER_FIRMWARE_VER macro
execute_process(
   COMMAND ${PROJECT_ROOT_DIR}/utils/get_commit_hash.sh
   WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
   OUTPUT_VARIABLE GIT_HASH
   OUTPUT_STRIP_TRAILING_WHITESPACE
   )
target_compile_definitions(app PRIVATE -DHYPER_FIRMWARE_VER="${GIT_HASH}")