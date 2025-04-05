/**
 * @file  TOMBOY/Realtime.h
 * @brief Contains the definition of the TOMBOY emulator real-time clock structure and public functions.
 */

#pragma once
#include <TOMBOY/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward-declaration of the TOMBOY emulator engine structure.
 */
typedef struct TOMBOY_Engine TOMBOY_Engine;

/**
 * @brief A forward-declaration of the TOMBOY real-time clock structure.
 */
typedef struct TOMBOY_Realtime TOMBOY_Realtime;

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new TOMBOY emulator real-time clock instance.
 * 
 * @param p_Engine  A pointer to the TOMBOY engine instance to associate with the real-time clock.
 * 
 * @return A pointer to the new TOMBOY real-time clock instance.
 */
TOMBOY_Realtime* TOMBOY_CreateRealtime (TOMBOY_Engine* p_Engine);

/**
 * @brief Resets the given TOMBOY emulator real-time clock instance.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to reset.
 */
void TOMBOY_ResetRealtime (TOMBOY_Realtime* p_Realtime);

/**
 * @brief Destroys the given TOMBOY emulator real-time clock instance, freeing its resources.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to destroy.
 */
void TOMBOY_DestroyRealtime (TOMBOY_Realtime* p_Realtime);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief Reads the given real-time clock instance's seconds register, `RTCS`.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to read from.
 * 
 * @return The value of the `RTCS` register.
 */
uint8_t TOMBOY_ReadRTCS (const TOMBOY_Realtime* p_Realtime);

/**
 * @brief Reads the given real-time clock instance's minutes register, `RTCM`.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to read from.
 * 
 * @return The value of the `RTCM` register.
 */
uint8_t TOMBOY_ReadRTCM (const TOMBOY_Realtime* p_Realtime);

/**
 * @brief Reads the given real-time clock instance's hours register, `RTCH`.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to read from.
 * 
 * @return The value of the `RTCH` register.
 */
uint8_t TOMBOY_ReadRTCH (const TOMBOY_Realtime* p_Realtime);

/**
 * @brief Reads the given real-time clock instance's day counter high byte register, `RTCDH`.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to read from.
 * 
 * @return The value of the `RTCDH` register.
 */
uint8_t TOMBOY_ReadRTCDH (const TOMBOY_Realtime* p_Realtime);

/**
 * @brief Reads the given real-time clock instance's day counter low byte register, `RTCDL`.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to read from.
 * 
 * @return The value of the `RTCDL` register.
 */
uint8_t TOMBOY_ReadRTCDL (const TOMBOY_Realtime* p_Realtime);

/**
 * @brief Reads the given real-time clock instance's RNG register, `RTCR`.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to read from.
 * 
 * @return The value of the `RTCR` register, which will be random.
 */
uint8_t TOMBOY_ReadRTCR (const TOMBOY_Realtime* p_Realtime);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief Writes to the given real-time clock instance's latch register, `RTCL`.
 * 
 * Writing any value to this register latches the current system clock's day and time into the
 * real-time clock's day counter registers.
 * 
 * @param p_Realtime A pointer to the TOMBOY real-time clock instance to write to.
 * @param p_Value    The value to write to the `RTCL` register.
 */
void TOMBOY_WriteRTCL (TOMBOY_Realtime* p_Realtime, uint8_t p_Value);
