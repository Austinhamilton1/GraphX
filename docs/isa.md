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
| 0 | `FLAG_I` | If set, use `Imm` value instead of register `Src2` |
| 1 | `FLAG_F` | If set, use float mode |
| 2-7 | Reserved | Future use (e.g., vector, predicate flags) |

### Register File

#### Graph registers:
 - `Rnode` - Current node
 - `Rnbr` - Next neighbor of current node
 - `Rval` - Edge weight

#### Integer registers:
 - `Racc` - Accumulator integer register
 - `Rtmp1-16` - General purpose integer registers

#### Floating point registers:
 - `Facc` - Accumulator float register
 - `Ftmp1-16` - General purpose float registers

#### Read-only registers:
 - `Rzero` - Integer zero register
 - `Fzero` - Float zero register
 - `Rcore` - Core ID
 - `FLAGS` - Stores result of compare operation
 - `niter[0-3]` - 4 node iterators
 - `eiter` - Edge iterator

## Instruction Categories

### Control Flow
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `HALT` | 0 | Halt the execution of the program | N/A | None  |
| `BZ` | 1 | Conditional branch if zero | `if(FLAGS == FLAG_ZERO) then PC = Imm` | `FLAG_I` |
| `BNZ` | 2 | Conditional branch if not zero | `if(FLAGS != FLAG_ZERO) then PC = Imm` | `FLAG_I` |
| `BLT` | 3 | Conditional branch if less than | `if(FLAGS == FLAG_NEG) then PC = Imm` | `FLAG_I` |
| `BGE` | 4 | Conditional branch if greater than or equal | `if(FLAGS == FLAG_POS) then  PC = Imm` | `FLAG_I` |
| `JMP` | 5 | Unconditional jump | `PC = Imm` | `FLAG_I` |


### Graph Access Instructions
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `NITER` | 6 | Initiate one of the 4 node iterators | `niter[Imm] = 0` | `FLAG_I` |
| `NNEXT` | 7 | Store the next neighbor (`v`, `weight`) of `Rnode` in (`Rnbr`, `Rval`) | `Rnbr = graph_nodes[Rnode + niter[Imm]]; Rval = graph_weights[Rnode + niter[Imm]]; niter[Imm]++` | `FLAG_I` |
| `EITER` | 8 | Initiate the edge iterator | `eiter = 0; Rnode = 0` | None |
| `ENEXT` | 9 | Store the next edge (`u`, `v`, `weight`) in (`Rnode`, `Rnbr`, `Rval`) | `Rnode` = graph_nodes[Rnode + eiter]; Rval = graph_weights[Rnode + eiter]` | None |
| `HASE` | 10 | Check if there exists an edge between `Rnode` and `Rnbr` | if(has_edge(Rnode, Rnbr)) then FLAGS = ~FLAG_ZERO | None |
| `DEG` | 11 | Store the out degree of node `dest` in `Rval` | `Rval = degree(Dest)` | None |

### Arithmetic and Logic
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `ADD` | 12 | Add two integer registers together and store in a third register | `Dest = Src1 + Src2` | None |
| `ADDI` | 13 | Add an immediate value to an integer register and store it in another register | `Dest = Src1 + Imm` | `FLAG_I` |
| `ADDF` | 14 | Add two floating point registers together and store in a third register | `Dest = Src1 + Src2` | `FLAG_F` |
| `ADDFI` | 15 | Add an immediate value to a floating point register and storer it in another register | `Dest = Src1 + Imm` | `FLAG_I`, `FLAG_F` |
| `SUB` | 16 | Subtract an integer register from another register and store in a third register | `Dest = Src1 - Src2` | None |
| `SUBI` | 17 | Subtract an immediate value from an integer register and store it in another register | `Dest = Src1 - Imm` | `FLAG_I` |
| `SUBF` | 18 | Subtract a floating point register from another register and store in a third register | `Dest = Src1 - Src2` | `FLAG_F` |
| `SUBFI` | 19 | Subtract an immediate value from a floating point register and store in another register | `Dest = Src1 - Imm` | `FLAG_I`, `FLAG_F` |
| `MULT` | 20 | Multiply two integer registers and store in a third register | `Dest = Src1 * Src2` | None |
| `MULTI` | 21 | Multiply an integer register by an immediate value and store in another register | `Dest = Src1 * Imm` | `FLAG_I` |
| `DIV` | 22 | Divide an integer register by another register and store in a third register | `Dest = Src1 / Src2` | None |
| `DIVI` | 23 | Divde an integer register by an immediate value and store in another register | `Dest = Src1 / Imm` | `FLAG_I` |
| `MULTF` | 24 | Multiply two floating point registers and store in a third register | `Dest = Src1 * Src2` | `FLAG_F` |
| `MULTFI` | 25 | Multiply a floating point register by an immediate value and store in another register | `Dest = Src1 * Imm` | `FLAG_I`, `FLAG_F` |
| `DIVF` | 26 | Divide a floating point register by another register and store in a third register | `Dest = Src1 / Src2` | `FLAG_F` |
| `DIVFI` | 27 | Divide a floating point register by an immediate value and store in another register | `Dest = Src1 / Imm` | `FLAG_I`, `FLAG_F` |
| `CMP` | 28 | Compare two integer registers and store result in `FLAGS` | `result = Dest - Src1; if(result == 0) then FLAGS = FLAG_ZERO; else if(result < 0) then FLAGS = FLAG_NEG; else FLAGS = FLAG_POS` | None |
| `CMPF` | 29 | Compare two floating point registers and store result in `FLAGS` | `result = Dest - Src1; if(result == 0) then FLAGS = FLAG_ZERO; else if(result < 0) then FLAGS = FLAG_NEG; else FLAGS = FLAG_POS` | `FLAG_F` |
| `MOV` | 30 | Move one integer register into another register | `Dest = Src1` | None |
| `MOVI` | 31 | Move an immediate value into an integer register | `Dest = Imm` | `FLAG_I` |
| `MOVF` | 32 | Move one floating point register into another register | `Dest = Src1` | `FLAG_F` |
| `MOVFI` | 33 | Move an immediate value into a floating point register | `Dest = Imm` | `FLAG_I`, `FLAG_F` |
| `MOVC` | 34 | Move an integer register into a floating point register and convert data types | `Dest = (float)Src1` | None |
| `MOVCF` | 35 | Move a floating point register into an integer register and convert data types | `Dest = (int32_t)Src1` | `FLAG_F` |

### Memory Access
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `LD` | 36 | Load a value from memory into an integer register | `Dest = memory[Imm]` | `FLAG_I` |
| `ST` | 37 | Store a value from an integer register into memory | `memory[Imm] = Dest` | `FLAG_I` |
| `LDF` | 38 | Load a value from memory into a floating point register | `Dest = memory[Imm]` | `FLAG_I` |
| `STF` | 39 | Store a value from a floating point register into memory | `memory[Imm] = Dest` | `FLAG_I` |
| `LDR` | 40 | Load a value from memory specified by an integer register into an integer register | `Dest = memory[Src1]` | None |
| `STR` | 41 | Store a value from an integer register into memory specified by an integer register | `memory[Src1] = Dest` | None |
| `LDRF` | 42 | Load a value from memory specified by an integer register into a floating point register | `Dest = memory[Src1]` | `FLAG_F` |
| `STRF` | 43 | Store a value from a floating point register into memory specified by an integer register | `memory[Src1] = Dest` | `FLAG_F` |

### Frontier Control
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `PUSH` | 44 | Push an integer register to the next frontier | `push(next_frontier, Dest)` | None |
| `POP` | 45 | Pop a node from the frontier into an integer register | `Dest = pop(frontier)` | None |
| `FEMPTY` | 46 | Check if frontier is empty and set `FLAGS` | `if(empty(froniter)) then FLAGS = FLAG_ZERO; else FLAGS = ~FLAG_ZERO` | None |
| `FSWAP` | 47 | Swap the frontier with the next frontier | `swap(frontier, next_frontier)` | None |
| `FFILL` | 48 | Fill the frontier with all nodes in the graph | `for(node in graph_nodes) do push(froniter, node)` | None |

### Multicore/Synchronization Control
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `PARALLEL` | 49 | Code implemented until the next `BARRIER` will run on all cores | Not Implemented | None |
| `BARRIER` | 50 | Ensure all cores reach this point before continuing, then switch back to single core execution | Not Implemented | None |
| `LOCK` | 51 | Mutual exclusion lock on a resource | Not Implemented | None |
| `UNLOCK` | 52 | Unlock a resource | Not Implemented | None |