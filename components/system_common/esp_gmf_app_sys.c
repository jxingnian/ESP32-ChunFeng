/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

/* 包含必要的头文件 */
#include <string.h>
#include "esp_gmf_oal_mem.h"     /* 内存管理相关头文件 */
#include "esp_gmf_oal_sys.h"     /* 系统功能相关头文件 */
#include "freertos/FreeRTOS.h"   /* FreeRTOS核心头文件 */
#include "freertos/event_groups.h"/* FreeRTOS事件组头文件 */

/* 监控任务运行标志 */
static bool monitor_run;

/**
 * @brief 系统监控任务函数
 * 
 * @param para 任务参数(未使用)
 * @details 该任务每5秒执行一次,显示内存使用情况并获取系统实时统计信息
 */
static void sys_monitor_task(void *para)
{
    while (monitor_run) {
        /* 延时5秒 */
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        /* 检查是否需要退出任务 */
        if (monitor_run == false) {
            break;
        }
        /* 显示内存使用情况 */
        ESP_GMF_MEM_SHOW("MONITOR");
        /* 获取系统实时统计信息,统计时间1秒,不打印CPU使用率 */
        esp_gmf_oal_sys_get_real_time_stats(1000, false);
    }
    /* 删除当前任务 */
    vTaskDelete(NULL);
}

/**
 * @brief 启动系统监控
 * 
 * @details 创建系统监控任务,设置4KB堆栈,优先级1,运行在CPU核心1上
 */
void esp_gmf_app_sys_monitor_start(void)
{
    monitor_run = true;
    xTaskCreatePinnedToCore(sys_monitor_task, "sys_monitor_task", (4 * 1024), NULL, 1, NULL, 1);
}

/**
 * @brief 停止系统监控
 * 
 * @details 通过设置运行标志使监控任务退出
 */
void esp_gmf_app_sys_monitor_stop(void)
{
    monitor_run = false;
}
