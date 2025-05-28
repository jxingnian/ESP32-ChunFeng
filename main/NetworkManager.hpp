/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-28 21:37:27
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-28 21:57:08
 * @FilePath: \ESP32-ChunFeng\main\NetworkManager.hpp
 * @Description: 网络管理类
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <string>
#include "esp_err.h"
#include "esp_wifi.h"

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    esp_err_t init();
    esp_err_t startWiFi();
    esp_err_t start4G();
    esp_err_t connect(const std::string& ssid, const std::string& password);
    bool isConnected() const;
    void disconnect();

    // WiFi配置相关
    esp_err_t startSmartConfig();
    esp_err_t stopSmartConfig();
    
    // 4G相关
    esp_err_t init4G();
    esp_err_t connect4G();
    void disconnect4G();

private:
    static void wifiEventHandler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
    
    bool wifiConnected;
    bool fourGConnected;
    static const char* TAG;
};