/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

/* 包含必要的头文件 */
#include "esp_log.h"                  // ESP日志功能
#include "esp_event.h"                // 事件系统
#include "nvs_flash.h"                // 非易失性存储
#include "esp_gmf_oal_mem.h"          // GMF内存管理
#include "esp_gmf_gpio_config.h"      // GPIO配置
#include "esp_gmf_setup_peripheral.h" // 外设设置接口

#include "esp_gmf_io_codec_dev.h"     // 编解码器设备IO
#include "driver/i2s_std.h"           // 标准I2S驱动

/* I2S接收通道相关变量 */
i2s_chan_handle_t            rx_handle   = NULL;  // I2S接收通道句柄

/* I2S发送通道相关变量 */
i2s_chan_handle_t            tx_handle    = NULL; // I2S发送通道句柄

/* I2S创建模式枚举 */
typedef enum {
    I2S_CREATE_MODE_TX_ONLY   = 0,    // 仅发送模式
    I2S_CREATE_MODE_RX_ONLY   = 1,    // 仅接收模式
    I2S_CREATE_MODE_TX_AND_RX = 2,    // 发送和接收模式
} i2s_create_mode_t;

/* 全局变量 */
static const char        *TAG = "AUDIO_SETUP";    // 日志标签

/**
 * @brief 初始化I2S发送通道
 * @param aud_info 音频参数信息
 * @return ESP_OK: 成功, 其他: 失败
 */
static esp_err_t setup_periph_i2s_tx_init(esp_gmf_setup_periph_aud_info *aud_info)
{
    // 其他芯片使用标准I2S配置
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(aud_info->sample_rate),  // 时钟配置
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(aud_info->bits_per_sample, aud_info->channel), // 时隙配置
        .gpio_cfg = {
            .mclk = ESP_GMF_I2S_DAC_MCLK_IO_NUM,  // 主时钟引脚
            .bclk = ESP_GMF_I2S_DAC_BCLK_IO_NUM,  // 位时钟引脚
            .ws = ESP_GMF_I2S_DAC_WS_IO_NUM,      // 字选择引脚
            .dout = ESP_GMF_I2S_DAC_DO_IO_NUM,    // 数据输出引脚
            .din = ESP_GMF_I2S_DAC_DI_IO_NUM,     // 数据输入引脚
        },
    };
    return i2s_channel_init_std_mode(tx_handle, &std_cfg);
}

/**
 * @brief 初始化I2S接收通道
 * @param aud_info 音频参数信息
 * @return ESP_OK: 成功, 其他: 失败
 */
static esp_err_t setup_periph_i2s_rx_init(esp_gmf_setup_periph_aud_info *aud_info)
{
    // 标准I2S接收配置
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(aud_info->sample_rate),  // 时钟配置
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(aud_info->bits_per_sample, aud_info->channel), // 时隙配置
        .gpio_cfg = {
            .mclk = ESP_GMF_I2S_ADC_MCLK_IO_NUM,  // 主时钟引脚
            .bclk = ESP_GMF_I2S_ADC_BCLK_IO_NUM,  // 位时钟引脚
            .ws = ESP_GMF_I2S_ADC_WS_IO_NUM,      // 字选择引脚
            .dout = ESP_GMF_I2S_ADC_DO_IO_NUM,    // 数据输出引脚
            .din = ESP_GMF_I2S_ADC_DI_IO_NUM,     // 数据输入引脚
        },
    };
    return i2s_channel_init_std_mode(rx_handle, &std_cfg);
}

/**
 * @brief 创建I2S通道
 * @param mode I2S创建模式(仅发送/仅接收/发送和接收)
 * @param aud_info 音频参数信息
 * @return ESP_OK: 成功, 其他: 失败
 */
static esp_err_t setup_periph_create_i2s(i2s_create_mode_t mode, esp_gmf_setup_periph_aud_info *aud_info)
{
    // I2S通道配置
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(aud_info->port_num, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;  // 自动清除缓冲区
    esp_err_t ret = ESP_OK;

    // 根据模式创建I2S通道
    if (mode == I2S_CREATE_MODE_TX_ONLY) {
        // 仅创建发送通道
        ret = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, {return ret;}, "Failed to new I2S tx handle");
        ret = setup_periph_i2s_tx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, {return ret;}, "Failed to initialize I2S tx");
    } else if (mode == I2S_CREATE_MODE_RX_ONLY) {
        // 仅创建接收通道
        ret = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, {return ret;}, "Failed to new I2S rx handle");
        ret = setup_periph_i2s_rx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, {return ret;}, "Failed to initialize I2S rx");
    } else {
        // 同时创建发送和接收通道
        ret = i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, {return ret;}, "Failed to new I2S tx and rx handle");
        ret = setup_periph_i2s_tx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, {return ret;}, "Failed to initialize I2S tx");
        ret = setup_periph_i2s_rx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, {return ret;}, "Failed to initialize I2S rx");
    }
    return ret;
}

/**
 * @brief 设置音频编解码器外设
 * @param play_info 播放设备配置信息
 * @param rec_info 录音设备配置信息  
 * @param play_dev 播放设备句柄指针
 * @param record_dev 录音设备句柄指针
 * @return ESP_GMF_ERR_OK 成功
 *         ESP_GMF_ERR_FAIL 失败
 * @note 该函数支持以下几种配置模式:
 *       1. 同时配置播放和录音设备(play_dev和record_dev都不为NULL)
 *       2. 仅配置播放设备(play_dev不为NULL, record_dev为NULL)
 *       3. 仅配置录音设备(play_dev为NULL, record_dev不为NULL)
 */
esp_gmf_err_t esp_gmf_setup_periph_codec(esp_gmf_setup_periph_aud_info *play_info, esp_gmf_setup_periph_aud_info *rec_info,
                                         void **play_dev, void **record_dev)
{
    // 同时配置播放和录音设备
    if ((play_dev != NULL) && (record_dev != NULL)) {
        // 如果播放和录音使用相同的I2S端口,创建双向I2S
        if (play_info->port_num == rec_info->port_num) {
            ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_TX_AND_RX, play_info),
                                  {return ESP_GMF_ERR_FAIL;}, "Failed to create I2S tx and rx");
        } else {
            // 使用不同的I2S端口,分别创建发送和接收I2S
            ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_TX_ONLY, play_info),
                                  {return ESP_GMF_ERR_FAIL;}, "Failed to create I2S tx");
            ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_RX_ONLY, rec_info),
                                  {return ESP_GMF_ERR_FAIL;}, "Failed to create I2S rx");
        }
    } else if (play_dev != NULL) {
        // 仅配置播放设备
        ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_TX_ONLY, play_info),
                              {return ESP_GMF_ERR_FAIL;}, "Failed to create I2S tx");
    } else if (record_dev != NULL) {
        // 仅配置录音设备
        ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_RX_ONLY, rec_info),
                              {return ESP_GMF_ERR_FAIL;}, "Failed to create I2S rx");
    } else {
        // 无效的参数组合
        return ESP_GMF_ERR_FAIL;
    }
    return ESP_GMF_ERR_OK;
}
