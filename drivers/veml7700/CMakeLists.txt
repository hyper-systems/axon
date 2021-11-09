
if(CONFIG_VEML7700)
    # zephyr_* doesnt work, fails linking
    # zephyr_include_directories(.)
    # zephyr_library()
    # zephyr_library_sources(veml7700.c)
    target_include_directories(app PUBLIC .)
    target_sources(app PRIVATE veml7700.c)
endif()