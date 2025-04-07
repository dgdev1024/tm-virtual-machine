/**
 * @file  TOMBOY/RAM.h
 */

#pragma once
#include <TOMBOY/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief   Forward declaration of the TOMBOY RAM context structure. 
 */
typedef struct TOMBOY_RAM TOMBOY_RAM;

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief   Creates a new instance of the TOMBOY random-access memory (RAM) context.
 * 
 * The amount of working RAM and static RAM is specified by the program header found in the loaded
 * program ROM.
 * 
 * @param   p_WRAMSize  The size of the working RAM in bytes.
 * @param   p_SRAMSize  The size of the static RAM in bytes.
 * @param   p_XRAMSize  The size of the executable RAM in bytes.
 * 
 * @return  A pointer to the newly created TOMBOY RAM context, or NULL if the creation failed.
 */
TOMBOY_RAM* TOMBOY_CreateRAM (uint32_t p_WRAMSize, uint32_t p_SRAMSize, uint32_t p_XRAMSize);

/**
 * @brief   Destroys the specified TOMBOY RAM context.
 * 
 * @param   p_RAM  A pointer to the TOMBOY RAM context to destroy.
 */
void TOMBOY_DestroyRAM (TOMBOY_RAM* p_RAM);

/**
 * @brief   Resets the specified TOMBOY RAM context to its initial state, clearing its allocated
 *          buffers.
 * 
 * @param   p_RAM  A pointer to the TOMBOY RAM context to reset.
 */
void TOMBOY_ResetRAM (TOMBOY_RAM* p_RAM);

/**
 * @brief   Loads the program's static RAM (SRAM) from the specified file.
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Filename The name of the file to load the SRAM from.
 * 
 * @return  True if the SRAM was loaded successfully, false otherwise.
 */
bool TOMBOY_LoadSRAM (TOMBOY_RAM* p_RAM, const char* p_Filename);

/**
 * @brief   Saves the program's static RAM (SRAM) to the specified file.
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Filename The name of the file to save the SRAM to.
 * 
 * @return  True if the SRAM was saved successfully, false otherwise.
 */
bool TOMBOY_SaveSRAM (const TOMBOY_RAM* p_RAM, const char* p_Filename);

// Public Function Prototypes - Memory Access //////////////////////////////////////////////////////

/**
 * @brief   Reads a byte from the specified address in the TOMBOY Working RAM (WRAM).
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to read from.
 * 
 * @return  The byte read from the specified address.
 */
uint8_t TOMBOY_ReadWRAMByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address);

/**
 * @brief   Writes a byte to the specified address in the TOMBOY Working RAM (WRAM).
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to write to.
 * @param   p_Value    The byte value to write.
 */
void TOMBOY_WriteWRAMByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief   Reads a byte from the specified address in the TOMBOY Static RAM (SRAM).
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to read from.
 * 
 * @return  The byte read from the specified address.
 */
uint8_t TOMBOY_ReadSRAMByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address);

/**
 * @brief   Writes a byte to the specified address in the TOMBOY Static RAM.
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to write to.
 * @param   p_Value    The byte value to write.
 */
void TOMBOY_WriteSRAMByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief   Reads a byte from the specified address in the TOMBOY's executable RAM (XRAM).
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to read from.
 * 
 * @return  The byte read from the specified address.
 */
uint8_t TOMBOY_ReadXRAMByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address);

/**
 * @brief   Writes a byte to the specified address in the TOMBOY's executable RAM (XRAM).
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to write to.
 * @param   p_Value    The byte value to write.
 */
void TOMBOY_WriteXRAMByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief   Reads a byte from the specified address in the TOMBOY's Quick RAM (QRAM).
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to read from.
 * 
 * @return  The byte read from the specified address.
 */
uint8_t TOMBOY_ReadQRAMByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address);

/**
 * @brief   Writes a byte to the specified address in the TOMBOY's Quick RAM (QRAM).
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to write to.
 * @param   p_Value    The byte value to write.
 */
void TOMBOY_WriteQRAMByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief   Reads a byte from the specified address in the TOMBOY's data stack.
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to read from.
 * 
 * @return  The byte read from the specified address.
 */
uint8_t TOMBOY_ReadDataStackByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address);

/**
 * @brief   Writes a byte to the specified address in the TOMBOY's data stack.
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to write to.
 * @param   p_Value    The byte value to write.
 */
void TOMBOY_WriteDataStackByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief   Reads a byte from the specified address in the TOMBOY's call stack.
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to read from.
 * 
 * @return  The byte read from the specified address.
 */
uint8_t TOMBOY_ReadCallStackByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address);

/**
 * @brief   Writes a byte to the specified address in the TOMBOY's call stack.
 * 
 * @param   p_RAM      A pointer to the TOMBOY RAM context.
 * @param   p_Address  The address to write to.
 * @param   p_Value    The byte value to write.
 */
void TOMBOY_WriteCallStackByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value);
