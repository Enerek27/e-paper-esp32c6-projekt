
#include "wake_up.h"
#include "esp_wifi.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "soc/gpio_num.h"

#define WAKEUP_PIN GPIO_NUM_0


static const char *TAG = "WAKEUP";    



void print_wakeup_reason(void){
  

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_causes();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : ESP_LOGI(TAG, "Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : ESP_LOGI(TAG, "Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : ESP_LOGI(TAG, "Wakeup caused by ULP program"); break;
    default : ESP_LOGI(TAG, "Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}



/* ---------- Funkcia na nastavenie tlačidla ---------- */
void init_wake_button(void) {
    //ESP_ERROR_CHECK(esp_sleep_disable_ext1_wakeup_io(WAKEUP_PIN));
    const uint64_t WAKEUP_PIN_MASK = 1ULL << WAKEUP_PIN;
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup_io(WAKEUP_PIN_MASK, ESP_EXT1_WAKEUP_ANY_LOW));
}

void go_to_sleep(void) {
    esp_wifi_stop();
    esp_wifi_deinit();
    vTaskDelay(pdMS_TO_TICKS(3000));
    init_wake_button();
    ESP_LOGI(TAG, "ESP going to sleep");
    esp_deep_sleep_start();
}