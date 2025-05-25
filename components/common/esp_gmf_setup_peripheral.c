/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* 包含必要的头文件 */
#include "esp_log.h"                    // ESP日志功能
#include "driver/sdmmc_host.h"          // SD/MMC主机驱动
#include "vfs_fat_internal.h"           // FAT文件系统内部接口
#include "esp_wifi.h"                   // WiFi功能
#include "esp_event.h"                  // 事件系统
#include "nvs_flash.h"                  // 非易失性存储
#include "esp_gmf_oal_mem.h"            // GMF内存管理
#include "driver/i2c_master.h"          // I2C主机驱动
#include "driver/i2c.h"                 // I2C驱动
#include "esp_gmf_gpio_config.h"        // GPIO配置
#include "esp_gmf_setup_peripheral.h"   // 外设设置
#include "sd_pwr_ctrl_by_on_chip_ldo.h" // SD卡电源控制

/* 如果定义了USE_ESP_GMF_ESP_CODEC_DEV_IO,包含音频编解码相关头文件 */
#ifdef USE_ESP_GMF_ESP_CODEC_DEV_IO
#include "esp_gmf_io_codec_dev.h"       // 编解码器设备IO
#include "esp_gmf_io_i2s_pdm.h"         // I2S/PDM接口
#include "driver/i2s_std.h"             // 标准I2S驱动
#include "driver/i2s_tdm.h"             // TDM驱动
#include "driver/i2s_pdm.h"             // PDM驱动
#include "esp_codec_dev_defaults.h"      // 编解码器默认配置
#endif  /* USE_ESP_GMF_ESP_CODEC_DEV_IO */

/* WiFi相关宏定义 */
#define WIFI_CONNECTED_BIT     BIT0     // WiFi连接成功标志位
#define WIFI_FAIL_BIT          BIT1     // WiFi连接失败标志位
#define WIFI_RECONNECT_RETRIES 30       // WiFi重连尝试次数

/* 如果定义了USE_ESP_GMF_ESP_CODEC_DEV_IO,声明音频相关全局变量 */
#ifdef USE_ESP_GMF_ESP_CODEC_DEV_IO
i2s_chan_handle_t            rx_handle   = NULL;  // I2S接收通道句柄
const audio_codec_data_if_t *in_data_if  = NULL;  // 音频输入数据接口
const audio_codec_ctrl_if_t *in_ctrl_if  = NULL;  // 音频输入控制接口
const audio_codec_if_t      *in_codec_if = NULL;  // 音频输入编解码器接口

i2s_chan_handle_t            tx_handle    = NULL; // I2S发送通道句柄
const audio_codec_data_if_t *out_data_if  = NULL; // 音频输出数据接口
const audio_codec_ctrl_if_t *out_ctrl_if  = NULL; // 音频输出控制接口
const audio_codec_if_t      *out_codec_if = NULL; // 音频输出编解码器接口

const audio_codec_gpio_if_t *gpio_if = NULL;      // GPIO接口
#endif  /* USE_ESP_GMF_ESP_CODEC_DEV_IO */

/* I2S创建模式枚举 */
typedef enum {
    I2S_CREATE_MODE_TX_ONLY   = 0,      // 仅发送模式
    I2S_CREATE_MODE_RX_ONLY   = 1,      // 仅接收模式
    I2S_CREATE_MODE_TX_AND_RX = 2,      // 发送和接收模式
} i2s_create_mode_t;

/* 全局变量定义 */
static const char        *TAG = "SETUP_PERIPH";  // 日志标签
i2c_master_bus_handle_t   i2c_handle  = NULL;    // I2C总线句柄

/* 如果定义了USE_ESP_GMF_ESP_CODEC_DEV_IO,实现音频相关函数 */
#ifdef USE_ESP_GMF_ESP_CODEC_DEV_IO

/* 初始化I2S发送通道 */
static esp_err_t setup_periph_i2s_tx_init(esp_gmf_setup_periph_aud_info *aud_info)
{
    // 配置I2S标准模式参数
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(aud_info->sample_rate),  // 时钟配置
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(aud_info->bits_per_sample, aud_info->channel), // 时隙配置
        .gpio_cfg = {  // GPIO引脚配置
            .mclk = ESP_GMF_I2S_DAC_MCLK_IO_NUM,  // 主时钟引脚
            .bclk = ESP_GMF_I2S_DAC_BCLK_IO_NUM,  // 位时钟引脚
            .ws = ESP_GMF_I2S_DAC_WS_IO_NUM,      // 字选择引脚
            .dout = ESP_GMF_I2S_DAC_DO_IO_NUM,    // 数据输出引脚
            .din = ESP_GMF_I2S_DAC_DI_IO_NUM,     // 数据输入引脚
        },
    };
    return i2s_channel_init_std_mode(tx_handle, &std_cfg);  // 初始化I2S通道
}

/* 初始化I2S接收通道 */
static esp_err_t setup_periph_i2s_rx_init(esp_gmf_setup_periph_aud_info *aud_info)
{
    // 配置I2S标准模式参数
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(aud_info->sample_rate),  // 时钟配置
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(aud_info->bits_per_sample, aud_info->channel), // 时隙配置
        .gpio_cfg = {  // GPIO引脚配置
            .mclk = ESP_GMF_I2S_ADC_MCLK_IO_NUM,  // 主时钟引脚
            .bclk = ESP_GMF_I2S_ADC_BCLK_IO_NUM,  // 位时钟引脚
            .ws = ESP_GMF_I2S_ADC_WS_IO_NUM,      // 字选择引脚
            .dout = ESP_GMF_I2S_ADC_DO_IO_NUM,    // 数据输出引脚
            .din = ESP_GMF_I2S_ADC_DI_IO_NUM,     // 数据输入引脚
        },
    };
    return i2s_channel_init_std_mode(rx_handle, &std_cfg);  // 初始化I2S通道
}

/* 创建I2S通道 */
static esp_err_t setup_periph_create_i2s(i2s_create_mode_t mode, esp_gmf_setup_periph_aud_info *aud_info)
{
    // 配置I2S通道参数
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(aud_info->port_num, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;  // 启用自动清除功能
    esp_err_t ret = ESP_OK;

    // 根据模式创建I2S通道
    if (mode == I2S_CREATE_MODE_TX_ONLY) {  // 仅发送模式
        ret = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, { return ret;}, "Failed to new I2S tx handle");
        ret = setup_periph_i2s_tx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, { return ret;}, "Failed to initialize I2S tx");
    } else if (mode == I2S_CREATE_MODE_RX_ONLY) {  // 仅接收模式
        ret = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, { return ret;}, "Failed to new I2S rx handle");
        ret = setup_periph_i2s_rx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, { return ret;}, "Failed to initialize I2S rx");
    } else {  // 发送和接收模式
        ret = i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, { return ret;}, "Failed to new I2S tx and rx handle");
        ret = setup_periph_i2s_tx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, { return ret;}, "Failed to initialize I2S tx");
        ret = setup_periph_i2s_rx_init(aud_info);
        ESP_GMF_RET_ON_NOT_OK(TAG, ret, { return ret;}, "Failed to initialize I2S rx");
    }
    return ret;
}

/* 创建新的I2S数据接口 */
static const audio_codec_data_if_t *setup_periph_new_i2s_data(void *tx_hd, void *rx_hd)
{
    audio_codec_i2s_cfg_t i2s_cfg = {
        .rx_handle = rx_hd,  // 接收通道句柄
        .tx_handle = tx_hd,  // 发送通道句柄
    };
    return audio_codec_new_i2s_data(&i2s_cfg);  // 创建I2S数据接口
}

/* 创建新的播放编解码器 */
static void setup_periph_new_play_codec()
{
    // 配置I2C控制接口
    audio_codec_i2c_cfg_t i2c_ctrl_cfg = {
        .addr = ES8311_CODEC_DEFAULT_ADDR,  // ES8311编解码器地址
        .port = 0,
        .bus_handle = i2c_handle
    };
    out_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_ctrl_cfg);
    gpio_if = audio_codec_new_gpio();  // 创建GPIO接口

    // 配置ES8311编解码器
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,  // DAC工作模式
        .ctrl_if = out_ctrl_if,
        .gpio_if = gpio_if,
        .pa_pin = ESP_GMF_AMP_IO_NUM,  // 功放引脚
        .use_mclk = false,
    };
    out_codec_if = es8311_codec_new(&es8311_cfg);  // 创建编解码器接口
}

/* 创建新的录音编解码器 */
static void setup_periph_new_record_codec()
{
#if CODEC_ES8311_IN_OUT  // ES8311用于输入输出
    // 配置I2C控制接口
    audio_codec_i2c_cfg_t i2c_ctrl_cfg = {
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .port = 0,
        .bus_handle = i2c_handle
    };
    in_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_ctrl_cfg);
    gpio_if = audio_codec_new_gpio();

    // 配置ES8311编解码器
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,  // 同时工作于ADC和DAC模式
        .ctrl_if = in_ctrl_if,
        .gpio_if = gpio_if,
        .pa_pin = ESP_GMF_AMP_IO_NUM,
        .use_mclk = false,
    };
    in_codec_if = es8311_codec_new(&es8311_cfg);

#elif CODEC_ES7210_IN_ES8311_OUT  // ES7210用于输入,ES8311用于输出
    // 配置I2C控制接口
    audio_codec_i2c_cfg_t i2c_ctrl_cfg = {
        .addr = ES7210_CODEC_DEFAULT_ADDR,
        .port = 0,
        .bus_handle = i2c_handle
    };
    in_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_ctrl_cfg);

    // 配置ES7210编解码器
    es7210_codec_cfg_t es7210_cfg = {
        .ctrl_if = in_ctrl_if,
        .mic_selected = ES7120_SEL_MIC1 | ES7120_SEL_MIC2 | ES7120_SEL_MIC3,  // 选择使用的麦克风
    };
    in_codec_if = es7210_codec_new(&es7210_cfg);
#endif
}

/* 创建编解码器设备 */
static esp_codec_dev_handle_t setup_periph_create_codec_dev(esp_codec_dev_type_t dev_type, esp_gmf_setup_periph_aud_info *aud_info)
{
    // 配置采样信息
    esp_codec_dev_sample_info_t fs = {
        .sample_rate = aud_info->sample_rate,        // 采样率
        .channel = aud_info->channel,                // 通道数
        .bits_per_sample = aud_info->bits_per_sample,// 采样位数
    };
    esp_codec_dev_cfg_t dev_cfg = {0};
    esp_codec_dev_handle_t codec_dev = NULL;

    if (dev_type == ESP_CODEC_DEV_TYPE_OUT) {  // 输出设备
        dev_cfg.codec_if = out_codec_if;
        dev_cfg.data_if = out_data_if;
        dev_cfg.dev_type = ESP_CODEC_DEV_TYPE_OUT;
        codec_dev = esp_codec_dev_new(&dev_cfg);
        esp_codec_dev_set_out_vol(codec_dev, 80.0);  // 设置输出音量
        esp_codec_dev_open(codec_dev, &fs);
    } else {  // 输入设备
        dev_cfg.codec_if = in_codec_if;
        dev_cfg.data_if = in_data_if;
        dev_cfg.dev_type = ESP_CODEC_DEV_TYPE_IN;
        codec_dev = esp_codec_dev_new(&dev_cfg);
        esp_codec_dev_set_in_gain(codec_dev, 30.0);  // 设置输入增益
        esp_codec_dev_open(codec_dev, &fs);
    }
    return codec_dev;
}

/* 设置播放编解码器 */
static void setup_periph_play_codec(esp_gmf_setup_periph_aud_info *aud_info, void **play_dev)
{
    out_data_if = setup_periph_new_i2s_data(tx_handle, NULL);  // 创建I2S输出数据接口
    setup_periph_new_play_codec();  // 初始化播放编解码器
    *play_dev = setup_periph_create_codec_dev(ESP_CODEC_DEV_TYPE_OUT, aud_info);  // 创建输出编解码器设备
}

/* 设置录音编解码器 */
static void setup_periph_record_codec(esp_gmf_setup_periph_aud_info *aud_info, void **record_dev)
{
    in_data_if = setup_periph_new_i2s_data(NULL, rx_handle);  // 创建I2S输入数据接口
    setup_periph_new_record_codec();  // 初始化录音编解码器
    *record_dev = setup_periph_create_codec_dev(ESP_CODEC_DEV_TYPE_IN, aud_info);  // 创建输入编解码器设备
}

/* 释放播放编解码器资源 */
void teardown_periph_play_codec(void *play_dev)
{
    esp_codec_dev_close(play_dev);  // 关闭编解码器设备
    esp_codec_dev_delete(play_dev);  // 删除编解码器设备
    audio_codec_delete_codec_if(out_codec_if);  // 删除编解码器接口
    audio_codec_delete_ctrl_if(out_ctrl_if);    // 删除控制接口
    audio_codec_delete_gpio_if(gpio_if);        // 删除GPIO接口
    audio_codec_delete_data_if(out_data_if);    // 删除数据接口
    i2s_channel_disable(tx_handle);             // 禁用I2S通道
    i2s_del_channel(tx_handle);                 // 删除I2S通道
    tx_handle = NULL;
}

/* 释放录音编解码器资源 */
void teardown_periph_record_codec(void *record_dev)
{
    esp_codec_dev_close(record_dev);  // 关闭编解码器设备
    esp_codec_dev_delete(record_dev);  // 删除编解码器设备
    audio_codec_delete_codec_if(in_codec_if);  // 删除编解码器接口
    audio_codec_delete_ctrl_if(in_ctrl_if);    // 删除控制接口
    audio_codec_delete_data_if(in_data_if);    // 删除数据接口
    i2s_channel_disable(rx_handle);            // 禁用I2S通道
    i2s_del_channel(rx_handle);                // 删除I2S通道
    rx_handle = NULL;
}
#endif  /* USE_ESP_GMF_ESP_CODEC_DEV_IO */

/* 设置SD/MMC卡 */
void esp_gmf_setup_periph_sdmmc(void **out_card)
{
#if defined SOC_SDMMC_HOST_SUPPORTED
    sdmmc_card_t *card = NULL;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();  // 获取默认主机配置
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();  // 获取默认插槽配置

#if defined SOC_SDMMC_IO_POWER_EXTERNAL
    host.slot = SDMMC_HOST_SLOT_0;  // 使用插槽0
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;  // 设置高速模式
    // 配置LDO电源控制
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = 4,
    };
    sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
    esp_err_t ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create a new on-chip LDO power control driver");
        return;
    }
    host.pwr_ctrl_handle = pwr_ctrl_handle;
#endif  /* SOC_SDMMC_IO_POWER_EXTERNAL  */

    slot_config.width = ESP_GMF_SD_WIDTH;  // 设置数据线宽度
    // 配置FAT文件系统挂载参数
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 12 * 1024
    };

#if SOC_SDMMC_USE_GPIO_MATRIX
    // 配置SD卡GPIO引脚
    slot_config.clk = ESP_GMF_SD_CLK_IO_NUM;
    slot_config.cmd = ESP_GMF_SD_CMD_IO_NUM;
    slot_config.d0 = ESP_GMF_SD_D0_IO_NUM;
    slot_config.d1 = ESP_GMF_SD_D1_IO_NUM;
    slot_config.d2 = ESP_GMF_SD_D2_IO_NUM;
    slot_config.d3 = ESP_GMF_SD_D3_IO_NUM;
    slot_config.d4 = ESP_GMF_SD_D4_IO_NUM;
    slot_config.d5 = ESP_GMF_SD_D5_IO_NUM;
    slot_config.d6 = ESP_GMF_SD_D6_IO_NUM;
    slot_config.d7 = ESP_GMF_SD_D7_IO_NUM;
    slot_config.cd = ESP_GMF_SD_CD_IO_NUM;
    slot_config.wp = ESP_GMF_SD_WP_IO_NUM;
#endif  /* SOC_SDMMC_USE_GPIO_MATRIX */

#if defined CONFIG_IDF_TARGET_ESP32P4
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;  // 启用内部上拉
#endif  /* defined CONFIG_IDF_TARGET_ESP32P4 */

    esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);  // 挂载SD卡
    *out_card = card;
#else
    ESP_LOGE(TAG, "SDMMC host is not supported");
#endif  /* SOC_SDMMC_HOST_SUPPORTED */
}

/* 卸载SD/MMC卡 */
void esp_gmf_teardown_periph_sdmmc(void *card)
{
#if defined SOC_SDMMC_HOST_SUPPORTED
    esp_vfs_fat_sdcard_unmount("/sdcard", card);  // 卸载SD卡
#endif  /* SOC_SDMMC_HOST_SUPPORTED */
}

/* 设置I2C总线 */
void esp_gmf_setup_periph_i2c(int port)
{
    // 配置I2C主机总线参数
    i2c_master_bus_config_t i2c_config = {
        .i2c_port = 0,
        .sda_io_num = ESP_GMF_I2C_SDA_IO_NUM,  // SDA引脚
        .scl_io_num = ESP_GMF_I2C_SCL_IO_NUM,  // SCL引脚
        .clk_source = I2C_CLK_SRC_DEFAULT,      // 时钟源
        .flags.enable_internal_pullup = true,    // 启用内部上拉
        .glitch_ignore_cnt = 7,                 // 毛刺忽略计数
    };
    i2c_new_master_bus(&i2c_config, &i2c_handle);  // 创建I2C主机总线
}

/* 释放I2C总线资源 */
void esp_gmf_teardown_periph_i2c(int port)
{
    if (i2c_handle != NULL) {
        i2c_del_master_bus(i2c_handle);  // 删除I2C主机总线
        i2c_handle = NULL;
    }
}

#ifdef USE_ESP_GMF_ESP_CODEC_DEV_IO
/* 设置音频编解码器 */
esp_gmf_err_t esp_gmf_setup_periph_codec(esp_gmf_setup_periph_aud_info *play_info, esp_gmf_setup_periph_aud_info *rec_info,
                                         void **play_dev, void **record_dev)
{
    if ((play_dev != NULL) && (record_dev != NULL)) {  // 同时需要播放和录音
        if (play_info->port_num == rec_info->port_num) {  // 使用相同的I2S端口
            ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_TX_AND_RX, play_info), { return ESP_GMF_ERR_FAIL;}, "Failed to create I2S tx and rx");
        } else {  // 使用不同的I2S端口
            ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_TX_ONLY, play_info), { return ESP_GMF_ERR_FAIL;}, "Failed to create I2S tx");
            ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_RX_ONLY, rec_info), { return ESP_GMF_ERR_FAIL;}, "Failed to create I2S rx");
        }
        setup_periph_play_codec(play_info, play_dev);    // 设置播放编解码器
        setup_periph_record_codec(rec_info, record_dev);  // 设置录音编解码器
    } else if (play_dev != NULL) {  // 仅需要播放
        ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_TX_ONLY, play_info), { return ESP_GMF_ERR_FAIL;}, "Failed to create I2S tx");
        setup_periph_play_codec(play_info, play_dev);
    } else if (record_dev != NULL) {  // 仅需要录音
        ESP_GMF_RET_ON_NOT_OK(TAG, setup_periph_create_i2s(I2S_CREATE_MODE_RX_ONLY, rec_info), { return ESP_GMF_ERR_FAIL;}, "Failed to create I2S rx");
        setup_periph_record_codec(rec_info, record_dev);
    } else {
        return ESP_GMF_ERR_FAIL;
    }
    return ESP_GMF_ERR_OK;
}

/* 释放音频编解码器资源 */
void esp_gmf_teardown_periph_codec(void *play_dev, void *record_dev)
{
    if (play_dev != NULL) {
        teardown_periph_play_codec(play_dev);    // 释放播放编解码器资源
    }
    if (record_dev != NULL) {
        teardown_periph_record_codec(record_dev);  // 释放录音编解码器资源
    }
}
#endif  /* USE_ESP_GMF_ESP_CODEC_DEV_IO */
