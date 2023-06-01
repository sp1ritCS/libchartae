src = [
	'chparser.c',
	'chtree.c',
	'chtreeview.c',
	[
		'nodes/chnode.c',
		'nodes/chtextnode.c',
		'nodes/chpangonode.c',
		'nodes/chtitlenode.c'
	]
]

inc_dirs = [ include_directories('.') ]

headers = [
	'chartae.h',
	'chparser.h',
	'chtree.h',
	'chtreeview.h',
	[
		'nodes/chnode.h',
		'nodes/chtextnode.h',
		'nodes/chpangonode.h',
		'nodes/chpangonode-parser.h',
		'nodes/chtitlenode.h',
		'nodes/chtitlenode-parser.h'
	]
]

pango2 = subproject('pango2')
inc_dirs += pango2.get_variable('root_inc')

public_deps = [
	dependency('glib-2.0'),
	dependency('gobject-2.0'),
	dependency('gtk4'),
	pango2.get_variable('libpango_dep')
]

chartae_lib = shared_library('chartae', src,
	include_directories: inc_dirs,
	dependencies: [
		public_deps
	],
	c_args: ['-DG_LOG_DOMAIN="Chartae"'],
	install: true
)

chartae_dep = declare_dependency(
	include_directories: inc_dirs,
	link_with: chartae_lib,
	dependencies: public_deps
)

install_headers(headers, subdir: meson.project_name())

pkg_mod = import('pkgconfig')
pkg_mod.generate(chartae_lib,
	name: meson.project_name(),
	filebase: meson.project_name(),
	description: 'Meson sample project.',
	subdirs: meson.project_name(),
	#libraries: ,
	version: meson.project_version()
)
