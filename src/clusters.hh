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

/* Looking for clusters */

#ifndef CLUSTERS_HH_
#define CLUSTERS_HH_

#include <cstdio>
#include <functional>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "point_set.hh"
#include "tree.hh"
#include "wspd.hh"

static inline long find(std::vector<long>& clusters, long v) {
  long r = v;
  for (; clusters[r] >= 0; r = clusters[r]) continue;
  while (clusters[v] >= 0) {
    long t = clusters[v];
    clusters[v] = r;
    v = t;
  }
  return r;
}

static inline bool unify(std::vector<long>& clusters, long u, long v) {
  long pu = find(clusters, u);
  long pv = find(clusters, v);
  if (pv == pu) return false;
  if (clusters[pv] < clusters[pu]) std::swap(pv, pu);
  if (clusters[pu] == clusters[pv]) clusters[pu] -= 1;
  clusters[pv] = pu;
  return true;
}

template <typename INFO>
struct clustering {
  using node = typename tree<INFO>::node;

  clustering(PointSet<INFO>& S, wspd<INFO>& w)
      : Set(S),
        W(w),
        split_tree(W.split_tree),
        nb_heads(0),
        nb_clusters(0),
        membership(Set.points.size()),
        avg_radius(0.0) {
    long count = find_heads(split_tree.root);
    nb_clusters = nb_heads;
    avg_radius = avg_radius / count;
    reconnect();
  }

  std::shared_ptr<node> parent(std::shared_ptr<node> n) {
    if (heads.find(n) != heads.end()) return n;
    for (auto v_ : heads) {
      auto v = v_.first;
      if (v->include(n)) return v;
    }
    return split_tree.root;  // should not happen
  }

  long parent_cluster(std::shared_ptr<node> n) {
    std::shared_ptr<node> p;
    if (!(p = n->cluster_parent.lock())) p = parent(n);
    if (heads.find(p) != heads.end()) return heads[p];
    return -1;  // should not happen
  }

  void assign_parent(std::shared_ptr<node> n, std::shared_ptr<node> p) {
    n->cluster_parent = p;
    if (n->left) {
      assign_parent(n->left, p);
      assign_parent(n->right, p);
    }
  }

  long find_heads(std::shared_ptr<node> n) {
    long non_null_head = 0;
    if (n->is_in_pair) {
      heads[n] = nb_heads;
      avg_radius += n->radius;
      non_null_head += n->radius > 0.0;
      nb_heads++;
      assign_parent(n, n);
      return non_null_head;
    }
    return find_heads(n->left) + find_heads(n->right);
  }

  void reconnect() {
    std::vector<long> clusters(heads.size(), -1);
    nb_clusters = nb_heads;
    for (auto cur = heads.begin(); cur != heads.end(); cur++) {
      auto u = cur->first;
      long uc = cur->second;
      auto sec = cur;
      sec++;
      for (; sec != heads.end(); sec++) {
        auto v = sec->first;
        long vc = sec->second;
# if 0
        if (u->close_to(v, avg_radius))
          if (unify(clusters, uc, vc)) nb_clusters--;
# endif
        if (!W.wellsepareted(u, v))
          if (unify(clusters, uc, vc)) nb_clusters--;
      }
    }
    std::unordered_map<long, long> cids;
    long cur = 0;
    for (auto& h_ : heads) {
      auto h = h_.first;
      long ch = find(clusters, h_.second);
      if (cids.find(ch) == cids.end()) {
        cids[ch] = cur;
        cur++;
      }
      h_.second = cids[ch];
      for (size_t p : h->points) membership[p] = cids[ch];
    }
  }

  double eval() {
    double q = 0;
    std::valarray<double> e(0.0, nb_clusters);
    std::valarray<double> a(0.0, nb_clusters);
    for (const auto& p : W.pairs) {
      auto p0 = parent_cluster(p.first);
      auto p1 = parent_cluster(p.second);
      double d = p.first->dist(p.second);
      d = 1 / (d * d);
      if (p0 == p1)
        e[p0] += d;
      else {
        a[p0] += d;
        a[p1] += d;
      }
    }
    double edges_sum = e.sum() + a.sum();
    e = e / (2 * edges_sum);
    a = a / (2 * edges_sum);
    q = (e + a * a).sum();
    return q;
  }

  void output_membership(FILE* file,
                         const std::function<std::string(INFO)>& tostr) {
    fprintf(file, "Id,Label,Cluster\n");
    for (size_t i = 0; i < membership.size(); i++) {
      fprintf(file, "%zu,%s,%ld\n", i, tostr(Set.infos[i]).c_str(),
              membership[i]);
    }
  }

  PointSet<INFO>& Set;
  wspd<INFO>& W;
  tree<INFO>& split_tree;
  std::unordered_map<std::shared_ptr<node>, long> heads;
  size_t nb_heads, nb_clusters;
  std::vector<size_t> membership;
  double avg_radius;
};

#endif /* CLUSTERS_HH_ */
