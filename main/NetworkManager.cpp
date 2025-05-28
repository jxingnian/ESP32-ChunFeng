 #include "NetworkManager.hpp"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_netif.h"
#include <string.h>

const char* NetworkManager::TAG = "NetworkManager";

NetworkManager::NetworkManager() 
    : wifiConnected(false)
    , fourGConnected(false) {
}

NetworkManager::~NetworkManager() {
    disconnect();
}

esp_err_t NetworkManager::init() {
    ESP_LOGI(TAG, "Initializing NetworkManager...");

    // 初始化底层TCP/IP栈
    ESP_ERROR_CHECK(esp_netif_init());
    
    // 创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 创建默认WiFi STA实例
    esp_netif_create_default_wifi_sta();
    
    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册WiFi事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                             &NetworkManager::wifiEventHandler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &NetworkManager::wifiEventHandler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID,
                                             &NetworkManager::wifiEventHandler, this));
    
    return ESP_OK;
}

esp_err_t NetworkManager::startWiFi() {
    ESP_LOGI(TAG, "Starting WiFi in STA mode...");
    
    // 设置WiFi工作模式为STA模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    // 启动WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    return ESP_OK;
}

esp_err_t NetworkManager::connect(const std::string& ssid, const std::string& password) {
    if (ssid.empty()) {
        ESP_LOGE(TAG, "SSID cannot be empty");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Connecting to WiFi network: %s", ssid.c_str());
    
    // 配置WiFi连接参数
    wifi_config_t wifi_config = {};
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    
    // 复制SSID和密码到配置结构体
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    if (!password.empty()) {
        strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password) - 1);
    }
    
    // 设置WiFi配置
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    // 启动WiFi连接
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    return ESP_OK;
}

esp_err_t NetworkManager::startSmartConfig() {
    ESP_LOGI(TAG, "Starting SmartConfig...");
    
    // 设置SmartConfig类型
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    
    // 启动SmartConfig
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    
    return ESP_OK;
}

esp_err_t NetworkManager::stopSmartConfig() {
    ESP_LOGI(TAG, "Stopping SmartConfig...");
    
    // 停止SmartConfig
    ESP_ERROR_CHECK(esp_smartconfig_stop());
    
    return ESP_OK;
}

void NetworkManager::disconnect() {
    ESP_LOGI(TAG, "Disconnecting from network...");
    
    if (wifiConnected) {
        esp_wifi_disconnect();
        esp_wifi_stop();
        wifiConnected = false;
    }
    
    if (fourGConnected) {
        disconnect4G();
        fourGConnected = false;
    }
}

bool NetworkManager::isConnected() const {
    return wifiConnected || fourGConnected;
}

// 4G相关实现
esp_err_t NetworkManager::init4G() {
    ESP_LOGI(TAG, "Initializing 4G module...");
    // TODO: 实现4G模块初始化
    return ESP_OK;
}

esp_err_t NetworkManager::start4G() {
    ESP_LOGI(TAG, "Starting 4G connection...");
    // TODO: 实现4G启动
    return ESP_OK;
}

esp_err_t NetworkManager::connect4G() {
    ESP_LOGI(TAG, "Connecting to 4G network...");
    // TODO: 实现4G连接
    return ESP_OK;
}

void NetworkManager::disconnect4G() {
    ESP_LOGI(TAG, "Disconnecting from 4G network...");
    // TODO: 实现4G断开连接
    fourGConnected = false;
}

void NetworkManager::wifiEventHandler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data) {
    NetworkManager* self = static_cast<NetworkManager*>(arg);
    
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi station mode started");
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WiFi disconnected, trying to reconnect...");
                self->wifiConnected = false;
                esp_wifi_connect();
                break;
                
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected");
                break;
        }
    } 
    else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
            self->wifiConnected = true;
        }
    }
    else if (event_base == SC_EVENT) {
        switch (event_id) {
            case SC_EVENT_SCAN_DONE:
                ESP_LOGI(TAG, "SmartConfig scan done");
                break;
                
            case SC_EVENT_FOUND_CHANNEL:
                ESP_LOGI(TAG, "SmartConfig found channel");
                break;
                
            case SC_EVENT_GOT_SSID_PSWD: {
                ESP_LOGI(TAG, "SmartConfig got SSID and password");
                
                smartconfig_event_got_ssid_pswd_t* evt = (smartconfig_event_got_ssid_pswd_t*)event_data;
                wifi_config_t wifi_config = {};
                
                // 复制SSID和密码
                memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
                memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
                
                // 设置WiFi配置
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
                
                // 连接到WiFi
                esp_wifi_connect();
                break;
            }
                
            case SC_EVENT_SEND_ACK_DONE:
                ESP_LOGI(TAG, "SmartConfig send ACK done");
                esp_smartconfig_stop();
                break;
        }
    }
}