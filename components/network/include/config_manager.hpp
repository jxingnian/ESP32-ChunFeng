/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 12:20:10
 * @LastEditTime: 2025-05-30 14:36:23
 * @LastEditors: 星年
 * @Description: 配网管理类，负责配网的初始化、反初始化及相关操作
 * @FilePath: \ESP32-ChunFeng\components\network\include\config_manager.hpp
 * @遇事不决，可问春风
 */
#pragma once

namespace chunfeng {

/**
 * @brief 配网管理类
 * 
 * 该类采用单例模式，负责配网模块的初始化、反初始化及后续配网相关操作。
 */
class ConfigManager {
public:
    /**
     * @brief 构造函数
     */
    ConfigManager();

    /**
     * @brief 析构函数
     */
    ~ConfigManager();

    /**
     * @brief 获取 ConfigManager 单例实例
     * @return ConfigManager& 单例引用
     */
    static ConfigManager& getInstance();

    /**
     * @brief 检查配网模块是否已初始化
     * @return true 已初始化
     * @return false 未初始化
     */
    bool isInitialized() const;

    /**
     * @brief 启动配网
     * @return true 启动成功
     * @return false 启动失败
     */
    bool startConfig();

    /**
     * @brief 停止配网
     */
    void stopConfig();

    // TODO: 可扩展配网相关接口，如获取配网状态等

private:
    /**
     * @brief 配网模块初始化状态
     */
    bool initialized_{false};

    /**
     * @brief 配网启动状态
     */
    bool started_{false};
};

} // namespace chunfeng 