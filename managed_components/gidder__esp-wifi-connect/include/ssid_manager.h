#ifndef SSID_MANAGER_H
#define SSID_MANAGER_H

#include <string>
#include <vector>
#include <esp_wifi_types_generic.h>

struct SsidItem {
    std::string ssid;
    std::string password;
    wifi_auth_mode_t authmode;
};

class SsidManager {
public:
    static SsidManager& GetInstance() {
        static SsidManager instance;
        return instance;
    }

    void AddSsid(const std::string& ssid, const std::string& password, wifi_auth_mode_t authmode);
    void RemoveSsid(int index);
    void SetDefaultSsid(int index);
    void Clear();
    const std::vector<SsidItem>& GetSsidList() const { return ssid_list_; }

private:
    SsidManager();
    ~SsidManager();

    void LoadFromNvs();
    void SaveToNvs();

    std::vector<SsidItem> ssid_list_;
};

#endif // SSID_MANAGER_H
