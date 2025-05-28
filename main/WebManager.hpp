/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-28 21:38:14
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-28 21:48:36
 * @FilePath: \ESP32-ChunFeng\main\WebManager.hpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
// /*
//  * @Author: xingnian j_xingnian@163.com
//  * @Date: 2025-05-28 21:38:14
//  * @LastEditors: 星年 && j_xingnian@163.com
//  * @LastEditTime: 2025-05-28 21:41:53
//  * @FilePath: \ESP32-ChunFeng\main\WebManager.hpp
//  * @Description: Web服务器管理类
//  * 
//  * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
//  */
// #pragma once

// #include <string>
// #include "esp_err.h"
// #include "esp_http_server.h" 

// /**
//  * @brief Web服务器管理类
//  * @details 负责初始化和管理HTTP服务器,处理各类HTTP请求,以及WiFi配置功能
//  */
// class WebManager {
// public:
//     /**
//      * @brief 构造函数
//      */
//     WebManager();

//     /**
//      * @brief 析构函数
//      */
//     ~WebManager();

//     /**
//      * @brief 初始化Web服务器
//      * @return esp_err_t 初始化结果
//      */
//     esp_err_t init();

//     /**
//      * @brief 启动Web服务器
//      * @return esp_err_t 启动结果
//      */
//     esp_err_t start();

//     /**
//      * @brief 停止Web服务器
//      */
//     void stop();

//     /**
//      * @brief WiFi配置回调函数类型定义
//      * @param ssid WiFi的SSID
//      * @param password WiFi密码
//      * @return esp_err_t 配置结果
//      */
//     using WiFiConfigCallback = std::function<esp_err_t(const std::string&, const std::string&)>;

//     /**
//      * @brief 设置WiFi配置回调函数
//      * @param callback 回调函数
//      */
//     void setWiFiConfigCallback(WiFiConfigCallback callback);

// private:
//     /**
//      * @brief 处理首页请求
//      * @param req HTTP请求结构体
//      * @return esp_err_t 处理结果
//      */
//     static esp_err_t indexHandler(httpd_req_t* req);

//     /**
//      * @brief 处理WiFi配置请求
//      * @param req HTTP请求结构体
//      * @return esp_err_t 处理结果
//      */
//     static esp_err_t wifiConfigHandler(httpd_req_t* req);

//     /**
//      * @brief 处理状态查询请求
//      * @param req HTTP请求结构体
//      * @return esp_err_t 处理结果
//      */
//     static esp_err_t statusHandler(httpd_req_t* req);

//     httpd_handle_t server;              ///< HTTP服务器句柄
//     WiFiConfigCallback wifiConfigCallback;  ///< WiFi配置回调函数
    
//     static const char* TAG;             ///< 日志标签
// };