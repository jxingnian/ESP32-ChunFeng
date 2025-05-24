/*
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 14:53:16
 * @LastEditTime: 2025-05-24 15:52:22
 * @LastEditors: 星年
 * @Description: ESP春风
 * @FilePath: \ESP32-ChunFeng\main\main.c
 * 遇事不决，可问春风
 */


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app_network/app_network.h"
#include "app_audio/audio_board.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP ChunFeng By.xingnian");

    // 联网
    network_init();

    // 音频板初始化
    audio_board_init();

    
}