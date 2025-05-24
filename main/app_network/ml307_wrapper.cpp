/**
 * @file ml307_wrapper.cpp
 * @brief ML307 4G模块的C语言封装实现
 */

#include "ml307_wrapper.h"
#include "ml307_at_modem.h"
#include <string>

/**
 * @brief ML307调制解调器结构体
 */
struct ml307_modem_t {
    Ml307AtModem* cpp_modem;      ///< C++实现的ML307调制解调器对象指针
    std::string ip_str;           ///< IP地址字符串缓存
    std::string imei_str;         ///< IMEI号码字符串缓存
    std::string iccid_str;        ///< ICCID号码字符串缓存
    std::string module_name_str;  ///< 模块名称字符串缓存
    std::string carrier_name_str; ///< 运营商名称字符串缓存
};

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

/**
 * @brief 创建ML307调制解调器实例
 * @param tx_pin 发送引脚
 * @param rx_pin 接收引脚
 * @param buffer_size 缓冲区大小
 * @return ML307调制解调器实例指针
 */
ml307_modem_t* ml307_modem_create(gpio_num_t tx_pin, gpio_num_t rx_pin, int buffer_size) {
    ml307_modem_t* modem = new ml307_modem_t();
    modem->cpp_modem = new Ml307AtModem(tx_pin, rx_pin, buffer_size);
    return modem;
}

/**
 * @brief 设置调试模式
 * @param modem ML307调制解调器实例指针
 * @param debug 是否启用调试模式
 */
void ml307_modem_set_debug(ml307_modem_t* modem, bool debug) {
    if (modem && modem->cpp_modem) {
        modem->cpp_modem->SetDebug(debug);
    }
}

/**
 * @brief 设置波特率
 * @param modem ML307调制解调器实例指针
 * @param baudrate 波特率值
 */
void ml307_modem_set_baudrate(ml307_modem_t* modem, int baudrate) {
    if (modem && modem->cpp_modem) {
        modem->cpp_modem->SetBaudRate(baudrate);
    }
}

/**
 * @brief 等待网络就绪
 * @param modem ML307调制解调器实例指针
 * @return ESP_OK: 成功, ESP_FAIL: 失败
 */
esp_err_t ml307_modem_wait_network(ml307_modem_t* modem) {
    if (modem && modem->cpp_modem) {
        return modem->cpp_modem->WaitForNetworkReady() == 0 ? ESP_OK : ESP_FAIL;
    }
    return ESP_FAIL;
}

/**
 * @brief 获取IP地址
 * @param modem ML307调制解调器实例指针
 * @return IP地址字符串，失败返回nullptr
 */
const char* ml307_modem_get_ip(ml307_modem_t* modem) {
    if (modem && modem->cpp_modem) {
        modem->ip_str = modem->cpp_modem->ip_address();
        return modem->ip_str.c_str();
    }
    return nullptr;
}

/**
 * @brief 获取IMEI号码
 * @param modem ML307调制解调器实例指针
 * @return IMEI号码字符串，失败返回nullptr
 */
const char* ml307_modem_get_imei(ml307_modem_t* modem) {
    if (modem && modem->cpp_modem) {
        modem->imei_str = modem->cpp_modem->GetImei();
        return modem->imei_str.c_str();
    }
    return nullptr;
}

/**
 * @brief 获取ICCID号码
 * @param modem ML307调制解调器实例指针
 * @return ICCID号码字符串，失败返回nullptr
 */
const char* ml307_modem_get_iccid(ml307_modem_t* modem) {
    if (modem && modem->cpp_modem) {
        modem->iccid_str = modem->cpp_modem->GetIccid();
        return modem->iccid_str.c_str();
    }
    return nullptr;
}

/**
 * @brief 获取模块名称
 * @param modem ML307调制解调器实例指针
 * @return 模块名称字符串，失败返回nullptr
 */
const char* ml307_modem_get_module_name(ml307_modem_t* modem) {
    if (modem && modem->cpp_modem) {
        modem->module_name_str = modem->cpp_modem->GetModuleName();
        return modem->module_name_str.c_str();
    }
    return nullptr;
}

/**
 * @brief 获取运营商名称
 * @param modem ML307调制解调器实例指针
 * @return 运营商名称字符串，失败返回nullptr
 */
const char* ml307_modem_get_carrier_name(ml307_modem_t* modem) {
    if (modem && modem->cpp_modem) {
        modem->carrier_name_str = modem->cpp_modem->GetCarrierName();
        return modem->carrier_name_str.c_str();
    }
    return nullptr;
}

/**
 * @brief 获取信号强度
 * @param modem ML307调制解调器实例指针
 * @return 信号强度值(0-31)，失败返回-1
 */
int ml307_modem_get_csq(ml307_modem_t* modem) {
    if (modem && modem->cpp_modem) {
        return modem->cpp_modem->GetCsq();
    }
    return -1;
}

/**
 * @brief 销毁ML307调制解调器实例
 * @param modem ML307调制解调器实例指针
 */
void ml307_modem_destroy(ml307_modem_t* modem) {
    if (modem) {
        if (modem->cpp_modem) {
            delete modem->cpp_modem;
        }
        delete modem;
    }
}