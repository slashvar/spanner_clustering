#include <gtest/gtest.h>

#include "clusters.hh"

TEST(UnionFindTest, FindSingleton) {
    std::vector<long> v(5, -1);
    for (long i = 0; i < 5; i++) EXPECT_EQ(find(v, i), i);
}

TEST(UnionFindTest, UnifyTwoElements) {
    std::vector<long> v(5, -1);
    EXPECT_TRUE(unify(v, 0, 1));
    EXPECT_EQ(find(v, 0), find(v, 1));
}

TEST(UnionFindTest, UnifyIdempotent) {
    std::vector<long> v(5, -1);
    EXPECT_TRUE(unify(v, 0, 1));
    EXPECT_FALSE(unify(v, 0, 1));
}

TEST(UnionFindTest, UnifyChain) {
    std::vector<long> v(5, -1);
    unify(v, 0, 1);
    unify(v, 1, 2);
    unify(v, 2, 3);
    long root = find(v, 0);
    EXPECT_EQ(find(v, 1), root);
    EXPECT_EQ(find(v, 2), root);
    EXPECT_EQ(find(v, 3), root);
    EXPECT_NE(find(v, 4), root);
}

TEST(UnionFindTest, PathCompression) {
    std::vector<long> v(4, -1);
    unify(v, 2, 3);
    unify(v, 1, 2);
    unify(v, 0, 1);
    long root = find(v, 0);
    EXPECT_EQ(find(v, 0), root);
    EXPECT_EQ(find(v, 1), root);
    EXPECT_EQ(find(v, 2), root);
    EXPECT_EQ(find(v, 3), root);
}

TEST(UnionFindTest, UnionByRank) {
    std::vector<long> v(6, -1);
    unify(v, 0, 1);
    unify(v, 0, 2);
    long big_root = find(v, 0);
    unify(v, 0, 3);
    EXPECT_EQ(find(v, 3), big_root);
}

TEST(UnionFindTest, LargeSet) {
    const int N = 1000;
    std::vector<long> v(N, -1);
    for (int i = 1; i < N; i++) unify(v, 0, i);
    long root = find(v, 0);
    for (int i = 0; i < N; i++) EXPECT_EQ(find(v, i), root);
}
