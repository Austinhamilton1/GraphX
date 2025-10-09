#ifndef GRAPH_X_H
#define GRAPH_X_H

#include <stdint.h>
#include "datastructures.h"
#include "graph.h"

#define VM_CONTINUE 1
#define VM_HALT     0
#define VM_ERROR    -1

#define OPCODE_ARG_MASK         0x0000001F  // 5 bit opcodes
#define REGISTER_ARG_MASK       0x00000007  // 3 bit register arguments
#define IMMEDIATE_ARG_MASK      0x07FFFFFF  // 27 bit immediate arguments
#define CONSTANT_ARG_MASK       0x00FFFFFF  // 24 bit constant arguments
#define REG_CONSTANT_ARG_MASK   0x001FFFFF  // 21 bit register constant arguments

#define ARG1(vm) (vm->A0)
#define ARG2(vm) (vm->A1)
#define ARG3(vm) (vm->A2)

#define IMM(vm) (vm->A0)

#define FLAG_0(vm) ((vm->R) & 0x1)
#define FLAG_POS(vm) (((vm->R) >> 1) & 0x1)
#define FLAG_NEG(vm) (((vm->R) >> 2) & 0x1)

typedef enum {

    /* Control flow */
    HALT,       // End of program
    BZ,         // Conditional branch if zero
    BNZ,        // Conditional branch if not zero
    JMP,        // Unconditional jump

    /* Graph access instructions */
    LDN,        // Load node into Rnode
    ITER,       // Initialize neighbor iteration
    NEXT,       // Load next neighbor into Rnbr
    LDV,        // Load edge weight into Rval
    HASN,       // Check if more neighbors exist
    HASE,       // Check if there is an edge between Rnode and input node
    
    /* Arithmetic and logic*/
    ADD,        // Add edge weight
    ADDI,       // Add immediate value
    SUB,        // Subtract edge weight,
    SUBI,       // Subtract immediate value
    CMP,        // Compare, set FLAGS
    MOV,        // Move
    MOVI,       // Move immediate
    CLR,        // Zero register

    /* Memory access */
    LD,         // Load accumulator from memory
    ST,         // Store accumulator
    PUSH,       // Add neighbor to next frontier
    POP,        // Load next node from frontier

    /* Frontier control */
    FEMPTY,     // Check if frontier is empty
    FSWAP,      // Swap next frontier and current frontier buffers

} instruction;

/* Graph Accelerator VM */
typedef struct graphX_vm_t {
    uint32_t            PC;             // Program counter
    instruction         ISA;            // Instruction register
    uint32_t            FLAGS;          // Operation result register
    uint32_t            A0, A1, A2;     // Argument registers
    uint32_t            Rnode, Rnbr;    // Current node and neighbor node
    int32_t             Rval;           // Current weight
    int32_t             Racc, Rtmp;     // Weight accumulator and temporary scratch register
    uint32_t            program[8192];  // Instructions to run
    uint32_t            memory[65536];  // Memory
    uint32_t            iter;           // Iterator index
    int                 debug;          // Debug flag
    graph_t             *graph;         // Graph data structure
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