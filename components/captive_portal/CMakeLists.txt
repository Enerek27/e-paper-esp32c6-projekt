idf_component_register(
    SRCS
        "src/dns_server.cc"
    INCLUDE_DIRS
        "include"
    REQUIRES
        esp_wifi
        esp_netif
        freertos
        lwip
        
        
)