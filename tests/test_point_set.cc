#include "test_harness.hh"
#include "point_set.hh"

void norm_zero_vector() {
  sample v = {0.0, 0.0, 0.0};
  APPROX_EQ(norm(v), 0.0, 1e-12);
}

void norm_unit_vectors() {
  APPROX_EQ(norm(sample{1.0, 0.0, 0.0}), 1.0, 1e-12);
  APPROX_EQ(norm(sample{0.0, 1.0, 0.0}), 1.0, 1e-12);
  APPROX_EQ(norm(sample{0.0, 0.0, 1.0}), 1.0, 1e-12);
}

void norm_pythagorean() {
  APPROX_EQ(norm(sample{3.0, 4.0}), 5.0, 1e-12);
}

void distance_same_point() {
  sample p = {1.0, 2.0, 3.0};
  APPROX_EQ(distance(p, p), 0.0, 1e-12);
}

void distance_known_value() {
  sample a = {0.0, 0.0};
  sample b = {3.0, 4.0};
  APPROX_EQ(distance(a, b), 5.0, 1e-12);
}

void distance_symmetry() {
  sample a = {1.0, 2.0, 3.0};
  sample b = {4.0, 6.0, 8.0};
  APPROX_EQ(distance(a, b), distance(b, a), 1e-12);
}

void pointset_bounding_box_2d() {
  std::vector<sample> pts = {{0.0, 0.0}, {10.0, 0.0}, {0.0, 5.0}, {10.0, 5.0}};
  std::vector<int> info = {0, 1, 2, 3};
  PointSet<int> S(2, pts, info);

  APPROX_EQ(S.low[0], 0.0, 1e-12);
  APPROX_EQ(S.low[1], 0.0, 1e-12);
  APPROX_EQ(S.upper[0], 10.0, 1e-12);
  APPROX_EQ(S.upper[1], 5.0, 1e-12);
  APPROX_EQ(S.sizes[0], 10.0, 1e-12);
  APPROX_EQ(S.sizes[1], 5.0, 1e-12);
  APPROX_EQ(S.center[0], 5.0, 1e-12);
  APPROX_EQ(S.center[1], 2.5, 1e-12);
  // radius = norm({10, 5}) / 2
  double expected_radius = std::sqrt(100.0 + 25.0) / 2.0;
  APPROX_EQ(S.radius, expected_radius, 1e-12);
}

void pointset_single_point() {
  std::vector<sample> pts = {{7.0, 3.0}};
  std::vector<int> info = {0};
  PointSet<int> S(2, pts, info);

  APPROX_EQ(S.radius, 0.0, 1e-12);
  APPROX_EQ(S.low[0], 7.0, 1e-12);
  APPROX_EQ(S.low[1], 3.0, 1e-12);
  APPROX_EQ(S.upper[0], 7.0, 1e-12);
  APPROX_EQ(S.upper[1], 3.0, 1e-12);
}

void pointset_dimension_sorting() {
  // Points: (5,1), (2,4), (8,2), (1,3)
  std::vector<sample> pts = {{5.0, 1.0}, {2.0, 4.0}, {8.0, 2.0}, {1.0, 3.0}};
  std::vector<int> info = {0, 1, 2, 3};
  PointSet<int> S(2, pts, info);

  // dimensions[0] sorted by x: indices 3(x=1), 1(x=2), 0(x=5), 2(x=8)
  ASSERT_EQ(S.dimensions[0][0], 3u);
  ASSERT_EQ(S.dimensions[0][1], 1u);
  ASSERT_EQ(S.dimensions[0][2], 0u);
  ASSERT_EQ(S.dimensions[0][3], 2u);

  // dimensions[1] sorted by y: indices 0(y=1), 2(y=2), 3(y=3), 1(y=4)
  ASSERT_EQ(S.dimensions[1][0], 0u);
  ASSERT_EQ(S.dimensions[1][1], 2u);
  ASSERT_EQ(S.dimensions[1][2], 3u);
  ASSERT_EQ(S.dimensions[1][3], 1u);
}

void pointset_dist() {
  std::vector<sample> pts = {{0.0, 0.0}, {3.0, 4.0}, {6.0, 8.0}};
  std::vector<int> info = {0, 1, 2};
  PointSet<int> S(2, pts, info);

  APPROX_EQ(S.dist(0, 1), 5.0, 1e-12);
  APPROX_EQ(S.dist(0, 2), 10.0, 1e-12);
  APPROX_EQ(S.dist(1, 2), 5.0, 1e-12);
}

int main() {
  RUN_TEST(norm_zero_vector);
  RUN_TEST(norm_unit_vectors);
  RUN_TEST(norm_pythagorean);
  RUN_TEST(distance_same_point);
  RUN_TEST(distance_known_value);
  RUN_TEST(distance_symmetry);
  RUN_TEST(pointset_bounding_box_2d);
  RUN_TEST(pointset_single_point);
  RUN_TEST(pointset_dimension_sorting);
  RUN_TEST(pointset_dist);
  SUMMARY();
}
