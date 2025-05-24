/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 14:53:16
 * @LastEditTime: 2025-05-24 15:51:41
 * @LastEditors: 星年
 * @Description: WiFi管理
 * @FilePath: \ESP32-ChunFeng\main\esp_network\wifi_manager.h
 * @遇事不决，可问春风
 */
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_wifi.h"
#include "esp_event.h"

// WiFi初始化函数
esp_err_t wifi_init(void);
esp_err_t wifi_reset_connection_retry(void);

// 尝试从nvs获取信息连接WiFi
esp_err_t wifi_try_connect(void);

#endif // WIFI_MANAGER_H
