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

#define AUDIO_PLAY_QUEUE_SIZE    32
#define AUDIO_PLAY_TIMEOUT_MS    500  // 100ms没有新数据就认为播放结束
#define AUDIO_PLAY_TASK_STACK    4096

// 添加新的全局变量
static QueueHandle_t audio_play_queue = NULL;
static TaskHandle_t audio_play_task_handle = NULL;

// 定义音频数据包结构
typedef struct {
    uint8_t *data;
    size_t len;
} audio_play_item_t;

// 音频播放任务
static void audio_play_task(void *arg)
{
    TickType_t last_play_time = xTaskGetTickCount();
    bool buffer_cleared = false;
    
    while (1) {
        audio_play_item_t item;
        
        // 等待新的音频数据，超时时间设为10ms
        if (xQueueReceive(audio_play_queue, &item, pdMS_TO_TICKS(10)) == pdTRUE) {
            // 收到新数据，更新最后播放时间
            last_play_time = xTaskGetTickCount();
            buffer_cleared = false;
            
            // 播放音频数据
            size_t bytes_written = 0;
            esp_err_t ret = i2s_write(I2S_NUM_1, item.data, item.len, &bytes_written, portMAX_DELAY);
            
            // 释放数据缓冲区
            free(item.data);
            
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to play audio data: %d", ret);
            }
        } else {
            // 检查是否超时
            if (!buffer_cleared && (xTaskGetTickCount() - last_play_time) > pdMS_TO_TICKS(AUDIO_PLAY_TIMEOUT_MS)) {
                // 超过100ms没有新数据，认为播放结束，只清空一次缓冲区
                ESP_LOGI(TAG, "Audio playback timeout, clearing buffer");
                i2s_zero_dma_buffer(I2S_NUM_1);
                buffer_cleared = true;
                last_play_time = xTaskGetTickCount();  // 重置计时器
            }
        }
    }
}
// 音频板初始化
esp_err_t audio_board_init(void)
{
    esp_err_t ret = ESP_OK;


    // 创建音频播放队列
    audio_play_queue = xQueueCreate(AUDIO_PLAY_QUEUE_SIZE, sizeof(audio_play_item_t));
    if (audio_play_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create audio play queue");
        return ESP_FAIL;
    }
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

    // 创建音频播放任务
    BaseType_t task_ret = xTaskCreate(
        audio_play_task,
        "audio_play",
        AUDIO_PLAY_TASK_STACK,
        NULL,
        configMAX_PRIORITIES - 3,
        &audio_play_task_handle
    );
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
    if (audio_play_queue == NULL) {
        return ESP_FAIL;
    }
    
    // 分配新的缓冲区并复制数据
    uint8_t *data = malloc(len);
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for audio data");
        return ESP_ERR_NO_MEM;
    }
    memcpy(data, buf, len);
    
    // 创建音频数据包
    audio_play_item_t item = {
        .data = data,
        .len = len
    };
    
    // 发送到队列
    if (xQueueSend(audio_play_queue, &item, 0) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue audio data");
        free(data);
        return ESP_FAIL;
    }
    
    // 更新已写入字节数
    if (bytes_written) {
        *bytes_written = len;
    }
    
    return ESP_OK;
}

// 清空音频缓冲区
esp_err_t audio_play_buffer_clear(void)
{
    esp_err_t ret = ESP_OK;
    // 播放一帧空数据
    size_t bytes_written = 0;
    uint8_t silence[32] = {0};
    ret |= audio_data_play(silence, sizeof(silence), &bytes_written);
    return ret;
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
    // 删除任务
    if (audio_play_task_handle != NULL) {
        vTaskDelete(audio_play_task_handle);
        audio_play_task_handle = NULL;
    }
    
    // 删除队列
    if (audio_play_queue != NULL) {
        vQueueDelete(audio_play_queue);
        audio_play_queue = NULL;
    }
    ret |= i2s_driver_uninstall(I2S_NUM_0);
    ret |= i2s_driver_uninstall(I2S_NUM_1);
    return ret;
}


