
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flower_black.c"
#include "flower_white.c"
#include "pusty_white.c"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_sleep.h"
#include "driver/gpio.h"

#include "DEV_Config.h"
#include "EPD_2in66.h"
#include "GUI_Paint.h"
#include "fonts.h"

#include "led_strip.h"
#include "driver/rmt_tx.h"
#include "led_strip_rmt.h"


#define WAKEUP_GPIO GPIO_NUM_2

#define LED_STRIP_GPIO 8
#define LED_STRIP_LED_NUM 1


/* ---------- Funkcia na zobrazenie textu na e-paper ---------- */
UBYTE *image_buffer;

void draw_text(const char *text, int x, int y, sFONT *font)
{
    Paint_DrawString_EN(x, y, (char *)text, font, BLACK, WHITE);
}



void display_update()
{
    EPD_2IN66_Display(image_buffer);
}

void display_sleep()
{
    EPD_2IN66_Sleep();
    free(image_buffer);
}

void display_init()
{
    DEV_Module_Init();

    EPD_2IN66_Init();
    EPD_2IN66_Clear();

    image_buffer = malloc(EPD_2IN66_WIDTH * EPD_2IN66_HEIGHT / 8);

    Paint_NewImage(image_buffer, EPD_2IN66_WIDTH, EPD_2IN66_HEIGHT, 0, WHITE);
    Paint_SelectImage(image_buffer);
   // Paint_SetRotate(ROTATE_90);
    Paint_Clear(WHITE);
}

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
    


    display_init();
    
    Paint_DrawBitmap_universal(gImage_pusty_white,WHITE, ROTATE_270);

    display_update();

    vTaskDelay(pdMS_TO_TICKS(3000));

    display_sleep();

}