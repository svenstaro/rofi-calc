project(
  'rofi-calc',
  'c',
  version: '2.3.1',
  license: 'MIT',
  default_options: ['warning_level=3'],
)

rofi = dependency('rofi', version: '>=1.5.4')
deps = [
  rofi,
  dependency('glib-2.0', version: '>=2.40'),
  dependency('gmodule-2.0'),
  dependency('cairo'),
]

add_project_arguments(
  '-Winline',
  '-Wunreachable-code',
  language: 'c',
)

subdir('src')
