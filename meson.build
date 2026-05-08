project(
  'pipewire-gobject',
  'c',
  version: '0.3.2',
  meson_version: '>=1.0',
  default_options: ['warning_level=2', 'werror=false'],
)

gnome = import('gnome')
pkg = import('pkgconfig')

for_wheel = get_option('wheel')
sh = find_program('sh')

if for_wheel
  py = import('python').find_installation('python3', required: true)
  python_gir_install_dir = py.get_install_dir() / 'pipewire_gobject' / 'gir'
  python_typelib_install_dir = py.get_install_dir() / 'pipewire_gobject' / 'typelib'
endif

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
  'src/pwg-client-info.c',
  'src/pwg-core.c',
  'src/pwg-device-info.c',
  'src/pwg-error.c',
  'src/pwg-global.c',
  'src/pwg-impl-module.c',
  'src/pwg-link-info.c',
  'src/pwg-metadata.c',
  'src/pwg-node.c',
  'src/pwg-node-info.c',
  'src/pwg-param.c',
  'src/pwg-param-info.c',
  'src/pwg-port-info.c',
  'src/pwg-registry.c',
  'src/pwg-stream.c',
]

pwg_headers = [
  'src/pwg-audio-block.h',
  'src/pwg-audio-capture.h',
  'src/pwg-audio-format.h',
  'src/pwg-client-info.h',
  'src/pwg-core.h',
  'src/pwg-device-info.h',
  'src/pwg-defs.h',
  'src/pwg-error.h',
  'src/pwg-global.h',
  'src/pwg-impl-module.h',
  'src/pwg-link-info.h',
  'src/pwg-metadata.h',
  'src/pwg-node.h',
  'src/pwg-node-info.h',
  'src/pwg-param.h',
  'src/pwg-param-info.h',
  'src/pwg-port-info.h',
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
  version: meson.project_version(),
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

if for_wheel
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
    install_dir_gir: python_gir_install_dir,
    install_dir_typelib: python_typelib_install_dir,
  )
else
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
endif

pwg_typelib = pwg_gir[1]

if for_wheel
  py.install_sources('python/pipewire_gobject/__init__.py', subdir: 'pipewire_gobject')
endif

gi_docgen = find_program('gi-docgen', required: false)
if gi_docgen.found()
  docs_config = files('docs/pwg.toml')
  docs_content = files(
    'docs/api/getting-started.md',
    'docs/api/main-loop-and-registry.md',
    'docs/api/pipewire-version-policy.md',
  )

  api_docs_check = custom_target(
    'api-docs-check',
    input: [pwg_gir[0], docs_config],
    output: 'api-docs-check.stamp',
    command: [
      sh,
      '-c',
      'set -e; "$1" check --quiet --fatal-warnings --add-include-path "$2" --config "$3" "$4"; touch "$5"',
      'api-docs-check',
      gi_docgen,
      meson.current_build_dir(),
      '@INPUT1@',
      '@INPUT0@',
      '@OUTPUT@',
    ],
    depends: pwg_typelib,
    build_by_default: false,
  )

  api_docs = custom_target(
    'api-docs',
    input: [pwg_gir[0], docs_config, docs_content],
    output: 'api-docs',
    command: [
      gi_docgen,
      'generate',
      '--quiet',
      '--fatal-warnings',
      '--add-include-path',
      meson.current_build_dir(),
      '--config',
      '@INPUT1@',
      '--content-dir',
      meson.current_source_dir() / 'docs' / 'api',
      '--output-dir',
      '@OUTPUT@',
      '--no-namespace-dir',
      '@INPUT0@',
    ],
    depends: [pwg_typelib, api_docs_check],
    build_by_default: false,
  )

  alias_target('docs', api_docs_check, api_docs)
else
  message('gi-docgen not found; API documentation target disabled')
endif

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

stream_dispatch_test = executable(
  'test-stream-dispatch',
  pwg_sources + ['tests/test_stream_dispatch.c'],
  dependencies: [glib_dep, gio_dep, gobject_dep, pipewire_dep],
  include_directories: pwg_inc,
  c_args: [
    '-DBUILDING_PWG',
    '-DPWG_STREAM_TESTING',
    '-DPWG_VERSION="@0@"'.format(meson.project_version()),
  ],
  gnu_symbol_visibility: 'hidden',
)

test('stream-dispatch', stream_dispatch_test)
