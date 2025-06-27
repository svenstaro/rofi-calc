with (import <nixpkgs> { });
mkShell {
  packages = [
    meson
    ninja
    rofi-unwrapped
    pkg-config
    glib
    cairo
    libqalculate

    gcc
    clang

    just
  ];
}
