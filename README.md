# GraphX: A Graph Processing Accelerator

**GraphX** is a custom graph processing accelerator designed to efficiently execute graph algorithms such as PageRank, BFS, and triangle counting using a compact, parallel instruction set architecture (ISA).  
The project consists of two major components:
- A **C-based Virtual Machine (VM)** implementation that simulates the ISA and executes single core graph workloads.
- A **Verilog-based FPGA implementation** that realizes the same ISA in hardware for high-performance, multicore graph analytics.

---

## Overview

GraphX is designed to provide a minimal yet expressive instruction set for graph processing, enabling both rapid prototyping in software and scalable execution in hardware.  
The system follows a **SIMT (Single Instruction, Multiple Thread)** execution model with support for barrier synchronization and parallel execution primitives.

### Key Features
- 64-bit fixed-length instruction format for deterministic decoding
- Support for both integer and floating-point computation
- Explicit parallel and synchronization control (e.g., `PARALLEL`, `BARRIER`)
- Register-based architecture with immediate or register-based operands
- VM for quick iteration and testing of algorithms before hardware synthesis
- Modular hardware backend targeting FPGA platforms such as the Basys3 or Zynq boards

---

## Instruction Set Architecture (ISA)

### Instruction Format
Each instruction is 64 bits, divided into the following fields:

| Bits | Field        | Description |
|------|---------------|-------------|
| 63 – 56 | **Opcode** | Operation code specifying the instruction |
| 55 – 48 | **Flags** | Operand mode (e.g., integer/float, immediate/register) |
| 47 – 40 | **Destination Register (rd)** | Target register for the result |
| 39 – 32 | **Source Register 1 (rs1)** | First source operand |
| 31 – 0  | **Immediate / Source Register 2 (rs2)** | Immediate value or second operand |

### Example

| 8 bits | 8 bits | 8 bits | 8 bits | 32 bits |
| ------ | ------ | ------ | ------ | ------- |
| opcode | flags | dest | src1 | src2/imm |

### Operand Modes
The **flags** field determines how operands are interpreted:
- `0x01`: Immediate operation  
- `0x02`: Floating point operation 

### Instruction Set Documentation
See [isa.md](docs/isa.md) for full documentation regarding the instruction set.

---

## Virtual Machine (Software Implementation)

The **VM** is implemented in **C** and emulates the GraphX ISA.  
It allows algorithm development and validation before FPGA synthesis.

### Features
- Executes binary instruction streams or assembly-like scripts
- Provides virtual register and memory management
- Supports timing/instruction count profiling
- Facilitates rapid debugging and ISA design iteration

### Building VM
```bash
make
./bin/graphX {path to executable}
```

### Assembling code
```bash
python helpers/asm.py {path to source} {destination}
```

---

## FPGA Implementation (Hardware Backend)

The **Verilog** implementation realizes the GraphX VM in hardware for parallel graph computation. This version targets FPGA boards and is intended for low-latency, high-throughput workloads.

### Features
 - Pipelined instruction decoder and ALU
 - Configurable number of compute cores
 - Barrier synchronization hardware
 - On-chip BRAM for fast node/edge storage

### Future Work
 - Add cache hierarchy and on-chip memory controller
 - Implement hardware scheduler for multi-core task management
 - Integrate DMA for graph streaming from external memory

---

## Future Extensions
 - Add memory hierarchy documentation (L1 cache, scratchpad, shared memory)
 - Extend instruction set for graph-specific operations (e.g., `RELAX`, `PUSH_EDGE`)
 - Develop compiler frontend for automatic graph algorithm translation
 - Incorporate performance counters and profiling support in FPGA version

---

## Documentation
 - [`isa.md`](docs/isa.md): Complete instruction set specification, including encoding, timing behavior, and usage.
 - [`memory.md`](docs/memory.md): In depth overview of memory hierarchy, including internal and external memory.
 - [`multicore.md`](docs/multicore.md): Describes the interconnect between cores on the hardware implementation.
 - [`synchronization.md`](docs/synchronization.md): Explains resource synchronization and core communication.
 - [`examples/`](docs/examples): Sample programs (PageRank, BFS, counting triangles).

---

## Authors

Developed by **Austin Hamilton**

M.S. in Computer Science - East Tennessee State University

---

## License

MIT License @ 2025 Austin Hamilton

---