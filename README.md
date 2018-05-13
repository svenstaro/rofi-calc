# rofi-calc plugin

Plugin that uses libqalculate's `qalc` to parse natural language input and provide results.

Run rofi like:

```bash
    rofi -show calc -modi calc -no-show-match -no-sort
```

## Compilation

### Dependencies

| Dependency | Version         |
|------------|-----------------|
| rofi 	     | 1.5 (or git)	   |

### Installation

**rofi-calc** uses autotools as build system. If installing from git, the following steps should install it:

```bash
$ autoreconf -i
$ mkdir build
$ cd build/
$ ../configure
$ make
$ make install
```
