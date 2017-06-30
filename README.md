# spanner_clustering: transform a set of points into a graph for clustering

This is a C++ implementation of geometric spanner using the Well Separated Pairs
Decomposition approach.

It also contains an experimental clustering algorithm that use the decomposition
and the fair split tree to construct clusters of points.

It uses C++14 and template only (so just header files).
