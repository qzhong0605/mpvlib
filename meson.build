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
    ## common
    'src/common.c',
    'src/path.c',
    'src/msg.c',

    ## glsl-related
    'src/shader_cache.c',
    'src/user_shaders.c',
    'src/utils.c',
    'src/ra.c',
    'src/spirv.c',
    'src/context.c',

    ## osdeps
    'src/osdep/timer.c',
    'src/osdep/threads.c',
    'src/osdep/io.c',

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

# android 
features += {'android': host_machine.system() == 'android'}
if features['android']
    dependencies += cc.find_library('android')
    sources += files('src/osdep/android/strnlen.c')
endif 

# OpenGL 
gl_allowed = get_option('gl').allowed()
features += {'gl': false}

egl = dependency('egl', version: '> 1.4.0', required: get_option('egl'))
features += {'egl': egl.found() and gl_allowed}
if features['egl']
    dependencies += egl
endif

egl_android_opt = get_option('egl-android').require(
    features['android'] and gl_allowed,
    error_message: 'the OS is not android!',
)
egl_android = cc.find_library('EGL', required: egl_android_opt)
features += {'egl-android': egl_android.found()}
if features['egl-android']
    dependencies += egl_android
    features += {'gl': true}
    sources += files('src/opengl/context_android.c')
endif

shaderc = dependency('shaderc', required: get_option('shaderc'))
features += {'shaderc': shaderc.found()}
if features['shaderc']
    dependencies += shaderc
    sources += files('src/spirv_shaderc.c')
endif

# vulkan
libplacebo = dependency('libplacebo', version: '>=4.157.0', required: get_option('libplacebo'))
features += {'libplacebo': libplacebo.found()}
if features['libplacebo']
    dependencies += libplacebo
    sources += files('video/out/placebo/ra_pl.c',
                     'video/out/placebo/utils.c')
endif

vulkan_opt = get_option('vulkan').require(
    libplacebo.get_variable('pl_has_vulkan', default_value: '0') == '1',
    error_message: 'libplacebo could not be found!',
)
vulkan = dependency('vulkan', required: vulkan_opt)
features += {'vulkan': vulkan.found()}
if features['vulkan']
    dependencies += vulkan
    sources += files('video/out/vulkan/context.c',
                     'video/out/vulkan/utils.c')
endif

if features['vulkan'] and features['android']
    sources += files('video/out/vulkan/context_android.c')
endif

# uwp
uwp_opt = get_option('uwp')
uwp = cc.find_library('windowsapp', required: uwp_opt)
features += {'uwp': uwp.found()}
if features['uwp']
    dependencies += uwp
    sources += files('osdep/path-uwp.c')
endif

features += {'glob-posix': cc.has_function('glob', prefix: '#include <glob.h>')}
features += {'glob': features['glob-posix'] or features['glob-win32']}

darwin = host_machine.system() == 'darwin'
win32 = host_machine.system() == 'cygwin' or host_machine.system() == 'windows'
posix = not win32

features += {'darwin': darwin}
features += {'posix': posix}
features += {'dos-paths': win32, 'win32': win32}

# atomic
stdatomic_dep = cc.find_library('atomic', required: false)
features += {'stdatomic': cc.has_header_symbol('stdatomic.h', 'atomic_int', dependencies: stdatomic_dep,
                                                 required: get_option('stdatomic'))}
if features['stdatomic']
    dependencies += stdatomic_dep
endif

# macOS's pthread_setname_np is a special snowflake and differs from literally every other platform.
features += {'osx-thread-name': darwin}

features += {'glibc-thread-name': false}
if not features['osx-thread-name']
    features += {'glibc-thread-name': posix and cc.has_function('pthread_setname_np', args: '-D_GNU_SOURCE',
                 dependencies: pthreads, prefix: '#include <pthread.h>')}
endif

features += {'bsd-thread-name': false}
if not features['osx-thread-name'] and not features['glibc-thread-name']
    features += {'bsd-thread-name': posix and cc.has_function('pthread_set_name_np', dependencies: pthreads,
                  prefix: '#include <pthread.h>\n#include <pthread_np.h>')}
endif

includedir = []
win32_pthreads = get_option('win32-internal-pthreads').require(
    win32 and not posix,
    error_message: 'the os is not win32!',
)
features += {'win32-internal-pthreads': win32_pthreads.allowed()}
if features['win32-internal-pthreads']
    flags += ['-isystem', '-I', '-DIN_WINPTHREAD']
    # Note: Adding this include causes POSIX_TIMERS to be defined for
    # unclear reasons (some confusion with <pthread.h> probably).
    # Hack around it by using HAVE_WIN32_INTERNAL_PTHREADS.
    includedir += include_directories('osdep/win32/include')
    sources += files('osdep/win32/pthread.c')
endif

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


add_project_arguments(flags, language: 'c')
add_project_link_arguments(link_flags, language: 'c')

includedir += ['src']

libmpv = library('mpv', sources, dependencies: dependencies, gnu_symbol_visibility: 'hidden',
                 include_directories: includedir,
                 install: true, build_by_default: true)
