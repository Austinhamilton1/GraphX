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
REGISTER_ARG_MASK       = 0x00000007 
IMMEDIATE_ARG_MASK      = 0x07FFFFFF 
CONSTANT_ARG_MASK       = 0x00FFFFFF  
REG_CONSTANT_ARG_MASK   = 0x001FFFFF  

'''=== Instruction Encoding ====================================================='''

OPCODES = {
    'HALT': 0,      # End of program
    'BZ': 1,        # Conditional branch if zero
    'BNZ': 2,       # Conditional branch if not zero
    'JMP': 3,       # Unconditional jump
    'LDN': 4,       # Load node into Rnode
    'ITER': 5,      # Initialize neighbor iteration
    'NEXT': 6,      # Load next neighbor into Rnbr
    'LDV': 7,       # Load edge weight into Rval
    'HASN': 8,      # Check if more neighbors exist
    'HASE': 9,      # Check if there is an edge between Rnode and input node
    'ADD': 10,      # Add edge weight
    'ADDI': 11,     # Add immediate value
    'SUB': 12,      # Subtract edge weight,
    'SUBI': 13,     # Subtract immediate value
    'CMP': 14,      # Compare, set FLAGS
    'MOV': 15,      # Move
    'MOVI': 16,     # Move immediate
    'CLR': 17,      # Zero register
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
    Encode an instruction to 32 bits.
    Simplified encoding:
        [ 5 bits opcode | 27 bits args (packed manually) ]
    '''
    opcode = OPCODES[op] << 27

    # No argument operations
    if op in ['HALT', 'ITER', 'NEXT', 'LDV', 'HASN', 'FEMPTY', 'FSWAP']:
        return opcode
    
    # Immediate operations
    elif op in ['BZ', 'BNZ', 'JMP', 'LDN', 'HASE']:
        imm = int(args[0]) & IMMEDIATE_ARG_MASK
        return opcode | imm
    
    # Register-Register-Register operations
    elif op in ['ADD', 'SUB']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        r2 = int(args[2]) & REGISTER_ARG_MASK
        return opcode | (r0 << 24) | (r1 << 21) | (r2 << 18)
    
    # Register-Register-Constant operations
    elif op in ['ADDI', 'SUBI']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        const = int(args[2]) & REG_CONSTANT_ARG_MASK
        return opcode | (r0 << 24) | (r1 << 21) | const
    
    # Register-Register operations
    elif op in ['CMP', 'MOV', 'LDR', 'STR']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        return opcode | (r0 << 24) | (r1 << 21)

    # Register-Constant operations
    elif op in ['MOVI', 'LD', 'ST']:
        r = int(args[0]) & REGISTER_ARG_MASK
        const = int(args[1]) & CONSTANT_ARG_MASK
        return opcode | (r << 24) | const

    # Register operations
    elif op in ['CLR', 'PUSH', 'POP']:
        r = int(args[0]) & REGISTER_ARG_MASK
        return opcode | (r << 24)

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
        'Rtmp': 4,
        'Rzero': 5,
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
            # Allow decimal or hex
            mem.append(int(line, 0))

    return code, data, mem

'''=== Binary Output ========================================================================'''

def write_binary(filename, code, data, mem):
    with open(filename, 'wb') as f:
        # Header: sizes (each 4 bytes)
        f.write(struct.pack('<IIIII', len(code), len(data['row_index']), len(data['col_index']), len(data['values']), len(mem)))

        # Sections
        for section in (code, data['row_index'], data['col_index'], data['values'], mem):
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