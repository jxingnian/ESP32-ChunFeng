#include "bsp_config_network.hpp"
#include "bsp_config_network_page.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>

static const char* TAG = "BspConfigNetwork";

namespace chunfeng {

// 工具函数：将WiFi扫描结果转为JSON字符串
static std::string wifiListToJson(const std::vector<WiFiInfo>& list) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < list.size(); ++i) {
        oss << "{\"ssid\":\"" << list[i].ssid << "\","
            << "\"rssi\":" << list[i].rssi << ","
            << "\"is_encrypted\":" << (list[i].is_encrypted ? "true" : "false") << "}";
        if (i + 1 < list.size()) oss << ",";
    }
    oss << "]";
    return oss.str();
}

BspConfigNetwork::BspConfigNetwork() {}

BspConfigNetwork::~BspConfigNetwork() {
    stop();
}

bool BspConfigNetwork::start() {
    // 1. 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 初始化TCP/IP栈和事件循环
    esp_netif_init();
    esp_event_loop_create_default();
    
    // 创建默认网络接口
    static esp_netif_t* ap_netif = nullptr;
    static esp_netif_t* sta_netif = nullptr;
    if (!ap_netif) ap_netif = esp_netif_create_default_wifi_ap();
    if (!sta_netif) sta_netif = esp_netif_create_default_wifi_sta();

    // 2. 启动AP+STA模式
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t ap_config = {};
    strcpy((char*)ap_config.ap.ssid, "ChunFeng_Config");
    ap_config.ap.ssid_len = strlen("ChunFeng_Config");
    strcpy((char*)ap_config.ap.password, "12345678");
    ap_config.ap.max_connection = 4;
    ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();

    ap_sta_started_ = true;

    // 3. 启动HTTP服务器
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = nullptr;
    if (httpd_start(&server, &server_config) == ESP_OK) {
        http_server_ = server;

        // 根页面
        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = [](httpd_req_t *req) {
                // 兼容手机浏览器特殊请求
                std::string uri = req->uri;
                if (uri == "/generate_204" || uri == "/favicon.ico" || uri == "/hotspot-detect.html") {
                    httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
                    return ESP_OK;
                }
                httpd_resp_set_type(req, "text/html");
                httpd_resp_send(req, CONFIG_PAGE_HTML, HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            },
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &root);

        // 扫描WiFi
        httpd_uri_t scan = {
            .uri = "/scan",
            .method = HTTP_GET,
            .handler = [](httpd_req_t *req) {
                auto* self = reinterpret_cast<BspConfigNetwork*>(req->user_ctx);
                auto list = self->scanWiFi();
                std::string json = wifiListToJson(list);
                httpd_resp_set_type(req, "application/json");
                httpd_resp_send(req, json.c_str(), json.length());
                return ESP_OK;
            },
            .user_ctx = this
        };
        httpd_register_uri_handler(server, &scan);

        // 连接WiFi
        httpd_uri_t connect = {
            .uri = "/connect",
            .method = HTTP_POST,
            .handler = [](httpd_req_t *req) {
                char buf[128] = {0};
                httpd_req_recv(req, buf, sizeof(buf) - 1);
                std::string body(buf);
                // 解析表单
                auto getValue = [](const std::string& body, const std::string& key) {
                    auto pos = body.find(key + "=");
                    if (pos == std::string::npos) return std::string();
                    auto start = pos + key.length() + 1;
                    auto end = body.find("&", start);
                    return body.substr(start, end - start);
                };
                std::string ssid = getValue(body, "ssid");
                std::string pwd = getValue(body, "password");
                auto* self = reinterpret_cast<BspConfigNetwork*>(req->user_ctx);
                bool ok = self->connectWiFi(ssid, pwd);
                httpd_resp_send(req, ok ? "连接请求已发送" : "连接失败", HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            },
            .user_ctx = this
        };
        httpd_register_uri_handler(server, &connect);

        // 删除WiFi
        httpd_uri_t del = {
            .uri = "/delete",
            .method = HTTP_POST,
            .handler = [](httpd_req_t *req) {
                auto* self = reinterpret_cast<BspConfigNetwork*>(req->user_ctx);
                bool ok = self->deleteWiFi();
                httpd_resp_send(req, ok ? "已删除" : "删除失败", HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            },
            .user_ctx = this
        };
        httpd_register_uri_handler(server, &del);

        // 当前连接信息
        httpd_uri_t info = {
            .uri = "/info",
            .method = HTTP_GET,
            .handler = [](httpd_req_t *req) {
                auto* self = reinterpret_cast<BspConfigNetwork*>(req->user_ctx);
                std::string info = self->getCurrentWiFiInfo();
                httpd_resp_set_type(req, "text/plain; charset=utf-8"); // 新增，确保编码
                httpd_resp_send(req, info.c_str(), info.length());
                return ESP_OK;
            },
            .user_ctx = this
        };
        httpd_register_uri_handler(server, &info);
    }

    ESP_LOGI(TAG, "AP+STA和HTTP服务器已启动");
    return true;
}

void BspConfigNetwork::stop() {
    if (ap_sta_started_) {
        esp_wifi_stop();
        esp_wifi_deinit();
        ap_sta_started_ = false;
    }
    if (http_server_) {
        httpd_stop((httpd_handle_t)http_server_);
        http_server_ = nullptr;
    }
}

// 扫描周围WiFi
std::vector<WiFiInfo> BspConfigNetwork::scanWiFi() {
    std::vector<WiFiInfo> result;
    wifi_scan_config_t scan_config = {};
    esp_wifi_scan_start(&scan_config, true); // 阻塞扫描
    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);
    wifi_ap_record_t *ap_records = new wifi_ap_record_t[ap_num];
    esp_wifi_scan_get_ap_records(&ap_num, ap_records);
    for (int i = 0; i < ap_num; ++i) {
        WiFiInfo info;
        info.ssid = reinterpret_cast<const char*>(ap_records[i].ssid);
        info.rssi = ap_records[i].rssi;
        info.is_encrypted = ap_records[i].authmode != WIFI_AUTH_OPEN;
        result.push_back(info);
    }
    delete[] ap_records;
    return result;
}

// 连接指定WiFi
bool BspConfigNetwork::connectWiFi(const std::string& ssid, const std::string& password) {
    wifi_config_t sta_config = {};
    strncpy((char*)sta_config.sta.ssid, ssid.c_str(), sizeof(sta_config.sta.ssid) - 1);
    strncpy((char*)sta_config.sta.password, password.c_str(), sizeof(sta_config.sta.password) - 1);
    esp_wifi_set_config(WIFI_IF_STA, &sta_config);
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_OK) {
        last_ssid_ = ssid;
        last_password_ = password;
        // 可保存到NVS
        return true;
    }
    return false;
}

// 删除已保存WiFi
bool BspConfigNetwork::deleteWiFi() {
    // 删除NVS中保存的WiFi信息
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_cfg", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return false;
    nvs_erase_key(nvs_handle, "ssid");
    nvs_erase_key(nvs_handle, "pwd");
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    last_ssid_.clear();
    last_password_.clear();
    esp_wifi_disconnect();
    return true;
}

// 获取当前连接信息
std::string BspConfigNetwork::getCurrentWiFiInfo() {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        std::ostringstream oss;
        int ssid_len = strnlen((const char*)ap_info.ssid, sizeof(ap_info.ssid));
        oss << "SSID: " << std::string((const char*)ap_info.ssid, ssid_len) << ", RSSI: " << ap_info.rssi;
        return oss.str();
    }
    return "SSID: 未连接";
}

} // namespace chunfeng