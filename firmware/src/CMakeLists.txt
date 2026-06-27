#This file was automatically generated for projects
#without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources "*.cpp" "*.c")
message(app_sources="${app_sources}")

idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS "../include"
                       PRIV_REQUIRES src esp_wifi wpa_supplicant nvs_flash spi_flash esp_lcd driver esp_timer
                                     esp_http_client espressif__cjson esp_driver_ledc)
