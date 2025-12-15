#include <stdio.h>


    #include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DEV_Config.h"
#include "EPD_2in66.h" // skopíruj z waveshare repo do components/epd2in66/

void app_main(void)
{
    // Inicializácia low-level (SPI, GPIO)
    if (DEV_Module_Init() != 0) {
        printf("DEV_Module_Init failed\n");
        return;
    }

    printf("EPD init...\n");
    EPD_2IN66_Init();
    printf("Clear screen...\n");
    EPD_2IN66_Clear();

    // Vytvorenie jednoduchého obrázku: tu plný biely/čierny buffer
    // Rozmery: 152 x 296, bytes per line = (152 + 7) / 8 = 19
    const int width = EPD_2IN66_WIDTH;
    const int height = EPD_2IN66_HEIGHT;
    const int bytes_per_line = (width + 7) / 8;
    const int buf_size = bytes_per_line * height;

    uint8_t *buf = malloc(buf_size);
    if (!buf) {
        printf("No memory\n");
        EPD_2IN66_Sleep();
        DEV_Module_Exit();
        return;
    }

    // vykreslíme len testovací vzor: preblikneme bielu -> čiernu -> biela
    // white = 0xFF (v mnohých drivers biela), black = 0x00
    memset(buf, 0xFF, buf_size);
    EPD_2IN66_Display(buf);
    vTaskDelay(pdMS_TO_TICKS(3000));

    memset(buf, 0x00, buf_size);
    EPD_2IN66_Display(buf);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // späť na bielu
    memset(buf, 0xFF, buf_size);
    EPD_2IN66_Display(buf);
    vTaskDelay(pdMS_TO_TICKS(2000));

    free(buf);

    printf("Sleep...\n");
    EPD_2IN66_Sleep();
    DEV_Module_Exit();
    printf("Done\n");
}
