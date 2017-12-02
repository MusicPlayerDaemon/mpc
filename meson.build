project('mpc', 'c',
  version: '0.29',
  default_options: [
    'c_std=c99',
  ],
  license: 'GPLv2+',
)

cc = meson.get_compiler('c')

conf = configuration_data()
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('VERSION', meson.project_version())

iconv = get_option('iconv')
if iconv == 'false'
  iconv = false
elif cc.has_function('iconv')
  iconv = true
elif iconv == 'auto'
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
  '-Wall',
  '-Wextra',
  '-Wno-deprecated-declarations',
  '-Wmissing-prototypes',
  '-Wshadow',
  '-Wpointer-arith',
  '-Wstrict-prototypes',
  '-Wcast-qual',
  '-Wwrite-strings',
]

test_ldflags = [
]

if get_option('buildtype') != 'debug'
  test_cflags += [
    '-ffunction-sections',
    '-fdata-sections',
    '-fvisibility=hidden',
  ]
  test_ldflags += [
    '-Wl,--gc-sections',
  ]
endif

foreach f: test_cflags
  if cc.has_argument(f)
    common_cflags += [ f ]
  endif
endforeach

add_global_arguments(common_cflags, language: 'c')

foreach f: test_ldflags
  if cc.has_argument(f)
    add_global_link_arguments(f, language: 'c')
  endif
endforeach

libmpdclient_dep = dependency('libmpdclient', version: '>= 2.9')

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
  'src/util.c',
  'src/command.c',
  'src/queue.c',
  'src/sticker.c',
  'src/tab.c',
  'src/idle.c',
  'src/message.c',
  'src/search.c',
  'src/options.c',
  'src/path.c',
  iconv_sources,
  include_directories: inc,
  dependencies: [
    libmpdclient_dep,
  ],
  install: true
)

install_man('doc/mpc.1')

install_data('AUTHORS', 'COPYING', 'NEWS', 'README.rst',
  'doc/mpd-m3u-handler.sh', 'doc/mpd-pls-handler.sh',
  'doc/mpc-completion.bash',
  install_dir : join_paths(get_option('datadir'), 'doc', meson.project_name()))

if get_option('test')
  check_dep = dependency('check')
  subdir('test')
endif
