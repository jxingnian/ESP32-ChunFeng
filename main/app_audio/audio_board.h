#ifndef _AUDIO_BOARD_H_
#define _AUDIO_BOARD_H_

#include "esp_err.h"
#include <stddef.h>

/**
 * @brief 音频板初始化
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_board_init(void);

/**
 * @brief 音频输入初始化
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_input_init(void);

/**
 * @brief 音频输出初始化
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_output_init(void);

/**
 * @brief 获取音频数据
 * @param buf 数据缓冲区
 * @param len 缓冲区长度
 * @param bytes_read 实际读取的字节数
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_data_get(void *buf, size_t len, size_t *bytes_read);

/**
 * @brief 播放音频数据
 * @param buf 数据缓冲区
 * @param len 缓冲区长度
 * @param bytes_written 实际写入的字节数
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_data_play(void *buf, size_t len, size_t *bytes_written);

/**
 * @brief 设置音量
 * @param volume 音量值(0-100)
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_volume_set(int volume);

/**
 * @brief 获取当前音量
 * @param volume 音量值指针
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_volume_get(int *volume);

/**
 * @brief 音频板反初始化
 * @return ESP_OK: 成功, 其他: 失败
 */
esp_err_t audio_board_deinit(void);

#endif /* _AUDIO_BOARD_H_ */
