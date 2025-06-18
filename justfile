PLUGIN_NAME := "calc"
TEST_CONFIG := join(source_directory(),"test/config.rasi")

build: build-gcc build-clang

build-gcc:
    CC=gcc meson setup build-gcc
    meson compile -C build-gcc

build-clang:
    CC=clang meson setup build-clang
    meson compile -C build-clang

run: build
    ROFI_PLUGIN_PATH="build/src" rofi -modes {{ PLUGIN_NAME }},drun,run -show {{ PLUGIN_NAME }} -config {{ TEST_CONFIG }}
