idf_component_register(
    SRCS 
        "main.cpp"
        "src/network_manager.cpp"
    INCLUDE_DIRS "." "include"
    REQUIRES 
        esp_wifi
        nvs_flash
        driver
        network
)

# 启用C++支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
target_compile_options(${COMPONENT_LIB} PRIVATE "-std=gnu++17")