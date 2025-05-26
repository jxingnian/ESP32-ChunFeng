/*
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 15:18:41
 * @LastEditTime: 2025-05-24 17:11:11
 * @LastEditors: 星年
 * @Description: 网络管理
 * @FilePath: \ESP32-ChunFeng\main\esp_network\esp_network.c
 * 遇事不决，可问春风
 */

#include "app_network.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_manager.h"
#include "http_server.h"
#include "ml307_wrapper.h"
#include "esp_coze_chat.h"
#include "coze_chat_app.h"
#include "esp_err.h"
#include <time.h>
#include "esp_sntp.h" 

static const char *TAG = "APP_NETWORK";

// 全局状态机实例
net_fsm_t g_net_fsm;

// 状态机初始化
static void net_fsm_init(net_fsm_t *fsm)
{
    if (fsm) {
        fsm->state = NET_STATE_WIFI_CONNECTING;
    }
}

// 状态切换
void net_fsm_set_state(net_fsm_t *fsm, net_state_t new_state)
{
    if (fsm) {
        fsm->state = new_state;
    }
}

// 获取当前状态
net_state_t net_fsm_get_state(net_fsm_t *fsm)
{
    if (fsm) {
        return fsm->state;
    }
    return NET_STATE_WIFI_CONNECTING;
}


// 检查并更新系统时间
static esp_err_t check_and_update_time(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // 打印当前时间
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "当前系统时间: %s", strftime_buf);

    // 检查是否需要更新时间(2025年之前)
    if (timeinfo.tm_year < (2025 - 1900)) {
        ESP_LOGI(TAG, "系统时间需要更新");
        
        // 配置 SNTP 服务
        esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_init();

        // 等待获取时间
        int retry = 0;
        const int retry_count = 10;
        while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
            ESP_LOGI(TAG, "等待SNTP同步... (%d/%d)", retry, retry_count);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        if (retry == retry_count) {
            ESP_LOGE(TAG, "SNTP同步超时");
            return ESP_FAIL;
        }

        // 获取更新后的时间
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "更新后的系统时间: %s", strftime_buf);
        
        // 停止SNTP服务
        esp_sntp_stop();
    }

    return ESP_OK;
}

// 状态机处理函数
static void net_fsm_handle(net_fsm_t *fsm)
{
    switch (fsm->state) {
        case NET_STATE_WIFI_CONNECTING:// 尝试WiFi连接
            ESP_LOGI(TAG, "State: 尝试WiFi连接");
            // if(wifi_try_connect() == ESP_OK){
            //     net_fsm_set_state(fsm, NET_STATE_WIFI_CONNECTED);
            // }else{
            //     ESP_LOGE(TAG, "WiFi连接失败");
            //     net_fsm_set_state(fsm, NET_STATE_4G_CONNECTING);
            // }
            break;
        case NET_STATE_4G_CONNECTING:// 尝试4G连接
            ESP_LOGI(TAG, "State: 尝试4G连接");
            ml307_modem_t* modem = ml307_modem_create(GPIO_NUM_13, GPIO_NUM_14, 2048);
            ml307_modem_set_debug(modem, true);
            ml307_modem_set_baudrate(modem, 921600);

            if (ml307_modem_wait_network(modem) == ESP_OK) {
                // Print IP Address
                ESP_LOGI(TAG, "IP Address: %s", ml307_modem_get_ip(modem));
                // Print IMEI, ICCID, Product ID, Carrier Name
                ESP_LOGI(TAG, "IMEI: %s", ml307_modem_get_imei(modem));
                ESP_LOGI(TAG, "ICCID: %s", ml307_modem_get_iccid(modem));
                ESP_LOGI(TAG, "Product ID: %s", ml307_modem_get_module_name(modem));
                ESP_LOGI(TAG, "Carrier Name: %s", ml307_modem_get_carrier_name(modem));
                // Print CSQ
                ESP_LOGI(TAG, "CSQ: %d", ml307_modem_get_csq(modem));
                net_fsm_set_state(fsm, NET_STATE_4G_CONNECTED);
            } else {
                ml307_modem_destroy(modem);
                net_fsm_set_state(fsm, NET_STATE_WIFI_CONFIG);
            }
            break;
        case NET_STATE_WIFI_CONFIG://进入WiFi配网
            ESP_LOGI(TAG, "State: 进入WiFi配网");
            start_webserver();// 启动配网
            while(1){
                vTaskDelay(pdMS_TO_TICKS(1000)); 
            }
            break;
        case NET_STATE_WIFI_CONNECTED:// WiFi已连接
            ESP_LOGI(TAG, "State: WiFi已连接");
            stop_webserver();// 停止配网
            check_and_update_time();

            /* 初始化COZE */
            esp_err_t ret = coze_chat_app_init();
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize audio board");
            }
            while(1){
                vTaskDelay(pdMS_TO_TICKS(1000)); 
            }
            break;
        case NET_STATE_4G_CONNECTED:// 4G已连接
            ESP_LOGI(TAG, "State: 4G已连接");
            while(1){
                vTaskDelay(pdMS_TO_TICKS(1000)); 
            }
            break;
            
        default:
            break;
    }
}

// 状态机任务
static void net_fsm_task(void *param)
{
    net_fsm_t *fsm = (net_fsm_t *)param;
    while (1) {
        net_fsm_handle(fsm);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒处理一次
    }
}

// 初始化SPIFFS
static esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,   // 最大打开文件数
        .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}


void network_init(void)
{
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化SPIFFS
    ESP_ERROR_CHECK(init_spiffs());


    // 初始化WiFi
    ESP_ERROR_CHECK(wifi_init());

    // 启动网络状态机
    net_fsm_init(&g_net_fsm);
    xTaskCreate(net_fsm_task, "net_fsm_task", 4096, &g_net_fsm, 5, NULL);
}
