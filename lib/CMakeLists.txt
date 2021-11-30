#
# Copyright (c) 2021 Hyper Collective LTD
#
cmake_minimum_required(VERSION 3.19)

# libcmp
if(CONFIG_LIBCMP)
  include(${PROJECT_ROOT_DIR}/cmake/add_git_submodule.cmake)
  add_git_submodule(cmp)
  zephyr_library()
  zephyr_include_directories(cmp)
  zephyr_library_sources(cmp/cmp.c)
endif()
