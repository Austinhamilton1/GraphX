#include <stdlib.h>

#include "graph.h"

/*
 * Get the weight between node u and node v.
 *
 * Arguments:
 *     graph_t *graph - The graph to query.
 *     uint32_t u - Source node.
 *     uint32_t v - Destination node.
 * Returns:
 *     uint32_t - The weight between u and v.
 */
uint32_t graph_get_weight(graph_t *graph, uint32_t u, uint32_t v) {
    // Start and end of the row
    uint32_t start = graph->row_index[u];
    uint32_t end = graph->row_index[u+1];

    // Columns are sorted by row, so search is binary
    int l = start, h = end-1;
    while (l <= h) {
        int m = (l + h) / 2;
        uint32_t val = graph->col_index[m];

        if (val == v)
            return graph->values[m];
        else if (val < v)
            l = m + 1;
        else
            h = m - 1;
    }

    // Value was not found
    return 0.0f;
}

/*
 * Check if graph has an edge between node u and node v.
 *
 * Arguments:
 *     graph_t *graph - Graph to query.
 *     uint32_t u - Source node.
 *     uint32_t v - Destination node.
 * Returns:
 *     int - 1 if there is an edge, 0 otherwise.
 */
int graph_has_edge(graph_t *graph, uint32_t u, uint32_t v) {
    // Start and end of row
    uint32_t start = graph->row_index[u];
    uint32_t end = graph->row_index[u+1];

    // Columns are sorted by row, so search is binary
    int l = start, h = end-1;
    while (l <= h) {
        int m = (l + h) / 2;
        uint32_t val = graph->col_index[m];

        if (val == v)
            return 1;
        else if (val < v)
            l = m + 1;
        else
            h = m - 1;
    }

    // Edge was not found
    return 0;
}

/*
 * Get all neighbors of node u.
 *
 * Arguments:
 *     graph_t *graph - The graph to query.
 *     unsigned in u - Query this node.
 *     int *size - Number of neighbors will go here.
 * Returns:
 *     uint32_t * - A pointer to the adjacency list of u.
 */
uint32_t *graph_get_neighbors(graph_t *graph, uint32_t u, int *size) {
    // Start and end of u row
    uint32_t start = graph->row_index[u];
    uint32_t end = graph->row_index[u+1];
    *size = (end - start);

    // Adjacency list
    return &graph->col_index[start];
}

/*
 * Initialize a graph iterator.
 *
 * Arguments:
 *     graph_iterator_t *it - The iterator to initialize.
 *     graph_t *graph - Reference graph.
 *     uint32_t node - Reference node.
 * Returns:
 *     int - 0 on success, -1 on error.
 */
int graph_init_iterator(graph_iterator_t *it, graph_t *graph, uint32_t node) {
    // NULL checks
    if(!it || !graph) return -1;

    it->graph = graph;
    it->it_node = node;
    it->node = node;
    it->start = graph->row_index[node];
    it->end = graph->row_index[node+1];
    it->current_index = 0;
    return 0;
}

/*
 * Get the next neighbor of a node.
 *
 * Arguments:
 *     graph_iterator_t *it - The iterator (should be initialized).
 *     uint32_t *neighbor - Save the neighbor here (set to NULL if unwanted).
 *     uint32_t *weight - Save the weight here (set to NULL if unwanted).
 * Returns:
 *     int - 0 on success, -1 on error/end.
 */
int graph_next_neighbor(graph_iterator_t *it, uint32_t *neighbor, uint32_t *weight) {
    // Check for null values
    if(!it || !it->graph) return -1;
    if(it->current_index >= (it->end - it->start)) return -1;

    // Get the node and weight of the next neighbor
    it->node = it->graph->col_index[it->current_index];
    it->weight = it->graph->values[it->current_index];

    // Save the neighbor if the caller wants it
    if(neighbor) *neighbor = it->graph->col_index[it->current_index];

    // Save the weight if the caller wants it
    if(weight) *weight = it->graph->values[it->current_index];

    // Update the index
    it->current_index++;
    return 0;
}

/*
 * Determine the degree of a node.
 *
 * Arguments:
 *     graph_t *graph - Graph to query.
 *     uint32_t node - Query this node.
 * Returns:
 *     size_t - The degree of node.
 */
size_t graph_degree(graph_t *graph, uint32_t node) {
    // Start and end of the row
    uint32_t start = graph->row_index[node];
    uint32_t end = graph->row_index[node+1];

    // Degree is how many non-zero values are in the row
    return (size_t)(end - start);
}