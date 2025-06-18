PLUGIN_NAME := "calc"
TEST_CONFIG := join(source_directory(),"test/config.rasi")

default: build

build: 
    meson setup build
    meson compile -C build

build-gcc:
    CC=gcc meson setup build-gcc
    meson compile -C build-gcc

build-clang:
    CC=clang meson setup build-clang
    meson compile -C build-clang

run: build
    ROFI_PLUGIN_PATH="build/src" rofi -modes {{ PLUGIN_NAME }},drun,run -show {{ PLUGIN_NAME }} -config {{ TEST_CONFIG }}
