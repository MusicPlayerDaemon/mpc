#!/usr/bin/env python3

import os, sys, shutil, subprocess

global_options = ['--werror']

flavors = {
    'debug': {
        'options': [
            '-Dtest=true',
        ],
    },

    'release': {
        'options': [
            '--buildtype', 'release',
            '-Db_ndebug=true',
            '-Db_lto=true',
            '-Dtest=true',
            '-Ddocs=false',
        ],
        'env': {
            'LDFLAGS': '-fuse-ld=gold -Wl,--gc-sections,--icf=all',
        },
    },

    'mini': {
        'options': [
            '--buildtype', 'release',
            '-Db_ndebug=true',
            '-Db_lto=true',
            '-Diconv=false',
            '-Ddocs=false',
        ],
        'env': {
            'LDFLAGS': '-fuse-ld=gold -Wl,--gc-sections,--icf=all',
        },
    },

    'musl': {
        'options': [
            '--buildtype', 'minsize',
            '--default-library', 'static',
            '-Db_ndebug=true',
            '-Db_lto=true',
            '-Ddocs=false',
        ],
        'env': {
            'CC': 'musl-gcc',
            'LDFLAGS': '-fuse-ld=gold -Wl,--gc-sections,--icf=all',
        },
    },

    'win32': {
        'arch': 'i686-w64-mingw32',
        'options': [
            '-Ddocs=false',
        ]
    },

    'win64': {
        'arch': 'x86_64-w64-mingw32',
        'options': [
            '-Ddocs=false',
        ]
    },
}

project_name = 'mpc'
source_root = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]) or '.', '..'))
output_path = os.path.join(source_root, 'output')
prefix_root = '/usr/local/stow'

for name, data in flavors.items():
    print(name)
    build_root = os.path.join(output_path, name)

    env = os.environ.copy()
    if 'env' in data:
        env.update(data['env'])

    cmdline = [
        'meson', source_root, build_root,
    ] + global_options

    if 'options' in data:
        cmdline.extend(data['options'])

    prefix = os.path.join(prefix_root, project_name + '-' + name)

    if 'arch' in data:
        prefix = os.path.join(prefix, data['arch'])
        cmdline += ('--cross-file', os.path.join(source_root, 'build', name, 'cross-file.txt'))

    cmdline += ('--prefix', prefix)

    try:
        shutil.rmtree(build_root)
    except:
        pass

    subprocess.check_call(cmdline, env=env)
