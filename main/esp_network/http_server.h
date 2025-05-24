/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-24 14:53:16
 * @LastEditTime: 2025-05-24 15:51:19
 * @LastEditors: 星年
 * @Description: HTTP服务器实现
 * @FilePath: \ESP32-ChunFeng\main\esp_network\http_server.h
 * @遇事不决，可问春风
 */

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include "esp_err.h"

#define FILE_PATH_MAX (128 + 128)
#define CHUNK_SIZE    (4096)

// 启动Web服务器
esp_err_t start_webserver(void);

// 停止Web服务器
esp_err_t stop_webserver(void);

#endif /* _HTTP_SERVER_H_ */