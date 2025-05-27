/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief  向GMF元素池注册编解码器设备IO
 *         Register codec device io to gmf element pool
 * 
 * @param[in]  pool        GMF池句柄
 *                         Handle of gmf pool
 * @param[in]  play_dev    播放编解码器设备句柄
 *                         Handle of play codec device  
 * @param[in]  record_dev  录音编解码器设备句柄
 *                         Handle of record codec device
 */
void pool_register_codec_dev_io(esp_gmf_pool_handle_t pool, void *play_dev, void *record_dev);

/**
 * @brief  向GMF元素池注册IO
 *         Register IO to gmf element pool
 *
 * @param[in]  pool  GMF池句柄
 *                   Handle of gmf pool
 */
void pool_register_io(esp_gmf_pool_handle_t pool);

/**
 * @brief  向GMF元素池注册音频编解码器元素
 *         Register audio codec element to gmf element pool
 *
 * @param[in]  pool  GMF池句柄
 *                   Handle of gmf pool
 */
void pool_register_audio_codecs(esp_gmf_pool_handle_t pool);

/**
 * @brief  注销音频编解码器
 *         Unregister audio codec
 */
void pool_unregister_audio_codecs(void);

/**
 * @brief  向GMF元素池注册音频效果元素
 *         Register audio effect element to gmf element pool
 *
 * @param[in]  pool  GMF池句柄
 *                   Handle of gmf pool
 */
void pool_register_audio_effects(esp_gmf_pool_handle_t pool);

/**
 * @brief  向GMF元素池注册图像元素
 *         Register image element to gmf element pool
 *
 * @param[in]  pool  GMF池句柄
 *                   Handle of gmf pool
 */
void pool_register_image(esp_gmf_pool_handle_t pool);

/**
 * @brief  向GMF元素池注册所有GMF元素
 *         Register all the gmf element to gmf element pool
 *
 * @param[in]  pool        GMF池句柄
 *                         Handle of gmf pool
 * @param[in]  play_dev    播放编解码器设备句柄
 *                         Handle of play codec device
 * @param[in]  codec_dev   编解码器设备句柄
 *                         Handle of codec device
 */
void pool_register_all(esp_gmf_pool_handle_t pool, void *play_dev, void *codec_dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */
