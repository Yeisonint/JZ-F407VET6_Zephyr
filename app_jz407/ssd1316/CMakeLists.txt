cmake_minimum_required(VERSION 3.20.0)
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/drivers)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ssd1316)

include(ExternalProject)
include(CMakePrintHelpers)

zephyr_get_system_include_directories_for_lang_as_string(C ext_system_includes)
zephyr_get_include_directories_for_lang_as_string(C ext_includes)
zephyr_get_compile_definitions_for_lang_as_string(C ext_defs)
zephyr_get_compile_options_for_lang_as_String(C ext_opts)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

target_sources(app PRIVATE src/main.c src/droid_sans_mono_8.c)
target_include_directories(app PRIVATE drivers/display)