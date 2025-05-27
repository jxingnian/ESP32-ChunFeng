/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file coze_chat_app.c
 * @brief COZE聊天应用程序实现
 * 
 * 该文件实现了基于ESP32的语音对话功能,支持按键触发和语音唤醒两种对话模式
 */
#include "coze_chat_app.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "opus.h"
#include "esp_coze_chat.h"
#include "audio_board.h"

/* 按键录音状态位定义 */
#define BUTTON_REC_READING (1 << 0)
#define SAMPLE_RATE         16000  // 采样率16kHz
#define CHANNELS           1       // 单声道
#define MAX_FRAME_SIZE     960    // 最大帧长(60ms @ 16kHz)
#define MAX_PACKET_SIZE    1500   // Opus包最大大小
#define FRAMES_PER_SECOND  18     // 每秒最大帧数

static char *TAG = "COZE_CHAT_APP";
// Opus解码器实例
static OpusDecoder *opus_decoder = NULL;
static QueueHandle_t audio_queue = NULL;
static uint8_t audio_read_flag = 1;
// 定义音频帧结构
struct audio_frame {
    uint8_t data[MAX_PACKET_SIZE];
    size_t len;
};

// 初始化Opus解码器
static esp_err_t init_opus_decoder(void)
{
    int err;
    
    // 创建音频队列
    audio_queue = xQueueCreate(FRAMES_PER_SECOND, sizeof(struct audio_frame));
    if (audio_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create audio queue");
        return ESP_FAIL;
    }
    
    // 创建Opus解码器，16kHz采样率，单声道
    opus_decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    if (err != OPUS_OK) {
        ESP_LOGE(TAG, "Failed to create Opus decoder: %d", err);
        return ESP_FAIL;
    }
    
    // 设置Opus解码器参数
    opus_decoder_ctl(opus_decoder, OPUS_SET_GAIN(0));          // 设置增益为0dB
    opus_decoder_ctl(opus_decoder, OPUS_SET_LSB_DEPTH(16));    // 设置位深度为16位
    
    return ESP_OK;
}

// 1. 进一步增加任务栈大小
#define OPUS_DECODE_TASK_STACK_SIZE (16*1024)  // 增加到16KB

// 2. 使用静态方式创建任务以确保内存分配
static StaticTask_t opus_decode_task_buffer;
static StackType_t opus_decode_task_stack[OPUS_DECODE_TASK_STACK_SIZE];

// 3. 修改opus解码任务实现，确保所有缓冲区都是静态分配
static void opus_decode_task(void *arg)
{
    // 使用静态分配的缓冲区
    static struct {
        int16_t pcm_out[MAX_FRAME_SIZE * 2];  // 增加缓冲区大小
        uint8_t opus_data[MAX_PACKET_SIZE];
        size_t data_len;
    } audio_buf = {0};
    
    ESP_LOGI(TAG, "Opus decode task started, stack: %d", uxTaskGetStackHighWaterMark(NULL));

    while (1) {
        struct audio_frame frame;
        if (xQueueReceive(audio_queue, &frame, portMAX_DELAY) == pdTRUE) {
            // ESP_LOGI(TAG, "Received frame len: %d, stack: %d", 
                    //  frame.len, 
                    //  uxTaskGetStackHighWaterMark(NULL));

            // 安全检查
            if (frame.len > MAX_PACKET_SIZE) {
                ESP_LOGE(TAG, "Frame too large: %d", frame.len);
                continue;
            }

            // 复制数据到静态缓冲区
            memcpy(audio_buf.opus_data, frame.data, frame.len);
            audio_buf.data_len = frame.len;

            // 解码Opus数据
            int frame_size = opus_decode(opus_decoder, 
                                       audio_buf.opus_data, 
                                       audio_buf.data_len, 
                                       audio_buf.pcm_out, 
                                       MAX_FRAME_SIZE * 2,  // 增加解码缓冲区大小
                                       0);
            
            if (frame_size > 0) {
                size_t bytes_written = 0;
                esp_err_t ret = audio_data_play(audio_buf.pcm_out, 
                                              frame_size * sizeof(int16_t), 
                                              &bytes_written);
                
                // ESP_LOGI(TAG, "Played frame, size: %d, written: %d, ret: %d, stack: %d",
                //          frame_size,
                //          bytes_written,
                //          ret,
                //          uxTaskGetStackHighWaterMark(NULL));
            } else {
                ESP_LOGE(TAG, "Opus decode failed: %d", frame_size);
            }
        }
    }
}

// 修改后的音频数据回调函数
static void audio_data_callback(char *data, int len, void *ctx)
{
    struct audio_frame frame;
    
    // 检查数据长度是否合法
    if (len > MAX_PACKET_SIZE) {
        ESP_LOGE(TAG, "Received data too large: %d > %d", len, MAX_PACKET_SIZE);
        return;
    }

    // 检查队列是否已满
    UBaseType_t spaces = uxQueueSpacesAvailable(audio_queue);
    if (spaces == 0) {
        ESP_LOGW(TAG, "Audio queue is full, dropping frame");
        return;
    }
    
    // 复制数据到帧结构
    memcpy(frame.data, data, len);
    frame.len = len;
    
    // 发送到解码队列
    if (xQueueSend(audio_queue, &frame, 0) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue audio frame");
    }
}

/**
 * @brief COZE聊天应用结构体
 */
struct coze_chat_t {
    esp_coze_chat_handle_t chat;       // COZE聊天句柄
    bool                   wakeuped;    // 语音唤醒状态标志
    EventGroupHandle_t     data_evt_group; // 事件组句柄
    QueueHandle_t          btn_evt_q;      // 按键事件队列
};

static struct coze_chat_t coze_chat;

/**
 * @brief 音频事件回调函数
 * 
 * @param event 事件类型
 * @param data 事件数据
 * @param ctx 上下文
 */
static void audio_event_callback(esp_coze_chat_event_t event, char *data, void *ctx)
{
    if (event == ESP_COZE_CHAT_EVENT_CHAT_SPEECH_STARTED) {
        ESP_LOGI(TAG, "chat start");
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_SPEECH_STOPED) {
        ESP_LOGI(TAG, "chat stop");
        audio_read_flag = 0;
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_CUSTOMER_DATA) {
        // cjson格式数据
        ESP_LOGI(TAG, "Customer data: %s", data);
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_SUBTITLE_EVENT) {
        ESP_LOGI(TAG, "Subtitle data: %s", data);
    }
}

/**
 * @brief 初始化COZE聊天模块
 * 
 * @return esp_err_t 
 */
static esp_err_t init_coze_chat()
{
    // 配置COZE聊天参数
    esp_coze_chat_config_t chat_config = ESP_COZE_CHAT_DEFAULT_CONFIG();
    chat_config.enable_subtitle = true;
    chat_config.bot_id = CONFIG_COZE_BOT_ID;
    chat_config.access_token = CONFIG_COZE_ACCESS_TOKEN;
    chat_config.audio_callback = audio_data_callback;
    chat_config.event_callback = audio_event_callback;

    esp_err_t ret = ESP_OK;
    ret = esp_coze_chat_init(&chat_config, &coze_chat.chat);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_coze_chat_init failed, err: %d", ret);
        return ESP_FAIL;
    }
    esp_coze_chat_start(coze_chat.chat);
    return ESP_OK;
}

/**
 * @brief 音频数据读取任务
 * 
 * 根据不同的对话模式(按键触发/语音唤醒)读取并发送音频数据
 */
static void audio_data_read_task(void *pv)
{
    uint8_t *data = heap_caps_calloc(1, 4096 * 3, MALLOC_CAP_SPIRAM);
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for audio buffer");
        vTaskDelete(NULL);
        return;
    }

    size_t bytes_read = 0;
    while (true) {
        #if defined CONFIG_VOICE_WAKEUP_MODE
            esp_err_t ret = audio_data_get(data, 4096 * 3, &bytes_read);
            if (ret == ESP_OK && coze_chat.wakeuped) {
                esp_coze_chat_send_audio_data(coze_chat.chat, (char *)data, bytes_read);
            }
        #else
            if(audio_read_flag){
                esp_err_t ret = audio_data_get(data, 4096 * 3, &bytes_read);
                if (ret == ESP_OK) {
                    esp_coze_chat_send_audio_data(coze_chat.chat, (char *)data, bytes_read);
                }
            }
        #endif 
        vTaskDelay(pdMS_TO_TICKS(10)); // 添加短暂延时避免占用过多CPU
    }

    heap_caps_free(data);
}

/**
 * @brief COZE聊天应用初始化
 * 
 * 该函数完成COZE聊天应用的初始化工作,包括:
 * 1. 设置日志级别
 * 2. 初始化唤醒状态标志
 * 3. 在按键对话模式下配置按键及相关事件处理
 * 4. 初始化COZE聊天核心功能
 * 5. 打开音频处理管道
 * 6. 创建音频数据读取线程
 * 
 * @return esp_err_t 成功返回ESP_OK,失败返回对应错误码
 */
esp_err_t coze_chat_app_init(void)
{
    /* 设置所有模块的日志级别为INFO */
    esp_log_level_set("*", ESP_LOG_INFO);
    
    /* 初始化唤醒状态为false */
    coze_chat.wakeuped = false;

    /* 初始化COZE聊天核心功能 */
    esp_err_t ret = init_coze_chat();
    if (ret != ESP_OK) {
        return ret;
    }
    // 初始化Opus解码器
    ret = init_opus_decoder();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Opus decoder");
        return ret;
    }
    
    // 创建解码任务
    // 使用静态方式创建任务
    TaskHandle_t task_handle = xTaskCreateStaticPinnedToCore(
        opus_decode_task,
        "opus_decode_task",
        OPUS_DECODE_TASK_STACK_SIZE,
        NULL,
        12,
        opus_decode_task_stack,
        &opus_decode_task_buffer,
        1);

    if (task_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create opus decode task");
        return ESP_FAIL;
    }

    /* 创建音频数据读取任务:
     * - 任务名称: audio_data_read_task
     * - 任务函数: audio_data_read_task
     * - 堆栈大小: 3096字节
     * - 任务参数: NULL
     * - 优先级: 12 
     * - 任务句柄: NULL
     * - 运行核心: 1
     */
    xTaskCreatePinnedToCore(audio_data_read_task,
                           "audio_data_read_task", 
                           3096,
                           NULL,
                           12,
                           NULL,
                           1);

    return ESP_OK;
}
