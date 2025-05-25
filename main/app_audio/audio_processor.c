/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* 标准库头文件 */
#include <string.h>
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

/* ESP GMF 相关头文件 */
#include "esp_gmf_element.h"
#include "esp_gmf_pool.h"
#include "esp_gmf_io.h"
#include "esp_gmf_pipeline.h"
#include "esp_gmf_pool.h"
#include "esp_audio_simple_player.h"
#include "esp_audio_simple_player_advance.h"
#include "esp_gmf_setup_pool.h"
#include "esp_gmf_setup_peripheral.h"
#include "esp_gmf_gpio_config.h"
#include "esp_codec_dev.h"
#include "esp_gmf_fifo.h"

/* 根据配置选择性包含头文件 */
#ifndef CONFIG_KEY_PRESS_DIALOG_MODE
#include "esp_vad.h"
#include "esp_afe_config.h"
#include "esp_gmf_afe_manager.h"
#include "esp_gmf_afe.h"
#endif  /* CONFIG_KEY_PRESS_DIALOG_MODE */
#include "audio_processor.h"

/* 常量定义 */
#define VAD_ENABLE       (true)      // 是否启用语音活动检测
#define VCMD_ENABLE      (false)     // 是否启用语音命令
#define DEFAULT_FIFO_NUM (5)         // 默认FIFO数量

/* 根据不同配置定义音频参数 */
#if CONFIG_KEY_PRESS_DIALOG_MODE
#define CAHNNELS    1                // 按键模式下的通道数
#define SAMPLE_BITS 16               // 按键模式下的采样位数
#else
#if CODEC_ES7210_IN_ES8311_OUT
#define INPUT_CH_ALLOCATION ("RMNM") // ES7210输入通道分配
#define CAHNNELS            2        // ES7210通道数
#define SAMPLE_BITS         32       // ES7210采样位数
#elif CODEC_ES8311_IN_OUT
#define INPUT_CH_ALLOCATION ("MR")   // ES8311输入通道分配
#define CAHNNELS            2        // ES8311通道数
#define SAMPLE_BITS         16       // ES8311采样位数
#endif  // CODEC_ES7210_IN_ES8311_OUT
#endif  // CONFIG_KEY_PRESS_DIALOG_MODE

#define DEFAULT_PLAYBACK_VOLUME (70) // 默认播放音量

static char *TAG = "AUDIO_PROCESSOR";

/* 音频播放器状态枚举 */
enum audio_player_state_e {
    AUDIO_PLAYER_STATE_IDLE,    // 空闲状态
    AUDIO_PLAYER_STATE_PLAYING, // 播放状态
    AUDIO_PLAYER_STATE_CLOSED,  // 关闭状态
};

/* 提示音播放器结构体 */
typedef struct {
    esp_asp_handle_t          player; // 播放器句柄
    enum audio_player_state_e state;  // 播放器状态
} audio_prompt_t;

/* 录音器结构体 */
typedef struct {
    esp_gmf_fifo_handle_t         fifo;  // FIFO句柄
    recorder_event_callback_t     cb;     // 事件回调函数
    void                         *ctx;    // 回调上下文
    enum audio_player_state_e     state;  // 录音器状态
#ifndef CONFIG_KEY_PRESS_DIALOG_MODE
    esp_gmf_pipeline_handle_t     pipe;        // 管道句柄
    esp_gmf_afe_manager_handle_t  afe_manager; // AFE管理器句柄
    afe_config_t                 *afe_cfg;     // AFE配置
    esp_gmf_task_handle_t         task;        // 任务句柄
#endif  /* CONFIG_KEY_PRESS_DIALOG_MODE */
} audio_recordert_t;

/* 音频回放结构体 */
typedef struct {
    esp_asp_handle_t          player; // 播放器句柄
    esp_gmf_fifo_handle_t     fifo;   // FIFO句柄
    enum audio_player_state_e state;  // 播放器状态
} audio_playback_t;

/* 音频管理器结构体 */
typedef struct {
    esp_codec_dev_handle_t play_dev; // 播放设备句柄
    esp_codec_dev_handle_t rec_dev;  // 录音设备句柄
    esp_gmf_pool_handle_t  pool;     // 资源池句柄
} audio_manager_t;

/* 全局变量定义 */
static audio_manager_t   audio_manager;   // 音频管理器实例
static audio_recordert_t audio_recorder;  // 录音器实例
static audio_playback_t  audio_playback;  // 音频回放实例
static audio_prompt_t    audio_prompt;    // 提示音播放器实例

/**
 * @brief 初始化音频管理器
 * 
 * 该函数完成以下初始化工作:
 * 1. 初始化I2C外设
 * 2. 配置音频播放和录音参数(采样率、通道数、采样位数等)
 * 3. 初始化音频编解码器
 * 4. 初始化资源池并注册各种音频组件
 * 5. 设置默认播放音量
 *
 * @return ESP_OK: 初始化成功
 *         ESP_FAIL: 初始化失败
 */
esp_err_t audio_manager_init(void)
{
    // 初始化I2C外设,端口号为0
    esp_gmf_setup_periph_i2c(0);

    // 配置音频播放参数
    esp_gmf_setup_periph_aud_info play_info = {
        .sample_rate = 16000,        // 采样率16kHz
        .channel = CAHNNELS,         // 通道数
        .bits_per_sample = SAMPLE_BITS, // 采样位数
        .port_num  = 0,              // 端口号
    };

    // 配置音频录音参数
    esp_gmf_setup_periph_aud_info record_info = {
        .sample_rate = 16000,        // 采样率16kHz
        .channel = CAHNNELS,         // 通道数
        .bits_per_sample = SAMPLE_BITS, // 采样位数
        .port_num = 0,               // 端口号
    };

    // 初始化音频编解码器
    esp_gmf_setup_periph_codec(&play_info, &record_info, &audio_manager.play_dev, &audio_manager.rec_dev);

    // 初始化资源池
    esp_gmf_pool_init(&audio_manager.pool);

    // 注册各种音频组件到资源池
    pool_register_io(audio_manager.pool);                    // 注册IO组件
    pool_register_audio_codecs(audio_manager.pool);         // 注册音频编解码器
    pool_register_audio_effects(audio_manager.pool);        // 注册音频效果器
    pool_register_codec_dev_io(audio_manager.pool, audio_manager.play_dev, audio_manager.rec_dev); // 注册编解码器设备IO

    // 设置默认播放音量
    esp_codec_dev_set_out_vol(audio_manager.play_dev, DEFAULT_PLAYBACK_VOLUME);

    return ESP_OK;
}

/**
 * @brief 反初始化音频管理器
 * 
 * 该函数完成以下清理工作:
 * 1. 注销音频编解码器
 * 2. 销毁资源池
 * 3. 关闭音频编解码器
 * 4. 关闭I2C外设
 *
 * @return ESP_OK: 反初始化成功
 *         ESP_FAIL: 反初始化失败
 */
esp_err_t audio_manager_deinit()
{
    pool_unregister_audio_codecs();  // 注销音频编解码器
    esp_gmf_pool_deinit(audio_manager.pool); // 销毁资源池
    esp_gmf_teardown_periph_codec(audio_manager.play_dev, audio_manager.rec_dev); // 关闭音频编解码器
    esp_gmf_teardown_periph_i2c(0);  // 关闭I2C外设
    return ESP_OK;
}

/**
 * @brief 提示音播放数据回调函数
 * 
 * @param data 音频数据缓冲区
 * @param data_size 数据大小
 * @param ctx 用户上下文
 * @return 0: 成功
 */
static int prompt_out_data_callback(uint8_t *data, int data_size, void *ctx)
{
    esp_codec_dev_write(audio_manager.play_dev, data, data_size);
    return 0;
}

/**
 * @brief 提示音播放事件回调函数
 * 
 * 处理以下事件:
 * 1. 音频信息事件: 打印采样率、通道数、位数等信息
 * 2. 状态事件: 处理播放器状态变化
 *
 * @param event 事件包
 * @param ctx 用户上下文
 * @return 0: 成功
 */
static int prompt_event_callback(esp_asp_event_pkt_t *event, void *ctx)
{
    if (event->type == ESP_ASP_EVENT_TYPE_MUSIC_INFO) {
        // 处理音频信息事件
        esp_asp_music_info_t info = {0};
        memcpy(&info, event->payload, event->payload_size);
        ESP_LOGI(TAG, "Get info, rate:%d, channels:%d, bits:%d", info.sample_rate, info.channels, info.bits);
    } else if (event->type == ESP_ASP_EVENT_TYPE_STATE) {
        // 处理状态事件
        esp_asp_state_t st = 0;
        memcpy(&st, event->payload, event->payload_size);
        ESP_LOGI(TAG, "Get State, %d,%s", st, esp_audio_simple_player_state_to_str(st));
        if (((st == ESP_ASP_STATE_STOPPED) || (st == ESP_ASP_STATE_FINISHED) || (st == ESP_ASP_STATE_ERROR))) {
            audio_prompt.state = AUDIO_PLAYER_STATE_IDLE;
        }
    }
    return 0;
}

#ifndef CONFIG_KEY_PRESS_DIALOG_MODE
/**
 * @brief 录音管道事件回调函数
 * 
 * @param event 事件包
 * @param ctx 用户上下文
 * @return esp_err_t 
 */
static esp_err_t recorder_pipeline_event(esp_gmf_event_pkt_t *event, void *ctx)
{
    ESP_LOGD(TAG, "CB: RECV Pipeline EVT: el:%s-%p, type:%d, sub:%s, payload:%p, size:%d,%p",
             OBJ_GET_TAG(event->from), event->from, event->type, esp_gmf_event_get_state_str(event->sub),
             event->payload, event->payload_size, ctx);
    return 0;
}

/**
 * @brief 录音输出端口写入回调函数
 * 
 * @param handle 句柄
 * @param load 数据负载
 * @param wanted_size 期望写入大小
 * @param block_ticks 阻塞时间
 * @return int 实际写入大小
 */
static int recorder_outport_acquire_write(void *handle, esp_gmf_payload_t *load, int wanted_size, int block_ticks)
{
    esp_gmf_data_bus_block_t blk;
    int ret = esp_gmf_fifo_acquire_write(audio_recorder.fifo, &blk, wanted_size, block_ticks);
    if (ret < 0) {
        ESP_LOGE(TAG, "Fifo acquire write failed");
        return ESP_FAIL;
    }
    memcpy(blk.buf, load->buf, wanted_size);
    blk.valid_size = wanted_size;
    ret = esp_gmf_fifo_release_write(audio_recorder.fifo, &blk, block_ticks);
    return wanted_size;
}

/**
 * @brief 录音输出端口写入释放回调函数
 * 
 * @param handle 句柄
 * @param load 数据负载
 * @param block_ticks 阻塞时间
 * @return int 有效数据大小
 */
static int recorder_outport_release_write(void *handle, esp_gmf_payload_t *load, int block_ticks)
{
    return load->valid_size;
}

/**
 * @brief 录音输入端口读取回调函数
 * 
 * @param handle 句柄
 * @param load 数据负载
 * @param wanted_size 期望读取大小
 * @param block_ticks 阻塞时间
 * @return int 实际读取大小
 */
static int recorder_inport_acquire_read(void *handle, esp_gmf_payload_t *load, int wanted_size, int block_ticks)
{
    esp_codec_dev_read(audio_manager.rec_dev, load->buf, wanted_size);
    load->valid_size = wanted_size;
    return wanted_size;
}

/**
 * @brief 录音输入端口读取释放回调函数
 * 
 * @param handle 句柄
 * @param load 数据负载
 * @param block_ticks 阻塞时间
 * @return int 有效数据大小
 */
static int recorder_inport_release_read(void *handle, esp_gmf_payload_t *load, int block_ticks)
{
    return load->valid_size;
}

/**
 * @brief AFE事件回调函数
 * 
 * 处理以下事件:
 * - 唤醒开始: 当检测到唤醒词时触发
 * - 唤醒结束: 当唤醒状态结束时触发
 * - VAD开始: 当检测到语音活动开始时触发
 * - VAD结束: 当检测到语音活动结束时触发
 * - 语音命令检测超时: 当语音命令检测超过设定时间时触发
 * - 其他语音命令相关事件: 如命令识别结果等
 * 
 * @param obj AFE对象句柄,用于控制音频前端处理
 * @param event AFE事件结构体,包含事件类型和相关数据
 * @param user_data 用户自定义数据指针
 */
static void esp_gmf_afe_event_cb(esp_gmf_obj_handle_t obj, esp_gmf_afe_evt_t *event, void *user_data)
{
    // 调用用户注册的回调函数,传递事件和上下文
    audio_recorder.cb((void *)event, audio_recorder.ctx);
    
    switch (event->type) {
        case ESP_GMF_AFE_EVT_WAKEUP_START: {
            // 唤醒开始事件处理
#if CONFIG_LANGUAGE_WAKEUP_MODE
            // 在语言唤醒模式下,重新开始语音命令检测
            esp_gmf_afe_vcmd_detection_cancel(obj);
            esp_gmf_afe_vcmd_detection_begin(obj);
#endif  /* CONFIG_LANGUAGE_WAKEUP_MODE */
            esp_gmf_afe_wakeup_info_t *info = event->event_data;
            ESP_LOGI(TAG, "WAKEUP_START [%d : %d]", info->wake_word_index, info->wakenet_model_index);
            break;
        }
        case ESP_GMF_AFE_EVT_WAKEUP_END: {
            // 唤醒结束事件处理
#if CONFIG_LANGUAGE_WAKEUP_MODE
            esp_gmf_afe_vcmd_detection_cancel(obj);
#endif  /* CONFIG_LANGUAGE_WAKEUP_MODE */
            ESP_LOGI(TAG, "WAKEUP_END");
            break;
        }
        case ESP_GMF_AFE_EVT_VAD_START: {
            // VAD开始事件处理
#ifndef CONFIG_LANGUAGE_WAKEUP_MODE
            // 在非语言唤醒模式下,重新开始语音命令检测
            esp_gmf_afe_vcmd_detection_cancel(obj);
            esp_gmf_afe_vcmd_detection_begin(obj);
#endif  /* CONFIG_LANGUAGE_WAKEUP_MODE */
            ESP_LOGI(TAG, "VAD_START");
            break;
        }
        case ESP_GMF_AFE_EVT_VAD_END: {
            // VAD结束事件处理
#ifndef CONFIG_LANGUAGE_WAKEUP_MODE
            esp_gmf_afe_vcmd_detection_cancel(obj);
#endif  /* CONFIG_LANGUAGE_WAKEUP_MODE */
            ESP_LOGI(TAG, "VAD_END");
            break;
        }
        case ESP_GMF_AFE_EVT_VCMD_DECT_TIMEOUT: {
            // 语音命令检测超时事件处理
            ESP_LOGI(TAG, "VCMD_DECT_TIMEOUT");
            break;
        }
        default: {
            // 其他语音命令相关事件处理
            esp_gmf_afe_vcmd_info_t *info = event->event_data;
            ESP_LOGW(TAG, "Command %d, phrase_id %d, prob %f, str: %s",
                     event->type, info->phrase_id, info->prob, info->str);
            break;
        }
    }
}
#endif  /* CONFIG_KEY_PRESS_DIALOG_MODE */

/**
 * @brief 打开音频录制器
 * 
 * 初始化音频录制相关组件:
 * - 创建FIFO缓冲区
 * - 初始化语音识别模型
 * - 配置AFE参数(VAD/唤醒/AEC等)
 * - 创建并配置AFE管理器
 * - 创建音频处理管道
 * - 注册输入输出端口
 * - 创建并绑定处理任务
 * 
 * @param cb 录音事件回调函数
 * @param ctx 回调函数上下文
 * @return esp_err_t 
 *         - ESP_OK: 成功
 *         - ESP_FAIL: 失败
 */
esp_err_t audio_recorder_open(recorder_event_callback_t cb, void *ctx)
{
    // 创建FIFO缓冲区
    esp_err_t err = esp_gmf_fifo_create(DEFAULT_FIFO_NUM, 1, &audio_recorder.fifo);
    if (err != ESP_GMF_ERR_OK) {
        ESP_LOGE(TAG, "esp_gmf_fifo_create failed, err: %d", err);
        return ESP_FAIL;
    }
#if CONFIG_KEY_PRESS_DIALOG_MODE
    // 按键触发对话模式下不需要初始化语音处理组件
    (void)cb;
    (void)ctx;
    return ESP_OK;
#else
    // 初始化语音识别模型
    srmodel_list_t *models = esp_srmodel_init("model");
    const char *ch_format = INPUT_CH_ALLOCATION;
    
    // 配置AFE参数
    audio_recorder.afe_cfg = afe_config_init(ch_format, models, AFE_TYPE_SR, AFE_MODE_HIGH_PERF);
    audio_recorder.afe_cfg->vad_init = VAD_ENABLE;
    audio_recorder.afe_cfg->vad_mode = VAD_MODE_3;
    audio_recorder.afe_cfg->vad_min_speech_ms = 64;
    audio_recorder.afe_cfg->vad_min_noise_ms = 1000;
#if CONFIG_VOICE_WAKEUP_MODE
    audio_recorder.afe_cfg->wakenet_init = true;
#else
    audio_recorder.afe_cfg->wakenet_init = false;
#endif  // CONFIG_VOICE_WAKEUP_MODE
    audio_recorder.afe_cfg->aec_init = true;

    // 创建AFE管理器
    esp_gmf_afe_manager_cfg_t afe_manager_cfg = DEFAULT_GMF_AFE_MANAGER_CFG(audio_recorder.afe_cfg, NULL, NULL, NULL, NULL);
    esp_gmf_afe_manager_create(&afe_manager_cfg, &audio_recorder.afe_manager);

    // 初始化AFE组件
    esp_gmf_element_handle_t gmf_afe = NULL;
    esp_gmf_afe_cfg_t gmf_afe_cfg = DEFAULT_GMF_AFE_CFG(audio_recorder.afe_manager, esp_gmf_afe_event_cb, NULL, models);
    gmf_afe_cfg.vcmd_detect_en = VCMD_ENABLE;
    gmf_afe_cfg.wakeup_end = 60000;
    esp_gmf_afe_init(&gmf_afe_cfg, &gmf_afe);

    // 注册AFE组件到处理池
    esp_gmf_pool_register_element(audio_manager.pool, gmf_afe, NULL);

    // 创建处理管道
    const char *name[] = {"gmf_afe"};
    esp_gmf_pool_new_pipeline(audio_manager.pool, NULL, name, sizeof(name) / sizeof(char *), NULL, &audio_recorder.pipe);
    if (audio_recorder.pipe == NULL) {
        ESP_LOGE(TAG, "There is no pipeline");
        goto __quit;
    }

    // 注册输出端口
    esp_gmf_port_handle_t outport = NEW_ESP_GMF_PORT_OUT_BYTE(recorder_outport_acquire_write,
                                                              recorder_outport_release_write,
                                                              NULL,
                                                              NULL,
                                                              2048,
                                                              100);
    esp_gmf_pipeline_reg_el_port(audio_recorder.pipe, "gmf_afe", ESP_GMF_IO_DIR_WRITER, outport);

    // 注册输入端口
    esp_gmf_port_handle_t inport = NEW_ESP_GMF_PORT_IN_BYTE(recorder_inport_acquire_read,
                                                            recorder_inport_release_read,
                                                            NULL,
                                                            NULL,
                                                            2048,
                                                            100);
    esp_gmf_pipeline_reg_el_port(audio_recorder.pipe, "gmf_afe", ESP_GMF_IO_DIR_READER, inport);

    // 创建并配置处理任务
    esp_gmf_task_cfg_t cfg = DEFAULT_ESP_GMF_TASK_CONFIG();
    cfg.ctx = NULL;
    cfg.cb = NULL;
    cfg.thread.core = 0;
    cfg.thread.prio = 5;
    cfg.thread.stack = 5120;
    esp_gmf_task_init(&cfg, &audio_recorder.task);

    // 绑定任务到管道并启动
    esp_gmf_pipeline_bind_task(audio_recorder.pipe, audio_recorder.task);
    esp_gmf_pipeline_loading_jobs(audio_recorder.pipe);
    esp_gmf_pipeline_set_event(audio_recorder.pipe, recorder_pipeline_event, NULL);
    esp_gmf_pipeline_run(audio_recorder.pipe);

    // 保存回调函数和上下文
    audio_recorder.cb = cb;
    audio_recorder.ctx = ctx;
    return ESP_OK;

__quit:
    // 错误处理:清理已创建的资源
    esp_gmf_pipeline_stop(audio_recorder.pipe);
    esp_gmf_task_deinit(audio_recorder.task);
    afe_config_free(audio_recorder.afe_cfg);
    esp_gmf_afe_manager_destroy(audio_recorder.afe_manager);
    return ESP_FAIL;
#endif  /* CONFIG_KEY_PRESS_DIALOG_MODE */
}

/**
 * @brief 关闭音频录制器
 * 
 * @return esp_err_t ESP_OK:成功 ESP_FAIL:失败
 */
esp_err_t audio_recorder_close(void)
{
    // 检查录音器是否已经关闭
    if (audio_recorder.state == AUDIO_PLAYER_STATE_CLOSED) {
        ESP_LOGW(TAG, "Audio recored is relady cloesed");
        return ESP_OK;
    }
#ifndef CONFIG_KEY_PRESS_DIALOG_MODE
    // 销毁管道
    esp_gmf_pipeline_destroy(audio_recorder.pipe);
    // 反初始化任务
    esp_gmf_task_deinit(audio_recorder.task);
    // 释放AFE配置
    afe_config_free(audio_recorder.afe_cfg);
    // 销毁AFE管理器
    esp_gmf_afe_manager_destroy(audio_recorder.afe_manager);
#endif  /* CONFIG_KEY_PRESS_DIALOG_MODE */
    // 设置录音器状态为关闭
    audio_recorder.state = AUDIO_PLAYER_STATE_CLOSED;
    return ESP_OK;
}

/**
 * @brief 从录音器读取音频数据
 * 
 * @param data 数据缓冲区指针
 * @param data_size 要读取的数据大小
 * @return esp_err_t 实际读取的数据大小或错误码
 */
esp_err_t audio_recorder_read_data(uint8_t *data, int data_size)
{
    // TODO: will support opus data
#if CONFIG_KEY_PRESS_DIALOG_MODE
    // 按键模式下直接从录音设备读取数据
    esp_codec_dev_read(audio_manager.rec_dev, data, data_size);
    return data_size;
#else
    esp_gmf_data_bus_block_t blk;
    // 从FIFO获取读取块
    int ret = esp_gmf_fifo_acquire_read(audio_recorder.fifo, &blk, data_size, portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Fifo acquire read failed (0x%x)", ret);
        return ESP_FAIL;
    }
    // 复制数据到目标缓冲区
    memcpy(data, blk.buf, blk.valid_size);
    // 释放读取块
    esp_gmf_fifo_release_read(audio_recorder.fifo, &blk, portMAX_DELAY);
    return blk.valid_size;
#endif  /* CONFIG_KEY_PRESS_DIALOG_MODE */
}

/**
 * @brief 向音频播放器提供数据
 * 
 * @param data 数据缓冲区指针
 * @param data_size 数据大小
 * @return esp_err_t ESP_OK:成功 ESP_FAIL:失败
 */
esp_err_t audio_playback_feed_data(uint8_t *data, int data_size)
{
    esp_gmf_data_bus_block_t blk = {0};
    // 从FIFO获取写入块
    int ret = esp_gmf_fifo_acquire_write(audio_playback.fifo, &blk, data_size, portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Fifo acquire write failed (0x%x)", ret);
        return ESP_FAIL;
    }
    // 复制数据到FIFO块
    memcpy(blk.buf, data, data_size);
    blk.valid_size = data_size;
    // 释放写入块
    esp_gmf_fifo_release_write(audio_playback.fifo, &blk, portMAX_DELAY);
    return ESP_OK;
}

/**
 * @brief 播放器读取回调函数
 * 
 * @param data 数据缓冲区指针
 * @param data_size 要读取的数据大小
 * @param ctx 上下文指针
 * @return int 实际读取的数据大小
 */
static int playback_read_callback(uint8_t *data, int data_size, void *ctx)
{
    esp_gmf_data_bus_block_t blk = {0};
    // 从FIFO获取读取块
    int ret = esp_gmf_fifo_acquire_read(audio_playback.fifo, &blk, data_size,
                                        portMAX_DELAY);
    if (ret < 0) {
        ESP_LOGE(TAG, "Fifo acquire read failed (0x%x)", ret);
    }
    // 复制数据到目标缓冲区
    memcpy(data, blk.buf, blk.valid_size);
    // 释放读取块
    esp_gmf_fifo_release_read(audio_playback.fifo, &blk, 0);
    return blk.valid_size;
}

/**
 * @brief 音频播放器写入回调函数
 * 
 * @param data 要写入的数据缓冲区指针
 * @param data_size 要写入的数据大小
 * @param ctx 上下文指针(编解码器设备句柄)
 * @return int 实际写入的数据大小,失败返回-1
 */
static int playback_write_callback(uint8_t *data, int data_size, void *ctx)
{
    // 如果提示音正在播放,跳过写入
    if (audio_prompt.state == AUDIO_PLAYER_STATE_PLAYING) {
        ESP_LOGW(TAG, "Audio prompt is playing, skip\n");
        return data_size;
    }
    // 获取编解码器设备句柄
    esp_codec_dev_handle_t dev = (esp_codec_dev_handle_t)ctx;
    // 写入数据到编解码器设备
    int ret =  esp_codec_dev_write(dev, data, data_size);
    if (ret != ESP_CODEC_DEV_OK) {
        ESP_LOGE(TAG, "Write to codec dev failed (0x%x)\n", ret);
        return -1;
    }
    return data_size;
}

/**
 * @brief 音频播放器事件回调函数
 * 
 * @param event 事件包指针
 * @param ctx 上下文指针
 * @return int 0:成功
 */
static int playback_event_callback(esp_asp_event_pkt_t *event, void *ctx)
{
    // 处理音乐信息事件
    if (event->type == ESP_ASP_EVENT_TYPE_MUSIC_INFO) {
        esp_asp_music_info_t info;
        memcpy(&info, event->payload, event->payload_size);
        ESP_LOGI(TAG, "Get info, rate:%d, channels:%d, bits:%d", info.sample_rate,
                 info.channels, info.bits);
    } 
    // 处理播放器状态事件
    else if (event->type == ESP_ASP_EVENT_TYPE_STATE) {
        esp_asp_state_t st = ESP_ASP_STATE_NONE;
        memcpy(&st, event->payload, event->payload_size);
        ESP_LOGI(TAG, "Get State, %d,%s", st,
                 esp_audio_simple_player_state_to_str(st));
        // 如果播放器停止、完成或出错,将提示音状态设为空闲
        if (ctx && ((st == ESP_ASP_STATE_STOPPED) || (st == ESP_ASP_STATE_FINISHED) || (st == ESP_ASP_STATE_ERROR))) {
            audio_prompt.state = AUDIO_PLAYER_STATE_IDLE;
        }
    }
    return 0;
}

/**
 * @brief 打开音频播放器
 * 
 * @return esp_err_t ESP_OK:成功 其他:失败
 */
esp_err_t audio_playback_open(void)
{
    esp_err_t err = ESP_GMF_ERR_OK;

    do {
        // 创建FIFO缓冲区
        err = esp_gmf_fifo_create(DEFAULT_FIFO_NUM, 1, &audio_playback.fifo);
        if (err != ESP_GMF_ERR_OK) {
            ESP_LOGE(TAG, "oai_plr_dec_fifo init failed (0x%x)", err);
            break;
        }
        // 配置播放器参数
        esp_asp_cfg_t player_cfg;
        player_cfg.in.cb = playback_read_callback;
        player_cfg.in.user_ctx = audio_playback.fifo;
        player_cfg.out.cb = playback_write_callback;
        player_cfg.out.user_ctx = audio_manager.play_dev;
        player_cfg.task_prio = 5;
        player_cfg.task_stack = 12 * 1024;
        player_cfg.task_core = 1;

        // 创建音频播放器
        err = esp_audio_simple_player_new(&player_cfg, &audio_playback.player);
        if (err != ESP_GMF_ERR_OK) {
            ESP_LOGE(TAG, "simple_player init failed (0x%x)", err);
            break;
        }
        // 设置事件回调
        err = esp_audio_simple_player_set_event(audio_playback.player,
                                                playback_event_callback, NULL);
        if (err != ESP_GMF_ERR_OK) {
            ESP_LOGE(TAG, "set_event failed (0x%x)", err);
            break;
        }
        audio_playback.state = AUDIO_PLAYER_STATE_IDLE;
        return ESP_OK;

    } while (0);

    // 错误处理:释放资源
    if (audio_playback.player) {
        esp_audio_simple_player_destroy(audio_playback.player);
    }
    if (audio_playback.fifo) {
        esp_gmf_fifo_destroy(audio_playback.fifo);
    }
    return err;
}

/**
 * @brief 关闭音频播放器
 * 
 * @return esp_err_t ESP_OK:成功 ESP_FAIL:失败
 */
esp_err_t audio_playback_close(void)
{
    // 检查播放器是否已关闭
    if (audio_playback.state == AUDIO_PLAYER_STATE_CLOSED) {
        ESP_LOGW(TAG, "Aduio playback is realdy closed");
        return ESP_OK;
    }
    // 如果正在播放,先停止播放
    if (audio_playback.state == AUDIO_PLAYER_STATE_PLAYING) {
        audio_playback_stop();
    }
    // 销毁播放器
    esp_err_t err = esp_audio_simple_player_destroy(audio_playback.player);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Audio playback closing failed");
        return ESP_FAIL;
    }
    audio_playback.state = AUDIO_PLAYER_STATE_CLOSED;
    return ESP_OK;
}

/**
 * @brief 运行音频播放器
 * 
 * @return esp_err_t ESP_OK:成功 ESP_FAIL:失败
 */
esp_err_t audio_playback_run(void)
{
    // 检查播放器是否已在运行
    if (audio_playback.state == AUDIO_PLAYER_STATE_PLAYING) {
        ESP_LOGW(TAG, "Audio playback is realdy running");
        return ESP_OK;
    }
    // 设置音频格式参数
    esp_asp_music_info_t music_info;
    music_info.sample_rate = 16000;
    music_info.channels = 1;
    music_info.bits = 16;
    music_info.bitrate = 0;

    // 启动播放器
    esp_err_t err = esp_audio_simple_player_run(audio_playback.player, "raw://sdcard/coze.opus",
                                                &music_info);
    if (err != ESP_GMF_ERR_OK) {
        ESP_LOGE(TAG, "run failed (0x%x)", err);
        return ESP_FAIL;
    }
    audio_playback.state = AUDIO_PLAYER_STATE_PLAYING;
    return ESP_OK;
}

/**
 * @brief 停止音频播放器
 * 
 * @return esp_err_t ESP_OK:成功 ESP_FAIL:失败
 */
esp_err_t audio_playback_stop(void)
{
    // 检查播放器是否已停止
    if (audio_playback.state == AUDIO_PLAYER_STATE_IDLE) {
        ESP_LOGW(TAG, "Audio playback is already stopped");
        return ESP_OK;
    }
    // 停止播放器
    esp_err_t ret = esp_audio_simple_player_stop(audio_playback.player);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Audio playback stop failed");
        return ESP_FAIL;
    }
    audio_playback.state = AUDIO_PLAYER_STATE_IDLE;
    return ESP_OK;
}

/**
 * @brief 打开提示音播放器
 * 
 * @return esp_err_t ESP_OK:成功 其他:失败
 */
esp_err_t audio_prompt_open(void)
{
    // 配置提示音播放器参数
    esp_asp_cfg_t cfg = {
        .in.cb = NULL,
        .in.user_ctx = NULL,
        .out.cb = prompt_out_data_callback,
        .out.user_ctx = NULL,
        .task_prio = 5,
    };
    // 创建提示音播放器
    esp_gmf_err_t err = esp_audio_simple_player_new(&cfg, &audio_prompt.player);
    // 设置事件回调
    err = esp_audio_simple_player_set_event(audio_prompt.player, prompt_event_callback, NULL);
    audio_prompt.state = AUDIO_PLAYER_STATE_IDLE;
    return err;
}

/**
 * @brief 关闭提示音播放器
 * 
 * @return esp_err_t ESP_OK:成功 ESP_FAIL:失败
 */
esp_err_t audio_prompt_close(void)
{
    // 如果正在播放,先停止播放
    if (audio_prompt.state == AUDIO_PLAYER_STATE_PLAYING) {
        esp_audio_simple_player_stop(audio_prompt.player);
    }
    // 销毁提示音播放器
    esp_err_t err = esp_audio_simple_player_destroy(audio_prompt.player);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Audio prompt closing failed");
        return ESP_FAIL;
    }
    audio_prompt.state = AUDIO_PLAYER_STATE_CLOSED;
    return ESP_OK;
}

/**
 * @brief 播放提示音
 * 
 * @param url 提示音文件URL
 * @return esp_err_t ESP_OK:成功 其他:失败
 */
esp_err_t audio_prompt_play(const char *url)
{
    // 检查提示音是否正在播放
    if (audio_prompt.state == AUDIO_PLAYER_STATE_PLAYING) {
        ESP_LOGE(TAG, "audio_prompt is alread playing");
        return ESP_OK;
    }
    // 启动提示音播放
    esp_audio_simple_player_run(audio_prompt.player, url, NULL);
    audio_prompt.state = AUDIO_PLAYER_STATE_PLAYING;
    return ESP_OK;
}

/**
 * @brief 停止提示音播放
 * 
 * @return esp_err_t ESP_OK:成功 ESP_FAIL:失败
 */
esp_err_t audio_prompt_stop(void)
{
    // 检查提示音是否已停止
    if (audio_prompt.state == AUDIO_PLAYER_STATE_IDLE) {
        ESP_LOGW(TAG, "audio_prompt_stop, but state is idle");
        return ESP_FAIL;
    }
    // 停止提示音播放
    esp_audio_simple_player_stop(audio_prompt.player);
    audio_prompt.state = AUDIO_PLAYER_STATE_IDLE;
    return ESP_OK;
}
