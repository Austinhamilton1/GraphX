# GraphX Instruction Set Architecture

## Overview

The **GraphX ISA** defines a 64-bit instruction format designed for graph processing acceleration. It supports integer and floating-point operations, graph-specific data movement, and parallel synchronization primitives. The ISA is designed to be implemented in hardware on FPGA or ASIC platforms, supporting multiple execution cores.

---

## Instruction Format

### General Instruction Layout (64 bits total)
| **Bits** | **Field Name** | **Description** |
| -------- | -------------- | --------------- |
| 63-56 | **Opcode** | Operation code (defines which instruction to execute) |
| 55-48 | **Flags** | Type/modifier bits (e.g., immediate/register, integer/float) |
| 47-40 | **Dest** | Destination register index |
| 39-32 | **Src1** | Source register 1 index |
| 31-0 | **Src2/Imm** | Immediate value or Source register 2 index, depending on flag |

### Flag Bit Usage
| **Bit** | **Name** | **Meaning** |
| ------- | -------- | ----------- |
| 0 | `FLAG_R` | If set, use register `Src2` |
| 1 | `FLAG_I` | If set, use `Imm` value |
| 2 | `FLAG_N` | If set, use integer mode |
| 3 | `FLAG_F` | If set, use float mode |
| 4-7 | Reserved | Future use (e.g., vector, predicate flags) |

### Register File

#### Graph registers:
 - `Rnode` - Current node
 - `Rnbr` - Next neighbor of current node
 - `Rval` - Edge weight

#### Integer registers:
 - `Racc` - Accumulator integer register
 - `Rtmp1` - General purpose integer register
 - `Rtmp2` - General purpose integer register
 - `Rtmp3` - General purpose integer register
 - `Rtmp4` - General purpose integer register

#### Floating point registers:
 - `Facc` - Accumulator float register
 - `Ftmp1` - General purpose float register
 - `Ftmp2` - General purpose float register
 - `Ftmp3` - General purpose float register
 - `Ftmp4` - General purpose float register

#### Read-only registers:
 - `Rzero` - Integer zero register
 - `Fzero` - Float zero register
 - `Rcore` - Core ID
 - `FLAGS` - Stores result of compare operation

## Instruction Categories

### Control Flow
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `HALT` | 0 | Halt the execution of the program | N/A | `FLAG_R \| FLAG_N`  |
| `BZ` | 1 | Conditional branch if zero | `if(FLAGS == FLAG_ZERO) then PC = Imm` | `FLAG_I \| FLAG_N` |
| `BNZ` | 2 | Conditional branch if not zero | `if(FLAGS != FLAG_ZERO) then PC = Imm` | `FLAG_I \| FLAG_N` |
| `BLT` | 3 | Conditional branch if less than | `if(FLAGS == FLAG_NEG) then PC = Imm` | `FLAG_I \| FLAG_N` |
| `BGE` | 4 | Conditional branch if greater than or equal | `if(FLAGS == FLAG_POS) then  PC = Imm` | `FLAG_I \| FLAG_N` |
| `JMP` | 5 | Unconditional jump | `PC = Imm` | `FLAG_I \| FLAG_N` |


### Graph Access Instructions
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `NITER` | 6 | Initiate one of the 4 node iterators |  |  |
| `NNEXT` | 7 | Store the next neighbor of `Rnode` in `Rnbr` |  |  |
| `EITER` | 8 | Initiate the edge iterator |  |  |
| `ENEXT` | 9 | Store the next edge (`u`, `v`, `weight`) in (`Rnode`, `Rnbr`, `Rval`) |  |  |
| `HASE` | 10 | Check if there exists an edge between `Rnode` and `Rnbr` |  |  |
| `DEG` | 11 | Store the out degree of node `Rnode` in `Rval` |  |  |

### Arithmetic and Logic
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `ADD` | 12 | Add two integer registers together and store in a third register |  |  |
| `ADDI` | 13 | Add an immediate value to an integer register and store it in another register |  |  |
| `ADDF` | 14 | Add two floating point registers together and store in a third register |  |  |
| `ADDFI` | 15 | Add an immediate value to a floating point register and storer it in another register |  |  |
| `SUB` | 16 | Subtract an integer register from another register and store in a third register |  |  |
| `SUBI` | 17 | Subtract an immediate value from an integer register and store it in another register |  |  |
| `SUBF` | 18 | Subtract a floating point register from another register and store in a third register |  |  |
| `SUBFI` | 19 | Subtract an immediate value from a floating point register and store in another register |  |  |
| `MULT` | 20 | Multiply two integer registers and store in a third register |  |  |
| `MULTI` | 21 | Multiply an integer register by an immediate value and store in another register |  |  |
| `DIV` | 22 | Divide an integer register by another register and store in a third register |  |  |
| `DIVI` | 23 | Divde an integer register by an immediate value and store in another register |  |  |
| `MULTF` | 24 | Multiply two floating point registers and store in a third register |  |  |
| `MULTFI` | 25 | Multiply a floating point register by an immediate value and store in another register |  |  |
| `DIVF` | 26 | Divide a floating point register by another register and store in a third register |  |  |
| `DIVFI` | 27 | Divide a floating point register by an immediate value and store in another register |  |  |
| `CMP` | 28 | Compare two integer registers and store result in `FLAGS` |  |  |
| `CMPF` | 29 | Compare two floating point registers and store result in `FLAGS` |  |  |
| `MOV` | 30 | Move one integer register into another register |  |  |
| `MOVI` | 31 | Move an immediate value into an integer register |  |  |
| `MOVF` | 32 | Move one floating point register into another register |  |  |
| `MOVFI` | 33 | Move an immediate value into a floating point register |  |  |
| `MOVC` | 34 | Move an integer register into a floating point register and convert data types |  |  |
| `MOVCF` | 35 | Move a floating point register into an integer register and convert data types |  |  |

### Memory Access
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `LD` | 36 | Load a value from memory into an integer register |  |  |
| `ST` | 37 | Store a value from an integer register into memory |  |  |
| `LDF` | 38 | Load a value from memory into a floating point register |  |  |
| `STF` | 39 | Store a value from a floating point register into memory |  |  |
| `LDR` | 40 | Load a value from memory specified by an integer register into an integer register |  |  |
| `STR` | 41 | Store a value from an integer register into memory specified by an integer register |  |  |
| `LDRF` | 42 | Load a value from memory specified by an integer register into a floating point register |  |  |
| `STRF` | 43 | Store a value from a floating point register into memory specified by an integer register |  |  |

### Frontier Control
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `PUSH` | 44 | Push an integer register to the next frontier |  |  |
| `POP` | 45 | Pop a node from the frontier into an integer register |  |  |
| `FEMPTY` | 46 | Check if frontier is empty and set `FLAGS` |  |  |
| `FSWAP` | 47 | Swap the frontier with the next frontier |  |  |
| `FFILL` | 48 | Fill the frontier with all nodes in the graph |  |  |

### Multicore/Synchronization Control
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `PARALLEL` | 49 | Code implemented until the next `BARRIER` will run on all cores |  |  |
| `BARRIER` | 50 | Ensure all cores reach this point before continuing, then switch back to single core execution |  |  |
| `LOCK` |  | 51 | Mutual exclusion lock on a resource |  |
| `UNLOCK` | 52 | Unlock a resource |  |  |