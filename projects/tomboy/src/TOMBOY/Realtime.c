/**
 * @file  TOMBOY/Realtime.c
 */

#include <TOMBOY/Engine.h>
#include <TOMBOY/Realtime.h>

// TOMBOY Realtime Structure ///////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_Realtime
{
    TOMBOY_Engine* m_ParentEngine;      ///< @brief A pointer to the real-time clock's parent engine instance.
    uint8_t        m_RTCS;              ///< @brief The real-time clock seconds register.
    uint8_t        m_RTCM;              ///< @brief The real-time clock minutes register.
    uint8_t        m_RTCH;              ///< @brief The real-time clock hours register.
    uint8_t        m_RTCDH;             ///< @brief The real-time clock day counter high byte register.
    uint8_t        m_RTCDL;             ///< @brief The real-time clock day counter low byte register.
} TOMBOY_Realtime;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_Realtime* TOMBOY_CreateRealtime (TOMBOY_Engine* p_Engine)
{
    // Validate the engine instance.
    if (p_Engine == NULL)
    {
        TM_error("Parent engine context is NULL!");
        return NULL;
    }

    // Allocate the TOMBOY Realtime instance.
    TOMBOY_Realtime* l_Realtime = TM_calloc(1, TOMBOY_Realtime);
    TM_pexpect(l_Realtime != NULL, "Failed to allocate TOMBOY Realtime");
    l_Realtime->m_ParentEngine = p_Engine;
    TOMBOY_ResetRealtime(l_Realtime);

    // Return the new realtime instance.
    return l_Realtime;
}

void TOMBOY_ResetRealtime (TOMBOY_Realtime* p_Realtime)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return;
    }

    // Get the current system time.
    time_t l_CurrentTime = time(NULL);
    struct tm* l_LocalTime = localtime(&l_CurrentTime);

    // Seed the random number generator with the current time.
    srand((uint32_t) l_CurrentTime);

    // Set the real-time clock registers to the current time.
    p_Realtime->m_RTCS = l_LocalTime->tm_sec;
    p_Realtime->m_RTCM = l_LocalTime->tm_min;
    p_Realtime->m_RTCH = l_LocalTime->tm_hour;
    p_Realtime->m_RTCDH = (l_LocalTime->tm_yday >> 8) & 0xFF;
    p_Realtime->m_RTCDL = l_LocalTime->tm_yday & 0xFF;
}

void TOMBOY_DestroyRealtime (TOMBOY_Realtime* p_Realtime)
{
    if (p_Realtime != NULL)
    {
        p_Realtime->m_ParentEngine = NULL; // Unset the parent engine.
        TM_free(p_Realtime);
    }
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

uint8_t TOMBOY_ReadRTCS (const TOMBOY_Realtime* p_Realtime)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return 0xFF;
    }

    // Return the RTCS register value.
    return p_Realtime->m_RTCS;
}

uint8_t TOMBOY_ReadRTCM (const TOMBOY_Realtime* p_Realtime)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return 0xFF;
    }

    // Return the RTCM register value.
    return p_Realtime->m_RTCM;
}

uint8_t TOMBOY_ReadRTCH (const TOMBOY_Realtime* p_Realtime)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return 0xFF;
    }

    // Return the RTCH register value.
    return p_Realtime->m_RTCH;
}

uint8_t TOMBOY_ReadRTCDH (const TOMBOY_Realtime* p_Realtime)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return 0xFF;
    }

    // Return the RTCDH register value.
    return p_Realtime->m_RTCDH;
}

uint8_t TOMBOY_ReadRTCDL (const TOMBOY_Realtime* p_Realtime)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return 0xFF;
    }

    // Return the RTCDL register value.
    return p_Realtime->m_RTCDL;
}

uint8_t TOMBOY_ReadRTCR (const TOMBOY_Realtime* p_Realtime)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return 0xFF;
    }

    // Return a random value for the RTCR register.
    return rand() % 256; // Random value between 0 and 255.
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void TOMBOY_WriteRTCL (TOMBOY_Realtime* p_Realtime, uint8_t p_Value)
{
    // Validate the realtime instance.
    if (p_Realtime == NULL)
    {
        TM_error("Realtime context is NULL!");
        return;
    }

    (void) p_Value; // Value is unused.

    // Store the old state of the real-time clock registers.
    uint8_t l_OldRTCS = p_Realtime->m_RTCS;
    uint8_t l_OldRTCM = p_Realtime->m_RTCM;
    uint8_t l_OldRTCH = p_Realtime->m_RTCH;
    uint8_t l_OldRTCDH = p_Realtime->m_RTCDH;
    uint8_t l_OldRTCDL = p_Realtime->m_RTCDL;

    // Get the current system time.
    time_t l_CurrentTime = time(NULL);
    struct tm* l_LocalTime = localtime(&l_CurrentTime);

    // Set the real-time clock registers to the current time.
    p_Realtime->m_RTCS = l_LocalTime->tm_sec;
    p_Realtime->m_RTCM = l_LocalTime->tm_min;
    p_Realtime->m_RTCH = l_LocalTime->tm_hour;
    p_Realtime->m_RTCDH = (l_LocalTime->tm_yday >> 8) & 0xFF;
    p_Realtime->m_RTCDL = l_LocalTime->tm_yday & 0xFF;

    // If any of the RTC registers have changed, then request the RTC interrupt.
    if (
        (p_Realtime->m_RTCS != l_OldRTCS) ||
        (p_Realtime->m_RTCM != l_OldRTCM) ||
        (p_Realtime->m_RTCH != l_OldRTCH) ||
        (p_Realtime->m_RTCDH != l_OldRTCDH) ||
        (p_Realtime->m_RTCDL != l_OldRTCDL)
    )
    {
        TOMBOY_RequestInterrupt(p_Realtime->m_ParentEngine, TOMBOY_IT_RTC);

        // Also, since the time has changed, re-seed the RNG.
        srand((uint32_t) l_CurrentTime);
    }
}
