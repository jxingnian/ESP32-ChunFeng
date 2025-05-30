/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 12:19:31
 * @LastEditTime: 2025-05-30 14:52:58
 * @LastEditors: 星年
 * @Description: WiFi 管理类，负责 WiFi 的初始化、反初始化、连接、断开与状态查询
 * @FilePath: \ESP32-ChunFeng\components\network\include\wifi_manager.hpp
 * @遇事不决，可问春风
 */
#pragma once

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
     * 尝试连接到配置的 WiFi 网络。
     * 连接前应确保已初始化。
     * @return true 连接成功
     * @return false 连接失败
     */
    bool connect();

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

    WiFiManager(); // 构造函数声明
    ~WiFiManager(); // 增加析构函数声明

private:
    /**
     * @brief 构造函数私有化，禁止外部实例化
     * 
     * 仅允许通过 getInstance() 获取单例对象。
     */
    // WiFiManager() = default; // 移除默认构造，改为 public 构造
    bool initialized_{false};
    bool connected_{false};

    // TODO: 可扩展 WiFi 状态、配置参数等私有成员
};

} // namespace chunfeng 