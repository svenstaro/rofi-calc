# rofi-calc

**🖩 Do calculations in rofi!**

[![Build Status](https://travis-ci.com/svenstaro/rofi-calc.svg?branch=master)](https://travis-ci.com/svenstaro/rofi-calc)
[![AUR](https://img.shields.io/aur/version/rofi-calc.svg)](https://aur.archlinux.org/packages/rofi-calc/)
[![license](http://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/svenstaro/rofi-calc/blob/master/LICENSE)

![](demo.gif)

A [rofi](https://github.com/DaveDavenport/rofi) plugin that uses libqalculate's `qalc` to parse natural language input and provide results.

Run rofi like:

```bash
rofi -show calc -modi calc -no-show-match -no-sort
```

## Compilation

### Dependencies

You need rofi version at least 1.5 as well as qalculate.

### Installation

#### On Arch Linux

[rofi-calc in AUR](https://aur.archlinux.org/packages/rofi-calc/)

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
