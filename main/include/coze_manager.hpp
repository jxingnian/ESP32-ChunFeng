/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-29 20:48:22
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-29 21:32:31
 * @FilePath: \ESP32-ChunFeng\main\include\coze_manager.hpp
 * @Description: Coze对话管理类
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <string>
#include <vector>

namespace chunfeng {

/**
 * @brief Coze对话参数
 */
struct CozeConfig {
    std::string api_key;        ///< API密钥
    std::string model;          ///< 模型名称
    int max_tokens;             ///< 最大token数
    float temperature;          ///< 温度参数
};

/**
 * @brief Coze管理类
 */
class CozeManager {
public:
    static CozeManager& getInstance();

    /**
     * @brief 初始化Coze
     */
    bool initialize(const CozeConfig& config);

    /**
     * @brief 反初始化Coze
     */
    void deinitialize();

    /**
     * @brief 处理音频输入
     */
    bool handleAudioInput(const std::vector<uint8_t>& audio_data);

    /**
     * @brief 获取音频输出
     */
    std::vector<uint8_t> getAudioOutput();

    /**
     * @brief 获取当前配置
     */
    CozeConfig getConfig() const;

private:
    CozeManager() = default;
    CozeConfig config_{};
};

} // namespace chunfeng