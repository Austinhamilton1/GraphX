#ifndef GRAPH_X_H
#define GRAPH_X_H

#include <stdint.h>
#include "datastructures.h"
#include "graph.h"

#define OPCODE_ARG_MASK         0x000000FF  // 8 bit opcodes
#define FLAGS_ARG_MASK          0x000000FF  // 8 bit flags
#define REGISTER_ARG_MASK       0x000000FF  // 8 bit register arguments
#define IMMEDIATE_ARG_MASK      0xFFFFFFFF  // 32 bit immediate arguments

#define ARG1(vm) (vm->A0)   // First argument of instruction
#define ARG2(vm) (vm->A1)   // Second argument of instruction
#define ARG3(vm) (vm->A2)   // Third argument of instruction
#define FARG(vm) (vm->FA)   // Third argument of instruction (if float type instruction)

#define FLAG_I      0x1     // Instruction is I-type
#define FLAG_F      0x2     // Instruction is float type

#define FLAG_ZERO   0x1     // Check if CMP resulted in zero
#define FLAG_NEG    0x2     // Check if CMP resulted in negative
#define FLAG_POS    0x4     // Check if CMP resulted in positive

#define PROGRAM_SIZE    8192    // Maximum program size
#define MEMORY_SIZE     65536   // Amount of RAM

typedef enum {
    /* Control flow */
    HALT = 0,   // End of program
    BZ,         // Conditional branch if zero
    BNZ,        // Conditional branch if not zero
    BLT,        // Conditional branch if less than
    BGE,        // Conditional branch if greater than or equal to
    JMP,        // Unconditional jump

    /* Graph access instructions */
    NITER,      // Initialize neighbor iteration
    NNEXT,      // Load next neighbor into Rnbr
    EITER,      // Initialize edge iteration
    ENEXT,      // Load next edge
    HASE,       // Check if there is an edge between Rnode and input node
    DEG,        // Get the degree of a node as an int
    
    /* Arithmetic and logic*/
    ADD,        // Add two registers
    ADDF,       // Add two float registers
    SUB,        // Subtract two registers,
    SUBF,       // Subtract two float registers
    MULT,       // Multiply two registers
    DIV,        // Divide two registers
    MULTF,      // Multiply two float registers
    DIVF,       // Divide two float registers
    CMP,        // Compare, set FLAGS
    CMPF,       // Compare float, set FLAGS
    MOV,        // Move register
    MOVF,       // Move float register
    MOVC,       // Cast a register to a float register
    MOVCF,      // Cast a float register to a register

    /* Memory access */
    LD,         // Load register from memory
    ST,         // Store register to memory
    LDF,        // Load float register from memory
    STF,        // Store float register to memory

    /* Frontier control */
    PUSH,       // Add neighbor to next frontier
    POP,        // Load next node from frontier
    FEMPTY,     // Check if frontier is empty
    FSWAP,      // Swap next frontier and current frontier buffers
    FFILL,      // Fill the frontier with all nodes in graph

    /* Multicore/synchronization control */
    /* These are left unimplemented for the VM */
    /* (only work on hardware) */
    PARALLEL,   // Initiate a parallel region
    BARRIER,    // Wait for all cores to reach before continuing
    LOCK,       // Mutual exclusion lock
    UNLOCK,     // Mutual exclusion unlock
} instruction;

enum {
    R_NODE = 0,
    R_NBR,
    R_VAL,
    R_ACC,
    R_TMP1,
    R_TMP2,
    R_TMP3,
    R_TMP4,
    R_TMP5,
    R_TMP6,
    R_TMP7,
    R_TMP8,
    R_TMP9,
    R_TMP10,
    R_TMP11,
    R_TMP12,
    R_TMP13,
    R_TMP14,
    R_TMP15,
    R_TMP16,
    R_ZERO,
    R_CORE,
    R_COUNT,
};

enum {
    F_ACC = 0,
    F_TMP1,
    F_TMP2,
    F_TMP3,
    F_TMP4,
    F_TMP5,
    F_TMP6,
    F_TMP7,
    F_TMP8,
    F_TMP9,
    F_TMP10,
    F_TMP11,
    F_TMP12,
    F_TMP13,
    F_TMP14,
    F_TMP15,
    F_TMP16,
    F_ZERO,
    F_COUNT,
};

/* VM state after each instruction */
typedef enum { VM_ERROR = -1, VM_HALT = 0, VM_CONTINUE = 1} vm_status_t;

/* Graph Accelerator VM */
typedef struct graphX_vm_t {
    uint32_t            PC;                     // Program counter
    instruction         ISA;                    // Instruction register
    uint32_t            FLAGS;                  // Operation result register
    int32_t             A0, A1, A2;             // Argument registers
    float               FA;                     // Float argument register

    /* Integer register file */
    union {
        struct {
            int32_t     Rnode;                  // Current node
            int32_t     Rnbr;                   // Neighbor node
            int32_t     Rval;                   // Weight value
            int32_t     Racc;                   // Accumulator register
            int32_t     Rtmp1;                  // Temporary register
            int32_t     Rtmp2;                  // Temporary register
            int32_t     Rtmp3;                  // Temporary register
            int32_t     Rtmp4;                  // Temporary register
            int32_t     Rtmp5;                  // Temporary register
            int32_t     Rtmp6;                  // Temporary register
            int32_t     Rtmp7;                  // Temporary register
            int32_t     Rtmp8;                  // Temporary register
            int32_t     Rtmp9;                  // Temporary register
            int32_t     Rtmp10;                  // Temporary register
            int32_t     Rtmp11;                  // Temporary register
            int32_t     Rtmp12;                  // Temporary register
            int32_t     Rtmp13;                  // Temporary register
            int32_t     Rtmp14;                  // Temporary register
            int32_t     Rtmp15;                  // Temporary register
            int32_t     Rtmp16;                  // Temporary register
            int32_t     Rzero;                  // Zero register
            int32_t     Rcore;                  // Core ID register
        };
        int32_t         R[R_COUNT];             // Register indexer
    };

    /* Float register file */
    union {
        struct {
            float       Facc;                   // Float accumulator register
            float       Ftmp1;                  // Temporary float register
            float       Ftmp2;                  // Temporary float register
            float       Ftmp3;                  // Temporary float register
            float       Ftmp4;                  // Temporary float register
            float       Ftmp5;                  // Temporary float register
            float       Ftmp6;                  // Temporary float register
            float       Ftmp7;                  // Temporary float register
            float       Ftmp8;                  // Temporary float register
            float       Ftmp9;                  // Temporary float register
            float       Ftmp10;                  // Temporary float register
            float       Ftmp11;                  // Temporary float register
            float       Ftmp12;                  // Temporary float register
            float       Ftmp13;                  // Temporary float register
            float       Ftmp14;                  // Temporary float register
            float       Ftmp15;                  // Temporary float register
            float       Ftmp16;                  // Temporary float register
            float       Fzero;                  // Zero float register
        };
        float           F[F_COUNT];             // Register indexer
    };
    

    /* Memory */
    uint64_t            program[PROGRAM_SIZE];  // Instructions to run
    int32_t             memory[MEMORY_SIZE];    // Memory
    uint32_t            niter[4];               // Node iterator index list
    uint32_t            eiter;                  // Edge iterator index
    graph_t             *graph;                 // Graph data structure
    frontier_t          *frontier;              // Frontier for graph exploration
    frontier_t          *next_frontier;         // Back frontier buffer (used for level-synchronicity)

    uint64_t            clock;                  // Used to count instructions executed

    /* Debug hook */
    void                (*debug_hook)(struct graphX_vm_t *);

    /* Exit hook */
    void                (*exit_hook)(struct graphX_vm_t *, int);
} graphX_vm_t;

/* Functions needed to run instructions */
uint64_t fetch(graphX_vm_t *vm);
int decode(graphX_vm_t *vm, uint64_t data);
vm_status_t execute(graphX_vm_t *vm, int flags);

/* Run a graphX program */
vm_status_t run(graphX_vm_t *vm);

/* Reset a VM */
void graphX_reset(graphX_vm_t *vm);

#endif