idf_component_register(SRCS "server.c" 
                       INCLUDE_DIRS "."
                        REQUIRES epd2in66 web_site display_wraper esp_http_server esp_wifi mdns)