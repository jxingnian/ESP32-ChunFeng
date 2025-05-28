// /*
//  * @Author: xingnian j_xingnian@163.com
//  * @Date: 2025-05-28 21:39:21
//  * @LastEditors: 星年 && j_xingnian@163.com
//  * @LastEditTime: 2025-05-28 21:42:35
//  * @FilePath: \ESP32-ChunFeng\main\ChunFeng.cpp
//  * @Description: ChunFeng主类实现
//  * 
//  * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
//  */
//  #include "ChunFeng.hpp"
// #include "NetworkManager.hpp"
// #include "AudioManager.hpp"
// #include "CozeManager.hpp"
// #include "WebManager.hpp"

// const char* ChunFeng::TAG = "ChunFeng";

// esp_err_t ChunFeng::init() {
//     ESP_LOGI(TAG, "Initializing ChunFeng...");

//     // 创建各个管理器实例
//     networkManager = std::make_unique<NetworkManager>();
//     audioManager = std::make_unique<AudioManager>();
//     cozeManager = std::make_unique<CozeManager>();
//     webManager = std::make_unique<WebManager>();

//     // 初始化各个管理器
//     ESP_ERROR_CHECK(networkManager->init());
//     ESP_ERROR_CHECK(webManager->init());

//     // 设置网页配网回调
//     webManager->setWiFiConfigCallback([this](const std::string& ssid, const std::string& password) {
//         return networkManager->connect(ssid, password);
//     });

//     // 设置音频配置
//     AudioManager::AudioConfig audioConfig{
//         .sampleRate = 16000,
//         .bitsPerSample = 16,
//         .channels = 1,
//         .bufferSize = 2048
//     };
//     ESP_ERROR_CHECK(audioManager->init(audioConfig));

//     // 设置音频回调
//     audioManager->setAudioCallback([this](const uint8_t* data, size_t length) {
//         if (cozeManager->isConnected()) {
//             cozeManager->sendAudioData(data, length);
//         }
//     });

//     // 初始化Coze管理器
//     // TODO: 从配置文件或NVS中读取API Key
//     std::string apiKey = "your_coze_api_key";
//     ESP_ERROR_CHECK(cozeManager->init(apiKey));

//     // 设置Coze音频回调
//     cozeManager->setAudioCallback([this](const uint8_t* data, size_t length) {
//         audioManager->playAudio(data, length);
//     });

//     return ESP_OK;
// }

// esp_err_t ChunFeng::start() {
//     ESP_LOGI(TAG, "Starting ChunFeng services...");

//     // 启动Web服务器
//     ESP_ERROR_CHECK(webManager->start());

//     // 尝试连接WiFi（如果有保存的配置）
//     // TODO: 从NVS中读取保存的WiFi配置
    
//     // 如果WiFi连接失败，启动配网模式
//     if (!networkManager->isConnected()) {
//         ESP_LOGI(TAG, "Starting SmartConfig...");
//         ESP_ERROR_CHECK(networkManager->startSmartConfig());
//     }

//     // 连接Coze服务器
//     ESP_ERROR_CHECK(cozeManager->connect());

//     return ESP_OK;
// }

// void ChunFeng::stop() {
//     ESP_LOGI(TAG, "Stopping ChunFeng services...");
    
//     if (cozeManager) cozeManager->disconnect();
//     if (networkManager) networkManager->disconnect();
//     if (webManager) webManager->stop();
//     if (audioManager) audioManager->stopRecording();
// }