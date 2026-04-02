

#include "../components/captive_portal/src/dns_server.h"

#include "wake_up.h"

#include "spiff_my.h"

#include "esp_err.h"
#include "GUI_Paint.h"
#include "fonts.h"
#include "server.h"
#include "dis_wraper.h"




#include "freertos/event_groups.h"

#include "nvs_flash.h"




/* ---------- Hlavný program ---------- */

void app_main(void)
{
    
    /*------------------ Wifi setup --------------------------*/
    print_wakeup_reason();
    ESP_ERROR_CHECK(nvs_flash_init());
    init_spiffs();
    wifi_init_ap();
    dns_server_start(esp_ip4addr_aton("192.168.4.1"));
    start_webserver(); 

    display_init();
    Paint_DrawString_EN(5, 70, "Name:", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(110, 68, WIFI_SSID, &Font20, BLACK, WHITE);
     Paint_DrawString_EN(5, 90, "Password:", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(110, 88, WIFI_PASS, &Font20, BLACK, WHITE);
    //Paint_DrawLine(25, 60, 175, 60, BLACK, 2, LINE_STYLE_DOTTED);
    //Paint_DrawNumDecimals(25, 100, 77, &Font16, 0, BLACK, WHITE);
    //Paint_DrawBitmap_universal(gImage_pusty_white,WHITE, ROTATE_270); kreslenie obrazku s otočením 
    
    display_update();
    save_original_buffer();

    vTaskDelay(pdMS_TO_TICKS(3000));
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    //display_sleep();
    

}