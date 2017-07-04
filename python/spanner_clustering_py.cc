/*
Copyright (c) 2017, Marwan Burelle
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * Python wrapper for geometric spanner and clustering
 */

/*
 * The goal is to provide an interface for the C++ code as simple as possible
 * Technically, the python part should only provide a set of points and get in
 * return a set of edges.
 *
 * Access to other parts (the tree and the decomposition) is optionnal, the only
 * interesting aspect is the specific clustering algorithm.
 *
 * In order to keep things as simple as possible, I just build everything
 * locally in C++, and expose an object with the list of edges and computed
 * clusters. All the constructed C++ object will be discarded silently avoiding
 * useless memory management. We want to keep both worlds as separated as it can
 * be.
 *
 * The mapping between original points and vertices will done by position in the
 * original vector of points.
 */

#include <Python.h>
#include <structmember.h>

#include <valarray>
#include <vector>

#include <clusters.hh>
#include <graph.hh>
#include <point_set.hh>
#include <tree.hh>
#include <wspd.hh>

/*
 * C++ helper code
 */

struct edge {
  unsigned src, dst;
  double   dist;

  edge(unsigned src_, unsigned dst_, double dist_) : src(src_), dst(dst_), dist(dist_) {}
};

struct graph_out {
  std::vector<edge> edges;
  std::vector<size_t> membership;
  size_t number_of_clusters;
};

graph_out build_graph(size_t dim, std::vector<sample>& points, double stretch)
{
  std::vector<unsigned> info;
  for (unsigned i = 0; i < points.size(); i++)
    info.push_back(i);
  PointSet<unsigned> S(dim, points, info);
  auto g = graph<unsigned>::builder(S, stretch)();
  graph_out res;
  for (const auto& p : g.edges) {
    res.edges.push_back({p.first, p.second, S.dist(p.first, p.second)});
  }
  clustering<unsigned> clusters(S, g.W);
  res.membership = clusters.membership;
  res.number_of_clusters = clusters.nb_clusters;
  return res;
}

/*
 * Python bindings
 */

/*
 * My python object
 */
struct SpannerGraph {
  PyObject_HEAD
  PyObject *edges;
  PyObject *membership;
  PyObject *number_of_clusters;
};

static
PyObject* SpannerGraph_new(PyTypeObject *typ, PyObject *args, PyObject *kwds)
{
  SpannerGraph self;

  return typ->tp_alloc(typ, 0);
}

static
sample build_sample(PyObject *smpl, size_t dim)
{
  sample v(dim);
  for (size_t i = 0; i < dim; i++)
    v[i] = PyFloat_AS_DOUBLE(PySequence_GetItem(smpl, i));
  return v;
}

static
std::vector<sample> build_points(PyObject *py_points, size_t dim)
{
  size_t len = PySequence_Size(py_points);
  std::vector<sample> points;
  for (size_t i = 0; i < len; i++) {
    points.push_back(build_sample(PySequence_GetItem(py_points, i), dim));
  }
  return points;
}

static
bool check_points_list(PyObject *py_points, size_t dim)
{
  if (PySequence_Check(py_points)) {
    size_t len = PySequence_Size(py_points);
    for (size_t i = 0; i < len; i++) {
      auto item = PySequence_GetItem(py_points, i);
      if (!PySequence_Check(item) || PySequence_Size(item) != dim)
        return false;
      for (size_t j = 0; j < dim; j++) {
        if (!PyFloat_Check(PySequence_GetItem(item, j)))
          return false;
      }
    }
    return true;
  }
  return false;
}

static
int SpannerGraph_init(SpannerGraph *self, PyObject *args, PyObject *kwds)
{
  unsigned long dim;
  double stretch ;
  PyObject *py_points;
  if (!PyArg_ParseTuple(args, "kOd", &dim, &py_points, &stretch))
    return -1;

  if (!check_points_list(py_points, dim))
    return -1;

  auto points = build_points(py_points, dim);

  auto g = build_graph(dim, points, stretch);

  return 0;
}
