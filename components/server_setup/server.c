
#include "server.h"


esp_err_t clear_white_handler(httpd_req_t *req)
{
    Paint_Clear(WHITE);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t clear_black_handler(httpd_req_t *req)
{
    Paint_Clear(BLACK);
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
    // Zobrazí čo je aktuálne v image_buffer — bez načítania BMP
    xTaskCreate(display_show_task, "show_task", 4096, NULL, 5, NULL);
    httpd_resp_sendstr(req, "Zobrazujem buffer...");
    return ESP_OK;
}


esp_err_t sleep_handler(httpd_req_t *req)
{
    display_sleep();
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t load_bmp_handler(httpd_req_t *req)
{
    bmp_to_c_array();  // BMP → bitmap.c

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

    Paint_Clear(WHITE);
    Paint_DrawBitmap_universal(img_buf, WHITE, ROTATE_270);
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

    // Načítaj celý obsah do bufferu po častiach a hľadaj začiatok BMP (0x42 0x4D = "BM")
    FILE *f = NULL;
    int remaining = req->content_len;
    bool bmp_found = false;

    // Temp buffer pre hľadanie BMP hlavičky
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

    // Hľadaj "BM" marker v prijatých dátach
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

    // Zapíš zvyšok z prvého čítania (od BMP offsetu)
    fwrite(search_buf + bmp_offset, 1, total_received - bmp_offset, f);
    free(search_buf);

    // Pokračuj so zvyškom dát
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

httpd_uri_t draw_bitmap_uri = {
    .uri = "/draw_bitmap",
    .method = HTTP_POST,
    .handler = draw_bitmap_post_handler,
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


httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192; 
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

    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI("wifi", "WiFi AP started. SSID:%s", WIFI_SSID);
}