calc_sources = ['calc.c']

# Get the rofi plugin directory from pkg-config
rofi_plugins_dir = rofi.get_variable('pluginsdir')

shared_module(
  'calc',
  calc_sources,
  dependencies: deps,
  install: true,
  install_dir: rofi_plugins_dir,
)
