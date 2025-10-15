#include <stdint.h>

int main(int argc, char **argv) {
    uint32_t graph[18][3] = {
        {0, 1, 7},
        {0, 2, 9},
        {0, 5, 14},
        {1, 0, 7},
        {1, 2, 10},
        {1, 3, 15},
        {2, 0, 9},
        {2, 1, 10},
        {2, 3, 11},
        {2, 5, 2},
        {3, 1, 15},
        {3, 2, 11},
        {3, 4, 6},
        {4, 3, 6},
        {4, 5, 9},
        {5, 0, 14},
        {5, 2, 2},
        {5, 4, 9},
    };

    uint32_t src = 0;
    uint32_t dist[6] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

    dist[src] = 0;

    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 18; j++) {
            uint32_t u = graph[j][0], v = graph[j][1], wt = graph[j][2];
            if(dist[u] != 0xFFFF && dist[u] + wt < dist[v]) {
                dist[v] = dist[u] + wt;
            }
        }
    }
}