#
# Copyright (c) 2021 Hyper Collective LTD
#

# libcmp
if(CONFIG_LIBCMP)
    zephyr_library()
    zephyr_include_directories(cmp)
    zephyr_library_sources(cmp/cmp.c)
endif()