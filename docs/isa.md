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
| `ENEXT` | 9 | Store the next edge (`u`, `v`, `weight`) in (`Rnode`, `Rnbr`, `Rval`) | `Rnode = graph_nodes[Rnode + eiter]; Rval = graph_weights[Rnode + eiter]` | None |
| `HASE` | 10 | Check if there exists an edge between `Rnode` and `Rnbr` | `if(has_edge(Rnode, Rnbr)) then FLAGS = ~FLAG_ZERO` | None |
| `DEG` | 11 | Store the out degree of node `dest` in `Rval` | `Rval = degree(Dest)` | None |

### Arithmetic and Logic
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `ADD` | 12 | Add two values together and store in a register | `Dest = Src1 + Src2/Imm` | `FLAG_I`, `FLAG_F` |
| `SUB` | 13 | Subtract a value from a register and store in a register | `Dest = Src1 - Src2/Imm` | `FLAGS_I`, `FLAGS_F` |
| `MULT` | 14 | Multiply two integer registers and store in a third register | `Dest = Src1 * Src2/Imm` | `FLAG_I`, `FLAG_F` |
| `DIV` | 15 | Divide a register by a value and store in a register | `Dest = Src1 / Src2/Imm` | `FLAG_I`, `FLAG_F` |
| `CMP` | 16 | Compare two registers and store result in `FLAGS` | `result = Dest - Src1; set(flags, result)` | `FLAG_F` |
| `MOV` | 17 | Move one register into another register | `Dest = Src1` | `FLAG_I`, `FLAG_F` |
| `MOVC` | 18 | Move a register into another register and convert data types | `Dest = (float)Src1`, or `Dest = (int32_t)Src1` | `FLAG_F` |

### Memory Access
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `LD` | 19 | Load a value from memory into a register | `Dest = memory[Src1/Imm]` | `FLAG_I`, `FLAG_F` |
| `ST` | 20 | Store a value from an register into memory | `memory[Src1/Imm] = Dest` | `FLAG_I`, `FLAG_F` |

### Frontier Control
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `FPUSH` | 21 | Push an integer register to the next frontier | `push(next_frontier, Dest)` | None |
| `FPOP` | 22 | Pop a node from the frontier into an integer register | `Dest = pop(frontier)` | None |
| `FEMPTY` | 23 | Check if frontier is empty and set `FLAGS` | `if(empty(froniter)) then FLAGS = FLAG_ZERO; else FLAGS = ~FLAG_ZERO` | None |
| `FSWAP` | 24 | Swap the frontier with the next frontier | `swap(frontier, next_frontier)` | None |
| `FFILL` | 25 | Fill the frontier with all nodes in the graph | `for(node in graph_nodes) do push(froniter, node)` | None |

### Multicore/Synchronization Control
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `PARALLEL` | 26 | Code implemented until the next `BARRIER` will run on all cores | Not Implemented | None |
| `BARRIER` | 27 | Ensure all cores reach this point before continuing, then switch back to single core execution | Not Implemented | None |
| `LOCK` | 28 | Mutual exclusion lock on a resource | Not Implemented | None |
| `UNLOCK` | 29 | Unlock a resource | Not Implemented | None |

## Instruction Semantics

### `HALT`
 - **Opcode:** 0x0
 - **Operands:** None (flags ignored)
 - **Effect:**
    - Suspends the execution of the VM
 - **Usage:**
```
...
CMP Rtmp1, Rzero
BZ done
...

done:
    HALT                    ; Program is finished
```

### `BZ`
 - **Opcode:** 0x1
 - **Operands:** Immediate PC location (flags ignored)
 - **Effect:**
    - Check current `FLAGS` register
    - If `FLAGS` has `FLAG_ZERO` bit set:
        - Jump to `Imm`
    - Otherwise
        - Continue execution
 - **Usage:**
```
...
loop:
    CMP Rtmp1, Rzero
    BZ done                 ; If Rtmp1 == Rzero, break out of the current loop

    SUB Rtmp1, Rtmp1, #1
    JMP loop
...

done:
    HALT
```

### `BNZ`
 - **Opcode:** 0x2
 - **Operands:** Immediate PC location (flags ignored)
 - **Effect:**
    - Check current `FLAGS` register
    - If `FLAGS` does not have `FLAGS_ZERO` bit set:
        - Jump to `Imm`
    - Otherwise:
        - Continue execution
 - **Usage:**
```
 ...
loop:
    FPOP Rnode

    FEMPTY
    BNZ loop                ; If the frontier is not empty, loop again
...
```

### `BLT`
 - **Opcode:** 0x3
 - **Operands:** Immediate PC location (flags ignored)
 - **Effect:**
    - Check current `FLAGS` register
    - If `FLAGS` has `FLAGS_NEG` bit set:
        - Jump to `Imm`
    - Otherwise:
        - Continue execution
 - **Usage:**
```
...
loop:
    ADD Rtmp1, #1
    CMP Rtmp1, Rtmp2
    BLT loop                ; If Rtmp1 is still less than Rtmp2, loop again
...
```

### `BGE`
 - **Opcode:** 0x4
 - **Operands:** Immediate PC location (flags ignored)
 - **Effect:**
    - Check current `FLAGS` register
    - If `FLAGS` has `FLAG_POS` or `FLAG_ZERO` bits set:
        - Jump to `Imm`
    - Otherwise:
        - Continue execution
 - **Usage:**
```
...
loop:
    SUB Rtmp1, #1
    CMP Rtmp1, Rzero
    BGE loop                ; If Rtmp1 is still greater than or equal to 0, loop again
...
```

### `JMP`
 - **Opcode:** 0x5
 - **Operands:** Immediate PC location (flags ignored)
 - **Effect:**
    - Jump to `Imm`
 - **Usage:**
```
...
JMP next_iter               ; Jump to the next_iter label
...
next_iter:
    LD Rtmp1, Rtmp2
...
```

### `NITER`
 - **Opcode:** 0x6
 - **Operands:** Immediate value 0-3. Determines which iterator to initialize (flags ignored)
 - **Effect:**
    - Initialize the specified `niter` register to 0
    - Subsequent calls to `NNEXT` receive the next neighbor of `Rnode`
 - **Usage:**
```
...
NITER #0                   ; Initialize niter[0] = 0
NNEXT #0
...
```

### `NNEXT`
 - **Opcode:** 0x7
 - **Operands:** Immedate value 0-3. Determines which iterator to use (flags ignored) 
 - **Effect:**
    - If there are no other neighbors:
        - Set `FLAGS` = `FLAG_ZERO`
    - Otherwise
        - Get the node at `graph->col_index[graph->row_index[Rnode] + niter[Imm]]` (next neighbor of `Rnode`)
            - Store this in `Rnbr`
        - Get the value at `graph->values[graph->row_index[Rnode] + niter[Imm]]` (weight between `Rnode` and new `Rnbr`)
            - Store this in `Rval`
        - Increment `niter[Imm]`
 - **Usage:**
```
...
NITER #0
NNEXT #0                    ; Rnbr now holds next neighbor, Rval now holds weight
... 
```

### `EITER`
 - **Opcode:** 0x8
 - **Operands:** None (flags ignored)
 - **Effect:**
    - Initialize `eiter` register to 0 and Rnode register to 0
    - Subsequent calls to `ENEXT` will put the next edge (`u`, `v`, `weight`) into (`Rnode`, `Rnbr`, `Rval`)
 - **Usage:**
```
...
EITER                       ; Initializes eiter = 0, Rnode = 0
ENEXT
...
```

### `ENEXT`
 - **Opcode:** 0x9
 - **Operands:** None (flags ignored)
 - **Effect:**
    - If there are no more edges:
        - Set `FLAGS` = `FLAG_ZERO`
    - Otherwise
        - If `Rnode` has no more neighbors:
            - Increment `Rnode`
        - Otherwise
            - Get the node at `graph->col_index[graph->row_index[Rnode] + eiter]`
                - Store this in `Rnbr`
            - Get the value at `graph->values[graph->row_index[Rnode] + eiter]`
                - Store this in `Rval`
            Increment `eiter`
 - **Usage:**
```
...
EITER
ENEXT                       ; Rnode now holds u, Rnbr now holds v, Rval now holds weight                   
...
```

### `HASE`
 - **Opcode:** 0xA
 - **Operands:** None (flags ignored)
 - **Effect:**
    - Determine if an edge exists between `Rnode` and `Rnbr`
    - If it does
        - Set `FLAGS` = `~FLAGS_ZERO`
    - Otherwise
        - Set `FLAGS` = `FLAGS_ZERO`
 - **Usage:**
```
...
FPOP Rnode
MOV Rnbr, #3
HASE                        ; Sets flags based on whether an edge exists between Rnode and Rnbr
BZ done
...
```

### `DEG`
 - **Opcode:** 0xB
 - **Operands:** Node register (flags ignored)
 - **Effect:**
    - Number of outbound connections counted for `Src1`
    - Connection count stored in `Rval`
 - **Usage:**
```
...
DEG Rtmp1                   ; Rval now contains the out degree of Rtmp1
MOV Rtmp2, Rval
...
```

### `ADD`
 - **Opcode:** 0xC
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `SUB`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `MULT`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `DIV`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `CMP`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `MOV`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `MOVC`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `LD`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `ST`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `FPUSH`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `FPOP`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `FEMPTY`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `FSWAP`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `FFILL`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `PARALLEL`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `BARRIER`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `LOCK`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```

### `UNLOCK`
 - **Opcode:** 0x
 - **Operands:** 
 - **Effect:**
    -
 - **Usage:**
```

```