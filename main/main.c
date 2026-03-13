
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spiff_my.h"

#include "esp_err.h"
#include "pusty_white.c"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_sleep.h"
#include "driver/gpio.h"

#include "DEV_Config.h"
#include "EPD_2in66.h"
#include "GUI_Paint.h"
#include "fonts.h"
#include "server.h"
#include "dis_wraper.h"

#include "led_strip.h"
#include "driver/rmt_tx.h"
#include "led_strip_rmt.h"



#include "freertos/event_groups.h"

#include "nvs_flash.h"

#define WAKEUP_GPIO GPIO_NUM_2

#define LED_STRIP_GPIO 8
#define LED_STRIP_LED_NUM 1














/* ---------- Funkcia na inicializáciu LED ---------- */

led_strip_handle_t init_led()
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_LED_NUM,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
    };

    led_strip_handle_t led_strip;

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip)
    );

    return led_strip;
}


/* ---------- Funkcia na nastavenie tlačidla ---------- */

void setup_button()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << WAKEUP_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };

    gpio_config(&io_conf);

    esp_sleep_enable_ext1_wakeup(
        1ULL << WAKEUP_GPIO,
        ESP_EXT1_WAKEUP_ANY_HIGH
    );
}


/* ---------- Hlavný program ---------- */

void app_main(void)
{
    /*------------------ Wifi setup --------------------------*/
    ESP_ERROR_CHECK(nvs_flash_init());
    init_spiffs();
    wifi_init_ap();
    start_webserver(); 

    display_init();

    Paint_DrawString_EN(70, 70, "192.168.4.1", &Font20, BLACK, WHITE);
    //Paint_DrawLine(25, 60, 175, 60, BLACK, 2, LINE_STYLE_DOTTED);
    //Paint_DrawNumDecimals(25, 100, 77, &Font16, 0, BLACK, WHITE);
    //Paint_DrawBitmap_universal(gImage_pusty_white,WHITE, ROTATE_270); kreslenie obrazku s otočením 
    
    display_update();

    vTaskDelay(pdMS_TO_TICKS(3000));
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    //display_sleep();
    
}