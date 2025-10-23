#include "graphX.h"

#include <stdio.h>
#include <string.h>

/*
 * Fetch the next instruction for the accelerator.
 *
 * Arguments:
 *     graphX_vm_t *vm - The VM to fetch from.
 * Returns:
 *     uint64_t - The encoded instruction.
 */
uint64_t fetch(graphX_vm_t *vm) {
    // Return HALT if the PC is out of bounds
    if(vm->PC >= PROGRAM_SIZE) return VM_HALT;

    // Fetch the next instruction
    uint64_t data = vm->program[vm->PC];
    vm->PC++;
    return data;
}

/*
 * Decode a fetched instruction.
 *
 * Arguments:
 *     graphX_vm_t *vm - The vm the instruction came from.
 *     uint64_t data - Data to decode.
 * Returns:
 *     int - 0 on sucess -1 on error.
 */
int decode(graphX_vm_t *vm, uint64_t data) {
    // Zero out arguments
    ARG1(vm) = 0;
    ARG2(vm) = 0;
    ARG3(vm) = 0;

    // Opcode is stored in most significant 5 bits
    vm->ISA = (data >> 59) & OPCODE_ARG_MASK;

    // Flags are stored in the next 3 bits
    int flags = (data >> 56) & FLAGS_ARG_MASK;

    // First two register args are stored in subsequent 8 bits
    ARG1(vm) = (data >> 48) & REGISTER_ARG_MASK;
    ARG2(vm) = (data >> 40) & REGISTER_ARG_MASK;

    // The flags will determine if this is an R-type or I-type instruction
    if(flags & FLAG_R) {
        ARG3(vm) = (data >> 32) & REGISTER_ARG_MASK;
    } else if(flags & FLAG_I) {
        ARG3(vm) = data & IMMEDIATE_ARG_MASK;
    } else {
        // Invalid flags argument was passed in
        return -1;
    }

    // If a valid opcode is passed, return 0
    switch(vm->ISA) {
    case HALT:
    case BZ:
    case BNZ:
    case BLT:
    case BGE:
    case JMP:
    case NITER:
    case NNEXT:
    case EITER:
    case ENEXT:
    case HASE:
    case ADD:
    case ADDI:
    case SUB:
    case SUBI:
    case CMP:
    case MOV:
    case MOVI:
    case LD:
    case ST:
    case LDR:
    case STR:
    case PUSH:
    case POP:
    case FEMPTY:
    case FSWAP:
        return 0;
    }

    // If an invalid opcode is passed, return -1
    return -1;
}

/*
 * Execute an instruction passed in.
 *
 * Arguments:
 *     graphX_vm_t *vm - Execute the instruction on this vm.
 * Returns:
 *     vm_status_t - The state of the VM after execution.
 */
vm_status_t execute(graphX_vm_t *vm) {
    // Parse out the opcode and arguments
    int32_t opcode, arg1, arg2, arg3, comp;
    opcode = vm->ISA;
    arg1 = ARG1(vm);
    arg2 = ARG2(vm);
    arg3 = ARG3(vm);

    // Execute the inputted opcode
    switch(opcode) {
    case HALT:
        // Halt does nothing
        return VM_HALT;
    case BZ:
        // Bounds checking
        if(arg3 >= MEMORY_SIZE || arg3 < 0) return VM_ERROR;
        
        // Conditional check for zero
        if(vm->FLAGS & FLAG_ZERO)
            vm->PC = arg3;
        break;
    case BNZ:
        // Bounds checking
        if(arg3 >= MEMORY_SIZE | arg3 < 0) return VM_ERROR;

        // Conditional check for non-zero
        if(!(vm->FLAGS & FLAG_ZERO))
            vm->PC = arg3;        
        break;
    case BLT:
        // Bounds checking
        if(arg3 >= MEMORY_SIZE || arg3 < 0) return VM_ERROR;

        // Conditional check for less than
        if(vm->FLAGS & FLAG_NEG)
            vm->PC = arg3;
        break;
    case BGE:
        // Bounds checking
        if(arg3 >= MEMORY_SIZE || arg3 < 0) return VM_ERROR;

        // Conditional check for greater than
        if(vm->FLAGS & FLAG_POS || vm->FLAGS & FLAG_ZERO)
            vm->PC = arg3;
        break;
    case JMP:
        // Bounds checking
        if(arg3 >= MEMORY_SIZE || arg3 < 0) return VM_ERROR;

        // Unconditional branch
        vm->PC = arg3;
        break;
    case NITER:
        // Bounds check
        if(arg3 >= 4 || arg3 < 0) return VM_ERROR;
        // Initialize one of the internal iterators
        vm->niter[arg3] = 0;
        break;
    case NNEXT:
        // Bounds check 
        if(arg3 >= 4 || arg3 < 0) return VM_ERROR;
        // Reset flags
        vm->FLAGS = 0;
        // Get the next neighbor
        if(vm->graph->row_index[vm->Rnode] + vm->niter[arg3] < vm->graph->row_index[vm->Rnode+1]) {
            vm->Rnbr = vm->graph->col_index[vm->graph->row_index[vm->Rnode] + vm->niter[arg3]];
            vm->Rval = vm->graph->values[vm->graph->row_index[vm->Rnode] + vm->niter[arg3]];
            vm->niter[arg3]++;
        }
        else
            vm->FLAGS |= FLAG_ZERO; // Signal done
        break;
    case EITER:
        // Initialize the internal iterator and start node
        vm->eiter = 0;
        vm->Rnode = 0;
        break;
    case ENEXT:
        // Reset flags
        vm->FLAGS = 0;

        // End of row, go to next populated column
        if(vm->Rnode < vm->graph->n && vm->graph->row_index[vm->Rnode] + vm->eiter >= vm->graph->row_index[vm->Rnode+1]) {
            vm->Rnode++;
            vm->eiter = 0;
        }
        
        // Bounds check
        if(vm->Rnode + vm->eiter >= vm->graph->n) {
            vm->FLAGS |= FLAG_ZERO; // Signal done
            break;
        }

        // Get current edge
        vm->Rnbr = vm->graph->col_index[vm->graph->row_index[vm->Rnode] + vm->eiter];
        vm->Rval = vm->graph->values[vm->graph->row_index[vm->Rnode] + vm->eiter];

        // Advance iterator
        vm->eiter++;
        break;
    case HASE:
        // Check if there is an edge between two nodes (binary search)
        // You can branch with BNZ after this
        vm->FLAGS = FLAG_ZERO;
        if(graph_has_edge(vm->graph, vm->Rnode, vm->Rnbr))
            vm->FLAGS &= ~(FLAG_ZERO);
        break;
    case ADD:
        // Add two registers and store the result in a third register
        vm->R[arg1] = vm->R[arg2] + vm->R[arg3];
        break;
    case ADDI:
        // Add an immediate value to a register and store it in a second register
        vm->R[arg1] = vm->R[arg2] + arg3;
        break;
    case SUB:
        // Subtract a register from another register and store the result in a third register
        vm->R[arg1] = vm->R[arg2] - vm->R[arg3];
        break;
    case SUBI:
        // Subtract an immediate value from a register and store it in a second register
        vm->R[arg1] = vm->R[arg2] - arg3;
        break;
    case CMP:
        // Compare two registers and store the results in FLAGS
        vm->FLAGS = 0;
        comp = vm->R[arg1] - vm->R[arg2];
        if(comp == 0) vm->FLAGS |= FLAG_ZERO;
        else if(comp < 0) vm->FLAGS |= FLAG_NEG;
        else vm->FLAGS |= FLAG_POS;
        break;
    case MOV:
        // Move one register to another
        vm->R[arg1] = vm->R[arg2];
        break;
    case MOVI:
        // Move an immediate value into a register
        vm->R[arg1] = arg3;
        break;
    case LD:
        // Load a value from memory into a register
        // Bounds check
        if(arg3 > MEMORY_SIZE || arg3 < 0) return VM_ERROR;
        vm->R[arg1] = vm->memory[arg3];
        break;
    case ST:
        // Store a value from a register into memory
        // Bounds check
        if(arg3 > MEMORY_SIZE || arg3 < 0) return VM_ERROR;
        vm->memory[arg3] = vm->R[arg1];
        break;
    case LDR:
        // Load a value from an memory specified by a register address
        // Bounds check
        if(vm->R[arg2] > MEMORY_SIZE || arg3 < 0) return VM_ERROR;
        vm->R[arg1] = vm->memory[vm->R[arg2]];
        break;
    case STR:
        // Store a value in memory specified by a register address
        // Bounds check
        if(vm->R[arg2] > MEMORY_SIZE || arg3 < 0) return VM_ERROR;
        vm->memory[vm->R[arg2]] = vm->R[arg1];
        break;
    case PUSH:
        // Push to the next frontier
        frontier_push(vm->next_frontier, vm->R[arg1]);
        break;
    case POP:
        // Pop from the frontier
        frontier_pop(vm->frontier, &vm->R[arg1]);
        break;
    case FEMPTY:
        // Check if the frontier is empty
        // If the frontier is empty, program can BZ
        if(frontier_empty(vm->frontier)) 
            vm->FLAGS |= FLAG_ZERO;
        else 
            vm->FLAGS &= ~FLAG_ZERO;
        break;
    case FSWAP:
        // Swap the back and front frontiers
        frontier_t *tmp = vm->frontier;
        vm->frontier = vm->next_frontier;
        vm->next_frontier = tmp;

        frontier_init(vm->next_frontier, vm->frontier->type);
        break;
    default:
        return VM_ERROR;
    }

    return VM_CONTINUE;
}

/* 
 * Run a GraphX VM.
 *
 * Arguments:
 *     graphX_vm_t *vm - The vm to run.
 * Returns:
 *     vm_status_t - The exit code of the program.
 */
vm_status_t run(graphX_vm_t *vm) {
    vm_status_t result = VM_CONTINUE;

    // Fetch, decode, execute pipeline
    while(result == VM_CONTINUE) {
        uint64_t data = fetch(vm);
        if(data == VM_HALT) {
            result = VM_HALT;
            break;
        }
        if(decode(vm, data) < 0) return VM_ERROR;
        result = execute(vm);
        if(vm->debug_hook) vm->debug_hook(vm);
        vm->clock++;
    }

    if(vm->exit_hook) vm->exit_hook(vm, result);

    return result;
}

/* 
 * Reset a VM.
 *
 * Arguments:
 *     graphX_vm_t *vm - The VM to restore.
 */
void graphX_reset(graphX_vm_t *vm) {
    vm->PC = 0;
    vm->FLAGS = 0;
    vm->ISA = HALT;
    vm->A0 = 0;
    vm->A1 = 0;
    vm->A2 = 0;
    vm->Rnode = 0;
    vm->Rval = 0;
    vm->Rnbr = 0;
    vm->Racc = 0;
    vm->Rtmp1 = 0;
    vm->Rtmp2 = 0;
    vm->Rtmp3 = 0;
    vm->Rzero = 0;
    for(int i = 0; i < 4; i++)
        vm->niter[i] = 0;
    vm->eiter = 0;
    memset(vm->memory, 0, sizeof(vm->memory));
    if(vm->frontier)
        frontier_init(vm->frontier, FRONTIER_QUEUE);
    if(vm->next_frontier)
        frontier_init(vm->next_frontier, FRONTIER_QUEUE);
    vm->clock = 0;
}