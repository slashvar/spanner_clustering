#include "clusters.hh"
#include "graph.hh"
#include "test_harness.hh"

static std::vector<sample> make_cluster(double cx, double cy, int n) {
    std::vector<sample> pts;
    for (int i = 0; i < n; i++) {
        double dx = (i % 3 - 1) * 0.1;
        double dy = (i / 3 - 1) * 0.1;
        pts.push_back({cx + dx, cy + dy});
    }
    return pts;
}

void three_clusters_detected() {
    auto c0 = make_cluster(0.0, 0.0, 10);
    auto c1 = make_cluster(100.0, 0.0, 10);
    auto c2 = make_cluster(0.0, 100.0, 10);

    std::vector<sample> pts;
    pts.insert(pts.end(), c0.begin(), c0.end());
    pts.insert(pts.end(), c1.begin(), c1.end());
    pts.insert(pts.end(), c2.begin(), c2.end());

    std::vector<unsigned> info;
    for (unsigned i = 0; i < pts.size(); i++) info.push_back(i);

    PointSet<unsigned> S(2, pts, info);
    auto g = graph<unsigned>::builder(S, 2.0)();
    clustering<unsigned> clusters(S, g.W);

    ASSERT_EQ(clusters.nb_clusters, 3u);

    // All points within a group share the same membership
    for (int i = 1; i < 10; i++) {
        ASSERT_EQ(clusters.membership[i], clusters.membership[0]);
        ASSERT_EQ(clusters.membership[10 + i], clusters.membership[10]);
        ASSERT_EQ(clusters.membership[20 + i], clusters.membership[20]);
    }

    // Different groups have different memberships
    ASSERT_TRUE(clusters.membership[0] != clusters.membership[10]);
    ASSERT_TRUE(clusters.membership[0] != clusters.membership[20]);
    ASSERT_TRUE(clusters.membership[10] != clusters.membership[20]);
}

void graph_edges_valid() {
    auto c0 = make_cluster(0.0, 0.0, 10);
    auto c1 = make_cluster(100.0, 0.0, 10);

    std::vector<sample> pts;
    pts.insert(pts.end(), c0.begin(), c0.end());
    pts.insert(pts.end(), c1.begin(), c1.end());

    std::vector<unsigned> info;
    for (unsigned i = 0; i < pts.size(); i++) info.push_back(i);

    PointSet<unsigned> S(2, pts, info);
    auto g = graph<unsigned>::builder(S, 2.0)();

    ASSERT_TRUE(g.edges.size() > 0);
    for (const auto& e : g.edges) {
        // Edges are normalized: u < v
        ASSERT_TRUE(e.first < e.second);
        // Indices in range
        ASSERT_TRUE(e.first < pts.size());
        ASSERT_TRUE(e.second < pts.size());
    }
}

void single_point_graph_only() {
    // NOTE: clustering crashes on a single point (find_heads recurses into
    // null children when root is a leaf with no WSPD pairs). Only test the
    // graph layer here.
    std::vector<sample> pts = {{5.0, 5.0}};
    std::vector<unsigned> info = {0};
    PointSet<unsigned> S(2, pts, info);
    auto g = graph<unsigned>::builder(S, 2.0)();

    ASSERT_EQ(g.edges.size(), 0u);
    ASSERT_EQ(g.W.pairs.size(), 0u);
}

void two_points_cluster_sanity() {
    std::vector<sample> pts = {{0.0, 0.0}, {1.0, 0.0}};
    std::vector<unsigned> info = {0, 1};
    PointSet<unsigned> S(2, pts, info);
    auto g = graph<unsigned>::builder(S, 2.0)();
    clustering<unsigned> clusters(S, g.W);

    // Two points should produce valid membership and at most 2 clusters
    ASSERT_EQ(clusters.membership.size(), 2u);
    ASSERT_TRUE(clusters.nb_clusters >= 1 && clusters.nb_clusters <= 2);
}

int main() {
    RUN_TEST(three_clusters_detected);
    RUN_TEST(graph_edges_valid);
    RUN_TEST(single_point_graph_only);
    RUN_TEST(two_points_cluster_sanity);
    SUMMARY();
}
