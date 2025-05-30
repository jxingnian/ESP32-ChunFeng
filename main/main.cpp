/*
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 14:53:16
 * @LastEditTime: 2025-05-29 21:34:24
 * @LastEditors: 星年 && j_xingnian@163.com
 * @Description: ESP春风
 * @FilePath: \ESP32-ChunFeng\main\main.cpp
 * 遇事不决，可问春风
 */
#include "network_manager.hpp"
// #include "audio_manager.hpp"
// #include "coze_manager.hpp"
// #include "display_manager.hpp"
// #include "backend_manager.hpp"
#include "freertos/FreeRTOS.h"   /* FreeRTOS核心头文件 */
// #include "freertos/event_groups.h"/* FreeRTOS事件组头文件 */

#include "esp_log.h"
#include "nvs_flash.h"

static const char* TAG = "ChunFeng";

using namespace chunfeng;

extern "C" void app_main(void)
{
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 获取各个管理器实例
    auto& network_mgr = NetworkManager::getInstance();
    // auto& audio_mgr = AudioManager::getInstance();
    // auto& coze_mgr = CozeManager::getInstance();
    // auto& display_mgr = DisplayManager::getInstance();
    // auto& backend_mgr = BackendManager::getInstance();
    
    // TODO: 初始化各个模块
    
    // 进入主循环
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // 避免占用过多CPU
    }
}