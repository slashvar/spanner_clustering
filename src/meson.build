src_headers = files(
    'point_set.hh',
    'tree.hh',
    'wspd.hh',
    'graph.hh',
    'clusters.hh',
)

spanner_clustering_dep = declare_dependency(
    include_directories: include_directories('.'),
)

install_headers(src_headers, subdir: 'spanner_clustering')
