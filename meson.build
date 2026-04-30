project(
    'spanner_clustering',
    'cpp',
    version: '0.2.0',
    license: 'BSD-2-Clause',
    default_options: [
        'cpp_std=c++20',
        'warning_level=2',
        'b_ndebug=if-release',
    ],
)

subdir('src')
subdir('tests')
subdir('python')

clang_format = find_program('clang-format', required: false)
if clang_format.found()
    format_files = src_headers + py_sources + tests_sources

    run_target('format', command: [clang_format, '-i', format_files])
    test(
        'format',
        clang_format,
        args: ['--dry-run', '--Werror', format_files],
        suite: 'lint',
    )
endif
