// /*
//  * @Author: xingnian j_xingnian@163.com
//  * @Date: 2025-05-28 21:38:01
//  * @LastEditors: 星年 && j_xingnian@163.com
//  * @LastEditTime: 2025-05-28 21:40:17
//  * @FilePath: \ESP32-ChunFeng\main\CozeManager.hpp
//  * @Description: AI助手的管理类
//  * 
//  * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
//  */
// #pragma once

// #include <string>
// #include <functional>
// #include "esp_websocket_client.h"

// /**
//  * @brief Coze AI助手的管理类
//  * 
//  * 该类负责管理与Coze AI服务器的WebSocket连接,
//  * 处理文本和音频数据的收发
//  */
// class CozeManager {
// public:
//     /** 接收文本消息的回调函数类型定义 */
//     using MessageCallback = std::function<void(const std::string&)>;
//     /** 接收音频数据的回调函数类型定义 */
//     using AudioCallback = std::function<void(const uint8_t*, size_t)>;

//     /** 构造函数 */
//     CozeManager();
//     /** 析构函数 */
//     ~CozeManager();

//     /**
//      * @brief 初始化CozeManager
//      * @param apiKey Coze API密钥
//      * @return esp_err_t 初始化结果
//      */
//     esp_err_t init(const std::string& apiKey);

//     /**
//      * @brief 连接到Coze WebSocket服务器
//      * @return esp_err_t 连接结果
//      */
//     esp_err_t connect();

//     /**
//      * @brief 断开与服务器的连接
//      */
//     void disconnect();
    
//     /**
//      * @brief 发送文本消息到服务器
//      * @param message 要发送的文本消息
//      * @return esp_err_t 发送结果
//      */
//     esp_err_t sendTextMessage(const std::string& message);

//     /**
//      * @brief 发送音频数据到服务器
//      * @param data 音频数据缓冲区
//      * @param length 音频数据长度
//      * @return esp_err_t 发送结果
//      */
//     esp_err_t sendAudioData(const uint8_t* data, size_t length);
    
//     /**
//      * @brief 设置接收文本消息的回调函数
//      * @param callback 回调函数
//      */
//     void setMessageCallback(MessageCallback callback);

//     /**
//      * @brief 设置接收音频数据的回调函数
//      * @param callback 回调函数
//      */
//     void setAudioCallback(AudioCallback callback);

//     /**
//      * @brief 检查是否已连接到服务器
//      * @return bool 连接状态
//      */
//     bool isConnected() const { return connected; }

// private:
//     /**
//      * @brief WebSocket事件处理函数
//      * @param handler_args 处理器参数
//      * @param base 事件基础类型
//      * @param event_id 事件ID
//      * @param event_data 事件数据
//      */
//     static void websocketEventHandler(void* handler_args, esp_event_base_t base,
//                                     int32_t event_id, void* event_data);
    
//     esp_websocket_client_handle_t client;    ///< WebSocket客户端句柄
//     MessageCallback messageCallback;         ///< 文本消息回调函数
//     AudioCallback audioCallback;             ///< 音频数据回调函数
//     bool connected;                          ///< 连接状态标志
//     std::string apiKey;                      ///< Coze API密钥
    
//     static const char* TAG;                  ///< 日志标签
// };