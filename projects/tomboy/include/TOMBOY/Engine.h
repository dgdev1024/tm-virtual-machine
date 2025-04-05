/**
 * @file  TOMBOY/Engine.h
 * @brief Contains the definition of the TOMBOY emulator engine structure and public functions.
 */

#pragma once
#include <TOMBOY/Common.h>
#include <TM/CPU.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward-declaration of the TOMBOY program structure.
 */
typedef struct TOMBOY_Program TOMBOY_Program;

/**
 * @brief A forward-declaration of the TOMBOY PPU structure.
 */
typedef struct TOMBOY_PPU TOMBOY_PPU;

/**
 * @brief A forward-declaration of the TOMBOY joypad structure.
 */
typedef struct TOMBOY_Joypad TOMBOY_Joypad;

/**
 * @brief A forward-declaration of the TOMBOY network interface structure.
 */
typedef struct TOMBOY_Network TOMBOY_Network;

/**
 * @brief A forward-declaration of the TOMBOY Engine structure.
 */
typedef struct TOMBOY_Engine TOMBOY_Engine;

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new TOMBOY emulator engine instance, supplying a loaded program ROM.
 * 
 * @param p_Program     A pointer to the program ROM to load into the engine.
 * 
 * @return A pointer to the new TOMBOY engine instance.
 */
TOMBOY_Engine* TOMBOY_CreateEngine (const TOMBOY_Program* p_Program);

/**
 * @brief Resets the given TOMBOY emulator engine instance, resetting it and its components to their
 *        initial state.
 * 
 * @param p_Engine      A pointer to the TOMBOY engine instance to reset.
 */
void TOMBOY_ResetEngine (TOMBOY_Engine* p_Engine);

/**
 * @brief Destroys the given TOMBOY emulator engine instance, freeing its resources.
 * 
 * @param p_Engine      A pointer to the TOMBOY engine instance to destroy.
 */
void TOMBOY_DestroyEngine (TOMBOY_Engine* p_Engine);

/**
 * @brief Makes the given TOMBOY emulator engine instance the current engine. The bus and cycle
 *        functions passed into the CPU component, as well the `TOMBOY_TickEngine` function, will
 *        use this engine instance.
 * 
 * @param p_Engine      A pointer to the TOMBOY engine instance to set as the current engine.
 */
void TOMBOY_MakeEngineCurrent (TOMBOY_Engine* p_Engine);

/**
 * @brief Gets the current TOMBOY emulator engine instance.
 * 
 * @return A pointer to the current TOMBOY engine instance.
 */
TOMBOY_Engine* TOMBOY_GetCurrentEngine ();

/**
 * @brief Checks if a TOMBOY engine instance is currently set as the current engine.
 * 
 * @return `true` if a TOMBOY engine instance is currently set, `false` otherwise.
 */
bool TOMBOY_IsCurrentEngineSet ();

/**
 * @brief Ticks the current TOMBOY emulator engine instance, prompting the CPU to execute the next
 *        instruction, and updating the engine's components.
 * 
 * @return `true` if the engine's components ticked with no errors; `false` otherwise, or if the
 *         current engine is not set.
 */
bool TOMBOY_TickEngine ();

/**
 * @brief Gets the number of cycles elapsed on the given TOMBOY emulator engine instance.
 * 
 * @param p_Engine      A pointer to the TOMBOY engine instance to get the cycle count from.
 * 
 * @return The number of cycles elapsed on the current engine, or 0 if the current engine is not set.
 */
uint64_t TOMBOY_GetCycleCount (const TOMBOY_Engine* p_Engine);

/**
 * @brief Requests an interrupt with the given index to be handled by the current TOMBOY emulator
 *        engine's CPU context.
 * 
 * @param p_Engine       A pointer to the TOMBOY engine instance whose CPU to request the interrupt on.
 * @param p_Interrupt    The index of the interrupt to request, which is the number of the bit in the
 *                       interrupt flag register to set. The interrupt number must be between 0 and 15,
 *                       inclusive.
 *
 * @return `true` if the interrupt was requested successfully; `false` otherwise, or if the current
 *         engine is not set.
 */
bool TOMBOY_RequestInterrupt (TOMBOY_Engine* p_Engine, uint8_t p_Interrupt);

/**
 * @brief Gets the TOMBOY CPU instance from the given TOMBOY emulator engine instance.
 * 
 * @param p_Engine      A pointer to the TOMBOY engine instance to get the CPU from.
 * 
 * @return A pointer to the TOMBOY CPU instance, or `NULL` if the engine is not set.
 */
TM_CPU* TOMBOY_GetCPU (TOMBOY_Engine* p_Engine);
