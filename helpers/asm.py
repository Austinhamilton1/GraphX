#!/usr/bin/env python3
'''
GraphX Assembler (Prototype)
----------------------------
Assembles .graphx source files into binary programs
for your GraphX VM.

Sections:
    .code   - Instructions
    .data   - Graph/constant data
    .mem    - Memory/frontier initialization
'''

import re
import struct
import sys

OPCODE_ARG_MASK         = 0x0000001F 
REGISTER_ARG_MASK       = 0x000000FF 
IMMEDIATE_ARG_MASK      = 0xFFFFFFFF
FLAG_R                  = 0x00000001
FLAG_I                  = 0x00000002

'''=== Instruction Encoding ====================================================='''

OPCODES = {
    'HALT': 0,      # End of program
    'BZ': 1,        # Conditional branch if zero
    'BNZ': 2,       # Conditional branch if not zero
    'BLT': 3,       # Branch if less than
    'BGE': 4,       # Branch if greater than
    'JMP': 5,       # Unconditional jump
    'NITER': 6,     # Initialize neighbor iteration
    'NNEXT': 7,     # Load next neighbor into Rnbr
    'EITER': 8,     # Initialize edge iteration     
    'ENEXT': 9,     # Load next edge into Rnode, Rnbr, Rval
    'HASE': 10,     # Check if there is an edge between Rnode and input node
    'ADD': 11,      # Add edge weight
    'ADDI': 12,     # Add immediate value
    'SUB': 13,      # Subtract edge weight,
    'SUBI': 14,     # Subtract immediate value
    'CMP': 15,      # Compare, set FLAGS
    'MOV': 16,      # Move
    'MOVI': 17,     # Move immediate
    'LD': 18,       # Load register from memory
    'ST': 19,       # Store register to memory
    'LDR': 20,      # Load from register address
    'STR': 21,      # Store to register address
    'PUSH': 22,     # Add neighbor to next frontier
    'POP': 23,      # Load next node from frontier
    'FEMPTY': 24,   # Check if frontier is empty
    'FSWAP': 25,    # Swap next frontier and current frontier buffers
}

def encode_instruction(op, args):
    '''
    Encode an instruction to 64 bits.
    Simplified encoding:
        [ 5 bits opcode | 3 bit flags | 8 bit dst | 8 bit src1 | 8 bit src2 | 32 bit imm ]
    '''
    opcode = OPCODES[op] << 59

    # No argument operations
    if op in ['HALT', 'EITER', 'ENEXT', 'FEMPTY', 'FSWAP', 'HASE']:
        return opcode | (FLAG_R << 56)
    
    # Immediate operations
    elif op in ['BZ', 'BNZ', 'BLT', 'BGE', 'JMP', 'NITER', 'NNEXT']:
        imm = int(args[0]) & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 56) | imm
    
    # Register-Register-Register operations
    elif op in ['ADD', 'SUB']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        r2 = int(args[2]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 56) | (r0 << 48) | (r1 << 40) | (r2 << 32)
    
    # Register-Register-Immediate operations
    elif op in ['ADDI', 'SUBI']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        imm = int(args[2]) & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 56) | (r0 << 48) | (r1 << 40) | imm
    
    # Register-Register operations
    elif op in ['CMP', 'MOV', 'LDR', 'STR']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 56) | (r0 << 48) | (r1 << 40)

    # Register-Immediate operations
    elif op in ['MOVI', 'LD', 'ST']:
        r = int(args[0]) & REGISTER_ARG_MASK
        imm = int(args[1]) & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 56) | (r << 48) | imm

    # Register operations
    elif op in ['PUSH', 'POP']:
        r = int(args[0]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 56) | (r << 48)

    # Invalid opcode
    else:
        raise ValueError(f'Unkown opcode: {op}')
    
'''=== Assembler Parser ====================================================================='''

def parse_assembly(lines):
    section = None
    code = []
    data = {'row_index': [], 'col_index': [], 'values': []}
    mem = []

    code_labels = {}
    register_names = {
        'Rnode': 0,
        'Rnbr': 1,
        'Rval': 2,
        'Racc': 3,
        'Rtmp1': 4,
        'Rtmp2': 5,
        'Rtmp3': 6,
        'Rzero': 7,
    }
    pc = 0

    # First pass (Resolve labels)
    for line in lines:
        # Remove comments
        line = re.sub(r';.*$', '', line)

        # Remove empty spaces
        line = line.strip()

        if not line:
            # Blank line or comment
            continue

        if line.startswith('.'):
            section = line
            continue

        if section == '.code':
            if line.endswith(':'):
                code_labels[line[:-1]] = pc
            else:
                pc += 1

    # Second pass (Parse assembly)
    for line in lines:
        # Remove comments 
        line = re.sub(r';.*$', '', line)

        # Remove empty spaces 
        line = line.strip()

        if not line:
            # Blank line or comment
            continue

        if line.startswith('.'):
            # Section heading
            section = line
            continue

        if section == '.code':
            # Code section, instructions

            if line.endswith(':'):
                # Label symbols
                continue

            tokens = re.split(r'[,\s]+', line)
            op = tokens[0]
            args = tokens[1:]

            # Substitute Register names and label names
            for i in range(len(args)):
                if args[i] in register_names:
                    args[i] = register_names[args[i]]
                elif args[i] in code_labels:
                    args[i] = code_labels[args[i]]

            code.append(encode_instruction(op, args))

        elif section == '.row_index':
            # Break up section into values
            tokens = re.split(r'[,\s]+', line)
            for token in tokens:
                # Allow decimal or hex
                data['row_index'].append(int(token, 0))
        
        elif section == '.col_index':
            # Break up section into values
            tokens = re.split(r'[,\s]+', line)
            for token in tokens:
                # Allow decimal or hex
                data['col_index'].append(int(token, 0))

        elif section == '.values':
            # Break up section into values
            tokens = re.split(r'[,\s]+', line)
            for token in tokens:
                # Allow decimal or hex
                data['values'].append(int(token, 0))

        elif section == '.mem':
            tokens = re.split(r'[,\s]+', line)
            for token in tokens:
                # Allow decimal or hex
                mem.append(int(token, 0))
                
    return code, data, mem

'''=== Binary Output ========================================================================'''

def write_binary(filename, code, data, mem):
    with open(filename, 'wb') as f:
        # Header: sizes (each 4 bytes)
        f.write(struct.pack('<IIIII', len(code), len(data['row_index']), len(data['col_index']), len(data['values']), len(mem)))

        # Sections
        for word in code:
            f.write(struct.pack('<Q', word))

        for section in (data['row_index'], data['col_index'], data['values'], mem):
            for word in section:
                f.write(struct.pack('<I', word))

    print(f'[OK] Assembled {filename}')
    print(f'    Code: {len(code)} instrs | Row Index: {len(data["row_index"])} words | Column Index: {len(data["col_index"])} words | Values: {len(data["values"])} words | Mem: {len(mem)} bytes')

'''=== Main ================================================================================='''

def main():
    if len(sys.argv) < 3:
        print('Usage asm.py <input.graphx> <output.bin>')
        sys.exit(1)

    with open(sys.argv[1], 'r') as f:
        lines = f.readlines()

    code, data, mem = parse_assembly(lines)
    write_binary(sys.argv[2], code, data, mem)

if __name__ == '__main__':
    main()