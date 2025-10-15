#include <stdint.h>

int main(int argc, char **argv) {
    uint32_t graph[6][6] = {
        {0, 1, 1, 0, 0, 1}, 
        {1, 0, 1, 1, 0, 0}, 
        {1, 1, 0, 1, 0, 1}, 
        {0, 1, 1, 0, 1, 0}, 
        {0, 0, 0, 1, 0, 1}, 
        {1, 0, 1, 0, 1, 0}
    };
    uint32_t distances[6] = {0};

    int visited[6] = {0};

    struct queue_entry_t {
        uint32_t node;
        int level;
    };

    struct queue_entry_t queue[1024] = {0};
    uint32_t head = 0, tail = 0;

    uint32_t curr = 0, dest = 5, level = 0;
    struct queue_entry_t entry = {
        .node = curr,
        .level = level,
    };
    queue[tail++] = entry;
    visited[curr] = 1;

    while(head != tail) {
        
        uint32_t node = queue[head].node;
        int l = queue[head++].level;
        if(node == 4) {
            level = l;
            break;
        }

        for(int i = 0; i < 6; i++) {
            if(!visited[i] && graph[node][i]) {
                entry.node = i;
                entry.level = l + 1;
                queue[tail++] = entry;
                visited[i] = 1;
            }
        }
    }
}