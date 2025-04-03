/**
 * @file  TM/Common.h
 * @brief Contains commonly-used includes, constants, typedefs and macros.
 */

#pragma once

// Include Files ///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

// Helper Macros - Logging /////////////////////////////////////////////////////////////////////////

#define TM_log(p_Stream, p_Level, ...) \
    fprintf(p_Stream, "[%s] %s: ", p_Level, __func__); \
    fprintf(p_Stream, __VA_ARGS__); \
    fprintf(p_Stream, "\n");
#define TM_info(...) TM_log(stdout, "INFO", __VA_ARGS__)
#define TM_warn(...) TM_log(stderr, "WARN", __VA_ARGS__)
#define TM_error(...) TM_log(stderr, "ERROR", __VA_ARGS__)
#define TM_fatal(...) TM_log(stderr, "FATAL", __VA_ARGS__)

#define TM_perror(...) \
{ \
    int l_Errno = errno; \
    fprintf(stderr, "[%s] %s: ", "ERROR", __func__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, " - %s\n", strerror(l_Errno)); \
}
#define TM_pfatal(...) \
{ \
    int l_Errno = errno; \
    fprintf(stderr, "[%s] %s: ", "FATAL", __func__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, " - %s\n", strerror(l_Errno)); \
}

#if defined(TM_DEBUG)
    #define TM_debug(...) TM_log(stdout, "DEBUG", __VA_ARGS__)
    #define TM_trace() \
        TM_log(stdout, "TRACE", " - In Function '%s'", __func__); \
        TM_log(stdout, "TRACE", " - In File '%s:%d'", __FILE__, __LINE__);
#else
    #define TM_debug(...)
    #define TM_trace()
#endif

// Helper Macros - Error Handling //////////////////////////////////////////////////////////////////

#define TM_assert(p_Clause) \
    if (!(p_Clause)) \
    { \
        TM_fatal("Assertion Failure: '%s'!", #p_Clause); \
        TM_trace(); \
        abort(); \
    }
#define TM_expect(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        TM_fatal(__VA_ARGS__); \
        TM_trace(); \
        exit(EXIT_FAILURE); \
    }
#define TM_pexpect(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        TM_pfatal(__VA_ARGS__); \
        TM_trace(); \
        exit(EXIT_FAILURE); \
    }
#define TM_check(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        TM_error(__VA_ARGS__); \
        TM_trace(); \
        return; \
    }
#define TM_pcheck(p_Clause, ...) \
    if (!(p_Clause)) \
    { \
        TM_perror(__VA_ARGS__); \
        TM_trace(); \
        return; \
    }
#define TM_vcheck(p_Clause, p_Value, ...) \
    if (!(p_Clause)) \
    { \
        TM_error(__VA_ARGS__); \
        TM_trace(); \
        return p_Value; \
    }
#define TM_pvcheck(p_Clause, p_Value, ...) \
    if (!(p_Clause)) \
    { \
        TM_perror(__VA_ARGS__); \
        TM_trace(); \
        return p_Value; \
    }

// Helper Macros - Memory Management ///////////////////////////////////////////////////////////////

#define TM_malloc(p_Count, p_Type) \
    ((p_Type*) malloc((p_Count) * sizeof(p_Type)))
#define TM_calloc(p_Count, p_Type) \
    ((p_Type*) calloc((p_Count), sizeof(p_Type)))
#define TM_realloc(p_Ptr, p_Count, p_Type) \
    ((p_Type*) realloc((p_Ptr), (p_Count) * sizeof(p_Type)))
#define TM_free(p_Ptr) \
    if (p_Ptr != NULL) { free(p_Ptr); } p_Ptr = NULL;

// Memory Map Constants ////////////////////////////////////////////////////////////////////////////

#define TM_ROM_BEGIN            0x00000000
#define TM_MDATA_BEGIN          0x00000000
#define TM_MDATA_END            0x00000FFF
#define TM_RST_BEGIN            0x00001000
#define TM_RST_END              0x00001FFF
#define TM_INT_BEGIN            0x00002000
#define TM_INT_END              0x00002FFF
#define TM_CODE_BEGIN           0x00003000
#define TM_CODE_END             0x3FFFFFFF
#define TM_DATA_BEGIN           0x40000000
#define TM_DATA_END             0x7FFFFFFF
#define TM_ROM_END              0x7FFFFFFF

#define TM_RAM_BEGIN            0x80000000
#define TM_DRAM_BEGIN           0x80000000
#define TM_DRAM_END             0xDFFFFFFF
#define TM_XRAM_BEGIN           0xE0000000
#define TM_XRAM_END             0xFFFCFFFF
#define TM_DSTACK_BEGIN         0xFFFD0000
#define TM_DSTACK_END           0xFFFDFFFF
#define TM_CSTACK_BEGIN         0xFFFE0000
#define TM_CSTACK_END           0xFFFEFFFF
#define TM_QRAM_BEGIN           0xFFFF0000
#define TM_QRAM_END             0xFFFFFEFF
#define TM_IO_BEGIN             0xFFFFFF00
#define TM_IO_END               0xFFFFFFFF
#define TM_RAM_END              0xFFFFFFFF

// CPU Register Enumeration ////////////////////////////////////////////////////////////////////////

/**
 * @brief   Enumerates the CPU's general-purpose registers.
 * 
 * The TM CPU has four 32-bit general-purpose registers: `A`, `B`, `C` and `D`. The registers can
 * be accessed as follows:
 * 
 * - As a whole register (e.g. `A`).
 * 
 * - As a 16-bit register (e.g. `AW`, the lower 16 bits of `A`).
 * 
 * - As an 8-bit register (e.g. `AH` and `AL`, the upper and lower 8 bits of `AW`, respectively).
 * 
 * The size of the registers can be determined by checking the lower 2 bits of the register's
 * enumeration value:
 * 
 * - `00`: 32-bit register
 * 
 * - `01`: 16-bit register
 * 
 * - `10` and `11`: 8-bit registers
 * 
 * The register being accessed, in the meantime, can be determined by the upper 2 bits:
 * 
 * - `00`: Register `A`, a general-purpose register, designated as the accumulator.
 * 
 * - `01`: Register `B`, a general-purpose register, traditionally used as a base register.
 * 
 * - `10`: Register `C`, a general-purpose register, traditionally used as a counter register.
 * 
 * - `11`: Register `D`, a general-purpose register, traditionally used as a data register.
 * 
 */
typedef enum TM_CPURegister
{
    TM_REG_A  = 0b0000,
    TM_REG_AW = 0b0001,
    TM_REG_AH = 0b0010,
    TM_REG_AL = 0b0011,

    TM_REG_B  = 0b0100,
    TM_REG_BW = 0b0101,
    TM_REG_BH = 0b0110,
    TM_REG_BL = 0b0111,

    TM_REG_C  = 0b1000,
    TM_REG_CW = 0b1001,
    TM_REG_CH = 0b1010,
    TM_REG_CL = 0b1011,

    TM_REG_D  = 0b1100,
    TM_REG_DW = 0b1101,
    TM_REG_DH = 0b1110,
    TM_REG_DL = 0b1111,
} TM_CPURegister;

// CPU Flag Enumeration ////////////////////////////////////////////////////////////////////////////

/**
 * @brief   Enumerates the CPU's flags.
 * 
 * The TM CPU keeps a special 8-bit register called the flags register, which contains the status of
 * the CPU's last operation. The register is modeled after the flags register of the Sharp LR35902 CPU,
 * whose flags are laid out as follows:
 * 
 * - `Z`: The zero flag, used to indicate if the result of the last operation was zero.
 * 
 * - `N`: The subtract flag, used to indicate if the last operation was (or involved) a subtraction.
 * 
 * - `H`: The half-carry flag, used to indicate if a carry occurred in a portion of the last operation's
 *        result.
 * 
 * - `C`: The carry flag, used to indicate if a carry occurred in the last operation's result.
 */
typedef enum TM_CPUFlag
{
    TM_FLAG_Z = 7, ///< @brief The zero flag, used to indicate if the result of the last operation was zero.
    TM_FLAG_N = 6, ///< @brief The subtract flag, used to indicate if the last operation was (or involved) a subtraction.
    TM_FLAG_H = 5, ///< @brief The half-carry flag, used to indicate if a carry occurred in a portion of the last operation's result.
    TM_FLAG_C = 4, ///< @brief The carry flag, used to indicate if a carry occurred in the last operation's result.
} TM_CPUFlag;

// CPU Execution Condition Enumeration /////////////////////////////////////////////////////////////

/**
 * @brief   Enumerates the CPU's execution conditions.
 * 
 * Certain instructions in the TM CPU's instruction set - particularly the control-transfer
 * instructions - can be executed conditionally, depending on the state of the CPU's flags.
 */
typedef enum TM_CPUCondition
{
    TM_COND_NONE = 0, ///< @brief No condition, used to indicate that the CPU should execute the next instruction.
    TM_COND_Z    = 1, ///< @brief The zero condition, used to indicate that the CPU should execute the next instruction if the zero flag is set.
    TM_COND_NZ   = 2, ///< @brief The not zero condition, used to indicate that the CPU should execute the next instruction if the zero flag is not set.
    TM_COND_C    = 3, ///< @brief The carry condition, used to indicate that the CPU should execute the next instruction if the carry flag is set.
    TM_COND_NC   = 4, ///< @brief The not carry condition, used to indicate that the CPU should execute the next instruction if the carry flag is not set.
} TM_CPUCondition;

// CPU Instruction Enumeration /////////////////////////////////////////////////////////////////////

/**
 * @brief   Enumerates the instructions in the TM CPU's instruction set.
 * 
 * Each instruction in the TM CPU's instruction set is represented by a unique 16-bit value, whose
 * nibbles are laid out in the following way: `0xIIXY`...
 * 
 * - `II`: The instruction's opcode, which determines the instruction's type and operation.
 * 
 * - `X`: The instruction's first parameter, which can be a register or an execution condition.
 * 
 * - `Y`: The instruction's second parameter, which is usually a register.
 * 
 * The instructions' enumeration values are their base opcodes, which don't account for variations
 * in the instruction.
 */
typedef enum TM_CPUInstruction
{
    TM_INST_NOP  = 0x0000, ///< @brief No operation, used to indicate that the CPU should do nothing.
    TM_INST_STOP = 0x0100, ///< @brief Stop the CPU, used to indicate that the CPU should stop executing instructions.
    TM_INST_HALT = 0x0200, ///< @brief Halt the CPU, used to indicate that the CPU should halt execution until an interrupt occurs.
    TM_INST_SEC  = 0x0300, ///< @brief Set the error code register. The value to set is in the instruction's lower 8 bits.
    TM_INST_CEC  = 0x0400, ///< @brief Clear the error code register, setting it to zero.
    TM_INST_DI   = 0x0500, ///< @brief Disable interrupts, used to indicate that the CPU should ignore interrupts.
    TM_INST_EI   = 0x0600, ///< @brief Enable interrupts, used to indicate that the CPU should process interrupts, starting at the next step.
    TM_INST_DAA  = 0x0700, ///< @brief Decimal adjust accumulator, used to adjust the value in one of the accumulator registers.
    TM_INST_SCF  = 0x0800, ///< @brief Set the carry flag, used to set the CPU's carry flag.
    TM_INST_CCF  = 0x0900, ///< @brief Complement the carry flag, used to toggle the CPU's carry flag.
    TM_INST_LD   = 0x1000, ///< @brief Load a value into a register.
    TM_INST_LDQ  = 0x1300, ///< @brief Load a value from QRAM into a register.
    TM_INST_LDH  = 0x1500, ///< @brief Load a value from a hardware I/O port into a register.
    TM_INST_ST   = 0x1700, ///< @brief Store a value from a register into memory.
    TM_INST_STQ  = 0x1900, ///< @brief Store a value from a register into QRAM.
    TM_INST_STH  = 0x1B00, ///< @brief Store a value from a register into a hardware I/O port.
    TM_INST_MV   = 0x1D00, ///< @brief Move a value from one register to another.
    TM_INST_PUSH = 0x1E00, ///< @brief Push a value onto the stack.
    TM_INST_POP  = 0x1F00, ///< @brief Pop a value from the stack into a register.
    TM_INST_JMP  = 0x2000, ///< @brief Jump to a specified address.
    TM_INST_JPB  = 0x2300, ///< @brief Jump by a specified offset.
    TM_INST_CALL = 0x2400, ///< @brief Call a subroutine at a specified address.
    TM_INST_RST  = 0x2500, ///< @brief Call one of 16 restart vector handlers.
    TM_INST_RET  = 0x2600, ///< @brief Return from a subroutine.
    TM_INST_RETI = 0x2700, ///< @brief Return from an interrupt.
    TM_INST_JPS  = 0x2800, ///< @brief Jump to the start of the program.
    TM_INST_INC  = 0x3000, ///< @brief Increment a value.
    TM_INST_DEC  = 0x3200, ///< @brief Decrement a value.
    TM_INST_ADD  = 0x3400, ///< @brief Add two values.
    TM_INST_ADC  = 0x3700, ///< @brief Add two values with carry.
    TM_INST_SUB  = 0x3A00, ///< @brief Subtract two values.
    TM_INST_SBC  = 0x3D00, ///< @brief Subtract two values with borrow.
    TM_INST_AND  = 0x4000, ///< @brief Bitwise AND two values.
    TM_INST_OR   = 0x4300, ///< @brief Bitwise OR two values.
    TM_INST_XOR  = 0x4600, ///< @brief Bitwise XOR two values.
    TM_INST_NOT  = 0x4900, ///< @brief Bitwise NOT a value.
    TM_INST_CMP  = 0x5000, ///< @brief Compare two values.
    TM_INST_SLA  = 0x6000, ///< @brief Shift a value left arithmetic.
    TM_INST_SRA  = 0x6200, ///< @brief Shift a value right arithmetic.
    TM_INST_SRL  = 0x6400, ///< @brief Shift a value right logical.
    TM_INST_RL   = 0x6600, ///< @brief Rotate a value left through the carry.
    TM_INST_RLC  = 0x6800, ///< @brief Rotate a value left.
    TM_INST_RR   = 0x6A00, ///< @brief Rotate a value right through the carry.
    TM_INST_RRC  = 0x6C00, ///< @brief Rotate a value right.
    TM_INST_BIT  = 0x7000, ///< @brief Bit test.
    TM_INST_RES  = 0x7200, ///< @brief Bit reset.
    TM_INST_SET  = 0x7400, ///< @brief Bit set.
    TM_INST_SWAP = 0x7600, ///< @brief Swap the nibbles/bytes/words of a value.
} TM_CPUInstruction;

// CPU Error Code Enumeration //////////////////////////////////////////////////////////////////////

/**
 * @brief   Enumerates the error codes generated by the CPU.
 * 
 * The TM CPU generates error codes when it encounters an error during execution. The error codes
 * are used to indicate the type of error that occurred, and can be set with the `SEC` instruction.
 */
typedef enum TM_ErrorCode
{
    TM_EC_OK,                           ///< @brief No error, used to indicate that the CPU's last operation was successful.
    TM_EC_INVALID_OPCODE,               ///< @brief Invalid opcode, used to indicate that the CPU encountered an invalid opcode.
    TM_EC_INVALID_ARGUMENT,             ///< @brief Invalid argument, used to indicate that the CPU encountered an invalid argument.
    TM_EC_BUS_READ,                     ///< @brief Bus read error, used to indicate that the CPU encountered an error while reading from the bus.
    TM_EC_BUS_WRITE,                    ///< @brief Bus write error, used to indicate that the CPU encountered an error while writing to the bus.
    TM_EC_BAD_READ,                     ///< @brief Bad read error, used to indicate that the CPU attempted to read from non-readable memory.
    TM_EC_BAD_WRITE,                    ///< @brief Bad write error, used to indicate that the CPU attempted to write to non-writable memory.
    TM_EC_BAD_EXECUTE,                  ///< @brief Bad execute error, used to indicate that the CPU attempted to execute non-executable memory.
    TM_EC_DATA_STACK_OVERFLOW,          ///< @brief Data stack overflow error, used to indicate that the CPU attempted to push a value onto the data stack when it was full.
    TM_EC_DATA_STACK_UNDERFLOW,         ///< @brief Data stack underflow error, used to indicate that the CPU attempted to pop a value from the data stack when it was empty.
    TM_EC_CALL_STACK_OVERFLOW,          ///< @brief Call stack overflow error, used to indicate that the CPU attempted to push an address onto the call stack when it was full.
    TM_EC_HARDWARE_FAULT,               ///< @brief Hardware fault error, used to indicate that the CPU encountered an error while ticking one of the bus's attached components.
} TM_ErrorCode;
