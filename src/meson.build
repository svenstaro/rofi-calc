calc_sources = ['calc.c']

# Get the rofi plugin directory from pkg-config
rofi_plugins_dir = rofi_dep.get_variable('pluginsdir')

shared_module(
  'calc',
  calc_sources,
  dependencies: [rofi_dep, glib_dep, gio_dep, gmodule_dep, cairo_dep],
  install: true,
  install_dir: rofi_plugins_dir,
)
