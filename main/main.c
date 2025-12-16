#include <stdio.h>


    #include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "DEV_Config.h"
#include "EPD_2in66.h" // skopíruj z waveshare repo do components/epd2in66/
#include "GUI_Paint.h"      /* alebo Paint.h / GUI.h */
#include "fonts.h" 


//TODO dorobit importy fontov a nastavenie vypisu





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

    

    UBYTE *image_buffer = (UBYTE *)malloc(EPD_2IN66_WIDTH * EPD_2IN66_HEIGHT / 8);
    if (image_buffer == NULL) {
        printf("Buffer allocation failed\n");
        EPD_2IN66_Sleep();
        DEV_Module_Exit();
        return;
    }
    Paint_NewImage(image_buffer, EPD_2IN66_WIDTH, EPD_2IN66_HEIGHT, 0, WHITE);
    Paint_SelectImage(image_buffer);
    Paint_SetRotate(ROTATE_90);
    Paint_Clear(WHITE);

     const char *line1 = "Najlepsi doktorand: ";
     const char *line2 = "Ing. Michal Kubaščík";

      Paint_DrawString_EN(10, 10, (char *)line1, &Font12, BLACK, WHITE);
      Paint_DrawString_EN(10, 40, (char *)line2, &Font12, BLACK, WHITE);

      /* 5) Poslať buffer na displej */
    EPD_2IN66_Display(image_buffer);
    /* Počkajte chvíľu aby bol text čitateľný */
    DEV_Delay_ms(2000);

    /* 6) (Voliteľné) reset/uloženie energie - uspanie displeja */
    EPD_2IN66_Sleep();

    /* uvoľnenie bufferu a ukončenie modulu */
    free(image_buffer);
    DEV_Module_Exit();

    printf("Done\n");
    return;

}
