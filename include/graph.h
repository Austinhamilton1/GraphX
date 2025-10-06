#ifndef GRAPH_H
#define GRAPH_H

#include "datastructures.h"

/* Represents a graph (compressed sparse row format) */
typedef struct graph_t {
    unsigned int    m;              // Rows
    unsigned int    n;              // Columns
    unsigned int    *col_index;     // Index of value in columna
    unsigned int    *row_index;     // Index of value in rows
    float           *values;        // Value (weights)
} graph_t;

/* Used to efficiently iterate a node's neighbors */
typedef struct graph_iterator_t {
    graph_t         *graph;         // The graph to iterate
    unsigned int    it_node;        // The node to iterate
    unsigned int    node;           // The next node
    float           weight;         // The weight of the node
    size_t          start;          // Start point of iteration
    size_t          end;            // End point of the iteration
    size_t          current_index;  // The current index in the iteration
} graph_iterator_t;

/* Edge functions  */
float graph_get_weight(graph_t *graph, unsigned int u, unsigned int v);
int graph_has_edge(graph_t *graph, unsigned int u, unsigned int v);

/* Get the neighbors of node u */
unsigned int *graph_get_neighbors(graph_t *graph, unsigned int u, int *size);
int graph_init_iterator(graph_iterator_t *it, graph_t *graph, unsigned int node);
int graph_next_neighbor(graph_iterator_t *it, unsigned int *neighbor, float *weight);

/* Get the degree of a node */
size_t graph_degree(graph_t *graph, unsigned int node);

#endif