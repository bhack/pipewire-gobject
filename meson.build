project(
  'pipewire-gobject',
  'c',
  version: '0.1.0',
  meson_version: '>=1.0',
  default_options: ['warning_level=2', 'werror=false'],
)

gnome = import('gnome')
pkg = import('pkgconfig')

glib_dep = dependency('glib-2.0', version: '>=2.66')
gio_dep = dependency('gio-2.0', version: '>=2.66')
gobject_dep = dependency('gobject-2.0', version: '>=2.66')
pipewire_dep = dependency('libpipewire-0.3', version: '>=1.0.2')

pwg_inc = include_directories('src')

pwg_sources = [
  'src/pwg.c',
  'src/pwg-audio-block.c',
  'src/pwg-audio-capture.c',
  'src/pwg-audio-format.c',
  'src/pwg-core.c',
  'src/pwg-error.c',
  'src/pwg-global.c',
  'src/pwg-metadata.c',
  'src/pwg-node-info.c',
  'src/pwg-registry.c',
  'src/pwg-stream.c',
]

pwg_headers = [
  'src/pwg-audio-block.h',
  'src/pwg-audio-capture.h',
  'src/pwg-audio-format.h',
  'src/pwg-core.h',
  'src/pwg-defs.h',
  'src/pwg-error.h',
  'src/pwg-global.h',
  'src/pwg-metadata.h',
  'src/pwg-node-info.h',
  'src/pwg.h',
  'src/pwg-registry.h',
  'src/pwg-stream.h',
]

pwg_lib = shared_library(
  'pwg-0.1',
  pwg_sources,
  dependencies: [glib_dep, gio_dep, gobject_dep, pipewire_dep],
  include_directories: pwg_inc,
  c_args: [
    '-DBUILDING_PWG',
    '-DPWG_VERSION="@0@"'.format(meson.project_version()),
  ],
  gnu_symbol_visibility: 'hidden',
  soversion: '0',
  version: '0.1.0',
  install: true,
)

install_headers(pwg_headers, subdir: 'pwg-0.1/pwg')

pkg.generate(
  pwg_lib,
  filebase: 'pwg-0.1',
  name: 'Pwg',
  description: 'Experimental GObject Introspection wrapper for PipeWire',
  subdirs: 'pwg-0.1',
  version: meson.project_version(),
  requires: ['glib-2.0', 'gio-2.0', 'gobject-2.0', 'libpipewire-0.3 >= 1.0.2'],
)

pwg_gir = gnome.generate_gir(
  pwg_lib,
  sources: pwg_sources + pwg_headers,
  namespace: 'Pwg',
  nsversion: '0.1',
  identifier_prefix: 'Pwg',
  symbol_prefix: 'pwg',
  export_packages: 'pwg-0.1',
  includes: ['GObject-2.0', 'Gio-2.0'],
  dependencies: [glib_dep, gio_dep, gobject_dep, pipewire_dep],
  header: 'pwg/pwg.h',
  extra_args: ['--warn-all'],
  fatal_warnings: get_option('werror'),
  install: true,
)

pwg_typelib = pwg_gir[1]

test_env = environment()
test_env.prepend('GI_TYPELIB_PATH', meson.current_build_dir())
test_env.prepend('LD_LIBRARY_PATH', meson.current_build_dir())

test(
  'python-gi-import',
  find_program('python3'),
  args: [files('tests/test_import.py')],
  env: test_env,
  depends: pwg_typelib,
)

test(
  'gir-metadata',
  find_program('python3'),
  args: [files('tests/test_gir_metadata.py'), pwg_gir[0]],
  depends: pwg_gir[0],
)

test(
  'public-symbols',
  find_program('python3'),
  args: [files('tests/test_symbols.py'), meson.current_build_dir()],
  depends: pwg_lib,
)
