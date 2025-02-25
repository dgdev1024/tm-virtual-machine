# Specification - TM Program File

Below is the specification for the TM program file format. A TM program file is a binary file which
contains the executable machine code for a TM program.

## File Format

A TM program file is a binary file with the following format:

| Start Address | Size        | Description                                                        |
|---------------|-------------|--------------------------------------------------------------------|
| `0x00000000`  | 4 Bytes     | Magic Number (ASCII "TM08", Long `0x544D3038`)                     |
| `0x00000004`  | 2 Bytes     | Major Version Number (Word)                                        |
| `0x00000006`  | 2 Bytes     | Minor Version Number (Word)                                        |
| `0x00000008`  | 4 Bytes     | Patch Version Number (Long)                                        |
| `0x0000000C`  | 4 Bytes     | Expected ROM Size (Long)                                           |
| `0x00000010`  | 64 Bytes    | Program Name (ASCII, Null-Terminated)                              |
| `0x00000050`  | 64 Bytes    | Author Name (ASCII, Null-Terminated)                               |
| `0x00000090`  | 256 Bytes   | Description (ASCII, Null-Terminated)                               |
| `0x00000190`  | 3696 Bytes  | Additional Metadata, Defined By Hardware                           |
| `0x00001000`  | 4096 Bytes  | Restart Vector Handlers (16 Handlers, 256 Bytes Each)              |
| `0x00002000`  | 4096 Bytes  | Interrupt Vector Handlers (16 Handlers, 256 Bytes Each)            |
| `0x00003000`  | Variable    | Program Code (16-bit instructions, variable-length operands)       |

### Magic Number

The magic number is a 4-byte ASCII string, "TM08". This is used to identify the file as a TM program file.

### Version Numbers

The version numbers are used to identify the version of the TM program file format. The version numbers are
used to determine if the file is compatible with the current version of the TM virtual machine.

### Expected ROM Size

The expected ROM size is the size of the ROM that the program is expected to run on. This is used to ensure
that the program is not run on a ROM that is too small to contain the program, or has been tampered with.

### Program Name, Author and Description

These fields are used to store information about the program, such as the name of the program, the author of
the program, and a description of the program.

### Additional Metadata

This field is used to store additional metadata that is defined by the hardware that the program is intended
to run on. This can be used to store information such as hardware configuration, memory maps, etc.

### Restart Vector Handlers

The restart vector handlers are used to store the handlers for the restart vectors. There are 16 restart vectors,
each with a 256-byte handler. The restart vectors are called with the `RST` instruction.

### Interrupt Handlers

The interrupt handlers are used to store the handlers for the interrupt vectors. There are 16 interrupt vectors,
each with a 256-byte handler. The interrupt vectors are called when their corresponding interrupt is enabled,
provided that interrupt and the interrupt master are enabled.

### Program Code

The program code is the actual machine code for the program. The program code is a series of 16-bit instructions,
with variable-length operands. The program code is loaded into memory and executed by the TM virtual machine.
