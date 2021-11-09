
if(CONFIG_MCP342X)
    # zephyr_* doesnt work, fails linking
    # zephyr_include_directories(.)
    # zephyr_library()
    # zephyr_library_sources(mcp342x.c)
    target_include_directories(app PUBLIC .)
    target_sources(app PRIVATE mcp342x.c)
endif()