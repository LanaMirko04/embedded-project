# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

#idf_component_register(SRCS ${app_sources}
#                       INCLUDE_DIRS "../include")

idf_component_register(
    SRCS
        "main.c"
        "ui_1/ui_1.c"
        "ui_1/ui_1_gen.c"
        "ui_1/fonts/Poppins_medium_48.c"
    INCLUDE_DIRS
        "."
        "ui_1"
)
