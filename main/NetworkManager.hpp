// /*
//  * @Author: xingnian j_xingnian@163.com
//  * @Date: 2025-05-28 21:37:27
//  * @LastEditors: 星年 && j_xingnian@163.com
//  * @LastEditTime: 2025-05-28 21:41:43
//  * @FilePath: \ESP32-ChunFeng\main\NetworkManager.hpp
//  * @Description: 网络管理类
//  * 
//  * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
//  */
// #pragma once

// #include <string>
// #include "esp_err.h"
// #include "esp_wifi.h"

// /**
//  * @brief 网络管理类,用于处理WiFi和4G网络连接
//  */
// class NetworkManager {
// public:
//     /**
//      * @brief 构造函数
//      */
//     NetworkManager();

//     /**
//      * @brief 析构函数
//      */
//     ~NetworkManager();

//     /**
//      * @brief 初始化网络管理器
//      * @return esp_err_t 初始化结果
//      */
//     esp_err_t init();

//     /**
//      * @brief 启动WiFi功能
//      * @return esp_err_t 启动结果
//      */
//     esp_err_t startWiFi();

//     /**
//      * @brief 启动4G功能
//      * @return esp_err_t 启动结果
//      */
//     esp_err_t start4G();

//     /**
//      * @brief 连接到指定的WiFi网络
//      * @param ssid WiFi网络名称
//      * @param password WiFi密码
//      * @return esp_err_t 连接结果
//      */
//     esp_err_t connect(const std::string& ssid, const std::string& password);

//     /**
//      * @brief 检查是否已连接到网络
//      * @return bool 连接状态
//      */
//     bool isConnected() const;

//     /**
//      * @brief 断开当前网络连接
//      */
//     void disconnect();

//     // WiFi配置相关
//     /**
//      * @brief 启动SmartConfig配网
//      * @return esp_err_t 启动结果
//      */
//     esp_err_t startSmartConfig();

//     /**
//      * @brief 停止SmartConfig配网
//      * @return esp_err_t 停止结果
//      */
//     esp_err_t stopSmartConfig();
    
//     // 4G相关
//     /**
//      * @brief 初始化4G模块
//      * @return esp_err_t 初始化结果
//      */
//     esp_err_t init4G();

//     /**
//      * @brief 连接4G网络
//      * @return esp_err_t 连接结果
//      */
//     esp_err_t connect4G();

//     /**
//      * @brief 断开4G网络连接
//      */
//     void disconnect4G();

// private:
//     /**
//      * @brief WiFi事件处理函数
//      * @param arg 参数指针
//      * @param event_base 事件基础类型
//      * @param event_id 事件ID
//      * @param event_data 事件数据
//      */
//     static void wifiEventHandler(void* arg, esp_event_base_t event_base,
//                                int32_t event_id, void* event_data);
    
//     bool wifiConnected;      ///< WiFi连接状态标志
//     bool fourGConnected;     ///< 4G连接状态标志
//     static const char* TAG;  ///< 日志标签
// };
//     void disconnect4G();

// private:
//     static void wifiEventHandler(void* arg, esp_event_base_t event_base,
//                                int32_t event_id, void* event_data);
    
//     bool wifiConnected;
//     bool fourGConnected;
//     static const char* TAG;
// };