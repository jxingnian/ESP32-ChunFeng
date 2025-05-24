/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 15:18:51
 * @LastEditTime: 2025-05-24 16:10:06
 * @LastEditors: 星年
 * @Description: 网络管理
 * @FilePath: \ESP32-ChunFeng\main\esp_network\esp_network.h
 * @遇事不决，可问春风
 */

#ifndef ESP_NETWORK_H
#define ESP_NETWORK_H

#include "esp_err.h"

// 网络状态机定义
typedef enum {
    NET_STATE_WIFI_CONNECTING,   // 尝试WiFi连接
    NET_STATE_4G_CONNECTING,     // 尝试4G连接
    NET_STATE_WIFI_CONFIG,            // 进入配网
    NET_STATE_WIFI_CONNECTED,    // WiFi已连接
    NET_STATE_4G_CONNECTED       // 4G已连接
} net_state_t;

typedef struct {
    net_state_t state;
} net_fsm_t;
// 全局状态机实例
extern net_fsm_t g_net_fsm;

// 网络初始化
void network_init(void);

// 状态切换
void net_fsm_set_state(net_fsm_t *fsm, net_state_t new_state);

// 获取当前状态
net_state_t net_fsm_get_state(net_fsm_t *fsm);
#endif // ESP_NETWORK_H
