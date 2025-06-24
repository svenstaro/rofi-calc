PLUGIN_NAME := "calc"
TEST_CONFIG := join(source_directory(),"test/config.rasi")

default: build

build: 
    meson setup build
    meson compile -C build

run: build
    ROFI_PLUGIN_PATH="build/src" rofi -modes {{ PLUGIN_NAME }},drun -show {{ PLUGIN_NAME }} -config {{ TEST_CONFIG }}

clean:
    rm build -r
