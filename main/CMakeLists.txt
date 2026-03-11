idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_wifi nvs_flash esp_http_server
                    REQUIRES DEV_config epd2in66 GUI Fonts esp_driver_gpio  web_site
)