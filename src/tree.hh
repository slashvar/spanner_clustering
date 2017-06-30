/* fairsplit tree */

#ifndef TREE_HH_
#define TREE_HH_

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_set>
#include <valarray>
#include <vector>

#include "point_set.hh"

template <typename INFO>
struct tree {
  struct node {
    PointSet<INFO>& Set;
    bool is_in_pair;
    std::shared_ptr<node> left, right;
    std::weak_ptr<node> cluster_parent;
    std::vector<std::vector<size_t>> dimensions;
    std::vector<size_t> points;
    sample low, sizes, center, upper;
    double radius;
    size_t id;
    tree<INFO>& cur_tree;
    std::atomic_ulong next_point;

    node(PointSet<INFO>& s, tree<INFO>& ct)
        : Set(s),
          is_in_pair(false),
          dimensions(Set.dim),
          low(Set.dim),
          sizes(Set.dim),
          upper(Set.dim),
          cur_tree(ct),
          next_point(0) {}

    node(PointSet<INFO>& s, tree<INFO>& ct, bool)
        : Set(s),
          is_in_pair(false),
          low(Set.dim),
          sizes(Set.dim),
          upper(Set.dim),
          cur_tree(ct),
          next_point(0) {}

    unsigned long next_point_id() {
      unsigned long n = next_point.fetch_add(1, std::memory_order_relaxed);
      return n % points.size();
    }

    bool leaf() { return radius == 0.0 && left == 0 && right == 0; }

    double dist(std::shared_ptr<node> n) {
      return distance(center, n->center) - radius - n->radius;
    }

    bool close_to(std::shared_ptr<node> n, double avg_radius) {
      double d = distance(center, n->center);
      double r = n->radius > 0.0 ? n->radius : avg_radius;
      r += radius > 0.0 ? radius : avg_radius;
      return d < r;
    }

    bool include(std::shared_ptr<node> n) {
      if (radius < n->radius) return false;
      return (low <= n->low).min() && (upper >= n->upper).min();
    }

    bool include_radius_based(std::shared_ptr<node> n) {
      return n->radius < radius && distance(center, n->center) <= radius;
    }

    bool include_tree_traversal(std::shared_ptr<node> n) {
      if (n->id == id) return true;
      size_t split_d = maxd();
      double split_val = low[split_d] + sizes[split_d] / 2;
      if (n->low[split_d] < split_val)
        return !!left && left->include_tree_traversal(n);
      return !!right && right->include_tree_traversal(n);
    }

    size_t maxd() {
      size_t m = 0;
      for (size_t i = 1; i < sizes.size(); i++) {
        if (sizes[i] > sizes[m]) m = i;
      }
      return m;
    }

    size_t split_point(size_t split_d, double split_val) {
      auto& v = dimensions[split_d];
      size_t l = 0, r = v.size();
      while (l < r) {
        size_t mid = l + (r - l) / 2;
        if (Set.get(split_d, v[mid]) == split_val) {
          for (; Set.get(split_d, v[mid - 1]) == split_val; mid--) continue;
          return mid;
        }
        if (split_val < Set.get(split_d, v[mid]))
          r = mid;
        else
          l = mid + 1;
      }
      return l;
    }

    void distribute(size_t d) {
      std::unordered_set<size_t> sleft;
      for (size_t p : left->dimensions[d]) sleft.insert(p);
      for (size_t i = 0; i < dimensions.size(); i++) {
        if (i != d) {
          for (size_t p : dimensions[i]) {
            if (sleft.find(p) == sleft.end())
              right->dimensions[i].push_back(p);
            else
              left->dimensions[i].push_back(p);
          }
        }
      }
      Set.updateBox(left);
      Set.updateBox(right);
    }

    bool split_r() {
      if (radius == 0.0) {
        left = 0;
        right = 0;
        return false;
      }
      size_t split_d = maxd();
      double split_val = low[split_d] + sizes[split_d] / 2;
      size_t p = split_point(split_d, split_val);
      left = std::make_shared<node>(Set, cur_tree);
      left->id = cur_tree.next_id();
      right = std::make_shared<node>(Set, cur_tree);
      right->id = cur_tree.next_id();
      std::vector<size_t> vleft(dimensions[split_d].begin(),
                                dimensions[split_d].begin() + p);
      std::vector<size_t> vright(dimensions[split_d].begin() + p,
                                 dimensions[split_d].end());
      left->dimensions[split_d] = vleft;
      right->dimensions[split_d] = vright;
      left->points = vleft;
      right->points = vright;
      distribute(split_d);
      return true;
    }

    void split() {
      if (split_r()) {
        left->split();
        right->split();
      }
    }

    void display() {
      std::clog << "  center:";
      for (double x : center) std::clog << " " << x;
      std::clog << "\n";
      std::clog << "  Radius: " << radius << "\n";
    }
  };

  PointSet<INFO>& Set;
  std::shared_ptr<node> root;
  size_t node_ids;

  size_t next_id() { return ++node_ids; }

  static void seq_split(tree<INFO>* tree) { tree->root->split(); }

  tree(PointSet<INFO>& s,
       const std::function<void(tree<INFO>*)>& splitter = &seq_split)
      : Set(s), root(std::make_shared<node>(Set, *this, false)), node_ids(0) {
    for (auto d : Set.dimensions) root->dimensions.push_back(d);
    Set.updateBox(root);
    root->id = next_id();
    splitter(this);
  }
};

#endif /* TREE_HH_ */
