#include "DEV_Config.h"
#include "EPD_2in66.h"
#include <stdlib.h>
#include "GUI_Paint.h"
#include "GUI_BMPfile.h"
#include "fonts.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
extern UBYTE * image_buffer; 



void display_init();
void display_update();
void display_sleep();
UBYTE* get_image_buffer(void);
int get_image_buffer_size(void);
void display_show(void);
void bmp_to_c_array();
void rotate_buffer_90(void);
void save_original_buffer(void);
void save_undo(void);
void undo_last(void);
