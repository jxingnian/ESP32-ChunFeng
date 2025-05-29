/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-29 20:48:51
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-29 20:49:12
 * @FilePath: \ESP32-ChunFeng\main\include\backend_manager.hpp
 * @Description: 后台管理类
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

#include <string>
#include "audio_manager.hpp"
#include "coze_manager.hpp"

namespace chunfeng {

/**
 * @brief 系统设置结构体
 */
struct SystemSettings {
    std::string device_name;
    std::string firmware_version;
    // TODO: 添加其他系统设置项
};

/**
 * @brief 后台管理类
 */
class BackendManager {
public:
    static BackendManager& getInstance();

    /**
     * @brief 获取当前系统状态
     */
    std::string getCurrentStatus() const;

    /**
     * @brief 更新网络设置
     */
    bool updateNetworkSettings(const std::string& settings);

    /**
     * @brief 更新音频设置
     */
    bool updateAudioSettings(const AudioConfig& config);

    /**
     * @brief 更新对话设置
     */
    bool updateCozeSettings(const CozeConfig& config);

    /**
     * @brief 更新系统设置
     */
    bool updateSystemSettings(const SystemSettings& settings);

private:
    BackendManager() = default;
    SystemSettings system_settings_{};
};

} // namespace chunfeng