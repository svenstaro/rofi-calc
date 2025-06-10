project('rofi-calc', 'c', version: '0.1.0', license: 'MIT')

rofi_dep = dependency('rofi', version: '>=1.5.4')
glib_dep = dependency('glib-2.0', version: '>=2.40')
gio_dep = dependency('gio-unix-2.0')
gmodule_dep = dependency('gmodule-2.0')
cairo_dep = dependency('cairo')

add_project_arguments(
  '-Wparentheses',
  '-Winline',
  '-pedantic',
  '-Wunreachable-code',
  language: 'c',
)

subdir('src')
