/**
 * @file  TOMBOY/Engine.c
 */

#include <TOMBOY/Program.h>
#include <TOMBOY/Timer.h>
#include <TOMBOY/Realtime.h>
#include <TOMBOY/APU.h>
#include <TOMBOY/PPU.h>
#include <TOMBOY/Joypad.h>
#include <TOMBOY/Network.h>
#include <TOMBOY/RAM.h>
#include <TOMBOY/Engine.h>

// TOMBOY Emulator Engine Structure ////////////////////////////////////////////////////////////////

typedef struct TOMBOY_Engine
{
    TM_CPU*                 m_CPU;              ///< @brief The TOMBOY CPU instance.
    const TOMBOY_Program*   m_Program;          ///< @brief The loaded program ROM.
    TOMBOY_Timer*           m_Timer;            ///< @brief The TOMBOY timer instance.
    TOMBOY_Realtime*        m_Realtime;         ///< @brief The TOMBOY real-time clock instance.
    TOMBOY_APU*             m_APU;              ///< @brief The TOMBOY audio processing unit instance.
    TOMBOY_PPU*             m_PPU;              ///< @brief The TOMBOY pixel processing unit instance.
    TOMBOY_Joypad*          m_Joypad;           ///< @brief The TOMBOY joypad instance.
    TOMBOY_Network*         m_Network;          ///< @brief The TOMBOY network interface instance.
    TOMBOY_RAM*             m_RAM;              ///< @brief The TOMBOY RAM instance.
    uint64_t                m_Cycles;           ///< @brief The number of cycles elapsed on the engine.
    bool                    m_DoubleSpeed;      ///< @brief Whether the engine is in double-speed mode.
} TOMBOY_Engine;

/*
    Note: In the TOMBOY system, the `KEY1` prepare speed switch register does not require a subsequent
    call of the `STOP` instruction for the speed switch to take effect. Writing to the `KEY1` register
    causes the speed switch to happen immediately.
*/

// Static Members //////////////////////////////////////////////////////////////////////////////////

static TOMBOY_Engine* s_CurrentEngine = NULL;

// Private Function Prototypes /////////////////////////////////////////////////////////////////////

static uint8_t TOMBOY_BusRead (uint32_t p_Address);
static void TOMBOY_BusWrite (uint32_t p_Address, uint8_t p_Data);
static bool TOMBOY_Cycle (uint32_t p_Cycles);

// Private Functions ///////////////////////////////////////////////////////////////////////////////

uint8_t TOMBOY_BusRead (uint32_t p_Address)
{
    assert(s_CurrentEngine != NULL);

    // `0x80000000` - `0xBFFFFFFF`: Working RAM Space
    if (p_Address >= TOMBOY_WRAM_START && p_Address <= TOMBOY_WRAM_END)
    {
        return TOMBOY_ReadWRAMByte(s_CurrentEngine->m_RAM, p_Address - TOMBOY_WRAM_START);
    }

    // `0xC0000000` - `0xDFFCFFFF`: Static RAM Space
    if (p_Address >= TOMBOY_SRAM_START && p_Address <= TOMBOY_SRAM_END)
    {
        return TOMBOY_ReadSRAMByte(s_CurrentEngine->m_RAM, p_Address - TOMBOY_SRAM_START);
    }

    // `0xE0000000` - `0xFFFCFFFF`: Executable RAM Space
    if (p_Address >= TM_XRAM_BEGIN && p_Address <= TM_XRAM_END)
    {
        return TOMBOY_ReadXRAMByte(s_CurrentEngine->m_RAM, p_Address - TM_XRAM_BEGIN);
    }

    // `0xDFFD0000` - `0xDFFE7FFF`: Screen Buffer Space
    if (p_Address >= TOMBOY_SCREEN_START && p_Address <= TOMBOY_SCREEN_END)
    {
        return TOMBOY_ReadScreenByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_SCREEN_START);
    }

    // `0xDFFF0000` - `0xDFFF00FF`: Network Send RAM Space
    if (p_Address >= TOMBOY_NSEND_START && p_Address <= TOMBOY_NSEND_END)
    {
        return TOMBOY_ReadNetSendByte(s_CurrentEngine->m_Network, p_Address - TOMBOY_NSEND_START);
    }

    // `0xDFFF0100` - `0xDFFF01FF`: Network Receive RAM Space
    if (p_Address >= TOMBOY_NRECV_START && p_Address <= TOMBOY_NRECV_END)
    {
        return TOMBOY_ReadNetRecvByte(s_CurrentEngine->m_Network, p_Address - TOMBOY_NRECV_START);
    }

    // `0x00000000` - `0x7FFFFFFF`: ROM Space
    if (p_Address >= TM_ROM_BEGIN && p_Address <= TM_ROM_END)
    {
        return TOMBOY_ReadProgramByte(s_CurrentEngine->m_Program, p_Address);
    }

    // `0xDFFF8000` - `0xDFFF9FFF`: Video RAM Space
    if (p_Address >= TOMBOY_VRAM_START && p_Address <= TOMBOY_VRAM_END)
    {
        return TOMBOY_ReadVRAMByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_VRAM_START);
    }

    // `0xDFFFA000` - `0xDFFFA07F`: Color RAM Space
    if (p_Address >= TOMBOY_CRAM_START && p_Address <= TOMBOY_CRAM_END)
    {
        return TOMBOY_ReadCRAMByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_CRAM_START);
    }

    // `0xDFFFFE00` - `0xDFFFFE9F`: OAM Space
    if (p_Address >= TOMBOY_OAM_START && p_Address <= TOMBOY_OAM_END)
    {
        return TOMBOY_ReadOAMByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_OAM_START);
    }

    // `0xDFFFFF30` - `0xDFFFFF3F`: Wave RAM Space
    if (p_Address >= TOMBOY_WAVE_START && p_Address <= TOMBOY_WAVE_END)
    {
        return TOMBOY_ReadWaveByte(s_CurrentEngine->m_APU, p_Address - TOMBOY_WAVE_START);
    }

    // `0xFFFD0000` - `0xFFFDFFFF`: Data Stack Space
    if (p_Address >= TM_DSTACK_BEGIN && p_Address <= TM_DSTACK_END)
    {
        return TOMBOY_ReadDataStackByte(s_CurrentEngine->m_RAM, p_Address - TM_DSTACK_BEGIN);
    }

    // `0xFFFE0000` - `0xFFFEFFFF`: Call Stack Space
    if (p_Address >= TM_CSTACK_BEGIN && p_Address <= TM_CSTACK_END)
    {
        return TOMBOY_ReadDataStackByte(s_CurrentEngine->m_RAM, p_Address - TM_CSTACK_BEGIN);
    }

    // `0xFFFE8000` - `0xFFFEFFFF`: Quick RAM Space
    if (p_Address >= TM_QRAM_BEGIN && p_Address <= TM_QRAM_END)
    {
        return TOMBOY_ReadQRAMByte(s_CurrentEngine->m_RAM, p_Address - TM_QRAM_BEGIN);
    }

    // `0xFFFFFF00` - `0xFFFFFFFF`: IO Space
    switch (p_Address)
    {
        case TOMBOY_HP_JOYP:  return TOMBOY_ReadJOYP(s_CurrentEngine->m_Joypad);
        case TOMBOY_HP_NTC:   return TOMBOY_ReadNTC(s_CurrentEngine->m_Network);
        case TOMBOY_HP_DIV:   return TOMBOY_ReadDIV(s_CurrentEngine->m_Timer);
        case TOMBOY_HP_TIMA:  return TOMBOY_ReadTIMA(s_CurrentEngine->m_Timer);
        case TOMBOY_HP_TMA:   return TOMBOY_ReadTMA(s_CurrentEngine->m_Timer);
        case TOMBOY_HP_TAC:   return TOMBOY_ReadTAC(s_CurrentEngine->m_Timer);
        case TOMBOY_HP_RTCS:  return TOMBOY_ReadRTCS(s_CurrentEngine->m_Realtime);
        case TOMBOY_HP_RTCM:  return TOMBOY_ReadRTCM(s_CurrentEngine->m_Realtime);
        case TOMBOY_HP_RTCH:  return TOMBOY_ReadRTCH(s_CurrentEngine->m_Realtime);
        case TOMBOY_HP_RTCDH: return TOMBOY_ReadRTCDH(s_CurrentEngine->m_Realtime);
        case TOMBOY_HP_RTCDL: return TOMBOY_ReadRTCDL(s_CurrentEngine->m_Realtime);
        case TOMBOY_HP_RTCL:  return 0xFF; // Write-only register
        case TOMBOY_HP_RTCR:  return TOMBOY_ReadRTCR(s_CurrentEngine->m_Realtime);
        case TOMBOY_HP_IF:    return TM_GetInterruptFlags(s_CurrentEngine->m_CPU);
        case TOMBOY_HP_NR10:  return TOMBOY_ReadNR10(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR11:  return TOMBOY_ReadNR11(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR12:  return TOMBOY_ReadNR12(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR13:  return 0xFF; // Write-only register.
        case TOMBOY_HP_NR14:  return TOMBOY_ReadNR14(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR21:  return TOMBOY_ReadNR21(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR22:  return TOMBOY_ReadNR22(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR23:  return 0xFF; // Write-only register.
        case TOMBOY_HP_NR24:  return TOMBOY_ReadNR24(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR30:  return TOMBOY_ReadNR30(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR31:  return 0xFF; // Write-only register.
        case TOMBOY_HP_NR32:  return TOMBOY_ReadNR32(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR33:  return 0xFF; // Write-only register.
        case TOMBOY_HP_NR34:  return TOMBOY_ReadNR34(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR41:  return 0xFF; // Write-only register.
        case TOMBOY_HP_NR42:  return TOMBOY_ReadNR42(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR43:  return TOMBOY_ReadNR43(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR44:  return TOMBOY_ReadNR44(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR50:  return TOMBOY_ReadNR50(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR51:  return TOMBOY_ReadNR51(s_CurrentEngine->m_APU);
        case TOMBOY_HP_NR52:  return TOMBOY_ReadNR52(s_CurrentEngine->m_APU);
        case TOMBOY_HP_LCDC:  return TOMBOY_ReadLCDC(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_STAT:  return TOMBOY_ReadSTAT(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_SCY:   return TOMBOY_ReadSCY(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_SCX:   return TOMBOY_ReadSCX(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_LY:    return TOMBOY_ReadLY(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_LYC:   return TOMBOY_ReadLYC(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_DMA1:  return 0xFF; // Write-only register
        case TOMBOY_HP_DMA2:  return 0xFF; // Write-only register
        case TOMBOY_HP_DMA3:  return 0xFF; // Write-only register
        case TOMBOY_HP_DMA:   return TOMBOY_ReadDMA(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_BGP:   return TOMBOY_ReadBGP(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_OBP0:  return TOMBOY_ReadOBP0(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_OBP1:  return TOMBOY_ReadOBP1(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_WY:    return TOMBOY_ReadWY(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_WX:    return TOMBOY_ReadWX(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_KEY1:  return s_CurrentEngine->m_DoubleSpeed ? 0x01 : 0x00;
        case TOMBOY_HP_VBK:   return TOMBOY_ReadVBK(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_HDMA1: return 0xFF; // Write-only register
        case TOMBOY_HP_HDMA2: return 0xFF; // Write-only register
        case TOMBOY_HP_HDMA3: return 0xFF; // Write-only register
        case TOMBOY_HP_HDMA4: return 0xFF; // Write-only register
        case TOMBOY_HP_HDMA5: return 0xFF; // Write-only register
        case TOMBOY_HP_HDMA6: return 0xFF; // Write-only register
        case TOMBOY_HP_HDMA7: return TOMBOY_ReadHDMA7(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_BGPI:  return TOMBOY_ReadBGPI(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_BGPD:  return TOMBOY_ReadBGPD(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_OBPI:  return TOMBOY_ReadOBPI(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_OBPD:  return TOMBOY_ReadOBPD(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_OPRI:  return TOMBOY_ReadOPRI(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_GRPM:  return TOMBOY_ReadGRPM(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_VBP:   return TOMBOY_ReadVBP(s_CurrentEngine->m_PPU);
        case TOMBOY_HP_IE:    return TM_GetInterruptEnable(s_CurrentEngine->m_CPU);
        default:              return 0xFF; // Invalid address
    }
}

void TOMBOY_BusWrite (uint32_t p_Address, uint8_t p_Data)
{
    assert(s_CurrentEngine != NULL);

    // `0x80000000` - `0xBFFFFFFF`: Working RAM Space
    if (p_Address >= TOMBOY_WRAM_START && p_Address <= TOMBOY_WRAM_END)
    {
        TOMBOY_WriteWRAMByte(s_CurrentEngine->m_RAM, p_Address - TOMBOY_WRAM_START, p_Data);
        return;
    }

    // `0xC0000000` - `0xDFFCFFFF`: Static RAM Space
    if (p_Address >= TOMBOY_SRAM_START && p_Address <= TOMBOY_SRAM_END)
    {
        TOMBOY_WriteSRAMByte(s_CurrentEngine->m_RAM, p_Address - TOMBOY_SRAM_START, p_Data);
        return;
    }

    // `0xE0000000` - `0xFFFCFFFF`: Executable RAM Space
    if (p_Address >= TM_XRAM_BEGIN && p_Address <= TM_XRAM_END)
    {
        TOMBOY_WriteXRAMByte(s_CurrentEngine->m_RAM, p_Address - TM_XRAM_BEGIN, p_Data);
        return;
    }

    // `0xDFFD0000` - `0xDFFE7FFF`: Screen Buffer Space
    if (p_Address >= TOMBOY_SCREEN_START && p_Address <= TOMBOY_SCREEN_END)
    {
        TOMBOY_WriteScreenByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_SCREEN_START, p_Data);
        return;
    }

    // `0xDFFF0000` - `0xDFFF00FF`: Network Send RAM Space
    if (p_Address >= TOMBOY_NSEND_START && p_Address <= TOMBOY_NSEND_END)
    {
        TOMBOY_WriteNetSendByte(s_CurrentEngine->m_Network, p_Address - TOMBOY_NSEND_START, p_Data);
        return;
    }

    // `0xDFFF8000` - `0xDFFF9FFF`: Video RAM Space
    if (p_Address >= TOMBOY_VRAM_START && p_Address <= TOMBOY_VRAM_END)
    {
        TOMBOY_WriteVRAMByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_VRAM_START, p_Data);
        return;
    }

    // `0xDFFFA000` - `0xDFFFA07F`: Color RAM Space
    if (p_Address >= TOMBOY_CRAM_START && p_Address <= TOMBOY_CRAM_END)
    {
        TOMBOY_WriteCRAMByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_CRAM_START, p_Data);
        return;
    }

    // `0xDFFFFE00` - `0xDFFFFE9F`: OAM Space
    if (p_Address >= TOMBOY_OAM_START && p_Address <= TOMBOY_OAM_END)
    {
        TOMBOY_WriteOAMByte(s_CurrentEngine->m_PPU, p_Address - TOMBOY_OAM_START, p_Data);
        return;
    }

    // `0xDFFFFF30` - `0xDFFFFF3F`: Wave RAM Space
    if (p_Address >= TOMBOY_WAVE_START && p_Address <= TOMBOY_WAVE_END)
    {
        TOMBOY_WriteWaveByte(s_CurrentEngine->m_APU, p_Address - TOMBOY_WAVE_START, p_Data);
        return;
    }

    // `0xFFFD0000` - `0xFFFDFFFF`: Data Stack Space
    if (p_Address >= TM_DSTACK_BEGIN && p_Address <= TM_DSTACK_END)
    {
        TOMBOY_WriteDataStackByte(s_CurrentEngine->m_RAM, p_Address - TM_DSTACK_BEGIN, p_Data);
        return;
    }

    // `0xFFFE0000` - `0xFFFEFFFF`: Call Stack Space
    if (p_Address >= TM_CSTACK_BEGIN && p_Address <= TM_CSTACK_END)
    {
        TOMBOY_WriteDataStackByte(s_CurrentEngine->m_RAM, p_Address - TM_CSTACK_BEGIN, p_Data);
        return;
    }

    // `0xFFFE8000` - `0xFFFEFFFF`: Quick RAM Space
    if (p_Address >= TM_QRAM_BEGIN && p_Address <= TM_QRAM_END)
    {
        TOMBOY_WriteQRAMByte(s_CurrentEngine->m_RAM, p_Address - TM_QRAM_BEGIN, p_Data);
        return;
    }

    // `0xFFFFFF00` - `0xFFFFFFFF`: IO Space
    switch (p_Address)
    {
        case TOMBOY_HP_JOYP:  TOMBOY_WriteJOYP(s_CurrentEngine->m_Joypad, p_Data); break;
        case TOMBOY_HP_NTC:   TOMBOY_WriteNTC(s_CurrentEngine->m_Network, p_Data); break;
        case TOMBOY_HP_DIV:   TOMBOY_WriteDIV(s_CurrentEngine->m_Timer, p_Data); break;
        case TOMBOY_HP_TIMA:  TOMBOY_WriteTIMA(s_CurrentEngine->m_Timer, p_Data); break;
        case TOMBOY_HP_TMA:   TOMBOY_WriteTMA(s_CurrentEngine->m_Timer, p_Data); break;
        case TOMBOY_HP_TAC:   TOMBOY_WriteTAC(s_CurrentEngine->m_Timer, p_Data); break;
        case TOMBOY_HP_RTCS:  break; // Read-only register
        case TOMBOY_HP_RTCM:  break; // Read-only register
        case TOMBOY_HP_RTCH:  break; // Read-only register
        case TOMBOY_HP_RTCDH: break; // Read-only register
        case TOMBOY_HP_RTCDL: break; // Read-only register
        case TOMBOY_HP_RTCL:  TOMBOY_WriteRTCL(s_CurrentEngine->m_Realtime, p_Data); break;
        case TOMBOY_HP_RTCR:  break; // Read-only register
        case TOMBOY_HP_IF:    TM_SetInterruptFlags(s_CurrentEngine->m_CPU, p_Data); break;
        case TOMBOY_HP_NR10:  TOMBOY_WriteNR10(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR11:  TOMBOY_WriteNR11(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR12:  TOMBOY_WriteNR12(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR13:  TOMBOY_WriteNR13(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR14:  TOMBOY_WriteNR14(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR21:  TOMBOY_WriteNR21(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR22:  TOMBOY_WriteNR22(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR23:  TOMBOY_WriteNR23(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR24:  TOMBOY_WriteNR24(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR30:  TOMBOY_WriteNR30(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR31:  TOMBOY_WriteNR31(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR32:  TOMBOY_WriteNR32(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR33:  TOMBOY_WriteNR33(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR34:  TOMBOY_WriteNR34(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR41:  TOMBOY_WriteNR41(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR42:  TOMBOY_WriteNR42(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR43:  TOMBOY_WriteNR43(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR44:  TOMBOY_WriteNR44(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR50:  TOMBOY_WriteNR50(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR51:  TOMBOY_WriteNR51(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_NR52:  TOMBOY_WriteNR52(s_CurrentEngine->m_APU, p_Data); break;
        case TOMBOY_HP_LCDC:  TOMBOY_WriteLCDC(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_STAT:  TOMBOY_WriteSTAT(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_SCY:   TOMBOY_WriteSCY(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_SCX:   TOMBOY_WriteSCX(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_LY:    break; // Read-only register
        case TOMBOY_HP_LYC:   TOMBOY_WriteLYC(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_DMA1:  TOMBOY_WriteDMA1(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_DMA2:  TOMBOY_WriteDMA2(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_DMA3:  TOMBOY_WriteDMA3(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_DMA:   TOMBOY_WriteDMA(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_BGP:   TOMBOY_WriteBGP(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_OBP0:  TOMBOY_WriteOBP0(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_OBP1:  TOMBOY_WriteOBP1(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_WY:    TOMBOY_WriteWY(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_WX:    TOMBOY_WriteWX(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_KEY1:  s_CurrentEngine->m_DoubleSpeed = (p_Data > 0); break;
        case TOMBOY_HP_VBK:   TOMBOY_WriteVBK(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_HDMA1: TOMBOY_WriteHDMA1(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_HDMA2: TOMBOY_WriteHDMA2(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_HDMA3: TOMBOY_WriteHDMA3(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_HDMA4: TOMBOY_WriteHDMA4(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_HDMA5: TOMBOY_WriteHDMA5(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_HDMA6: TOMBOY_WriteHDMA6(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_HDMA7: TOMBOY_WriteHDMA7(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_BGPI:  TOMBOY_WriteBGPI(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_BGPD:  TOMBOY_WriteBGPD(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_OBPI:  TOMBOY_WriteOBPI(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_OBPD:  TOMBOY_WriteOBPD(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_OPRI:  TOMBOY_WriteOPRI(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_GRPM:  TOMBOY_WriteGRPM(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_VBP:   TOMBOY_WriteVBP(s_CurrentEngine->m_PPU, p_Data); break;
        case TOMBOY_HP_IE:    TM_SetInterruptEnable(s_CurrentEngine->m_CPU, p_Data); break;
        default:              break; // Invalid address
    }
}

bool TOMBOY_Cycle (uint32_t p_Cycles)
{
    assert(s_CurrentEngine != NULL);
    
    uint8_t l_TicksPerCycle             = (s_CurrentEngine->m_DoubleSpeed) ? 8 : 4;
    uint8_t l_AudioDividerTimerBit      = (s_CurrentEngine->m_DoubleSpeed) ? 12 : 13;
    uint8_t l_NetworkDividerTimerBit    = (s_CurrentEngine->m_DoubleSpeed) ? 14 : 15;
    uint8_t l_ODMATickFrequency          = (s_CurrentEngine->m_DoubleSpeed) ? 2 : 4;

    for (uint32_t l_MachineCycle = 0; l_MachineCycle < p_Cycles; ++l_MachineCycle)
    {
        for (uint8_t l_Tick = 0; l_Tick < l_TicksPerCycle; ++l_Tick)
        {
            s_CurrentEngine->m_Cycles++;

            TOMBOY_TickTimer(s_CurrentEngine->m_Timer);
            TOMBOY_TickAPU(s_CurrentEngine->m_APU, 
                TOMBOY_TestTimerDividerBit(s_CurrentEngine->m_Timer, l_AudioDividerTimerBit));
            TOMBOY_TickPPU(s_CurrentEngine->m_PPU,
                (s_CurrentEngine->m_Cycles % l_ODMATickFrequency) == 0);
            
            if (TOMBOY_TestTimerDividerBit(s_CurrentEngine->m_Timer, l_NetworkDividerTimerBit))
            {
                TOMBOY_TickNetwork(s_CurrentEngine->m_Network);
            }
        }
    }

    return true;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_Engine* TOMBOY_CreateEngine (const TOMBOY_Program* p_Program)
{
    if (p_Program == NULL)
    {
        TM_error("Cannot create engine with no program loaded.");
        return NULL;
    }

    // Allocate the TOMBOY Engine instance.
    TOMBOY_Engine* l_Engine = TM_calloc(1, TOMBOY_Engine);
    TM_pexpect(l_Engine != NULL, "Failed to allocate TOMBOY Engine");

    // Create the CPU instance.
    l_Engine->m_CPU = TM_CreateCPU(TOMBOY_BusRead, TOMBOY_BusWrite, TOMBOY_Cycle);
    TM_expect(l_Engine->m_CPU != NULL, "Failed to create TOMBOY CPU!");

    // Create the timer instance.
    l_Engine->m_Timer = TOMBOY_CreateTimer(l_Engine);
    TM_expect(l_Engine->m_Timer != NULL, "Failed to create TOMBOY Timer!");

    // Create the real-time clock instance.
    l_Engine->m_Realtime = TOMBOY_CreateRealtime(l_Engine);
    TM_expect(l_Engine->m_Realtime != NULL, "Failed to create TOMBOY Realtime!");

    // Create the APU instance.
    l_Engine->m_APU = TOMBOY_CreateAPU(l_Engine);
    TM_expect(l_Engine->m_APU != NULL, "Failed to create TOMBOY APU!");

    // Create the PPU instance.
    l_Engine->m_PPU = TOMBOY_CreatePPU(l_Engine);
    TM_expect(l_Engine->m_PPU != NULL, "Failed to create TOMBOY PPU!");

    // Create the joypad instance.
    l_Engine->m_Joypad = TOMBOY_CreateJoypad(l_Engine);
    TM_expect(l_Engine->m_Joypad != NULL, "Failed to create TOMBOY Joypad!");

    // Create the network instance.
    l_Engine->m_Network = TOMBOY_CreateNetwork(l_Engine);
    TM_expect(l_Engine->m_Network != NULL, "Failed to create TOMBOY Network!");

    // Get the expected WRAM and SRAM sizes from the program. Use them to create the RAM instance.
    uint32_t l_WRAMSize = TOMBOY_GetRequestedWRAMSize(p_Program);
    uint32_t l_SRAMSize = TOMBOY_GetRequestedSRAMSize(p_Program);
    uint32_t l_XRAMSize = TOMBOY_GetRequestedXRAMSize(p_Program);
    l_Engine->m_RAM = TOMBOY_CreateRAM(l_WRAMSize, l_SRAMSize, l_XRAMSize);
    TM_expect(l_Engine->m_RAM != NULL, "Failed to create TOMBOY RAM!");

    // Load the program into the engine.
    l_Engine->m_Program = p_Program;

    // If there is no current engine context set, then make this engine the current engine.
    if (TOMBOY_IsCurrentEngineSet() == false)
    {
        TOMBOY_MakeEngineCurrent(l_Engine);
    }

    // Return the new engine instance.
    return l_Engine;
}

void TOMBOY_ResetEngine (TOMBOY_Engine* p_Engine)
{
    // Validate the engine instance.
    TM_expect(p_Engine != NULL, "Engine context is NULL!");

    // Reset the CPU instance and its components.
    TM_ResetCPU(p_Engine->m_CPU);
    TOMBOY_ResetTimer(p_Engine->m_Timer);
    TOMBOY_ResetRealtime(p_Engine->m_Realtime);
    TOMBOY_ResetAPU(p_Engine->m_APU);
    p_Engine->m_Cycles = 0;
}

void TOMBOY_DestroyEngine (TOMBOY_Engine* p_Engine)
{
    if (p_Engine != NULL)
    {
        // If this engine is the current engine for shortforms, un-set it.
        if (TOMBOY_GetCurrentEngine() == p_Engine)
        {
            TOMBOY_MakeEngineCurrent(NULL);
        }

        // Destroy the CPU instance and its components.
        TOMBOY_DestroyRAM(p_Engine->m_RAM);
        TOMBOY_DestroyNetwork(p_Engine->m_Network);
        TOMBOY_DestroyJoypad(p_Engine->m_Joypad);
        TOMBOY_DestroyPPU(p_Engine->m_PPU);
        TOMBOY_DestroyAPU(p_Engine->m_APU);
        TOMBOY_DestroyRealtime(p_Engine->m_Realtime);
        TOMBOY_DestroyTimer(p_Engine->m_Timer);
        TM_DestroyCPU(p_Engine->m_CPU);

        // Free the engine instance.
        TM_free(p_Engine);
    }
}

void TOMBOY_MakeEngineCurrent (TOMBOY_Engine* p_Engine)
{
    s_CurrentEngine = p_Engine;
}

TOMBOY_Engine* TOMBOY_GetCurrentEngine ()
{
    return s_CurrentEngine;
}

bool TOMBOY_IsCurrentEngineSet ()
{
    return s_CurrentEngine != NULL;
}

bool TOMBOY_TickEngine ()
{
    if (s_CurrentEngine == NULL)
    {
        TM_error("No current engine set!");
        return false;
    }

    TM_StepCPU(s_CurrentEngine->m_CPU);

    // Check if the CPU is stopped.
    bool l_Stopped = TM_IsStopped(s_CurrentEngine->m_CPU);
    if (l_Stopped == true)
    {
        // If the CPU has been stopped, then report the error code before returning.
        TM_info("Program exited with code %u.", TM_GetErrorCode(s_CurrentEngine->m_CPU));
    }

    return l_Stopped == false;
}

uint64_t TOMBOY_GetCycleCount (const TOMBOY_Engine* p_Engine)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return 0;
    }

    return p_Engine->m_Cycles;
}

bool TOMBOY_RequestInterrupt (TOMBOY_Engine* p_Engine, uint8_t p_Interrupt)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return false;
    }

    // Request the interrupt on the CPU instance.
    TM_RequestInterrupt(p_Engine->m_CPU, p_Interrupt);

    return true;
}

TM_CPU* TOMBOY_GetCPU (TOMBOY_Engine* p_Engine)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return NULL;
    }

    return p_Engine->m_CPU;
}

TOMBOY_APU* TOMBOY_GetAPU (TOMBOY_Engine* p_Engine)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return NULL;
    }

    return p_Engine->m_APU;
}

TOMBOY_PPU* TOMBOY_GetPPU (TOMBOY_Engine* p_Engine)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return NULL;
    }

    return p_Engine->m_PPU;
}

TOMBOY_Joypad* TOMBOY_GetJoypad (TOMBOY_Engine* p_Engine)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return NULL;
    }

    return p_Engine->m_Joypad;
}

void TOMBOY_SetCallbacks (TOMBOY_Engine* p_Engine, TOMBOY_FrameRenderedCallback p_FrameCallback, TOMBOY_AudioMixCallback p_AudioCallback)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return;
    }

    TOMBOY_SetFrameRenderedCallback(p_Engine->m_PPU, p_FrameCallback);
    TOMBOY_SetAudioMixCallback(p_Engine->m_APU, p_AudioCallback);
}
