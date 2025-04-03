/**
 * @file  TMM/Value.h
 * @brief Contains functions for managing runtime values.
 */

#pragma once
#include <TM/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define TMM_STRING_INITIAL_CAPACITY 80

// Value Type Enumeration //////////////////////////////////////////////////////////////////////////

typedef enum TMM_ValueType
{
    TMM_VT_VOID,
    TMM_VT_NUMBER,
    TMM_VT_STRING
} TMM_ValueType;

// Value Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct TMM_Value
{
    TMM_ValueType m_Type;
    union
    {
        struct
        {
            double      m_Number;
            uint64_t    m_IntegerPart;
            uint64_t    m_FractionalPart;
        };

        char* m_String;
    };
} TMM_Value;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TMM_Value* TMM_CreateVoidValue ();
TMM_Value* TMM_CreateNumberValue (double p_Number);
TMM_Value* TMM_CreateStringValue (const char* p_String);
TMM_Value* TMM_CopyValue (const TMM_Value* p_Value);
void TMM_DestroyValue (TMM_Value* p_Value);
void TMM_PrintValue (const TMM_Value* p_Value);
void TMM_SetNumberValue (TMM_Value* p_Value, double p_Number);
void TMM_SetStringValue (TMM_Value* p_Value, const char* p_String);
TMM_Value* TMM_ConcatenateStringValues (const TMM_Value* p_LeftValue, const TMM_Value* p_RightValue);
