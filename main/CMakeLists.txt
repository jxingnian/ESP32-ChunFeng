idf_component_register(
    SRCS 
        "main.cpp"
        "ChunFeng.cpp"
        "NetworkManager.cpp"
#        "AudioManager.cpp"
#        "CozeManager.cpp"
#        "WebManager.cpp"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_wifi
        nvs_flash
        driver
)

# 启用C++支持
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
target_compile_options(${COMPONENT_LIB} PRIVATE "-std=gnu++17")