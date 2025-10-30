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
    'MULT': 14,     # Multiply registers
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
    'PARALLEL': 26, # Run next block of code with multicore mode
    'BARRIER': 27,  # Wait until all cores reach this code before continuing
    'LOCK': 28,     # Mutual exclusion lock on a resource
    'UNLOCK': 29,   # Unlock mutual exclusion lock
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
    elif op in ['ADD', 'SUB', 'MULT', 'DIV']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        
        # Check for immediate instruction
        if args[2].startswith('#'):
            # Check for float instruction
            if r0 >= 22:
                # Set the correct register indexes
                r0 -= 22
                r1 -= 22

                # Set immediate value and flags
                r2 = struct.unpack('<I', struct.pack('<f', float(args[2][1:])))[0] & IMMEDIATE_ARG_MASK
                flags = FLAG_I | FLAG_F
            else:
                r2 = int(args[2][1:]) & IMMEDIATE_ARG_MASK
                flags = FLAG_I
        else:
            # Check for float instruction
            if r0 >= 22:
                # Set the correct register indexes
                r0 -= 22
                r1 -= 22

                # Set correct register index and flags
                r2 = ((int(args[2]) - 22) & REGISTER_ARG_MASK) << 24
                flags = FLAG_F
            else:
                r2 = (int(args[2]) & REGISTER_ARG_MASK) << 24
                flags = 0x0

        return opcode | (flags << 48) | (r0 << 40) | (r1 << 32) | r2
    
    # R-R/I integer operations
    elif op in ['MOV', 'CMP', 'MOVC', 'LD', 'ST']:
        r0 = int(args[0]) & REGISTER_ARG_MASK

        # Check for immediate instruction
        if args[1].startswith('#'):
            # Check for float instruction
            if r0 >= 22:
                # Set the correct register indexes
                r0 -= 22

                # Set immediate value and flags
                r1 = struct.unpack('<I', struct.pack('<f', float(args[1][1:])))[0] & IMMEDIATE_ARG_MASK
                flags = FLAG_I | FLAG_F
            else:
                r1 = int(args[1][1:]) & IMMEDIATE_ARG_MASK
                flags = FLAG_I
        else:
            # Check for float instruction
            if r0 >= 22:
                # Set the correct register indexes
                r0 -= 22
                r1 = int(args[1])
                if r1 >= 22:
                    r1 -= 22

                r1 = (r1 & REGISTER_ARG_MASK) << 32
                flags = FLAG_F
            else:
                r1 = (int(args[1]) & REGISTER_ARG_MASK) << 32
                flags = 0x0

        return opcode | (flags << 48) | (r0 << 40) | r1

    # Register operations
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
    register_names = {
        'Rnode': 0,
        'Rnbr': 1,
        'Rval': 2,
        'Racc': 3,
        'Rtmp1': 4,
        'Rtmp2': 5,
        'Rtmp3': 6,
        'Rtmp4': 7,
        'Rtmp5': 8,
        'Rtmp6': 9,
        'Rtmp7': 10,
        'Rtmp8': 11,
        'Rtmp9': 12,
        'Rtmp10': 13,
        'Rtmp11': 14,
        'Rtmp12': 15,
        'Rtmp13': 16,
        'Rtmp14': 17,
        'Rtmp15': 18,
        'Rtmp16': 19,
        'Rzero': 20,
        'Rcore': 21,
        'Facc': 22,
        'Ftmp1': 23,
        'Ftmp2': 24,
        'Ftmp3': 25,
        'Ftmp4': 26,
        'Ftmp5': 27,
        'Ftmp6': 28,
        'Ftmp7': 29,
        'Ftmp8': 30,
        'Ftmp9': 31,
        'Ftmp10': 32,
        'Ftmp11': 33,
        'Ftmp12': 34,
        'Ftmp13': 35,
        'Ftmp14': 36,
        'Ftmp15': 37,
        'Ftmp16': 38,
        'Fzero': 39,
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