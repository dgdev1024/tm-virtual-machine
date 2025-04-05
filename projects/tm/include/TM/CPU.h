/**
 * @file  TM/CPU.h
 * @brief Contains the TM virtual CPU structure and function prototypes.
 */

#pragma once
#include <TM/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief   Forward declaration of the TM virtual CPU structure.
 */
typedef struct TM_CPU TM_CPU;

/**
 * @brief   Pointer to a function called by the CPU's functions to read a byte from the bus.
 * @param   p_Address The 32-bit address to read from.
 * @return  The byte read from the bus.
 */
typedef uint8_t (*TM_BusRead) (uint32_t);

/**
 * @brief   Pointer to a function called by the CPU's functions to write a byte to the bus.
 * @param   p_Address The 32-bit address to write to.
 * @param   p_Data    The byte to write to the bus.
 */
typedef void (*TM_BusWrite) (uint32_t, uint8_t);

/**
 * @brief   Pointer to a function called by the CPU whenever a CPU cycle is completed. This function
 *          is to tick the other hardware components attached to the CPU's bus.
 * @param   p_Cycles  The number of cycles to tick the other components.
 * @return  `true` if all of the components were ticked successfully; `false` otherwise.
 */
typedef bool (*TM_Cycle) (uint32_t);

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief   Creates a new instance of the TM virtual CPU.
 * @param   p_BusRead  The function to read from the bus.
 * @param   p_BusWrite The function to write to the bus.
 * @param   p_Cycle    The function to call when a CPU cycle is completed.
 * @return  A pointer to the new TM CPU instance.
 */
TM_CPU* TM_CreateCPU (TM_BusRead p_BusRead, TM_BusWrite p_BusWrite, TM_Cycle p_Cycle);

/**
 * @brief   Resets the TM CPU instance, setting its registers and state to their initial values.
 * @param   p_CPU     The TM CPU instance to reset.
 */
void TM_ResetCPU (TM_CPU* p_CPU);

/**
 * @brief   Destroys the given TM CPU instance, freeing its memory.
 * @param   p_CPU     The TM CPU instance to destroy.
 */
void TM_DestroyCPU (TM_CPU* p_CPU);

/**
 * @brief   Cycles the CPU for a specified number of cycles, ticking the other components attached
 *          to the CPU's bus.
 * 
 * Every time the CPU accesses the address bus (to read a byte from, or write a byte to) or modifies
 * its program counter or stack pointers, a CPU cycle is completed, upon which this function should
 * be called to tick the other components attached to the CPU's bus.
 * 
 * CPU cycles due to bus access only occur when the CPU internally accesses the bus.
 * 
 * @param   p_CPU     The TM CPU instance to cycle.
 * @param   p_Cycles  The number of cycles to cycle the CPU.
 */
void TM_CycleCPU (TM_CPU* p_CPU, uint32_t p_Cycles);

/**
 * @brief   Steps the CPU, either executing the next instruction or waiting for an interrupt to be
 *          requested if the CPU is halted.
 * 
 * @param   p_CPU     The TM CPU instance to step.
 * 
 * @return  `true` if the CPU was stepped successfully; `false` if the CPU has been or is stopped.
 */
bool TM_StepCPU (TM_CPU* p_CPU);

/**
 * @brief   Reads a byte from the bus at the specified address.
 * @param   p_CPU     The TM CPU instance to read from.
 * @param   p_Address The address to read from.
 * @return  The byte read from the bus.
 */
uint8_t TM_ReadByte (const TM_CPU* p_CPU, uint32_t p_Address);

/**
 * @brief   Reads a word from the bus at the specified address.
 * @param   p_CPU     The TM CPU instance to read from.
 * @param   p_Address The address to read from.
 * @return  The word read from the bus.
 */
uint16_t TM_ReadWord (const TM_CPU* p_CPU, uint32_t p_Address);

/**
 * @brief   Reads a double word from the bus at the specified address.
 * @param   p_CPU     The TM CPU instance to read from.
 * @param   p_Address The address to read from.
 * @return  The double word read from the bus.
 */
uint32_t TM_ReadDoubleWord (const TM_CPU* p_CPU, uint32_t p_Address);

/**
 * @brief   Writes a byte to the bus at the specified address.
 * @param   p_CPU     The TM CPU instance to write to.
 * @param   p_Address The address to write to.
 * @param   p_Data    The byte to write to the bus.
 */
void TM_WriteByte (TM_CPU* p_CPU, uint32_t p_Address, uint8_t p_Data);

/**
 * @brief   Writes a word to the bus at the specified address.
 * @param   p_CPU     The TM CPU instance to write to.
 * @param   p_Address The address to write to.
 * @param   p_Data    The word to write to the bus.
 */
void TM_WriteWord (TM_CPU* p_CPU, uint32_t p_Address, uint16_t p_Data);

/**
 * @brief   Writes a double word to the bus at the specified address.
 * @param   p_CPU     The TM CPU instance to write to.
 * @param   p_Address The address to write to.
 * @param   p_Data    The double word to write to the bus.
 */
void TM_WriteDoubleWord (TM_CPU* p_CPU, uint32_t p_Address, uint32_t p_Data);

/**
 * @brief   Requests an interrupt to be handled by the CPU.
 * 
 * This function sets one of 16 bits in the CPU's interrupt flag register, corresponding to an
 * interrupt to be handled. The CPU will then handle the interrupt on the next step, unless that
 * bit in the interrupt enable register or the interrupt master enable flag is not set, in which
 * case the interrupt will wait until both are set in order to be handled.
 * 
 * @param   p_CPU       The TM CPU instance to request the interrupt on.
 * @param   p_Interrupt The interrupt to request, which the number of the bit in the interrupt flag
 *                      register to set. The interrupt number must be between 0 and 15, inclusive.
 */
void TM_RequestInterrupt (TM_CPU* p_CPU, uint8_t p_Interrupt);

/**
 * @brief   Gets the value of a register in the TM CPU instance.
 * @param   p_CPU       The TM CPU instance to get the register from.
 * @param   p_Register  The register to get the value from.
 * @return  The value of the register.
 */
uint32_t TM_GetRegister (const TM_CPU* p_CPU, TM_CPURegister p_Register);

/**
 * @brief   Sets the value of a register in the TM CPU instance.
 * @param   p_CPU       The TM CPU instance to set the register in.
 * @param   p_Register  The register to set the value of.
 * @param   p_Value     The value to set the register to.
 */
void TM_SetRegister (TM_CPU* p_CPU, TM_CPURegister p_Register, uint32_t p_Value);

/**
 * @brief   Gets the value of a flag in the TM CPU instance.
 * @param   p_CPU     The TM CPU instance to get the flag from.
 * @param   p_Flag    The flag to get the value of.
 * @return  The value of the flag.
 */
bool TM_GetFlag (const TM_CPU* p_CPU, TM_CPUFlag p_Flag);

/**
 * @brief   Sets the value of a flag in the TM CPU instance.
 * @param   p_CPU     The TM CPU instance to set the flag in.
 * @param   p_Flag    The flag to set the value of.
 * @param   p_Value   The value to set the flag to.
 */
void TM_SetFlag (TM_CPU* p_CPU, TM_CPUFlag p_Flag, bool p_Value);

/**
 * @brief   Sets the values of the CPU's flags.
 * 
 * This function sets the values of the CPU's flags in a single call, allowing for more efficient
 * setting of multiple flags at once. Passing `0` for a flag clears it; passing a positive value
 * sets it; passing a negative value leaves it unchanged.
 * 
 * @param   p_CPU     The TM CPU instance to set the flags in.
 * @param   p_Z       The value to set the zero flag to. `0` clears it, positive value sets it,
 *                    negative value leaves it unchanged.
 * @param   p_N       The value to set the subtract flag to. `0` clears it, positive value sets it,
 *                    negative value leaves it unchanged.
 * @param   p_H       The value to set the half-carry flag to. `0` clears it, positive value sets it,
 *                    negative value leaves it unchanged.
 * @param   p_C       The value to set the carry flag to. `0` clears it, positive value sets it,
 *                    negative value leaves it unchanged.
 */
void TM_SetFlags (TM_CPU* p_CPU, int8_t p_Z, int8_t p_N, int8_t p_H, int8_t p_C);

/**
 * @brief   Gets the value of the CPU's interrupt enable register
 * 
 * The interrupt enable register is a special 16-bit register that contains a mask for the enabled
 * interrupts. Each bit in the register corresponds to an interrupt, and if the bit is set, the
 * interrupt is enabled; otherwise, it is disabled. The interrupt enable register is used to
 * determine which interrupts are enabled and can be handled by the CPU.
 * 
 * @param   p_CPU     The TM CPU instance to get the interrupt master enable flag from.
 * @return  `true` if interrupts are enabled; `false` otherwise.
 */
uint8_t TM_GetInterruptEnable (const TM_CPU* p_CPU);

/**
 * @brief   Gets the value of the CPU's interrupt flag register.
 * 
 * The interrupt flag register is a special 16-bit register that contains a mask for the pending
 * interrupts. Each bit in the register corresponds to an interrupt, and if the bit is set, the
 * interrupt is pending; otherwise, it is not.
 * 
 * @param   p_CPU     The TM CPU instance to get the interrupt flag register from.
 * @return  The value of the interrupt flag register.
 */
uint8_t TM_GetInterruptFlags (const TM_CPU* p_CPU);

/**
 * @brief   Sets the value of the CPU's interrupt enable register.
 * 
 * The interrupt enable register is a special 16-bit register that contains a mask for the enabled
 * interrupts. Each bit in the register corresponds to an interrupt, and if the bit is set, the
 * interrupt is enabled; otherwise, it is disabled.
 * 
 * @param   p_CPU     The TM CPU instance to set the interrupt enable register in.
 * @param   p_Value   The value to set the interrupt enable register to.
 */
void TM_SetInterruptEnable (TM_CPU* p_CPU, uint8_t p_Value);

/**
 * @brief   Sets the value of the CPU's interrupt flag register.
 * 
 * The interrupt flag register is a special 16-bit register that contains a mask for the pending
 * interrupts. Each bit in the register corresponds to an interrupt, and if the bit is set, the
 * interrupt is pending; otherwise, it is not.
 * 
 * @param   p_CPU     The TM CPU instance to set the interrupt flag register in.
 * @param   p_Value   The value to set the interrupt flag register to.
 */
void TM_SetInterruptFlags (TM_CPU* p_CPU, uint8_t p_Value);

/**
 * @brief   Gets the value of the CPU's interrupt master enable flag.
 * 
 * The interrupt master enable flag is a special flag that indicates whether interrupts are enabled
 * or disabled. If the flag is set, interrupts are enabled; otherwise, they are disabled. The
 * interrupt master enable flag is used to determine whether the CPU should process interrupts or
 * ignore them.
 * 
 * @param   p_CPU     The TM CPU instance to get the interrupt master enable flag from.
 * @return  `true` if interrupts are enabled; `false` otherwise.
 */
bool TM_GetInterruptMasterEnable (const TM_CPU* p_CPU);

/**
 * @brief   Gets the value of the CPU's program counter.
 * @param   p_CPU     The TM CPU instance to get the program counter from.
 * @return  The value of the program counter.
 */
uint32_t TM_GetProgramCounter (const TM_CPU* p_CPU);

/**
 * @brief   Sets the value of the CPU's program counter.
 * @param   p_CPU     The TM CPU instance to set the program counter in.
 * @param   p_Value   The value to set the program counter to.
 */
void TM_SetProgramCounter (TM_CPU* p_CPU, uint32_t p_Value);

/**
 * @brief   Gets the value of the CPU's data stack pointer.
 * @param   p_CPU     The TM CPU instance to get the stack pointer from.
 * @return  The value of the data stack pointer.
 */
uint32_t TM_GetDataStackPointer (const TM_CPU* p_CPU);

/**
 * @brief   Gets the value of the CPU's call stack pointer.
 * @param   p_CPU     The TM CPU instance to get the stack pointer from.
 * @return  The value of the call stack pointer.
 */
uint32_t TM_GetCallStackPointer (const TM_CPU* p_CPU);

/**
 * @brief   Gets the value of the CPU's error code register.
 * 
 * The error code register is a special 8-bit register that contains the last error code generated
 * by the CPU, or set with the `SEC` instruction. It is used to indicate whether or not something
 * went wrong during the execution of the CPU's last instruction or ticking of its bus's components.
 * 
 * @param   p_CPU     The TM CPU instance to get the error code from.
 * @return  The value of the error code register.
 */
uint8_t TM_GetErrorCode (const TM_CPU* p_CPU);

/**
 * @brief   Sets the value of the CPU's error code register.
 * 
 * The error code register is a special 8-bit register that contains the last error code generated
 * by the CPU, or set with the `SEC` instruction. It is used to indicate whether or not something
 * went wrong during the execution of the CPU's last instruction or ticking of its bus's components.
 * Setting the error code this way also sets the CPU's stop flag.
 * 
 * @param   p_CPU           The TM CPU instance to set the error code in.
 * @param   p_ErrorCode     The value to set the error code register to.
 * @return  `true` if the error code set is zero, indicating a normal exit; `false` otherwise.
 */
bool TM_SetErrorCode (TM_CPU* p_CPU, uint8_t p_ErrorCode);

/**
 * @brief   Gets the value of the CPU's halt flag.
 * 
 * The halt flag is a special flag that indicates whether or not the CPU is currently halted. The
 * CPU is halted by the `HALT` instruction, and it remains halted until an interrupt is requested,
 * or the CPU is reset.
 * 
 * @param   p_CPU     The TM CPU instance to get the halt flag from.
 * @return  `true` if the CPU is halted; `false` otherwise.
 */
bool TM_IsHalted (const TM_CPU* p_CPU);

/**
 * @brief   Gets the value of the CPU's stop flag.
 * 
 * The stop flag is a special flag that indicates whether or not the CPU is currently stopped. The
 * CPU is stopped by the `STOP` instruction, or when the error code register is set to a non-zero
 * value with the `TM_SetErrorCode` function. The CPU remains stopped until it is reset.
 * 
 * @param   p_CPU     The TM CPU instance to get the stop flag from.
 * @return  `true` if the CPU is stopped; `false` otherwise.
 */
bool TM_IsStopped (const TM_CPU* p_CPU);
