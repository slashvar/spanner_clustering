import pytest


@pytest.fixture(scope="session")
def tight_clusters_2d():
    """Three well-separated clusters of 10 points each in 2D."""
    clusters = []
    for cx, cy in [(0.0, 0.0), (100.0, 0.0), (0.0, 100.0)]:
        for i in range(10):
            dx = (i % 3 - 1) * 0.1
            dy = (i // 3 - 1) * 0.1
            clusters.append([cx + dx, cy + dy])
    return clusters


