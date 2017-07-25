import csv
from sys import stdout, stderr, argv
from math import log1p

import spanner_graph

def load_data(fname):
    full_data = []
    with open(fname) as f:
        for line in f:
            data = line.strip().split(',')
            p = list(map(lambda x : float(x), data[2:]))
            full_data.append({'name': data[0], 'class': int(data[1]), 'point': p})
    return full_data

def check_points(full_data):
    mem = dict()
    to_remove = [False] * len(full_data)
    for i in range(len(full_data)):
        p = ','.join(list(map(lambda x : str(x), full_data[i]['point'])))
        if p not in mem:
            mem[p] = True
        else:
            to_remove[i] = True
    end = len(full_data)
    for k in range(end, 0, -1):
        i = k - 1
        if to_remove[i]:
            end -= 1
            full_data[i], full_data[end] = full_data[end], full_data[i]
    return full_data[:end]

def prepare(full_data):
    points = []
    orig_clusters = []
    for p in full_data:
        points.append(p['point'])
        orig_clusters.append(p['class'])
    return len(points[0]), points, orig_clusters

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
    fname = 'points.csv'
    if len(argv) > 1:
        fname = argv[1]
    dim, points, orig_clusters = prepare(check_points(load_data(fname)))
    g = spanner_graph.SpannerGraph(dim, points, 8)
    print("found clusters:", g.number_of_clusters, file=stderr)
    generate_dot(g, orig_clusters)


