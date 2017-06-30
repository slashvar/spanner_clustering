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
