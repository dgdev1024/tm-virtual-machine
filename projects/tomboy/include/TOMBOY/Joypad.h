/**
 * @file     TOMBOY/Joypad.h
 * @brief    Contains the TOMBOY Engine's joypad component structure and public functions.
 * 
 * The TOMBOY Engine's joypad component simulates the Game Boy's internal joypad hardware. The joypad
 * component is responsible for reading the state of the Game Boy's buttons and directional pad, and
 * updating the joypad register in the memory map.
 * 
 * The joypad component is responsible for the following hardware register:
 * 
 * - `JOYP` or `P1` (Joypad Register) - The joypad register, which is used to read the state of the
 *   Game Boy buttons and/or directional pad. The bits of this register control the following settings:
 *       - Bit 0: A Button / Right Directional Pad. Read-only.
 *       - Bit 1: B Button / Left Directional Pad. Read-only.
 *       - Bit 2: Select Button / Up Directional Pad. Read-only.
 *       - Bit 3: Start Button / Down Directional Pad. Read-only.
 *       - Bit 4: Select D-Pad.
 *       - Bit 5: Select Buttons.
 *       - Bits 6-7: Not used.
 *       - Note that the bits of this register are inverted, with a bit set to 0 indicating that the
 *         corresponding button or directional pad is pressed.
 * 
 * The joypad component is responsible for the following hardware interrupt:
 * 
 * - `JOYPAD` (Joypad Interrupt) - The joypad interrupt, which is requested when a button or directional
 *   pad's pressed state changes, assuming that its group of buttons is being selected by the `JOYP`
 *   register. This interrupt is typically used to update the game's engine state in response to player
 *   input.
 */

 #pragma once
 #include <TOMBOY/Common.h>
 
 // Joypad Button Enumeration ///////////////////////////////////////////////////////////////////////
 
 /**
  * @brief Enumerates the eight buttons found on the Game Boy's joypad.
  */
 typedef enum TOMBOY_JoypadButton
 {
     TOMBOY_JB_A        = 0b000,            ///< @brief The A button.
     TOMBOY_JB_B        = 0b001,            ///< @brief The B button.
     TOMBOY_JB_SELECT   = 0b010,            ///< @brief The Select button.
     TOMBOY_JB_START    = 0b011,            ///< @brief The Start button.
     TOMBOY_JB_RIGHT    = 0b100,            ///< @brief The Right directional pad button.
     TOMBOY_JB_LEFT     = 0b101,            ///< @brief The Left directional pad button.
     TOMBOY_JB_UP       = 0b110,            ///< @brief The Up directional pad button.
     TOMBOY_JB_DOWN     = 0b111             ///< @brief The Down directional pad button.
 } TOMBOY_JoypadButton;
 
 // Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////
 
 /**
  * @brief A forward-declaration of the TOMBOY Engine structure.
  */
 typedef struct TOMBOY_Engine TOMBOY_Engine;
 
 /**
  * @brief A forward-declaration of the TOMBOY Engine joypad structure.
  */
 typedef struct TOMBOY_Joypad TOMBOY_Joypad;
 
 // Public Functions ////////////////////////////////////////////////////////////////////////////////
 
 /**
  * @brief      Creates a new instance of the TOMBOY's joypad component.
  * 
  * @param      p_Engine  A pointer to the joypad's parent TOMBOY emulator engine instance.
  * 
  * @return     A pointer to the newly created TOMBOY joypad component instance.
  */
 TOMBOY_Joypad* TOMBOY_CreateJoypad (TOMBOY_Engine* p_Engine);
 
 /**
  * @brief      Resets the given TOMBOY joypad instance.
  * 
  * @param      p_Joypad  A pointer to the TOMBOY joypad instance to reset.
  */
 void TOMBOY_ResetJoypad (TOMBOY_Joypad* p_Joypad);
 
 /**
  * @brief      Destroys the given TOMBOY joypad instance.
  * 
  * @param      p_Joypad  A pointer to the TOMBOY joypad instance to destroy.
  */
 void TOMBOY_DestroyJoypad (TOMBOY_Joypad* p_Joypad);
 
 /**
  * @brief      Presses a button on the given TOMBOY joypad instance.
  * 
  * @param      p_Joypad  A pointer to the TOMBOY joypad instance.
  * @param      p_Button  The button to press.
  */
 void TOMBOY_PressButton (TOMBOY_Joypad* p_Joypad, TOMBOY_JoypadButton p_Button);
 
 /**
  * @brief      Releases a button on the given TOMBOY joypad instance.
  * 
  * @param      p_Joypad  A pointer to the TOMBOY joypad instance.
  * @param      p_Button  The button to release.
  */
 void TOMBOY_ReleaseButton (TOMBOY_Joypad* p_Joypad, TOMBOY_JoypadButton p_Button);
 
 // Public Functions - Hardware Register Getters ////////////////////////////////////////////////////
 
 /**
  * @brief      Gets the value of the joypad state register, `JOYP`.
  * 
  * @param      p_Joypad  A pointer to the TOMBOY joypad instance.
  * 
  * @return     The value of the `JOYP` register.
  */
 uint8_t TOMBOY_ReadJOYP (const TOMBOY_Joypad* p_Joypad);
 
 // Public Functions - Hardware Register Setters ////////////////////////////////////////////////////
 
 /**
  * @brief      Sets the value of the joypad state register, `JOYP`.
  * 
  * @param      p_Joypad  A pointer to the TOMBOY Engine joypad instance.
  * @param      p_Value   The new value of the `JOYP` register.
  */
 void TOMBOY_WriteJOYP (TOMBOY_Joypad* p_Joypad, uint8_t p_Value);
 