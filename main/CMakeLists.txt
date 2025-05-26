idf_component_register(SRCS "main.c" 
                            "app_network/wifi_manager.c" 
                            "app_network/http_server.c" 
                            "app_network/app_network.c"
                            "app_network/ml307_wrapper.cpp"
                            "app_audio/audio_board.c"
                            "app_coze/coze_chat_app.c"
                    INCLUDE_DIRS "." 
                                "app_network/" 
                                "app_coze/" 
                                "app_audio/"
                    REQUIRES esp_wifi esp_http_server nvs_flash json 
                                spiffs 78__esp-ml307 driver espressif__esp_coze 78__esp-opus)

# 启用C++支持
set_source_files_properties("esp_network/ml307_wrapper.cpp" PROPERTIES COMPILE_FLAGS -std=gnu++17)
# target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)