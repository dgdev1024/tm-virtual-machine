/**
 * @file     TMM/Keyword.c
 */

#include <TMM/Keyword.h>

// Keyword Lookup Table ////////////////////////////////////////////////////////////////////////////

static const TMM_Keyword TMM_KEYWORD_TABLE[] = {
    { "ASSERT",     TMM_KT_ASSERT,      0 },
    { "ORG",        TMM_KT_ORG,         0 },
    { "ROM",        TMM_KT_ROM,         0 },
    { "RAM",        TMM_KT_RAM,         0 },
    { "DF",         TMM_KT_DF,          0 },
    { "DB",         TMM_KT_DB,          0 },
    { "DW",         TMM_KT_DW,          0 },
    { "DL",         TMM_KT_DL,          0 },
    { "DS",         TMM_KT_DS,          0 },
    { "FLOAT",      TMM_KT_DF,          0 },
    { "BYTE",       TMM_KT_DB,          0 },
    { "WORD",       TMM_KT_DW,          0 },
    { "LONG",       TMM_KT_DL,          0 },
    { "STRING",     TMM_KT_DS,          0 },
    { "ASCII",      TMM_KT_DS,          0 },
    { "INCLUDE",    TMM_KT_INCLUDE,     0 },
    { "INCBIN",     TMM_KT_INCBIN,      0 },
    { "DEF",        TMM_KT_DEF,         0 },
    { "DEFINE",     TMM_KT_DEF,         0 },
    { "MACRO",      TMM_KT_MACRO,       0 },
    { "ENDM",       TMM_KT_ENDM,        0 },
    { "NARG",       TMM_KT_NARG,        0 },
    { "_NARG",      TMM_KT_NARG,        0 },
    { "SHIFT",      TMM_KT_SHIFT,       0 },
    { "REPEAT",     TMM_KT_REPEAT,      0 },
    { "REPT",       TMM_KT_REPEAT,      0 },
    { "FOR",        TMM_KT_FOR,         0 },
    { "IF",         TMM_KT_IF,          0 },
    { "ELIF",       TMM_KT_ELIF,        0 },
    { "ELSEIF",     TMM_KT_ELIF,        0 },
    { "ELSE",       TMM_KT_ELSE,        0 },
    { "ENDR",       TMM_KT_ENDR,        0 },
    { "ENDC",       TMM_KT_ENDC,        0 },
    { "ENDIF",      TMM_KT_ENDC,        0 },
    { "A",          TMM_KT_A,           0 },
    { "AW",         TMM_KT_AW,          0 },
    { "AH",         TMM_KT_AH,          0 },
    { "AL",         TMM_KT_AL,          0 },
    { "B",          TMM_KT_B,           0 },
    { "BW",         TMM_KT_BW,          0 },
    { "BH",         TMM_KT_BH,          0 },
    { "BL",         TMM_KT_BL,          0 },
    { "C",          TMM_KT_C,           0 },
    { "CW",         TMM_KT_CW,          0 },
    { "CH",         TMM_KT_CH,          0 },
    { "CL",         TMM_KT_CL,          0 },
    { "E",          TMM_KT_E,           0 },
    { "EW",         TMM_KT_EW,          0 },
    { "EH",         TMM_KT_EH,          0 },
    { "EL",         TMM_KT_EL,          0 },
    { "NC",         TMM_KT_NC,          0 },
    { "ZS",         TMM_KT_ZS,          0 },
    { "ZC",         TMM_KT_ZC,          0 },
    { "CS",         TMM_KT_CS,          0 },
    { "CC",         TMM_KT_CC,          0 },
    { "NOP",        TMM_KT_NOP,         0 },
    { "STOP",       TMM_KT_STOP,        0 },
    { "HALT",       TMM_KT_HALT,        0 },
    { "SEC",        TMM_KT_SEC,         1 },
    { "CEC",        TMM_KT_CEC,         0 },
    { "DI",         TMM_KT_DI,          0 },
    { "EI",         TMM_KT_EI,          0 },
    { "DAA",        TMM_KT_DAA,         0 },
    { "SCF",        TMM_KT_SCF,         0 },
    { "CCF",        TMM_KT_CCF,         0 },
    { "LD",         TMM_KT_LD,          2 },
    { "LDQ",        TMM_KT_LDQ,         2 },
    { "LDH",        TMM_KT_LDH,         2 },
    { "ST",         TMM_KT_ST,          2 },
    { "STQ",        TMM_KT_STQ,         2 },
    { "STH",        TMM_KT_STH,         2 },
    { "MV",         TMM_KT_MV,          2 },
    { "MOV",        TMM_KT_MV,          2 },
    { "PUSH",       TMM_KT_PUSH,        1 },
    { "POP",        TMM_KT_POP,         1 },
    { "JMP",        TMM_KT_JMP,         2 },
    { "JP",         TMM_KT_JMP,         2 },
    { "JPB",        TMM_KT_JPB,         2 },
    { "JR",         TMM_KT_JPB,         2 },
    { "CALL",       TMM_KT_CALL,        2 },
    { "RST",        TMM_KT_RST,         1 },
    { "RET",        TMM_KT_RET,         1 },
    { "RETI",       TMM_KT_RETI,        0 },
    { "JPS",        TMM_KT_JPS,         0 },
    { "JS",         TMM_KT_JPS,         0 },
    { "INC",        TMM_KT_INC,         1 },
    { "DEC",        TMM_KT_DEC,         1 },
    { "ADD",        TMM_KT_ADD,         2 },
    { "ADC",        TMM_KT_ADC,         2 },
    { "SUB",        TMM_KT_SUB,         2 },
    { "SBC",        TMM_KT_SBC,         2 },
    { "AND",        TMM_KT_AND,         2 },
    { "OR",         TMM_KT_OR,          2 },
    { "XOR",        TMM_KT_XOR,         2 },
    { "NOT",        TMM_KT_NOT,         1 },
    { "CPL",        TMM_KT_NOT,         1 },
    { "CMP",        TMM_KT_CMP,         2 },
    { "CP",         TMM_KT_CMP,         2 },
    { "SLA",        TMM_KT_SLA,         1 },
    { "SRA",        TMM_KT_SRA,         1 },
    { "SRL",        TMM_KT_SRL,         1 },
    { "RL",         TMM_KT_RL,          1 },
    { "RLC",        TMM_KT_RLC,         1 },
    { "RR",         TMM_KT_RR,          1 },
    { "RRC",        TMM_KT_RRC,         1 },
    { "BIT",        TMM_KT_BIT,         2 },
    { "RES",        TMM_KT_RES,         2 },
    { "SET",        TMM_KT_SET,         2 },
    { "SWAP",       TMM_KT_SWAP,        1 },
    { "",           TMM_KT_NONE,        0 }
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
        case TMM_KT_ORG:            return "ORG";
        case TMM_KT_ROM:            return "ROM";
        case TMM_KT_RAM:            return "RAM";
        case TMM_KT_DB:             return "DB";
        case TMM_KT_DW:             return "DW";
        case TMM_KT_DL:             return "DL";
        case TMM_KT_DS:             return "DS";
        case TMM_KT_DF:             return "DF";
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
        case TMM_KT_NOP:            return "NOP";
        case TMM_KT_STOP:           return "STOP";
        case TMM_KT_HALT:           return "HALT";
        case TMM_KT_SEC:            return "SEC";
        case TMM_KT_CEC:            return "CEC";
        case TMM_KT_DI:             return "DI";
        case TMM_KT_EI:             return "EI";
        case TMM_KT_DAA:            return "DAA";
        case TMM_KT_SCF:            return "SCF";
        case TMM_KT_CCF:            return "CCF";
        case TMM_KT_LD:             return "LD";
        case TMM_KT_LDQ:            return "LDQ";
        case TMM_KT_LDH:            return "LDH";
        case TMM_KT_ST:             return "ST";
        case TMM_KT_STQ:            return "STQ";
        case TMM_KT_STH:            return "STH";
        case TMM_KT_MV:             return "MV";
        case TMM_KT_PUSH:           return "PUSH";
        case TMM_KT_POP:            return "POP";
        case TMM_KT_JMP:            return "JMP";
        case TMM_KT_JPB:            return "JPB";
        case TMM_KT_CALL:           return "CALL";
        case TMM_KT_RST:            return "RST";
        case TMM_KT_RET:            return "RET";
        case TMM_KT_RETI:           return "RETI";
        case TMM_KT_JPS:            return "JPS";
        case TMM_KT_INC:            return "INC";
        case TMM_KT_DEC:            return "DEC";
        case TMM_KT_ADD:            return "ADD";
        case TMM_KT_ADC:            return "ADC";
        case TMM_KT_SUB:            return "SUB";
        case TMM_KT_SBC:            return "SBC";
        case TMM_KT_AND:            return "AND";
        case TMM_KT_OR:             return "OR";
        case TMM_KT_XOR:            return "XOR";
        case TMM_KT_NOT:            return "NOT";
        case TMM_KT_CMP:            return "CMP";
        case TMM_KT_SLA:            return "SLA";
        case TMM_KT_SRA:            return "SRA";
        case TMM_KT_SRL:            return "SRL";
        case TMM_KT_RL:             return "RL";
        case TMM_KT_RLC:            return "RLC";
        case TMM_KT_RR:             return "RR";
        case TMM_KT_RRC:            return "RRC";
        case TMM_KT_BIT:            return "BIT";
        case TMM_KT_RES:            return "RES";
        case TMM_KT_SET:            return "SET";
        case TMM_KT_SWAP:           return "SWAP";
        default:                    return "NONE";
    }
}

bool TMM_IsRegisterKeyword (const TMM_Keyword* p_Keyword)
{
    return (p_Keyword != NULL && p_Keyword->m_Type >= TMM_KT_A && p_Keyword->m_Type <= TMM_KT_EL);
}

bool TMM_IsConditionKeyword (const TMM_Keyword* p_Keyword)
{
    return (p_Keyword != NULL && p_Keyword->m_Type >= TMM_KT_NC && p_Keyword->m_Type <= TMM_KT_CC);
}

bool TMM_IsInstructionKeyword (const TMM_Keyword* p_Keyword)
{
    return (p_Keyword != NULL && p_Keyword->m_Type >= TMM_KT_NOP && p_Keyword->m_Type <= TMM_KT_SWAP);
}

bool TMM_IsDataKeyword (const TMM_Keyword* p_Keyword)
{
    return (p_Keyword != NULL && p_Keyword->m_Type >= TMM_KT_DB && p_Keyword->m_Type <= TMM_KT_DF);
}
