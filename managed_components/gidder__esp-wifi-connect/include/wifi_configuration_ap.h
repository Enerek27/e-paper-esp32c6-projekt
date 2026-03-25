#ifndef _WIFI_CONFIGURATION_AP_H_
#define _WIFI_CONFIGURATION_AP_H_

#include <string>
#include <vector>
#include <mutex>
#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_timer.h>
#include "dns_server.h"
#include <esp_netif.h>
#include <esp_wifi_types_generic.h>


/**
  * @brief Minitmal description of a Wi-Fi AP
  */
 typedef struct wifi_ap_record_min_t {
    uint8_t ssid[33];                     /**< SSID of AP */
    int8_t  rssi;                         /**< Signal strength of AP. Note that in some rare cases where signal strength is very strong, RSSI values can be slightly positive */
    wifi_auth_mode_t authmode;            /**< Auth mode of AP */

    wifi_ap_record_min_t(wifi_ap_record_t& other)
        : rssi(other.rssi), authmode(other.authmode)
    {
        strlcpy((char *)ssid, (const char *)other.ssid, 32);
    }
} wifi_ap_record_min_t;

typedef struct 
{
    bool operator()(const wifi_ap_record_min_t& lhs, const wifi_ap_record_min_t& rhs) const
    {
        return lhs.rssi > rhs.rssi;
    }
} wifi_ap_record_min_cmp;
 

class WifiConfigurationAp {
public:
    static WifiConfigurationAp& GetInstance();
    void SetSsidPrefix(const std::string &&ssid_prefix);
    void Start();
    void Stop();
    void StartSmartConfig();

    std::string GetSsid();
    std::string GetWebServerUrl();

    // Delete copy constructor and assignment operator
    WifiConfigurationAp(const WifiConfigurationAp&) = delete;
    WifiConfigurationAp& operator=(const WifiConfigurationAp&) = delete;

private:
    // Private constructor
    WifiConfigurationAp();
    ~WifiConfigurationAp();

    std::mutex mutex_aps_;
    DnsServer dns_server_;
    httpd_handle_t server_ = NULL;
    EventGroupHandle_t event_group_;
    std::string ssid_prefix_;
    esp_event_handler_instance_t instance_any_id_;
    esp_event_handler_instance_t instance_got_ip_;
    esp_timer_handle_t scan_timer_ = nullptr;
    bool is_connecting_ = false;
    esp_netif_t* ap_netif_ = nullptr;
    std::vector<wifi_ap_record_min_t> ap_records_;

    void StartAccessPoint();
    void StartWebServer();
    bool ConnectToWifi(const std::string &ssid, const std::string &password, wifi_auth_mode_t authmode);
    void Save(const std::string &ssid, const std::string &password, wifi_auth_mode_t authmode);

    // Event handlers
    static void WifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void IpEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void SmartConfigEventHandler(void* arg, esp_event_base_t event_base, 
                                      int32_t event_id, void* event_data);
    esp_event_handler_instance_t sc_event_instance_ = nullptr;
};

#endif // _WIFI_CONFIGURATION_AP_H_
