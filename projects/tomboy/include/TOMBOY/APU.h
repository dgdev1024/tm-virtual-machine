/**
 * @file  TOMBOY/APU.h
 * @brief Contains the definition of the TOMBOY emulator's audio-processing unit (APU) structure and 
 *        public functions.
 * 
 * The TOMBOY Emulator's APU context structure seeks to simulate the Game Boy's audio-processing unit
 * (APU) hardware. The APU is responsible for generating sound effects and music in games built on
 * the engine, output via four audio channels: two square-wave pulse channels (we'll call them
 * `PC1` and `PC2`), a programmable, sampled wave channel (`WC`), and a noise channel (`NC`).
 * 
 * The APU provides the following hardware registers:
 * 
 * - `NR52` (Audio Master Control): The APU's master control register is used to enable or disable
 *   the APU hardware, as well as to check the enable status of the four audio channels. This
 *   register's bits are defined as follows:
 *     - Bit 0 - `PC1` Enable: Set if `PC1` is enabled; clear otherwise. Read-only.
 *     - Bit 1 - `PC2` Enable: Set if `PC2` is enabled; clear otherwise. Read-only.
 *     - Bit 2 - `WC` Enable: Set if `WC` is enabled; clear otherwise. Read-only.
 *     - Bit 3 - `NC` Enable: Set if `NC` is enabled; clear otherwise. Read-only.
 *     - Bit 7 - APU Enable: Set to enable the APU; clear to disable the APU. Clearing this bit
 *       also resets all of the APU's hardware registers and makes them read-only, except for this
 *       register.
 * 
 * - `NR51` (Sound Panning): The APU's sound panning register is used to control the stereo panning
 *   of the four audio channels. This register's bits are defined as follows:
 *     - Bit 0 - `PC1` Right: Set to output `PC1` to the right speaker; clear otherwise.
 *     - Bit 1 - `PC2` Right: Set to output `PC2` to the right speaker; clear otherwise.
 *     - Bit 2 - `WC` Right: Set to output `WC` to the right speaker; clear otherwise.
 *     - Bit 3 - `NC` Right: Set to output `NC` to the right speaker; clear otherwise.
 *     - Bit 4 - `PC1` Left: Set to output `PC1` to the left speaker; clear otherwise.
 *     - Bit 5 - `PC2` Left: Set to output `PC2` to the left speaker; clear otherwise.
 *     - Bit 6 - `WC` Left: Set to output `WC` to the left speaker; clear otherwise.
 *     - Bit 7 - `NC` Left: Set to output `NC` to the left speaker; clear otherwise.
 * 
 * - `NR50` (Master Volume Control and VIN Panning): The APU's master volume control and VIN panning
 *   register is used to control the overall volume of the APU's audio output, as well as the panning
 *   of the `VIN` channel, an external audio channel which was never used by any Game Boy games.
 *   The bits of this register control the following settings:
 *      - Bits 0-2 - Right Speaker Volume: The volume level of the right speaker, from 0 (mute) to
 *        7 (max volume).
 *      - Bit 3: `VIN` Right: Set to output the `VIN` channel to the right speaker; clear otherwise.
 *      - Bits 4-6 - Left Speaker Volume: The volume level of the left speaker, from 0 (mute) to 7
 *        (max volume).
 *      - Bit 7: `VIN` Left: Set to output the `VIN` channel to the left speaker; clear otherwise.
 * 
 * - `NR10` (`PC1` Frequency Sweep): `PC1` comes with a frequency sweep unit that can change the
 *   frequency of the square wave at a specified rate. This register controls the sweep unit's
 *   settings. The bits of this register control the following settings:
 *      - Bits 0-2 - Individual Step: Controls how much the channel's frequency changes with each
 *        iteration of the frequency sweep.
 *      - Bit 3 - Direction: Controls the direction of the frequency sweep (Set = decrease;
 *        Clear = increase).
 *      - Bits 4-6 - Sweep Pace: Controls the duration of each sweep step, in units of 128 Hz ticks.
 * 
 * - `NR11` (`PC1` Length Timer & Duty Cycle): This register controls `PC1`s initial length timer
 *   value, as well as its wave pattern duty cycle. This register's bits control the following
 *   settings:
 *      - Bits 0-5 - Initial Length Timer: The value to set the length timer to when the channel is
 *        triggered. Write-only.
 *      - Bits 6-7 - Wave Pattern Duty Cycle: The duty cycle of the square wave, which controls the
 *        ratio of high to low signal levels, affecting how the square wave sounds. The possible
 *        enumerated duty cycles are as follows:
 *          - `0b00` - 12.5% duty cycle: 1 part high, 7 parts low. Sounds like a thin pulse wave.
 *          - `0b01` - 25% duty cycle: 2 parts high, 6 parts low. Sounds like a medium pulse wave.
 *          - `0b10` - 50% duty cycle: 4 parts high, 4 parts low. Sounds like a square wave.
 *          - `0b11` - 75% duty cycle: 6 parts high, 2 parts low. Sounds like a wide pulse wave, and
 *            sounds just like the 25% duty cycle wave.
 * 
 * - `NR12` (`PC1` Volume & Envelope): This register controls the initial volume of `PC1`, as well
 *   as the channel's envelope sweep settings. The bits of this register control the following
 *   settings:
 *      - Bits 0-2 - Sweep Pace: Controls how much the channel's volume changes with each iteration
 *        of the envelope sweep, in units of 64 Hz ticks. A value of 0 stops the envelope sweep.
 *      - Bit 3 - Direction: Controls the direction of the envelope sweep (Set = increase; Clear =
 *        decrease).
 *      - Bits 4-7 - Initial Volume: The volume level to set the channel to when the channel is
 *        triggered, from 0 (mute) to 15 (max volume).
 *      - Setting Bits 3-7 to 0 disables the channel's digital-to-analog converter (DAC), and
 *        thereby disables the channel.
 * 
 * - `NR13` (`PC1` Period Low Byte): This register contains the lower 8 bits of `PC1`'s 11-bit
 *   frequency period value. Write-only.
 * 
 * - `NR14` (`PC1` Period High Bits & Control): This register contains the upper 3 bits of `PC1`'s
 *   11-bit frequency period value, as well as the channel's length timer enable and trigger bits.
 *   The bits of this register control the following settings:
 *      - Bits 0-2 - Period High Bits: The upper 3 bits of the 11-bit frequency period value. Write-only.
 *      - Bit 6 - Length Timer Enable: Set to enable the channel's length timer; clear otherwise.
 *      - Bit 7 - Trigger: Write to this bit to trigger the channel's sound generation. Write-only.
 * 
 * - `NR21`, `NR22`, `NR23`, and `NR24` work exactly the same as `NR11`, `NR12`, `NR13`, and `NR14`,
 *   but for `PC2` instead of `PC1`. Note that `PC2` does not have a frequency sweep unit.
 * 
 * - `NR30` (`WC` DAC Enable): This register controls the digital-to-analog converter (DAC) of the
 *   wave channel. The bits of this register control the following settings:
 *      - Bit 7 - DAC Power: Set to enable the channel's DAC; clear otherwise. Clearing this bit
 *        disables the channel's DAC and thereby disables the channel. Although not required, it is
 *        recommended to turn off this channel's DAC before writing to the APU's wave sample buffer.
 * 
 * - `NR31` (`WC` Length Timer): This register controls the initial length timer value of the wave
 *   channel. The bits of this register control the following settings:
 *      - Bits 0-7 - Initial Length Timer: The value to set the length timer to when the channel is
 *        triggered. Write-only.
 * 
 * - `NR32` (`WC` Output Level): The wave channel does not have a volume envelope like the pulse
 *   channels. Instead, it has a coarser, fixed volume control. This register controls that volume
 *   level. The bits of this register control the following settings:
 *      - Bits 5-6 - Output Level: The volume level of the channel, from 0 (mute) to 3 (max volume).
 *        The possible enumerated volume levels are as follows:
 *          - `0b00` - Mute: The channel is silent and outputs no sound.
 *          - `0b01` - 100% Volume: The channel outputs at full volume.
 *          - `0b10` - 50% Volume: The channel outputs at half volume.
 *          - `0b11` - 25% Volume: The channel outputs at quarter volume.
 * 
 * - `NR33` and `NR34` work exactly the same as `NR13` and `NR14`, but for `WC` instead of `PC1`.
 * 
 * - `NR41` (`NC` Length Timer): This register controls the initial length timer value of the noise
 *   channel. The bits of this register control the following settings:
 *      - Bits 0-5 - Initial Length Timer: The value to set the length timer to when the channel is
 *        triggered. Write-only.
 * 
 * - `NR42` (`NC` Volume & Envelope): This register controls the initial volume of the noise channel,
 *   as well as the channel's envelope sweep settings. This register works exactly the same as `NR12`,
 *   but for the noise channel instead of `PC1`.
 * 
 * - `NR43` (`NC` Frequency & Randomness): The noise channel uses a linear-feedback shift register
 *   (LFSR) to generate pseudo-random noise. This register controls the frequency of the noise channel
 *   and the feedback pattern of the LFSR. The bits of this register control the following settings:
 *      - Bits 0-2 - Clock Divider: Used along with the Clock Shift setting (see below) to determine
 *        the frequency of the noise channel.
 *      - Bit 3 - LFSR Width: Controls the width of the LFSR's feedback pattern. Set to 0 for a 15-bit
 *        pattern, or 1 for a 7-bit pattern.
 *      - Bits 4-7 - Clock Shift: Used along with the Clock Divider setting (see above) to determine
 *        the frequency of the noise channel.
 *      - The following formula is used to calculate the noise channel's frequency:
 *          - `F = 262144 / (D * (2 ^ S))`
 *          - `F` is the resultant frequency.
 *          - `D` is the Clock Divider setting. If `D` = 0, then `D` = 0.5, instead.
 *          - `S` is the Clock Shift setting.
 * 
 * - `NR44` (`NC` Control): This register controls the noise channel's length timer enable and trigger
 *   bits. The bits of this register control the following settings:
 *      - Bit 6 - Length Timer Enable: Set to enable the channel's length timer; clear otherwise.
 *      - Bit 7 - Trigger: Write to this bit to trigger the channel's sound generation. Write-only.
 * 
 * The APU component provides the following memory buffer:
 * 
 * - The Wave Pattern RAM (`WAVE`): The wave pattern RAM is a 16-byte buffer which stores 32 4-bit
 *   samples that make up the wave channel's waveform. The wave channel can play back these samples
 *   at a specified frequency to generate a custom waveform. The wave channel's output level setting
 *   (`NR32`) controls the volume level of the wave channel. The wave pattern RAM is mapped to the
 *   `0xFF30` to `0xFF3F` memory region.
 * 
 * The APU component is clocked at the same rate as the engine's timer and PPU components - at a
 * frequency of 4.194304 MHz.
 * - The pulse channels are clocked at a frequency of 1.048576 MHz - every 4 clock cycles.
 * - The wave channel is clocked at a frequency of 2.084576 MHz - every 2 clock cycles.
 * - The noise channel is clocked at a frequency dictated by the clock divider and clock shift settings
 *   in the `NR43` register (see the formula above).
 * 
 * When the pulse and wave channels are clocked, an internal period divider register is incremented,
 * resetting to the period value in the `NR*3` and `NR*4` registers when it overflows. The higher
 * the initial period value in these registers, the lower the period, the higher the resultant
 * frequency, and the shorter the wavelength of the sound wave.
 * - The resultant sample rate and size of the pulse channels' square waves are determined by the
 *   following formulae:
 *      - `R = 1048756 / (2048 - PV)`
 *      - `S = 131072 / (2048 - PV)`
 *      - `R` is the resultant sample rate.
 *      - `S` is the resultant sample size.
 *      - `PV` is the channels' current period values.
 * - The wave channel's sample rate is determined by the following formulae:
 *      - `R = 2097152 / (2048 - PV)`
 *      - `S = 65536 / (2048 - PV)`
 *      - `R`, `S`, and `PV` are the same as above.
 * 
 * When the noise channel is clocked, its LFSR is processed as follows:
 * - Bit 15 is set if the values of bits 0 and 1 are equal; otherwise, it is cleared.
 * - If `NR43`'s LFSR width is set, then bit 7 is set to the same value.
 * - The LFSR is shifted right by one bit.
 * - The new value of bit 0 determines the output of the noise channel.
 * 
 * The APU component is equipped with its own internal divider register, which is incremented every
 * time the 4th bit (or 5th bit in double-speed mode) of the engine timer's `DIV` register changes from
 * high to low. This happens at a frequency of 512 Hz, and we'll call this unit of time an "APU-DIV tick".
 * The following events occur at the following intervals:
 * - The channels' envelope sweep units are clocked every 8 APU-DIV ticks, at a frequency of 64 Hz.
 * - The channels' length timers are clocked every 2 APU-DIV ticks, at a frequency of 256 Hz.
 * - `PC1`'s frequency sweep unit is clocked every 4 APU-DIV ticks, at a frequency of 128 Hz.
 */

#pragma once
#include <TOMBOY/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief The size of the TOMBOY APU's wave pattern RAM, in bytes.
 */
#define TOMBOY_WAVE_RAM_SIZE 16

/**
 * @brief The size of the TOMBOY APU's wave pattern RAM, in nibbles.
 */
#define TOMBOY_WAVE_RAM_NIBBLES 32

/**
 * @brief The sample rate of the audio output by the TOMBOY APU, in Hz.
 */
#define TOMBOY_AUDIO_SAMPLE_RATE 44100

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward declaration of the TOMBOY emulator engine structure.
 */
typedef struct TOMBOY_Engine TOMBOY_Engine;

/**
 * @brief A forward declaration of the TOMBOY APU audio sample structure.
 */
typedef struct TOMBOY_AudioSample TOMBOY_AudioSample;

/**
 * @brief A forward declaration of the TOMBOY audio-processing unit (APU) structure.
 */
typedef struct TOMBOY_APU TOMBOY_APU;

/**
 * @brief A pointer to a function that is called by the APU when it is time to generate a new
 *        audio sample.
 */
typedef void (*TOMBOY_AudioMixCallback) (const TOMBOY_AudioSample*);

// Enumerations ////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Enumerates the APU's four audio channels.
 */
typedef enum TOMBOY_AudioChannel
{
    TOMBOY_AC_PC1 = 0,     ///< @brief The `PC1` audio channel.
    TOMBOY_AC_PC2,         ///< @brief The `PC2` audio channel.
    TOMBOY_AC_WC,          ///< @brief The `WC` audio channel.
    TOMBOY_AC_NC,          ///< @brief The `NC` audio channel.

    TOMBOY_AC_PULSE_1 = TOMBOY_AC_PC1,     ///< @brief The `PC1` audio channel.
    TOMBOY_AC_PULSE_2 = TOMBOY_AC_PC2,     ///< @brief The `PC2` audio channel.
    TOMBOY_AC_WAVE = TOMBOY_AC_WC,         ///< @brief The `WC` audio channel.
    TOMBOY_AC_NOISE = TOMBOY_AC_NC         ///< @brief The `NC` audio channel.
} TOMBOY_AudioChannel;

/**
 * @brief Enumerates the sweep directions of `PC1`'s frequency sweep unit.
 */
typedef enum TOMBOY_FrequencySweepDirection
{
    TOMBOY_FSD_INCREASE = 0,     ///< @brief Increase the frequency.
    TOMBOY_FSD_DECREASE          ///< @brief Decrease the frequency.
} TOMBOY_FrequencySweepDirection;

/**
 * @brief Enumerates the sweep direction of the pulse and noise channels' volume envelope units.
 */
typedef enum TOMBOY_EnvelopeSweepDirection
{
    TOMBOY_ESD_DECREASE = 0,     ///< @brief Decrease the volume.
    TOMBOY_ESD_INCREASE          ///< @brief Increase the volume.
} TOMBOY_EnvelopeSweepDirection;

/**
 * @brief Enumerates the possible duty cycles of the TOMBOY Engine's pulse channels.
 */
typedef enum TOMBOY_PulseDutyCycle
{
    TOMBOY_PDC_12_5 = 0,     ///< @brief 12.5% duty cycle: 1 part high, 7 parts low.
    TOMBOY_PDC_25,           ///< @brief 25% duty cycle: 2 parts high, 6 parts low.
    TOMBOY_PDC_50,           ///< @brief 50% duty cycle: 4 parts high, 4 parts low.
    TOMBOY_PDC_75            ///< @brief 75% duty cycle: 6 parts high, 2 parts low.
} TOMBOY_PulseDutyCycle;

/**
 * @brief Enumerates the possible output levels of the TOMBOY Engine's wave channel.
 */
typedef enum TOMBOY_WaveOutputLevel
{
    TOMBOY_WOL_MUTE = 0,     ///< @brief Mute: The channel is silent and outputs no sound.
    TOMBOY_WOL_100,          ///< @brief 100% Volume: The channel outputs at full volume.
    TOMBOY_WOL_50,           ///< @brief 50% Volume: The channel outputs at half volume.
    TOMBOY_WOL_25,           ///< @brief 25% Volume: The channel outputs at quarter volume.

    TOMBOY_WOL_FULL = TOMBOY_WOL_100,     ///< @brief Full volume: The channel outputs at full volume.
    TOMBOY_WOL_HALF = TOMBOY_WOL_50,      ///< @brief Half volume: The channel outputs at half volume.
    TOMBOY_WOL_QUARTER = TOMBOY_WOL_25    ///< @brief Quarter volume: The channel outputs at quarter volume.
} TOMBOY_WaveOutputLevel;

// Hardware Register Unions ////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the TOMBOY Engine's audio master control (`NR52`) register.
 */
typedef union TOMBOY_AudioMasterControl
{
    struct
    {
        uint8_t m_PC1Enable : 1;                  ///< @brief The `PC1` channel enable bit.
        uint8_t m_PC2Enable : 1;                  ///< @brief The `PC2` channel enable bit.
        uint8_t m_WCEnable : 1;                   ///< @brief The `WC` channel enable bit.
        uint8_t m_NCEnable : 1;                   ///< @brief The `NC` channel enable bit.
        uint8_t : 3;                              ///< @brief Unused bits.
        uint8_t m_Enable : 1;                     ///< @brief The APU enable bit.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_AudioMasterControl;

/**
 * @brief A union representing the TOMBOY Engine's sound panning (`NR51`) register.
 */
typedef union TOMBOY_SoundPanning
{
    struct
    {
        uint8_t m_PC1Right : 1;                   ///< @brief The `PC1` right speaker bit.
        uint8_t m_PC2Right : 1;                   ///< @brief The `PC2` right speaker bit.
        uint8_t m_WCRight : 1;                    ///< @brief The `WC` right speaker bit.
        uint8_t m_NCRight : 1;                    ///< @brief The `NC` right speaker bit.
        uint8_t m_PC1Left : 1;                    ///< @brief The `PC1` left speaker bit.
        uint8_t m_PC2Left : 1;                    ///< @brief The `PC2` left speaker bit.
        uint8_t m_WCLeft : 1;                     ///< @brief The `WC` left speaker bit.
        uint8_t m_NCLeft : 1;                     ///< @brief The `NC` left speaker bit.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_SoundPanning;

/**
 * @brief A union representing the TOMBOY Engine's master volume control and `VIN` panning (`NR50`)
 * register.
 */
typedef union TOMBOY_MasterVolumeControl
{
    struct
    {
        uint8_t m_RightVolume : 3;                ///< @brief The right speaker volume bits.
        uint8_t m_VINRight : 1;                   ///< @brief The `VIN` right speaker bit.
        uint8_t m_LeftVolume : 3;                 ///< @brief The left speaker volume bits.
        uint8_t m_VINLeft : 1;                    ///< @brief The `VIN` left speaker bit.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_MasterVolumeControl;

/**
 * @brief A union representing the TOMBOY Engine's `PC1` frequency sweep (`NR10`) register.
 */
typedef union TOMBOY_PulseFrequencySweep
{
    struct
    {
        uint8_t m_IndividualStep : 3;             ///< @brief The individual step bits.
        uint8_t m_Direction : 1;                  ///< @brief The direction bit.
        uint8_t m_SweepPace : 3;                  ///< @brief The sweep pace bits.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_PulseFrequencySweep;

/**
 * @brief A union representing the TOMBOY Engine's pulse channels' length timer and duty cycle 
 *        (`NR*1`) registers.
 */
typedef union TOMBOY_PulseLengthDuty
{
    struct
    {
        uint8_t m_InitialLength : 6;              ///< @brief The initial length timer bits.
        uint8_t m_DutyCycle : 2;                  ///< @brief The duty cycle bits.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_PulseLengthDuty;

/**
 * @brief A union representing the TOMBOY Engine's pulse and noise channels' volume and envelope
 *        (`NR*2`) registers.
 */
typedef union TOMBOY_VolumeEnvelope
{
    struct
    {
        uint8_t m_SweepPace : 3;                  ///< @brief The sweep pace bits.
        uint8_t m_Direction : 1;                  ///< @brief The direction bit.
        uint8_t m_InitialVolume : 4;              ///< @brief The initial volume bits.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_VolumeEnvelope;

/**
 * @brief A union representing the TOMBOY Engine's pulse and wave channels' period low byte (`NR*3`)
 *        registers.
 */
typedef union TOMBOY_PeriodLowByte
{
    struct
    {
        uint8_t m_PeriodLow;                      ///< @brief The period low byte.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_PeriodLowByte;

/**
 * @brief A union representing the TOMBOY Engine's audio channels' period high bits and control
 *        (`NR*4`) registers.
 * 
 * @note The `m_PeriodHigh` field is not used by the noise channel, and should not be accessed.
 */
typedef union TOMBOY_PeriodHighControl
{
    struct
    {
        uint8_t m_PeriodHigh : 3;                 ///< @brief The period high bits.
        uint8_t : 3;                              ///< @brief Unused bits.
        uint8_t m_LengthEnable : 1;               ///< @brief The length timer enable bit.
        uint8_t m_Trigger : 1;                    ///< @brief The trigger bit.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_PeriodHighControl;

/**
 * @brief A union representing the TOMBOY Engine's wave channel DAC enable (`NR30`) register.
 */
typedef union TOMBOY_WaveDACEnable
{
    struct
    {
        uint8_t : 7;                              ///< @brief Unused bits.
        uint8_t m_DACPower : 1;                   ///< @brief The DAC power bit.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_WaveDACEnable;

/**
 * @brief A union representing the TOMBOY Engine's wave channel length timer (`NR31`) register.
 */
typedef union TOMBOY_WaveLengthTimer
{
    struct
    {
        uint8_t m_InitialLength;                  ///< @brief The initial length timer.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_WaveLengthTimer;

/**
 * @brief A union representing the TOMBOY Engine's wave channel output level (`NR32`) register.
 */
typedef union TOMBOY_WaveOutputLevelControl
{
    struct
    {
        uint8_t : 5;                              ///< @brief Unused bits.
        uint8_t m_OutputLevel : 2;                ///< @brief The output level bits.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_WaveOutputLevelControl;

/**
 * @brief A union representing the TOMBOY Engine's noise channel length timer (`NR41`) register.
 */
typedef union TOMBOY_NoiseLengthTimer
{
    struct
    {
        uint8_t m_InitialLength : 6;              ///< @brief The initial length timer.
        uint8_t : 2;                              ///< @brief Unused bits.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_NoiseLengthTimer;

/**
 * @brief A union representing the TOMBOY Engine's noise channel frequency and randomness (`NR43`)
 *        register.
 */
typedef union TOMBOY_NoiseFrequencyRandomness
{
    struct
    {
        uint8_t m_ClockDivider : 3;               ///< @brief The clock divider bits.
        uint8_t m_LFSRWidth : 1;                  ///< @brief The LFSR width bit.
        uint8_t m_ClockShift : 4;                 ///< @brief The clock shift bits.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_NoiseFrequencyRandomness;

/**
 * @brief A union representing the TOMBOY Engine's noise channel control (`NR44`) register.
 */
typedef union TOMBOY_NoiseControl
{
    struct
    {
        uint8_t : 6;                              ///< @brief Unused bits.
        uint8_t m_LengthEnable : 1;               ///< @brief The length timer enable bit.
        uint8_t m_Trigger : 1;                    ///< @brief The trigger bit.
    };

    uint8_t m_Register;                           ///< @brief The raw register value.
} TOMBOY_NoiseControl;

// Audio Sample Structure //////////////////////////////////////////////////////////////////////////

/**
 * @brief Represents a single audio sample mixed by the TOMBOY Engine's APU.
 */
typedef struct TOMBOY_AudioSample
{
    float m_Left;     ///< @brief The left speaker audio sample.
    float m_Right;    ///< @brief The right speaker audio sample.
} TOMBOY_AudioSample;

// Public Function Prototypes //////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new instance of the TOMBOY audio-processing unit (APU).
 * 
 * @param p_Engine  A pointer to the TOMBOY engine instance to associate with the APU.
 * 
 * @return A pointer to the new TOMBOY APU instance.
 */
TOMBOY_APU* TOMBOY_CreateAPU (TOMBOY_Engine* p_Engine);

/**
 * @brief Resets the given TOMBOY APU instance, resetting it and its components to their initial state.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to reset.
 */
void TOMBOY_ResetAPU (TOMBOY_APU* p_APU);

/**
 * @brief Destroys the given TOMBOY APU instance, freeing its resources.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to destroy.
 */
void TOMBOY_DestroyAPU (TOMBOY_APU* p_APU);

/**
 * @brief Sets the audio mix callback function for the given TOMBOY APU instance.
 * 
 * The audio mix callback function is called by the APU every time it generates a new audio sample.
 * The callback function is passed a pointer to the audio sample generated by the APU.
 * 
 * @param p_APU      A pointer to the TOMBOY APU instance to set the callback for.
 * @param p_Callback  The audio mix callback function to set.
 */
void TOMBOY_SetAudioMixCallback (TOMBOY_APU* p_APU, TOMBOY_AudioMixCallback p_Callback);

/**
 * @brief Gets the latest audio sample mixed by the given TOMBOY APU instance.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to get the audio sample from.
 * 
 * @return A pointer to the latest audio sample mixed by the APU.
 */
const TOMBOY_AudioSample* TOMBOY_GetLatestAudioSample (const TOMBOY_APU* p_APU);

/**
 * @brief Ticks the given TOMBOY APU instance, updating its components and state.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to tick.
 * @param p_DIV  Does the APU's internal divider need to be ticked?
 * 
 * @return `true` if the APU was ticked successfully; `false` otherwise.
 */
bool TOMBOY_TickAPU (TOMBOY_APU* p_APU, bool p_DIV);

// Public Function Prototypes - Memory Access //////////////////////////////////////////////////////

/**
 * @brief Reads a byte from the APU's wave pattern RAM.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Address     The address in the wave pattern RAM to read from.
 * 
 * @return The byte read from the wave pattern RAM.
 */
uint8_t TOMBOY_ReadWaveByte (const TOMBOY_APU* p_APU, uint32_t p_Address);

/**
 * @brief Writes a byte to the APU's wave pattern RAM.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Address     The address in the wave pattern RAM to write to.
 * @param p_Value       The byte to write to the wave pattern RAM.
 */
void TOMBOY_WriteWaveByte (TOMBOY_APU* p_APU, uint32_t p_Address, uint8_t p_Value);

// Public Function Prototypes - Hardware Register Getters //////////////////////////////////////////

/**
 * @brief Reads the given APU instance's audio master control register, `NR52`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR52` register.
 */
uint8_t TOMBOY_ReadNR52 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's sound panning register, `NR51`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR51` register.
 */
uint8_t TOMBOY_ReadNR51 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's master volume control register, `NR50`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR50` register.
 */
uint8_t TOMBOY_ReadNR50 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC1` frequency sweep register, `NR10`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR10` register.
 */
uint8_t TOMBOY_ReadNR10 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC1` length timer and duty cycle register, `NR11`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR11` register.
 */
uint8_t TOMBOY_ReadNR11 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC1` volume and envelope register, `NR12`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR12` register.
 */
uint8_t TOMBOY_ReadNR12 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC1` period low byte register, `NR13`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR13` register.
 */
uint8_t TOMBOY_ReadNR13 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC1` period high byte and control register, `NR14`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR14` register.
 */
uint8_t TOMBOY_ReadNR14 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC2` length timer and duty cycle register, `NR21`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR21` register.
 */
uint8_t TOMBOY_ReadNR21 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC2` volume and envelope register, `NR22`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR22` register.
 */
uint8_t TOMBOY_ReadNR22 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC2` period low byte register, `NR23`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR23` register.
 */
uint8_t TOMBOY_ReadNR23 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's `PC2` period high byte and control register, `NR24`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR24` register.
 */
uint8_t TOMBOY_ReadNR24 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's wave channel DAC enable register, `NR30`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR30` register.
 */
uint8_t TOMBOY_ReadNR30 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's wave channel length timer register, `NR31`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR31` register.
 */
uint8_t TOMBOY_ReadNR31 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's wave channel output level register, `NR32`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR32` register.
 */
uint8_t TOMBOY_ReadNR32 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's wave channel period low byte register, `NR33`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR33` register.
 */
uint8_t TOMBOY_ReadNR33 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's wave channel period high byte and control register, `NR34`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR34` register.
 */
uint8_t TOMBOY_ReadNR34 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's noise channel length timer register, `NR41`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR41` register.
 */
uint8_t TOMBOY_ReadNR41 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's noise channel volume and envelope register, `NR42`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR42` register.
 */
uint8_t TOMBOY_ReadNR42 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's noise channel frequency and randomness register, `NR43`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR43` register.
 */
uint8_t TOMBOY_ReadNR43 (const TOMBOY_APU* p_APU);

/**
 * @brief Reads the given APU instance's noise channel control register, `NR44`.
 * 
 * @param p_APU  A pointer to the TOMBOY APU instance to read from.
 * 
 * @return The value of the `NR44` register.
 */
uint8_t TOMBOY_ReadNR44 (const TOMBOY_APU* p_APU);

// Public Function Prototypes - Hardware Register Setters //////////////////////////////////////////

/**
 * @brief Sets the audio master control register (`NR52`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new audio master control register value.
 */
void TOMBOY_WriteNR52 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the sound panning register (`NR51`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new sound panning register value.
 */
void TOMBOY_WriteNR51 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the master volume control register (`NR50`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new master volume control register value.
 */
void TOMBOY_WriteNR50 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC1` frequency sweep register (`NR10`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC1` frequency sweep register value.
 */
void TOMBOY_WriteNR10 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC1` length timer and duty cycle register (`NR11`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC1` length timer and duty cycle register value.
 */
void TOMBOY_WriteNR11 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC1` volume and envelope register (`NR12`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC1` volume and envelope register value.
 */
void TOMBOY_WriteNR12 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC1` period low byte register (`NR13`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC1` period low byte register value.
 */
void TOMBOY_WriteNR13 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC1` period high byte and control register (`NR14`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC1` period high byte and control register value.
 */
void TOMBOY_WriteNR14 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC2` length timer and duty cycle register (`NR21`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC2` length timer and duty cycle register value.
 */
void TOMBOY_WriteNR21 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC2` volume and envelope register (`NR22`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC2` volume and envelope register value.
 */
void TOMBOY_WriteNR22 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC2` period low byte register (`NR23`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC2` period low byte register value.
 */
void TOMBOY_WriteNR23 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the `PC2` period high byte and control register (`NR24`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new `PC2` period high byte and control register value.
 */
void TOMBOY_WriteNR24 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the wave channel DAC enable register (`NR30`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new wave channel DAC enable register value.
 */
void TOMBOY_WriteNR30 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the wave channel length timer register (`NR31`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new wave channel length timer register value.
 */
void TOMBOY_WriteNR31 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the wave channel output level register (`NR32`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new wave channel output level register value.
 */
void TOMBOY_WriteNR32 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the wave channel period low byte register (`NR33`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new wave channel period low byte register value.
 */
void TOMBOY_WriteNR33 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the wave channel period high byte and control register (`NR34`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new wave channel period high byte and control register value.
 */
void TOMBOY_WriteNR34 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the noise channel length timer register (`NR41`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new noise channel length timer register value.
 */
void TOMBOY_WriteNR41 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the noise channel volume and envelope register (`NR42`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new noise channel volume and envelope register value.
 */
void TOMBOY_WriteNR42 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the noise channel frequency and randomness register (`NR43`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new noise channel frequency and randomness register value.
 */
void TOMBOY_WriteNR43 (TOMBOY_APU* p_APU, uint8_t p_Register);

/**
 * @brief Sets the noise channel control register (`NR44`) of the TOMBOY APU.
 * 
 * @param p_APU         A pointer to the TOMBOY APU instance.
 * @param p_Register    The new noise channel control register value.
 */
void TOMBOY_WriteNR44 (TOMBOY_APU* p_APU, uint8_t p_Register);
