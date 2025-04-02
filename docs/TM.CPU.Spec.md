# Specification - TM Virtual CPU

Below is the specification for the TM Virtual CPU, the emulated CPU which powers the TM Virtual
Machine.

## Table Of Contents

1. [Introduction](#introduction)
2. [Glossary](#glossary)
3. [Memory Map](#memory-map)
4. [The Stacks](#the-stacks)
    - [Data Stack](#data-stack)
    - [Call Stack](#call-stack)
5. [Registers](#registers)
    - [General Purpose Registers](#general-purpose-registers)
    - [Special Purpose Registers](#special-purpose-registers)
    - [Flags Register](#flags-register)
    - [Error Code Register](#error-code-register)
6. [Restart Vectors and Interrupts](#restart-vectors-and-interrupts)
7. [Execution Conditions](#execution-conditions)
8. [Instruction Set](#instruction-set)
    - [Timing](#timing)
    - [General Instructions](#general-instructions)
        - [NOP](#nop)
        - [STOP](#stop)
        - [HALT](#halt)
        - [SEC](#sec)
        - [CEC](#cec)
        - [DI](#di)
        - [EI](#ei)
        - [DAA](#daa)
        - [SCF](#scf)
        - [CCF](#ccf)
    - [Data Transfer Instructions](#data-transfer-instructions)
        - [LD](#ld)
        - [LDQ](#ldq)
        - [LDH](#ldh)
        - [ST](#st)
        - [STQ](#stq)
        - [STD](#std)
        - [MV](#mv)
        - [PUSH](#push)
        - [POP](#pop)
    - [Control Transfer Instructions](#control-transfer-instructions)
        - [JMP](#jmp)
        - [JPB](#jpb)
        - [CALL](#call)
        - [RST](#rst)
        - [RET](#ret)
        - [RETI](#reti)
        - [JPS](#jps)
    - [Arithmetic Instructions](#arithmetic-instructions)
        - [INC](#inc)
        - [DEC](#dec)
        - [ADD](#add)
        - [ADC](#adc)
        - [SUB](#sub)
        - [SBC](#sbc)
    - [Bitwise Instructions](#bitwise-instructions)
        - [AND](#and)
        - [OR](#or)
        - [XOR](#xor)
        - [NOT](#not)
        - [CPL](#cpl)
    - [Comparison Instructions](#comparison-instructions)
        - [CMP](#cmp)
    - [Shift and Rotate Instructions](#shift-and-rotate-instructions)
        - [SLA](#sla)
        - [SRA](#sra)
        - [SRL](#srl)
        - [RL](#rl)
        - [RLC](#rlc)
        - [RR](#rr)
        - [RRC](#rrc)
    - [Bit Manipulation](#bit-manipulation)
        - [BIT](#bit)
        - [SET](#set)
        - [RES](#res)
        - [SWAP](#swap)

## Introduction

The TM Virtual CPU is a simple 32-bit CPU which powers the TM Virtual Machine. It is designed to be
simple to implement and understand, while still being powerful enough to run the TM Virtual Machine
efficiently.

The CPU is a stack-based CPU which uses two stacks: the data stack and the call stack. The data
stack is used to store data and intermediate results, while the call stack is used to store return
addresses and other control information.

The CPU has a set of general purpose registers, special purpose registers, and a flags register. The
general purpose registers are used to store data and intermediate results, while the special purpose
registers are used for special purposes such as the program counter and the stack pointers. The flags
register is used to store status information such as the carry flag and the zero flag.

The CPU has a set of restart vectors which are used to handle interrupts and other special conditions.
The CPU also has a set of instructions which can be used to perform various operations such as data
transfer, arithmetic, bitwise operations, comparison, shift and rotate, and bit manipulation.

## Glossary

- **Bit**: The smallest unit of data in the CPU, which can be either 0 or 1.
- **Nibble**: A group of 4 bits (a 4-bit integer, a half-byte, between 0 and 15).
- **Byte**: A group of 8 bits (an 8-bit integer, between 0 and 255).
- **Word**: A group of 16 bits (a 16-bit integer, between 0 and 65535).
- **Long**: A group of 32 bits (a 32-bit integer, between 0 and 4294967295). Also called a "double word".
- **Memory Map**: A map of the memory layout of the CPU, showing the address ranges used for various
  purposes.
- **ROM**: Read-Only Memory, a type of memory which can only be read from, not written to.
- **RAM**: Random Access Memory, a type of memory which can be read from and written to.
- **QRAM**: Quick RAM, a type of memory which is faster to access than regular RAM, requiring a
    16-bit relative address, rather than a full 32-bit address.
- **XRAM**: Executable RAM, a space in RAM intended to be used for executable code, but can be used
    for any purpose.
- **DRAM**: Data RAM, a space in RAM intended to be used for storing non-executable data.
- **Stack**: A data structure which stores data in a last-in, first-out (LIFO) order.
- **Data Stack**: A stack used to store data and intermediate results.
- **Call Stack**: A stack used to store return addresses and other control information.
- **Registers**: Small, fast storage locations in the CPU used to store data and intermediate results.
- **General Purpose Registers**: Registers used to store data and intermediate results.
- **Special Purpose Registers**: Registers used for special purposes such as the program counter and
    the stack pointers.
- **Flags Register**: A register used to store status information such as the carry flag and the zero
    flag.
- **Restart Vectors**: Special memory locations which are used to handle interrupts and other special
    conditions.
- **Interrupts**: Signals sent by external devices to the CPU to request its attention.
- **Execution Conditions**: Conditions which determine whether an instruction is executed or not.
- **Instruction Set**: The set of instructions which the CPU can execute.
- **Instruction**: A single operation which the CPU can perform, represented by a 16-bit opcode.
- **Opcode**: "Operation code" - a number which represents an instruction in machine code.
- **Operand**: The data on which an instruction operates, such as a register, a memory location, or a
    constant value.

## Memory Map

The TM Virtual CPU operates on a 32-bit address space, with the following memory map:

| Start Address | End Address   | Description                                                      |
|---------------|---------------|------------------------------------------------------------------|
| `0x00000000`  | `0x7FFFFFFF`  | Reserved for ROM (Read-Only Memory)                              |
| `0x00000000`  | `0x00000FFF`  | Reserved for program metadata                                    |
| `0x00001000`  | `0x00001FFF`  | Reserved for restart vector handlers.                            |
| `0x00002000`  | `0x00002FFF`  | Reserved for interrupt handlers.                                 |
| `0x00003000`  | `0x3FFFFFFF`  | Reserved for the program code.                                   |
| `0x40000000`  | `0x7FFFFFFF`  | Reserved for non-executable program data.                        |
| `0x80000000`  | `0xFFFFFFFF`  | Reserved for RAM (Random Access Memory)                          |
| `0x80000000`  | `0xDFFFFFFF`  | Reserved for Data RAM (DRAM)                                     |
| `0xE0000000`  | `0xFFFCFFFF`  | Reserved for Executable RAM (XRAM)                               |
| `0xFFFD0000`  | `0xFFFDFFFF`  | Reserved for the data stack                                      |
| `0xFFFE0000`  | `0xFFFEFFFF`  | Reserved for the call stack                                      |
| `0xFFFF0000`  | `0xFFFFFEFF`  | Reserved for Quick RAM (QRAM)                                    |
| `0xFFFFFF00`  | `0xFFFFFFFF`  | Reserved for I/O hardware registers                              |

Any hardware powered by the TM CPU may define additional memory regions for its own use.
The I/O hardware registers are used to communicate with external devices. The exact layout of these
registers is defined by the hardware.

## The Stacks

The TM Virtual CPU uses two stacks: the data stack and the call stack. The data stack is used to
store data and intermediate results, while the call stack is used to store return addresses and other
control information. The stacks grow downwards in memory, with the stack pointer pointing to the top
of the stack. The TM assumes the stacks to be mapped at the following address ranges:

- Data Stack: `0xFFFD0000` to `0xFFFDFFFF`
- Call Stack: `0xFFFE0000` to `0xFFFEFFFF`

### Data Stack

The data stack is used to store data and intermediate results. It is a LIFO (Last-In, First-Out)
stack, meaning that the last item pushed onto the stack is the first item popped off the stack. The
data stack is used by instructions to store temporary values and intermediate results. The data stack
pointer (`DSP`) is a 16-bit relative address which points to the top of the data stack. The only
instructions which may modify the data stack and `DSP` are `PUSH` and `POP`.

### Call Stack

The call stack is used to store return addresses and other control information. As with the data
stack, the call stack is a LIFO stack, with the last item pushed onto the stack being the first item
popped off the stack. The call stack is used to store return addresses when calling subroutines,
restart vectors, and interrupt handlers. The call stack pointer (`CSP`) is a 16-bit relative address
which points to the top of the call stack. The only instructions which may modify the call stack and
`CSP` are `CALL`, `RST`, `RET`, and `RETI`.

## Registers

The TM Virtual CPU has a set of general purpose registers, special purpose registers, and a flags
register. The general purpose registers are used to store data and intermediate results, while the
special purpose registers are used for special purposes such as the program counter and the stack
pointers. The flags register is used to store status information such as the carry flag and the zero
flag.

### General Purpose Registers

The TM Virtual CPU has four general purpose registers, each of which is 32 bits in size:

| Long      | Word      | Byte 1      | Byte 0      | Description                                  |
|-----------|-----------|-------------|-------------|----------------------------------------------|
| `A`       | `AW`      | `AH`        | `AL`        | Accumulator                                  |
| `B`       | `BW`      | `BH`        | `BL`        | General-Purpose Register                     |
| `C`       | `CW`      | `CH`        | `CL`        | General-Purpose Register                     |
| `D`       | `DW`      | `DH`        | `DL`        | General-Purpose Register                     |

The general purpose registers are used to store data and intermediate results.
- `A` is the accumulator register, used for arithmetic and logical operations.
- `B`, `C`, and `D` are general-purpose registers which can be used for any purpose.
- The general purpose registers can be accessed as 32-bit long, 16-bit word, or 8-bit byte.
- `A`, `B`, `C`, and `D` are direct 32-bit registers.
- `AW`, `BW`, `CW`, and `DW` are the lower 16 bits of the corresponding 32-bit register.
- `AH`, `BH`, `CH`, and `DH` are the high 8 bits of the corresponding 16-bit register.
- `AL`, `BL`, `CL`, and `DL` are the low 8 bits of the corresponding 16-bit register.
- Each register is initialized to `0x00` at the start of the program and when the CPU is reset.

### Special Purpose Registers

The TM Virtual CPU has a set of special purpose registers which are used for special purposes such as
the program counter and the stack pointers. The special purpose registers are as follows:

| Register | Size   | Initial Value | Description                                                           |
|----------|--------|---------------|-----------------------------------------------------------------------|
| `PC`     | Long   | `0x00003000`  | Program Counter, points to the next instruction to be executed.       |
| `IA`     | Long   | `0x00000000`  | Instruction Address, address of the current instruction.              |
| `EA`     | Long   | `0x00000000`  | Error Address, address of last access which caused an error.          |
| `MA`     | Long   | `0x00000000`  | Memory Address, address of last memory access.                        |
| `MD`     | Long   | `0x00000000`  | Memory Data, data fetched from last memory access.                    |
| `DSP`    | Word   | `0xFFFC`      | Data Stack Pointer, points to the top of the data stack.              |
| `CSP`    | Word   | `0xFFFC`      | Call Stack Pointer, points to the top of the call stack.              |
| `IE`     | Word   | `0x0000`      | Interrupt Enable, enables or disables interrupts.                     |
| `IF`     | Word   | `0x0000`      | Interrupt Flag, indicates which interrupts are pending.               |
| `F`      | Byte   | `0x00`        | Flags Register, stores status information such as the carry flag.     |
| `IME`    | Byte   | `0x00`        | Interrupt Master Enable, enables or disables interrupt processing.    |
| `EIM`    | Byte   | `0x00`        | Enable Interrupt Master, should `IME` be enabled on next frame.       |
| `EC`     | Byte   | `0x00`        | Error Code Register, stores an error code when an error occurs.       |
| `DA`     | Bit    | `0`           | Destination Address Flag Register, set by certain fetch modes.        |
| `HALT`   | Bit    | `0`           | Halt Flag Register, set when the CPU is halted.                       |
| `STOP`   | Bit    | `0`           | Stop Flag Register, set when the CPU is stopped.                      |

- The `HALT` flag register is set when the CPU is halted.
    - The CPU will not execute any instructions while the `HALT` flag register is set.
    - The `HALT` flag register can be set by the `HALT` instruction.
    - The `HALT` flag register is cleared automatically when an interrupt is requested.
- The `STOP` flag register is set when the CPU is stopped.
    - The CPU will not operate while the `STOP` flag register is set.
    - The `STOP` flag register can be set by the `STOP` instruction, or if an exception occurs.
    - The `STOP` flag register is intended to signal program termination or an unrecoverable error.
    - The `STOP` flag register can only be cleared by resetting the CPU.
    - The `EC` register can be used to determine what went wrong.
    - A `STOP` with the `EC` register set to zero indicates normal program termination.
    - A `RET` or `RETI` instruction while the call stack is empty will set the `STOP` flag register, 
      and set the `EC` register to `0x00`.

### Flags Register

The flags register (`F`) is a 8-bit register which stores status information such as the carry flag
and the zero flag. The flags register is used to store the following flags:

| Bit  | Symbol | Name              | Description                                                           |
|------|--------|-------------------|-----------------------------------------------------------------------|
| 7    | `Z`    | Zero Flag         | Set if the result of the last operation was zero.                     |
| 6    | `N`    | Subtraction Flag  | Set if the last operation was a subtraction.                          |
| 5    | `H`    | Half-Carry Flag   | Set if there was a carry from the lower nibble in the last operation. |
| 4    | `C`    | Carry Flag        | Set if there was a carry from the last operation.                     |

### Error Code Register

The error code register (`EC`) is a 8-bit register which stores an error code when an error occurs.
The error code register is used to store the following error codes:

| Code   | Enumeration                      | Description                                                           |
|--------|----------------------------------|-----------------------------------------------------------------------|
| `0x00` | `TM_EC_OK`                       | No error, operation successful.                                       |
| `0x01` | `TM_EC_INVALID_OPCODE`           | Attempted to execute an invalid instruction.                          |
| `0x02` | `TM_EC_INVALID_ARGUMENT`         | Invalid argument to an instruction.                                   |
| `0x03` | `TM_EC_BUS_READ`                 | The address bus could not be read from.                               |
| `0x04` | `TM_EC_BUS_WRITE`                | The address bus could not be written to.                              |
| `0x05` | `TM_EC_BAD_READ`                 | Attempt to read non-readable memory.                                  |
| `0x06` | `TM_EC_BAD_WRITE`                | Attempt to write to read-only memory.                                 |
| `0x07` | `TM_EC_BAD_EXECUTE`              | Attempt to execute non-executable memory.                             |
| `0x08` | `TM_EC_DATA_STACK_OVERFLOW`      | Data stack overflow.                                                  |
| `0x09` | `TM_EC_DATA_STACK_UNDERFLOW`     | Data stack underflow.                                                 |
| `0x0A` | `TM_EC_CALL_STACK_OVERFLOW`      | Call stack overflow.                                                  |
| `0x0B` | `TM_EC_HARDWARE`                 | Hardware error; an external hardware component failed to clock.       |

The TM CPU may set the `EC` register to one of the above error codes when it throws an exception,
in which case the `T` flag will also be set to indicate that the CPU is stopped. The `EC` register
can be used to determine what went wrong, and to take appropriate action. The `EC` register is
initialized to `0x00` at the start of the program and when the CPU is reset.

Certain exceptions may also set the `EA` register to the address of the instruction which caused the
exception, or to the address of the memory access which caused the exception. If a `RET` or `RETI`
instruction is executed while the call stack is empty, the CPU will interpret that as a normal
program exit, setting the `T` flag and the `EC` register to `0x00`. 

Further error codes may be defined by the hardware powered by, and software running on, the TM CPU.
A custom error code may be set by the `SEC` instruction.

## Restart Vectors and Interrupts

The TM Virtual CPU has a set of restart vectors which can be quickly called by the `RST` instruction.
These restart vectors are used to handle certain special conditions defined by the hardware and
software. The restart vectors are located in the memory region `0x00001000` to `0x00001FFF`. Each
restart vector has `0x100` bytes reserved for them.

The CPU also supports interrupts, which are signals sent by external devices to request the CPU's
attention. The CPU can be configured to handle different types of interrupts, and can enable or
disable interrupts as needed. The CPU has an interrupt enable register (`IE`) which can be used to
enable or disable interrupts, and an interrupt flag register (`IF`) which indicates which interrupts
have been requested and are pending. The CPU also has an interrupt master enable register (`IME`)
which can be used to enable or disable interrupt processing altogether. The interrupt handler
subroutines are located in the memory region `0x00002000` to `0x00002FFF`. As with the restart
vectors, each interrupt handler has `0x100` bytes reserved for it.

The CPU has a special register called the `EIM` register which is used to enable the interrupt master
(`IME`) on the next frame. This register is used to prevent interrupts from being processed while the
CPU is in the middle of executing an instruction. The `EIM` register is set by the `EI` instruction,
and is cleared the instant the `IME` register is set.

## Execution Conditions

Certain instructions in the TM CPU's instruction set - particularly the control transfer instructions
- can be conditionally executed based on the state of the flags register. The **execution conditions**
are listed as follows:

| Condition | Description                                                                          |
|-----------|--------------------------------------------------------------------------------------|
| `NC`      | No Condition: The instruction is always executed.                                    |
| `ZS`      | Zero Set: The instruction is executed if the zero flag is set.                       |
| `ZC`      | Zero Clear: The instruction is executed if the zero flag is clear.                   |
| `CS`      | Carry Set: The instruction is executed if the carry flag is set.                     |
| `CC`      | Carry Clear: The instruction is executed if the carry flag is clear.                 |

## Instruction Set

The TM Virtual CPU has a set of instructions which can be used to perform various operations such as
data transfer, arithmetic, bitwise operations, comparison, shift and rotate, and bit manipulation.
The instructions are represented by 16-bit "operation codes" (opcodes) which are laid out as follows:

- `OOXY`
    - `OO`: The opcode proper.
    - `X`: The first parameter.
    - `Y`: The second parameter.

The instruction parameters in the low byte of the opcode may be used to specify destination (`X`)
and source (`Y`) registers. Certain instructions may also use the parameters to specify execution
conditions and, in the case of the `SEC` instruction, an immediate value.

In addition to the parameters in the low byte of the opcode, some instructions may have additional
operands which are read from the memory immediately following the opcode. The operands are always
read in little-endian order (least significant byte first, `0x12345678` is read as `0x78`, `0x56`, 
`0x34`, `0x12`, for example).

Certain instructions can affect the CPU's flags register. How these instructions affect the flags
register is detailed in the description of each instruction, and is listed as follows:

- `ZNHC`
    - `Z`: Zero Flag
    - `N`: Subtraction Flag
    - `H`: Half-Carry Flag
    - `C`: Carry Flag
- The symbols used to indicate how the flags are affected are as follows:
    - `+`: The flag is set.
    - `-`: The flag is cleared.
    - `0`: The flag is unchanged.
    - `?`: The flag is affected, depending on the result of the operation.
    - See the notes below each instruction table for more information.

### Timing

The TM CPU operates at a clock speed dictated by the hardware it is running on. The CPU elapses one
clock cycle every time a byte is read from or written to memory, and every time the program counter
or stack pointers are manipulated.

When the CPU is halted, one clock cycle elapses per frame. When the CPU is stopped, no clock cycles
elapse. Hardware components powered by the TM CPU may also add additional clock cycles to the CPU's
execution time.

### General Instructions

| Opcode          | Mnemonic    | Flags    | Description                                                                  |
|-----------------|-------------|----------|------------------------------------------------------------------------------|
| `0x0000`        | `NOP`       | `0000`   | No Operation: Do nothing.                                                    |
| `0x0100`        | `STOP`      | `0000`   | Stop CPU: Stop the CPU.                                                      |
| `0x0200`        | `HALT`      | `0000`   | Halt CPU: Halt the CPU.                                                      |
| `0x03XY`        | `SEC XY`    | `0000`   | Set Error Code: Set the error code to the immediate value `XY`.              |
| `0x0400`        | `CEC`       | `0000`   | Clear Error Code: Clear the error code.                                      |
| `0x0500`        | `DI`        | `0000`   | Disable Interrupts: Disable interrupts.                                      |
| `0x0600`        | `EI`        | `0000`   | Enable Interrupts: Enable interrupts.                                        |
| `0x0700`        | `DAA`       | `?0-?`   | Decimal Adjust Accumulator: Adjust the accumulator for BCD arithmetic.       |
| `0x0800`        | `SCF`       | `0--+`   | Set Carry Flag: Set the carry flag.                                          |
| `0x0900`        | `CCF`       | `0--?`   | Compliment Carry Flag: Compliment the carry flag.                            |

#### Flags

- `DAA`:
    - `Z`: Set if the result is zero.
    - `C`: Set if the result caused a carry.
- `SWAP`:
    - `Z`: Set if the result is zero.
- `CCF`:
    - `C`: Set if cleared, cleared if set.

### Data Transfer Instructions

The instructions below do not affect the flags register.

| Opcode          | Mnemonic        | Description                                                                                                   |
|-----------------|-----------------|---------------------------------------------------------------------------------------------------------------|
| `0x10X0`        | `LD X, I`       | Load Immediate: Load the immediate value `I` into register `X`.                                               |
| `0x11X0`        | `LD X, [A32]`   | Load from Memory: Load the value at address `A32` into register `X`.                                          |
| `0x12XY`        | `LD X, [Y]`     | Load from Register Pointer: Load the value at address in register `Y` into `X`.                               |
| `0x13X0`        | `LDQ X, [A16]`  | Load from QRAM: Load the value at address `A16` into register `X`.                                            |
| `0x14XY`        | `LDQ X, [Y]`    | Load from QRAM (Register Pointer): Load the value at relative address in register `Y` into `X`.               |
| `0x15X0`        | `LDH X, [A8]`   | Load from Hardware: Load the value at address `A8` into register `X`.                                         |
| `0x16XY`        | `LDH X, [Y]`    | Load from Hardware (Register Pointer): Load the value at relative address in register `Y` into `X`.           |
| `0x170Y`        | `ST [A32], Y`   | Store to Memory: Store the value in register `Y` at address `A32`.                                            |
| `0x18XY`        | `ST [X], Y`     | Store to Register Pointer: Store the value in register `Y` at address in register `X`.                        |
| `0x19X0`        | `STQ [A16], X`  | Store to QRAM: Store the value in register `X` at address `A16`.                                              |
| `0x1A0Y`        | `STQ [Y], X`    | Store to QRAM (Register Pointer): Store the value in register `X` at relative address in register `Y`.        |
| `0x1BX0`        | `STH [A8], X`   | Store to Hardware: Store the value in register `X` at address `A8`.                                           |
| `0x1CXY`        | `STH [Y], X`    | Store to Hardware (Register Pointer): Store the value in register `X` at relative address in register `Y`.    |
| `0x1DXY`        | `MV X, Y`       | Move: Move the value in register `Y` to register `X`.                                                         |
| `0x1E0Y`        | `PUSH Y`        | Push: Push the value in register `Y` onto the data stack.                                                     |
| `0x1FX0`        | `POP X`         | Pop: Pop the value from the data stack into register `X`.                                                     |

### Control Transfer Instructions

The instructions below do not affect the flags register. The first parameter `X` here is used to
specify the execution condition.

| Opcode          | Mnemonic      | Description                                                                                           |
|-----------------|---------------|-------------------------------------------------------------------------------------------------------|
| `0x20X0`        | `JMP X, A32`  | Jump: Jump to address `A32` if the execution condition `X` is met.                                    |
| `0x21XY`        | `JMP X, Y`    | Jump to Register: Jump to the address in register `Y` if the execution condition `X` is met.          |
| `0x22X0`        | `JPB X, S16`  | Jumb By Offset: Jump by the signed offset `S16` if the execution condition `X` is met.                |
| `0x23X0`        | `CALL X, A32` | Call: Call the subroutine at address `A32` if the execution condition `X` is met.                     |
| `0x240Y`        | `RST Y`       | Restart: Call the restart vector `Y`.                                                                 |
| `0x25X0`        | `RET X`       | Return: Return from a subroutine if the execution condition `X` is met.                               |
| `0x2600`        | `RETI`        | Return from Interrupt: Return from an interrupt handler.                                              |
| `0x2700`        | `JPS`         | Jump to Start: Jump to the start of the program.                                                      |

### Arithmetic Instructions

| Opcode          | Mnemonic     | Flags    | Description                                                                                                           |
|-----------------|--------------|----------|-----------------------------------------------------------------------------------------------------------------------|
| `0x30X0`        | `INC X`      | `?-?0`   | Increment: Increment register `X`.                                                                                    |
| `0x310Y`        | `INC [Y]`    | `?-?0`   | Increment Memory: Increment the value at address in register `Y`.                                                     |
| `0x32X0`        | `DEC X`      | `?+?0`   | Decrement: Decrement register `X`.                                                                                    |
| `0x330Y`        | `DEC [Y]`    | `?+?0`   | Decrement Memory: Decrement the value at address in register `Y`.                                                     |
| `0x34X0`        | `ADD X, I`   | `?-??`   | Add Immediate: Add the immediate value `I` to accumulator register `X`.                                               |
| `0x35XY`        | `ADD X, Y`   | `?-??`   | Add Register: Add register `Y` to accumulator register `X`.                                                           |
| `0x36XY`        | `ADD X, [Y]` | `?-??`   | Add Memory: Add the value at address in register `Y` to accumulator register `X`.                                     |
| `0x37X0`        | `ADC X, I`   | `?-??`   | Add with Carry Immediate: Add the immediate value `I` to accumulator register `X` with carry.                         |
| `0x38XY`        | `ADC X, Y`   | `?-??`   | Add with Carry Register: Add register `Y` to accumulator register `X` with carry.                                     |
| `0x39XY`        | `ADC X, [Y]` | `?-??`   | Add with Carry Memory: Add the value at address in register `Y` to accumulator register `X`.                          |
| `0x3AX0`        | `SUB X, I`   | `?+??`   | Subtract Immediate: Subtract the immediate value `I` from accumulator register `X`.                                   |
| `0x3BXY`        | `SUB X, Y`   | `?+??`   | Subtract Register: Subtract register `Y` from accumulator register `X`.                                               |
| `0x3CXY`        | `SUB X, [Y]` | `?+??`   | Subtract Memory: Subtract the value at address in register `Y` from accumulator register `X`.                         |
| `0x3DX0`        | `SBC X, I`   | `?+??`   | Subtract with Carry Immediate: Subtract the immediate value `I` from accumulator register `X` with carry.             |
| `0x3EXY`        | `SBC X, Y`   | `?+??`   | Subtract with Carry Register: Subtract register `Y` from accumulator register `X` with carry.                         |
| `0x3FXY`        | `SBC X, [Y]` | `?+??`   | Subtract with Carry Memory: Subtract the value at address in register `Y` from accumulator register `X` with carry.   |

#### Flags

- `INC`, `DEC`:
    - `Z`: Set if the result is zero.
    - `H`: For byte operations, set if there was a carry from the lower nibble.
- `ADD`, `ADC`:
    - `Z`: Set if the result is zero.
    - `H`: Set if there was a carry from the lower nibble(s).
    - `C`: Set if there was a carry from the operation.
- `SUB`, `SBC`:
    - `Z`: Set if the result is zero.
    - `H`: Set if there was a carry from the lower nibble(s).
    - `C`: Set if there was a carry from the operation.

### Bitwise Instructions

| Opcode          | Mnemonic     | Flags    | Description                                                                                           |
|-----------------|--------------|----------|-------------------------------------------------------------------------------------------------------|
| `0x40X0`        | `AND X, I`   | `?-+-`   | Bitwise AND Immediate: Bitwise AND the immediate value `I` with accumulator register `X`.             |
| `0x41XY`        | `AND X, Y`   | `?-+-`   | Bitwise AND Register: Bitwise AND register `Y` with accumulator register `X`.                         |
| `0x42XY`        | `AND X, [Y]` | `?-+-`   | Bitwise AND Memory: Bitwise AND the value at address in register `Y` with accumulator register `X`.   |
| `0x43X0`        | `OR X, I`    | `?---`   | Bitwise OR Immediate: Bitwise OR the immediate value `I` with accumulator register `X`.               |
| `0x44XY`        | `OR X, Y`    | `?---`   | Bitwise OR Register: Bitwise OR register `Y` with accumulator register `X`.                           |
| `0x45XY`        | `OR X, [Y]`  | `?---`   | Bitwise OR Memory: Bitwise OR the value at address in register `Y` with accumulator register `X`.     |
| `0x46X0`        | `XOR X, I`   | `?---`   | Bitwise XOR Immediate: Bitwise XOR the immediate value `I` with accumulator register `X`.             |
| `0x47XY`        | `XOR X, Y`   | `?---`   | Bitwise XOR Register: Bitwise XOR register `Y` with accumulator register `X`.                         |
| `0x48XY`        | `XOR X, [Y]` | `?---`   | Bitwise XOR Memory: Bitwise XOR the value at address in register `Y` with accumulator register `X`.   |
| `0x49X0`        | `NOT X`      | `?++0`   | Bitwise NOT: Compliment the bits in register `X`.                                                     |
| `0x4A0Y`        | `NOT [Y]`    | `?++0`   | Bitwise NOT Memory: Compliment the bits of value at address in register `Y`.                          |
|                 | `CPL X`      | `?++0`   | Compliment: Alias of `NOT X`.                                                                         |
|                 | `CPL [Y]`    | `?++0`   | Compliment Memory: Alias of `NOT [Y]`.                                                                |

#### Flags

- `AND`, `OR`, `XOR`, `NOT`:
    - `Z`: Set if the result is zero.

### Comparison Instructions

| Opcode          | Mnemonic     | Flags    | Description                                                                                           |
|-----------------|--------------|----------|-------------------------------------------------------------------------------------------------------|
| `0x50X0`        | `CMP X, I`   | `?+??`   | Compare Immediate: Compare the immediate value `I` with accumulator register `X`.                     |
| `0x51XY`        | `CMP X, Y`   | `?+??`   | Compare Register: Compare register `Y` with accumulator register `X`.                                 |
| `0x52XY`        | `CMP X, [Y]` | `?+??`   | Compare Memory: Compare the value at address in register `Y` with accumulator register `X`.           |

#### Flags

- `CMP`:
    - `Z`: Set if the result is zero.
    - `H`: Set if there was a carry from the lower nibble.
    - `C`: Set if there was a carry from the operation.

### Shift and Rotate Instructions

| Opcode          | Mnemonic     | Flags    | Description                                                                                                               |
|-----------------|--------------|----------|---------------------------------------------------------------------------------------------------------------------------|
| `0x60X0`        | `SLA X`      | `?--?`   | Shift Left Arithmetic: Shift the bits in accumulator register `X` left, shifting in zeros.                                |
| `0x610Y`        | `SLA [Y]`    | `?--?`   | Shift Left Arithmetic Memory: Shift the bits of byte at address in register `Y` left, shifting in zeros.                  |
| `0x62X0`        | `SRA X`      | `?--?`   | Shift Right Arithmetic: Shift the bits in accumulator register `X` right, leaving the old bit 7 unchanged.                |
| `0x630Y`        | `SRA [Y]`    | `?--?`   | Shift Right Arithmetic Memory: Shift the bits of byte at address in register `Y` right, leaving the old bit 7 unchanged.  |
| `0x64X0`        | `SRL X`      | `?--?`   | Shift Right Logical: Shift the bits in accumulator register `X` right, shifting in zeros.                                 |
| `0x650Y`        | `SRL [Y]`    | `?--?`   | Shift Right Logical Memory: Shift the bits of byte at address in register `Y` right, shifting in zeros.                   |
| `0x66X0`        | `RL X`       | `?--?`   | Rotate Left Through Carry: Rotate the bits in accumulator register `X` left through the carry flag.                       |
| `0x670Y`        | `RL [Y]`     | `?--?`   | Rotate Left Through Carry Memory: Rotate the bits of byte at address in register `Y` left through the carry flag.         |
| `0x68X0`        | `RLC X`      | `?--?`   | Rotate Left: Rotate the bits in accumulator register `X` left, shifting in the carry flag.                                |
| `0x690Y`        | `RLC [Y]`    | `?--?`   | Rotate Left Memory: Rotate the bits of byte at address in register `Y` left, shifting in the carry flag.                  |
| `0x6AX0`        | `RR X`       | `?--?`   | Rotate Right Through Carry: Rotate the bits in accumulator register `X` right through the carry flag.                     |
| `0x6B0Y`        | `RR [Y]`     | `?--?`   | Rotate Right Through Carry Memory: Rotate the bits of byte at address in register `Y` right through the carry flag.       |
| `0x6CX0`        | `RRC X`      | `?--?`   | Rotate Right: Rotate the bits in accumulator register `X` right, shifting in the carry flag.                              |
| `0x6D0Y`        | `RRC [Y]`    | `?--?`   | Rotate Right Memory: Rotate the bits of byte at address in register `Y` right, shifting in the carry flag.                |

#### Flags

- All shift and rotate instructions:
    - `Z`: Set if the result is zero.
    - `C`: Set if the result has a carry.

### Bit Manipulation Instructions

| Opcode          | Mnemonic     | Flags    | Description                                                                                           |
|-----------------|--------------|----------|-------------------------------------------------------------------------------------------------------|
| `0x700Y`        | `BIT I, Y`   | `?-+0`   | Bit Test: Test bit `I` of register `Y`. `I` is an immedate value.                                     |
| `0x710Y`        | `BIT I, [Y]` | `?-+0`   | Bit Test Memory: Test bit `I` of the value at address in register `Y`. `I` is an immedate value.      |
| `0x72X0`        | `SET I, X`   | `0000`   | Bit Set: Set bit `I` of register `X`. `I` is an immedate value.                                       |
| `0x730Y`        | `SET I, [Y]` | `0000`   | Bit Set Memory: Set bit `I` of the value at address in register `Y`. `I` is an immedate value.        |
| `0x74X0`        | `RES I, X`   | `0000`   | Bit Reset: Reset bit `I` of register `X`. `I` is an immedate value.                                   |
| `0x750Y`        | `RES I, [Y]` | `0000`   | Bit Reset Memory: Reset bit `I` of the value at address in register `Y`. `I` is an immedate value.    |
| `0x76X0`        | `SWAP X`     | `?+0-`   | Swap: Swap the high and low halves of register `X`.                                                   |
| `0x770Y`        | `SWAP [Y]`   | `?+0-`   | Swap Memory: Swap the high and low nibbles of the byte at address in register `Y`.                    |

#### Flags

- `BIT`:
    - `Z`: Set if the bit is zero.
