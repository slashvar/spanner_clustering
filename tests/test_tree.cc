#include <gtest/gtest.h>

#include <set>

#include "tree.hh"

TEST(TreeTest, SinglePointIsLeaf) {
    std::vector<sample> pts = {{7.0, 3.0}};
    std::vector<int> info = {0};
    PointSet<int> S(2, pts, info);
    tree<int> T(S);

    EXPECT_TRUE(T.root->leaf());
    EXPECT_FALSE(T.root->left);
    EXPECT_FALSE(T.root->right);
    EXPECT_NEAR(T.root->radius, 0.0, 1e-12);
}

TEST(TreeTest, TwoPointsSplit) {
    std::vector<sample> pts = {{0.0, 0.0}, {10.0, 0.0}};
    std::vector<int> info = {0, 1};
    PointSet<int> S(2, pts, info);
    tree<int> T(S);

    EXPECT_FALSE(T.root->leaf());
    ASSERT_NE(T.root->left, nullptr);
    ASSERT_NE(T.root->right, nullptr);
    EXPECT_TRUE(T.root->left->leaf());
    EXPECT_TRUE(T.root->right->leaf());
    // Each leaf has exactly one point
    EXPECT_EQ(T.root->left->points.size(), 1u);
    EXPECT_EQ(T.root->right->points.size(), 1u);
}

static void collect_leaves(std::shared_ptr<tree<int>::node> n, std::vector<size_t>& out) {
    if (n->leaf()) {
        for (size_t p : n->points) out.push_back(p);
        return;
    }
    if (n->left) collect_leaves(n->left, out);
    if (n->right) collect_leaves(n->right, out);
}

TEST(TreeTest, AllLeavesSinglePoint) {
    std::vector<sample> pts = {{1.0, 9.0}, {3.0, 2.0}, {7.0, 5.0}, {2.0, 8.0}, {9.0, 1.0},
                               {4.0, 6.0}, {6.0, 3.0}, {8.0, 7.0}, {5.0, 4.0}, {0.0, 0.0}};
    std::vector<int> info;
    for (int i = 0; i < (int)pts.size(); i++) info.push_back(i);
    PointSet<int> S(2, pts, info);
    tree<int> T(S);

    std::vector<size_t> leaf_points;
    collect_leaves(T.root, leaf_points);

    // Every point appears exactly once across all leaves
    std::set<size_t> unique(leaf_points.begin(), leaf_points.end());
    EXPECT_EQ(unique.size(), pts.size());
    EXPECT_EQ(leaf_points.size(), pts.size());
}

static void check_boxes(std::shared_ptr<tree<int>::node> n,
                        std::shared_ptr<tree<int>::node> parent) {
    // ASSERT_* halts the helper to avoid cascading failures into children
    // when a node's bounding box is already corrupt.
    size_t dim = n->low.size();
    for (size_t d = 0; d < dim; d++) {
        ASSERT_LE(n->low[d], n->upper[d]) << "low > upper at dim " << d;
        if (parent) {
            ASSERT_GE(n->low[d], parent->low[d] - 1e-9) << "child low < parent low at dim " << d;
            ASSERT_LE(n->upper[d], parent->upper[d] + 1e-9)
                << "child upper > parent upper at dim " << d;
        }
    }
    if (n->left) check_boxes(n->left, n);
    if (n->right) check_boxes(n->right, n);
}

TEST(TreeTest, BoundingBoxesConsistent) {
    std::vector<sample> pts = {{0.0, 0.0}, {10.0, 5.0}, {3.0, 8.0}, {7.0, 2.0}, {5.0, 5.0}};
    std::vector<int> info = {0, 1, 2, 3, 4};
    PointSet<int> S(2, pts, info);
    tree<int> T(S);

    check_boxes(T.root, nullptr);
}

static void collect_ids(std::shared_ptr<tree<int>::node> n, std::set<size_t>& ids) {
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

TEST(TreeTest, NodeIdsUnique) {
    std::vector<sample> pts = {{1.0, 9.0}, {3.0, 2.0}, {7.0, 5.0}, {2.0, 8.0}, {9.0, 1.0}};
    std::vector<int> info = {0, 1, 2, 3, 4};
    PointSet<int> S(2, pts, info);
    tree<int> T(S);

    std::set<size_t> ids;
    collect_ids(T.root, ids);
    EXPECT_EQ(ids.size(), count_nodes(T.root));
}

TEST(TreeTest, CollinearPointsSplitCorrectly) {
    std::vector<sample> pts = {{0.0, 0.0}, {5.0, 0.0}, {10.0, 0.0}};
    std::vector<int> info = {0, 1, 2};
    PointSet<int> S(2, pts, info);
    tree<int> T(S);

    ASSERT_FALSE(T.root->leaf());
    ASSERT_NE(T.root->left, nullptr);
    ASSERT_NE(T.root->right, nullptr);
    size_t left_sz = T.root->left->points.size();
    size_t right_sz = T.root->right->points.size();
    EXPECT_EQ(left_sz + right_sz, 3u);
    EXPECT_GE(left_sz, 1u);
    EXPECT_GE(right_sz, 1u);
}
