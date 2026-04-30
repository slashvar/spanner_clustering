#include <gtest/gtest.h>

#include "clusters.hh"
#include "graph.hh"

static std::vector<sample> make_cluster(double cx, double cy, int n) {
    std::vector<sample> pts;
    for (int i = 0; i < n; i++) {
        double dx = (i % 3 - 1) * 0.1;
        double dy = (i / 3 - 1) * 0.1;
        pts.push_back({cx + dx, cy + dy});
    }
    return pts;
}

TEST(IntegrationTest, ThreeClustersDetected) {
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

    EXPECT_EQ(clusters.nb_clusters, 3u);

    // All points within a group share the same membership
    for (int i = 1; i < 10; i++) {
        EXPECT_EQ(clusters.membership[i], clusters.membership[0]);
        EXPECT_EQ(clusters.membership[10 + i], clusters.membership[10]);
        EXPECT_EQ(clusters.membership[20 + i], clusters.membership[20]);
    }

    // Different groups have different memberships
    EXPECT_NE(clusters.membership[0], clusters.membership[10]);
    EXPECT_NE(clusters.membership[0], clusters.membership[20]);
    EXPECT_NE(clusters.membership[10], clusters.membership[20]);
}

TEST(IntegrationTest, GraphEdgesValid) {
    auto c0 = make_cluster(0.0, 0.0, 10);
    auto c1 = make_cluster(100.0, 0.0, 10);

    std::vector<sample> pts;
    pts.insert(pts.end(), c0.begin(), c0.end());
    pts.insert(pts.end(), c1.begin(), c1.end());

    std::vector<unsigned> info;
    for (unsigned i = 0; i < pts.size(); i++) info.push_back(i);

    PointSet<unsigned> S(2, pts, info);
    auto g = graph<unsigned>::builder(S, 2.0)();

    EXPECT_GT(g.edges.size(), 0u);
    for (const auto& e : g.edges) {
        // Edges are normalized: u < v
        EXPECT_LT(e.first, e.second);
        // Indices in range
        EXPECT_LT(e.first, pts.size());
        EXPECT_LT(e.second, pts.size());
    }
}

TEST(IntegrationTest, SinglePointGraphOnly) {
    std::vector<sample> pts = {{5.0, 5.0}};
    std::vector<unsigned> info = {0};
    PointSet<unsigned> S(2, pts, info);
    auto g = graph<unsigned>::builder(S, 2.0)();
    clustering<unsigned> clusters(S, g.W);

    EXPECT_EQ(g.edges.size(), 0u);
    EXPECT_EQ(g.W.pairs.size(), 0u);
    EXPECT_EQ(clusters.membership.size(), 1u);
}

TEST(IntegrationTest, TwoPointsClusterSanity) {
    std::vector<sample> pts = {{0.0, 0.0}, {1.0, 0.0}};
    std::vector<unsigned> info = {0, 1};
    PointSet<unsigned> S(2, pts, info);
    auto g = graph<unsigned>::builder(S, 2.0)();
    clustering<unsigned> clusters(S, g.W);

    // Two points should produce valid membership and at most 2 clusters
    EXPECT_EQ(clusters.membership.size(), 2u);
    EXPECT_GE(clusters.nb_clusters, 1u);
    EXPECT_LE(clusters.nb_clusters, 2u);
}
