/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 12:20:50
 * @LastEditTime: 2025-05-30 17:48:22
 * @LastEditors: 星年
 * @Description: LTE 管理类实现，负责 LTE 的初始化、连接、断开与状态查询
 * @FilePath: \ESP32-ChunFeng\components\network\src\lte_manager.cpp
 * @遇事不决，可问春风
 */
#include "lte_manager.hpp"
#include <iostream>

static const char *TAG = "ML307";

namespace chunfeng {

// 构造函数
LTEManager::LTEManager()
    : initialized_{false}, connected_{false}
{
    std::cout << "[LTEManager] 构造: 初始化 LTE 管理器..." << std::endl;
}

// 析构函数
LTEManager::~LTEManager()
{
    std::cout << "[LTEManager] 析构: 释放 LTE 相关资源..." << std::endl;
    // 这里可以添加资源释放代码
    initialized_ = false;
    connected_ = false;
}

// 获取 LTEManager 单例实例
LTEManager& LTEManager::getInstance() {
    static LTEManager instance;
    return instance;
}

// void TestWebSocket(Ml307AtModem& modem) {
//     ESP_LOGI(TAG, "Starting WebSocket test");

//     WebSocket ws(new Ml307SslTransport(modem, 0));
//     ws.SetHeader("Protocol-Version", "2");

//     ws.OnConnected([]() {
//         ESP_LOGI(TAG, "Connected to server");
//     });

//     ws.OnData([](const char* data, size_t length, bool binary) {
//         ESP_LOGI(TAG, "Received data: %.*s", length, data);
//     });

//     ws.OnDisconnected([]() {
//         ESP_LOGI(TAG, "Disconnected from server");
//     });

//     ws.OnError([](int error) {
//         ESP_LOGE(TAG, "WebSocket error: %d", error);
//     });

//     if (!ws.Connect("wss://api.tenclass.net/xiaozhi/v1/")) {
//         ESP_LOGE(TAG, "Failed to connect to server");
//         return;
//     }

//     for (int i = 0; i < 10; i++) {
//         ws.Send("{\"type\": \"hello\"}");
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
//     ws.Close();
// }

bool LTEManager::initialize(){
    Ml307AtModem modem(GPIO_NUM_13, GPIO_NUM_14, 2048);
    modem.SetDebug(true);
    modem.SetBaudRate(921600);

    modem.WaitForNetworkReady();

    // Print IP Address
    ESP_LOGI(TAG, "IP Address: %s", modem.ip_address().c_str());
    // Print IMEI, ICCID, Product ID, Carrier Name
    ESP_LOGI(TAG, "IMEI: %s", modem.GetImei().c_str());
    ESP_LOGI(TAG, "ICCID: %s", modem.GetIccid().c_str());
    ESP_LOGI(TAG, "Product ID: %s", modem.GetModuleName().c_str());
    ESP_LOGI(TAG, "Carrier Name: %s", modem.GetCarrierName().c_str());
    // Print CSQ
    ESP_LOGI(TAG, "CSQ: %d", modem.GetCsq());
    return true;
    // TestWebSocket(modem);
}

// 连接 LTE（4G）网络
bool LTEManager::connect() {
    if (!initialized_) {
        std::cerr << "[LTEManager] 错误：LTE 管理器未初始化，无法连接 4G。" << std::endl;
        return false;
    }
    if (connected_) {
        std::cout << "[LTEManager] 已连接 4G，无需重复连接。" << std::endl;
        return true;
    }
    std::cout << "[LTEManager] 正在连接 4G..." << std::endl;
    // 这里添加实际的 4G 连接代码
    // 假定连接成功
    connected_ = true;
    return connected_;
}

// 断开 LTE（4G）网络连接
void LTEManager::disconnect() {
    if (!initialized_) {
        std::cerr << "[LTEManager] 错误：LTE 管理器未初始化，无法断开 4G。" << std::endl;
        return;
    }
    if (!connected_) {
        std::cout << "[LTEManager] 4G 已断开，无需重复断开。" << std::endl;
        return;
    }
    std::cout << "[LTEManager] 正在断开 4G..." << std::endl;
    // 这里添加实际的 4G 断开代码
    connected_ = false;
}

// 查询 LTE 是否已连接
bool LTEManager::isConnected() const {
    return connected_;
}

} // namespace chunfeng 