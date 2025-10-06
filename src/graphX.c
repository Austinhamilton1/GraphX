#include "graphX.h"

/*
 * Load a node into a node register.
 *
 * Arguments:
 *     graphX_vm_t *vm - The virtual machine to update.
 *     uint8_t reg - Load the node into this register.
 *     unsigned int node - Load this node into the register.
 * Returns:
 *     int - 0 on success, -1 on failure.
 */
static inline int load_node(graphX_vm_t *vm, uint8_t reg, unsigned int node) {
    switch(reg) {
    case 0:
        vm->N0 = node;
        break;
    case 1:
        vm->N1 = node;
        break;
    case 2:
        vm->N2 = node;
        break;
    case 3:
        vm->N3 = node;
        break;
    default:
        return -1;
        break;
    }

    return 0;
}

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
    if(vm->PC >= 65536) return VM_HALT;

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
    } else if(vm->ISA >= LDNI && vm->ISA <= BZ) {
        // Immediate instructions utilize all lower 27 bits
        vm->A0 = data & IMMEDIATE_ARG_MASK;
        return 0;
    } else if(vm->ISA >= ADD && vm->ISA <= CMP) {
        // Arguments are stored in least significant 27 bits
        vm->A0 = (data >> 18) & 0x1FF;
        vm->A1 = (data >> 9) & 0x1FF;
        vm->A2 = data & 0x1FF;
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
    case LDNI:

        return VM_CONTINUE;
        break;
    case LDN:

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
    case BZ:

        return VM_CONTINUE;
        break;
    case CMP:
        
        return VM_CONTINUE;
        break;
    case JMP:

        return VM_CONTINUE;
        break;
    case ADD:

        return VM_CONTINUE;
        break;
    case SUB:

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
    }

    return result;
}