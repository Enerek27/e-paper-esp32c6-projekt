#include "DEV_Config.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DEV_Config";


static spi_device_handle_t spi_handle = NULL;


int DEV_Module_Init(void)
{
    esp_err_t ret;

   
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<EPD_DC_PIN) | (1ULL<<EPD_RST_PIN) | (1ULL<<EPD_BUSY_PIN) | (1ULL<<EPD_CS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
   
    io_conf.pin_bit_mask = (1ULL<<EPD_DC_PIN) | (1ULL<<EPD_RST_PIN) | (1ULL<<EPD_CS_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);
    gpio_set_level(EPD_CS_PIN, 1);

    gpio_config_t in_conf = {
        .pin_bit_mask = (1ULL<<EPD_BUSY_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&in_conf);

    
    spi_bus_config_t buscfg = {
        .mosi_io_num = EPD_MOSI_PIN,
        .miso_io_num = -1,
        .sclk_io_num = EPD_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 8*6000,
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %d", ret);
        return -1;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 8 * 1000 * 1000, // 8 MHz
        .mode = 0,
        .spics_io_num = EPD_CS_PIN,
        .queue_size = 7,
        .flags = 0,
    };

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %d", ret);
        return -1;
    }

    
    gpio_set_level(EPD_RST_PIN, 1);
    gpio_set_level(EPD_DC_PIN, 0);
    gpio_set_level(EPD_CS_PIN, 1);

    ESP_LOGI(TAG, "DEV_Module_Init ok");
    return 0;
}

void DEV_Module_Exit(void)
{
    if (spi_handle) {
        spi_bus_remove_device(spi_handle);
        spi_handle = NULL;
    }
    spi_bus_free(SPI2_HOST);
    ESP_LOGI(TAG, "DEV_Module_Exit");
}

void DEV_Delay_ms(UDOUBLE xms)
{
    vTaskDelay(pdMS_TO_TICKS(xms));
}

void DEV_Digital_Write(uint8_t Pin, uint8_t Value)
{
    gpio_set_level((gpio_num_t)Pin, Value);
}

int DEV_Digital_Read(uint8_t Pin)
{
    return gpio_get_level((gpio_num_t)Pin);
}

void DEV_SPI_WriteByte(UBYTE Data)
{
    if (!spi_handle) return;
    spi_transaction_t t = {0};
    t.length = 8;
    t.flags = SPI_TRANS_USE_TXDATA;
    t.tx_data[0] = Data;
    esp_err_t ret = spi_device_transmit(spi_handle, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI write byte failed: %d", ret);
    }
}

void DEV_SPI_Write_nByte(UBYTE *pData, UWORD Len)
{
    if (!spi_handle || Len == 0) return;
    spi_transaction_t t = {0};
    t.length = Len * 8;
    t.tx_buffer = pData;
    esp_err_t ret = spi_device_transmit(spi_handle, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI write nByte failed: %d", ret);
    }
}