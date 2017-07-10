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

#include <chrono>
#include <iostream>
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
  // Just some timer for debug, to be removed
  auto t0 = std::chrono::steady_clock::now();
  PointSet<unsigned> S(dim, points, info);
  auto g = graph<unsigned>::builder(S, stretch)();
  graph_out res;
  for (const auto& p : g.edges) {
    res.edges.push_back({p.first, p.second, S.dist(p.first, p.second)});
  }
  clustering<unsigned> clusters(S, g.W);
  auto t1 = std::chrono::steady_clock::now();
  {
    std::chrono::duration<double> diff = t1 - t0;
    std::clog << "Spanner+cluster time: " << diff.count() << "s\n";
  }
  res.membership = clusters.membership;
  res.number_of_clusters = clusters.nb_clusters;
  return res;
}

/*
 * Translation unit between C++ and python
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

/* my error */
static PyObject *spanner_graph_error;

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
      if (!PySequence_Check(item) || PySequence_Size(item) != (Py_ssize_t)dim)
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
bool generate_py_object(SpannerGraph* self, graph_out& g)
{

  /*
   * creating/allocating base data
   * XXX: maybe some place for exceptions, also need to check for refcount
   * decrement ...
   */
  self->edges = PyList_New(0);
  if (!self->edges)
    return false;

  self->membership = PyList_New(g.membership.size());
  if (!self->membership)
    return false;

  self->number_of_clusters = PyLong_FromSize_t(g.number_of_clusters);
  if (!self->number_of_clusters)
    return false;

  /*
   * Now we can translate the 2 vectors
   */

  /*
   * First, the membership
   */
  for (size_t i = 0; i < g.membership.size(); i++) {
    auto n = PyLong_FromSize_t(g.membership[i]);
    if (!n)
      return false;
    PyList_SET_ITEM(self->membership, i, n);
  }

  /*
   * Now edges
   */
  for (const auto& e : g.edges) {
    auto item = Py_BuildValue("IId", e.src, e.dst, e.dist);
    if (!item)
      return false;
    if (PyList_Append(self->edges, item) == -1)
      return false;
  }

  return true;
}


/*
 * Python bindings
 */

static
PyObject* SpannerGraph_new(PyTypeObject *typ, PyObject*, PyObject*)
{
  return typ->tp_alloc(typ, 0);
}

static
int SpannerGraph_init(SpannerGraph *self, PyObject *args, PyObject*)
{
  unsigned long dim;
  double stretch ;
  PyObject *py_points;
  if (!PyArg_ParseTuple(args, "kOd", &dim, &py_points, &stretch)) {
    return -1;
  }

  if (!check_points_list(py_points, dim)) {
    PyErr_SetString(spanner_graph_error, "Wrong parameters form");
    return -1;
  }

  auto points = build_points(py_points, dim);

  auto g = build_graph(dim, points, stretch);

  if (!generate_py_object(self, g)) {
    PyErr_SetString(spanner_graph_error, "Error while translating graph object");
    return -1;
  }

  return 0;
}

static
void SpannerGraph_dealloc(SpannerGraph* self)
{
  Py_XDECREF(self->edges);
  Py_XDECREF(self->membership);
  Py_XDECREF(self->number_of_clusters);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static
PyMemberDef SpannerGraph_members[] = {
  {(char*)"edges", T_OBJECT_EX, offsetof(SpannerGraph, edges), 0, (char*)"list of edges"},
  {
    (char*)"membership", T_OBJECT_EX, offsetof(SpannerGraph, membership), 0,
    (char*)"cluster membership"
  },
  {
    (char*)"number_of_clusters", T_OBJECT_EX,
    offsetof(SpannerGraph, number_of_clusters),
    0, (char*)"number of clusters"
  },
  {0,0,0,0,0}
};

static PyTypeObject SpannerGraphType= {
    PyVarObject_HEAD_INIT(NULL, 0)
    "spanner_graph.SpannerGraph",             /* tp_name */
    sizeof(SpannerGraph),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)SpannerGraph_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "Spanner Graph Object",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    0,             /* tp_methods */
    SpannerGraph_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)SpannerGraph_init,      /* tp_init */
    0,                         /* tp_alloc */
    SpannerGraph_new,                 /* tp_new */
};

static PyModuleDef spanner_graph_module = {
  PyModuleDef_HEAD_INIT,
  "spanner_graph",
  "Geometric Spanner and clusters.",
  -1,
  0, 0, 0, 0, 0
};

PyMODINIT_FUNC
PyInit_spanner_graph(void)
{
  PyObject *m;
  if (PyType_Ready(&SpannerGraphType) < 0)
    return NULL;

  m = PyModule_Create(&spanner_graph_module);
  if (!m)
    return NULL;

  spanner_graph_error = PyErr_NewException("spanner_graph.error", NULL, NULL);
  Py_INCREF(spanner_graph_error);
  PyModule_AddObject(m, "error", spanner_graph_error);

  Py_INCREF(&SpannerGraphType);
  PyModule_AddObject(m, "SpannerGraph", (PyObject*)&SpannerGraphType);

  return m;
}
