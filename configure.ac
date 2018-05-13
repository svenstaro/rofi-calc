AC_INIT([calc], [0.1.0], [https://github.com/svenstaro/rofi-calc],[],[https://github.com/svenstaro/rofi-calc/issues])

AC_CONFIG_HEADER([config.h])


AC_CONFIG_MACRO_DIRS([m4])
dnl ---------------------------------------------------------------------
dnl Setup automake to be silent and in foreign mode.
dnl We want xz distribution
dnl ---------------------------------------------------------------------
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects dist-xz])
AM_SILENT_RULES([no])

dnl ---------------------------------------------------------------------
dnl  C99 standard
dnl ---------------------------------------------------------------------
AC_PROG_CC_C99

dnl ---------------------------------------------------------------------
dnl C to Object rules.
dnl ---------------------------------------------------------------------
AM_PROG_CC_C_O

dnl ---------------------------------------------------------------------
dnl System extensions
dnl ---------------------------------------------------------------------
AC_USE_SYSTEM_EXTENSIONS

dnl ---------------------------------------------------------------------
dnl Static libraries programs
dnl ---------------------------------------------------------------------
AM_PROG_AR

dnl ---------------------------------------------------------------------
dnl Base CFLAGS
dnl ---------------------------------------------------------------------
AM_CFLAGS="-Wall -Wextra -Wparentheses -Winline -pedantic -Wunreachable-code"


dnl ---------------------------------------------------------------------
dnl Check dependencies
dnl ---------------------------------------------------------------------
PKG_PROG_PKG_CONFIG


dnl ---------------------------------------------------------------------
dnl PKG_CONFIG based dependencies
dnl ---------------------------------------------------------------------
PKG_CHECK_MODULES([glib],     [glib-2.0 >= 2.40 gio-unix-2.0 gmodule-2.0 ])
PKG_CHECK_MODULES([rofi],     [rofi])

LT_INIT([disable-static])

dnl ---------------------------------------------------------------------
dnl Add extra compiler flags
dnl ---------------------------------------------------------------------
AC_SUBST([AM_CFLAGS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
