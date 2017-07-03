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

/* Graph structure for the t-spanner */

#ifndef GRAPH_HH_
#define GRAPH_HH_

#include <cstdio>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "point_set.hh"
#include "tree.hh"
#include "wspd.hh"

template <typename INFO>
struct graph {
  unsigned order;
  std::vector<std::pair<unsigned, unsigned>> edges;
  const std::vector<INFO>& info;
  wspd<INFO> W;

  graph(unsigned o, const std::vector<INFO>& inf, PointSet<INFO>& Set,
        double stretch)
      : order(o), info(inf), W(Set, 4 * (stretch + 1) / (stretch - 1)) {}

  graph(unsigned o, const std::vector<INFO>& inf, PointSet<INFO>& Set,
        double stretch, std::function<void(tree<INFO>*)>& splitter)
      : order(o),
        info(inf),
        W(Set, 4 * (stretch + 1) / (stretch - 1), splitter) {}

  graph(unsigned o, const std::vector<INFO>& inf, PointSet<INFO>& Set,
        double stretch, std::function<void(tree<INFO>*)>& splitter, bool)
      : order(o),
        info(inf),
        W(Set, 4 * (stretch + 1) / (stretch - 1, true), splitter) {}

  void add_edge(unsigned u, unsigned v) {
    if (v < u) std::swap(u, v);
    edges.push_back({u, v});
  }

  struct builder {
    PointSet<INFO>& Set;
    graph<INFO> g;
    std::unordered_map<typename wspd<INFO>::box, size_t> next_pos;

    builder(PointSet<INFO>& s, double stretch)
        : Set(s), g(Set.points.size(), Set.infos, Set, stretch) {}

    builder(PointSet<INFO>& s, double stretch,
            std::function<void(tree<INFO>*)>& splitter)
        : Set(s), g(Set.points.size(), Set.infos, Set, stretch, splitter) {}

    size_t next(typename wspd<INFO>::box node) {
      size_t cur = next_pos[node];
      next_pos[node] = (cur + 1) % node->points.size();
      return node->points[cur];
    }

    graph operator()() {
      for (const auto& p : g.W.pairs) {
        size_t p1 = next(p.first);
        size_t p2 = next(p.second);
        g.add_edge(p1, p2);
      }
      return g;
    }

    /* Don't build graph, usefull in order to use an external implementation */
    graph operator()(bool) { return g; }
  };
};

#endif /* GRAPH_HH_ */
