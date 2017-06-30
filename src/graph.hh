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

    graph operator()(bool) { return g; }
  };

  void info_out(FILE* file, const std::function<std::string(INFO)>& printer) {
    for (size_t i = 0; i < info.size(); i++) {
      fprintf(file, "  %zu [%s];\n", i, printer(info[i]).c_str());
    }
  }

  void edges_print(FILE* file) {
    for (auto e : edges) {
      fprintf(file, "  %u -- %u;\n", e.first, e.second);
    }
  }

  void dot_output(FILE* file,
                  const std::function<std::string(INFO)>& info_printer) {
    fprintf(file, "graph {\n");
    info_out(file, info_printer);
    edges_print(file);
    fprintf(file, "}\n");
  }

  void edge_lists_csv(FILE* file, const PointSet<INFO>& Set) {
    fprintf(file, "Source,Target,Distance,Weight,type\n");
    for (auto e : edges) {
      double d = Set.dist(e.first, e.second);
      fprintf(file, "%u,%u,%g,%g,undirected\n", e.first, e.second, d,
              1 / (d * d));
    }
  }
};

#endif /* GRAPH_HH_ */
