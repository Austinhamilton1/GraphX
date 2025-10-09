#include "graphX.h"

#include <stdio.h>
#include <string.h>

/*
 * Fetch the next instruction for the accelerator.
 *
 * Arguments:
 *     graphX_vm_t *vm - The VM to fetch from.
 * Returns:
 *     uint32_t - The encoded instruction.
 */
uint32_t fetch(graphX_vm_t *vm) {
    // Return HALT if the PC is out of bounds
    if(vm->PC >= 8192) return VM_HALT;

    // Fetch the next instruction
    uint32_t data = vm->program[vm->PC];
    vm->PC++;
    return data;
}

/*
 * Decode a fetched instruction.
 *
 * Arguments:
 *     graphX_vm_t *vm - The vm the instruction came from.
 *     uint32_t data - Data to decode.
 * Returns:
 *     int - 0 on sucess -1 on error.
 */
int decode(graphX_vm_t *vm, uint32_t data) {
    // Opcode is stored in most significant 5 bits
    vm->ISA = (data >> 27) & 0x1F;

    // If a valid opcode is passed, return it
    switch(vm->ISA) {
    case HALT:
        // Halt does nothing
        return 0;
    case BZ:
        // BZ is an immediate instruction
        ARG1(vm) = data & IMMEDIATE_ARG_MASK;
        return 0;
    case BNZ:
        // BNZ is an immediate instruction
        ARG(vm) = data & IMMEDIATE_ARG_MASK;
        return 0;
    case JMP:
        // JMP is an immediate instruction
        ARG1(vm) = data & IMMEDIATE_ARG_MASK;
        return 0;
    case LDN:
        // LDN is an immediate instruction
        ARG1(vm) = data & IMMEDIATE_ARG_MASK;
        return 0;
    case ITER:
        // ITER takes no arguments
        return 0;
    case NEXT:
        // NEXT doesn't take any arguments
        return 0;
    case LDV:
        // LDV doesn't take any arguments
        return 0;
    case HASN:
        // HASN doesn't take any arguments
        return 0;
    case HASE:
        // HASE is an immediate instruction
        ARG1(vm) = data & IMMEDIATE_ARG_MASK;
        return 0;
    case ADD:
        // ADD adds a register value to a register and stores it in another register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = (data >> 21) & REGISTER_ARG_MASK;
        ARG3(vm) = (data >> 18) & REGISTER_ARG_MASK;
        return 0;
    case ADDI:
        // ADDI adds a constant value to a register and stores it in another register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = (data >> 21) & REGISTER_ARG_MASK;
        ARG3(vm) = data & REG_CONSTANT_ARG_MASK;
    case SUB:
        // SUB subtracts a register value from a register and stores it in another register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = (data >> 21) & REGISTER_ARG_MASK;
        ARG3(vm) = (data >> 18) & REGISTER_ARG_MASK;
        return 0;
    case SUBI:
        // SUBI subtracts a constant value from a register and stores it in another register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = (data >> 21) & REGISTER_ARG_MASK;
        ARG3(vm) = data & REG_CONSTANT_ARG_MASK;
        return 0;
    case CMP:
        // CMP compares two register values and stores the results in the FLAG register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = (data >> 21) & REGISTER_ARG_MASK;
        return 0;
    case MOV:
        // MOV moves a register value from a source register to a destination register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = (data >> 21) & REGISTER_ARG_MASK;
        return 0;
    case MOVI:
        // MOVI moves a constant value into a register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = data & CONSTANT_ARG_MASK;
        return 0;
    case CLR:
        // CLR sets the value of a register to 0
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        return 0;
    case LD:
        // LD loads a value from memory to a register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = data & CONSTANT_ARG_MASK;
        return 0;
    case ST:
        // ST stores a valeu from a register to memory
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        ARG2(vm) = data & CONSTANT_ARG_MASK;
        return 0;
    case PUSH:
        // PUSH pushes a register to the frontier
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        return 0;
    case POP:
        // POP pops a node from the frontier into a register
        ARG1(vm) = (data >> 24) & REGISTER_ARG_MASK;
        return 0;
    case FEMPTY:
        // FEMPTY takes no arguments
        return 0;
    case FSWAP:
        // FSWAP takes no arguments
        return 0;
    }

    // If an invalid opcode is passed, assume halt
    return -1;
}

/*
 * Execute an instruction passed in.
 *
 * Arguments:
 *     graphX_vm_t *vm - Execute the instruction on this vm.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
int execute(graphX_vm_t *vm) {
    // Parse out the opcode and arguments
    uint32_t opcode, arg1, arg2, arg3;
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
        if(arg1 >= 8192) return VM_ERROR;
        
        // Conditional check for zero
        if(FLAG_0(vm))
            vm->PC = arg1;
        break;
    case BNZ:
        // Bounds checking
        if(arg1 >= 8192) return VM_ERROR;

        // Conditional check for non-zero
        if(!FLAG_0(vm))
            vm->PC = arg1;        
        break;
    case JMP:
        // Bounds checking
        if(arg1 >= 8192) return VM_ERROR;

        // Unconditional branch
        vm->PC = arg1;
        break;
    case LDN:
        // Load in argument into register
        vm->Rnode = arg1;
        break;
    case ITER:
        // Initialize the internal iterator
        vm->iter = 0;
        break;
    case NEXT:
        // Get the next neighbor
        vm->Rnbr = vm->graph->col_index[vm->graph->row_index[vm->Rnode] + vm->iter++];
        break;
    case LDV:
        // Get the weight between the current node and the next neighbor
        vm->Rval = vm->graph->values[vm->graph->row_index[vm->Rnode] + vm->iter-1];
        break;
    case HASN:
        // Check if the current node has any more neighbors
        if(vm->graph->row_index[vm->Rnode] + vm->iter >= vm->graph->row_index[vm->Rnode+1])
            vm->FLAGS |= 0x1;
        break;
    case HASE:
        // Check if there is an edge between two nodes (binary search)
        /* Needs to be implemented */
        break;
    case ADD:
        
        break;
    case ADDI:
        
        break;
    case SUB:
        
        break;
    case SUBI:
        
        break;
    case CMP:
        
        break;
    case MOV:
        
        break;
    case MOVI:
        
        break;
    case CLR:
        
        break;
    case LD:
        
        break;
    case ST:
        
        break;
    case PUSH:
        
        break;
    case POP:
        
        break;
    case FEMPTY:
        
        break;
    case FSWAP:
        
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
 *     int - The exit code of the program.
 */
int run(graphX_vm_t *vm) {
    int result = VM_CONTINUE;

    // Fetch, decode, execute pipeline
    while(result > 0) {
        uint32_t data = fetch(vm);
        if(decode(vm, data) < 0) break;
        result = execute(vm);
        if(vm->debug) {
            printf("PC: %u, ISA=%u, R=%u\n", vm->PC, vm->ISA, vm->R);
            printf("A0=%u, A1=%u, A2=%u\n", vm->A0, vm->A1, vm->A2);
            printf("N0=%u, N1=%u, N2=%u, N3=%u\n", vm->N0, vm->N1, vm->N2, vm->N3);
            printf("W0=%f, W1=%f, W2=%f, W3=%f\n", vm->W0, vm->W1, vm->W2, vm->W3);
            printf("FN=%u, FW=%f\n", vm->FN, vm->FW);
            printf("\n");
        }
    }

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
    vm->R = 0;
    vm->ISA = HALT;
    vm->A0 = 0;
    vm->A1 = 0;
    vm->A2 = 0;
    vm->N0 = 0;
    vm->N1 = 0;
    vm->N2 = 0;
    vm->N3 = 0;
    vm->W0 = 0.0f;
    vm->W1 = 0.0f;
    vm->W2 = 0.0f;
    vm->W3 = 0.0f;
    vm->FN = 0;
    vm->FW = 0.0f;
    memset(vm->memory, 0, sizeof(vm->memory));
}