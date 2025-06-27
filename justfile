PLUGIN_NAME := "calc"
TEST_CONFIG := join(source_directory(),"test/config.rasi")

default: build

build:
    meson setup build
    meson compile -C build

run *args: build
    rofi -plugin-path "build/src" -modes {{ PLUGIN_NAME }},drun -show {{ PLUGIN_NAME }} -config {{ TEST_CONFIG }} {{ args }}

clean:
    rm build -r
