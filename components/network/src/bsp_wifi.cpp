/*
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 16:20:42
 * @LastEditTime: 2025-05-30 16:20:43
 * @LastEditors: 星年
 * @Description: ESP32 WiFi底层封装，支持STA模式初始化、NVS存取WiFi信息、连接/断开等
 * @FilePath: \ESP32-ChunFeng\components\network\src\bsp_wifi.c
 * 遇事不决，可问春风
 */
#include "bsp_wifi.hpp"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <cstring>
#include <iostream>

namespace chunfeng {

#define WIFI_NVS_NAMESPACE "wifi_cfg"
#define WIFI_NVS_KEY_SSID  "ssid"
#define WIFI_NVS_KEY_PWD   "pwd"

// 构造函数：初始化NVS和WiFi（STA模式）
BspWiFi::BspWiFi() : initialized_(false), connected_(false) {
    // 初始化NVS（非易失性存储），用于保存WiFi信息
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS空间不足或版本不兼容时，擦除并重新初始化
        nvs_flash_erase();
        nvs_flash_init();
    }
    // 初始化WiFi为STA（Station）模式
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    initialized_ = true;
}

// 析构函数：反初始化WiFi
BspWiFi::~BspWiFi() {
    if (initialized_) {
        esp_wifi_stop();      // 停止WiFi
        esp_wifi_deinit();    // 反初始化WiFi驱动
        initialized_ = false;
    }
}

// 保存WiFi信息（SSID和密码）到NVS
bool BspWiFi::saveWiFiInfo(const std::string& ssid, const std::string& password) {
    nvs_handle_t nvs_handle;
    // 以读写方式打开NVS命名空间
    esp_err_t err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return false;
    // 保存SSID和密码
    nvs_set_str(nvs_handle, WIFI_NVS_KEY_SSID, ssid.c_str());
    nvs_set_str(nvs_handle, WIFI_NVS_KEY_PWD, password.c_str());
    nvs_commit(nvs_handle); // 提交更改
    nvs_close(nvs_handle);  // 关闭NVS句柄
    return true;
}

// 从NVS读取WiFi信息（SSID和密码）
bool BspWiFi::loadWiFiInfo(std::string& ssid, std::string& password) {
    nvs_handle_t nvs_handle;
    // 以只读方式打开NVS命名空间
    esp_err_t err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return false;

    char ssid_buf[33] = {0}; // SSID最大长度32+1
    char pwd_buf[65] = {0};  // 密码最大长度64+1
    size_t ssid_len = sizeof(ssid_buf);
    size_t pwd_len = sizeof(pwd_buf);

    // 读取SSID
    err = nvs_get_str(nvs_handle, WIFI_NVS_KEY_SSID, ssid_buf, &ssid_len);
    if (err != ESP_OK) { nvs_close(nvs_handle); return false; }
    // 读取密码
    err = nvs_get_str(nvs_handle, WIFI_NVS_KEY_PWD, pwd_buf, &pwd_len);
    if (err != ESP_OK) { nvs_close(nvs_handle); return false; }

    ssid = ssid_buf;
    password = pwd_buf;
    nvs_close(nvs_handle);
    return true;
}

// 删除NVS中保存的WiFi信息
bool BspWiFi::deleteWiFiInfo() {
    nvs_handle_t nvs_handle;
    // 以读写方式打开NVS命名空间
    esp_err_t err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return false;
    // 擦除SSID和密码键
    nvs_erase_key(nvs_handle, WIFI_NVS_KEY_SSID);
    nvs_erase_key(nvs_handle, WIFI_NVS_KEY_PWD);
    nvs_commit(nvs_handle); // 提交更改
    nvs_close(nvs_handle);  // 关闭NVS句柄
    return true;
}

// 连接指定WiFi（STA模式）
bool BspWiFi::connect(const std::string& ssid, const std::string& password) {
    if (!initialized_) return false;
    wifi_config_t wifi_config = {};
    // 拷贝SSID和密码到配置结构体
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password) - 1);
    // 设置WiFi配置
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    // 发起连接
    esp_err_t err = esp_wifi_connect();
    connected_ = (err == ESP_OK);
    return connected_;
}

// 断开WiFi连接
void BspWiFi::disconnect() {
    if (!initialized_) return;
    esp_wifi_disconnect();
    connected_ = false;
}

// 头文件实现
} // namespace chunfeng
