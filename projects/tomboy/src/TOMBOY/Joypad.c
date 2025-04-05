/**
 * @file TOMBOY/Joypad.c
 */

#include <TOMBOY/Engine.h>
#include <TOMBOY/Joypad.h>

// Joypad Component Structure //////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_Joypad
{
    TOMBOY_Engine*          m_ParentEngine;                 ///< @brief A pointer to the parent TOMBOY Engine instance.
    bool                    m_SelectedButtons;              ///< @brief `true` if the joypad buttons are mapped to the low nibble of the `JOYP` register; `false` if not.
    bool                    m_SelectedDirectionalPad;       ///< @brief `true` if the joypad directional pad is mapped to the low nibble of the `JOYP` register; `false` if not.
    bool                    m_States[8];                    ///< @brief The state of the joypad buttons and directional pad.
} TOMBOY_Joypad;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_Joypad* TOMBOY_CreateJoypad (TOMBOY_Engine* p_Engine)
{

    TOMBOY_Joypad* l_Joypad = TM_calloc(1, TOMBOY_Joypad);
    TM_pexpect(l_Joypad, "Failed to allocate memory for the joypad component");

    TOMBOY_ResetJoypad(l_Joypad);
    l_Joypad->m_ParentEngine = p_Engine;

    return l_Joypad;
}

void TOMBOY_ResetJoypad (TOMBOY_Joypad* p_Joypad)
{
    if (p_Joypad == NULL)
    {
        TM_error("Joypad component is NULL.");
        return;
    }

    // The `JOYP` register is reset to 0xCF (0b11001111).
    p_Joypad->m_SelectedButtons = true;
    p_Joypad->m_SelectedDirectionalPad = true;
    memset(p_Joypad->m_States, 0x00, sizeof(p_Joypad->m_States));
}

void TOMBOY_DestroyJoypad (TOMBOY_Joypad* p_Joypad)
{
    if (p_Joypad != NULL)
    {
        p_Joypad->m_ParentEngine = NULL;
        TM_free(p_Joypad);
    }
}

void TOMBOY_PressButton (TOMBOY_Joypad* p_Joypad, TOMBOY_JoypadButton p_Button)
{
    if (p_Joypad == NULL)
    {
        TM_error("Joypad component is NULL.");
        return;
    }

    // Check to see if the button being pressed is a directional pad button.
    bool l_IsDirectionalPadButton = (((p_Button & (1 << 2))) != 0);

    // Get the old state of the button.
    bool l_Old = p_Joypad->m_States[p_Button & 0b111];

    // Change the state of the button to pressed.
    p_Joypad->m_States[p_Button & 0b111] = true;

    // Clear the bit in the appropriate button state.
    if (l_IsDirectionalPadButton == true)
    {
        // If the button was not pressed before, and the DPAD group is selected, then trigger the joypad interrupt.
        if (p_Joypad->m_SelectedDirectionalPad == true && l_Old == false)
        {
            TOMBOY_RequestInterrupt(p_Joypad->m_ParentEngine, TOMBOY_IT_JOYPAD);
        }
    }
    else
    {
        // If the button was not pressed before, and the BUTTON group is selected, then trigger the joypad interrupt.
        if (p_Joypad->m_SelectedButtons == true && l_Old == false)
        {
            TOMBOY_RequestInterrupt(p_Joypad->m_ParentEngine, TOMBOY_IT_JOYPAD);
        }
    }
}

void TOMBOY_ReleaseButton (TOMBOY_Joypad* p_Joypad, TOMBOY_JoypadButton p_Button)
{
    // Set the button state to released.
    // Releasing a button does not trigger an interrupt.
    p_Joypad->m_States[p_Button & 0b111] = false;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

uint8_t TOMBOY_ReadJOYP (const TOMBOY_Joypad* p_Joypad)
{
    if (p_Joypad == NULL)
    {
        TM_error("Joypad component is NULL.");
        return 0xFF;
    }

    // Set up the value to be returned.
    uint8_t l_JOYP = 0xCF;

    // Get the watch state of the face buttons and directional pad.
    // - If the face buttons are selected, then clear bit 5; otherwise, set it.
    // - If the directional pad is selected, then clear bit 4; otherwise, set it.
    if (p_Joypad->m_SelectedButtons == true) { l_JOYP &= ~(1 << 5); }
    else { l_JOYP |= (1 << 5); }
    if (p_Joypad->m_SelectedDirectionalPad == true) { l_JOYP &= ~(1 << 4); }
    else { l_JOYP |= (1 << 4); }

    // If the face buttons are selected, then copy the face button state to bits 3-0.
    // Otherwise, copy the directional pad state to bits 3-0.
    if (p_Joypad->m_SelectedButtons == true)
    {
        if (p_Joypad->m_States[TOMBOY_JB_START] == true)
            l_JOYP &= ~(1 << 3);
        if (p_Joypad->m_States[TOMBOY_JB_SELECT] == true)
            l_JOYP &= ~(1 << 2);
        if (p_Joypad->m_States[TOMBOY_JB_B] == true)
            l_JOYP &= ~(1 << 1);
        if (p_Joypad->m_States[TOMBOY_JB_A] == true)
            l_JOYP &= ~(1 << 0);
    }
    else
    {
        if (p_Joypad->m_States[TOMBOY_JB_DOWN] == true)
            l_JOYP &= ~(1 << 3);
        if (p_Joypad->m_States[TOMBOY_JB_UP] == true)
            l_JOYP &= ~(1 << 2);
        if (p_Joypad->m_States[TOMBOY_JB_LEFT] == true)
            l_JOYP &= ~(1 << 1);
        if (p_Joypad->m_States[TOMBOY_JB_RIGHT] == true)
            l_JOYP &= ~(1 << 0);
    }

    return l_JOYP;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void TOMBOY_WriteJOYP (TOMBOY_Joypad* p_Joypad, uint8_t p_Value)
{
    if (p_Joypad == NULL)
    {
        TM_error("Joypad component is NULL.");
        return;
    }
    
    // The low-nibble of the JOYP register is read-only.

    // The high-nibble of the JOYP register is used to select the buttons or the directional pad.
    // - If bit 5 is clear, then the face buttons are selected.
    // - If bit 4 is clear, then the directional pad is selected.
    p_Joypad->m_SelectedButtons = ((p_Value & (1 << 5)) == 0);
    p_Joypad->m_SelectedDirectionalPad = ((p_Value & (1 << 4)) == 0);
}
