py = import('python').find_installation(pure: false)

py_sources = files('spanner_clustering_py.cc')

py.extension_module(
    'spanner_graph',
    py_sources,
    dependencies: spanner_clustering_dep,
    cpp_args: ['-Wno-missing-field-initializers'],
    install: true,
)
