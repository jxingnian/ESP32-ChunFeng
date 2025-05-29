/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-29 20:47:35
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-29 20:48:15
 * @FilePath: \ESP32-ChunFeng\main\include\system_state.hpp
 * @Description: 系统状态管理类
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#pragma once

namespace chunfeng {

/**
 * @brief 系统状态枚举
 */
enum class SystemState {
    INIT,       ///< 初始化状态
    NETWORKING, ///< 联网状态
    RUNNING,    ///< 运行状态
    ERROR       ///< 错误状态
};

/**
 * @brief 系统状态管理类
 */
class SystemStateManager {
public:
    /**
     * @brief 获取单例实例
     */
    static SystemStateManager& getInstance();

    /**
     * @brief 获取当前系统状态
     */
    SystemState getCurrentState() const;

    /**
     * @brief 设置系统状态
     */
    void setState(SystemState state);

private:
    SystemStateManager() = default;
    SystemState current_state_{SystemState::INIT};
};

} // namespace chunfeng