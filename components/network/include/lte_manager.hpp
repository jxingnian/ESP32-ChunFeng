/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 12:19:57
 * @LastEditTime: 2025-05-30 17:42:26
 * @LastEditors: 星年
 * @Description: LTE（4G）管理类，负责 LTE 的初始化、连接、断开、反初始化与状态查询
 * @FilePath: \ESP32-ChunFeng\components\network\include\lte_manager.hpp
 * @遇事不决，可问春风
 */
#pragma once

#include "esp_log.h"
#include "ml307_at_modem.h"
#include "ml307_ssl_transport.h"
#include "ml307_http.h"
#include "ml307_mqtt.h"

namespace chunfeng {

/**
 * @brief LTE（4G）管理类
 * 
 * 该类采用单例模式，负责LTE模块的初始化、反初始化、连接、断开和状态查询。
 * 主要功能包括：
 *   1. 初始化和反初始化底层LTE硬件及相关资源
 *   2. 连接和断开LTE网络
 *   3. 查询当前LTE连接状态
 */
class LTEManager {
public:
    /**
     * @brief 获取 LTEManager 单例实例
     * 
     * 该方法返回 LTEManager 的唯一实例，确保全局只有一个 LTEManager 对象。
     * 
     * @return LTEManager& 单例引用
     */
    static LTEManager& getInstance();

    /**
     * @brief 初始化 LTE（4G）网络
     * 
     * 尝试连接到配置的 LTE 网络。调用前需确保已初始化。
     * 
     * @return true 连接成功
     * @return false 连接失败
     */
    bool initialize();

    /**
     * @brief 连接 LTE（4G）网络
     * 
     * 尝试连接到配置的 LTE 网络。调用前需确保已初始化。
     * 
     * @return true 连接成功
     * @return false 连接失败
     */
    bool connect();

    /**
     * @brief 断开 LTE（4G）网络连接
     * 
     * 断开当前的 LTE 连接并释放相关资源。调用后 LTE 处于断开状态。
     * 若未连接，则该方法应为幂等操作。
     */
    void disconnect();

    /**
     * @brief 查询 LTE 是否已连接
     * 
     * 用于判断当前 LTE 网络连接状态。
     * 
     * @return true 已连接
     * @return false 未连接
     */
    bool isConnected() const;

    LTEManager(); // 构造函数声明
    ~LTEManager(); // 增加析构函数声明

private:
    /**
     * @brief 构造函数私有化，禁止外部实例化
     * 
     * 仅允许通过 getInstance() 获取单例对象。
     */
    bool initialized_{false};
    bool connected_{false};
};

} // namespace chunfeng 