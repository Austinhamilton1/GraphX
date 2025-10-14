#include "graphX.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Load a GraphX program from a binary file */
int graphX_load(graphX_vm_t *vm, const char *filename);

/* Debug hook */
void debug_hook(graphX_vm_t *vm);

/* Exit call back */
void exit_hook(graphX_vm_t *vm, int result);

static const char *opcodes[] = {
    "HALT",
    "BZ",
    "BNZ",
    "BLT",
    "BGT",
    "JMP",    
    "NITER",     
    "NNEXT",
    "EITER",
    "ENEXT",      
    "HASE", 
    "ADD",       
    "ADDI",    
    "SUB",   
    "SUBI",   
    "CMP",   
    "MOV", 
    "MOVI", 
    "LD",  
    "ST",  
    "LDR", 
    "STR", 
    "PUSH",
    "POP", 
    "FEMPTY", 
    "FSWAP",
};

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: %s <program.bin> [--debug]\n", argv[0]);
        return 1;
    }

    // Initialize the VM
    graph_t g = {0};
    frontier_t f = {0};
    frontier_t n = {0};

    graphX_vm_t vm = {0};
    vm.graph = &g;
    vm.frontier = &f;
    vm.next_frontier = &n;
    vm.debug_hook = (argc > 2 && strcmp(argv[2], "--debug") == 0 ? debug_hook : 0);
    vm.exit_hook = exit_hook;

    // Load the binary instruction
    if(graphX_load(&vm, argv[1]) < 0) {
        fprintf(stderr, "Failed to load program.\n");
        return 1;
    }

    // Run the instructions on the VM and check for result
    run(&vm);

    return 0;
}

/* 
 * Load a binary file to run on the VM.
 *
 * Layout:
 *     [0:4) -> code_len (uint32_t)
 *     [4:8) -> row_index_len (uint32_t)
 *     [8:12) -> col_index_len (uint32_t)
 *     [12:16) -> values_len section
 *     [16:20) -> mem length
 *     [20:...] -> code section
 *     [...] -> row index section
 *     [...] -> column index section
 *     [...] -> values section
 *     [...] -> memory section
 * 
 * Arguments:
 *     graphX_vm_t *vm - The VM to run the program on.
 *     const char *filename - The binary file with instructions in it.
 * Returns:
 *     int - 0 on success -1 on failure.
 */
int graphX_load(graphX_vm_t *vm, const char *filename) {
    // Reset VM state
    graphX_reset(vm);
    
    FILE *f = fopen(filename, "rb");
    if(!f) {
        perror("Error opening program file");
        return -1;
    }

    uint32_t code_len, row_index_len, col_index_len, values_len, mem_len;

    // Read the header of the file
    if(fread(&code_len, sizeof(uint32_t), 1, f) != 1 ||
       fread(&row_index_len, sizeof(uint32_t), 1, f) != 1 ||
       fread(&col_index_len, sizeof(uint32_t), 1, f) != 1 ||
       fread(&values_len, sizeof(uint32_t), 1, f) != 1 ||
       fread(&mem_len, sizeof(uint32_t), 1, f) != 1) {
       fprintf(stderr, "Error: invalid header in %s\n", filename);
       fclose(f);
       return -1;
    }

    // Bounds checking
    if(code_len > PROGRAM_SIZE || mem_len > MEMORY_SIZE) {
        fprintf(stderr, "Error: program too large\n");
        fclose(f);
        return -1;
    }

    // Load program
    if(fread(vm->program, sizeof(uint32_t), code_len, f) != code_len) {
        fprintf(stderr, "Error: failed to read program section\n");
        fclose(f);
        return -1;
    }

    // Load graph
    vm->graph->n = col_index_len;
    if(row_index_len > 0 && fread(vm->graph->row_index, sizeof(uint32_t), row_index_len, f) != row_index_len) {
        fprintf(stderr, "Error: failed to read row index section\n");
        fclose(f);
        return -1;
    }
    if(col_index_len > 0 && fread(vm->graph->col_index, sizeof(uint32_t), col_index_len, f) != col_index_len) {
        fprintf(stderr, "Error: failed to read column index section\n");
        fclose(f);
        return -1;
    }
    if(values_len > 0 && fread(vm->graph->values, sizeof(uint32_t), values_len, f) != values_len) {
        fprintf(stderr, "Error: failed to read values section\n");
        fclose(f);
        return -1;
    }

    // Load memory initialization
    if(mem_len > 0) {
        if(fread(vm->memory, sizeof(uint32_t), mem_len, f) != mem_len) {
            fprintf(stderr, "Error: failed to read memory section\n");
            fclose(f);
            return -1;
        }
    }

    fclose(f);

    return 0;
}

/*
 * Debug hook for the graphX VM.
 *
 * Arguments:
 *     graphX_vm_t *vm - The VM to hook into.
 */
void debug_hook(graphX_vm_t *vm) {
    printf("PC=%u, ISA=%s, FLAGS=%u\n", vm->PC, opcodes[vm->ISA], vm->FLAGS);
    printf("niter0=%u, niter1=%u, niter2=%u, niter3=%u, eiter=%u\n", vm->niter[0], vm->niter[1], vm->niter[2], vm->niter[3], vm->eiter);
    printf("Rnode=%u, Rnbr=%u, Rval=%u, Racc=%u, Rptr=%u\n", vm->Rnode, vm->Rnbr, vm->Rval, vm->Racc, vm->Rptr);
    printf("Rtmp1=%u, Rtmp2=%u, Rtmp3=%u\n", vm->Rtmp1, vm->Rtmp2, vm->Rtmp3);
    printf("Frontier:\n");
    printf("Front=%lu, Back=%lu\n", vm->frontier->backend.queue.front, vm->frontier->backend.queue.back);
    for(int i = 0; i < 10; i++)
        printf("%u ", vm->frontier->backend.queue.data[i]);
    printf("...\n");
    printf("Next frontier:\n");
    printf("Front=%lu, Back=%lu\n", vm->next_frontier->backend.queue.front, vm->next_frontier->backend.queue.back);
    for(int i = 0; i < 10; i++)
        printf("%u ", vm->next_frontier->backend.queue.data[i]);
    printf("...\n");
    printf("Memory:\n");
    for(int i = 0; i < 10; i++)
        printf("%u ", vm->memory[i]);
    printf("...");
    for(int i = 10; i > 0; i--)
        printf("%u ", vm->memory[65536-i]);
    printf("\n");
    getchar();
}

/*
 * Exit callback for the graphX VM.
 *
 * Arguments:
 *     graphX_vm_t *vm - The VM to callback from.
 *     int result - The result code of the execution.
 */
void exit_hook(graphX_vm_t *vm, int result) {
    if(result == VM_HALT) {
        printf("VM execution succeeded.\n");
        printf("Total number of instructions: %lu\n", vm->clock);
        printf("Memory at end of program:\n");
        printf("\n");
        for(int i = 0; i < 256; i++) {
            for(int j = 0; j < 256; j++) {
                printf("%u ", vm->memory[(256*i) + j]);
            }
            printf("\n");
        }
    } else if(result == VM_ERROR) {
        printf("VM execution failed.\n");
        vm->PC--; // Get the last executed instruction
        printf("Execution failed on PC=%u.\n", vm->PC);
        printf("Inst=%s, A0=%u, A1=%u, A2=%u.\n", opcodes[(vm->program[vm->PC] >> 27) & 0x1F], vm->A0, vm->A1, vm->A2);
    }
}