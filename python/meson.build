py = import('python').find_installation(pure: false)

py.extension_module(
    'spanner_graph',
    'spanner_clustering_py.cc',
    dependencies: spanner_clustering_dep,
    cpp_args: ['-Wno-missing-field-initializers'],
    install: true,
)
