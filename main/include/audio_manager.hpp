/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-29 20:48:04
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-29 20:48:30
 * @FilePath: \ESP32-ChunFeng\main\include\audio_manager.hpp
 * @Description: 音频管理类
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <vector>

namespace chunfeng {

/**
 * @brief 音频参数结构体
 */
struct AudioConfig {
    int sample_rate;    ///< 采样率
    int channels;       ///< 通道数
    int bit_depth;      ///< 位深度
};

/**
 * @brief 音频管理类
 */
class AudioManager {
public:
    static AudioManager& getInstance();

    /**
     * @brief 初始化音频系统
     */
    bool initialize(const AudioConfig& config);

    /**
     * @brief 反初始化音频系统
     */
    void deinitialize();

    /**
     * @brief 获取麦克风数据
     */
    std::vector<uint8_t> getMicData();

    /**
     * @brief 播放音频数据
     */
    bool playAudio(const std::vector<uint8_t>& audio_data);

    /**
     * @brief 获取音频配置
     */
    AudioConfig getConfig() const;

private:
    AudioManager() = default;
    AudioConfig config_{};
};

} // namespace chunfeng