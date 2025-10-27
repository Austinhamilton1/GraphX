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
FLAG_R                  = 0x00000001
FLAG_I                  = 0x00000002
FLAG_N                  = 0x00000004
FLAG_F                  = 0x00000008

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
    'ADDI': 13,     # Add immediate value
    'ADDF': 14,     # Add float registers
    'ADDFI': 15,    # Add immediate float value
    'SUB': 16,      # Subtract registers,
    'SUBI': 17,     # Subtract immediate value
    'SUBF': 18,     # Subtract float registers
    'SUBFI': 19,    # Subtract immediate float value
    'MULT': 20,     # Multiply registers
    'MULTI': 21,    # Multiply register by immediate
    'DIV': 22,      # Divide two registers
    'DIVI': 23,     # Divide register by immediate
    'MULTF': 24,    # Multiply two float registers
    'MULTFI': 25,   # Multiply float register by float immediate
    'DIVF': 26,     # Divide float registers
    'DIVFI': 27,    # Divide float register by float immediate
    'CMP': 28,      # Compare, set FLAGS
    'CMPF': 29,     # Compare floats, set FLAGS
    'MOV': 30,      # Move
    'MOVI': 31,     # Move immediate
    'MOVF': 32,     # Move float
    'MOVFI': 33,    # Move float immediate
    'MOVC': 34,     # Cast a register to a float register
    'MOVCF': 35,    # Cast a float register to a register
    'LD': 36,       # Load register from memory
    'ST': 37,       # Store register to memory
    'LDF': 38,      # Load float register from memory
    'STF': 39,      # Store float register to memory
    'LDR': 40,      # Load from register address
    'STR': 41,      # Store to register address
    'LDRF': 42,     # Load float from register address
    'STRF': 43,     # Store float to register address
    'PUSH': 44,     # Add neighbor to next frontier
    'POP': 45,      # Load next node from frontier
    'FEMPTY': 46,   # Check if frontier is empty
    'FSWAP': 47,    # Swap next frontier and current frontier buffers
    'FFILL': 48,    # Fill the frontier with all nodes in graph
}

def encode_instruction(op, args):
    '''
    Encode an instruction to 64 bits.
    Simplified encoding:
        [ 8 bit opcode | 8 bit flags | 8 bit dst | 8 bit src1 | 32 bit imm (top 8 used for register) ]
    '''
    opcode = OPCODES[op] << 56

    # No argument operations
    if op in ['HALT', 'EITER', 'ENEXT', 'FEMPTY', 'FSWAP', 'HASE', 'FFILL']:
        return opcode | (FLAG_R << 48) | (FLAG_N << 48)
    
    # Immediate operations
    elif op in ['BZ', 'BNZ', 'BLT', 'BGE', 'JMP', 'NITER', 'NNEXT']:
        imm = int(args[0]) & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 48) | (FLAG_N << 48) | imm
    
    # Register-Register-Register integer operations
    elif op in ['ADD', 'SUB', 'MULT', 'DIV']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        r2 = int(args[2]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 48) | (FLAG_N << 48) | (r0 << 40) | (r1 << 32) | (r2 << 24)
    
    # Register-Register-Register float operations
    elif op in ['ADDF', 'SUBF', 'MULTF', 'DIVF']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        r2 = int(args[2]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 48) | (FLAG_F << 48) | (r0 << 40) | (r1 << 32) | (r2 << 24)

    # Register-Register-Immediate integer operations
    elif op in ['ADDI', 'SUBI', 'MULTI', 'DIVI']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        imm = int(args[2]) & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 48) | (FLAG_N << 48) | (r0 << 40) | (r1 << 32) | imm
    
    # Register-Register-Immediate float operations
    elif op in ['ADDFI', 'SUBFI', 'MULTFI', 'DIVFI']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        imm = float(args[2])
        imm_bits = struct.unpack('<I', struct.pack('<f', imm))[0] & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 48) | (FLAG_F << 48) | (r0 << 40) | (r1 << 32) | imm_bits

    # Register-Register integer operations
    elif op in ['CMP', 'MOV', 'LDR', 'STR', 'MOVC']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 48) | (FLAG_N << 48) | (r0 << 40) | (r1 << 32)

    # Register-Register float operations
    elif op in ['CMPF', 'MOVF', 'LDRF', 'STRF', 'MOVCF']:
        r0 = int(args[0]) & REGISTER_ARG_MASK
        r1 = int(args[1]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 48) | (FLAG_F << 48) | (r0 << 40) | (r1 << 32)

    # Register-Immediate integer operations
    elif op in ['MOVI', 'LD', 'ST']:
        r = int(args[0]) & REGISTER_ARG_MASK
        imm = int(args[1]) & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 48) | (FLAG_N << 48) | (r << 40) | imm

    # Register-Immediate float operations
    elif op in ['MOVFI', 'LDF', 'STF']:
        r = int(args[0]) & REGISTER_ARG_MASK
        imm = float(args[1])
        imm_bits = struct.unpack('<I', struct.pack('<f', imm))[0] & IMMEDIATE_ARG_MASK
        return opcode | (FLAG_I << 48) | (FLAG_F << 48) | (r << 40) | imm_bits

    # Register operations
    elif op in ['PUSH', 'POP', 'DEG']:
        r = int(args[0]) & REGISTER_ARG_MASK
        return opcode | (FLAG_R << 48) | (FLAG_N << 48) | (r << 40)

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
        'Rzero': 8,
        'Facc': 0,
        'Ftmp1': 1,
        'Ftmp2': 2,
        'Ftmp3': 3,
        'Ftmp4': 4,
        'Fzero': 5,
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