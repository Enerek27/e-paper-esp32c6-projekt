
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "flower_black.c"
#include "flower_white.c"
#include "pusty_white.c"
#include "esp_mac.h"
#include "website.h"

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


#include <esp_http_server.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"




#define WAKEUP_GPIO GPIO_NUM_2

#define LED_STRIP_GPIO 8
#define LED_STRIP_LED_NUM 1

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define WIFI_SSID      "E-vizitka"
#define WIFI_PASS      "evizitka1"
#define WIFI_CHANNEL 1
#define MAX_STA_CONN 4

esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, webpage, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL
};


httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
    }

    return server;
}


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;

        ESP_LOGI("wifi", "device connected: "MACSTR, MAC2STR(event->mac));
    }

    if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;

        ESP_LOGI("wifi", "device disconnected: "MACSTR, MAC2STR(event->mac));
    }
}

void wifi_init_ap(void)
{
    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT,
                                         ESP_EVENT_ANY_ID,
                                         &wifi_event_handler,
                                         NULL,
                                         NULL);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK
        },
    };

    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI("wifi", "WiFi AP started. SSID:%s", WIFI_SSID);
}

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

    Paint_NewImage(image_buffer, EPD_2IN66_WIDTH, EPD_2IN66_HEIGHT, 90, WHITE);
    Paint_SelectImage(image_buffer);
    Paint_SetRotate(ROTATE_90);
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
    /*------------------ Wifi setup --------------------------*/
    /* ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_ap();
    start_webserver(); */
    
    display_init();

    Paint_DrawString_EN(25, 40, "penis", &Font16, BLACK, WHITE);
    Paint_DrawLine(25, 60, 175, 60, BLACK, 2, LINE_STYLE_DOTTED);
    Paint_DrawNumDecimals(25, 100, 77, &Font16, 0, BLACK, WHITE);
    //Paint_DrawBitmap_universal(gImage_pusty_white,WHITE, ROTATE_270); kreslenie obrazku s otočením 
    
    display_update();

    vTaskDelay(pdMS_TO_TICKS(3000));

    display_sleep();

}