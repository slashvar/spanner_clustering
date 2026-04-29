# CLAUDE.md

## Project

Geometric spanner construction using Well Separated Pairs Decomposition (WSPD) with a clustering algorithm built on the fair split tree and union-find. Header-only C++20 library with Python bindings via the CPython C API.

## Build & Test

```bash
# C++ tests (no dependencies beyond a C++20 compiler)
make -C tests test

# Python extension + tests (requires Python 3 dev headers)
cd python
python3 -m venv .venv && source .venv/bin/activate
pip install setuptools pytest
python3 setup.py build_ext --inplace
python3 -m pytest tests/ -v
```

On macOS arm64, the Makefile automatically codesigns test binaries.

## Code Style

`.clang-format` at repo root: Google base, 4-space indent, 100 column limit.

```bash
clang-format -i src/*.hh python/spanner_clustering_py.cc tests/test_harness.hh tests/test_*.cc
```

## Architecture

Header-only, dependency order (each includes the ones before it):

1. `src/point_set.hh` — `sample` type alias (`valarray<double>`), `norm()`, `distance()`, `PointSet<INFO>` with bounding box and dimension-sorted indices
2. `src/tree.hh` — Fair split tree: recursive binary space partitioning, `node::split()`, cached `max_dim`
3. `src/wspd.hh` — Well Separated Pairs Decomposition: `wellseparated()` predicate, recursive `findpairs()` with optional edge callback
4. `src/graph.hh` — T-spanner graph: `builder` constructs edges from WSPD pairs using round-robin point selection
5. `src/clusters.hh` — Union-find (`find`/`unify` free functions), `clustering` struct that identifies cluster heads from the WSPD and merges nearby ones

Python binding: `python/spanner_clustering_py.cc` exposes `SpannerGraph(dim, points, stretch)` returning edges, membership, and cluster count.

## Known Bugs

- `PointSet::updateBox()` has UB on empty points — calls `.front()`/`.back()` on empty dimension vectors (`point_set.hh:74-75`)

## Roadmap

- Migrate test framework from minimal assert harness to Google Test
- Migrate build system to Meson
- Add clang-tidy checks
- Implement a community detection algorithm (Louvain or more recent)
