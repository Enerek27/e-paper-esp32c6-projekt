

#include "spiff_my.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <string.h>

void init_spiffs()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE("SPIFFS", "Mount failed: %s", esp_err_to_name(ret));
        return;
    }
    
    
    size_t total = 0, used = 0;
    esp_spiffs_info(NULL, &total, &used);
    ESP_LOGI("SPIFFS", "Mounted OK. Total: %d, Used: %d", total, used);
    
    esp_err_t check = esp_spiffs_check(NULL);
    if (check == ESP_FAIL) {
        ESP_LOGE("SPIFFS","Spiffs check went wrong");
    } else if (check == ESP_ERR_INVALID_STATE) {
        ESP_LOGE("SPIFFS", "Spiffs state is invalid");
    }
}