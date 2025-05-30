/*** 
 * @Author: jixingnian@gmail.com
 * @Date: 2025-05-30 16:21:06
 * @LastEditTime: 2025-05-30 16:51:07
 * @LastEditors: 星年
 * @Description: 
 * @FilePath: \ESP32-ChunFeng\components\network\include\bsp_wifi.hpp
 * @遇事不决，可问春风
 */
#pragma once
#include <string>

namespace chunfeng {

// BspWiFi类：封装ESP32 WiFi底层操作
class BspWiFi {
public:
    BspWiFi();   // 构造函数，初始化WiFi
    ~BspWiFi();  // 析构函数，反初始化WiFi

    // 保存WiFi信息到NVS
    bool saveWiFiInfo(const std::string& ssid, const std::string& password);
    // 从NVS读取WiFi信息
    bool loadWiFiInfo(std::string& ssid, std::string& password);
    // 删除NVS中的WiFi信息
    bool deleteWiFiInfo();

    // 连接指定WiFi
    bool connect(const std::string& ssid, const std::string& password);
    // 断开WiFi连接
    void disconnect();

private:
    bool initialized_; // WiFi是否已初始化
    bool connected_;   // WiFi是否已连接
};

} // namespace chunfeng


