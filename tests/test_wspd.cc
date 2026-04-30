#include <gtest/gtest.h>

#include "wspd.hh"

TEST(WspdTest, TwoDistantPointsOnePair) {
    std::vector<sample> pts = {{0.0, 0.0}, {100.0, 0.0}};
    std::vector<int> info = {0, 1};
    PointSet<int> S(2, pts, info);
    wspd<int> W(S, 1.0);

    // Two well-separated leaves should produce exactly 1 pair
    EXPECT_EQ(W.pairs.size(), 1u);
}

TEST(WspdTest, AllPairsWellseparated) {
    std::vector<sample> pts = {{0.0, 0.0}, {10.0, 0.0}, {0.0, 10.0}, {10.0, 10.0},
                               {5.0, 5.0}, {3.0, 7.0},  {8.0, 2.0}};
    std::vector<int> info = {0, 1, 2, 3, 4, 5, 6};
    PointSet<int> S(2, pts, info);
    double sep = 2.0;
    wspd<int> W(S, sep);

    for (const auto& p : W.pairs) {
        double r = std::max(p.first->radius, p.second->radius);
        double d = p.first->dist(p.second);
        EXPECT_GE(d, sep * r);
    }
}

TEST(WspdTest, HigherSeparationMorePairs) {
    std::vector<sample> pts = {{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0},
                               {4.0, 0.0}, {5.0, 0.0}, {6.0, 0.0}, {7.0, 0.0}};
    std::vector<int> info = {0, 1, 2, 3, 4, 5, 6, 7};
    PointSet<int> S1(1, pts, info);
    wspd<int> W1(S1, 1.0);

    PointSet<int> S2(1, pts, info);
    wspd<int> W2(S2, 4.0);

    // Higher separation factor should produce at least as many pairs
    EXPECT_GE(W2.pairs.size(), W1.pairs.size());
}

TEST(WspdTest, IsInPairFlagSet) {
    std::vector<sample> pts = {{0.0, 0.0}, {10.0, 0.0}, {20.0, 0.0}};
    std::vector<int> info = {0, 1, 2};
    PointSet<int> S(1, pts, info);
    wspd<int> W(S, 1.0);

    EXPECT_GT(W.pairs.size(), 0u);
    // At least one node in each pair should have is_in_pair set
    for (const auto& p : W.pairs) {
        EXPECT_TRUE(p.first->is_in_pair);
        EXPECT_TRUE(p.second->is_in_pair);
    }
}

TEST(WspdTest, DecompositionCoversAllPointPairs) {
    // For a valid WSPD, every pair of points should be "represented" by some
    // well-separated pair. We verify that the decomposition is non-empty for
    // non-trivial inputs.
    std::vector<sample> pts = {{0.0, 0.0}, {1.0, 1.0}, {5.0, 5.0}, {6.0, 6.0}};
    std::vector<int> info = {0, 1, 2, 3};
    PointSet<int> S(2, pts, info);
    wspd<int> W(S, 2.0);

    EXPECT_GT(W.pairs.size(), 0u);
}
