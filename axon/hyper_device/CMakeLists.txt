#
# Copyright (c) 2021 Hyper Collective LTD
#
if(CONFIG_HYPER_DEVICE)
  add_subdirectory(hyper-device-utils)
  add_subdirectory(hyper-device-axon)
  add_subdirectory(hyper-device-mcu)
  add_subdirectory(hyper-device-hydrokit)
  add_subdirectory(hyper-device-as-co2)
  add_subdirectory(hyper-device-as-hum)
endif()
