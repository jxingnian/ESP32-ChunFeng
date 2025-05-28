/*
 * @Author: xingnian j_xingnian@163.com
 * @Date: 2025-05-28 21:39:21
 * @LastEditors: 星年 && j_xingnian@163.com
 * @LastEditTime: 2025-05-28 23:14:40
 * @FilePath: \ESP32-ChunFeng\main\ChunFeng.cpp
 * @Description: ChunFeng主类实现
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#include "ChunFeng.hpp"

const char* ChunFeng::TAG = "ChunFeng";

// 构造函数
ChunFeng::ChunFeng() 
    : currentState(State::INIT)
    , eventQueue(nullptr)
    , stateMachineTaskHandle(nullptr)
    , stateChangeCallback(nullptr) {
    // 创建事件队列
    eventQueue = xQueueCreate(10, sizeof(Event));
    if (eventQueue == nullptr) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return;
    }

    // 创建状态机任务
    BaseType_t ret = xTaskCreate(
        stateMachineTask,
        "state_machine",
        4096,
        this,
        5,
        &stateMachineTaskHandle
    );

    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create state machine task");
    }
}

// 析构函数
ChunFeng::~ChunFeng() {
    if (stateMachineTaskHandle != nullptr) {
        vTaskDelete(stateMachineTaskHandle);
    }
    if (eventQueue != nullptr) {
        vQueueDelete(eventQueue);
    }
}

esp_err_t ChunFeng::init() {
    ESP_LOGI(TAG, "Initializing ChunFeng...");

    // 创建各个管理器实例
    networkManager = std::make_unique<NetworkManager>();
    ESP_ERROR_CHECK(networkManager->init());

    // 发送初始化完成事件
    sendEvent(Event::INIT_DONE);

    return ESP_OK;
}

esp_err_t ChunFeng::start() {
    ESP_LOGI(TAG, "Starting ChunFeng services...");
    return ESP_OK;
}

void ChunFeng::stop() {
    ESP_LOGI(TAG, "Stopping ChunFeng services...");
    if (networkManager) networkManager->disconnect();
}

void ChunFeng::sendEvent(Event event) {
    if (eventQueue != nullptr) {
        xQueueSend(eventQueue, &event, portMAX_DELAY);
    }
}

void ChunFeng::setState(State newState) {
    if (currentState != newState) {
        State oldState = currentState;
        currentState = newState;
        
        // 调用状态变化回调函数
        if (stateChangeCallback) {
            stateChangeCallback(oldState, newState);
        }
        
        ESP_LOGI(TAG, "State changed: %s -> %s", 
                 getStateString(oldState), 
                 getStateString(newState));
    }
}

const char* ChunFeng::getStateString() const {
    return getStateString(currentState);
}

const char* ChunFeng::getStateString(State state) {
    switch (state) {
        case State::INIT:        return "INIT";
        case State::WIFI_CONFIG: return "WIFI_CONFIG";
        case State::CONNECTING:  return "CONNECTING";
        case State::IDLE:        return "IDLE";
        case State::LISTENING:   return "LISTENING";
        case State::RECORDING:   return "RECORDING";
        case State::PROCESSING:  return "PROCESSING";
        case State::SPEAKING:    return "SPEAKING";
        case State::ERROR:       return "ERROR";
        default:                 return "UNKNOWN";
    }
}

void ChunFeng::handleState() {
    Event event;
    if (xQueueReceive(eventQueue, &event, portMAX_DELAY) == pdTRUE) {
        switch (currentState) {
            case State::INIT:
                if (event == Event::INIT_DONE) {
                    setState(State::WIFI_CONFIG);
                }
                break;

            case State::WIFI_CONFIG:
                if (event == Event::CONFIG_DONE) {
                    setState(State::CONNECTING);
                }
                break;

            case State::CONNECTING:
                if (event == Event::WIFI_CONNECTED) {
                    setState(State::IDLE);
                } else if (event == Event::WIFI_DISCONNECTED) {
                    setState(State::WIFI_CONFIG);
                }
                break;

            case State::IDLE:
                if (event == Event::WAKE_UP) {
                    setState(State::LISTENING);
                } else if (event == Event::WIFI_DISCONNECTED) {
                    setState(State::CONNECTING);
                }
                break;

            case State::LISTENING:
                if (event == Event::START_RECORD) {
                    setState(State::RECORDING);
                }
                break;

            case State::RECORDING:
                if (event == Event::STOP_RECORD) {
                    setState(State::PROCESSING);
                }
                break;

            case State::PROCESSING:
                if (event == Event::PROCESS_DONE) {
                    setState(State::SPEAKING);
                }
                break;

            case State::SPEAKING:
                if (event == Event::SPEAK_DONE) {
                    setState(State::IDLE);
                }
                break;

            case State::ERROR:
                if (event == Event::RESET) {
                    setState(State::INIT);
                }
                break;
        }

        // 检查是否发生错误
        if (event == Event::ERROR_OCCURRED) {
            setState(State::ERROR);
        }
    }
}

void ChunFeng::stateMachineTask(void* parameter) {
    ChunFeng* chunFeng = static_cast<ChunFeng*>(parameter);
    while (true) {
        chunFeng->handleState();
    }
}