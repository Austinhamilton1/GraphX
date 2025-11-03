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
| `MUL` | 14 | Multiply two integer registers and store in a third register | `Dest = Src1 * Src2/Imm` | `FLAG_I`, `FLAG_F` |
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

### Vectorized Instructions
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `VADD` | 26 | Vectorized addition | `Dest[0:3] = Src1[0:3] + Src2[0:3]` | `FLAG_F` |
| `VSUB` | 27 | Vectorized subtraction | `Dest[0:3] = Src1[0:3] - Src2[0:3]` | `FLAG_F` |
| `VMUL` | 28 | Vectorized multiplication | `Dest[0:3] = Src1[0:3] * Src2[0:3]` | `FLAG_F` |
| `VDIV` | 29 | Vectorized division | `Dest[0:3] = Src1[0:3] / Src2[0:3]` | `FLAG_F` |
| `VLD` | 30 | Vectorized load | `Dest[0:3] = memory[Imm:Imm+3]` or `Dest[0:3] = memory[Src1:Src1+3]` | `FLAG_I`, `FLAG_F` |
| `VST` | 31 | Vectorized store | `memory[Imm:Imm+3] = Dest[0:3]` or `memory[Src1:Src1+3] = Dest[0:3]` | `FLAG_I`, `FLAG_F` |
| `VSET` | 32 | Set all values in a vector to an immediate value | `Dest[0:3] = Imm` | `FLAG_F` |
| `VSUM` | 33 | Sum all values in a vector and store result in a register | `Dest = sum(Src1[0:3])` | `FLAG_F` |

### Multicore/Synchronization Control
| **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
| ------------ | ---------- | --------------- | ------------- | --------- |
| `PARALLEL` | 34 | Code implemented until the next `BARRIER` will run on all cores | Not Implemented | None |
| `BARRIER` | 35 | Ensure all cores reach this point before continuing, then switch back to single core execution | Not Implemented | None |
| `LOCK` | 36 | Mutual exclusion lock on a resource | Not Implemented | None |
| `UNLOCK` | 37 | Unlock a resource | Not Implemented | None |

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
 - **Operands:** Destination register, first source register, second source register or immediate value (`FLAG_I` determines register/immediate, `FLAG_F` determines integer/float)
 - **Effect:**
    - Add `Src2/Imm` to `Src1`
    - Store result in `Dest`
 - **Usage:**
```
...
ADD Rtmp1, Rtmp1, Rtmp2     ; Rtmp1 += Rtmp2
...
ADD Rtmp1, Rtmp1, #1        ; Rtmp1 += 1
...
ADD Ftmp1, Ftmp1, Ftmp2     ; Ftmp1 += Ftmp2
...
ADD Ftmp1, Ftmp1, #0.43     ; Ftmp1 += 0.43
...
```

### `SUB`
 - **Opcode:** 0xD
 - **Operands:** Destination register, first source register, second source register or immediate value (`FLAG_I` determines register/immediate, `FLAG_F` determines integer/float)
 - **Effect:**
    - Subtract `Src2/Imm` from `Src1`
    - Store result in `Dest`
 - **Usage:**
```
...
SUB Rtmp4, Rtmp1, Rtmp2     ; Rtmp4 = Rtmp1 - Rtmp2
...
SUB Rtmp4, Rtmp1, #8        ; Rtmp4 = Rtmp1 - 8
...
SUB Ftmp4, Ftmp6, Ftmp2     ; Ftmp4 = Ftmp6 - Ftmp2
...
SUB Ftmp1, Ftmp10, #12.9    ; Ftmp1 = Ftmp10 - 12.9
...
```

### `MUL`
 - **Opcode:** 0xE
 - **Operands:** Destination register, first source register, second source register or immediate value (`FLAG_I` determines register/immediate, `FLAG_F` determines integer/float)
 - **Effect:**
    - Multiply `Src1` by `Src2/Imm`
    - Store result in `Dest`
 - **Usage:**
```
...
MULT Rtmp5, Rtmp9, Rtmp2    ; Rtmp5 = Rtmp9 * Rtmp2
...
MULT Rtmp16, Rtmp16, #8     ; Rtmp16 *= 8
...
MULT Ftmp1, Ftmp3, Ftmp12   ; Ftmp1 = Ftmp3 * Ftmp12
...
MULT Ftmp5, Ftmp5, #19.477  ; Ftmp5 *= 19.477
...
```

### `DIV`
 - **Opcode:** 0xF
 - **Operands:** Destination register, first source register, second source register or immediate value (`FLAG_I` determines register/immediate, `FLAG_F` determines integer/float)
 - **Effect:**
    - Divide `Src1` by `Src2/Imm`
    - Store result in `Dest`
 - **Usage:**
```
...
DIV Rtmp1, Rtmp3, Rtmp7     ; Rtmp1 = Rtmp3 / Rtmp7
...
DIV Rtmp5, Rtmp9, #2        ; Rtmp5 = Rtmp9 / 2
...
DIV Ftmp9, Ftmp12, Ftmp2    ; Ftmp9 = Ftmp12 / Ftmp2
...
DIV Ftmp10, Ftmp13, #7.8    ; Ftmp10 = Ftmp13 / 7.8
...
```

### `CMP`
 - **Opcode:** 0x10
 - **Operands:** Two registers (`FLAG_F` determines if they are integer or float)
 - **Effect:**
    - `result = Src1 - Src2`
    - if `result == 0` then
        - `FLAGS = FLAG_ZERO`
    - else if `result < 0` then
        - `FLAGS = FLAG_NEG`
    - else
        - `FLAGS = FLAG_POS`
 - **Usage:**
```
...
CMP Rtmp1, Rzero            ; Compare Rtmp1 with 0
BZ done
...
CMP Ftmp5, Facc             ; Compare Ftmp5 with Facc     
BLT loop
...
```

### `MOV`
 - **Opcode:** 0x11
 - **Operands:** Destination register, source register or immediate value (`FLAG_I` determines register/immediate, `FLAG_F` determines integer/float)
 - **Effect:**
    - Store `Src1/Imm` in `Dest`
 - **Usage:**
```
...
MOV Rtmp1, Racc             ; Rtmp1 = Racc
...
MOV Rtmp4, #18              ; Rtmp4 = 18
...
MOV Ftmp4, Ftmp2            ; Ftmp4 = Ftmp2
...
MOV Ftmp14, #38.9           ; Ftmp14 = 38.9
...
```

### `MOVC`
 - **Opcode:** 0x12
 - **Operands:** Destination register, source register (`FLAG_F` determines if converting float->integer or integer->float)
 - **Effect:**
    - Convert `Src1` to `Dest` type
    - Store converted value in `Dest`
 - **Usage:**
```
...
MOVC Rtmp1, Ftmp1           ; Rtmp1 = (int32_t)Ftmp1
...
MOVC Ftmp2, Rtmp4           ; Ftmp2 = (float)Rtmp4
...
```

### `LD`
 - **Opcode:** 0x13
 - **Operands:** Destination register holds register to load into, source register or immediate value determines place in memory to read from (`FLAG_I` determines if the address is stored in a register or immediate value, `FLAG_F` determines if reading a float or integer)
 - **Effect:**
    - Read `memory[Src1]` or `memory[Imm]`
    - Store the value into `Dest`
 - **Usage:**
```
...
LD Rtmp5, #2                ; Read memory[2] into Rtmp5
...
LD Rtmp5, Rtmp8             ; Read memory[Rtmp8] into Rtmp5
...
LD Ftmp1, #1                ; Read memory[1] into Ftmp1
...
LD Ftmp12, Rtmp1            ; Read memory[Rtmp1] into Ftmp12
...
```

### `ST`
 - **Opcode:** 0x14
 - **Operands:** Destination register holds register to write to memory, source register or immediate value determines place in memory to write to (`FLAG_I` determines if the address is stored in a register or immediate value, `FLAG_F` determines if reading a float or integer)
 - **Effect:**
    - Read the value from `Dest`
    - Store the value in either `memory[Src1]` or `memory[Imm]`
 - **Usage:**
```
...
ST Rtmp1, #32               ; memory[32] = Rtmp1
...
ST Rtmp9, Rtmp8             ; memory[Rtmp8] = Rtmp9
...
ST Ftmp1, #1                ; memory[1] = Ftmp1
...
ST Ftmp9, Rtmp9             ; memory[Rtmp9] = Ftmp9
...
```

### `FPUSH`
 - **Opcode:** 0x15
 - **Operands:** Node register (flags ignored)
 - **Effect:**
    - Add the node into the back frontier
 - **Usage:**
```
...
FPUSH Rnode                 ; Rnode is added to the back of the back frontier 
...
FSWAP
FPOP Rnode
...
```

### `FPOP`
 - **Opcode:** 0x16
 - **Operands:** Node register (flags ignored)
 - **Effect:**
    - Get the front of the current frontier and put it in a register
 - **Usage:**
```
...
FPUSH Rnode
FSWAP
FPOP Rtmp1                  ; Rtmp1 now has the old Rnode in it 
...
```

### `FEMPTY`
 - **Opcode:** 0x17
 - **Operands:** None (flags ignored)
 - **Effect:**
    - Check if the current frontier is empty
    - If it is:
        - `FLAGS` = `~FLAG_ZERO`
    - Otherwise
        - `FLAGS` = `FLAG_ZERO`
 - **Usage:**
```
...
FPUSH Rnode
FSWAP
FEMPTY                      ; Frontier has old Rnode in it
BZ done
FPOP Rnode
FEMPTY                      ; After popping, frontier should be empty now
BZ done
...
```

### `FSWAP`
 - **Opcode:** 0x18
 - **Operands:** None (flags ignored)
 - **Effect:**
    - Swap back and front frontiers
        - Current frontier = back frontier
        - Back frontier = old frontier
 - **Usage:**
```
...
FPUSH Rnode
FSWAP                       ; Frontier with Rnode in it is now current frontier
FPOP Rtmp1
...
```

### `FFILL`
 - **Opcode:** 0x19
 - **Operands:** None (flags ignored)
 - **Effect:**
    - Current frontier is filled with all nodes in graph
 - **Usage:**
```
...
FFILL                       ; All nodes should now be in current frontier
FPOP Rnode
...
```

### `VADD`
 - **Opcode:** 0x1A
 - **Operands:** Destination vector register and two source vector registers (flags determines integer or floating point arithmetic)
 - **Effect:**
    - Add source registers in parallel
    - Store in destination register
 - **Usage:**
```
...
VSET Vr1, #1
VSET Vr2, #2
VADD Vr3, Vr1, Vr2          ; Vr3 is full of 3's now
...
```

### `VSUB`
 - **Opcode:** 0x1B
 - **Operands:** Destination vector register and two source vector registers (flags determines integer or floating point arithmetic)
 - **Effect:**
    - Subtract source registers in parallel
    - Store in destination register
 - **Usage:**
```
...
VSET Vr1, #7
VSET Vr2, #3
VSUB Vr3, Vr1, Vr2          ; Vr3 is full of 4's now
...
```

### `VMUL`
 - **Opcode:** 0x1C
 - **Operands:** Destination vector register and two source vector registers (flags determines integer or floating point arithmetic)
 - **Effect:**
    - Multiply source registers in parallel
    - Store in destination register
 - **Usage:**
```
...
VSET Vr1, #5
VSET Vr2, #5
VMUL Vr3, Vr1, Vr2          ; Vr3 is full of 25's now
...
```

### `VDIV`
 - **Opcode:** 0x1D
 - **Operands:** Destination vector register and two source vector registers (flags determines integer or floating point arithmetic)
 - **Effect:**
    - Divide source registers in parallel
    - Store in destination register
 - **Usage:**
```
...
VSET Vr1, #10
VSET Vr2, #2
VADD Vr3, Vr1, Vr2          ; Vr3 is full of 5's now
...
```

### `VLD`
 - **Opcode:** 0x1E
 - **Operands:** Destination register and immediate or source register (`FLAG_I` determines if immediate or register, `FLAG_F` determines if integer or float)
 - **Effect:**
    - Vector register is filled with data from memory
 - **Usage:**
```
...
VLD Vf3, #16                ; Vf3 is filled with data from memory[16:19]
...
VLD Vr4, Rtmp7              ; Vr4 is filled with data from memory[Rtmp7:Rtmp7+3]
...
```

### `VST`
 - **Opcode:** 0x1F
 - **Operands:** Destination register and immediate or source register (`FLAG_I` determines if immediate or register, `FLAG_F` determines if integer or float)
 - **Effect:**
    - Memory is filled with data from vector register
 - **Usage:**
```
...
VST Vf3, #16                ; memory[16:19] is filled with data from Vf3
...
VST Vr4, Rtmp7              ; memory[Rtmp7:Rtmp7+3] is filled with data from Vr4
...
```

### `VSET`
 - **Opcode:** 0x20
 - **Operands:** Destination register and immediate value (`FLAG_I` determines if immediate or register, `FLAG_F` determines if integer or float)
 - **Effect:**
    - `Dest` is filled with `Src1`/`Imm`
 - **Usage:**
```
...
VSET Vr8, #5                ; Vr8 is filled with 5's
...
VSET Vf5, Ftmp8             ; Vf5 is filled with Ftmp8
...
```

### `VSUM`
 - **Opcode:** 0x21
 - **Operands:** Destination register and vector register (`FLA_F` determines if integer or float)
 - **Effect:**
    - Sum up vector register values
    - Store result in `Dest`
 - **Usage:**
```
...
VSET Vr4, 4
VSUM Rtmp1, Vr4             ; Rtmp1 = Rtmp1 + 16
...
```

### `PARALLEL`
 - **Opcode:** 0x22
 - **Operands:** Not implemented yet 
 - **Effect:**
    - Not implemented yet
 - **Usage:**
```
Not implemented yet
```

### `BARRIER`
 - **Opcode:** 0x23
 - **Operands:** Not implemented yet
 - **Effect:**
    - Not implemented yet
 - **Usage:**
```
Not implemented yet
```

### `LOCK`
 - **Opcode:** 0x24
 - **Operands:** Not implemented yet
 - **Effect:**
    - Not implemented yet
 - **Usage:**
```
Not implemented yet
```

### `UNLOCK`
 - **Opcode:** 0x25
 - **Operands:** Not implemented yet
 - **Effect:** 
    - Not implemented yet
 - **Usage:**
```
Not implemented yet
```