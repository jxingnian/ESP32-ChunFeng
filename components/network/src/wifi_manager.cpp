/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 12:20:35
 * @LastEditTime: 2025-05-30 14:53:44
 * @LastEditors: 星年
 * @Description: WiFi 管理类实现，负责 WiFi 的初始化、连接、断开与状态查询
 * @FilePath: \ESP32-ChunFeng\components\network\src\wifi_manager.cpp
 * @遇事不决，可问春风
 */
#include "wifi_manager.hpp"
#include <iostream>

namespace chunfeng {

// 构造函数
WiFiManager::WiFiManager()
    : initialized_{false}, connected_{false}
{
    std::cout << "[WiFiManager] 构造: 初始化 WiFi 管理器..." << std::endl;
    // 这里可以进行必要的成员初始化
    initialized_ = true;
    connected_ = false;
}

// 析构函数
WiFiManager::~WiFiManager()
{
    std::cout << "[WiFiManager] 析构: 释放 WiFi 相关资源..." << std::endl;
    // 这里可以添加资源释放代码
    initialized_ = false;
    connected_ = false;
}

// 获取 WiFiManager 单例实例
WiFiManager& WiFiManager::getInstance() {
    static WiFiManager instance;
    return instance;
}

// 连接 WiFi
bool WiFiManager::connect() {
    if (!initialized_) {
        std::cerr << "[WiFiManager] 错误：WiFi 管理器未初始化，无法连接 WiFi。" << std::endl;
        return false;
    }
    if (connected_) {
        std::cout << "[WiFiManager] 已连接 WiFi，无需重复连接。" << std::endl;
        return true;
    }
    std::cout << "[WiFiManager] 正在连接 WiFi..." << std::endl;
    // 这里添加实际的 WiFi 连接代码
    // 例如：esp_wifi_connect()，并处理连接结果
    // 假定连接成功
    connected_ = true;
    return connected_;
}

// 断开 WiFi 连接
void WiFiManager::disconnect() {
    if (!initialized_) {
        std::cerr << "[WiFiManager] 错误：WiFi 管理器未初始化，无法断开 WiFi。" << std::endl;
        return;
    }
    if (!connected_) {
        std::cout << "[WiFiManager] WiFi 已断开，无需重复断开。" << std::endl;
        return;
    }
    std::cout << "[WiFiManager] 正在断开 WiFi..." << std::endl;
    // 这里添加实际的 WiFi 断开代码
    // 例如：esp_wifi_disconnect()
    connected_ = false;
}

// 查询 WiFi 是否已连接
bool WiFiManager::isConnected() const {
    // 这里返回 WiFi 连接状态
    // 实际实现可查询底层 WiFi 状态
    // 例如：esp_wifi_sta_get_ap_info() 或维护内部状态
    return connected_;
}

} // namespace chunfeng 