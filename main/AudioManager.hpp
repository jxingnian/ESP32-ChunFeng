// /*
//  * @Author: xingnian j_xingnian@163.com
//  * @Date: 2025-05-28 21:37:43
//  * @LastEditors: 星年 && j_xingnian@163.com
//  * @LastEditTime: 2025-05-28 21:41:23
//  * @FilePath: \ESP32-ChunFeng\main\AudioManager.hpp
//  * @Description: 音频管理类
//  * 
//  * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
//  */
// #pragma once

// #include <functional>
// #include <vector>
// #include "esp_err.h"
// #include "driver/i2s.h"

// /**
//  * @brief 音频管理类,用于处理音频录制和播放功能
//  */
// class AudioManager {
// public:
//     /**
//      * @brief 音频配置结构体
//      */
//     struct AudioConfig {
//         int sampleRate;     ///< 采样率(Hz)
//         int bitsPerSample;  ///< 采样位数
//         int channels;       ///< 声道数
//         int bufferSize;     ///< 缓冲区大小(字节)
//     };

//     /**
//      * @brief 音频回调函数类型定义
//      * @param data 音频数据缓冲区
//      * @param size 音频数据大小
//      */
//     using AudioCallback = std::function<void(const uint8_t*, size_t)>;

//     /**
//      * @brief 构造函数
//      */
//     AudioManager();

//     /**
//      * @brief 析构函数
//      */
//     ~AudioManager();

//     /**
//      * @brief 初始化音频管理器
//      * @param config 音频配置参数
//      * @return esp_err_t 操作结果
//      */
//     esp_err_t init(const AudioConfig& config);

//     /**
//      * @brief 开始录音
//      * @return esp_err_t 操作结果
//      */
//     esp_err_t startRecording();

//     /**
//      * @brief 停止录音
//      * @return esp_err_t 操作结果
//      */
//     esp_err_t stopRecording();

//     /**
//      * @brief 播放音频数据
//      * @param data 音频数据缓冲区
//      * @param length 音频数据长度
//      * @return esp_err_t 操作结果
//      */
//     esp_err_t playAudio(const uint8_t* data, size_t length);

//     /**
//      * @brief 停止播放
//      * @return esp_err_t 操作结果
//      */
//     esp_err_t stopPlayback();

//     /**
//      * @brief 设置音频数据回调函数
//      * @param callback 回调函数
//      */
//     void setAudioCallback(AudioCallback callback);

//     /**
//      * @brief 检查是否正在录音
//      * @return bool 录音状态
//      */
//     bool isRecording() const { return recording; }

//     /**
//      * @brief 检查是否正在播放
//      * @return bool 播放状态
//      */
//     bool isPlaying() const { return playing; }

// private:
//     /**
//      * @brief 录音任务函数
//      * @param param 任务参数
//      */
//     static void recordTask(void* param);

//     /**
//      * @brief 播放任务函数
//      * @param param 任务参数
//      */
//     static void playbackTask(void* param);

//     AudioConfig config;           ///< 音频配置
//     AudioCallback callback;       ///< 音频回调函数
//     bool recording;              ///< 录音状态标志
//     bool playing;                ///< 播放状态标志
//     TaskHandle_t recordTaskHandle; ///< 录音任务句柄
//     TaskHandle_t playTaskHandle;   ///< 播放任务句柄
    
//     static const char* TAG;      ///< 日志标签
// };