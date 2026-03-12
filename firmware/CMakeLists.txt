cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS src)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(firmware)

idf_build_set_property(COMPILE_OPTIONS -DLV_CONF_PATH="${CMAKE_SOURCE_DIR}/include/lv_conf.h" APPEND)
