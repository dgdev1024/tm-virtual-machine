/**
 * @file  TOMBOY/Timer.c
 */

#include <TOMBOY/Engine.h>
#include <TOMBOY/Timer.h>

// TOMBOY Timer Structure //////////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_Timer
{
    TOMBOY_Engine*      m_ParentEngine;  ///< @brief A pointer to the timer's parent engine instance.
    uint16_t            m_OldDIV;        ///< @brief The old value of the 16-bit timer divider register.
    uint16_t            m_DIV;           ///< @brief The 16-bit timer divider register.
    uint8_t             m_TIMA;          ///< @brief The 8-bit timer counter register.
    uint8_t             m_TMA;           ///< @brief The 8-bit timer modulo register.
    TOMBOY_TimerControl m_TAC;           ///< @brief The 8-bit timer control register.
} TOMBOY_Timer;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_Timer* TOMBOY_CreateTimer (TOMBOY_Engine* p_Engine)
{
    // Validate the engine instance.
    if (p_Engine == NULL)
    {
        TM_error("Parent engine context is NULL!");
        return NULL;
    }

    // Allocate the TOMBOY Timer instance.
    TOMBOY_Timer* l_Timer = TM_calloc(1, TOMBOY_Timer);
    TM_pexpect(l_Timer != NULL, "Failed to allocate TOMBOY Timer");
    l_Timer->m_ParentEngine = p_Engine;
    TOMBOY_ResetTimer(l_Timer);

    // Return the new timer instance.
    return l_Timer;
}

void TOMBOY_ResetTimer (TOMBOY_Timer* p_Timer)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return;
    }

    // Reset the timer's properties.
    p_Timer->m_OldDIV = 0;
    p_Timer->m_DIV = 0;
    p_Timer->m_TIMA = 0;
    p_Timer->m_TMA = 0;
    p_Timer->m_TAC.m_Register = 0xF8; // 0xF8 = 0b11111000
}

void TOMBOY_DestroyTimer (TOMBOY_Timer* p_Timer)
{
    if (p_Timer != NULL)
    {
        p_Timer->m_ParentEngine = NULL; // Unset the parent engine.

        // Free the timer instance.
        TM_free(p_Timer);
    }
}

bool TOMBOY_TickTimer (TOMBOY_Timer* p_Timer)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return false;
    }

    // Update the old DIV register value.
    p_Timer->m_OldDIV = p_Timer->m_DIV;

    // Increment the DIV register by 1.
    p_Timer->m_DIV++;
    
    // Check if the timer is enabled.
    if (p_Timer->m_TAC.m_Enable == false)
    {
        return true;
    }

    // Depending on the timer's clock speed, determine which divider bit needs to be checked.
    uint8_t l_Bit = 0;
    switch (p_Timer->m_TAC.m_ClockSpeed)
    {
        case TOMBOY_TCS_4096_HZ:     l_Bit = 9; break;
        case TOMBOY_TCS_262144_HZ:   l_Bit = 3; break;
        case TOMBOY_TCS_65536_HZ:    l_Bit = 5; break;
        case TOMBOY_TCS_16384_HZ:    l_Bit = 7; break;
    }

    // Check if the divider bit has transitioned from high to low.
    bool l_TimerNeedsTick = TOMBOY_TestTimerDividerBit(p_Timer, l_Bit);
    if (l_TimerNeedsTick == true && ++p_Timer->m_TIMA == 0)
    {
        // Reset the TIMA register to the TMA register value.
        p_Timer->m_TIMA = p_Timer->m_TMA;

        // Request a timer interrupt.
        TOMBOY_RequestInterrupt(p_Timer->m_ParentEngine, TOMBOY_IT_TIMER);
    }

    return true;
}

bool TOMBOY_TestTimerDividerBit (const TOMBOY_Timer* p_Timer, uint8_t p_Bit)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return false;
    }

    // Check if the specified bit has transitioned from high to low.
    bool l_OldBit = ((p_Timer->m_OldDIV >> p_Bit) & 0x01);
    bool l_NewBit = ((p_Timer->m_DIV >> p_Bit) & 0x01);
    return (l_OldBit == true && l_NewBit == false);
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

uint8_t TOMBOY_ReadDIV (const TOMBOY_Timer* p_Timer)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return 0xFF;
    }

    // Return the DIV register value.
    return (p_Timer->m_DIV >> 8) & 0xFF;
}

uint8_t TOMBOY_ReadTIMA (const TOMBOY_Timer* p_Timer)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return 0xFF;
    }

    // Return the TIMA register value.
    return p_Timer->m_TIMA;
}

uint8_t TOMBOY_ReadTMA (const TOMBOY_Timer* p_Timer)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return 0xFF;
    }

    // Return the TMA register value.
    return p_Timer->m_TMA;
}

uint8_t TOMBOY_ReadTAC (const TOMBOY_Timer* p_Timer)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return 0xFF;
    }

    // Return the TAC register value.
    return p_Timer->m_TAC.m_Register;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void TOMBOY_WriteDIV (TOMBOY_Timer* p_Timer, uint8_t p_Value)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return;
    }

    (void) p_Value; // Unused.

    // Any write to the DIV register resets it to zero.
    p_Timer->m_DIV = 0;
}

void TOMBOY_WriteTIMA (TOMBOY_Timer* p_Timer, uint8_t p_Value)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return;
    }

    // Set the TIMA register value.
    p_Timer->m_TIMA = p_Value;
}

void TOMBOY_WriteTMA (TOMBOY_Timer* p_Timer, uint8_t p_Value)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return;
    }

    // Set the TMA register value.
    p_Timer->m_TMA = p_Value;
}

void TOMBOY_WriteTAC (TOMBOY_Timer* p_Timer, uint8_t p_Value)
{
    // Validate the timer instance.
    if (p_Timer == NULL)
    {
        TM_error("Timer context is NULL!");
        return;
    }

    // Set the TAC register value.
    p_Timer->m_TAC.m_Register = p_Value;
}
