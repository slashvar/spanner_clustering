from random import uniform
from sys import stdout

import spanner_graph

def generate_rand_points(center, radius, amount):
    """
    generate a group of amount points contains in an hypersphere defined by center and radius
    """
    points = []
    for i in range(amount):
        points.append(list(map(lambda x : x + uniform(-radius, radius), center)))
    return points

def generate_cloud():
    """
    Generate 9 groups of random points with membership
    """
    amount = 1000
    radius = 2.0
    space = 15
    points = []
    membership = []
    for i in range(9):
        center = (i // 3) * space * radius, (i % 3) * space * radius
        points += generate_rand_points(center, radius, amount)
        membership += [i] * amount
    return points, membership

def generate_dot(g, orig_clusters, file=stdout):
    print("graph {", file=file)
    vertices = [''] * len(orig_clusters)
    for i in range(len(orig_clusters)):
        vertices[i] = "vert%d" % i
        print("  %s [label=%d class=%d cluster=%d];" % (vertices[i], i,
            orig_clusters[i], g.membership[i]), file=file)
    for e in g.edges:
        src, dst, d = e
        edge = "  %s -- %s [weight=%f dist=%f];" % (vertices[src],
                vertices[dst], 1 / (d*d), d)
        print(edge, file=file)
    print('}', file=file)

if __name__ == '__main__':
    points, orig_clusters = generate_cloud()
    g = spanner_graph.SpannerGraph(2, points, 32)
    generate_dot(g, orig_clusters)

