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
    if(vm->ISA == HALT) {
        // Halt does nothing
        return 0;
    } else if(vm->ISA >= JMP && vm->ISA <= BZ) {
        // Immediate instructions utilize all lower 27 bits
        vm->A0 = data & IMMEDIATE_ARG_MASK;
        return 0;
    } else if(vm->ISA >= LD && vm->ISA <= MOV) {
        // Memory instructions have a register and a constant
        vm->A0 = (data >> 18) & REGISTER_ARG_MASK;
        vm->A1 = data & CONSTANT_ARG_MASK;
        return 0;
    } else if(vm->ISA >= ADD && vm->ISA <= CMP) {
        // Arguments are stored in least significant 27 bits
        vm->A0 = (data >> 18) & REGISTER_ARG_MASK;
        vm->A1 = (data >> 9) & REGISTER_ARG_MASK;
        vm->A2 = data & REGISTER_ARG_MASK;
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
    arg1 = vm->A0;
    arg2 = vm->A1;
    arg3 = vm->A2;

    switch (opcode) {
    case HALT:
        return VM_HALT;
        break;
    case JMP:

        return VM_CONTINUE;
        break;
    case BZ:

        return VM_CONTINUE;
        break;
    case LD:
        
        return VM_CONTINUE;
        break;
    case ST:

        return VM_CONTINUE;
        break;
    case MOV:

        return VM_CONTINUE;
        break;
    case ADD:

        return VM_CONTINUE;
        break;
    case SUB:

        return VM_CONTINUE;
        break;
    case ITER:

        return VM_CONTINUE;
        break;
    case NEIGHBOR:

        return VM_CONTINUE;
        break;
    case DEGREE:

        return VM_CONTINUE;
        break;
    case FPUSH:

        return VM_CONTINUE;
        break;
    case FPOP:
        
        return VM_CONTINUE;
        break;
    case FEMPTY:

        return VM_CONTINUE;
        break;
    case CMP:
        
        return VM_CONTINUE;
        break;
    default:
        return VM_ERROR;
        break;
    }

    return VM_HALT;
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
    vm->A3 = 0;
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