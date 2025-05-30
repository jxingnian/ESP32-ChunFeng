/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-30 20:17:11
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-30 20:25:27
 * @FilePath: \ESP32-ChunFeng\components\network\include\bsp_config_network.hpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once
#include <string>
#include <vector>

namespace chunfeng {

/**
 * @brief WiFi信息结构体
 */
struct WiFiInfo {
    std::string ssid;      ///< WiFi名称
    int rssi;              ///< 信号强度
    bool is_encrypted;     ///< 是否加密
};

/**
 * @brief 配网管理类，支持AP+STA模式和网页配置
 */
class BspConfigNetwork {
public:
    BspConfigNetwork();
    ~BspConfigNetwork();

    /**
     * @brief 启动AP+STA模式和HTTP服务器
     * @return true 启动成功
     */
    bool start();

    /**
     * @brief 停止AP+STA和HTTP服务器
     */
    void stop();

    /**
     * @brief 扫描周围WiFi
     * @return WiFi列表
     */
    std::vector<WiFiInfo> scanWiFi();

    /**
     * @brief 连接指定WiFi
     * @param ssid WiFi名称
     * @param password 密码
     * @return true 连接成功
     */
    bool connectWiFi(const std::string& ssid, const std::string& password);

    /**
     * @brief 删除已保存WiFi
     * @return true 删除成功
     */
    bool deleteWiFi();

    /**
     * @brief 获取当前连接信息
     * @return 当前连接信息字符串
     */
    std::string getCurrentWiFiInfo();

private:
    bool ap_sta_started_{false};      ///< AP+STA是否已启动
    void* http_server_{nullptr};      ///< HTTP服务器句柄
    std::string last_ssid_;           ///< 最近连接的SSID
    std::string last_password_;       ///< 最近连接的密码
};

} // namespace chunfeng