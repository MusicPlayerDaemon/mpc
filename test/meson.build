test('test_format', executable('test_format',
  'test_format.c',
  '../src/format.c',
  '../src/song_format.c',
  iconv_sources,
  include_directories: inc,
  dependencies: [
    libmpdclient_dep,
    check_dep,
  ]))
