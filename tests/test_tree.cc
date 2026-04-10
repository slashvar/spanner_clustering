#include "test_harness.hh"
#include "tree.hh"
#include <set>

void single_point_is_leaf() {
  std::vector<sample> pts = {{7.0, 3.0}};
  std::vector<int> info = {0};
  PointSet<int> S(2, pts, info);
  tree<int> T(S);

  ASSERT_TRUE(T.root->leaf());
  ASSERT_TRUE(!T.root->left);
  ASSERT_TRUE(!T.root->right);
  APPROX_EQ(T.root->radius, 0.0, 1e-12);
}

void two_points_split() {
  std::vector<sample> pts = {{0.0, 0.0}, {10.0, 0.0}};
  std::vector<int> info = {0, 1};
  PointSet<int> S(2, pts, info);
  tree<int> T(S);

  ASSERT_TRUE(!T.root->leaf());
  ASSERT_TRUE(T.root->left != nullptr);
  ASSERT_TRUE(T.root->right != nullptr);
  ASSERT_TRUE(T.root->left->leaf());
  ASSERT_TRUE(T.root->right->leaf());
  // Each leaf has exactly one point
  ASSERT_EQ(T.root->left->points.size(), 1u);
  ASSERT_EQ(T.root->right->points.size(), 1u);
}

static void collect_leaves(std::shared_ptr<tree<int>::node> n,
                           std::vector<size_t>& out) {
  if (n->leaf()) {
    for (size_t p : n->points) out.push_back(p);
    return;
  }
  if (n->left) collect_leaves(n->left, out);
  if (n->right) collect_leaves(n->right, out);
}

void all_leaves_single_point() {
  std::vector<sample> pts = {
    {1.0, 9.0}, {3.0, 2.0}, {7.0, 5.0}, {2.0, 8.0}, {9.0, 1.0},
    {4.0, 6.0}, {6.0, 3.0}, {8.0, 7.0}, {5.0, 4.0}, {0.0, 0.0}
  };
  std::vector<int> info;
  for (int i = 0; i < (int)pts.size(); i++) info.push_back(i);
  PointSet<int> S(2, pts, info);
  tree<int> T(S);

  std::vector<size_t> leaf_points;
  collect_leaves(T.root, leaf_points);

  // Every point appears exactly once across all leaves
  std::set<size_t> unique(leaf_points.begin(), leaf_points.end());
  ASSERT_EQ(unique.size(), pts.size());
  ASSERT_EQ(leaf_points.size(), pts.size());
}

static void check_boxes(std::shared_ptr<tree<int>::node> n,
                         std::shared_ptr<tree<int>::node> parent) {
  size_t dim = n->low.size();
  for (size_t d = 0; d < dim; d++) {
    if (n->low[d] > n->upper[d])
      throw test_failure(__FILE__, __LINE__, "low > upper");
    if (parent) {
      if (n->low[d] < parent->low[d] - 1e-9)
        throw test_failure(__FILE__, __LINE__, "child low < parent low");
      if (n->upper[d] > parent->upper[d] + 1e-9)
        throw test_failure(__FILE__, __LINE__, "child upper > parent upper");
    }
  }
  if (n->left) check_boxes(n->left, n);
  if (n->right) check_boxes(n->right, n);
}

void bounding_boxes_consistent() {
  std::vector<sample> pts = {
    {0.0, 0.0}, {10.0, 5.0}, {3.0, 8.0}, {7.0, 2.0}, {5.0, 5.0}
  };
  std::vector<int> info = {0, 1, 2, 3, 4};
  PointSet<int> S(2, pts, info);
  tree<int> T(S);

  check_boxes(T.root, nullptr);
}

static void collect_ids(std::shared_ptr<tree<int>::node> n,
                        std::set<size_t>& ids) {
  ids.insert(n->id);
  if (n->left) collect_ids(n->left, ids);
  if (n->right) collect_ids(n->right, ids);
}

static size_t count_nodes(std::shared_ptr<tree<int>::node> n) {
  size_t c = 1;
  if (n->left) c += count_nodes(n->left);
  if (n->right) c += count_nodes(n->right);
  return c;
}

void node_ids_unique() {
  std::vector<sample> pts = {
    {1.0, 9.0}, {3.0, 2.0}, {7.0, 5.0}, {2.0, 8.0}, {9.0, 1.0}
  };
  std::vector<int> info = {0, 1, 2, 3, 4};
  PointSet<int> S(2, pts, info);
  tree<int> T(S);

  std::set<size_t> ids;
  collect_ids(T.root, ids);
  ASSERT_EQ(ids.size(), count_nodes(T.root));
}

void collinear_points_split_correctly() {
  // All points on x-axis: must split along dimension 0
  std::vector<sample> pts = {{0.0, 0.0}, {5.0, 0.0}, {10.0, 0.0}};
  std::vector<int> info = {0, 1, 2};
  PointSet<int> S(2, pts, info);
  tree<int> T(S);

  ASSERT_TRUE(!T.root->leaf());
  // One side should have 1 point, the other 2 (split at midpoint of [0,10])
  size_t left_sz = T.root->left->points.size();
  size_t right_sz = T.root->right->points.size();
  ASSERT_EQ(left_sz + right_sz, 3u);
  ASSERT_TRUE(left_sz >= 1 && right_sz >= 1);
}

int main() {
  RUN_TEST(single_point_is_leaf);
  RUN_TEST(two_points_split);
  RUN_TEST(all_leaves_single_point);
  RUN_TEST(bounding_boxes_consistent);
  RUN_TEST(node_ids_unique);
  RUN_TEST(collinear_points_split_correctly);
  SUMMARY();
}
