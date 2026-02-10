# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

#idf_component_register(SRCS ${app_sources}
#                       INCLUDE_DIRS "../include")

idf_component_register(SRCS main.c demo.c lcd.c touch.c first_screen.c images/clock_screen.c images/timer.c
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd driver esp_timer
                    )
