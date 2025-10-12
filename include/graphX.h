#ifndef GRAPH_X_H
#define GRAPH_X_H

#include <stdint.h>
#include "datastructures.h"
#include "graph.h"

#define OPCODE_ARG_MASK         0x0000001F  // 5 bit opcodes
#define REGISTER_ARG_MASK       0x00000007  // 3 bit register arguments
#define IMMEDIATE_ARG_MASK      0x07FFFFFF  // 27 bit immediate arguments
#define CONSTANT_ARG_MASK       0x00FFFFFF  // 24 bit constant arguments
#define REG_CONSTANT_ARG_MASK   0x001FFFFF  // 21 bit register constant arguments

#define ARG1(vm) (vm->A0)   // First argument of instruction
#define ARG2(vm) (vm->A1)   // Second argument of instruction
#define ARG3(vm) (vm->A2)   // Third argument of instruction

#define FLAG_ZERO   0x1 // Check if CMP resulted in zero
#define FLAG_NEG    0x2 // Check if CMP resulted in negative
#define FLAG_POS    0x4 // Check if CMP resulted in positive

#define PROGRAM_SIZE    8192    // Maximum program size
#define MEMORY_SIZE     65536   // Amount of RAM

typedef enum {
    /* Control flow */
    HALT = 0,   // End of program
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
    ADD,        // Add two registers
    ADDI,       // Add immediate value to register
    SUB,        // Subtract two registers,
    SUBI,       // Subtract immediate value from register
    CMP,        // Compare, set FLAGS
    MOV,        // Move register
    MOVI,       // Move immediate
    CLR,        // Zero register

    /* Memory access */
    LD,         // Load register from memory
    ST,         // Store register to memory
    LDR,        // Load from register address
    STR,        // Store to register address
    PUSH,       // Add neighbor to next frontier
    POP,        // Load next node from frontier

    /* Frontier control */
    FEMPTY,     // Check if frontier is empty
    FSWAP,      // Swap next frontier and current frontier buffers
} instruction;

typedef enum {
    R_NODE = 0,
    R_NBR,
    R_VAL,
    R_ACC,
    R_TMP,
    R_PTR,
    R_ZERO,
    R_COUNT,
} reg;

/* VM state after each instruction */
typedef enum { VM_ERROR = -1, VM_HALT = 0, VM_CONTINUE = 1} vm_status_t;

/* Graph Accelerator VM */
typedef struct graphX_vm_t {
    uint32_t            PC;                     // Program counter
    instruction         ISA;                    // Instruction register
    uint32_t            FLAGS;                  // Operation result register
    uint32_t            A0, A1, A2;             // Argument registers

    /* Register file */
    union {
        struct {
            uint32_t    Rnode;                  // Current node
            uint32_t    Rnbr;                   // Neighbor node
            uint32_t    Rval;                   // Weight value
            uint32_t    Racc;                   // Accumulator register
            uint32_t    Rtmp;                   // Temporary register
            uint32_t    Rptr;                   // Temporary register for pointers
            uint32_t    Rzero;                  // Zero register
        };
        uint32_t        R[R_COUNT];             // Register indexer
    };

    /* Memory */
    uint32_t            program[PROGRAM_SIZE];  // Instructions to run
    uint32_t            memory[MEMORY_SIZE];    // Memory
    uint32_t            iter;                   // Iterator index
    graph_t             *graph;                 // Graph data structure
    frontier_t          *frontier;        // Frontier for graph exploration
    frontier_t          *next_frontier;         // Back frontier buffer (used for level-synchronicity)

    /* Debug hook */
    void                (*debug)(struct graphX_vm_t *);
} graphX_vm_t;

/* Functions needed to run instructions */
uint32_t fetch(graphX_vm_t *vm);
int decode(graphX_vm_t *vm, uint32_t data);
vm_status_t execute(graphX_vm_t *vm);

/* Run a graphX program */
vm_status_t run(graphX_vm_t *vm);

/* Reset a VM */
void graphX_reset(graphX_vm_t *vm);

#endif