/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 12:19:31
 * @LastEditTime: 2025-05-30 17:47:16
 * @LastEditors: 星年
 * @Description: WiFi 管理类，负责 WiFi 的初始化、反初始化、连接、断开与状态查询
 * @FilePath: \ESP32-ChunFeng\components\network\include\wifi_manager.hpp
 * @遇事不决，可问春风
 */
#pragma once

#include <string>

namespace chunfeng {

/**
 * @brief WiFi 管理类
 * 
 * 该类采用单例模式，负责 WiFi 的初始化、反初始化、连接、断开和状态查询。
 * 
 * 主要功能包括：
 * - 初始化和反初始化底层 WiFi 硬件及相关资源
 * - 连接和断开 WiFi 网络
 * - 查询当前 WiFi 连接状态
 * - WiFi 信息的保存、读取与删除
 */
class WiFiManager {
public:
    /**
     * @brief 获取 WiFiManager 单例实例
     * 
     * 采用单例模式，确保全局仅有一个 WiFiManager 实例。
     * @return WiFiManager& 单例引用
     */
    static WiFiManager& getInstance();

    /**
     * @brief 连接 WiFi
     * 
     * 尝试连接到指定的 WiFi 网络。
     * 连接前应确保已初始化。
     * @param ssid WiFi 名称
     * @param password WiFi 密码
     * @return true 连接成功
     * @return false 连接失败
     */
    bool connect(const std::string& ssid, const std::string& password);

    /**
     * @brief 断开 WiFi 连接
     * 
     * 断开当前的 WiFi 连接并释放相关资源。
     * 断开后可选择反初始化 WiFi 模块。
     */
    void disconnect();

    /**
     * @brief 查询 WiFi 是否已连接
     * 
     * 检查当前 WiFi 连接状态。
     * @return true 已连接
     * @return false 未连接
     */
    bool isConnected() const;

    /**
     * @brief 保存 WiFi 信息
     * 
     * 保存指定的 WiFi SSID 和密码到持久化存储（或内存示例）。
     * @param ssid WiFi 名称
     * @param password WiFi 密码
     * @return true 保存成功
     * @return false 保存失败
     */
    bool saveWiFiInfo(const std::string& ssid, const std::string& password);

    /**
     * @brief 读取已保存的 WiFi 信息
     * 
     * 读取已保存的 WiFi SSID 和密码。
     * @param ssid [out] 读取到的 WiFi 名称
     * @param password [out] 读取到的 WiFi 密码
     * @return true 读取成功
     * @return false 未找到已保存信息
     */
    bool loadWiFiInfo(std::string& ssid, std::string& password);

    /**
     * @brief 删除已保存的 WiFi 信息
     * 
     * 删除已保存的 WiFi SSID 和密码。
     * @return true 删除成功
     * @return false 删除失败
     */
    bool deleteWiFiInfo();

    WiFiManager(); // 构造函数声明
    ~WiFiManager(); // 析构函数声明

private:
    // 禁止拷贝和赋值
    WiFiManager(const WiFiManager&) = delete;
    WiFiManager& operator=(const WiFiManager&) = delete;

    bool initialized_{false};
    bool connected_{false};

    // 内存保存的 WiFi 信息（实际应持久化到NVS等）
    std::string saved_ssid_;
    std::string saved_password_;
};

} // namespace chunfeng 