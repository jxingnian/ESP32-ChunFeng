/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 12:15:47
 * @LastEditTime: 2025-05-30 16:54:10
 * @LastEditors: 星年
 * @Description: 网络管理器
 * @FilePath: \ESP32-ChunFeng\main\src\network_manager.cpp
 * @遇事不决，可问春风
 */
#include "network_manager.hpp"
#include "wifi_manager.hpp"
#include "lte_manager.hpp"
#include "config_manager.hpp"
#include <iostream>
#include <string>

namespace chunfeng {

// 单例获取
NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

// 状态机事件处理
void NetworkManager::handleEvent(NetworkEvent event) {
    switch (event) {
        case NetworkEvent::WIFI_CONNECTED:
            current_state_ = NetworkState::WIFI_CONNECTED;
            std::cout << "[NetworkManager] WiFi 已连接" << std::endl;
            break;
        case NetworkEvent::WIFI_FAILED:
            current_state_ = NetworkState::CONNECTING;
            std::cerr << "[NetworkManager] WiFi 连接失败，尝试4G" << std::endl;
            break;
        case NetworkEvent::LTE_CONNECTED:
            current_state_ = NetworkState::LTE_CONNECTED;
            std::cout << "[NetworkManager] 4G 已连接" << std::endl;
            break;
        case NetworkEvent::LTE_FAILED:
            current_state_ = NetworkState::FAILED;
            std::cerr << "[NetworkManager] 4G 连接失败，网络连接失败" << std::endl;
            break;
        case NetworkEvent::DISCONNECT:
            std::cerr << "[NetworkManager] 网络断开，重新初始化" << std::endl;
            current_state_ = NetworkState::INIT;
            break;
        default:
            break;
    }
}

// 状态机主循环
void NetworkManager::runStateMachine() {
    while (true) {
        switch (current_state_) {
            case NetworkState::INIT: {
                std::cout << "[NetworkManager] 状态: INIT" << std::endl;
                // 组件化创建各功能对象
                // 1. 配置管理器（如有需要可初始化）
                ConfigManager& config = ConfigManager::getInstance();

                // // 2. WiFi 管理器
                // WiFiManager& wifi = WiFiManager::getInstance();

                // // 3. LTE 管理器
                // LTEManager& lte = LTEManager::getInstance();
                
                current_state_ = NetworkState::CONNECTING;
                break;
            }
            case NetworkState::CONNECTING: {
                std::cout << "[NetworkManager] 状态: CONNECTING" << std::endl;
                ConfigManager::getInstance().startConfig();
                handleEvent(NetworkEvent::WIFI_CONNECTED);
                // // 先尝试从NVS读取WiFi信息
                // std::string ssid, password;
                // bool wifiInfoLoaded = WiFiManager::getInstance().loadWiFiInfo(ssid, password);
                // if (wifiInfoLoaded) {
                //     // 有WiFi信息，尝试连接
                //     if (WiFiManager::getInstance().connect(ssid, password)) {
                //         handleEvent(NetworkEvent::WIFI_CONNECTED);
                //     } else {
                //         handleEvent(NetworkEvent::WIFI_FAILED);
                //         // WiFi连接失败，尝试4G
                //         if (LTEManager::getInstance().connect()) {
                //             handleEvent(NetworkEvent::LTE_CONNECTED);
                //         } else {
                //             handleEvent(NetworkEvent::LTE_FAILED);
                //         }
                //     }
                // } else {
                //     // 没有WiFi信息，直接进入4G
                //     std::cerr << "[NetworkManager] 未找到WiFi信息，直接尝试4G" << std::endl;
                //     if (LTEManager::getInstance().connect()) {
                //         handleEvent(NetworkEvent::LTE_CONNECTED);
                //     } else {
                //         handleEvent(NetworkEvent::LTE_FAILED);
                //     }
                // }
                // break;
            }
            case NetworkState::WIFI_CONNECTED: {
                // std::cout << "[NetworkManager] 状态: WIFI_CONNECTED" << std::endl;
                // // 检查WiFi是否掉线
                // if (!WiFiManager::getInstance().isConnected()) {
                //     handleEvent(NetworkEvent::DISCONNECT);
                // }
                break;
            }
            case NetworkState::LTE_CONNECTED: {
                std::cout << "[NetworkManager] 状态: LTE_CONNECTED" << std::endl;
                // 检查LTE是否掉线
                if (!LTEManager::getInstance().isConnected()) {
                    handleEvent(NetworkEvent::DISCONNECT);
                }
                break;
            }
            case NetworkState::FAILED: {
                std::cerr << "[NetworkManager] 状态: FAILED" << std::endl;
                break;
            }
            default:
                break;
        }
        // 延时
        vTaskDelay(pdMS_TO_TICKS(1000)); // 避免占用过多CPU
    }
}

// 构造函数，自动完成网络初始化并启动状态机
NetworkManager::NetworkManager() {
    // 初始化状态
    current_state_ = NetworkState::INIT;

    // 启动状态机
    runStateMachine();
}

// 析构函数，自动完成网络反初始化
NetworkManager::~NetworkManager() {
    ConfigManager::getInstance().stopConfig();
    WiFiManager::getInstance().disconnect();
    LTEManager::getInstance().disconnect();
    // 其他资源释放可在各自管理器中完成
}

NetworkState NetworkManager::getState() const {
    return current_state_;
}

} // namespace chunfeng 