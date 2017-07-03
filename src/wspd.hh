/* well separated pair decomposition */

#ifndef WSPD_HH_
#define WSPD_HH_

#include <functional>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "point_set.hh"
#include "tree.hh"

template <typename INFO>
struct wspd {
  PointSet<INFO>& Set;
  tree<INFO> split_tree;
  double sep;

  using box = std::shared_ptr<typename tree<INFO>::node>;
  using wspair = typename std::pair<box, box>;

  std::vector<wspair> pairs;

  wspd(PointSet<INFO>& set, double s) : Set(set), split_tree(Set), sep(s) {
    decompose(split_tree.root);
  }

  wspd(PointSet<INFO>& set, double s,
       std::function<void(tree<INFO>*)>& splitter)
      : Set(set), split_tree(Set, splitter), sep(s) {
    decompose(split_tree.root);
  }

  /*
   * No decompose version, usefull for external implementation of the
   * decomposition (like parallel implementation.)
   */
  wspd(PointSet<INFO>& set, double s, bool)
      : Set(set), split_tree(Set), sep(s) {}

  wspd(PointSet<INFO>& set, double s,
       std::function<void(tree<INFO>*)>& splitter, bool)
      : Set(set), split_tree(Set, splitter), sep(s) {}

  template <typename BOX>
  bool wellsepareted(BOX b1, BOX b2) {
    double r = std::max(b1->radius, b2->radius);
    return b1->dist(b2) >= sep * r;
  }

  void addpair(box b1, box b2) {
    pairs.push_back({b1, b2});
    b1->is_in_pair = true;
    b2->is_in_pair = true;
  }

  void findpairs(box b1, box b2) {
    if (wellsepareted(b1, b2)) {
      addpair(b1, b2);
      return;
    }
    size_t d1 = b1->maxd(), d2 = b2->maxd();
    if (b1->sizes[d1] > b2->sizes[d2]) {
      std::swap(b1, b2);
    }
    findpairs(b1, b2->left);
    findpairs(b1, b2->right);
  }

  void findpairs(box b1, box b2, std::function<void(box, box)>& edge) {
    if (wellsepareted(b1, b2)) {
      addpair(b1, b2);
      edge(b1, b2);
      return;
    }
    size_t d1 = b1->maxd(), d2 = b2->maxd();
    if (b1->sizes[d1] > b2->sizes[d2]) {
      std::swap(b1, b2);
    }
    findpairs(b1, b2->left, edge);
    findpairs(b1, b2->right, edge);
  }

  void decompose(box n) {
    if (n->leaf()) return;
    findpairs(n->left, n->right);
    decompose(n->left);
    decompose(n->right);
  }
};

#endif /* WSPD_HH_ */
