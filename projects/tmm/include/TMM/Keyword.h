/**
 * @file     TMM/Keyword.h
 * @brief    Contains functions for looking up reserved keywords.
 */

#pragma once

#include <TM/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define TMM_KEYWORD_STRLEN 16

// Keyword Type Enumeration ////////////////////////////////////////////////////////////////////////

typedef enum TMM_KeywordType
{
    TMM_KT_NONE = 0,                ///< @brief No keyword type.
    TMM_KT_ORG,                     ///< @brief "Origin" keyword (eg. `org rom`, `org ram, 0x1400`).
    TMM_KT_ROM,                     ///< @brief ROM origin keyword.
    TMM_KT_RAM,                     ///< @brief RAM origin keyword.
    TMM_KT_ASSERT,                  ///< @brief "Assert" keyword (eg. `assert x == 0`).
    TMM_KT_DB,                      ///< @brief "Define Byte" keyword (eg. `db 0x00`).
    TMM_KT_DW,                      ///< @brief "Define Word" keyword (eg. `dw 0x0000`).
    TMM_KT_DL,                      ///< @brief "Define Long" keyword (eg. `dl 0x00000000`).
    TMM_KT_DS,                      ///< @brief "Define Sequence" keyword (eg. `ds 3, 0x00, 0x11`, `ds 5, 0x44`).
    TMM_KT_DF,                      ///< @brief "Define Fixed Point" keyword (eg. `df 3.14`).
    TMM_KT_INCLUDE,                 ///< @brief "Include" keyword (eg. `include "file.asm"`).
    TMM_KT_INCBIN,                  ///< @brief "Include Binary" keyword (eg. `incbin "file.bin"`).
    TMM_KT_DEF,                     ///< @brief "Define" keyword (eg. `def x = 0x00`).
    TMM_KT_MACRO,                   ///< @brief "Macro" keyword (eg. `macro name`).
    TMM_KT_ENDM,                    ///< @brief "End Macro" keyword (eg. `endm`).
    TMM_KT_NARG,                    ///< @brief "Number of Arguments" keyword (eg. `narg`).
    TMM_KT_SHIFT,                   ///< @brief "Shift" keyword (eg. `shift 2`).
    TMM_KT_REPEAT,                  ///< @brief "Repeat" keyword (eg. `repeat 3`, `rept 5`).
    TMM_KT_FOR,                     ///< @brief "For" keyword (eg. `for n, 256`, `for x, 0, 20, 4`).
    TMM_KT_IF,                      ///< @brief "If" keyword (eg. `if n == 0`).
    TMM_KT_ELIF,                    ///< @brief "Else If" keyword (eg. `elif n == 1`).
    TMM_KT_ELSE,                    ///< @brief "Else" keyword (eg. `else`).
    TMM_KT_ENDR,                    ///< @brief "End Repeat", "End For" keyword (eg. `endr`).
    TMM_KT_ENDC,                    ///< @brief "End If" keyword (eg. `endc`).

    // Registers
    TMM_KT_A,
    TMM_KT_AW,
    TMM_KT_AH,
    TMM_KT_AL,
    TMM_KT_B,
    TMM_KT_BW,
    TMM_KT_BH,
    TMM_KT_BL,
    TMM_KT_C,
    TMM_KT_CW,
    TMM_KT_CH,
    TMM_KT_CL,
    TMM_KT_E,
    TMM_KT_EW,
    TMM_KT_EH,
    TMM_KT_EL,

    // Conditions
    TMM_KT_NC,
    TMM_KT_ZS,
    TMM_KT_ZC,
    TMM_KT_CS,
    TMM_KT_CC,

    // Instructions
    TMM_KT_NOP,
    TMM_KT_STOP,
    TMM_KT_HALT,
    TMM_KT_SEC,
    TMM_KT_CEC,
    TMM_KT_DI,
    TMM_KT_EI,
    TMM_KT_DAA,
    TMM_KT_SCF,
    TMM_KT_CCF,
    TMM_KT_LD,
    TMM_KT_LDQ,
    TMM_KT_LDH,
    TMM_KT_ST,
    TMM_KT_STQ,
    TMM_KT_STH,
    TMM_KT_MV,
    TMM_KT_PUSH,
    TMM_KT_POP,
    TMM_KT_JMP,
    TMM_KT_JPB,
    TMM_KT_CALL,
    TMM_KT_RST,
    TMM_KT_RET,
    TMM_KT_RETI,
    TMM_KT_JPS,
    TMM_KT_INC,
    TMM_KT_DEC,
    TMM_KT_ADD,
    TMM_KT_ADC,
    TMM_KT_SUB,
    TMM_KT_SBC,
    TMM_KT_AND,
    TMM_KT_OR,
    TMM_KT_XOR,
    TMM_KT_NOT,
    TMM_KT_CMP,
    TMM_KT_SLA,
    TMM_KT_SRA,
    TMM_KT_SRL,
    TMM_KT_RL,
    TMM_KT_RLC,
    TMM_KT_RR,
    TMM_KT_RRC,
    TMM_KT_BIT,
    TMM_KT_RES,
    TMM_KT_SET,
    TMM_KT_SWAP
} TMM_KeywordType;

// Keyword Structure ///////////////////////////////////////////////////////////////////////////////

typedef struct TMM_Keyword
{
    char                    m_Name[TMM_KEYWORD_STRLEN];     ///< @brief Keyword Name
    TMM_KeywordType         m_Type;                         ///< @brief Keyword Type
    int32_t                 m_Param;                        ///< @brief Optional Parameter
} TMM_Keyword;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const TMM_Keyword* TMM_LookupKeyword (const char* p_Name);
const char* TMM_StringifyKeywordType (TMM_KeywordType p_Type);
bool TMM_IsRegisterKeyword (const TMM_Keyword* p_Keyword);
bool TMM_IsConditionKeyword (const TMM_Keyword* p_Keyword);
bool TMM_IsInstructionKeyword (const TMM_Keyword* p_Keyword);
bool TMM_IsDataKeyword (const TMM_Keyword* p_Keyword);
