 #ifndef ML307_WRAPPER_H
#define ML307_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "driver/gpio.h"

// ML307 Modem 句柄
typedef struct ml307_modem_t ml307_modem_t;

// 创建 ML307 Modem 实例
ml307_modem_t* ml307_modem_create(gpio_num_t tx_pin, gpio_num_t rx_pin, int buffer_size);

// 设置调试模式
void ml307_modem_set_debug(ml307_modem_t* modem, bool debug);

// 设置波特率
void ml307_modem_set_baudrate(ml307_modem_t* modem, int baudrate);

// 等待网络就绪
esp_err_t ml307_modem_wait_network(ml307_modem_t* modem);

// 获取IP地址
const char* ml307_modem_get_ip(ml307_modem_t* modem);

// 获取IMEI
const char* ml307_modem_get_imei(ml307_modem_t* modem);

// 获取ICCID
const char* ml307_modem_get_iccid(ml307_modem_t* modem);

// 获取模块名称
const char* ml307_modem_get_module_name(ml307_modem_t* modem);

// 获取运营商名称
const char* ml307_modem_get_carrier_name(ml307_modem_t* modem);

// 获取信号强度
int ml307_modem_get_csq(ml307_modem_t* modem);

// 销毁 ML307 Modem 实例
void ml307_modem_destroy(ml307_modem_t* modem);

#ifdef __cplusplus
}
#endif

#endif // ML307_WRAPPER_H