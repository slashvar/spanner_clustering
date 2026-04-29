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

/* fairsplit tree */

#ifndef TREE_HH_
#define TREE_HH_

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
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
        size_t max_dim;
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

        unsigned long next_point_id() {
            unsigned long n = next_point.fetch_add(1, std::memory_order_relaxed);
            return n % points.size();
        }

        bool leaf() { return radius == 0.0 && !left && !right; }

        double dist(std::shared_ptr<node> n) {
            return distance(center, n->center) - radius - n->radius;
        }

        bool close_to(std::shared_ptr<node> n, double avg_radius) {
            double d = distance(center, n->center);
            double r = radius == 0.0 || n->radius == 0.0 ? avg_radius : radius + n->radius;
            return d < r;
        }

        bool include(std::shared_ptr<node> n) {
            if (radius < n->radius) return false;
            return (low <= n->low).min() && (upper >= n->upper).min();
        }

        void update_max_dim() {
            max_dim = 0;
            for (size_t i = 1; i < sizes.size(); i++) {
                if (sizes[i] > sizes[max_dim]) max_dim = i;
            }
        }

        size_t maxd() { return max_dim; }

        size_t split_point(size_t split_d, double split_val) {
            auto& v = dimensions[split_d];
            size_t l = 0, r = v.size();
            while (l < r) {
                size_t mid = l + (r - l) / 2;
                if (Set.get(split_d, v[mid]) == split_val) {
                    for (; mid > 0 && Set.get(split_d, v[mid - 1]) == split_val; mid--) continue;
                    return mid;
                }
                if (split_val < Set.get(split_d, v[mid]))
                    r = mid;
                else
                    l = mid + 1;
            }
            return l;
        }

        void distribute(size_t split_d) {
            std::vector<bool> in_left(Set.points.size(), false);
            for (size_t p : left->dimensions[split_d]) in_left[p] = true;
            for (size_t d = 0; d < dimensions.size(); d++) {
                if (d == split_d) continue;
                for (size_t p : dimensions[d]) {
                    if (in_left[p])
                        left->dimensions[d].push_back(p);
                    else
                        right->dimensions[d].push_back(p);
                }
            }
            Set.updateBox(left);
            Set.updateBox(right);
        }

        bool split_r() {
            if (radius == 0.0) return false;
            size_t split_d = maxd();
            double split_val = low[split_d] + sizes[split_d] / 2;
            size_t p = split_point(split_d, split_val);
            left = std::make_shared<node>(Set, cur_tree);
            left->id = cur_tree.next_id();
            right = std::make_shared<node>(Set, cur_tree);
            right->id = cur_tree.next_id();
            std::vector<size_t> vleft(dimensions[split_d].begin(), dimensions[split_d].begin() + p);
            std::vector<size_t> vright(dimensions[split_d].begin() + p, dimensions[split_d].end());
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
    };

    PointSet<INFO>& Set;
    std::shared_ptr<node> root;
    size_t node_ids;

    size_t next_id() { return ++node_ids; }

    static void seq_split(tree<INFO>* tree) { tree->root->split(); }

    tree(PointSet<INFO>& s, const std::function<void(tree<INFO>*)>& splitter = &seq_split)
        : Set(s), root(std::make_shared<node>(Set, *this)), node_ids(0) {
        root->dimensions = Set.dimensions;
        Set.updateBox(root);
        root->id = next_id();
        splitter(this);
    }
};

#endif /* TREE_HH_ */
