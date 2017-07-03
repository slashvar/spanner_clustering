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

#ifndef POINT_SET_HH_
#define POINT_SET_HH_

#include <valarray>
#include <vector>

using sample = std::valarray<double>;

double norm(const sample& v) { return std::sqrt((v * v).sum()); }

double distance(const sample& v1, const sample& v2) { return norm(v1 - v2); }

template <typename INFO>
struct PointSet {
  PointSet(size_t d, std::vector<sample>& v, std::vector<INFO>& inf)
      : points(v),
        infos(inf),
        dim(d),
        dimensions(dim),
        low(dim),
        sizes(dim),
        upper(dim) {
    double time;
    for (size_t j = 0; j < points.size(); j++) {
      for (size_t i = 0; i < dim; i++) {
        dimensions[i].push_back(j);
      }
    }
    for (size_t i = 0; i < dim; i++) {
      std::sort(dimensions[i].begin(), dimensions[i].end(),
                [&](size_t u, size_t v) -> bool {
                  return points[u][i] < points[v][i];
                });
    }
    updateBox();
  }

  double dist(size_t u, size_t v) const {
    return distance(points[u], points[v]);
  }

  double get(size_t d, size_t p) { return points[p][d]; }

  template <typename BOX>
  void updateBox(BOX b) {
    for (size_t i = 0; i < dim; i++) {
      b->low[i] = get(i, b->dimensions[i].front());
      b->upper[i] = get(i, b->dimensions[i].back());
    }
    b->sizes = b->upper - b->low;
    b->center = b->low + b->sizes / 2.0;
    if (b->dimensions[0].size() == 1)
      b->radius = 0.0;
    else
      b->radius = norm(b->sizes) / 2;
  }

  void updateBox() { this->updateBox<PointSet<INFO>*>(this); }

  std::vector<sample> points;
  std::vector<INFO> infos;
  size_t dim;
  std::vector<std::vector<size_t>> dimensions;
  sample low, sizes, center, upper;
  double radius;
};

#endif /* POINT_SET_HH_ */
