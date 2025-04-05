/**
 * @file  TOMBOY/APU.c
 */

#include <TOMBOY/Engine.h>
#include <TOMBOY/APU.h>

// Private Constants ///////////////////////////////////////////////////////////////////////////////

static const uint8_t TOMBOY_WAVE_DUTY_PATTERNS[4] = {
    [TOMBOY_PDC_12_5]    = 0b00000001,
    [TOMBOY_PDC_25]      = 0b00000011,
    [TOMBOY_PDC_50]      = 0b00001111,
    [TOMBOY_PDC_75]      = 0b00111111
};

// Audio Channel Structures ////////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_PulseChannel
{

    // Hardware Registers
    TOMBOY_PulseFrequencySweep       m_FrequencySweep;           ///< @brief The frequency sweep unit (`PC1` only).
    TOMBOY_PulseLengthDuty           m_LengthDuty;               ///< @brief The length timer and duty cycle control.
    TOMBOY_VolumeEnvelope            m_VolumeEnvelope;           ///< @brief The volume and envelope control.
    TOMBOY_PeriodLowByte             m_PeriodLow;                ///< @brief The period low byte.
    TOMBOY_PeriodHighControl         m_PeriodHighControl;        ///< @brief The period high bits and control control.

    // Internal Registers
    uint16_t                         m_CurrentPeriod;            ///< @brief The current period of the channel.
    uint16_t                         m_PeriodDivider;            ///< @brief The period divider.
    uint8_t                          m_CurrentLengthTimer;       ///< @brief The current length timer.
    uint8_t                          m_CurrentVolume;            ///< @brief The current volume.
    uint8_t                          m_CurrentWavePointer;       ///< @brief Points to the current wave sample in the current duty cycle pattern.
    uint8_t                          m_CurrentFrequencyTicks;    ///< @brief The frequency sweep unit's current tick counter (`PC1` only).
    uint8_t                          m_CurrentEnvelopeTicks;     ///< @brief The envelope sweep unit's current tick counter.

    // Digital-to-Analog Converter (DAC)
    bool                             m_DACEnabled;               ///< @brief The channel's DAC enable flag.
    uint8_t                          m_DACInput;                 ///< @brief The DAC's digital input.
    float                            m_DACOutput;                ///< @brief The DAC's analog output.

} TOMBOY_PulseChannel;

typedef struct TOMBOY_WaveChannel
{
    
    // Hardware Registers
    TOMBOY_WaveDACEnable             m_DACEnable;                    ///< @brief The wave DAC enable control.
    TOMBOY_WaveOutputLevelControl    m_OutputLevel;                  ///< @brief The wave output level control.
    TOMBOY_WaveLengthTimer           m_LengthTimer;                  ///< @brief The wave length timer control.
    TOMBOY_PeriodLowByte             m_PeriodLow;                    ///< @brief The period low byte.
    TOMBOY_PeriodHighControl         m_PeriodHighControl;            ///< @brief The period high bits and control unit.

    // Wave Memory Buffer
    uint8_t                          m_WaveRAM[TOMBOY_WAVE_RAM_SIZE]; ///< @brief The wave channel's wave RAM buffer.

    // Internal Registers
    uint16_t                         m_CurrentPeriod;                ///< @brief The current period of the channel.
    uint16_t                         m_PeriodDivider;                ///< @brief The period divider.
    uint8_t                          m_CurrentLengthTimer;           ///< @brief The current length timer.
    uint8_t                          m_CurrentSampleIndex;           ///< @brief Points to the current 4-bit sample in the wave RAM buffer.

    // Digital-to-Analog Converter (DAC) - The wave channel's DAC enable flag is stored in the `m_DACEnable` register.
    uint8_t                          m_DACInput;                     ///< @brief The DAC's digital input.
    float                            m_DACOutput;                    ///< @brief The DAC's analog output.

} TOMBOY_WaveChannel;

typedef struct TOMBOY_NoiseChannel
{

    // Hardware Registers
    TOMBOY_NoiseLengthTimer          m_LengthTimer;                  ///< @brief The noise length timer control.
    TOMBOY_VolumeEnvelope            m_VolumeEnvelope;               ///< @brief The volume and envelope control.
    TOMBOY_NoiseFrequencyRandomness  m_FrequencyRandomness;          ///< @brief The noise channel's LFSR control.
    TOMBOY_NoiseControl              m_Control;                      ///< @brief The noise channel's control register.

    // Internal Registers
    uint16_t                         m_LFSR;                         ///< @brief The noise channel's linear feedback shift register.
    uint8_t                          m_CurrentLengthTimer;           ///< @brief The current length timer.
    uint8_t                          m_CurrentVolume;                ///< @brief The current volume.
    uint8_t                          m_CurrentEnvelopeTicks;         ///< @brief The envelope sweep unit's current tick counter.
    uint64_t                         m_CurrentClockFrequency;        ///< @brief The current clock frequency of the noise channel.

    // Digital-to-Analog Converter (DAC)
    bool                            m_DACEnabled;                   ///< @brief The channel's DAC enable flag.
    uint8_t                         m_DACInput;                     ///< @brief The DAC's digital input.
    float                           m_DACOutput;                    ///< @brief The DAC's analog output.

} TOMBOY_NoiseChannel;

// Audio Processing Unit (APU) Structure ///////////////////////////////////////////////////////////

typedef struct TOMBOY_APU
{

    // Parent Engine
    TOMBOY_Engine*                   m_ParentEngine;                  ///< @brief The parent engine of the APU.

    // Control Registers
    TOMBOY_AudioMasterControl        m_MasterControl;                ///< @brief The audio master control register.
    TOMBOY_SoundPanning              m_SoundPanning;                 ///< @brief The sound panning control register.
    TOMBOY_MasterVolumeControl       m_MasterVolumeControl;          ///< @brief The master volume control and VIN panning register.

    // Audio Channels
    TOMBOY_PulseChannel              m_PulseChannel1;                ///< @brief The first pulse channel.
    TOMBOY_PulseChannel              m_PulseChannel2;                ///< @brief The second pulse channel.
    TOMBOY_WaveChannel               m_WaveChannel;                  ///< @brief The wave channel.
    TOMBOY_NoiseChannel              m_NoiseChannel;                 ///< @brief The noise channel.

    // Audio Sample Buffer
    TOMBOY_AudioSample               m_AudioSample;                  ///< @brief The current audio sample mixed by the APU.

    // Mix Handler and State
    TOMBOY_AudioMixCallback          m_MixCallback;                  ///< @brief The audio mix callback function.
    float                            m_PreviousLeftInput;            ///< @brief The previous left speaker input.
    float                            m_PreviousRightInput;           ///< @brief The previous right speaker input.
    float                            m_PreviousLeftOutput;           ///< @brief The previous left speaker output.
    float                            m_PreviousRightOutput;          ///< @brief The previous right speaker output.

    // Internal Registers
    uint16_t                         m_Divider;                      ///< @brief The APU's internal divider.
    uint64_t                         m_MixClockFrequency;            ///< @brief How often should the mix callback be called.

} TOMBOY_APU;

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static void TOMBOY_TriggerChannelInternal (TOMBOY_APU* p_APU, TOMBOY_AudioChannel p_Channel);
static uint8_t TOMBOY_ReadWaveNibble (const TOMBOY_APU* p_APU, uint8_t p_Index);
static void TOMBOY_WriteWaveNibble (TOMBOY_APU* p_APU, uint8_t p_Index, uint8_t p_Value);
static void TOMBOY_TickPulseChannels (TOMBOY_APU* p_APU);
static void TOMBOY_TickWaveChannel (TOMBOY_APU* p_APU);
static void TOMBOY_TickNoiseChannel (TOMBOY_APU* p_APU);
static void TOMBOY_TickLengthTimers (TOMBOY_APU* p_APU);
static void TOMBOY_TickFrequencySweep (TOMBOY_APU* p_APU);
static void TOMBOY_TickEnvelopeSweeps (TOMBOY_APU* p_APU);
static void TOMBOY_UpdateAudioSample (TOMBOY_Engine* p_Engine, TOMBOY_APU* p_APU);

// Static Functions ////////////////////////////////////////////////////////////////////////////////

void TOMBOY_TriggerChannelInternal (TOMBOY_APU* p_APU, TOMBOY_AudioChannel p_Channel)
{
    // Channels can only be triggered if the APU is enabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    switch (p_Channel)
    {
        case TOMBOY_AC_PULSE_1:
        {
            TOMBOY_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthDuty.m_InitialLength;
            p_Channel->m_CurrentVolume = p_Channel->m_VolumeEnvelope.m_InitialVolume;
            p_Channel->m_CurrentPeriod = (p_Channel->m_PeriodHighControl.m_PeriodHigh << 8) | p_Channel->m_PeriodLow.m_PeriodLow;
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            p_Channel->m_CurrentWavePointer = 0;
            p_Channel->m_CurrentFrequencyTicks = 0;
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_PC1Enable = p_Channel->m_DACEnabled;

        } break;

        case TOMBOY_AC_PULSE_2:
        {
            TOMBOY_PulseChannel* p_Channel = &p_APU->m_PulseChannel2;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthDuty.m_InitialLength;
            p_Channel->m_CurrentVolume = p_Channel->m_VolumeEnvelope.m_InitialVolume;
            p_Channel->m_CurrentPeriod = (p_Channel->m_PeriodHighControl.m_PeriodHigh << 8) | p_Channel->m_PeriodLow.m_PeriodLow;
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            p_Channel->m_CurrentWavePointer = 0;
            p_Channel->m_CurrentFrequencyTicks = 0;
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_PC2Enable = p_Channel->m_DACEnabled;

        } break;

        case TOMBOY_AC_WAVE:
        {
            TOMBOY_WaveChannel* p_Channel = &p_APU->m_WaveChannel;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthTimer.m_InitialLength;
            p_Channel->m_CurrentPeriod = (p_Channel->m_PeriodHighControl.m_PeriodHigh << 8) | p_Channel->m_PeriodLow.m_PeriodLow;
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            p_Channel->m_CurrentSampleIndex = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_WCEnable = p_Channel->m_DACEnable.m_DACPower;

        } break;

        case TOMBOY_AC_NOISE:
        {
            TOMBOY_NoiseChannel* p_Channel = &p_APU->m_NoiseChannel;

            // Trigger the channel
            p_Channel->m_CurrentLengthTimer = p_Channel->m_LengthTimer.m_InitialLength;
            p_Channel->m_CurrentVolume = p_Channel->m_VolumeEnvelope.m_InitialVolume;
            p_Channel->m_LFSR = 0;
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // Enable the channel only if its DAC is enabled.
            p_APU->m_MasterControl.m_NCEnable = p_Channel->m_DACEnabled;

        } break;
    }
}

uint8_t TOMBOY_ReadWaveNibble (const TOMBOY_APU* p_APU, uint8_t p_Index)
{
    // Get the byte index and the nibble index.
    uint8_t l_ByteIndex = p_Index / 2;
    uint8_t l_NibbleIndex = p_Index % 2;

    // Get the byte from the wave RAM buffer.
    uint8_t l_Byte = p_APU->m_WaveChannel.m_WaveRAM[l_ByteIndex];

    // Return the nibble from the byte.
    return (l_NibbleIndex == 0) ? ((l_Byte >> 4) & 0xF) : (l_Byte & 0xF);
}

void TOMBOY_WriteWaveNibble (TOMBOY_APU* p_APU, uint8_t p_Index, uint8_t p_Value)
{
    // Get the byte index and the nibble index.
    uint8_t l_ByteIndex = p_Index / 2;
    uint8_t l_NibbleIndex = p_Index % 2;

    // Get the byte from the wave RAM buffer.
    uint8_t l_Byte = p_APU->m_WaveChannel.m_WaveRAM[l_ByteIndex];

    // Set the nibble in the byte.
    if (l_NibbleIndex == 0)
    {
        l_Byte &= 0x0F;
        l_Byte |= (p_Value << 4);
    }
    else
    {
        l_Byte &= 0xF0;
        l_Byte |= p_Value;
    }

    // Write the byte back to the wave RAM buffer.
    p_APU->m_WaveChannel.m_WaveRAM[l_ByteIndex] = l_Byte;
}

void TOMBOY_TickPulseChannels (TOMBOY_APU* p_APU)
{

    // Point to PC1.
    TOMBOY_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // If the channel is enabled, then increment its period divider. When that value exceeds 2047 (0x7FF),
    // the divider overflows and is then reset to the channel's period.
    if (p_APU->m_MasterControl.m_PC1Enable && ++p_Channel->m_PeriodDivider > 0x800)
    {

        // Reset the period divider to the channel's current period. The difference between this
        // value and the overflow value (0x800) determines the channel's frequency.
        p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;

        // Advance the wave pointer by one place.
        p_Channel->m_CurrentWavePointer = (p_Channel->m_CurrentWavePointer + 1) & 0b111;

        // The channel's DAC input value is set to the pointed-to bit in the current duty cycle pattern...
        p_Channel->m_DACInput = (
            TOMBOY_WAVE_DUTY_PATTERNS[p_Channel->m_LengthDuty.m_DutyCycle] 
                >> p_Channel->m_CurrentWavePointer
        ) & 0b1;

        // ...then multiplied by the channel's current volume.
        p_Channel->m_DACInput *= p_Channel->m_CurrentVolume;

        // The channel's DAC input is then translated to an analog DAC output value, by...
        // - Dividing the input by 7.5, then subtracting 1.0.
        p_Channel->m_DACOutput = -(((float) p_Channel->m_DACInput / 7.5f) - 1.0f);

    }

    // Repeat the above process for PC2.
    p_Channel = &p_APU->m_PulseChannel2;
    if (p_APU->m_MasterControl.m_PC2Enable && ++p_Channel->m_PeriodDivider > 0x800)
    {
        p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
        p_Channel->m_CurrentWavePointer = (p_Channel->m_CurrentWavePointer + 1) & 0b111;
        p_Channel->m_DACInput = (
            TOMBOY_WAVE_DUTY_PATTERNS[p_Channel->m_LengthDuty.m_DutyCycle] 
                >> p_Channel->m_CurrentWavePointer
        ) & 0b1;
        p_Channel->m_DACInput *= p_Channel->m_CurrentVolume;
        p_Channel->m_DACOutput = -(((float) p_Channel->m_DACInput / 7.5f) - 1.0f);
    }

}

void TOMBOY_TickWaveChannel (TOMBOY_APU* p_APU)
{

    // Point to the wave channel.
    TOMBOY_WaveChannel* p_Channel = &p_APU->m_WaveChannel;

    // If the channel is enabled, then increment its period divider. When that value exceeds 2047 (0x7FF),
    // the divider overflows and is then reset to the channel's period.
    if (p_APU->m_MasterControl.m_WCEnable && ++p_Channel->m_PeriodDivider > 0x800)
    {

        // Reset the period divider to the channel's current period. The difference between this
        // value and the overflow value (0x800) determines the channel's frequency.
        p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
        
        // Advance the sample index by one place.
        p_Channel->m_CurrentSampleIndex = (p_Channel->m_CurrentSampleIndex + 1) % TOMBOY_WAVE_RAM_NIBBLES;

        // The channel's DAC input value is set to the pointed-to 4-bit sample in the wave RAM buffer...
        p_Channel->m_DACInput = TOMBOY_ReadWaveNibble(p_APU, p_Channel->m_CurrentSampleIndex);

        // ...then affected by the channel's wave output level.
        switch (p_Channel->m_OutputLevel.m_OutputLevel)
        {
            case TOMBOY_WOL_MUTE:
                p_Channel->m_DACInput = 0;
                break;
            case TOMBOY_WOL_FULL:
                break;
            case TOMBOY_WOL_HALF:
                p_Channel->m_DACInput >>= 1;
                break;
            case TOMBOY_WOL_QUARTER:
                p_Channel->m_DACInput >>= 2;
                break;
        }

        // The channel's DAC input is then translated to an analog DAC output value, by...
        // - Dividing the input by 7.5, then subtracting 1.0.
        p_Channel->m_DACOutput = -(((float) p_Channel->m_DACInput / 7.5f) - 1.0f);

    }

}

void TOMBOY_TickNoiseChannel (TOMBOY_APU* p_APU)
{

    // Point to the noise channel.
    TOMBOY_NoiseChannel* p_Channel = &p_APU->m_NoiseChannel;

    // If the channel is enabled, then update the channel's LFSR.
    if (p_APU->m_MasterControl.m_NCEnable)
    {

        // Get bits 0 and 1 of the LFSR.
        uint8_t bit0 = (p_Channel->m_LFSR >> 0) & 0b1;
        uint8_t bit1 = (p_Channel->m_LFSR >> 1) & 0b1;

        // Determine the new value of bit 15 (and 7 if the LFSR width is set).
        uint8_t bit15 = (bit0 == bit1) ? 1 : 0;

        // Set the new value of bit 15 (and 7 if the LFSR width is set).
        p_Channel->m_LFSR |= (bit15 << 15);
        if (p_Channel->m_FrequencyRandomness.m_LFSRWidth)
        {
            p_Channel->m_LFSR |= (bit15 << 7);
        }

        // Shift the LFSR right by one bit, then clear the new value of bit 15 (and 7 if the LFSR width is set).
        p_Channel->m_LFSR >>= 1;
        p_Channel->m_LFSR &= ~(1 << 15);
        if (p_Channel->m_FrequencyRandomness.m_LFSRWidth)
        {
            p_Channel->m_LFSR &= ~(1 << 7);
        }

        // The new value of bit 0 determines the DAC input value, which is then multiplied by the channel's current volume.
        p_Channel->m_DACInput = (p_Channel->m_LFSR & 0b1) * p_Channel->m_CurrentVolume;

        // The channel's DAC input is then translated to an analog DAC output value, by...
        // - Dividing the input by 7.5, then subtracting 1.0.
        p_Channel->m_DACOutput = -(((float) p_Channel->m_DACInput / 7.5f) - 1.0f);

    }

}

void TOMBOY_TickLengthTimers (TOMBOY_APU* p_APU)
{

    // Point to PC1.
    TOMBOY_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // If the channel and its length timer are enabled, then increment the length timer.
    if (p_APU->m_MasterControl.m_PC1Enable && p_Channel->m_CurrentLengthTimer)
    {
        
        // Increment the length timer. If it reaches `0b111111`, then the channel is disabled.
        if (++p_Channel->m_CurrentLengthTimer >= 0b111111)
        {
            p_APU->m_MasterControl.m_PC1Enable = false;
        }

    }

    // Repeat the above process for PC2.
    p_Channel = &p_APU->m_PulseChannel2;
    if (p_APU->m_MasterControl.m_PC2Enable && p_Channel->m_CurrentLengthTimer)
    {
        if (++p_Channel->m_CurrentLengthTimer >= 0b111111)
        {
            p_APU->m_MasterControl.m_PC2Enable = false;
        }
    }

    // Repeat the above process for WC. For the wave channel, the length timer threshold is
    // `0b11111111` (255).
    TOMBOY_WaveChannel* p_WaveChannel = &p_APU->m_WaveChannel;
    if (p_APU->m_MasterControl.m_WCEnable && p_WaveChannel->m_CurrentLengthTimer)
    {
        if (++p_WaveChannel->m_CurrentLengthTimer == 0b11111111)
        {
            p_APU->m_MasterControl.m_WCEnable = false;
        }
    }

    // Repeat the above process for NC.
    TOMBOY_NoiseChannel* p_NoiseChannel = &p_APU->m_NoiseChannel;
    if (p_APU->m_MasterControl.m_NCEnable && p_NoiseChannel->m_CurrentLengthTimer)
    {
        if (++p_NoiseChannel->m_CurrentLengthTimer >= 0b111111)
        {
            p_APU->m_MasterControl.m_NCEnable = false;
        }
    }

}

void TOMBOY_TickFrequencySweep (TOMBOY_APU* p_APU)
{

    // Point to PC1.
    TOMBOY_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // Check to see if the channel is enabled, and the step setting in its frequency sweep unit is
    // non-zero.
    if (p_APU->m_MasterControl.m_PC1Enable && p_Channel->m_FrequencySweep.m_IndividualStep)
    {

        // Figure out how much to shift the period divider by.
        uint16_t l_PeriodDelta = (p_Channel->m_CurrentPeriod >> p_Channel->m_FrequencySweep.m_IndividualStep);

        // If the frequency sweep unit is sweeping the frequency upwards, and increasing the period
        // by the above-calculated delta would exceed 2047 (0x7FF), then the channel is disabled,
        // instead. Early out in this case, too.
        if (
            p_Channel->m_FrequencySweep.m_Direction == TOMBOY_FSD_INCREASE &&
            p_Channel->m_CurrentPeriod + l_PeriodDelta > 0x7FF
        )
        {
            p_APU->m_MasterControl.m_PC1Enable = false;
            return;
        }

        // Incremeht the frequency sweep unit's tick counter. Once it reaches the sweep pace value,
        // then update the frequency sweep unit.
        if (++p_Channel->m_CurrentFrequencyTicks >= p_Channel->m_FrequencySweep.m_SweepPace)
        {
            
            // Reset the frequency sweep unit's tick counter.
            p_Channel->m_CurrentFrequencyTicks = 0;

            // If the frequency sweep unit is sweeping the frequency upwards, then increase the period
            // by the delta value.
            if (p_Channel->m_FrequencySweep.m_Direction == TOMBOY_FSD_INCREASE)
            {
                p_Channel->m_CurrentPeriod += l_PeriodDelta;
            }

            // If the frequency sweep unit is sweeping the frequency downwards, then decrease the period
            // by the delta value.
            else
            {
                p_Channel->m_CurrentPeriod -= l_PeriodDelta;
            }

            // Update the period divider with the new period value.
            p_Channel->m_PeriodDivider = p_Channel->m_CurrentPeriod;
            
        }

    }

}

void TOMBOY_TickEnvelopeSweeps (TOMBOY_APU* p_APU)
{

    // Point to `PC1`.
    TOMBOY_PulseChannel* p_Channel = &p_APU->m_PulseChannel1;

    // If the channel is enabled, and the envelope sweep unit's pace is non-zero, then increment the
    // envelope sweep unit's tick counter.
    if (p_APU->m_MasterControl.m_PC1Enable && p_Channel->m_VolumeEnvelope.m_SweepPace > 0)
    {

        // Once the tick counter reaches the sweep pace value, then update the envelope sweep unit.
        if (++p_Channel->m_CurrentEnvelopeTicks >= p_Channel->m_VolumeEnvelope.m_SweepPace)
        {

            // Reset the envelope sweep unit's tick counter.
            p_Channel->m_CurrentEnvelopeTicks = 0;

            // If the envelope sweep unit is increasing the volume, and the volume is less than 15,
            // then increment the volume.
            if (
                p_Channel->m_VolumeEnvelope.m_Direction == TOMBOY_ESD_INCREASE &&
                p_Channel->m_CurrentVolume < 0xF
            )
            {
                p_Channel->m_CurrentVolume++;
            }

            // If the envelope sweep unit is decreasing the volume, and the volume is greater than 0,
            // then decrement the volume.
            else if (
                p_Channel->m_VolumeEnvelope.m_Direction == TOMBOY_ESD_DECREASE &&
                p_Channel->m_CurrentVolume > 0
            )
            {
                p_Channel->m_CurrentVolume--;
            }

        }

    }

    // Repeat the above process for `PC2`.
    p_Channel = &p_APU->m_PulseChannel2;
    if (p_APU->m_MasterControl.m_PC2Enable && p_Channel->m_VolumeEnvelope.m_SweepPace > 0)
    {
        if (++p_Channel->m_CurrentEnvelopeTicks >= p_Channel->m_VolumeEnvelope.m_SweepPace)
        {
            if (
                p_Channel->m_VolumeEnvelope.m_Direction == TOMBOY_ESD_INCREASE &&
                p_Channel->m_CurrentVolume < 0xF
            )
            {
                p_Channel->m_CurrentVolume++;
            }
            else if (
                p_Channel->m_VolumeEnvelope.m_Direction == TOMBOY_ESD_DECREASE &&
                p_Channel->m_CurrentVolume > 0
            )
            {
                p_Channel->m_CurrentVolume--;
            }
        }
    }

    // Repeat the above process for `NC`.
    TOMBOY_NoiseChannel* p_NoiseChannel = &p_APU->m_NoiseChannel;
    if (p_APU->m_MasterControl.m_NCEnable && p_NoiseChannel->m_VolumeEnvelope.m_SweepPace > 0)
    {
        if (++p_NoiseChannel->m_CurrentEnvelopeTicks >= p_NoiseChannel->m_VolumeEnvelope.m_SweepPace)
        {
            if (
                p_NoiseChannel->m_VolumeEnvelope.m_Direction == TOMBOY_ESD_INCREASE &&
                p_NoiseChannel->m_CurrentVolume < 0xF
            )
            {
                p_NoiseChannel->m_CurrentVolume++;
            }
            else if (
                p_NoiseChannel->m_VolumeEnvelope.m_Direction == TOMBOY_ESD_DECREASE &&
                p_NoiseChannel->m_CurrentVolume > 0
            )
            {
                p_NoiseChannel->m_CurrentVolume--;
            }
        }
    }

}

void TOMBOY_UpdateAudioSample (TOMBOY_Engine* p_Engine, TOMBOY_APU* p_APU)
{

    // Reset the audio sample.
    p_APU->m_AudioSample.m_Left = 0.0f;
    p_APU->m_AudioSample.m_Right = 0.0f;

    // For each channel, if the channel and its DAC are enabled, then add the channel's DAC output
    // to the audio sample.
    if (p_APU->m_MasterControl.m_PC1Enable && p_APU->m_PulseChannel1.m_DACEnabled)
    {
        if (p_APU->m_SoundPanning.m_PC1Left)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_PulseChannel1.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_PC1Right)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_PulseChannel1.m_DACOutput;
        }
    }

    if (p_APU->m_MasterControl.m_PC2Enable && p_APU->m_PulseChannel2.m_DACEnabled)
    {
        if (p_APU->m_SoundPanning.m_PC2Left)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_PulseChannel2.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_PC2Right)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_PulseChannel2.m_DACOutput;
        }
    }

    if (p_APU->m_MasterControl.m_WCEnable && p_APU->m_WaveChannel.m_DACEnable.m_DACPower)
    {
        if (p_APU->m_SoundPanning.m_WCLeft)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_WaveChannel.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_WCRight)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_WaveChannel.m_DACOutput;
        }
    }

    if (p_APU->m_MasterControl.m_NCEnable && p_APU->m_NoiseChannel.m_DACEnabled)
    {
        if (p_APU->m_SoundPanning.m_NCLeft)
        {
            p_APU->m_AudioSample.m_Left += p_APU->m_NoiseChannel.m_DACOutput;
        }

        if (p_APU->m_SoundPanning.m_NCRight)
        {
            p_APU->m_AudioSample.m_Right += p_APU->m_NoiseChannel.m_DACOutput;
        }
    }

    // Affect the total DAC output by the master volume control.
    p_APU->m_AudioSample.m_Left *= p_APU->m_MasterVolumeControl.m_LeftVolume / 7.5f;
    p_APU->m_AudioSample.m_Right *= p_APU->m_MasterVolumeControl.m_RightVolume / 7.5f;

    // Apply a high-pass filter to the audio sample to remove DC offset.
    // The filter coefficient alpha is chosen to match the Game Boy APU hardware's behavior.
    static const float L_ALPHA = 0.999958f; // Example value, adjust as needed

    float l_NewLeftOutput = p_APU->m_AudioSample.m_Left - p_APU->m_PreviousLeftInput + L_ALPHA * p_APU->m_PreviousLeftOutput;
    float l_NewRightOutput = p_APU->m_AudioSample.m_Right - p_APU->m_PreviousRightInput + L_ALPHA * p_APU->m_PreviousRightOutput;

    p_APU->m_PreviousLeftInput = p_APU->m_AudioSample.m_Left;
    p_APU->m_PreviousRightInput = p_APU->m_AudioSample.m_Right;

    p_APU->m_AudioSample.m_Left = l_NewLeftOutput;
    p_APU->m_AudioSample.m_Right = l_NewRightOutput;

    p_APU->m_PreviousLeftOutput = l_NewLeftOutput;
    p_APU->m_PreviousRightOutput = l_NewRightOutput;

    // Divide the output by the number of channels mixed to bring the overall output between
    // -1.0 and 1.0.
    p_APU->m_AudioSample.m_Left /= 4.0f;
    p_APU->m_AudioSample.m_Right /= 4.0f;

    // If the mix callback is set, then call it with the audio sample.
    if (p_APU->m_MixCallback != NULL)
    {
        p_APU->m_MixCallback(&p_APU->m_AudioSample);
    }

}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_APU* TOMBOY_CreateAPU (TOMBOY_Engine* p_Engine)
{
    if (p_Engine == NULL)
    {
        TM_error("Engine context is NULL!");
        return NULL;
    }

    // Allocate memory for the APU context.
    TOMBOY_APU* l_APU = TM_calloc(1, TOMBOY_APU);
    TM_pexpect(l_APU != NULL, "Failed to allocate memory for APU context");

    // Initialize the APU context by resetting it.
    TOMBOY_ResetAPU(l_APU);
    l_APU->m_ParentEngine = p_Engine;

    return l_APU;
}

void TOMBOY_ResetAPU (TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // Reset the APU structure's memory.

    memset(p_APU, 0, sizeof(TOMBOY_APU));

    // Reset the APU's hardware registers.
    //
    // First, the primary control registers:
    /* NR52 = 0xF1 */   p_APU->m_MasterControl.m_Register = 0xF1;
    /* NR51 = 0xF3 */   p_APU->m_SoundPanning.m_Register = 0xF3;
    /* NR50 = 0x77 */   p_APU->m_MasterVolumeControl.m_Register = 0x77;

    // Next, the pulse channel 1 registers:
    /* NR10 = 0x80 */   p_APU->m_PulseChannel1.m_FrequencySweep.m_Register = 0x80;
    /* NR11 = 0xBF */   p_APU->m_PulseChannel1.m_LengthDuty.m_Register = 0xBF;
    /* NR12 = 0xF3 */   p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register = 0xF3;
    /* NR13 = 0xFF */   p_APU->m_PulseChannel1.m_PeriodLow.m_Register = 0xFF;
    /* NR14 = 0xBF */   p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register = 0xBF;

    // Next, the pulse channel 2 registers:
    /* NR21 = 0x3F */   p_APU->m_PulseChannel2.m_LengthDuty.m_Register = 0x3F;
    /* NR22 = 0x00 */   p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register = 0x00;
    /* NR23 = 0xFF */   p_APU->m_PulseChannel2.m_PeriodLow.m_Register = 0xFF;
    /* NR24 = 0xBF */   p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register = 0xBF;

    // Next, the wave channel registers:
    /* NR30 = 0x7F */   p_APU->m_WaveChannel.m_DACEnable.m_Register = 0x7F;
    /* NR31 = 0xFF */   p_APU->m_WaveChannel.m_LengthTimer.m_Register = 0xFF;
    /* NR32 = 0x9F */   p_APU->m_WaveChannel.m_OutputLevel.m_Register = 0x9F;
    /* NR33 = 0xFF */   p_APU->m_WaveChannel.m_PeriodLow.m_Register = 0xFF;
    /* NR34 = 0xBF */   p_APU->m_WaveChannel.m_PeriodHighControl.m_Register = 0xBF;

    // Next, the noise channel registers:
    /* NR41 = 0xFF */   p_APU->m_NoiseChannel.m_LengthTimer.m_Register = 0xFF;
    /* NR42 = 0x00 */   p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register = 0x00;
    /* NR43 = 0x00 */   p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register = 0x00;
    /* NR44 = 0xBF */   p_APU->m_NoiseChannel.m_Control.m_Register = 0xBF;

    // Reset the APU's mix clock frequency and the noise channel's current clock frequency.
    p_APU->m_MixClockFrequency = (4194304 / TOMBOY_AUDIO_SAMPLE_RATE);
    if (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider == 0)
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (0.5 * pow(2, p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift));
    }
    else
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider 
                << p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift);
    }
}

void TOMBOY_DestroyAPU (TOMBOY_APU* p_APU)
{
    if (p_APU != NULL)
    {
        // Unset the parent engine.
        p_APU->m_ParentEngine = NULL;

        // Free the APU context.
        TM_free(p_APU);
    }
}

void TOMBOY_SetAudioMixCallback (TOMBOY_APU* p_APU, TOMBOY_AudioMixCallback p_Callback)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // Set the audio mix callback function.
    p_APU->m_MixCallback = p_Callback;
}

const TOMBOY_AudioSample* TOMBOY_GetLatestAudioSample (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return NULL;
    }

    // Return the latest audio sample.
    return &p_APU->m_AudioSample;
}

bool TOMBOY_TickAPU (TOMBOY_APU* p_APU, bool p_DIV)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return false;
    }

    // Get the parent engine's cycle count.
    uint64_t l_Cycles = TOMBOY_GetCycleCount(p_APU->m_ParentEngine);

    // Tick the channels:
    // - Wave channel every two ticks.
    // - Pulse channels every four ticks.
    // - Noise channel at a rate of ticks dictated by the channel's clock frequency.
    if (l_Cycles % 2 == 0) 
        { TOMBOY_TickWaveChannel(p_APU); }
    if (l_Cycles % 4 == 0) 
        { TOMBOY_TickPulseChannels(p_APU); }
    if (l_Cycles % p_APU->m_NoiseChannel.m_CurrentClockFrequency == 0) 
        { TOMBOY_TickNoiseChannel(p_APU); }

    // If the APU's internal divider needs to be ticked, then do so here.
    if (p_DIV == true)
    {
        // Increment the APU's internal divider.
        p_APU->m_Divider++;

        // Tick...
        // - ...the length timers every 2 DIV-APU ticks.
        // . ...`PC1`'s frequency sweep unit every 4 DIV-APU ticks.
        // - ...the envelope sweeps every 8 DIV-APU ticks.
        if (p_APU->m_Divider % 2 == 0) 
            { TOMBOY_TickLengthTimers(p_APU); }
        if (p_APU->m_Divider % 4 == 0) 
            { TOMBOY_TickFrequencySweep(p_APU); }
        if (p_APU->m_Divider % 8 == 0) 
            { TOMBOY_TickEnvelopeSweeps(p_APU); }
    }

    // If the APU's mix clock frequency has been reached, then update the audio sample.
    if (l_Cycles % p_APU->m_MixClockFrequency == 0)
    {
        TOMBOY_UpdateAudioSample(p_APU->m_ParentEngine, p_APU);
    }

    return true;
}

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

uint8_t TOMBOY_ReadWaveByte (const TOMBOY_APU* p_APU, uint32_t p_Address)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    else if (p_Address >= TOMBOY_WAVE_RAM_SIZE)
    {
        TM_error("Wave RAM relative read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    // Read the value from the wave RAM buffer.
    return p_APU->m_WaveChannel.m_WaveRAM[p_Address];
}

void TOMBOY_WriteWaveByte (TOMBOY_APU* p_APU, uint32_t p_Address, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }
    else if (p_Address >= TOMBOY_WAVE_RAM_SIZE)
    {
        TM_error("Wave RAM relative write address $%08X is out of bounds.", p_Address);
        return;
    }

    // Write the value to the wave RAM buffer.
    p_APU->m_WaveChannel.m_WaveRAM[p_Address] = p_Value;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

uint8_t TOMBOY_ReadNR52 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_MasterControl.m_Register;
}

uint8_t TOMBOY_ReadNR51 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_SoundPanning.m_Register;
}

uint8_t TOMBOY_ReadNR50 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_MasterVolumeControl.m_Register;
}

uint8_t TOMBOY_ReadNR10 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_PulseChannel1.m_FrequencySweep.m_Register;
}

uint8_t TOMBOY_ReadNR11 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_PulseChannel1.m_LengthDuty.m_Register & 0b11000000;
}

uint8_t TOMBOY_ReadNR12 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register;
}

uint8_t TOMBOY_ReadNR14 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register;
}

uint8_t TOMBOY_ReadNR21 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_PulseChannel2.m_LengthDuty.m_Register & 0b11000000;
}

uint8_t TOMBOY_ReadNR22 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register;
}

uint8_t TOMBOY_ReadNR24 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register;
}

uint8_t TOMBOY_ReadNR30 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_WaveChannel.m_DACEnable.m_Register;
}

uint8_t TOMBOY_ReadNR32 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_WaveChannel.m_OutputLevel.m_Register;
}

uint8_t TOMBOY_ReadNR34 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_WaveChannel.m_PeriodHighControl.m_Register;
}

uint8_t TOMBOY_ReadNR42 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register;
}

uint8_t TOMBOY_ReadNR43 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register;
}

uint8_t TOMBOY_ReadNR44 (const TOMBOY_APU* p_APU)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return 0xFF;
    }
    
    return p_APU->m_NoiseChannel.m_Control.m_Register & 0b01111111;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void TOMBOY_WriteNR52 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }
    p_APU->m_MasterControl.m_Register |= (p_Value & 0b11110000);

    // If the APU is disabled, then reset all hardware registers, except for `NR52`, and make
    // them read-only.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        p_APU->m_SoundPanning.m_Register = 0x00;
        p_APU->m_MasterVolumeControl.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_FrequencySweep.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_LengthDuty.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register = 0x00;
        p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register = 0x00;
        p_APU->m_PulseChannel2.m_LengthDuty.m_Register = 0x00;
        p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register = 0x00;
        p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register = 0x00;
        p_APU->m_WaveChannel.m_DACEnable.m_Register = 0x00;
        p_APU->m_WaveChannel.m_OutputLevel.m_Register = 0x00;
        p_APU->m_WaveChannel.m_PeriodHighControl.m_Register = 0x00;
        p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register = 0x00;
        p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register = 0x00;
        p_APU->m_NoiseChannel.m_Control.m_Register = 0x00;
    }
}

void TOMBOY_WriteNR51 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_SoundPanning.m_Register = p_Value;
}

void TOMBOY_WriteNR50 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_MasterVolumeControl.m_Register = p_Value;
}

void TOMBOY_WriteNR10 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_FrequencySweep.m_Register = p_Value;
}

void TOMBOY_WriteNR11 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_LengthDuty.m_Register = p_Value;
}

void TOMBOY_WriteNR12 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Register = p_Value;

    // If the volume is zero, and the sweep direction is decreasing, then the channel and its DAC
    // are disabled.
    if (p_APU->m_PulseChannel1.m_VolumeEnvelope.m_InitialVolume == 0 &&
        p_APU->m_PulseChannel1.m_VolumeEnvelope.m_Direction == TOMBOY_ESD_DECREASE)
    {
        p_APU->m_PulseChannel1.m_DACEnabled = false;
        p_APU->m_MasterControl.m_PC1Enable = false;
    }
    else
    {
        p_APU->m_PulseChannel1.m_DACEnabled = true;
    }
}

void TOMBOY_WriteNR13 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_PeriodLow.m_PeriodLow = p_Value;
    p_APU->m_PulseChannel1.m_CurrentPeriod =
        (p_APU->m_PulseChannel1.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel1.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel1.m_PeriodDivider = p_APU->m_PulseChannel1.m_CurrentPeriod;
}

void TOMBOY_WriteNR14 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel1.m_PeriodHighControl.m_Register = p_Value;
    p_APU->m_PulseChannel1.m_CurrentPeriod =
        (p_APU->m_PulseChannel1.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel1.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel1.m_PeriodDivider = p_APU->m_PulseChannel1.m_CurrentPeriod;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_PulseChannel1.m_PeriodHighControl.m_Trigger)
    {
        TOMBOY_TriggerChannelInternal(p_APU, TOMBOY_AC_PULSE_1);
    }
}

void TOMBOY_WriteNR21 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_LengthDuty.m_Register = p_Value;
}

void TOMBOY_WriteNR22 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Register = p_Value;

    // If the volume is zero, and the sweep direction is decreasing, then the channel and its DAC
    // are disabled.
    if (p_APU->m_PulseChannel2.m_VolumeEnvelope.m_InitialVolume == 0 &&
        p_APU->m_PulseChannel2.m_VolumeEnvelope.m_Direction == TOMBOY_ESD_DECREASE)
    {
        p_APU->m_PulseChannel2.m_DACEnabled = false;
        p_APU->m_MasterControl.m_PC2Enable = false;
    }
    else
    {
        p_APU->m_PulseChannel2.m_DACEnabled = true;
    }
}

void TOMBOY_WriteNR23 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_PeriodLow.m_PeriodLow = p_Value;
    p_APU->m_PulseChannel2.m_CurrentPeriod =
        (p_APU->m_PulseChannel2.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel2.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel2.m_PeriodDivider = p_APU->m_PulseChannel2.m_CurrentPeriod;
}

void TOMBOY_WriteNR24 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_PulseChannel2.m_PeriodHighControl.m_Register = p_Value;
    p_APU->m_PulseChannel2.m_CurrentPeriod =
        (p_APU->m_PulseChannel2.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_PulseChannel2.m_PeriodLow.m_PeriodLow;
    p_APU->m_PulseChannel2.m_PeriodDivider = p_APU->m_PulseChannel2.m_CurrentPeriod;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_PulseChannel2.m_PeriodHighControl.m_Trigger)
    {
        TOMBOY_TriggerChannelInternal(p_APU, TOMBOY_AC_PULSE_2);
    }
}

void TOMBOY_WriteNR30 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_DACEnable.m_Register = p_Value;

    // If the DAC is disabled, then the channel is disabled.
    if (p_APU->m_WaveChannel.m_DACEnable.m_DACPower == false)
    {
        p_APU->m_MasterControl.m_WCEnable = false;
    }
}

void TOMBOY_WriteNR31 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_LengthTimer.m_Register = p_Value;
}

void TOMBOY_WriteNR32 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_OutputLevel.m_Register = p_Value;
}

void TOMBOY_WriteNR33 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_PeriodLow.m_PeriodLow = p_Value;
    p_APU->m_WaveChannel.m_CurrentPeriod =
        (p_APU->m_WaveChannel.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_WaveChannel.m_PeriodLow.m_PeriodLow;
    p_APU->m_WaveChannel.m_PeriodDivider = p_APU->m_WaveChannel.m_CurrentPeriod;
}

void TOMBOY_WriteNR34 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_WaveChannel.m_PeriodHighControl.m_Register = p_Value;
    p_APU->m_WaveChannel.m_CurrentPeriod =
        (p_APU->m_WaveChannel.m_PeriodHighControl.m_PeriodHigh << 8) |
        p_APU->m_WaveChannel.m_PeriodLow.m_PeriodLow;
    p_APU->m_WaveChannel.m_PeriodDivider = p_APU->m_WaveChannel.m_CurrentPeriod;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_WaveChannel.m_PeriodHighControl.m_Trigger)
    {
        TOMBOY_TriggerChannelInternal(p_APU, TOMBOY_AC_WAVE);
    }
}

void TOMBOY_WriteNR41 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_LengthTimer.m_Register = p_Value;
}

void TOMBOY_WriteNR42 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Register = p_Value;

    // If the volume is zero, and the sweep direction is decreasing, then the channel and its DAC
    // are disabled.
    if (p_APU->m_NoiseChannel.m_VolumeEnvelope.m_InitialVolume == 0 &&
        p_APU->m_NoiseChannel.m_VolumeEnvelope.m_Direction == TOMBOY_ESD_DECREASE)
    {
        p_APU->m_NoiseChannel.m_DACEnabled = false;
        p_APU->m_MasterControl.m_NCEnable = false;
    }
    else
    {
        p_APU->m_NoiseChannel.m_DACEnabled = true;
    }
}

void TOMBOY_WriteNR43 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_FrequencyRandomness.m_Register = p_Value;

    // Update the noise channel's clock frequency.
    if (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider == 0)
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (0.5 * pow(2, p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift));
    }
    else
    {
        p_APU->m_NoiseChannel.m_CurrentClockFrequency = 
            262144 / (p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockDivider 
                << p_APU->m_NoiseChannel.m_FrequencyRandomness.m_ClockShift);
    }
}

void TOMBOY_WriteNR44 (TOMBOY_APU* p_APU, uint8_t p_Value)
{
    if (p_APU == NULL)
    {
        TM_error("APU context is NULL!");
        return;
    }

    // This register is read-only when the APU is disabled.
    if (p_APU->m_MasterControl.m_Enable == false)
    {
        return;
    }

    p_APU->m_NoiseChannel.m_Control.m_Register = p_Value;

    // If the trigger bit is set, then trigger the channel.
    if (p_APU->m_NoiseChannel.m_Control.m_Trigger)
    {
        TOMBOY_TriggerChannelInternal(p_APU, TOMBOY_AC_NOISE);
    }
}
