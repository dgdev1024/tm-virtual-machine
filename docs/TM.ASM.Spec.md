# Specification - TMM Assembly Language

Below is the specification for the TMM assembly language. The TMM assembly language is a human-readable
representation of the machine code for a program to be run on the TM virtual machine.

## Syntax

The TMM assembly language is a line-based language. Each line in a TMM assembly file is a statement.
A statement can be a comment, a label, a directive, or an instruction. A statement can also be empty.

### Comments

Comments in the TMM assembly language are lines which start with a semicolon (`;`). Comments are ignored
by the assembler.

Example:

```asm
; This is a comment
```

### Labels

An address **label** is a name which is associated with a memory address. Labels are used to mark locations
in the program code which can be referenced by other parts of the program. Labels are defined by placing
the label name at the start of a line, followed by a colon (`:`). Labels must be unique within the program,
and must not conflict with any reserved keywords. Labels are case-insensitive, must start with either
a letter or an underscore (`_`), and can contain letters, numbers, and underscores.

Example:

```asm
Start:
```

### Directives

A **directive** is a command which is used to control the behavior of the assembler. Directives are used
to define data, set options, include source and binary files, and define macros. Directives are defined
by placing a period (`.`) at the start of a line, followed by the directive name and any arguments
the directive requires. Directives are case-insensitive.

Example:

```asm
.org     0x12345678                     ; Set the location counter to a specific address.
.byte    0x00, 0x01, 0x02, 0x03         ; Define one or more bytes in ROM, or byte spacing in RAM.
.word    0x1234, 0x5678, 0x9ABC         ; Define one or more words in ROM, or word spacing in RAM.
.long    0x12345678, 0x9ABCDEF0         ; Define one or more longs in ROM, or long spacing in RAM.
.ascii   "Hello, World!", "Goodbye!"    ; Define one or more ASCII strings in ROM.
.asciiz  "Hello, World!", "Goodbye!"    ; Define one or more null-terminated ASCII strings in ROM.
.include "./path/to/file.asm"           ; Include another TMM assembly file.
.incbin  "./path/to/file.bin"           ; Include a binary file.
.macro   name ...                       ; Define a macro.
.endm                                   ; End a macro definition.
.shift   count                          ; Shift the parameter list of a macro.
.define  name value                     ; Define a variable.
.undef   name                           ; Undefine a variable.
.if      condition                      ; Begin an if block.
.elif    condition                      ; Begin an else if block.
.else                                   ; Begin an else block.
.endif                                  ; End an if block.
.repeat  count                          ; Begin a repeat block.
.endr                                   ; End a repeat block.
.while   condition                      ; Begin a while block.
.endw                                   ; End a while block.
.break                                  ; Break out of a loop.
.continue                               ; Continue to the next iteration of a loop.
```

### Instructions

An **instruction** is a command which is used to perform an operation on the TM virtual machine. Instructions
are defined by the operation code (opcode) and any operands the instruction requires. Instructions are case-insensitive.
The TM's instruction set can be found in the [TM CPU Specification](TM.CPU.Spec.md).

Example:

```asm
    ld aw, 0x1234                        ; Load immediate value 0x1234 into register AW.
    st [0x80005678], aw                  ; Store the value in register AW to memory address 0x80005678.
    mv bw, aw                            ; Move the value in register AW to register BW.
    add aw, 0x1234                       ; Add immediate value 0x1234 to register AW.
    mv cw, aw                            ; Move the value in register AW to register CW.
    sub cw, 0x1234                       ; Subtract immediate value 0x1234 from register CW.
    stop                                 ; Stop the program.
```

### Operands

An **operand** is a value which is used by an instruction to perform an operation. Operands can be immediate
values, memory addresses, register names, or labels. Operands are separated from the instruction by a comma (`,`).
Operands can be in decimal, hexadecimal, or binary format. Operands can also be labels, which are resolved
by the assembler to memory addresses, or variables defined with the `.define` directive.

### Expressions

An **expression** is a combination of operands, operators, and parentheses which is used to calculate a value.
Expressions can be used in place of immediate values, memory addresses, or labels. Expressions are evaluated
by the assembler to a single value. Expressions can contain the following operators, listed in order
of operator precedence:

1. Primary Expressions
    - Parentheses (`(`, `)`)
    - Immediate Values
    - Memory Addresses
    - Labels
    - Variables
2. Unary Operators
    - Unary Minus (`-`)
    - Unary Plus (`+`)
    - Logical NOT (`!`)
    - Bitwise NOT (`~`)
3. Multiplicative Operators
    - Multiplication (`*`)
    - Exponentiation (`**`)
    - Division (`/`)
    - Modulus (`%`)
4. Additive Operators
    - Addition (`+`)
    - Subtraction (`-`)
5. Shift Operators
    - Left Shift (`<<`)
    - Right Shift (`>>`)
6. Relational Operators
    - Less Than (`<`)
    - Less Than or Equal To (`<=`)
    - Greater Than (`>`)
    - Greater Than or Equal To (`>=`)
7. Equality Operators
    - Equal To (`==`)
    - Not Equal To (`!=`)
8. Bitwise AND (`&`)
9. Bitwise XOR (`^`)
10. Bitwise OR (`|`)
11. Logical AND (`&&`)
12. Logical OR (`||`)
13. Conditional Operator (`?`, `:`)

Example:

```asm
    ld aw, 0x1234 + 0x5678              ; Load the sum of 0x1234 and 0x5678 into register AW.
    st [0x80005678 + 0x00000004], aw    ; Store the value in register AW to memory address 0x8000567C.
    mv bw, aw * 2                       ; Move the value in register AW multiplied by 2 to register BW.
    add aw, (0x1234 + 0x5678) / 2       ; Add the average of 0x1234 and 0x5678 to register AW.
    mv cw, aw << 2                      ; Move the value in register AW shifted left by 2 to register CW.
    sub cw, aw >> 2                     ; Subtract the value in register AW shifted right by 2 from register CW.
    stop                                ; Stop the program.
```