project('libinput-config', 'c')

cc = meson.get_compiler('c')

libdir = get_option('prefix') / get_option('libdir')
project_name = meson.project_name()

if get_option('shitty_sandboxing')
    add_project_arguments(
        '-DLIBINPUT_SHITTY_SANDBOXING=1',
        language: 'c'
    )

    libdir = '/etc'
endif

if get_option('non_glibc')
    add_project_arguments('-DLIBINPUT_NON_GLIBC=1', language: 'c')

    configure_file(
        input: 'scripts/profile.sh.in',
        output: 'libinput-config.sh',

        configuration: {
            'libdir': libdir,
            'project_name': project_name
        },

        install: true,
        install_dir: '/etc/profile.d'
    )

    configure_file(
        input: 'scripts/profile.fish.in',
        output: 'libinput-config.fish',

        configuration: {
            'libdir': libdir,
            'project_name': project_name
        },

        install: true,
        install_dir: '/etc/fish/conf.d'
    )

    run_target('pre-uninstall', command: ['/bin/true'])
else
    meson.add_install_script('scripts/preload.sh', libdir, project_name)

    run_target('pre-uninstall',
        command: ['scripts/unpreload.sh', libdir, project_name]
    )
endif

libinput_dep = dependency('libinput')
libudev_dep = dependency('libudev')

if (libinput_dep.version().version_compare('>=1.21'))
    add_project_arguments('-DLIBINPUT_HAS_DWTP=1', language: 'c')
endif

shared_library('input-config',
    [
        'src/hooks/gestures.c',
        'src/hooks/init.c',
        'src/hooks/keyboard.c',
        'src/hooks/pointer.c',
        'src/keymap.c',
        'src/main.c',
        'src/override.c'
    ],

    dependencies: [
        libinput_dep.partial_dependency(compile_args: true),
        libudev_dep.partial_dependency(compile_args: true),
        cc.find_library('dl', required: false)
    ],

    link_args: '-Wl,--unresolved-symbols=ignore-all',

    include_directories: include_directories('src'),

    install: true,
    install_dir: libdir
)
