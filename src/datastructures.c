#include "datastructures.h"

/*
 * Initialize a queue data structure.
 *
 * Arguments:
 *     struct queue_t *queue - The queue to initialize. 
 * Returns:
 *     int - 0 on success, -1 on error.
 */
static int queue_init(struct queue_t *queue) {
    // NULL check
    if(!queue) return -1;

    // Initialize the queue
    for(size_t i = 0; i < MAX_QUEUE_SIZE; i++)
        queue->data[i] = 0;
    queue->front = 0;
    queue->back = 0;

    return 0;
}

/*
 * Push to the queue.
 *
 * Arguments:
 *     struct queue_t *queue - The queue to push to.
 *     int32_t node - Push this node to the queue.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
static int queue_push(struct queue_t *queue, int32_t node) {
    // NULL check
    if(!queue) return -1;

    // Check if queue is full
    if(queue->front - queue->back == MAX_QUEUE_SIZE) return -1;

    // Add node to queue
    queue->data[queue->front & QUEUE_MASK] = node;
    queue->front++;
    return 0;
}

/*
 * Pop from the queue.
 *
 * Arguments:
 *     struct queue_t *queue - Queue to pop from.
 *     int32_t *result - Put result here.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
static int queue_pop(struct queue_t *queue, int32_t *result) {
    // NULL check
    if(!queue || !result) return -1;

    // Check for empty queue
    if(queue->front == queue->back) return -1;

    // Pop from the queue
    *result = queue->data[queue->back & QUEUE_MASK];
    queue->back++;
    return 0;
}

/*
 * Check if the queue is empty.
 *
 * Arguments:
 *     struct queue_t *queue - The queue to check.
 * Returns:
 *     int - 1 if empty, 0 if not empty, -1 on error.
 */
static int queue_empty(struct queue_t *queue) {
    // NULL check
    if(!queue) return -1;

    // Check if queue is empty
    return queue->front == queue->back;
}

/*
 * Initialize a frontier with a specific backing type.
 *
 * Arguments:
 *     frontier_t *frontier - The frontier to initialize.
 *     frontier_type_t backend_type - The type of frontier to initialize.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int frontier_init(frontier_t *frontier, frontier_type_t backend_type) {
    // NULL checking
    if(!frontier) return -1;

    // Set the type
    frontier->type = backend_type;
    int result;

    // Initialize the correct backend
    switch (backend_type) {
    case FRONTIER_QUEUE:
        result = queue_init(&frontier->backend.queue);
        break;
    default:
        result = -1;
        break;
    }

    return result;
}

/*
 * Push a node into the frontier.
 *
 * Arguments:
 *     frontier_t *frontier - Push to this frontier.
 *     int32_t node - Push this node.
 * Returns:
 *     int - 0 on success, -1 on error.
 */
int frontier_push(frontier_t *frontier, int32_t node) {
    // NULL check
    if(!frontier) return -1;
    int result;

    // Push to the correct backend
    switch (frontier->type) {
    case FRONTIER_QUEUE:  
        result = queue_push(&frontier->backend.queue, node);
        break;
    default:
        result = -1;
        break;
    }

    return result;
}

/*
 * Pop from the frontier.
 *
 * Arguments:
 *     frontier_t *frontier - The frontier to pop from.
 *     int32_t *node - Pop result to this node.
 * Returns:
 *     int - 0 on success, -1 on error.
 */
int frontier_pop(frontier_t *frontier, int32_t *node) {
    // NULL check 
    if(!frontier) return -1;
    int result;

    // Pop from the correct backend
    switch (frontier->type) {
    case FRONTIER_QUEUE:
        result = queue_pop(&frontier->backend.queue, node);
        break;
    default:
        result = -1;
        break;
    }

    return result;
}

/*
 * Check if a frontier is empty.
 *
 * Arguments:
 *     froniter_t *frontier - The frontier to check.
 * Returns:
 *     int - 1 on empty, 0 on non-empty, -1 on error.
 */
int frontier_empty(frontier_t *frontier) {
    // NULL check
    if(!frontier) return -1;
    int result;

    // Check the correct backend
    switch(frontier->type) {
    case FRONTIER_QUEUE:
        result = queue_empty(&frontier->backend.queue);
        break;
    default:
        result = -1;
        break;
    }

    return result;
}