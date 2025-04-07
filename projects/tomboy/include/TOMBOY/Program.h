/**
 * @file  TOMBOY/Program.h
 * @brief Contains the definition of the TOMBOY program structure and its associated functions.
 */

#pragma once
#include <TOMBOY/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief      Forward declaration of the TOMBOY program structure.
 */
typedef struct TOMBOY_Program TOMBOY_Program;

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief      Creates a new TOMBOY program instance, loading the program from the specified file.
 * 
 * @param      p_Filename  The path to the file containing the program data.
 * 
 * @return     A pointer to the newly created TOMBOY program instance, or `NULL` if the program
 *             could not be created or loaded.
 */
TOMBOY_Program* TOMBOY_CreateProgram (const char* p_Filename);

/**
 * @brief      Destroys a TOMBOY program instance, freeing its resources.
 * 
 * @param      p_Program  A pointer to the TOMBOY program instance to destroy.
 */
void TOMBOY_DestroyProgram (TOMBOY_Program* p_Program);

/**
 * @brief      Reads a byte from the specified address in the TOMBOY program's memory.
 * 
 * @param      p_Program  A pointer to the TOMBOY program instance.
 * @param      p_Address  The relative address to read from.
 * 
 * @return     The byte read from the specified address in the program's memory.
 */
uint8_t TOMBOY_ReadProgramByte (const TOMBOY_Program* p_Program, uint32_t p_Address);

/**
 * @brief      Gets the requested size of the program's static RAM (SRAM).
 * 
 * @param      p_Program  A pointer to the TOMBOY program instance.
 * 
 * @return     The requested size of the program's static RAM (SRAM) in bytes.
 */
uint32_t TOMBOY_GetRequestedSRAMSize (const TOMBOY_Program* p_Program);

/**
 * @brief      Gets the requested size of the program's working RAM (WRAM).
 * 
 * @param      p_Program  A pointer to the TOMBOY program instance.
 * 
 * @return     The requested size of the program's working RAM (WRAM) in bytes.
 */
uint32_t TOMBOY_GetRequestedWRAMSize (const TOMBOY_Program* p_Program);

/**
 * @brief      Gets the requested size of the program's executable RAM (XRAM).
 * 
 * @param      p_Program  A pointer to the TOMBOY program instance.
 * 
 * @return     The requested size of the program's executable RAM (XRAM) in bytes.
 */
uint32_t TOMBOY_GetRequestedXRAMSize (const TOMBOY_Program* p_Program);

/**
 * @brief      Gets the program's title, if any.
 * 
 * @param      p_Program  A pointer to the TOMBOY program instance.
 * 
 * @return     A pointer to the program's title string, or `NULL` if no title is available.
 */
const char* TOMBOY_GetProgramTitle (const TOMBOY_Program* p_Program);
