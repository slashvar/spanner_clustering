# setup.py: building/installing spanner graph python module

from distutils.core import setup, Extension

INCLUDE_DIRS = ['../src']

spannerModule = Extension (
        'spanner_graph',
        sources = ['spanner_clustering_py.cc'],
        extra_compile_args = ['-std=c++14', '-O3', '-Wno-missing-field-initializers'],
        include_dirs = INCLUDE_DIRS
        )

setup (
        name = 'SpannerGraphPkg',
        version = '0.1',
        description = 'Building spanner graph and WSPD based clustering',
        ext_modules = [spannerModule]
        )
