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

# Set config.h
conf_data = configuration_data()
# conf_data.set_quoted('CONFIGURATION', configuration)
# conf_data.set_quoted('DEFAULT_DVD_DEVICE', dvd_device)
# conf_data.set_quoted('DEFAULT_CDROM_DEVICE', cd_device)

# Loop over all features in the build, create a define and add them to config.h
feature_keys = []
foreach feature, allowed: features
    define = 'HAVE_@0@'.format(feature.underscorify().to_upper())
    conf_data.set10(define, allowed)
    # special handling for lua
    if feature == 'lua' and allowed
        feature_keys += lua_version
        continue
    endif
    if allowed
        feature_keys += feature
    endif
endforeach

# Script to sort the feature_keys object.
feature_sort = '''
#!/usr/bin/env python3
import sys
features = " ".join(sorted(sys.argv[1:]))
sys.stdout.write(features)
'''
feature_str = run_command(python, '-c', feature_sort, feature_keys, check: true).stdout()
conf_data.set_quoted('FULLCONFIG', feature_str)
conf_data.set_quoted('MPV_CONFDIR', join_paths(get_option('prefix'), get_option('sysconfdir'), 'mpv'))
conf_data.set_quoted('PLATFORM', host_machine.system())
configure_file(output : 'config.h', configuration : conf_data)
message('List of enabled features: ' + feature_str)

# generic sources
sources = files(
    ## compilation 
    'src/compilation/shader_cache.c',

    ## common
    'src/common/common.c',
    'src/common/msg.c',

    ## options
    'src/options/options.c',
    'src/options/path.c',
    'src/options/m_option.c',

    ## os deps
    'src/osdep/io.c',

    ## misc 
    'src/misc/bstr.c',
    'src/misc/random.c',

    ## tree allocator
    'src/ta/ta.c',
    'src/ta/ta_talloc.c',
    'src/ta/ta_utils.c',
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

add_project_arguments(flags, language: 'c')
add_project_link_arguments(link_flags, language: 'c')

includedir = ['src']

libmpv = library('mpv', sources, dependencies: dependencies, gnu_symbol_visibility: 'hidden',
                 include_directories: includedir,
                 install: true, build_by_default: true)
