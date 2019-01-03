project('libjoystick', 'c', 'cpp',
	version: '0.0.1',
	license: 'LGPLv2',
	default_options: [ 'c_std=gnu99', 'warning_level=2' ],
	meson_version: '>= 0.41.0')

libjoystick_version = meson.project_version().split('.')

dir_src         = join_paths(meson.source_root(), 'src')

# We use libtool-version numbers because it's easier to understand.
# Before making a release, the libjoystick_so_*
# numbers should be modified. The components are of the form C:R:A.
# a) If binary compatibility has been broken (eg removed or changed interfaces)
#    change to C+1:0:0.
# b) If interfaces have been changed or added, but binary compatibility has
#    been preserved, change to C+1:0:A+1
# c) If the interface is the same as the previous version, change to C:R+1:A
libjoystick_lt_c=23
libjoystick_lt_r=0
libjoystick_lt_a=13

# convert to soname
libjoystick_so_version = '@0@.@1@.@2@'.format((libjoystick_lt_c - libjoystick_lt_a),
                                               libjoystick_lt_a, libjoystick_lt_r)

############ compiler setup ###########
cc = meson.get_compiler('c')
cppflags = ['-Wno-unused-parameter', '-g', '-fvisibility=hidden']
cflags = cppflags + ['-Wmissing-prototypes', '-Wstrict-prototypes']
add_project_arguments(cflags, language: 'c')
add_project_arguments(cppflags, language: 'cpp')

############ config.h ###########
config_h = configuration_data()
config_h.set('_GNU_SOURCE', '1')
config_h.set('LIBJOYSTICK_VERSION_MAJOR', libjoystick_version[0])
config_h.set('LIBJOYSTICK_VERSION_MINOR', libjoystick_version[1])
config_h.set('LIBJOYSTICK_VERSION_MICRO', libjoystick_version[2])
config_h.set('LIBJOYSTICK_VERSION', meson.project_version())

prefix = '''#define _GNU_SOURCE 1
#include <assert.h>
'''
if cc.get_define('static_assert', prefix: prefix) == ''
	config_h.set('static_assert(...)', '/* */')
endif

############ dependencies ###########
pkgconfig = import('pkgconfig')
dep_udev = dependency('libudev')
dep_libevdev = dependency('libevdev')

############ include directories ###########
includes_src = include_directories('src')
includes_include = include_directories('include')

############ libjoystick.so ############
install_headers('src/libjoystick.h')
src_libjoystick = [
	'src/libjoystick.c',
]

deps_libjoystick = [
	dep_udev,
	dep_libevdev,
]

mapfile = join_paths(dir_src, 'libjoystick.sym')

version_flag = '-Wl,--version-script,@0@'.format(mapfile)
lib_libjoystick = shared_library('joystick',
		src_libjoystick,
		include_directories: [include_directories('.'), includes_include],
		dependencies: deps_libjoystick,
		version: libjoystick_so_version,
		link_args: version_flag,
		link_depends: mapfile,
		install: true
		)

dep_libjoystick = declare_dependency(link_with: lib_libjoystick,
				     dependencies: deps_libjoystick)

pkgconfig.generate(
	filebase: 'libjoystick',
	name: 'Joystick',
	description: 'Joystick device library',
	version: meson.project_version(),
	libraries: lib_libjoystick
)

############ documentation ############
if get_option('documentation')
	subdir('doc')
endif

############ tests ############
executable('test-build-c',
	   'test/build-c.c',
	   dependencies: [dep_libjoystick],
	   include_directories: [includes_src, includes_include],
	   install: false)

executable('test-build-cxx',
	   'test/build-cxx.cc',
	   dependencies: [dep_libjoystick],
	   include_directories: [includes_src, includes_include],
	   install: false)

############ output files ############
configure_file(output: 'config.h', install: false, configuration: config_h)
