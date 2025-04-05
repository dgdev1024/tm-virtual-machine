/**
 * @file  TOMBOY/Timer.h
 * @brief Contains the definition of the TOMBOY emulator timer structure and public functions.
 */

#pragma once
#include <TOMBOY/Common.h>

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward-declaration of the TOMBOY emulator engine structure.
 */
typedef struct TOMBOY_Engine TOMBOY_Engine;

/**
 * @brief A forward-declaration of the TOMBOY timer context structure.
 */
typedef struct TOMBOY_Timer TOMBOY_Timer;

// Timer Clock Speed Enumeration ///////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the possible clock speeds of the TOMBOY timer, which can be set in the `TAC`
 *        register.
 */
typedef enum TOMBOY_TimerClockSpeed
{
    TOMBOY_TCS_4096_HZ = 0,  ///< @brief 4096 Hz, the timer increments every 1024 cycles.
    TOMBOY_TCS_262144_HZ,    ///< @brief 262144 Hz, the timer increments every 16 cycles.
    TOMBOY_TCS_65536_HZ,     ///< @brief 65536 Hz, the timer increments every 64 cycles.
    TOMBOY_TCS_16384_HZ,     ///< @brief 16384 Hz, the timer increments every 256 cycles.
    TOMBOY_TCS_SLOWEST = TOMBOY_TCS_4096_HZ,      ///< @brief The slowest clock speed.
    TOMBOY_TCS_FASTEST = TOMBOY_TCS_262144_HZ,    ///< @brief The fastest clock speed.
    TOMBOY_TCS_FAST = TOMBOY_TCS_65536_HZ,        ///< @brief A fast clock speed.
    TOMBOY_TCS_SLOW = TOMBOY_TCS_16384_HZ         ///< @brief A slow clock speed.
} TOMBOY_TimerClockSpeed;

// Timer Control Union /////////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the TOMBOY timer control (`TAC`) register.
 */
typedef union TOMBOY_TimerControl
{
    struct
    {
        uint8_t m_ClockSpeed : 2;  ///< @brief The clock speed select bits.
        uint8_t m_Enable : 1;      ///< @brief The timer enable bit.
        uint8_t : 5;               ///< @brief Unused bits.
    };
    uint8_t m_Register;           ///< @brief The raw register value.
} TOMBOY_TimerControl;

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new TOMBOY timer instance.
 * 
 * @return A pointer to the new TOMBOY timer instance.
 */
TOMBOY_Timer* TOMBOY_CreateTimer (TOMBOY_Engine* p_Engine);

/**
 * @brief Resets the given TOMBOY timer instance, resetting it and its components to their initial state.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to reset.
 */
void TOMBOY_ResetTimer (TOMBOY_Timer* p_Timer);

/**
 * @brief Destroys the given TOMBOY timer instance, freeing its resources.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to destroy.
 */
void TOMBOY_DestroyTimer (TOMBOY_Timer* p_Timer);

/**
 * @brief Ticks the given TOMBOY timer instance, updating its components and state.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to tick.
 * 
 * @return `true` if the timer was ticked successfully; `false` otherwise.
 */
bool TOMBOY_TickTimer (TOMBOY_Timer* p_Timer);

/**
 * @brief Tests the given bit of the given engine's timer's 16-bit divider register, to see if it
 *        has changed from high to low (from 1 to 0).
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to check.
 * @param p_Bit        The bit of the divider register to check.
 * 
 * @return `true` if the bit has transitioned from high to low; `false` otherwise
 */
bool TOMBOY_TestTimerDividerBit (const TOMBOY_Timer* p_Timer, uint8_t p_Bit);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief Reads the current engine timer's 8-bit divider register, `DIV`.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to read from.
 * 
 * @return The value of the `DIV` register.
 */
uint8_t TOMBOY_ReadDIV (const TOMBOY_Timer* p_Timer);

/**
 * @brief Reads the current engine timer's 8-bit timer counter register, `TIMA`.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to read from.
 * 
 * @return The value of the `TIMA` register.
 */
uint8_t TOMBOY_ReadTIMA (const TOMBOY_Timer* p_Timer);;

/**
 * @brief Reads the current engine timer's 8-bit timer modulo register, `TMA`.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to read from.
 * 
 * @return The value of the `TMA` register.
 */
uint8_t TOMBOY_ReadTMA (const TOMBOY_Timer* p_Timer);

/**
 * @brief Reads the current engine timer's 8-bit timer control register, `TAC`.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to read from.
 * 
 * @return The value of the `TAC` register.
 */
uint8_t TOMBOY_ReadTAC (const TOMBOY_Timer* p_Timer);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief Writes a value to the current engine timer's 8-bit divider register, `DIV`.
 * 
 * Any write to the `DIV` register will reset the timer's 16-bit divider register to zero.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to write to.
 * @param p_Value      The value to write to the `DIV` register.
 */
void TOMBOY_WriteDIV (TOMBOY_Timer* p_Timer, uint8_t p_Value);

/**
 * @brief Writes a value to the current engine timer's 8-bit timer counter register, `TIMA`.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to write to.
 * @param p_Value      The value to write to the `TIMA` register.
 */
void TOMBOY_WriteTIMA (TOMBOY_Timer* p_Timer, uint8_t p_Value);

/**
 * @brief Writes a value to the current engine timer's 8-bit timer modulo register, `TMA`.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to write to.
 * @param p_Value      The value to write to the `TMA` register.
 */
void TOMBOY_WriteTMA (TOMBOY_Timer* p_Timer, uint8_t p_Value);

/**
 * @brief Writes a value to the current engine timer's 8-bit timer control register, `TAC`.
 * 
 * @param p_Timer      A pointer to the TOMBOY timer instance to write to.
 * @param p_Value      The value to write to the `TAC` register.
 */
void TOMBOY_WriteTAC (TOMBOY_Timer* p_Timer, uint8_t p_Value);
