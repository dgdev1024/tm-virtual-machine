/**
 * @file  TMM/Value.c
 */

#include <TMM/Value.h>

// Static Functions ////////////////////////////////////////////////////////////////////////////////

static TMM_Value* TMM_CreateValue (TMM_ValueType p_Type)
{
    TMM_Value* l_Value = TM_calloc(1, TMM_Value);
    TM_pexpect(l_Value != NULL, "Could not allocate memory for a runtime value");

    l_Value->m_Type = p_Type;
    return l_Value;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TMM_Value* TMM_CreateVoidValue ()
{
    return TMM_CreateValue(TMM_VT_VOID);
}

TMM_Value* TMM_CreateNumberValue (double p_Number)
{
    TMM_Value* l_Value = TMM_CreateValue(TMM_VT_NUMBER);
    
    double l_IntegerPart = 0.0;
    double l_FractionalPart = modf(p_Number, &l_IntegerPart);

    l_Value->m_Number = p_Number;
    l_Value->m_IntegerPart = (uint32_t) l_IntegerPart;
    l_Value->m_FractionalPart = (uint32_t) (l_FractionalPart * UINT32_MAX);

    return l_Value;
}

TMM_Value* TMM_CreateStringValue (const char* p_String)
{
    TM_assert(p_String != NULL);

    TMM_Value* l_Value = TMM_CreateValue(TMM_VT_STRING);

    // Get the size of the string. Allocate memory for the string and copy it.
    size_t l_Strlen = strlen(p_String);
    l_Value->m_String = TM_calloc(l_Strlen + 1, char);
    TM_pexpect(l_Value->m_String != NULL, "Could not allocate memory for a string value");

    strncpy(l_Value->m_String, p_String, l_Strlen);
    return l_Value;
}

TMM_Value* TMM_CopyValue (const TMM_Value* p_Value)
{
    if (p_Value == NULL)
    {
        TM_error("Cannot copy a null value.");
        return NULL;
    }

    switch (p_Value->m_Type)
    {
        case TMM_VT_VOID:
            return TMM_CreateVoidValue();

        case TMM_VT_NUMBER:
            return TMM_CreateNumberValue(p_Value->m_Number);

        case TMM_VT_STRING:
            return TMM_CreateStringValue(p_Value->m_String);

        default:
            return NULL;
    }
}

void TMM_DestroyValue (TMM_Value* p_Value)
{
    if (p_Value != NULL)
    {
        if (p_Value->m_Type == TMM_VT_STRING)
        {
            TM_free(p_Value->m_String);
        }

        TM_free(p_Value);
    }
}

void TMM_PrintValue (const TMM_Value* p_Value)
{
    if (p_Value == NULL)
    {
        printf("null");
        return;
    }

    switch (p_Value->m_Type)
    {
        case TMM_VT_VOID:
            printf("void");
            break;

        case TMM_VT_NUMBER:
            if (p_Value->m_FractionalPart == 0)
            {
                printf("%u", p_Value->m_IntegerPart);
            }
            else
            {
                printf("%lf", p_Value->m_Number);
            }
            break;

        case TMM_VT_STRING:
            printf("%s", p_Value->m_String);
            break;

        default:
            printf("unknown");
            break;
    }
}

void TMM_SetNumberValue (TMM_Value* p_Value, double p_Number)
{
    TM_assert(p_Value != NULL);

    if (p_Value->m_Type != TMM_VT_NUMBER)
    {
        TM_error("Cannot set a number value on a non-number value.");
        return;
    }

    double l_IntegerPart = 0.0;
    double l_FractionalPart = modf(p_Number, &l_IntegerPart);

    p_Value->m_Number = p_Number;
    p_Value->m_IntegerPart = (uint32_t) l_IntegerPart;
    p_Value->m_FractionalPart = (uint32_t) (l_FractionalPart * UINT32_MAX);
}

void TMM_SetStringValue (TMM_Value* p_Value, const char* p_String)
{
    TM_assert(p_Value != NULL && p_String != NULL);

    if (p_Value->m_Type != TMM_VT_STRING)
    {
        TM_error("Cannot set a string value on a non-string value.");
        return;
    }
    
    // Calculate the size of the new string, and see if a reallocation is necessary.
    size_t l_Strlen = strlen(p_String);
    if (l_Strlen > strlen(p_Value->m_String))
    {
        char* l_NewString = TM_realloc(p_Value->m_String, l_Strlen + 1, char);
        TM_pexpect(l_NewString != NULL, "Could not reallocate memory for a string value");
        p_Value->m_String = l_NewString;
    }

    // Copy the new string into the value.
    strncpy(p_Value->m_String, p_String, l_Strlen);
    p_Value->m_String[l_Strlen] = '\0';
}

TMM_Value* TMM_ConcatenateStringValues (const TMM_Value* p_LeftValue, const TMM_Value* p_RightValue)
{
    TM_assert(p_LeftValue != NULL && p_LeftValue->m_String != NULL);
    TM_assert(p_RightValue != NULL && p_RightValue->m_String != NULL);

    if (p_LeftValue->m_Type != TMM_VT_STRING || p_RightValue->m_Type != TMM_VT_STRING)
    {
        TM_error("Cannot concatenate non-string values.");
        return NULL;
    }

    // Get the size of the new string.
    size_t l_LeftSize = strlen(p_LeftValue->m_String);
    size_t l_RightSize = strlen(p_RightValue->m_String);
    size_t l_NewSize = l_LeftSize + l_RightSize;

    // Allocate memory for the new string.
    char* l_NewString = TM_calloc(l_NewSize + 1, char);
    TM_pexpect(l_NewString != NULL, "Could not allocate memory for a concatenated string value");

    // Copy the left and right strings into the new string.
    strncpy(l_NewString, p_LeftValue->m_String, l_LeftSize);
    strncpy(l_NewString + l_LeftSize, p_RightValue->m_String, l_RightSize);

    // Create a new string value and return it.
    TMM_Value* l_NewValue = TMM_CreateValue(TMM_VT_STRING);
    l_NewValue->m_String = l_NewString;
    
    return l_NewValue;
}