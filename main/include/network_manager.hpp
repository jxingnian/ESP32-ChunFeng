/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-29 20:47:50
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-29 20:47:58
 * @FilePath: \ESP32-ChunFeng\main\include\network_manager.hpp
 * @Description: 网络管理类
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <string>

namespace chunfeng {

/**
 * @brief 网络状态枚举
 */
enum class NetworkState {
    INIT,           ///< 初始化
    CONNECTING,     ///< 正在连接
    WIFI_CONNECTED, ///< WiFi已连接
    LTE_CONNECTED,  ///< 4G已连接
    FAILED          ///< 连接失败
};

// 状态机事件枚举
enum class NetworkEvent {
    WIFI_CONNECTED,
    WIFI_FAILED,
    LTE_CONNECTED,
    LTE_FAILED,
    DISCONNECT
};

/**
 * @brief 网络管理类
 */
class NetworkManager {
public:
    /**
     * @brief 获取单例实例
     */
    static NetworkManager& getInstance();

    /**
     * @brief 构造函数，自动完成网络初始化并启动状态机
     */
    NetworkManager();

    /**
     * @brief 析构函数，自动完成网络反初始化
     */
    ~NetworkManager();

    /**
     * @brief 获取当前网络状态
     */
    NetworkState getState() const;

private:
    // 禁止外部拷贝和赋值
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    /**
     * @brief 状态机主循环
     */
    void runStateMachine();

    /**
     * @brief 处理状态机事件
     */
    void handleEvent(enum class NetworkEvent event);

    NetworkState current_state_{NetworkState::INIT};
};

} // namespace chunfeng