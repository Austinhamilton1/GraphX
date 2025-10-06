#ifndef GRAPH_X_H
#define GRAPH_X_H

#include <stdint.h>
#include "datastructures.h"
#include "graph.h"

#define VM_MEMORY_SIZE 65536

#define VM_CONTINUE 1
#define VM_HALT     0
#define VM_ERROR    -1

#define REGISTER_ARG_MASK   0x000001FF
#define IMMEDIATE_ARG_MASK  0x07FFFFFF
#define CONSTANT_ARG_MASK   0x0003FFFF

#define ARG1(vm) (vm->A0)
#define ARG2(vm) (vm->A1)
#define ARG3(vm) (vm->A2)

#define IMM(vm) (vm->A0)

typedef enum {
    HALT,       // End of program

    /* Immediate instructions */
    JMP,        // Unconditional branch
    BZ,         // Conditional branch if zero

    /* Memory instructions */
    LD,         // Load from memory
    ST,         // Store to memory
    MOV,        // Constant register function

    /* Register instructions */
    ADD,        // Addition
    SUB,        // Subtraction
    ITER,       // Iterator
    NEIGHBOR,   // Next neighbor
    DEGREE,     // Degree of node
    FPUSH,      // Frontier push
    FPOP,       // Frontier pop
    FEMPTY,     // Frontier empty
    CMP,        // Compare two registers
} instruction;

/* Graph Accelerator VM */
typedef struct graphX_vm_t {
    uint32_t            PC;             // Program counter
    instruction         ISA;            // Instruction register
    int32_t             R;              // Operation result register
    uint32_t            A0, A1, A2;     // Argument registers
    uint32_t            N0, N1, N2, N3; // Node registers
    float               W0, W1, W2, W3; // Weight registers
    uint32_t            FN;             // Frontier node
    float               FW;             // Frontier weight
    uint32_t            program[8192];  // Instructions to run
    uint32_t             memory[65536]; // Memory
    int                 debug;          // Debug flag
    graph_t             *graph;         // Graph data structure
    graph_iterator_t    *it;            // Graph iterator if needed
    frontier_t          *frontier;      // Frontier if needed
} graphX_vm_t;

/* Functions needed to run instructions */
uint32_t fetch(graphX_vm_t *vm);
int decode(graphX_vm_t *vm, uint32_t data);
int execute(graphX_vm_t *vm);

/* Run a graphX program */
int run(graphX_vm_t *vm);

/* Reset a VM */
void graphX_reset(graphX_vm_t *vm);

#endif