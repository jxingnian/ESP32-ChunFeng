/*
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 14:53:16
 * @LastEditTime: 2025-05-28 21:55:49
 * @LastEditors: 星年 && j_xingnian@163.com
 * @Description: ESP春风
 * @FilePath: \ESP32-ChunFeng\main\main.cpp
 * 遇事不决，可问春风
 */

// #include "ChunFeng.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ChunFeng.hpp"

static const char* TAG = "main";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP ChunFeng By.xingnian");

    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 获取ChunFeng实例并初始化
    ChunFeng& chunFeng = ChunFeng::getInstance();
    ESP_ERROR_CHECK(chunFeng.init());
    ESP_ERROR_CHECK(chunFeng.start());

    // 主程序永远运行
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}