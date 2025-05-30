/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-30 20:05:17
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-30 20:37:21
 * @FilePath: \ESP32-ChunFeng\components\network\src\config_manager.cpp
 * @Description: 
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "config_manager.hpp"
#include <iostream>

namespace chunfeng {

ConfigManager::ConfigManager() {
    std::cout << "[ConfigManager] 初始化..." << std::endl;
    initialized_ = true;
    started_ = false;
}

ConfigManager::~ConfigManager() {
    std::cout << "[ConfigManager] 析构..." << std::endl;
    stopConfig();
    initialized_ = false;
    started_ = false;
}

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::isInitialized() const {
    return initialized_;
}

bool ConfigManager::startConfig() {
    if (!initialized_) {
        std::cerr << "[ConfigManager] 错误：未初始化，无法启动配网。" << std::endl;
        return false;
    }
    if (started_) {
        std::cout << "[ConfigManager] 配网已启动，无需重复启动。" << std::endl;
        return true;
    }
    std::cout << "[ConfigManager] 启动配网（AP+STA+网页）..." << std::endl;
    // 启动配网驱动
    if (!config_network_.start()) {
        std::cerr << "[ConfigManager] 配网驱动启动失败！" << std::endl;
        return false;
    }
    started_ = true;
    return true;
}

void ConfigManager::stopConfig() {
    if (!started_) {
        std::cout << "[ConfigManager] 配网未启动，无需停止。" << std::endl;
        return;
    }
    std::cout << "[ConfigManager] 停止配网..." << std::endl;
    // 停止配网驱动
    config_network_.stop();
    started_ = false;
}

} // namespace chunfeng 