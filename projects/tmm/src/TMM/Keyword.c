/**
 * @file     TMM/Keyword.c
 */

#include <TMM/Keyword.h>

// Keyword Lookup Table ////////////////////////////////////////////////////////////////////////////

static const TMM_Keyword TMM_KEYWORD_TABLE[] = {
    { "ASSERT",     TMM_KT_ASSERT,     0 },
    { "DB",         TMM_KT_DB,         0 },
    { "DW",         TMM_KT_DW,         0 },
    { "DL",         TMM_KT_DL,         0 },
    { "DS",         TMM_KT_DS,         0 },
    { "INCLUDE",    TMM_KT_INCLUDE,    0 },
    { "INCBIN",     TMM_KT_INCBIN,     0 },
    { "DEF",        TMM_KT_DEF,        0 },
    { "MACRO",      TMM_KT_MACRO,      0 },
    { "ENDM",       TMM_KT_ENDM,       0 },
    { "_NARG",      TMM_KT_NARG,       0 },
    { "SHIFT",      TMM_KT_SHIFT,      0 },
    { "REPEAT",     TMM_KT_REPEAT,     0 },
    { "REPT",       TMM_KT_REPEAT,     0 },
    { "FOR",        TMM_KT_FOR,        0 },
    { "IF",         TMM_KT_IF,         0 },
    { "ELIF",       TMM_KT_ELIF,       0 },
    { "ELSE",       TMM_KT_ELSE,       0 },
    { "ENDR",       TMM_KT_ENDR,       0 },
    { "ENDC",       TMM_KT_ENDC,       0 },
    { "",           TMM_KT_NONE,       0 }
};

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const TMM_Keyword* TMM_LookupKeyword (const char* p_Name)
{
    for (int i = 0; ; ++i)
    {
        if (
            TMM_KEYWORD_TABLE[i].m_Type == TMM_KT_NONE ||
            strncmp(TMM_KEYWORD_TABLE[i].m_Name, p_Name, TMM_KEYWORD_STRLEN) == 0
        )
        {
            return &TMM_KEYWORD_TABLE[i];
        }
    }
}

const char* TMM_StringifyKeywordType (TMM_KeywordType p_Type)
{
    switch (p_Type)
    {
        case TMM_KT_ASSERT:         return "ASSERT";
        case TMM_KT_DB:             return "DB";
        case TMM_KT_DW:             return "DW";
        case TMM_KT_DL:             return "DL";
        case TMM_KT_DS:             return "DS";
        case TMM_KT_DF:             return "DF";
        case TMM_KT_DD:             return "DD";
        case TMM_KT_INCLUDE:        return "INCLUDE";
        case TMM_KT_INCBIN:         return "INCBIN";
        case TMM_KT_DEF:            return "DEF";
        case TMM_KT_MACRO:          return "MACRO";
        case TMM_KT_ENDM:           return "ENDM";
        case TMM_KT_NARG:           return "_NARG";
        case TMM_KT_SHIFT:          return "SHIFT";
        case TMM_KT_REPEAT:         return "REPEAT";
        case TMM_KT_FOR:            return "FOR";
        case TMM_KT_IF:             return "IF";
        case TMM_KT_ELIF:           return "ELIF";
        case TMM_KT_ELSE:           return "ELSE";
        case TMM_KT_ENDR:           return "ENDR";
        case TMM_KT_ENDC:           return "ENDC";
        default:                    return "NONE";
    }
}
