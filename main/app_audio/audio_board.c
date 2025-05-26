#include "audio_board.h"
#include <string.h>
#include "esp_log.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#define TAG "AUDIO_BOARD"

// I2S配置
#define I2S_SAMPLE_RATE     (16000)
#define I2S_BITS_PER_SAMPLE (16)
#define I2S_DMA_BUF_COUNT   (8)
#define I2S_DMA_BUF_LEN     (1024)

// 音频板初始化
esp_err_t audio_board_init(void)
{
    esp_err_t ret = ESP_OK;
    
    // I2S0配置(输入)
    i2s_config_t i2s0_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 单声道
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = I2S_DMA_BUF_COUNT,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1
    };
    
    // I2S1配置(输出)
    i2s_config_t i2s1_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 单声道
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = I2S_DMA_BUF_COUNT,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1
    };
    
    ret |= i2s_driver_install(I2S_NUM_0, &i2s0_config, 0, NULL);
    ret |= i2s_driver_install(I2S_NUM_1, &i2s1_config, 0, NULL);
    audio_input_init();
    audio_output_init();
    ESP_LOGI(TAG, "音频配置完成");
    return ret;
}

#define GPIO_I2S0_LRCK       (GPIO_NUM_42)/*!< I2S左右声道时钟信号引脚 */
#define GPIO_I2S0_MCLK       (GPIO_NUM_NC)/*!< I2S主时钟信号引脚,当前未使用 */
#define GPIO_I2S0_SCLK       (GPIO_NUM_41)/*!< I2S串行时钟信号引脚 */
#define GPIO_I2S0_SDIN       (GPIO_NUM_2) /*!< I2S数据输入信号引脚 */
#define GPIO_I2S0_DOUT       (GPIO_NUM_NC)/*!< I2S数据输出信号引脚,当前未使用 */
// 音频输入初始化
esp_err_t audio_input_init(void)
{
    esp_err_t ret = ESP_OK;
    i2s_pin_config_t pin_config = {
        .bck_io_num = GPIO_I2S0_SCLK,     // 位时钟
        .ws_io_num = GPIO_I2S0_LRCK,      // 字时钟
        .data_out_num = GPIO_I2S0_DOUT,   // 数据输出
        .data_in_num = GPIO_I2S0_SDIN,    // 数据输入
        .mck_io_num = GPIO_I2S0_MCLK,     // 主时钟
    };
    ret |= i2s_set_pin(I2S_NUM_0, &pin_config);
    return ret;
}

#define GPIO_I2S1_LRCK       (GPIO_NUM_8)
#define GPIO_I2S1_MCLK       (GPIO_NUM_NC)
#define GPIO_I2S1_SCLK       (GPIO_NUM_19)
#define GPIO_I2S1_SDIN       (GPIO_NUM_NC)
#define GPIO_I2S1_DOUT       (GPIO_NUM_20)
// 音频输出初始化  
esp_err_t audio_output_init(void)
{
    esp_err_t ret = ESP_OK;
    i2s_pin_config_t pin_config = {
        .bck_io_num = GPIO_I2S1_SCLK,     // 位时钟
        .ws_io_num = GPIO_I2S1_LRCK,      // 字时钟
        .data_out_num = GPIO_I2S1_DOUT,   // 数据输出
        .data_in_num = GPIO_I2S1_SDIN,    // 数据输入
        .mck_io_num = GPIO_I2S1_MCLK,     // 主时钟
    };
    ret |= i2s_set_pin(I2S_NUM_1, &pin_config);
    return ret;
}

// 获取音频数据
esp_err_t audio_data_get(void *buf, size_t len, size_t *bytes_read)
{
    return i2s_read(I2S_NUM_0, buf, len, bytes_read, portMAX_DELAY);
}

// 播放音频数据
esp_err_t audio_data_play(void *buf, size_t len, size_t *bytes_written)
{
    return i2s_write(I2S_NUM_1, buf, len, bytes_written, portMAX_DELAY);
}

// 设置音量
esp_err_t audio_volume_set(int volume)
{
    esp_err_t ret = ESP_OK;
    if(volume < 0 || volume > 100) {
        return ESP_ERR_INVALID_ARG;
    }
    // TODO: 实现音量控制
    ESP_LOGI(TAG, "Set volume to %d%%", volume);
    return ret;
}

// 获取当前音量
esp_err_t audio_volume_get(int *volume)
{
    esp_err_t ret = ESP_OK;
    // TODO: 获取当前音量
    return ret;
}

// 音频板反初始化
esp_err_t audio_board_deinit(void)
{
    esp_err_t ret = ESP_OK;
    ret |= i2s_driver_uninstall(I2S_NUM_0);
    ret |= i2s_driver_uninstall(I2S_NUM_1);
    return ret;
}


