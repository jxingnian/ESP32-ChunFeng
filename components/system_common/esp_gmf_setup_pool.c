/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

/* 系统头文件 */
#include <string.h>
#include "esp_log.h"

/* ESP 系统组件头文件 */
#include "esp_log.h"
#include "esp_err.h"

/* GMF 核心组件头文件 */
#include "esp_gmf_element.h"
#include "esp_gmf_pipeline.h"
#include "esp_gmf_pool.h"
#include "esp_gmf_oal_mem.h"

/* GMF IO 组件头文件 */
#include "esp_gmf_io_file.h"
#include "esp_gmf_io_http.h"
#include "esp_gmf_io_embed_flash.h"
#include "esp_gmf_copier.h"
#include "esp_gmf_setup_pool.h"

/* GMF 音频处理组件头文件 */
#include "esp_gmf_ch_cvt.h"      // 声道转换
#include "esp_gmf_bit_cvt.h"     // 位深转换
#include "esp_gmf_rate_cvt.h"    // 采样率转换
#include "esp_gmf_sonic.h"       // 音频变速变调
#include "esp_gmf_alc.h"         // 自动音量控制
#include "esp_gmf_eq.h"          // 均衡器
#include "esp_gmf_fade.h"        // 淡入淡出
#include "esp_gmf_mixer.h"       // 混音器
#include "esp_gmf_interleave.h"  // 交织器
#include "esp_gmf_deinterleave.h"// 解交织器
#include "esp_gmf_audio_dec.h"   // 音频解码器
#include "esp_audio_simple_dec_default.h" // 默认简单解码器

/* 音频编码相关头文件 */
#include "esp_audio_enc_default.h"
#include "esp_gmf_audio_enc.h"
#include "esp_gmf_io_codec_dev.h"

/* 其他系统组件头文件 */
#include "esp_audio_dec_default.h"
#include "esp_audio_dec_reg.h"
#include "esp_gmf_gpio_config.h"
#include "esp_gmf_audio_helper.h"

/* 日志标签定义 */
static const char *TAG = "ESP_GMF_SETUP_POOL";

/* 音频参数默认配置 */
#define SETUP_AUDIO_SAMPLE_RATE 16000  // 默认采样率16kHz
#define SETUP_AUDIO_BITS        16     // 默认位深16bit
#define SETUP_AUDIO_CHANNELS    1      // 默认单声道

/**
 * @brief 注册编解码设备IO
 * 初始化并注册播放和录音设备到GMF池中
 * @param pool GMF池句柄
 * @param play_dev 播放设备句柄
 * @param record_dev 录音设备句柄
 */
void pool_register_codec_dev_io(esp_gmf_pool_handle_t pool, void *play_dev, void *record_dev)
{
    esp_gmf_io_handle_t dev = NULL;
    
    /* 注册播放设备 */
    if (play_dev != NULL) {
        codec_dev_io_cfg_t tx_codec_dev_cfg = ESP_GMF_IO_CODEC_DEV_CFG_DEFAULT();
        tx_codec_dev_cfg.dir = ESP_GMF_IO_DIR_WRITER;
        tx_codec_dev_cfg.dev = play_dev;
        tx_codec_dev_cfg.name = "codec_dev_tx";
        esp_gmf_io_codec_dev_init(&tx_codec_dev_cfg, &dev);
        esp_gmf_pool_register_io(pool, dev, NULL);
    }
    
    /* 注册录音设备 */
    if (record_dev != NULL) {
        codec_dev_io_cfg_t rx_codec_dev_cfg = ESP_GMF_IO_CODEC_DEV_CFG_DEFAULT();
        rx_codec_dev_cfg.dir = ESP_GMF_IO_DIR_READER;
        rx_codec_dev_cfg.dev = record_dev;
        rx_codec_dev_cfg.name = "codec_dev_rx";
        esp_gmf_io_codec_dev_init(&rx_codec_dev_cfg, &dev);
        esp_gmf_pool_register_io(pool, dev, NULL);
    }
}

/**
 * @brief 注册音频编解码器
 * 初始化并注册音频编码器和解码器到GMF池中
 * @param pool GMF池句柄
 */
void pool_register_audio_codecs(esp_gmf_pool_handle_t pool)
{
    esp_gmf_element_handle_t hd = NULL;

    /* 注册音频编码器 */
    esp_audio_enc_register_default();
    esp_audio_enc_config_t es_enc_cfg = DEFAULT_ESP_GMF_AUDIO_ENC_CONFIG();
    esp_gmf_audio_enc_init(&es_enc_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册音频解码器 */
    esp_audio_dec_register_default();
    esp_audio_simple_dec_register_default();
    esp_audio_simple_dec_cfg_t es_dec_cfg = DEFAULT_ESP_GMF_AUDIO_DEC_CONFIG();
    esp_gmf_audio_dec_init(&es_dec_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);
}

/**
 * @brief 注销音频编解码器
 * 注销所有已注册的音频编解码器
 */
void pool_unregister_audio_codecs()
{
    esp_audio_enc_unregister_default();
    esp_audio_dec_unregister_default();
    esp_audio_simple_dec_unregister_default();
}

/**
 * @brief 注册音频效果处理组件
 * 初始化并注册各种音频效果处理组件到GMF池中
 * @param pool GMF池句柄
 */
void pool_register_audio_effects(esp_gmf_pool_handle_t pool)
{
    esp_gmf_element_handle_t hd = NULL;

    /* 注册自动音量控制 */
    esp_ae_alc_cfg_t alc_cfg = DEFAULT_ESP_GMF_ALC_CONFIG();
    esp_gmf_alc_init(&alc_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册均衡器 */
    esp_ae_eq_cfg_t eq_cfg = DEFAULT_ESP_GMF_EQ_CONFIG();
    esp_gmf_eq_init(&eq_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册声道转换器 */
    esp_ae_ch_cvt_cfg_t ch_cvt_cfg = DEFAULT_ESP_GMF_CH_CVT_CONFIG();
    esp_gmf_ch_cvt_init(&ch_cvt_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册位深转换器 */
    esp_ae_bit_cvt_cfg_t bit_cvt_cfg = DEFAULT_ESP_GMF_BIT_CVT_CONFIG();
    esp_gmf_bit_cvt_init(&bit_cvt_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册采样率转换器 */
    esp_ae_rate_cvt_cfg_t rate_cvt_cfg = DEFAULT_ESP_GMF_RATE_CVT_CONFIG();
    esp_gmf_rate_cvt_init(&rate_cvt_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册淡入淡出效果器 */
    esp_ae_fade_cfg_t fade_cfg = DEFAULT_ESP_GMF_FADE_CONFIG();
    esp_gmf_fade_init(&fade_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册变速变调效果器 */
    esp_ae_sonic_cfg_t sonic_cfg = DEFAULT_ESP_GMF_SONIC_CONFIG();
    esp_gmf_sonic_init(&sonic_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册解交织器 */
    esp_gmf_deinterleave_cfg deinterleave_cfg = DEFAULT_ESP_GMF_DEINTERLEAVE_CONFIG();
    esp_gmf_deinterleave_init(&deinterleave_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册交织器 */
    esp_gmf_interleave_cfg interleave_cfg = DEFAULT_ESP_GMF_INTERLEAVE_CONFIG();
    esp_gmf_interleave_init(&interleave_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);

    /* 注册混音器 */
    esp_ae_mixer_cfg_t mixer_cfg = DEFAULT_ESP_GMF_MIXER_CONFIG();
    esp_gmf_mixer_init(&mixer_cfg, &hd);
    esp_gmf_pool_register_element(pool, hd, NULL);
}

/**
 * @brief 注册所有组件
 * 注册所有音频处理相关的组件到GMF池中
 * @param pool GMF池句柄
 * @param play_dev 播放设备句柄
 * @param codec_dev 编解码设备句柄
 */
void pool_register_all(esp_gmf_pool_handle_t pool, void *play_dev, void *codec_dev)
{
    pool_register_audio_codecs(pool);
    pool_register_audio_effects(pool);
    pool_register_codec_dev_io(pool, play_dev, codec_dev);
}
