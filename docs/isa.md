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
 | `HALT` |
 | `BZ` |
 | `BNZ` |
 | `BLT` |
 | `BGE` |
 | `JMP` |


 ### Graph Access Instructions
 | **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
 | ------------ | ---------- | --------------- | ------------- | --------- |

 ### Arithmetic and Logic
 | **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
 | ------------ | ---------- | --------------- | ------------- | --------- |

 ### Memory Access
 | **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
 | ------------ | ---------- | --------------- | ------------- | --------- |

 ### Frontier Control
 | **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
 | ------------ | ---------- | --------------- | ------------- | --------- |

 ### Multicore/Synchronization Control
 | **Mnemonic** | **Opcode** | **Description** | **Operation** | **Flags** |
 | ------------ | ---------- | --------------- | ------------- | --------- |