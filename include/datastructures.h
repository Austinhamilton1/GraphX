#ifndef DS_H
#define DS_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_QUEUE_SIZE  1024
#define QUEUE_MASK      1023

/* Ring-buffer implemented FIFO data structure */
struct queue_t {
    uint32_t        data[MAX_QUEUE_SIZE];   // Data in the queue
    uint64_t        front;                  // Front of the queue (pop from this)
    uint64_t        back;                   // Back of the queue (push to this)
};

/* Type of frontier */
typedef enum {
    FRONTIER_QUEUE,             // FIFO type
    FRONTIER_PRIORITY_QUEUE,    // Priority type
    FRONTIER_BUCKET_QUEUE,      // Parallel type
    FRONTIER_SET,               // Disjoint set type
} frontier_type_t;

/* Frontier data structure. Can be queue or priority queue */
typedef struct frontier_t {
    frontier_type_t type;       // Backend type of the frontier
    union backend {             // Backend of the frontier (can be one of several types of backends)
        struct queue_t queue;   // Queue for things like BFS
    } backend;
} frontier_t;

/* Frontier functions */
int frontier_init(frontier_t *frontier, frontier_type_t type);
int frontier_push(frontier_t *frontier, uint32_t node);
int frontier_pop(frontier_t *frontier, uint32_t *node);
int frontier_empty(frontier_t *frontier);

#endif