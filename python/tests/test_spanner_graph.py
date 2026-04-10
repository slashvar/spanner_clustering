import pytest
import spanner_graph


@pytest.fixture(scope="class")
def graph_3clusters(tight_clusters_2d):
    return spanner_graph.SpannerGraph(2, tight_clusters_2d, 2.0)


class TestConstruction:
    def test_basic_construction(self, graph_3clusters):
        g = graph_3clusters
        assert hasattr(g, 'edges')
        assert hasattr(g, 'membership')
        assert hasattr(g, 'number_of_clusters')

    def test_membership_length(self, graph_3clusters, tight_clusters_2d):
        assert len(graph_3clusters.membership) == len(tight_clusters_2d)

    def test_edge_format(self, graph_3clusters):
        assert len(graph_3clusters.edges) > 0
        for e in graph_3clusters.edges:
            assert len(e) == 3
            src, dst, dist = e
            assert isinstance(src, int)
            assert isinstance(dst, int)
            assert isinstance(dist, float)
            assert dist >= 0.0

    def test_edges_in_range(self, graph_3clusters, tight_clusters_2d):
        n = len(tight_clusters_2d)
        for src, dst, _ in graph_3clusters.edges:
            assert 0 <= src < n
            assert 0 <= dst < n


class TestClustering:
    def test_three_cluster_count(self, graph_3clusters):
        assert graph_3clusters.number_of_clusters == 3

    def test_membership_consistency(self, graph_3clusters):
        m = graph_3clusters.membership
        for i in range(1, 10):
            assert m[i] == m[0]
            assert m[10 + i] == m[10]
            assert m[20 + i] == m[20]
        assert m[0] != m[10]
        assert m[0] != m[20]
        assert m[10] != m[20]


class TestErrorHandling:
    def test_wrong_dimension_raises(self):
        with pytest.raises(spanner_graph.error):
            spanner_graph.SpannerGraph(3, [[0.0, 0.0]], 2.0)

    def test_non_sequence_raises(self):
        with pytest.raises((spanner_graph.error, TypeError)):
            spanner_graph.SpannerGraph(2, "not a list", 2.0)

    def test_non_float_coords_raises(self):
        with pytest.raises(spanner_graph.error):
            spanner_graph.SpannerGraph(2, [["a", "b"]], 2.0)

    def test_wrong_arg_types_raises(self):
        with pytest.raises(TypeError):
            spanner_graph.SpannerGraph("two", [[0.0, 0.0]], 2.0)
