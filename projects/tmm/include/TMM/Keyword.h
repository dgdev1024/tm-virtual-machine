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
    TMM_KT_ASSERT,                  ///< @brief "Assert" keyword (eg. `assert x == 0`).
    TMM_KT_DB,                      ///< @brief "Define Byte" keyword (eg. `db 0x00`).
    TMM_KT_DW,                      ///< @brief "Define Word" keyword (eg. `dw 0x0000`).
    TMM_KT_DL,                      ///< @brief "Define Long" keyword (eg. `dl 0x00000000`).
    TMM_KT_DS,                      ///< @brief "Define Sequence" keyword (eg. `ds 3, 0x00, 0x11`, `ds 5, 0x44`).
    TMM_KT_DF,                      ///< @brief "Define Float" keyword (eg. `df 3.14`).
    TMM_KT_DD,                      ///< @brief "Define Double" keyword (eg. `dd 3.14159`).
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
