
#include "server.h"
#include "wake_up.h"
#include "website.h"
#include <stdbool.h>


static esp_err_t captive_redirect_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}




esp_err_t draw_shape_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    char *buf = malloc(total_len + 1);
    if (!buf) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OOM");
        return ESP_FAIL;
    }

    int received = httpd_req_recv(req, buf, total_len);
    if (received <= 0) {
        free(buf);
        return ESP_FAIL;
    }
    buf[received] = '\0';

    char type[32] = {0};
    char text[64] = {0};
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    int radius = 0;
    bool color = 0;
    int width = 1;
    int style = 0;
    int filled = 0;
    int font_size = 16;
    int number = 0;
    int digit = 0;

    char *p;

    p = strstr(buf, "type=");
    if (p) {
        p += 5;
        char *e = strchr(p, '&');
        int l = e ? (e - p) : strlen(p);
        if (l > 31) l = 31;
        strncpy(type, p, l);
    }

    p = strstr(buf, "text=");
    if (p) {
        p += 5;
        char *e = strchr(p, '&');
        int l = e ? (e - p) : strlen(p);
        if (l > 63) l = 63;
        strncpy(text, p, l);
    }
    for (int i = 0; text[i]; i++) {
        if (text[i] == '+') text[i] = ' ';
    }

    p = strstr(buf, "x1=");
    if (p) x1 = atoi(p + 3);

    p = strstr(buf, "y1=");
    if (p) y1 = atoi(p + 3);

    p = strstr(buf, "x2=");
    if (p) x2 = atoi(p + 3);

    p = strstr(buf, "y2=");
    if (p) y2 = atoi(p + 3);

    p = strstr(buf, "radius=");
    if (p) radius = atoi(p + 7);

    p = strstr(buf, "color=");
    if (p) color = (bool)atoi(p + 6);

    p = strstr(buf, "width=");
    if (p) width = atoi(p + 6);

    p = strstr(buf, "style=");
    if (p) style = atoi(p + 6);

    p = strstr(buf, "filled=");
    if (p) filled = atoi(p + 7);

    p = strstr(buf, "font=");
    if (p) font_size = atoi(p + 5);

    p = strstr(buf, "number=");
    if (p) number = atoi(p + 7);

    p = strstr(buf, "digit=");
    if (p) digit = atoi(p + 6);

    free(buf);

    UWORD c = color ? WHITE : BLACK;

    sFONT *font = &Font16;
    if (font_size == 8){       
        font = &Font8;
    } else if (font_size == 12){
        font = &Font12;
    } else if (font_size == 20) {
        font = &Font20;
    } else if (font_size == 24) {
        font = &Font24;
    }
    save_undo();
    if (strcmp(type, "line") == 0) {
        Paint_DrawLine(x1, y1, x2, y2, c, width, style);
    } else if (strcmp(type, "rect") == 0) {
        Paint_DrawRectangle(x1, y1, x2, y2, c, width, filled);
    } else if (strcmp(type, "circle") == 0) {
        Paint_DrawCircle(x1, y1, radius, c, width, filled);
    } else if (strcmp(type, "string") == 0) {
        Paint_DrawString_EN(x1, y1, text, font, c, c ? BLACK : WHITE);
    } else if (strcmp(type, "number") == 0) {
        Paint_DrawNum(x1, y1, number, font, c, c ? BLACK : WHITE);
    } else if (strcmp(type, "decimal") == 0) {
        Paint_DrawNumDecimals(x1, y1, (double)number / 100.0, font, digit, c, c ? BLACK : WHITE);
    }

    httpd_resp_sendstr(req, "OK"); 
    save_original_buffer();
    return ESP_OK; 
}




esp_err_t clear_white_handler(httpd_req_t *req)
{
    save_undo();
    Paint_Clear(WHITE);
    save_original_buffer();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t clear_black_handler(httpd_req_t *req)
{
    save_undo();
    Paint_Clear(BLACK);
    save_original_buffer();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t undo_handler(httpd_req_t *req)
{
    undo_last();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t get_display_handler(httpd_req_t *req)
{
    UBYTE *buf = get_image_buffer();
    int size = get_image_buffer_size();
    if (!buf || size <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Buffer not ready");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Expose-Headers", "X-Width,X-Height");
    httpd_resp_send(req, (char*)buf, size);
    return ESP_OK;
}


esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_send(req, webpage, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t draw_bitmap_post_handler(httpd_req_t *req)
{

    xTaskCreate(display_show_task, "show_task", 4096, NULL, 5, NULL);
    httpd_resp_sendstr(req, "Zobrazujem buffer...");
    return ESP_OK;
}

esp_err_t rotate_buffer_handler(httpd_req_t *req)
{
    rotate_buffer_90();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t sleep_handler(httpd_req_t *req)
{
    display_sleep();
    httpd_resp_sendstr(req, "OK");
    go_to_sleep();
    return ESP_OK;
}

esp_err_t load_bmp_handler(httpd_req_t *req)
{
    bmp_to_c_array();  

    FILE *f = fopen("/spiffs/bitmap.c", "rb");
    if (!f) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "bitmap.c not found");
        return ESP_FAIL;
    }

    uint8_t header[6];
    if (fread(header, 1, 6, f) != 6 || header[0] != 0x00 || header[1] != 0x01) {
        fclose(f);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid format");
        return ESP_FAIL;
    }

    UWORD img_width  = header[2] | (header[3] << 8);
    UWORD img_height = header[4] | (header[5] << 8);

    int out_row_bytes = (img_width + 7) / 8;
    int total_bytes   = out_row_bytes * img_height;

    uint8_t *img_buf = malloc(6 + total_bytes);
    if (!img_buf) {
        fclose(f);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "malloc failed");
        return ESP_FAIL;
    }

    memcpy(img_buf, header, 6);
    fread(img_buf + 6, 1, total_bytes, f);
    fclose(f);

    Paint_DrawBitmap_universal(img_buf, WHITE, ROTATE_270);
    save_original_buffer();
    free(img_buf);

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}


esp_err_t upload_handler(httpd_req_t *req)
{
    char *buf = malloc(4000);
    if (!buf) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }

    
    FILE *f = NULL;
    int remaining = req->content_len;
    uint8_t *search_buf = malloc(remaining < 8192 ? remaining : 8192);
    if (!search_buf) {
        free(buf);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }

    int total_received = 0;
    int to_read = remaining < 8192 ? remaining : 8192;
    int received = httpd_req_recv(req, (char*)search_buf, to_read);
    if (received <= 0) {
        free(buf);
        free(search_buf);
        return ESP_FAIL;
    }
    total_received = received;
    remaining -= received;

    
    int bmp_offset = -1;
    for (int i = 0; i < total_received - 1; i++) {
        if (search_buf[i] == 0x42 && search_buf[i+1] == 0x4D) {
            bmp_offset = i;
            break;
        }
    }

    if (bmp_offset < 0) {
        ESP_LOGE("UPLOAD", "BMP marker not found!");
        free(buf);
        free(search_buf);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "BMP not found");
        return ESP_FAIL;
    }

    ESP_LOGI("UPLOAD", "BMP starts at offset %d", bmp_offset);

    f = fopen("/spiffs/bitmap.bmp", "wb");
    if (!f) {
        free(buf);
        free(search_buf);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Cannot open file");
        return ESP_FAIL;
    }

    
    fwrite(search_buf + bmp_offset, 1, total_received - bmp_offset, f);
    free(search_buf);

   
    while (remaining > 0) {
        int chunk = remaining < 4000 ? remaining : 4000;
        received = httpd_req_recv(req, buf, chunk);
        if (received <= 0) {
            fclose(f);
            free(buf);
            return ESP_FAIL;
        }
        fwrite(buf, 1, received, f);
        remaining -= received;
    }

    fclose(f);
    free(buf);

    ESP_LOGI("UPLOAD", "Bitmap saved to SPIFFS");
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}
httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL
};

httpd_uri_t upload = {
    .uri = "/upload",
    .method = HTTP_POST,
    .handler = upload_handler,
    .user_ctx = NULL
};


httpd_uri_t undo_uri = {
    .uri = "/undo", .method = HTTP_POST,
    .handler = undo_handler, .user_ctx = NULL
};

httpd_uri_t rotate_buf_uri = {
    .uri = "/rotate_buffer", 
    .method = HTTP_POST,
    .handler = rotate_buffer_handler, 
    .user_ctx = NULL
};

httpd_uri_t draw_bitmap_uri = {
    .uri = "/draw_bitmap",
    .method = HTTP_POST,
    .handler = draw_bitmap_post_handler,
    .user_ctx = NULL
};

httpd_uri_t draw_shape_uri = {
    .uri = "/draw_shape", 
    .method = HTTP_POST,
    .handler = draw_shape_handler, 
    .user_ctx = NULL
};


httpd_uri_t get_display_uri = {
    .uri = "/get_display",
    .method = HTTP_GET,
    .handler = get_display_handler,
    .user_ctx = NULL
};

httpd_uri_t clear_white_uri = {
    .uri = "/clear_white", .method = HTTP_POST,
    .handler = clear_white_handler, .user_ctx = NULL
};
httpd_uri_t clear_black_uri = {
    .uri = "/clear_black", .method = HTTP_POST,
    .handler = clear_black_handler, .user_ctx = NULL
};

httpd_uri_t sleep_uri = {
    .uri = "/sleep", .method = HTTP_POST,
    .handler = sleep_handler, .user_ctx = NULL
};
httpd_uri_t load_bmp_uri = {
    .uri = "/load_bmp", .method = HTTP_POST,
    .handler = load_bmp_handler, .user_ctx = NULL
};

httpd_uri_t cp1 = { 
    .uri = "/hotspot-detect.html",
    .method = HTTP_GET, 
    .handler = captive_redirect_handler,
    .user_ctx = NULL 
};
httpd_uri_t cp2 = {
    .uri = "/generate_204",            
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp3 = {
    .uri = "/gen_204",                 
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp4 = {
    .uri = "/mobile/status.php",       
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp5 = {
    .uri = "/ncsi.txt",                
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp6 = {
    .uri = "/library/test/success.html", 
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp7 = {
    .uri = "/success.txt",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp8 = {
    .uri = "/portal.html",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp9 = {
    .uri = "/connectivity-check.html",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp10 = {
    .uri = "/fwlink/",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp11 = {
    .uri = "/check_network_status.txt",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp12 = {
    .uri = "/nm-check.txt",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp13 = {
    .uri = "/canonical.html",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp14 = {
    .uri = "/favicon.ico",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp15 = {
    .uri = "/generate204",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp16 = {
    .uri = "/connecttest.txt",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};
httpd_uri_t cp17 = {
    .uri = "/redirect",             
    .method = HTTP_GET, 
    .handler = captive_redirect_handler, 
    .user_ctx = NULL 
};

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192; 
    config.max_uri_handlers = 40;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &upload);
        httpd_register_uri_handler(server, &draw_bitmap_uri);
        httpd_register_uri_handler(server, &get_display_uri);
        httpd_register_uri_handler(server, &clear_white_uri);
        httpd_register_uri_handler(server, &clear_black_uri);
        httpd_register_uri_handler(server, &sleep_uri);
        httpd_register_uri_handler(server, &load_bmp_uri);
        httpd_register_uri_handler(server, &rotate_buf_uri);
        httpd_register_uri_handler(server, &draw_shape_uri);
        httpd_register_uri_handler(server, &undo_uri);
        httpd_register_uri_handler(server, &cp1);
        httpd_register_uri_handler(server, &cp2);
        httpd_register_uri_handler(server, &cp3);
        httpd_register_uri_handler(server, &cp4);
        httpd_register_uri_handler(server, &cp5);
        httpd_register_uri_handler(server, &cp6);
        httpd_register_uri_handler(server, &cp7);
        httpd_register_uri_handler(server, &cp8);
        httpd_register_uri_handler(server, &cp9);
        httpd_register_uri_handler(server, &cp10);
        httpd_register_uri_handler(server, &cp11);
        httpd_register_uri_handler(server, &cp12);
        httpd_register_uri_handler(server, &cp13);
        httpd_register_uri_handler(server, &cp14);
        httpd_register_uri_handler(server, &cp15);
        httpd_register_uri_handler(server, &cp16);
        httpd_register_uri_handler(server, &cp17);
    }
    return server;
}

/* -------------------wifi -------------------------*/


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

    

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI("wifi", "WiFi AP started. SSID:%s", WIFI_SSID);
}