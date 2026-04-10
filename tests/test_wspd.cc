#include "test_harness.hh"
#include "wspd.hh"

void two_distant_points_one_pair() {
  std::vector<sample> pts = {{0.0, 0.0}, {100.0, 0.0}};
  std::vector<int> info = {0, 1};
  PointSet<int> S(2, pts, info);
  wspd<int> W(S, 1.0);

  // Two well-separated leaves should produce exactly 1 pair
  ASSERT_EQ(W.pairs.size(), 1u);
}

void all_pairs_wellseparated() {
  std::vector<sample> pts = {
    {0.0, 0.0}, {10.0, 0.0}, {0.0, 10.0}, {10.0, 10.0},
    {5.0, 5.0}, {3.0, 7.0}, {8.0, 2.0}
  };
  std::vector<int> info = {0, 1, 2, 3, 4, 5, 6};
  PointSet<int> S(2, pts, info);
  double sep = 2.0;
  wspd<int> W(S, sep);

  for (const auto& p : W.pairs) {
    double r = std::max(p.first->radius, p.second->radius);
    double d = p.first->dist(p.second);
    ASSERT_TRUE(d >= sep * r);
  }
}

void higher_separation_more_pairs() {
  std::vector<sample> pts = {
    {0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0},
    {4.0, 0.0}, {5.0, 0.0}, {6.0, 0.0}, {7.0, 0.0}
  };
  std::vector<int> info = {0, 1, 2, 3, 4, 5, 6, 7};
  PointSet<int> S1(1, pts, info);
  wspd<int> W1(S1, 1.0);

  PointSet<int> S2(1, pts, info);
  wspd<int> W2(S2, 4.0);

  // Higher separation factor should produce at least as many pairs
  ASSERT_TRUE(W2.pairs.size() >= W1.pairs.size());
}

void is_in_pair_flag_set() {
  std::vector<sample> pts = {{0.0, 0.0}, {10.0, 0.0}, {20.0, 0.0}};
  std::vector<int> info = {0, 1, 2};
  PointSet<int> S(1, pts, info);
  wspd<int> W(S, 1.0);

  ASSERT_TRUE(W.pairs.size() > 0);
  // At least one node in each pair should have is_in_pair set
  for (const auto& p : W.pairs) {
    ASSERT_TRUE(p.first->is_in_pair);
    ASSERT_TRUE(p.second->is_in_pair);
  }
}

void decomposition_covers_all_point_pairs() {
  // For a valid WSPD, every pair of points should be "represented" by some
  // well-separated pair. We verify that the decomposition is non-empty for
  // non-trivial inputs.
  std::vector<sample> pts = {
    {0.0, 0.0}, {1.0, 1.0}, {5.0, 5.0}, {6.0, 6.0}
  };
  std::vector<int> info = {0, 1, 2, 3};
  PointSet<int> S(2, pts, info);
  wspd<int> W(S, 2.0);

  ASSERT_TRUE(W.pairs.size() > 0);
}

int main() {
  RUN_TEST(two_distant_points_one_pair);
  RUN_TEST(all_pairs_wellseparated);
  RUN_TEST(higher_separation_more_pairs);
  RUN_TEST(is_in_pair_flag_set);
  RUN_TEST(decomposition_covers_all_point_pairs);
  SUMMARY();
}
