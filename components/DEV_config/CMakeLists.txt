idf_component_register(SRCS "DEV_Config.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver GUI esp_driver_spi esp_driver_gpio)