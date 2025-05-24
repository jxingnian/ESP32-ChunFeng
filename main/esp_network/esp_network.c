/*
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 15:18:41
 * @LastEditTime: 2025-05-24 17:11:11
 * @LastEditors: 星年
 * @Description: 网络管理
 * @FilePath: \ESP32-ChunFeng\main\esp_network\esp_network.c
 * 遇事不决，可问春风
 */

#include "esp_network.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_manager.h"
#include "http_server.h"

static const char *TAG = "esp_network";

// 全局状态机实例
static net_fsm_t g_net_fsm;

// 状态机初始化
static void net_fsm_init(net_fsm_t *fsm)
{
    if (fsm) {
        fsm->state = NET_STATE_WIFI_CONNECTING;
    }
}

// 状态切换
static void net_fsm_set_state(net_fsm_t *fsm, net_state_t new_state)
{
    if (fsm) {
        fsm->state = new_state;
    }
}

// 获取当前状态
static net_state_t net_fsm_get_state(net_fsm_t *fsm)
{
    if (fsm) {
        return fsm->state;
    }
    return NET_STATE_WIFI_CONNECTING;
}
// 状态机处理函数
static void net_fsm_handle(net_fsm_t *fsm)
{
    switch (fsm->state) {
        case NET_STATE_WIFI_CONNECTING:// 尝试WiFi连接
            ESP_LOGI(TAG, "State: 尝试WiFi连接");
            if(wifi_try_connect() == ESP_OK){
                net_fsm_set_state(fsm, NET_STATE_WIFI_CONNECTED);
            }else{
                ESP_LOGE(TAG, "WiFi连接失败");
                net_fsm_set_state(fsm, NET_STATE_4G_CONNECTING);
            }
            break;
        case NET_STATE_4G_CONNECTING:// 尝试4G连接
            // 启动配网
            ESP_LOGI(TAG, "State: 尝试4G连接");
            // TODO: 实现配网逻辑
            break;
        case NET_STATE_WIFI_CONFIG://进入WiFi配网
            // 处理4G模式逻辑
            // 例如：初始化4G模块
            ESP_LOGI(TAG, "State: 进入WiFi配网");
            // TODO: 实现4G逻辑
            break;
        case NET_STATE_WIFI_CONNECTED:// WiFi已连接
            ESP_LOGI(TAG, "State: WiFi已连接");
            // 检查网络连接 如果WiFi和4G都没连接 则进入
            break;
        case NET_STATE_4G_CONNECTED:// 4G已连接
            ESP_LOGI(TAG, "State: 4G已连接");
            // 检查网络连接 如果WiFi和4G都没连接 则进入
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
