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
    std::cout << "[ConfigManager] 启动配网..." << std::endl;
    // 这里添加实际的配网启动代码
    started_ = true;
    return true;
}

void ConfigManager::stopConfig() {
    if (!started_) {
        std::cout << "[ConfigManager] 配网未启动，无需停止。" << std::endl;
        return;
    }
    std::cout << "[ConfigManager] 停止配网..." << std::endl;
    // 这里添加实际的配网停止代码
    started_ = false;
}

} // namespace chunfeng 