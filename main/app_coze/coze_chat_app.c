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


#include "esp_coze_chat.h"
#include "audio_board.h"

/* 按键录音状态位定义 */
#define BUTTON_REC_READING (1 << 0)

static char *TAG = "COZE_CHAT_APP";

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
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_CUSTOMER_DATA) {
        // cjson格式数据
        ESP_LOGI(TAG, "Customer data: %s", data);
    } else if (event == ESP_COZE_CHAT_EVENT_CHAT_SUBTITLE_EVENT) {
        ESP_LOGI(TAG, "Subtitle data: %s", data);
    }
}

/**
 * @brief 音频数据回调函数,用于播放音频
 */
static void audio_data_callback(char *data, int len, void *ctx)
{
    size_t bytes_written = 0;
    audio_data_play(data, len, &bytes_written);
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
        esp_err_t ret = audio_data_get(data, 4096 * 3, &bytes_read);
        if (ret == ESP_OK) {
            esp_coze_chat_send_audio_data(coze_chat.chat, (char *)data, bytes_read);
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
