/**
 * @file  TOMBOY/Common.h
 * @brief Contains commonly used definitions and includes for the TOMBOY emulation library.
 */

#pragma once
#include <TM/Common.h>

// Constants - TOMBOY Memory Map ///////////////////////////////////////////////////////////////////

#define TOMBOY_WRAM_START       0x80000000
#define TOMBOY_WRAM_END         0xBFFFFFFF
#define TOMBOY_WRAM_SIZE        (TOMBOY_WRAM_END - TOMBOY_WRAM_START + 1)
#define TOMBOY_SRAM_START       0xC0000000
#define TOMBOY_SRAM_END         0xDFFCFFFF
#define TOMBOY_SRAM_SIZE        (TOMBOY_SRAM_END - TOMBOY_SRAM_START + 1)
#define TOMBOY_SCREEN_START     0xDFFD0000
#define TOMBOY_SCREEN_END       0xDFFE7FFF
#define TOMBOY_NSEND_START      0xDFFF0000
#define TOMBOY_NSEND_END        0xDFFF00FF
#define TOMBOY_NRECV_START      0xDFFF0100
#define TOMBOY_NRECV_END        0xDFFF01FF
#define TOMBOY_VRAM_START       0xDFFF8000
#define TOMBOY_TDATA_START      0xDFFF8000
#define TOMBOY_TDATA0_START     0xDFFF8000
#define TOMBOY_TDATA1_START     0xDFFF8800
#define TOMBOY_TDATA2_START     0xDFFF9000
#define TOMBOY_TDATA_END        0xDFFF97FF
#define TOMBOY_SCRN_START       0xDFFF9800
#define TOMBOY_SCRN0_START      0xDFFF9800
#define TOMBOY_SCRN1_START      0xDFFF9C00
#define TOMBOY_SCRN_END         0xDFFF9FFF
#define TOMBOY_VRAM_END         0xDFFF9FFF
#define TOMBOY_CRAM_START       0xDFFFA000
#define TOMBOY_CRAM0_START      0xDFFFA000
#define TOMBOY_CRAM1_START      0xDFFFA040
#define TOMBOY_CRAM_END         0xDFFFA07F
#define TOMBOY_OAM_START        0xDFFFFE00
#define TOMBOY_OAM_END          0xDFFFFE9F
#define TOMBOY_WAVE_START       0xDFFFFF30
#define TOMBOY_WAVE_END         0xDFFFFF3F

// Interrupt Type Enumeration //////////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the possible interrupt types for the TOMBOY emulator.
 */
typedef enum TOMBOY_InterruptType
{
    TOMBOY_IT_VBLANK = 0,   ///< @brief V-Blank Interrupt
    TOMBOY_IT_LCDSTAT,      ///< @brief LCD STAT Interrupt
    TOMBOY_IT_TIMER,        ///< @brief Timer Interrupt
    TOMBOY_IT_NET,          ///< @brief Network Transfer Interrupt
    TOMBOY_IT_JOYPAD,       ///< @brief Joypad Interrupt
    TOMBOY_IT_RTC,          ///< @brief Real-Time Clock Interrupt
} TOMBOY_InterruptType;

// Hardware Port Register Enumeration //////////////////////////////////////////////////////////////

typedef enum TOMBOY_HardwarePort
{
    TOMBOY_HP_JOYP      = 0xFFFFFF00,       ///< @brief `JOYP` / `P1` - Joypad Register
    TOMBOY_HP_P1        = TOMBOY_HP_JOYP,   ///< @brief `P1` - Joypad Register (alias for `JOYP`)
    TOMBOY_HP_NTC,                          ///< @brief `NTC` - Network Transfer Control
    TOMBOY_HP_DIV,                          ///< @brief `DIV` - Timer Divider Register
    TOMBOY_HP_TIMA,                         ///< @brief `TIMA` - Timer Counter Register
    TOMBOY_HP_TMA,                          ///< @brief `TMA` - Timer Modulo Register
    TOMBOY_HP_TAC,                          ///< @brief `TAC` - Timer Control Register
    TOMBOY_HP_RTCS,                         ///< @brief `RTCS` - Real-Time Clock Seconds Register
    TOMBOY_HP_RTCM,                         ///< @brief `RTCM` - Real-Time Clock Minutes Register
    TOMBOY_HP_RTCH,                         ///< @brief `RTCH` - Real-Time Clock Hours Register
    TOMBOY_HP_RTCDH,                        ///< @brief `RTCDH` - Real-Time Clock Day/Hour Register
    TOMBOY_HP_RTCDL,                        ///< @brief `RTCDL` - Real-Time Clock Day/Minute Register
    TOMBOY_HP_RTCL,                         ///< @brief `RTCL` - Real-Time Clock Day/Second Register
    TOMBOY_HP_RTCR,                         ///< @brief `RTCR` - RTC-seeded Random Number Register
    TOMBOY_HP_IF        = 0xFFFFFF0F,       ///< @brief `IF` - Interrupt Flag Register
    TOMBOY_HP_NR10,                         ///< @brief `NR10` - Sound Channel 1 Sweep Register
    TOMBOY_HP_NR11,                         ///< @brief `NR11` - Sound Channel 1 Sound Length/Wave Pattern Duty Register
    TOMBOY_HP_NR12,                         ///< @brief `NR12` - Sound Channel 1 Volume Envelope Register
    TOMBOY_HP_NR13,                         ///< @brief `NR13` - Sound Channel 1 Frequency Low Register
    TOMBOY_HP_NR14,                         ///< @brief `NR14` - Sound Channel 1 Frequency High/Register Control Register
    TOMBOY_HP_NR21,                         ///< @brief `NR21` - Sound Channel 2 Sound Length/Wave Pattern Duty Register
    TOMBOY_HP_NR22,                         ///< @brief `NR22` - Sound Channel 2 Volume Envelope Register
    TOMBOY_HP_NR23,                         ///< @brief `NR23` - Sound Channel 2 Frequency Low Register
    TOMBOY_HP_NR24,                         ///< @brief `NR24` - Sound Channel 2 Frequency High/Register Control Register
    TOMBOY_HP_NR30,                         ///< @brief `NR30` - Sound Channel 3 Sound On/Off Register  
    TOMBOY_HP_NR31,                         ///< @brief `NR31` - Sound Channel 3 Sound Length Register  
    TOMBOY_HP_NR32,                         ///< @brief `NR32` - Sound Channel 3 Select Output Level Register
    TOMBOY_HP_NR33,                         ///< @brief `NR33` - Sound Channel 3 Frequency Low Register
    TOMBOY_HP_NR34,                         ///< @brief `NR34` - Sound Channel 3 Frequency High/Register Control Register
    TOMBOY_HP_NR41,                         ///< @brief `NR41` - Sound Channel 4 Sound Length Register
    TOMBOY_HP_NR42,                         ///< @brief `NR42` - Sound Channel 4 Volume Envelope Register
    TOMBOY_HP_NR43,                         ///< @brief `NR43` - Sound Channel 4 Polynomial Counter Register
    TOMBOY_HP_NR44,                         ///< @brief `NR44` - Sound Channel 4 Frequency High/Register Control Register
    TOMBOY_HP_NR50,                         ///< @brief `NR50` - Channel Control/Volume Register
    TOMBOY_HP_NR51,                         ///< @brief `NR51` - Sound Output Terminal Register
    TOMBOY_HP_NR52,                         ///< @brief `NR52` - Sound On/Off Register
    TOMBOY_HP_LCDC,                         ///< @brief `LCDC` - LCD Control Register
    TOMBOY_HP_STAT,                         ///< @brief `STAT` - LCD Status Register
    TOMBOY_HP_SCY,                          ///< @brief `SCY` - Scroll Y Register
    TOMBOY_HP_SCX,                          ///< @brief `SCX` - Scroll X Register
    TOMBOY_HP_LY,                           ///< @brief `LY` - Current Line Register
    TOMBOY_HP_LYC,                          ///< @brief `LYC` - LY Compare Register
    TOMBOY_HP_DMA1,                         ///< @brief `DMA3` - DMA Start Address Byte 3
    TOMBOY_HP_DMA2,                         ///< @brief `DMA2` - DMA Start Address Byte 2
    TOMBOY_HP_DMA3,                         ///< @brief `DMA1` - DMA Start Address Byte 1
    TOMBOY_HP_DMA,                          ///< @brief `DMA` - DMA Transfer Initiation Register
    TOMBOY_HP_BGP,                          ///< @brief `BGP` - Background Palette Data Register
    TOMBOY_HP_OBP0,                         ///< @brief `OBP0` - Object Palette 0 Data Register
    TOMBOY_HP_OBP1,                         ///< @brief `OBP1` - Object Palette 1 Data Register
    TOMBOY_HP_WY,                           ///< @brief `WY` - Window Y Position Register
    TOMBOY_HP_WX,                           ///< @brief `WX` - Window X Position Register
    TOMBOY_HP_KEY1,                         ///< @brief `KEY1` - Speed Register
    TOMBOY_HP_VBK,                          ///< @brief `VBK` - Video RAM Bank Register
    TOMBOY_HP_HDMA1,                        ///< @brief `HDMA1` - HDMA Source Address Byte 3
    TOMBOY_HP_HDMA2,                        ///< @brief `HDMA2` - HDMA Source Address Byte 2
    TOMBOY_HP_HDMA3,                        ///< @brief `HDMA3` - HDMA Source Address Byte 1
    TOMBOY_HP_HDMA4,                        ///< @brief `HDMA4` - HDMA Source Address Byte 0
    TOMBOY_HP_HDMA5,                        ///< @brief `HDMA5` - HDMA Destination Address Byte 1
    TOMBOY_HP_HDMA6,                        ///< @brief `HDMA6` - HDMA Destination Address Byte 0
    TOMBOY_HP_HDMA7,                        ///< @brief `HDMA7` - HDMA Transfer Length/Mode Register
    TOMBOY_HP_BGPI,                         ///< @brief `BGPI` - Background Palette Index Register
    TOMBOY_HP_BGPD,                         ///< @brief `BGPD` - Background Palette Data Register
    TOMBOY_HP_OBPI,                         ///< @brief `OBPI` - Object Palette Index Register
    TOMBOY_HP_OBPD,                         ///< @brief `OBPD` - Object Palette Data Register
    TOMBOY_HP_OPRI,                         ///< @brief `OPRI` - Object Priority Register
    TOMBOY_HP_GRPM,                         ///< @brief `GRPM` - Graphics Mode Register
    TOMBOY_HP_PCM12,                        ///< @brief `PCM12` - PCM Channel 1/2 Output Register
    TOMBOY_HP_PCM34,                        ///< @brief `PCM34` - PCM Channel 3/4 Output Register
    TOMBOY_HP_IE = 0xFFFFFFFF               ///< @brief `IE` - Interrupt Enable Register
} TOMBOY_HardwarePort;