# rofi-calc

**🖩 Do live calculations in rofi!**

[![Build Status](https://travis-ci.com/svenstaro/rofi-calc.svg?branch=master)](https://travis-ci.com/svenstaro/rofi-calc)
[![AUR](https://img.shields.io/aur/version/rofi-calc.svg)](https://aur.archlinux.org/packages/rofi-calc/)
[![license](http://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/svenstaro/rofi-calc/blob/master/LICENSE)

A [rofi](https://github.com/DaveDavenport/rofi) plugin that uses libqalculate's `qalc` to parse natural language input and provide results.

Since this uses libqalculate's amazing `qalc` you can try natural language queries such `500 + 25%` or `5000 EUR to USD` or `150 to hex`. It can also solve linear equations on the fly. Try `60x + 30 = 50`, for instance.

![](demo.gif)

Run rofi like:

    rofi -show calc -modi calc -no-show-match -no-sort

It's convenient to bind it to a key combination in i3. For instance, you could use:

    bindsym $mod+c exec --no-startup-id "rofi -show calc -modi calc -no-show-match -no-sort"

## Compilation

### Dependencies

You need a C compilation toolchain (a `cc`, `autoconf`, `pkg-config`, ...), `rofi` (version >= 1.5) as well as `libqalculate` (version > 2.0).

You will also need development headers for `rofi` and `libqalculate`. Depending on your distribution these may be included in different packages:

* Arch, Gentoo: included with `rofi`, `libqalculate`
* OpenSUSE: `zypper in rofi qalculate` 
* Debian: `dpkg --install rofi-dev qalc libqalculate-dev`
* Ubuntu: `apt install rofi-dev qalc libqalculate-dev`
* Solus: `eopkg it rofi-devel libqalculate-devel`
* CentOS, Fedora: Install `qalculate` `libqalculate-devel` (find `rofi-devel` headers yourself)
* Others: look it up :)

Some distributions ship an [extremely outdated](https://github.com/svenstaro/rofi-calc/issues/7) version of `libqalculate` so you might have to compile your own.

### Installation

#### Package Manager

* [Arch AUR](https://aur.archlinux.org/packages/rofi-calc/)
* [OpenSUSE Leap](https://software.opensuse.org/package/rofi-calc)

#### From source

**rofi-calc** uses autotools as build system. If installing from git, the following steps should install it:

```bash
$ autoreconf -i
$ mkdir build
$ cd build/
$ ../configure
$ make
$ make install
```
