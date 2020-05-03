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

The result of the current input can be selected with `Ctrl+Enter`, and history entries can be selected with `Enter`. By default this will just output the equation/result.

## Installation

### Via package manager

* [Arch Linux](https://www.archlinux.org/packages/community/x86_64/rofi-calc/)
* [FreeBSD](https://www.freshports.org/x11/rofi-calc/)
* [Gentoo](https://packages.gentoo.org/packages/x11-misc/rofi-calc)
* [openSUSE](https://software.opensuse.org/package/rofi-calc)

### From source

You need a C compilation toolchain (a `cc`, `autoconf`, `pkg-config`, ...), `rofi` (version >= 1.5) as well as `libqalculate` (version > 2.0).

You will also need development headers for `rofi` and `libqalculate`. Depending on your distribution these may be included in different packages:

* Arch, Gentoo: included with `rofi`, `libqalculate`
* OpenSUSE: `zypper in rofi rofi-devel qalculate`
* Debian: `dpkg --install rofi-dev qalc libqalculate-dev`
* Ubuntu: `apt install rofi-dev qalc libqalculate-dev`
* Solus: `eopkg it rofi-devel libqalculate-devel`
* CentOS, Fedora: Install `qalculate` `libqalculate-devel` (find `rofi-devel` headers yourself)
* Others: look it up :)

Some distributions ship an [extremely outdated](https://github.com/svenstaro/rofi-calc/issues/7) version of `libqalculate` so you might have to compile your own. If that is the case, see [here](https://github.com/svenstaro/rofi-calc/wiki/Installing-libqalculate-from-source).

**rofi-calc** uses autotools as build system. If installing from git, the following steps should install it:

```bash
$ autoreconf -i
$ mkdir build
$ cd build/
$ ../configure
$ make
$ make install
```

## Advanced Usage

Use the `-qalc-binary` option to specify the name or location of qalculate's `qalc` binary. Defaults to `qalc`.

Use the `-terse` option to reduce the output of `qalc` to just the result of the input expression.

Use the `-calc-command` option to specify a shell command to execute which will be interpolated with the following keys:

* `{expression}`: the left-side of the equation (currently not available when using `-terse`)
* `{result}`: the right of the equation

The following example copies the result to the clipboard upon pressing the key combination defined by `-kb-accept-custom`
(by default Control+Return).
NOTE: `{result}` should be quoted since it may contain characters that your shell would otherwise interpret:

    rofi -show calc -modi calc -no-show-match -no-sort -calc-command "echo -n '{result}' | xclip"

It's convenient to bind it to a key combination in i3. For instance, you could use:

    bindsym $mod+c exec --no-startup-id "rofi -show calc -modi calc -no-show-match -no-sort"

To disable the bold font applied to the results by default, you can use the flag `-no-bold` and run rofi like:

    rofi -show calc -modi calc -no-show-match -no-sort -no-bold

To disable the history, use `-no-history`:

    rofi -show calc -modi calc -no-show-match -no-sort -no-history

To enable thousand separators in the output (e.g. `5 * 12 = 6,000`, rather than `6000`) add the following to `~/.config/qalculate/qalc.cfg`

> For `,` separator:

    digit_grouping=2

> For space separator:

    digit_grouping=1

To use a different output format for numeric representations (for instance, some locales use `,` instead of `.` as a decimal separator),
set `LC_NUMERIC` to a different value like this:

    LC_NUMERIC=de_DE.UTF-8 rofi -show calc -modi calc -no-show-match -no-sort

Make sure the locale is actually available on your system!

## Development

If you're developing this, it might be helpful to start rofi directly with a locally compiled plugin like this:

    rofi -plugin-path build/.libs -show calc -modi calc -no-show-match -no-sort
