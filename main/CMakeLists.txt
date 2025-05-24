idf_component_register(SRCS "main.c" "esp_network/wifi_manager.c" "esp_network/http_server.c" "esp_network/esp_network.c"
                    INCLUDE_DIRS "." "esp_network/"
                    REQUIRES esp_wifi esp_http_server nvs_flash json spiffs)
