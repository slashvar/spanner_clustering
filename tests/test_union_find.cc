#include "clusters.hh"
#include "test_harness.hh"

void find_singleton() {
    std::vector<long> v(5, -1);
    for (long i = 0; i < 5; i++) ASSERT_EQ(find(v, i), i);
}

void unify_two_elements() {
    std::vector<long> v(5, -1);
    ASSERT_TRUE(unify(v, 0, 1));
    ASSERT_EQ(find(v, 0), find(v, 1));
}

void unify_idempotent() {
    std::vector<long> v(5, -1);
    ASSERT_TRUE(unify(v, 0, 1));
    ASSERT_TRUE(!unify(v, 0, 1));
}

void unify_chain() {
    std::vector<long> v(5, -1);
    unify(v, 0, 1);
    unify(v, 1, 2);
    unify(v, 2, 3);
    long root = find(v, 0);
    ASSERT_EQ(find(v, 1), root);
    ASSERT_EQ(find(v, 2), root);
    ASSERT_EQ(find(v, 3), root);
    // element 4 is still separate
    ASSERT_TRUE(find(v, 4) != root);
}

void path_compression() {
    std::vector<long> v(4, -1);
    // Build a chain: 0->1->2->3
    unify(v, 2, 3);
    unify(v, 1, 2);
    unify(v, 0, 1);
    long root = find(v, 0);
    // After find(0), path compression should make 0 point directly to root
    // Check that a second find is still consistent
    ASSERT_EQ(find(v, 0), root);
    ASSERT_EQ(find(v, 1), root);
    ASSERT_EQ(find(v, 2), root);
    ASSERT_EQ(find(v, 3), root);
}

void union_by_rank() {
    std::vector<long> v(6, -1);
    // Build a larger set {0,1,2} and a smaller set {3}
    unify(v, 0, 1);
    unify(v, 0, 2);
    // Now unify with singleton 3 — the larger tree's root should remain root
    long big_root = find(v, 0);
    unify(v, 0, 3);
    ASSERT_EQ(find(v, 3), big_root);
}

void large_set() {
    const int N = 1000;
    std::vector<long> v(N, -1);
    for (int i = 1; i < N; i++) unify(v, 0, i);
    long root = find(v, 0);
    for (int i = 0; i < N; i++) ASSERT_EQ(find(v, i), root);
}

int main() {
    RUN_TEST(find_singleton);
    RUN_TEST(unify_two_elements);
    RUN_TEST(unify_idempotent);
    RUN_TEST(unify_chain);
    RUN_TEST(path_compression);
    RUN_TEST(union_by_rank);
    RUN_TEST(large_set);
    SUMMARY();
}
