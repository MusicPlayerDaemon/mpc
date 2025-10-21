project('mpc', 'c',
  version: '0.36',
  meson_version: '>= 0.47',
  default_options: [
    'c_std=c99',
    'warning_level=3',

    # If we build libmpdclient, it should be linked statically into
    # our executable.
    'libmpdclient:default_library=static',
  ],
  license: 'GPLv2+',
)

cc = meson.get_compiler('c')

conf = configuration_data()
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('VERSION', meson.project_version())

conf.set('HAVE_STRNDUP', cc.has_function('strndup', prefix: '#define _GNU_SOURCE\n#include <string.h>'))

iconv = get_option('iconv')
if iconv.disabled()
  iconv = false
elif cc.has_function('iconv')
  iconv = true
elif iconv.auto()
  iconv = false
else
  error('iconv() not available')
endif
conf.set('HAVE_ICONV', iconv)

configure_file(output: 'config.h', configuration: conf)

common_cflags = [
  # for localtime_r() with glibc
  '-D_GNU_SOURCE',
]

test_cflags = [
  '-fvisibility=hidden',

  '-Wcast-qual',
  '-Wdouble-promotion',
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wshadow',
  '-Wstrict-prototypes',
  '-Wunused',
  '-Wstrict-prototypes',
  '-Wvla',
  '-Wwrite-strings',

  '-Wno-deprecated-declarations',
]

if get_option('buildtype') != 'debug'
  test_cflags += [
    '-ffunction-sections',
    '-fdata-sections',
  ]

  add_global_link_arguments(
    cc.get_supported_link_arguments(
      '-Wl,--gc-sections',
      '-Wl,--icf=all',
    ),
    language: 'c'
  )
endif

common_cflags += cc.get_supported_arguments(test_cflags)

add_project_arguments(common_cflags, language: 'c')

libmpdclient_dep = dependency('libmpdclient', version: '>= 2.18',
                              fallback: ['libmpdclient', 'libmpdclient_dep'])

inc = include_directories(
  'src',

  # for the generated config.h
  '.',
)

if iconv
  iconv_sources = files('src/charset.c')
else
  iconv_sources = []
endif

executable('mpc',
  'src/main.c',
  'src/list.c',
  'src/password.c',
  'src/status.c',
  'src/args.c',
  'src/format.c',
  'src/song_format.c',
  'src/status_format.c',
  'src/audio_format.c',
  'src/tags.c',
  'src/util.c',
  'src/command.c',
  'src/binary.c',
  'src/queue.c',
  'src/output.c',
  'src/sticker.c',
  'src/tab.c',
  'src/idle.c',
  'src/message.c',
  'src/mount.c',
  'src/neighbors.c',
  'src/search.c',
  'src/options.c',
  'src/path.c',
  'src/group.c',
  iconv_sources,
  include_directories: inc,
  dependencies: [
    libmpdclient_dep,
  ],
  install: true
)

install_data('AUTHORS', 'COPYING', 'NEWS', 'README.rst',
  install_dir : join_paths(get_option('datadir'), 'doc', meson.project_name()))

install_data(
  'contrib/mpd-m3u-handler.sh', 'contrib/mpd-pls-handler.sh',
  'contrib/mpc-completion.bash',
  install_dir: join_paths(get_option('datadir'), 'doc', meson.project_name(), 'contrib'))

if get_option('test')
  check_dep = dependency('check')
  subdir('test')
endif

with_documentation = get_option('documentation')
if not with_documentation.disabled()
  sphinx = find_program('sphinx-build', required: with_documentation)
  if sphinx.found()
    subdir('doc')
  endif
endif
