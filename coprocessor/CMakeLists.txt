#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(NRF_SDK_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ncs/nrf)

set(COPROCESSOR_SAMPLE_ROOT_DIR ${NRF_SDK_ROOT_DIR}/ncs/nrf/samples/openthread/coprocessor)

set(OT_NCP_VENDOR_HOOK_SOURCE_DIR ${COPROCESSOR_SAMPLE_ROOT_DIR})

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(openthread_coprocessor)

# NORDIC SDK APP START
target_sources(app PRIVATE
  ${COPROCESSOR_SAMPLE_ROOT_DIR}/src/main.c
  ${COPROCESSOR_SAMPLE_ROOT_DIR}/src/nrf_802154_radio_wrapper.c
)
# NORDIC SDK APP END
