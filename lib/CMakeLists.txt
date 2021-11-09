#
# Copyright (c) 2021 Hyper Collective LTD
#


# libcmp
target_include_directories(app PUBLIC cmp)
target_sources(app PRIVATE cmp/cmp.c)