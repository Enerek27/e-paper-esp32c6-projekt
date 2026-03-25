# ESP32 WiFi Connect

This is fork of [78/esp-wifi-connect][origin] with focus mainly on Captive Portal (removed the new OTA URL)

This component helps with WiFi connection for the device.

It first tries to connect to a WiFi network using the credentials stored in the flash. If this fails, it starts an access point and a web server to allow the user to connect to a WiFi network.

Here is a screenshot of the web server:

![Access Point Configuration][image]

## Requirements

- ESP IDF 5.3+ (Recommended ESP IDF 5.5+)

## Configuration

Add dependency:  
`idf.py add-dependency "gidder/esp-wifi-connect"`  

Configure easy import by adding following line at the end of file `CMakeLists.txt`:  
`set(COMPONENT_PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES} esp-wifi-connect)`

In case of usage of ESP IDF older than 5.5, set CONFIG_HTTPD_MAX_REQ_HDR_LEN to 1024 in sdkconfig:  
`CONFIG_HTTPD_MAX_REQ_HDR_LEN=1024`  
It resolves issue with "Header Fields are too long for the server to interpret" showed on phones.  
More info about issue: [Add Documentation for "Header Fields are too long for the server to interpret" error][header_length].

## Usage

```cpp
#include <esp_log.h>
#include <nvs_flash.h>

#include <wifi_station.h>
#include <wifi_configuration_ap.h>
#include <ssid_manager.h>

#define TAG "main"

extern "C" void app_main(void)
{
    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Get the WiFi configuration
    auto& ssid_list = SsidManager::GetInstance().GetSsidList();
    if (ssid_list.empty()) {
        // Start the WiFi configuration AP
        auto& ap = WifiConfigurationAp::GetInstance();
        ap.Start();
        return;
    }

    // Otherwise, connect to the WiFi network
    WifiStation::GetInstance().Start();

    // Device is connected to AP
    while (true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        int min_free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        ESP_LOGI(TAG, "Free internal: %u minimal internal: %u", free_sram, min_free_sram);
    }
}
```

## Changelog

Changelog at [CHANGELOG.md][changelog]

[origin]: https://github.com/78/esp-wifi-connect
[changelog]: CHANGELOG.md
[image]: assets/ap_list.png
[header_length]: https://github.com/tonyp7/esp32-wifi-manager/issues/147