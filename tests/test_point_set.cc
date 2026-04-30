#include <gtest/gtest.h>

#include "point_set.hh"

TEST(PointSetTest, NormZeroVector) {
    sample v = {0.0, 0.0, 0.0};
    EXPECT_NEAR(norm(v), 0.0, 1e-12);
}

TEST(PointSetTest, NormUnitVectors) {
    EXPECT_NEAR(norm(sample{1.0, 0.0, 0.0}), 1.0, 1e-12);
    EXPECT_NEAR(norm(sample{0.0, 1.0, 0.0}), 1.0, 1e-12);
    EXPECT_NEAR(norm(sample{0.0, 0.0, 1.0}), 1.0, 1e-12);
}

TEST(PointSetTest, NormPythagorean) { EXPECT_NEAR(norm(sample{3.0, 4.0}), 5.0, 1e-12); }

TEST(PointSetTest, DistanceSamePoint) {
    sample p = {1.0, 2.0, 3.0};
    EXPECT_NEAR(distance(p, p), 0.0, 1e-12);
}

TEST(PointSetTest, DistanceKnownValue) {
    sample a = {0.0, 0.0};
    sample b = {3.0, 4.0};
    EXPECT_NEAR(distance(a, b), 5.0, 1e-12);
}

TEST(PointSetTest, DistanceSymmetry) {
    sample a = {1.0, 2.0, 3.0};
    sample b = {4.0, 6.0, 8.0};
    EXPECT_NEAR(distance(a, b), distance(b, a), 1e-12);
}

TEST(PointSetTest, BoundingBox2d) {
    std::vector<sample> pts = {{0.0, 0.0}, {10.0, 0.0}, {0.0, 5.0}, {10.0, 5.0}};
    std::vector<int> info = {0, 1, 2, 3};
    PointSet<int> S(2, pts, info);

    EXPECT_NEAR(S.low[0], 0.0, 1e-12);
    EXPECT_NEAR(S.low[1], 0.0, 1e-12);
    EXPECT_NEAR(S.upper[0], 10.0, 1e-12);
    EXPECT_NEAR(S.upper[1], 5.0, 1e-12);
    EXPECT_NEAR(S.sizes[0], 10.0, 1e-12);
    EXPECT_NEAR(S.sizes[1], 5.0, 1e-12);
    EXPECT_NEAR(S.center[0], 5.0, 1e-12);
    EXPECT_NEAR(S.center[1], 2.5, 1e-12);
    // radius = norm({10, 5}) / 2
    double expected_radius = std::sqrt(100.0 + 25.0) / 2.0;
    EXPECT_NEAR(S.radius, expected_radius, 1e-12);
}

TEST(PointSetTest, SinglePoint) {
    std::vector<sample> pts = {{7.0, 3.0}};
    std::vector<int> info = {0};
    PointSet<int> S(2, pts, info);

    EXPECT_NEAR(S.radius, 0.0, 1e-12);
    EXPECT_NEAR(S.low[0], 7.0, 1e-12);
    EXPECT_NEAR(S.low[1], 3.0, 1e-12);
    EXPECT_NEAR(S.upper[0], 7.0, 1e-12);
    EXPECT_NEAR(S.upper[1], 3.0, 1e-12);
}

TEST(PointSetTest, DimensionSorting) {
    std::vector<sample> pts = {{5.0, 1.0}, {2.0, 4.0}, {8.0, 2.0}, {1.0, 3.0}};
    std::vector<int> info = {0, 1, 2, 3};
    PointSet<int> S(2, pts, info);

    EXPECT_EQ(S.dimensions[0][0], 3u);
    EXPECT_EQ(S.dimensions[0][1], 1u);
    EXPECT_EQ(S.dimensions[0][2], 0u);
    EXPECT_EQ(S.dimensions[0][3], 2u);

    EXPECT_EQ(S.dimensions[1][0], 0u);
    EXPECT_EQ(S.dimensions[1][1], 2u);
    EXPECT_EQ(S.dimensions[1][2], 3u);
    EXPECT_EQ(S.dimensions[1][3], 1u);
}

TEST(PointSetTest, Dist) {
    std::vector<sample> pts = {{0.0, 0.0}, {3.0, 4.0}, {6.0, 8.0}};
    std::vector<int> info = {0, 1, 2};
    PointSet<int> S(2, pts, info);

    EXPECT_NEAR(S.dist(0, 1), 5.0, 1e-12);
    EXPECT_NEAR(S.dist(0, 2), 10.0, 1e-12);
    EXPECT_NEAR(S.dist(1, 2), 5.0, 1e-12);
}
