#include "dis_wraper.h"
#include "DEV_Config.h"


UBYTE * image_buffer = NULL;
extern PAINT Paint;


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
    FILE *f = fopen("/spiffs/bitmap.bmp", "rb");
    if (!f) {
        printf("BMP not found\n");
        return;
    }

    
    uint8_t bmp_header[54];
    if (fread(bmp_header, 1, 54, f) != 54) {
        printf("BMP header read failed\n");
        fclose(f);
        return;
    }

    
    int32_t bmp_width  = bmp_header[18] | (bmp_header[19] << 8) |
                         (bmp_header[20] << 16) | (bmp_header[21] << 24);
    int32_t bmp_height = bmp_header[22] | (bmp_header[23] << 8) |
                         (bmp_header[24] << 16) | (bmp_header[25] << 24);
    
    if (bmp_height < 0) bmp_height = -bmp_height;

    printf("BMP size: %ldx%ld\n", bmp_width, bmp_height);

    
    int bmp_row_bytes = ((bmp_width + 31) / 32) * 4;  
   
    uint8_t bpp = bmp_header[28]; // bits per pixel
    if (bpp == 24) bmp_row_bytes = ((bmp_width * 3 + 3) / 4) * 4;
    else if (bpp == 8) bmp_row_bytes = ((bmp_width + 3) / 4) * 4;
    else if (bpp == 1) bmp_row_bytes = ((bmp_width + 31) / 32) * 4;

    
    long pixel_offset = bmp_header[10] | (bmp_header[11] << 8);
    fseek(f, pixel_offset, SEEK_SET);

   
    int out_row_bytes = (bmp_width + 7) / 8;
    int total_bytes   = out_row_bytes * bmp_height;

    uint8_t *out_buf = malloc(total_bytes);
    if (!out_buf) {
        printf("malloc failed\n");
        fclose(f);
        return;
    }
    memset(out_buf, 0xFF, total_bytes); 

    uint8_t *row_buf = malloc(bmp_row_bytes);
    if (!row_buf) {
        free(out_buf);
        fclose(f);
        return;
    }

    
    for (int y = bmp_height - 1; y >= 0; y--) {
        fread(row_buf, 1, bmp_row_bytes, f);
        int out_y = (bmp_height - 1 - y); 

        for (int x = 0; x < bmp_width; x++) {
            uint8_t pixel_black = 0;

            if (bpp == 24) {
                uint8_t b = row_buf[x * 3 + 0];
                uint8_t g = row_buf[x * 3 + 1];
                uint8_t r = row_buf[x * 3 + 2];
                uint8_t gray = (r * 77 + g * 150 + b * 29) >> 8;
                pixel_black = (gray < 128) ? 1 : 0;
            } else if (bpp == 1) {
                uint8_t byte = row_buf[x / 8];
                pixel_black = !((byte >> (7 - (x % 8))) & 1);
            } else if (bpp == 8) {
                pixel_black = (row_buf[x] < 128) ? 1 : 0;
            }

            if (pixel_black) {
                
                int byte_idx = out_y * out_row_bytes + (x / 8);
                out_buf[byte_idx] &= ~(0x80 >> (x % 8));
            }
        }
    }

    free(row_buf);
    fclose(f);

    
    FILE *out = fopen("/spiffs/bitmap.c", "wb");
    if (!out) {
        printf("Cannot create C file\n");
        free(out_buf);
        return;
    }

    
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




static uint8_t *original_buffer = NULL;
static int original_buf_size = 0;
void save_original_buffer(void)
{
    int buf_size = Paint.WidthByte * Paint.HeightMemory;
    if (original_buffer) {
        free(original_buffer);
    }
    original_buffer = malloc(buf_size);
    if (!original_buffer) {
        ESP_LOGE("DRAW", "save original malloc failed");
        return;
    }
    memcpy(original_buffer, image_buffer, buf_size);
    original_buf_size = buf_size;
    ESP_LOGI("DRAW", "Original buffer saved");
}




void rotate_buffer_90(void)
{
    save_undo();
    if (!original_buffer) {
        ESP_LOGE("DRAW", "No original buffer saved!");
        return;
    }

    static int rotation_count = 0;
    rotation_count = (rotation_count + 1) % 4;

    int w = Paint.WidthMemory;   
    int h = Paint.HeightMemory;  
    int bpr = Paint.WidthByte;   
    int buf_size = bpr * h;

    uint8_t *tmp = malloc(buf_size);
    if (!tmp) {
        ESP_LOGE("DRAW", "rotate malloc failed");
        return;
    }
    memset(tmp, 0xFF, buf_size);

   
    int rotated_w = (rotation_count == 1 || rotation_count == 3) ? h : w;
    int rotated_h = (rotation_count == 1 || rotation_count == 3) ? w : h;

   
    int x_offset = (w - rotated_w) / 2;
    int y_offset = (h - rotated_h) / 2;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int src_byte = x / 8 + y * bpr;
            int src_bit  = 7 - (x % 8);
            int pixel = (original_buffer[src_byte] >> src_bit) & 1;

            if (pixel == 1) continue;

            int nx = x;
            int ny = y;

            switch (rotation_count) {
                case 1: 
                    nx = h - 1 - y;
                    ny = x;
                    break;
                case 2: 
                    nx = w - 1 - x;
                    ny = h - 1 - y;
                    break;
                case 3: 
                    nx = y;
                    ny = w - 1 - x;
                    break;
                default:
                    nx = x;
                    ny = y;
                    break;
            }

          
            nx += x_offset;
            ny += y_offset;

            if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

            int dst_byte = nx / 8 + ny * bpr;
            int dst_bit  = 7 - (nx % 8);
            tmp[dst_byte] &= ~(1 << dst_bit);
        }
    }

    memcpy(image_buffer, tmp, buf_size);
    free(tmp);

    ESP_LOGI("DRAW", "Buffer rotated %d x 90 from original", rotation_count);
}


static uint8_t *undo_buffer = NULL;

void save_undo(void)
{
    int buf_size = Paint.WidthByte * Paint.HeightMemory;
    if (!undo_buffer) {
        undo_buffer = malloc(buf_size);
    }
    if (undo_buffer) {
        memcpy(undo_buffer, image_buffer, buf_size);
    }
}

void undo_last(void)
{
    if (!undo_buffer) {
        ESP_LOGE("DRAW", "No undo buffer!");
        return;
    }
    int buf_size = Paint.WidthByte * Paint.HeightMemory;
    memcpy(image_buffer, undo_buffer, buf_size);
    save_original_buffer();
}