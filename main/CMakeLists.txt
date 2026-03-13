idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES nvs_flash 
                    REQUIRES DEV_config epd2in66 GUI Fonts esp_driver_gpio  web_site server_setup spiff_my display_wraper)