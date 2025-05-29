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
    FAILED         ///< 连接失败
};

/**
 * @brief 网络管理类
 */
class NetworkManager {
public:
    static NetworkManager& getInstance();

    /**
     * @brief 初始化网络
     */
    bool initialize();

    /**
     * @brief 反初始化网络
     */
    void deinitialize();

    /**
     * @brief 获取当前网络状态
     */
    NetworkState getState() const;

    /**
     * @brief 上报网络状态
     */
    void reportState();

private:
    NetworkManager() = default;
    NetworkState current_state_{NetworkState::INIT};
};

} // namespace chunfeng