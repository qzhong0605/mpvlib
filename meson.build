project('libmpv',
        'c',
        license: ['GPL2+', 'LGPL2.1+'],
        version: files('./VERSION'),
        meson_version: '>=0.60.3',
        default_options: [
            'buildtype=debugoptimized',
            'b_lundef=false',
            'c_std=c11',
            'warning_level=1',
        ]
)

build_root = meson.project_build_root()
source_root = meson.project_source_root()
python = find_program('python3')

pthreads = dependency('threads')

dependencies = [pthreads]

# Keeps track of all enabled/disabled features
features = {
    'debug': get_option('debug'),
    'threads': true,
}


# generic sources
sources = files(
    ## compilation 
    'src/compilation/shader_cache.c',
)


# compiler stuff
cc = meson.get_compiler('c')

flags = ['-D_ISOC99_SOURCE', '-D_GNU_SOURCE',
         '-D_FILE_OFFSET_BITS=64']
link_flags = []

test_flags = ['-Werror=implicit-function-declaration',
              '-Wno-error=deprecated-declarations',
              '-Wno-error=unused-function',
              '-Wempty-body',
              '-Wdisabled-optimization',
              '-Wstrict-prototypes',
              '-Wno-format-zero-length',
              '-Wno-redundant-decls',
              '-Wvla',
              '-Wno-format-truncation',
              '-Wimplicit-fallthrough',
              '-fno-math-errno']

flags += cc.get_supported_arguments(test_flags)

if cc.has_multi_arguments('-Wformat', '-Werror=format-security')
    flags += '-Werror=format-security'
endif

if cc.get_id() == 'gcc'
    gcc_flags = ['-Wundef', '-Wmissing-prototypes', '-Wshadow',
                 '-Wno-switch', '-Wparentheses', '-Wpointer-arith',
                 '-Wno-pointer-sign',
                 # GCC bug 66425
                 '-Wno-unused-result']
    flags += gcc_flags
endif

if cc.get_id() == 'clang'
    clang_flags = ['-Wno-logical-op-parentheses', '-Wno-switch',
                   '-Wno-tautological-compare', '-Wno-pointer-sign',
                   '-Wno-tautological-constant-out-of-range-compare']
    flags += clang_flags
endif

features += {'posix': true}

noexecstack = false
if cc.has_link_argument('-Wl,-z,noexecstack')
    link_flags += '-Wl,-z,noexecstack'
    noexecstack = true
endif

if cc.has_link_argument('-Wl,--nxcompat,--no-seh,--dynamicbase')
    link_flags += '-Wl,--nxcompat,--no-seh,--dynamicbase'
    noexecstack = true
endif

features += {'noexecstack': noexecstack}

features += {'build-date': get_option('build-date')}
if not features['build-date']
    flags += '-DNO_BUILD_TIMESTAMPS'
endif

features += {'ta-leak-report': get_option('ta-leak-report')}

libdl_dep = cc.find_library('dl', required: false)
features += {'libdl': cc.has_function('dlopen', dependencies: libdl_dep, prefix: '#include <dlfcn.h>')}
if features['libdl']
    dependencies += libdl_dep
endif

client_h_define = cc.get_define('MPV_CLIENT_API_VERSION', prefix: '#include "libmpv/client.h"',
                                include_directories: include_directories('.'))
major = client_h_define.split('|')[0].split('<<')[0].strip('() ')
minor = client_h_define.split('|')[1].strip('() ')
client_api_version = major + '.' + minor + '.0'

libmpv = library('mpv', sources, dependencies: dependencies, gnu_symbol_visibility: 'hidden',
                 version: client_api_version, include_directories: includedir,
                 install: get_option('libmpv'), build_by_default: get_option('libmpv'))
