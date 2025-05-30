// /*
//  * @Author: xingnian j_xingnian@163.com
//  * @Date: 2025-05-29 20:48:35
//  * @LastEditors: 星年 && j_xingnian@163.com
//  * @LastEditTime: 2025-05-29 20:48:59
//  * @FilePath: \ESP32-ChunFeng\main\include\display_manager.hpp
//  * @Description: 显示管理类
//  * 
//  * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
//  */
// #pragma once

// #include "system_state.hpp"
// #include "network_manager.hpp"
// #include <string>

// namespace chunfeng {

// /**
//  * @brief 显示管理类
//  */
// class DisplayManager {
// public:
//     static DisplayManager& getInstance();

//     /**
//      * @brief 更新系统状态显示
//      */
//     void updateSystemState(SystemState state);

//     /**
//      * @brief 更新网络状态显示
//      */
//     void updateNetworkState(NetworkState state);

//     /**
//      * @brief 更新音频状态显示
//      */
//     void updateAudioState(const std::string& state);

//     /**
//      * @brief 更新Coze状态显示
//      */
//     void updateCozeState(const std::string& state);

// private:
//     DisplayManager() = default;
// };

// } // namespace chunfeng