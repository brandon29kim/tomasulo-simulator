# tomasulo-simulator
A cycle-accurate RISC-V simulator implemented using Tomasulo's algorithm for out-of-order instruction execution. 

## Overview

This simulator models a RISC-V processor that dynamically schedules instructions using reservation stations, register renaming, and a common data bus. It also handles data, structural, and control hazards. 

## Features

- **Out-of-Order Execution**: Dynamic instruction scheduling using Tomasulo's algorithm
- **Pipeline Stages**: 4-stage pipeline (Fetch, Dispatch, Execute, Broadcast)
- **Multiple Functional Units**: Configurable ALU, FP add/multiply/divide, and memory units
- **Hazard Handling**: Automatic resolution of RAW hazards; detection of structural and control hazards
- **Register Renaming**: Eliminates WAW and WAR hazards through reservation station mapping
- **Cycle-Accurate Timing**: Precise simulation of instruction latencies and resource conflicts

## Supported Instructions

Implements a subset of RISC-V ISA including:
- Integer arithmetic/logic operations (add, sub, and, or, xor, slt, etc.)
- Single-precision floating-point operations (add.s, sub.s, mult.s, div.s)
- Memory operations (lw, sw, l.s, s.s)
- Control flow (beqz, bnez, j, jal, jr, jalr)

## Usage
Make the project and the assembler. 
```zsh
make
make asm
```
Assemble a program.
```zsh
./asm programs/simple.s programs/simple.bin
```
Run the simulator on the binary file. 
``` zsh
./sim -o machine/machine.cfg -b programs/simple.bin > programs/simple.out
```



