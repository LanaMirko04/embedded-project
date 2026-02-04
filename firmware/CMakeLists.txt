cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(firmware)

idf_build_set_property(COMPILE_OPTIONS -DLV_CONF_PATH="${CMAKE_SOURCE_DIR}/main/lv_conf.h" APPEND)
