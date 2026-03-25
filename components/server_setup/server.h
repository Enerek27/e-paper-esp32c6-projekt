
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mdns.h"

#include "esp_mac.h"
#include "website.h"
#include <esp_http_server.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "dis_wraper.h"




#define WIFI_SSID      "E-vizitka"
#define WIFI_PASS      "evizitka1"
#define WIFI_CHANNEL 1
#define MAX_STA_CONN 4



esp_err_t clear_white_handler(httpd_req_t *req);
esp_err_t clear_black_handler(httpd_req_t *req);
esp_err_t root_get_handler(httpd_req_t *req);
esp_err_t draw_bitmap_post_handler(httpd_req_t *req);
esp_err_t upload_handler(httpd_req_t *req);
esp_err_t get_display_handler(httpd_req_t *req);
esp_err_t get_display_handler(httpd_req_t *req);
httpd_handle_t start_webserver(void);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_ap(void);
void display_show_task(void *pvParameters);
esp_err_t sleep_handler(httpd_req_t *req);
esp_err_t load_bmp_handler(httpd_req_t *req);
esp_err_t draw_shape_handler(httpd_req_t *req);