custom_target(
  'HTML documentation',
  output: 'html',
  input: ['index.rst', 'conf.py'],
  command: [sphinx, '-q', '-b', 'html', '-d', '@OUTDIR@/html_doctrees', meson.current_source_dir(), '@OUTPUT@'],
  build_by_default: true,
  install: true,
  install_dir: join_paths(get_option('datadir'), 'doc', meson.project_name()),
)

custom_target(
  'Manpage documentation',
  output: 'man',
  input: ['index.rst', 'conf.py'],
  command: [sphinx, '-q', '-b', 'man', '-d', '@OUTDIR@/man_doctrees', meson.current_source_dir(), '@OUTPUT@/man1'],
  build_by_default: true,
  install: true,
  install_dir: get_option('datadir'),
)
