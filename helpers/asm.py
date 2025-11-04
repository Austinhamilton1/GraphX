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

OPCODE_ARG_MASK         = 0x000000FF 
REGISTER_ARG_MASK       = 0x000000FF 
IMMEDIATE_ARG_MASK      = 0xFFFFFFFF
FLAG_I                  = 0x00000001
FLAG_F                  = 0x00000002

register_names = {
    'Rnode': 0,
    'Rnbr': 1,
    'Rval': 2,
    'r1': 3,
    'r2': 4,
    'r3': 5,
    'r4': 6,
    'r5': 7,
    'r6': 8,
    'r7': 9,
    'r8': 10,
    'r9': 11,
    'r10': 12,
    'r11': 13,
    'r12': 14,
    'r13': 15,
    'r14': 16,
    'r15': 17,
    'r16': 18,
    'Rzero': 19,
    'Rcore': 20,
    'f1': 21,
    'f2': 22,
    'f3': 23,
    'f4': 24,
    'f5': 25,
    'f6': 26,
    'f7': 27,
    'f8': 28,
    'f9': 29,
    'f10': 30,
    'f11': 31,
    'f12': 32,
    'f13': 33,
    'f14': 34,
    'f15': 35,
    'f16': 36,
    'Fzero': 37,
}

def reg_type(reg):
    if reg >= register_names['Rnode'] and reg <= register_names['Rcore']:
        return 'r'
    if reg >= register_names['f1'] and reg <= register_names['Fzero']:
        return 'f'

'''=== Instruction Encoding ====================================================='''

OPCODES = {
    'HALT': 0,      # End of program
    'BZ': 1,        # Conditional branch if zero
    'BNZ': 2,       # Conditional branch if not zero
    'BLT': 3,       # Branch if less than
    'BGE': 4,       # Branch if greater than equal to
    'JMP': 5,       # Unconditional jump
    'NITER': 6,     # Initialize neighbor iteration
    'NNEXT': 7,     # Load next neighbor into Rnbr
    'EITER': 8,     # Initialize edge iteration     
    'ENEXT': 9,     # Load next edge into Rnode, Rnbr, Rval
    'HASE': 10,     # Check if there is an edge between Rnode and input node
    'DEG': 11,      # Store the degree of a node in a register
    'ADD': 12,      # Add registers
    'SUB': 13,      # Subtract registers
    'MUL': 14,      # Multiply registers
    'DIV': 15,      # Divide two registers
    'CMP': 16,      # Compare, set FLAGS
    'MOV': 17,      # Move
    'MOVC': 18,     # Cast a register to a float register
    'LD': 19,       # Load register from memory
    'ST': 20,       # Store register to memory
    'FPUSH': 21,    # Add neighbor to next frontier
    'FPOP': 22,     # Load next node from frontier
    'FEMPTY': 23,   # Check if frontier is empty
    'FSWAP': 24,    # Swap next frontier and current frontier buffers
    'FFILL': 25,    # Fill the frontier with all nodes in graph
    # 'PARALLEL': , # Run next block of code with multicore mode
    # 'BARRIER': ,  # Wait until all cores reach this code before continuing
    # 'LOCK': ,     # Mutual exclusion lock on a resource
    # 'UNLOCK': ,   # Unlock mutual exclusion lock
}

def encode_instruction(op, args):
    '''
    Encode an instruction to 64 bits.
    Simplified encoding:
        [ 8 bit opcode | 8 bit flags | 8 bit dst | 8 bit src1 | 32 bit imm (top 8 used for register) ]
    '''
    opcode = OPCODES[op] << 56

    # No argument operations
    if op in ['HALT', 'EITER', 'ENEXT', 'FEMPTY', 'FSWAP', 'HASE', 'FFILL', 'PARALLEL', 'BARRIER']:
        return opcode
    
    # I operations
    elif op in ['BZ', 'BNZ', 'BLT', 'BGE', 'JMP', 'NITER', 'NNEXT', 'LOCK', 'UNLOCK']:
        imm = int(args[0][1:]) & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 48) | imm
    
    # R-R-R/I operations
    elif op in ['ADD', 'SUB', 'MUL', 'DIV']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK

        rtype = reg_type(r0)
        
        # Check for immediate instruction
        if args[2].startswith('#'):
            # Check for float instruction
            if rtype == 'f':
                # Set the correct register indexes
                r0 -= register_names['f1']
                r1 -= register_names['f1']

                # Set immediate value and flags
                r2 = struct.unpack('<I', struct.pack('<f', float(args[2][1:])))[0] & IMMEDIATE_ARG_MASK
                flags = FLAG_I | FLAG_F
            elif rtype == 'r':
                r2 = int(args[2][1:]) & IMMEDIATE_ARG_MASK
                flags = FLAG_I
            else:
                raise ValueError('Invalid register')
        else:
            # Check for float instruction
            if rtype == 'f':
                # Set the correct register indexes
                r0 -= register_names['f1']
                r1 -= register_names['f1']

                # Set correct register index and flags
                r2 = ((int(args[2]) - register_names['f1']) & REGISTER_ARG_MASK) << 24
                flags = FLAG_F
            elif rtype == 'r':
                r2 = (int(args[2]) & REGISTER_ARG_MASK) << 24
                flags = 0x0
            else:
                raise ValueError('Invalid register')

        return opcode | (flags << 48) | (r0 << 40) | (r1 << 32) | r2
    
    # R-R/I integer operations
    elif op in ['MOV', 'CMP', 'MOVC', 'LD', 'ST']:
        r0 = int(args[0]) & REGISTER_ARG_MASK

        rtype = reg_type(r0)

        # Check for immediate instruction
        if args[1].startswith('#'):
            # Check for float instruction
            if rtype == 'f':
                # Set the correct register indexes
                r0 -= register_names['f1']

                # Set immediate value and flags
                r1 = struct.unpack('<I', struct.pack('<f', float(args[1][1:])))[0] & IMMEDIATE_ARG_MASK
                flags = FLAG_I | FLAG_F
            elif rtype == 'r' or rtype == 'm':
                r1 = int(args[1][1:]) & IMMEDIATE_ARG_MASK
                flags = FLAG_I
            else:
                raise ValueError('Invalid register')
        else:
            # Check for float instruction
            if rtype == 'f':
                # Set the correct register indexes
                r0 -= register_names['f1']
                r1 = int(args[1])
                if reg_type(r1) == 'f':
                    r1 -= register_names['f1']

                r1 = (r1 & REGISTER_ARG_MASK) << 32
                flags = FLAG_F
            elif rtype == 'r' or rtype == 'm':
                r1 = (int(args[1]) & REGISTER_ARG_MASK) << 32
                flags = 0x0
            else:
                raise ValueError('Invalid register')

        return opcode | (flags << 48) | (r0 << 40) | r1

    # R operations
    elif op in ['FPUSH', 'FPOP', 'DEG']:
        r = int(args[0]) & REGISTER_ARG_MASK
        return opcode | (r << 40)

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
                    args[i] = str(register_names[args[i]])
                elif args[i] in code_labels:
                    args[i] = '#' + str(code_labels[args[i]])

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
                if token.endswith('f'):
                    mem.append(float(token, 0))
                else:
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
                f.write(struct.pack('<i', word))

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