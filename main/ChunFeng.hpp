// /*
//  * @Author: xingnian j_xingnian@163.com
//  * @Date: 2025-05-28 21:37:06
//  * @LastEditors: 星年 && j_xingnian@163.com
//  * @LastEditTime: 2025-05-28 21:37:24
//  * @FilePath: \ESP32-ChunFeng\main\ChunFeng.hpp
//  * @Description: ChunFeng主类
//  * 
//  * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
//  */
//  #pragma once

// // 标准库头文件
// #include <memory>   // 用于智能指针
// #include <string>   // 用于字符串处理
// // ESP32 系统头文件
// #include "esp_err.h"  // ESP32错误处理
// #include "esp_log.h"  // ESP32日志系统

// // 前向声明各个管理器类,避免循环依赖
// class NetworkManager;  // 网络管理器
// class AudioManager;    // 音频管理器
// class CozeManager;     // 对话管理器
// class WebManager;      // Web服务管理器

// /**
//  * @brief ChunFeng主类 - 采用单例模式实现
//  * 
//  * 该类作为整个系统的核心控制器,管理各个子系统模块
//  */
// class ChunFeng {
// public:
//     /**
//      * @brief 获取ChunFeng单例对象
//      * @return ChunFeng& 返回单例对象引用
//      */
//     static ChunFeng& getInstance() {
//         static ChunFeng instance;
//         return instance;
//     }

//     /**
//      * @brief 初始化ChunFeng系统
//      * @return esp_err_t 返回初始化结果
//      */
//     esp_err_t init();

//     /**
//      * @brief 启动ChunFeng系统
//      * @return esp_err_t 返回启动结果
//      */
//     esp_err_t start();

//     /**
//      * @brief 停止ChunFeng系统
//      */
//     void stop();

//     // 获取各个管理器对象的访问接口
//     NetworkManager* getNetworkManager() { return networkManager.get(); }
//     AudioManager* getAudioManager() { return audioManager.get(); }
//     CozeManager* getCozeManager() { return cozeManager.get(); }
//     WebManager* getWebManager() { return webManager.get(); }

// private:
//     // 私有构造和析构函数,确保单例模式
//     ChunFeng() = default;
//     ~ChunFeng() = default;
//     // 删除拷贝构造和赋值操作符,防止对象被复制
//     ChunFeng(const ChunFeng&) = delete;
//     ChunFeng& operator=(const ChunFeng&) = delete;

//     // 使用智能指针管理各个子系统,自动管理内存
//     std::unique_ptr<NetworkManager> networkManager;  // 网络管理器实例
//     std::unique_ptr<AudioManager> audioManager;      // 音频管理器实例
//     std::unique_ptr<CozeManager> cozeManager;        // 对话管理器实例
//     std::unique_ptr<WebManager> webManager;          // Web管理器实例

//     static const char* TAG;  // 日志标签
// };