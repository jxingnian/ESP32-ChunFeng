/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

#pragma once

#include "esp_gmf_err.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief  音频信息结构体
 * 
 * 该结构体用于存储音频相关的配置信息,包括采样率、声道数、采样位数等
 */
typedef struct {
    uint32_t sample_rate;     /*!< 音频采样率,单位Hz,如8000Hz、16000Hz等 */
    uint8_t  channel;         /*!< 音频声道数,如单声道为1,立体声为2 */
    uint8_t  bits_per_sample; /*!< 每个采样点的位数,如8位、16位、24位等 */
    uint8_t  port_num;        /*!< I2S端口号,用于指定使用哪个I2S接口 */
} esp_gmf_setup_periph_aud_info;

#if SOC_SDMMC_HOST_SUPPORTED

/**
 * @brief  初始化并配置SDMMC外设
 * 
 * 该函数用于初始化SD/MMC卡接口,包括引脚配置、时钟设置等
 *
 * @param[out]  out_card  输出参数,用于存储SDMMC卡的句柄
 */
void esp_gmf_setup_periph_sdmmc(void **out_card);

/**
 * @brief  释放SDMMC外设资源
 * 
 * 该函数用于关闭SDMMC接口,释放相关资源
 *
 * @param[in]  card  SDMMC卡的句柄
 */
void esp_gmf_teardown_periph_sdmmc(void *card);

#endif  // SOC_SDMMC_HOST_SUPPORTED

/**
 * @brief  初始化并配置WiFi
 * 
 * 该函数用于初始化WiFi模块,包括设置工作模式、连接参数等
 */
void esp_gmf_setup_periph_wifi(void);

/**
 * @brief  释放WiFi资源
 * 
 * 该函数用于关闭WiFi连接,释放相关资源
 */
void esp_gmf_teardown_periph_wifi(void);

/**
 * @brief  初始化并配置I2C接口
 * 
 * 该函数用于初始化指定的I2C接口,包括引脚配置、时钟设置等
 *
 * @param[in]  port  I2C端口号,用于指定要初始化的I2C接口
 */
void esp_gmf_setup_periph_i2c(int port);

/**
 * @brief  释放I2C接口资源
 * 
 * 该函数用于关闭指定的I2C接口,释放相关资源
 *
 * @param[in]  port  I2C端口号,指定要关闭的I2C接口
 */
void esp_gmf_teardown_periph_i2c(int port);

#ifdef USE_ESP_GMF_ESP_CODEC_DEV_IO

/**
 * @brief  初始化并配置音频编解码器
 * 
 * 该函数用于同时初始化播放和录音的编解码器设备
 *
 * @param[in]   play_info   播放设备的音频参数配置
 * @param[in]   rec_info    录音设备的音频参数配置
 * @param[out]  play_dev    输出参数,用于存储播放设备的句柄
 * @param[out]  record_dev  输出参数,用于存储录音设备的句柄
 *
 * @return
 *       - ESP_GMF_ERR_OK    初始化成功
 *       - ESP_GMF_ERR_FAIL  初始化失败
 */
esp_gmf_err_t esp_gmf_setup_periph_codec(esp_gmf_setup_periph_aud_info *play_info, esp_gmf_setup_periph_aud_info *rec_info,
                                         void **play_dev, void **record_dev);

/**
 * @brief  释放音频编解码器资源
 * 
 * 该函数用于关闭播放和录音设备,释放相关资源
 *
 * @param[in]  play_dev    播放设备的句柄
 * @param[in]  record_dev  录音设备的句柄
 */
void esp_gmf_teardown_periph_codec(void *play_dev, void *record_dev);

#endif /* USE_ESP_GMF_ESP_CODEC_DEV_IO */
#ifdef __cplusplus
}
#endif /* __cplusplus */
