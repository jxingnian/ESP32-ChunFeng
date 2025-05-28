/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO., LTD
 * SPDX-License-Identifier: LicenseRef-Espressif-Modified-MIT
 *
 * See LICENSE file for details.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// I2S DAC配置
#define ESP_GMF_I2S_DAC_MCLK_IO_NUM (GPIO_NUM_16) /* DAC主时钟引脚 */
#define ESP_GMF_I2S_DAC_BCLK_IO_NUM (GPIO_NUM_9)  /* DAC位时钟引脚 */
#define ESP_GMF_I2S_DAC_WS_IO_NUM   (GPIO_NUM_45) /* DAC字选择引脚 */
#define ESP_GMF_I2S_DAC_DO_IO_NUM   (GPIO_NUM_8)  /* DAC数据输出引脚 */
#define ESP_GMF_I2S_DAC_DI_IO_NUM   (GPIO_NUM_10) /* DAC数据输入引脚 */

// I2S ADC配置
#define ESP_GMF_I2S_ADC_MCLK_IO_NUM (GPIO_NUM_16) /* ADC主时钟引脚 */
#define ESP_GMF_I2S_ADC_BCLK_IO_NUM (GPIO_NUM_9)  /* ADC位时钟引脚 */
#define ESP_GMF_I2S_ADC_WS_IO_NUM   (GPIO_NUM_45) /* ADC字选择引脚 */
#define ESP_GMF_I2S_ADC_DO_IO_NUM   (GPIO_NUM_8)  /* ADC数据输出引脚 */
#define ESP_GMF_I2S_ADC_DI_IO_NUM   (GPIO_NUM_10) /* ADC数据输入引脚 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
