# Specification - TM Object File Format

Below is the specification for the TM object file format. A TM object file is a binary file emitted
by the TM assembler, which contains the assembled machine code and data for a TM program, as well as tables
containing any address labels which were defined in the assembly source, and any external labels
which were referenced in the assembly source.

The TM linker reads TM object files and combines - "links" - them into an executable TM program file.

## File Format

A TM object file is a binary file with the following format:

| Start Address | Size        | Description                                                        |
|---------------|-------------|--------------------------------------------------------------------|
| `0x00000000`  | 4 Bytes     | Magic Number (ASCII "TMOB", Long `0x544D4F42`)                     |
| `0x00000004`  | 2 Bytes     | Major Version Number (Word)                                        |
| `0x00000006`  | 2 Bytes     | Minor Version Number (Word)                                        |
| `0x00000008`  | 4 Bytes     | Patch Version Number (Long)                                        |
| `0x0000000C`  | 4 Bytes     | Resolved Address Label Count (Long)                                |
| `0x00000010`  | 4 Bytes     | External Label Count (Long)                                        |
| `0x00000014`  | 4 Bytes     | Code Section Size (Long)                                           |
| `0x00000018`  | Variable    | Resolved Address Labels Array                                      |
|               | Variable    | Unresolved External Labels Array                                   |
|               | Variable    | Code Section                                                       |

### Magic Number

The magic number is a 4-byte ASCII string, "TMOB". This is used to identify the file as a TM object file.

### Version Numbers

The version numbers are used to identify the version of the TM object file format. The version numbers are
used to determine if the file is compatible with the current version of the TM virtual machine, and
its assembler and linker.

### Resolved Address Label Count

The resolved address label count is the number of address labels which were defined in the assembly source
and resolved by the assembler. This is used to determine the size of the resolved address labels array.

### External Label Count

The external label count is the number of external labels which were referenced in the assembly source
and not resolved by the assembler. This is used to determine the size of the unresolved external labels array.

### Code Section Size

The code section size is the size of the code section in bytes. This is used to determine the size of the code section.

### Resolved Address Labels Array

The resolved address labels array is an array of resolved address labels. Each resolved address label is a
structure with the following format:

| Offset        | Size        | Description                                                        |
|---------------|-------------|--------------------------------------------------------------------|
| `0x00000000`  | 4 Bytes     | Address (Long)                                                     |
| `0x00000004`  | 64 Bytes    | Label Name (ASCII, Null-Terminated)                                |

### Unresolved External Labels Array

The unresolved external labels array is an array of unresolved external labels. Each unresolved external label is a
structure with the following format:

| Offset        | Size        | Description                                                        |
|---------------|-------------|--------------------------------------------------------------------|
| `0x00000000`  | 64 Bytes    | Label Name (ASCII, Null-Terminated)                                |

### Code Section

The code section is the assembled machine code and data for the TM program. The code section contains the
following:

- Machine code for the program, in the form of 16-bit instructions with variable-length operands.
- Data for the program, such as strings, numbers, etc.
- Binary files, such as images, audio, etc., included with the **".incbin"** directive.

The code section is the largest section of the TM object file, and contains the actual executable code and data
for the TM program.

## Location Counter

The location counter is a special symbol which is used by the assembler to keep track of the current
address in the code section. The location counter is incremented by the size of each instruction or
data element as it is assembled, and every binary file as it is included. The location counter is used
to dictate where each instruction, data element, or binary file is placed in the final ROM image.

The location counter can be manipulated with the **".org"** directive, which sets the location counter to
a specific address, and the **".align"** directive, which aligns the location counter to a specific boundary.

The location counter also determines whether an operation is performed in ROM or RAM. If the location
counter is less than `0x80000000`, the operation is performed in ROM. If the location counter is greater
than or equal to `0x80000000`, the operation is performed in RAM.

The following types of elements affect the location counter:

- Data Elements
    - Can be placed using the **".byte"**, **".word"**, **".long"**, and **".ascii"** directives.
    - Can only be placed in ROM.
    - Are placed at the current location counter address.
- Instructions
    - Can only be placed in the *Program Code* section of ROM.
    - Are placed at the current location counter address.
- Binary Files
    - Can be placed using the **".incbin"** directive.
    - Can only be placed in ROM.
    - Are placed at the current location counter address.
- Data Reservation/Spacing:
    - Can be reserved using the **".byte"**, **".word"**, and **".long"** directives while the location counter is in RAM.
    - Can be spaced using the **".space"** directive.
    - Are placed at the current location counter address.
