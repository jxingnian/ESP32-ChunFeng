idf_component_register(
    SRCS "src/config_manager.cpp"
            "src/wifi_manager.cpp"
            "src/lte_manager.cpp"
            "src/bsp_wifi.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_wifi
        nvs_flash
        driver
        78__esp-ml307
)

# 启用C++支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
target_compile_options(${COMPONENT_LIB} PRIVATE "-std=gnu++17")