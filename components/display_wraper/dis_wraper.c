

#include "dis_wraper.h"
#include "DEV_Config.h"
#include <time.h>

UBYTE * image_buffer = NULL;
extern PAINT Paint;


void draw_bitmap_task(void *pvParameters)
{
    draw_bitmap();
    vTaskDelete(NULL);
}

void draw_bitmap_async(void)
{
    xTaskCreate(draw_bitmap_task, "draw_bmp", 8192, NULL, 5, NULL);
}



// Funkcia ktorá vráti pointer na buffer a jeho veľkosť
UBYTE* get_image_buffer(void)
{
    return image_buffer;
}

int get_image_buffer_size(void)
{
    return Paint.WidthByte * Paint.HeightMemory;
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

void display_update()
{
    EPD_2IN66_Display(image_buffer);
}

void display_sleep()
{
    EPD_2IN66_Sleep();
}

void draw_text(const char *text, int x, int y, sFONT *font)
{
    Paint_DrawString_EN(x, y, (char *)text, font, BLACK, WHITE);
}

void clean_buffer() 
{
    free(image_buffer);
    image_buffer = NULL;
}

// Toto len zobrazí čo je aktuálne v image_buffer na displej
void display_show(void)
{
    display_update();
    ESP_LOGI("DRAW", "Buffer displayed.");
}

void display_show_task(void *pvParameters)
{
    display_show();
    vTaskDelete(NULL);
}

void bmp_to_c_array()
{
    FILE *bmp = fopen("/spiffs/bitmap.bmp", "rb");
    if (!bmp) {
        printf("BMP not found\n");
        return;
    }

    // Čítaj BMP hlavičku (54 bajtov)
    uint8_t bmp_header[54];
    if (fread(bmp_header, 1, 54, bmp) != 54) {
        printf("BMP header read failed\n");
        fclose(bmp);
        return;
    }

    // Z BMP hlavičky vytiahni rozmery (offset 18 = width, 22 = height, little-endian)
    int32_t bmp_width  = bmp_header[18] | (bmp_header[19] << 8) |
                         (bmp_header[20] << 16) | (bmp_header[21] << 24);
    int32_t bmp_height = bmp_header[22] | (bmp_header[23] << 8) |
                         (bmp_header[24] << 16) | (bmp_header[25] << 24);
    // BMP môže mať záporný height (top-down), berieme absolútnu hodnotu
    if (bmp_height < 0) bmp_height = -bmp_height;

    printf("BMP size: %ldx%ld\n", bmp_width, bmp_height);

    // Čítaj pixel dáta (od offsetu 54)
    // BMP riadky sú zarovnané na 4 bajty
    int bmp_row_bytes = ((bmp_width + 31) / 32) * 4;  // pre 1bpp
    // Ak je to 24bpp BMP, uprav podľa farebnej hĺbky
    uint8_t bpp = bmp_header[28]; // bits per pixel
    if (bpp == 24) bmp_row_bytes = ((bmp_width * 3 + 3) / 4) * 4;
    else if (bpp == 8) bmp_row_bytes = ((bmp_width + 3) / 4) * 4;
    else if (bpp == 1) bmp_row_bytes = ((bmp_width + 31) / 32) * 4;

    // Načítaj všetky pixel dáta
    long pixel_offset = bmp_header[10] | (bmp_header[11] << 8);
    fseek(bmp, pixel_offset, SEEK_SET);

    // Výsledná bitmap: 1 bit na pixel, bez zarovnania (len (width+7)/8 bajtov na riadok)
    int out_row_bytes = (bmp_width + 7) / 8;
    int total_bytes   = out_row_bytes * bmp_height;

    uint8_t *out_buf = malloc(total_bytes);
    if (!out_buf) {
        printf("malloc failed\n");
        fclose(bmp);
        return;
    }
    memset(out_buf, 0xFF, total_bytes); // default biela

    uint8_t *row_buf = malloc(bmp_row_bytes);
    if (!row_buf) {
        free(out_buf);
        fclose(bmp);
        return;
    }

    // BMP je uložený zdola nahor (bottom-up), preto čítame riadky v opačnom poradí
    for (int y = bmp_height - 1; y >= 0; y--) {
        fread(row_buf, 1, bmp_row_bytes, bmp);
        int out_y = (bmp_height - 1 - y); // prehodíme poradie

        for (int x = 0; x < bmp_width; x++) {
            uint8_t pixel_black = 0;

            if (bpp == 24) {
                uint8_t b = row_buf[x * 3 + 0];
                uint8_t g = row_buf[x * 3 + 1];
                uint8_t r = row_buf[x * 3 + 2];
                // Ak je pixel tmavý (čierny) → bit = 0, inak biely → bit = 1
                uint8_t gray = (r * 77 + g * 150 + b * 29) >> 8;
                pixel_black = (gray < 128) ? 1 : 0;
            } else if (bpp == 1) {
                uint8_t byte = row_buf[x / 8];
                pixel_black = !((byte >> (7 - (x % 8))) & 1);
            } else if (bpp == 8) {
                pixel_black = (row_buf[x] < 128) ? 1 : 0;
            }

            if (pixel_black) {
                // Nastav bit na 0 (čierna = 0 v tvojom formáte)
                int byte_idx = out_y * out_row_bytes + (x / 8);
                out_buf[byte_idx] &= ~(0x80 >> (x % 8));
            }
        }
    }

    free(row_buf);
    fclose(bmp);

    // Zapis do /spiffs/bitmap.c v požadovanom formáte
    FILE *out = fopen("/spiffs/bitmap.c", "wb");
    if (!out) {
        printf("Cannot create C file\n");
        free(out_buf);
        return;
    }

    // Hlavička: marker + width (little-endian) + height (little-endian)
    uint8_t header[6] = {
        0x00, 0x01,
        (uint8_t)(bmp_width  & 0xFF), (uint8_t)((bmp_width  >> 8) & 0xFF),
        (uint8_t)(bmp_height & 0xFF), (uint8_t)((bmp_height >> 8) & 0xFF)
    };
    fwrite(header, 1, 6, out);
    fwrite(out_buf, 1, total_bytes, out);

    fclose(out);
    free(out_buf);

    printf("bitmap.c created: %ldx%ld, %d bytes\n", bmp_width, bmp_height, 6 + total_bytes);
}


void draw_bitmap()
{
    bmp_to_c_array();
    // Otvor binárny súbor zo SPIFFS
    FILE *f = fopen("/spiffs/bitmap.c", "rb");
    if (!f) {
        ESP_LOGE("DRAW", "bitmap.c file not found!");
        return;
    }

    // Čítaj hlavičku (6 bajtov)
    uint8_t header[6];
    if (fread(header, 1, 6, f) != 6) {
        ESP_LOGE("DRAW", "Header read failed!");
        fclose(f);
        return;
    }

    // Skontroluj marker
    if (header[0] != 0x00 || header[1] != 0x01) {
        ESP_LOGE("DRAW", "Invalid bitmap.c format!");
        fclose(f);
        return;
    }

    UWORD img_width  = header[2] | (header[3] << 8);
    UWORD img_height = header[4] | (header[5] << 8);
    ESP_LOGI("DRAW", "Bitmap size: %dx%d", img_width, img_height);

    int out_row_bytes = (img_width + 7) / 8;
    int total_bytes   = out_row_bytes * img_height;

    // Alokuj buffer pre celý obrázok (hlavička + dáta)
    uint8_t *img_buf = malloc(6 + total_bytes);
    if (!img_buf) {
        ESP_LOGE("DRAW", "malloc failed!");
        fclose(f);
        return;
    }

    // Skopíruj hlavičku do bufferu
    memcpy(img_buf, header, 6);

    // Čítaj dáta
    size_t read_bytes = fread(img_buf + 6, 1, total_bytes, f);
    fclose(f);

    if (read_bytes != total_bytes) {
        ESP_LOGE("DRAW", "Data read failed! Got %d, expected %d", read_bytes, total_bytes);
        free(img_buf);
        return;
    }

    // Vyčisti obrazovku a nakresli bitmapu
    Paint_Clear(WHITE);
    Paint_DrawBitmap_universal(img_buf, WHITE, ROTATE_270);

    free(img_buf);

    // Aktualizuj displej a uspaj ho
    display_update();
    

    ESP_LOGI("DRAW", "Bitmap drawn.");
}


