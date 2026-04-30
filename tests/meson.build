test_names = [
    'test_point_set',
    'test_union_find',
    'test_tree',
    'test_wspd',
    'test_integration',
]

gtest_dep = dependency('gtest_main', fallback: ['gtest', 'gtest_main_dep'])

# macOS arm64 needs an explicit ad-hoc codesign — the linker-signed signature
# Meson's default toolchain produces is rejected by Gatekeeper, killing the
# process with SIGKILL before it can print anything.
needs_codesign = host_machine.system() == 'darwin' and host_machine.cpu_family() == 'aarch64'
codesign = find_program('codesign', required: false)

tests_sources = []

foreach t : test_names
    src = files(t + '.cc')
    tests_sources += src
    exe = executable(t, src, dependencies: [spanner_clustering_dep, gtest_dep])
    deps = []
    if needs_codesign and codesign.found()
        deps += custom_target(
            t + '_signed',
            input: exe,
            output: t + '.signed',
            command: [codesign, '--force', '-s', '-', '@INPUT@'],
            build_by_default: true,
        )
    endif
    test(t, exe, depends: deps, protocol: 'gtest')
endforeach
