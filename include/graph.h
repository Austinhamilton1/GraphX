#ifndef GRAPH_H
#define GRAPH_H

#include "datastructures.h"

#define MAX 65536

/* Represents a graph (compressed sparse row format) */
typedef struct graph_t {
    int32_t     n;              // nodes
    int32_t     col_index[MAX]; // Index of value in columna
    int32_t     row_index[MAX]; // Index of value in rows
    int32_t     values[MAX];    // Value (weights)
} graph_t;

/* Used to efficiently iterate a node's neighbors */
typedef struct graph_iterator_t {
    graph_t     *graph;         // The graph to iterate
    int32_t     it_node;        // The node to iterate
    int32_t     node;           // The next node
    int32_t     weight;         // The weight of the node
    size_t      start;          // Start point of iteration
    size_t      end;            // End point of the iteration
    size_t      current_index;  // The current index in the iteration
} graph_iterator_t;

/* Edge functions  */
int32_t graph_get_weight(graph_t *graph, int32_t u, int32_t v);
int graph_has_edge(graph_t *graph, int32_t u, int32_t v);

/* Get the neighbors of node u */
uint32_t *graph_get_neighbors(graph_t *graph, int32_t u, int *size);
int graph_init_iterator(graph_iterator_t *it, graph_t *graph, int32_t node);
int graph_next_neighbor(graph_iterator_t *it, int32_t *neighbor, int32_t *weight);

/* Get the degree of a node */
size_t graph_degree(graph_t *graph, int32_t node);

#endif