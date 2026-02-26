cmake_minimum_required(VERSION 3.16.0)
add_compile_options(-std=gnu++14)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(firmware)

idf_build_set_property(COMPILE_OPTIONS "-Ofast -Wall" APPEND)

