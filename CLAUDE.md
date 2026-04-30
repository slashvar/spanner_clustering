# CLAUDE.md

## Project

Geometric spanner construction using Well Separated Pairs Decomposition (WSPD) with a clustering algorithm built on the fair split tree and union-find. Header-only C++20 library with Python bindings via the CPython C API.

## Build & Test

Build system: Meson (with `meson-python` for the Python extension).

```bash
# C++ tests
meson setup build
meson test -C build

# Python extension + tests (requires Python 3 dev headers)
python3 -m venv .venv && source .venv/bin/activate
pip install . pytest
python3 -m pytest python/tests/ -v
```

On macOS arm64, `tests/meson.build` adds an explicit `codesign` step after each test binary is linked — the linker's adhoc signature alone is rejected by Gatekeeper (SIGKILL with no output).

## Code Style

`.clang-format` at repo root: Google base, 4-space indent, 100 column limit.

**After editing any C++ file, run the format target before committing:**

```bash
meson compile -C build format
```

The validation suite includes a `format` test (`meson test -C build`) that fails on any unformatted file. Filter it out with `meson test -C build --no-suite lint` when iterating.

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
- Add clang-tidy checks (consume `build/compile_commands.json`)
- Implement a community detection algorithm (Louvain or more recent)
