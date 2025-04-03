/**
 * @file  TMM/Builder.c
 */

#include <TMM/Lexer.h>
#include <TMM/Parser.h>
#include <TMM/Builder.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define TMM_BUILDER_INITIAL_CAPACITY 8
#define TMM_BUILDER_OUTPUT_CAPACITY 0x4000
#define TMM_BUILDER_CALL_STACK_SIZE 32

// Label Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct TMM_Label
{
    char*       m_Name;
    uint16_t*   m_References;
    size_t      m_ReferenceCount;
    size_t      m_ReferenceCapacity;
    uint16_t    m_Address;
    bool        m_Resolved;
} TMM_Label;

// Macro Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct TMM_Macro
{
    char*       m_Name;
    TMM_Syntax* m_Block;
} TMM_Macro;

// Macro Call Structure ////////////////////////////////////////////////////////////////////////////

typedef struct TMM_MacroCall
{
    TMM_Macro*  m_Macro;
    TMM_Value** m_Arguments;
    size_t      m_ArgumentCount;
    size_t      m_ArgumentOffset;
} TMM_MacroCall;

// Builder Context Structure ///////////////////////////////////////////////////////////////////////

static struct
{
    uint8_t         m_Output[TMM_BUILDER_OUTPUT_CAPACITY];
    TMM_Value*      m_Result;

    TMM_Label*      m_Labels;
    size_t          m_LabelCount;
    size_t          m_LabelCapacity;

    TMM_Macro*      m_Macros;
    size_t          m_MacroCount;
    size_t          m_MacroCapacity;

    TMM_Value**     m_DefineValues;
    char**          m_DefineKeys;
    size_t          m_DefineCount;
    size_t          m_DefineCapacity;

    TMM_MacroCall*  m_MacroCallStack[TMM_BUILDER_CALL_STACK_SIZE];
    size_t          m_MacroCallStackIndex;

    size_t          m_OutputSize;
} s_Builder = {
    .m_Output = { 0 },
    .m_Result = NULL,
    .m_Labels = NULL,
    .m_LabelCount = 0,
    .m_LabelCapacity = 0,
    .m_Macros = NULL,
    .m_MacroCount = 0,
    .m_MacroCapacity = 0,
    .m_DefineValues = NULL,
    .m_DefineKeys = NULL,
    .m_DefineCount = 0,
    .m_DefineCapacity = 0,
    .m_MacroCallStack = { 0 },
    .m_MacroCallStackIndex = 0,
    .m_OutputSize = 0
};

// Static Function Prototypes //////////////////////////////////////////////////////////////////////

static TMM_Value* TMM_Evaluate (const TMM_Syntax* p_SyntaxNode);
static TMM_Value* TMM_EvaluateBlock (const TMM_Syntax* p_SyntaxNode);

// Static Functions - Macro Call Management ////////////////////////////////////////////////////////

static TMM_MacroCall* TMM_CreateMacroCall (size_t p_ArgumentCount)
{
    TMM_MacroCall* l_MacroCall = TM_calloc(1, TMM_MacroCall);
    TM_pexpect(l_MacroCall != NULL, "Failed to allocate memory for macro call structure");

    l_MacroCall->m_Arguments = TM_calloc(p_ArgumentCount, TMM_Value*);
    TM_pexpect(l_MacroCall->m_Arguments != NULL, "Failed to allocate memory for macro call arguments");

    l_MacroCall->m_ArgumentCount = p_ArgumentCount;
    l_MacroCall->m_ArgumentOffset = 0;
    return l_MacroCall;
}

static void TMM_DestroyMacroCall (TMM_MacroCall* p_MacroCall)
{
    if (p_MacroCall != NULL)
    {
        if (p_MacroCall->m_Arguments != NULL)
        {
            for (size_t i = 0; i < p_MacroCall->m_ArgumentCount; ++i)
            {
                TMM_DestroyValue(p_MacroCall->m_Arguments[i]);
            }

            TM_free(p_MacroCall->m_Arguments);
        }

        TM_free(p_MacroCall);
    }
}

// Static Functions - Internal Array Management ////////////////////////////////////////////////////

static void TMM_ResizeLabelReferences (TMM_Label* p_Label)
{
    if (p_Label->m_ReferenceCount + 1 >= p_Label->m_ReferenceCapacity)
    {
        size_t l_NewCapacity      = p_Label->m_ReferenceCapacity * 2;
        uint16_t* l_NewReferences = TM_realloc(p_Label->m_References, l_NewCapacity, uint16_t);
        TM_pexpect(l_NewReferences != NULL, "Failed to reallocate memory for label references array");

        p_Label->m_References           = l_NewReferences;
        p_Label->m_ReferenceCapacity    = l_NewCapacity;
    }
}

static void TMM_ResizeLabelsArray ()
{
    if (s_Builder.m_LabelCount + 1 >= s_Builder.m_LabelCapacity)
    {
        size_t l_NewCapacity  = s_Builder.m_LabelCapacity * 2;
        TMM_Label* l_NewLabels = TM_realloc(s_Builder.m_Labels, l_NewCapacity, TMM_Label);
        TM_pexpect(l_NewLabels != NULL, "Failed to reallocate memory for the builder's address labels array");

        s_Builder.m_Labels          = l_NewLabels;
        s_Builder.m_LabelCapacity   = l_NewCapacity;
    }
}

static void TMM_ResizeMacrosArray ()
{
    if (s_Builder.m_MacroCount + 1 >= s_Builder.m_MacroCapacity)
    {
        size_t l_NewCapacity  = s_Builder.m_MacroCapacity * 2;
        TMM_Macro* l_NewMacros = TM_realloc(s_Builder.m_Macros, l_NewCapacity, TMM_Macro);
        TM_pexpect(l_NewMacros != NULL, "Failed to reallocate memory for the builder's macros array");

        s_Builder.m_Macros          = l_NewMacros;
        s_Builder.m_MacroCapacity   = l_NewCapacity;
    }
}

static void TMM_ResizeDefinesArrays ()
{
    if (s_Builder.m_DefineCount + 1 >= s_Builder.m_DefineCapacity)
    {
        size_t l_NewCapacity = s_Builder.m_DefineCapacity * 2;

        TMM_Value** l_NewDefineValues = TM_realloc(s_Builder.m_DefineValues, l_NewCapacity, TMM_Value*);
        TM_pexpect(l_NewDefineValues != NULL, "Failed to reallocate memory for the builder's define values array");

        char** l_NewDefineKeys = TM_realloc(s_Builder.m_DefineKeys, l_NewCapacity, char*);
        TM_pexpect(l_NewDefineKeys != NULL, "Failed to reallocate memory for the builder's define keys array");

        s_Builder.m_DefineValues    = l_NewDefineValues;
        s_Builder.m_DefineKeys      = l_NewDefineKeys;
        s_Builder.m_DefineCapacity  = l_NewCapacity;
    }
}

// Static Functions - Output Buffer Management /////////////////////////////////////////////////////

static bool TMM_DefineByte (uint8_t p_Value)
{
    if (s_Builder.m_OutputSize + 1 >= TMM_BUILDER_OUTPUT_CAPACITY)
    {
        TM_error("Output buffer overflowed while defining a byte.");
        return false;
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = p_Value;
    return true;
}

static bool TMM_DefineWord (uint16_t p_Value)
{
    if (s_Builder.m_OutputSize + 2 >= TMM_BUILDER_OUTPUT_CAPACITY)
    {
        TM_error("Output buffer overflowed while defining a word.");
        return false;
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = (uint8_t) (p_Value & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (uint8_t) ((p_Value >> 8) & 0xFF);
    return true;
}

static bool TMM_DefineLong (uint32_t p_Value)
{
    if (s_Builder.m_OutputSize + 4 >= TMM_BUILDER_OUTPUT_CAPACITY)
    {
        TM_error("Output buffer overflowed while defining a long.");
        return false;
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = (uint8_t) (p_Value & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (uint8_t) ((p_Value >> 8) & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (uint8_t) ((p_Value >> 16) & 0xFF);
    s_Builder.m_Output[s_Builder.m_OutputSize++] = (uint8_t) ((p_Value >> 24) & 0xFF);
    return true;
}

static bool TMM_DefineStringASCII (const char* p_String)
{
    size_t l_Length = strlen(p_String);
    if (s_Builder.m_OutputSize + l_Length + 1 >= TMM_BUILDER_OUTPUT_CAPACITY)
    {
        TM_error("Output buffer overflowed while defining an ASCII string.");
        return false;
    }

    for (size_t i = 0; i < l_Length; ++i)
    {
        s_Builder.m_Output[s_Builder.m_OutputSize++] = (uint8_t) p_String[i];
    }

    s_Builder.m_Output[s_Builder.m_OutputSize++] = 0;

    return true;
}

static bool TMM_DefineBinaryFile (const char* p_Filename, size_t p_Offset, size_t p_Length)
{
    // Attempt to open the binary file for reading.
    FILE* l_File = fopen(p_Filename, "rb");
    if (l_File == NULL)
    {
        TM_perror("Failed to open included binary file '%s' for reading", p_Filename);
        return false;
    }

    // Attempt to get and validate the file size.
    fseek(l_File, 0, SEEK_END);
    int64_t l_SignedFilesize = ftell(l_File);
    if (l_SignedFilesize < 0)
    {
        TM_perror("Failed to get the size of included binary file '%s'", p_Filename);
        fclose(l_File);
        return false;
    }
    size_t l_Filesize = (size_t) l_SignedFilesize;
    rewind(l_File);

    // Correct the length according to the file's size and the provided offset.
    if (p_Length == 0)
    {
        p_Length = l_Filesize - p_Offset;
    }
    else if (p_Offset + p_Length > l_Filesize)
    {
        TM_error("Attempted to read past the end of included binary file '%s'", p_Filename);
        fclose(l_File);
        return false;
    }

    // Ensure the output buffer has enough space for the binary data.
    if (s_Builder.m_OutputSize + p_Length >= TMM_BUILDER_OUTPUT_CAPACITY)
    {
        TM_error("Output buffer overflowed while including a binary file.");
        fclose(l_File);
        return false;
    }

    // Read the binary data from the file into the output buffer.
    fseek(l_File, p_Offset, SEEK_SET);
    fread(s_Builder.m_Output + s_Builder.m_OutputSize, 1, p_Length, l_File);
    if (ferror(l_File) && !feof(l_File))
    {
        TM_perror("Failed to read included binary file '%s'", p_Filename);
        fclose(l_File);
        return false;
    }

    s_Builder.m_OutputSize += p_Length;

    // Close the file and return success.
    fclose(l_File);
    return true;
}

// Static Functions - Assignment Operations ////////////////////////////////////////////////////////

static TMM_Value* TMM_PerformAssignmentOperation (const TMM_Value* p_LeftValue,
    const TMM_Value* p_RightValue, TMM_TokenType p_Operator)
{
    // If the operator is '=', then just return a copy of the right value.
    if (p_Operator == TMM_TOKEN_ASSIGN_EQUAL)
    {
        return TMM_CopyValue(p_RightValue);
    }

    // Check the type of the lefthand value.
    if (p_LeftValue->m_Type == TMM_VT_NUMBER)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == TMM_VT_NUMBER)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case TMM_TOKEN_ASSIGN_PLUS:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number + p_RightValue->m_Number);
                case TMM_TOKEN_ASSIGN_MINUS:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number - p_RightValue->m_Number);
                case TMM_TOKEN_ASSIGN_MULTIPLY:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number * p_RightValue->m_Number);
                case TMM_TOKEN_ASSIGN_DIVIDE:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        TM_error("Encountered attempted division by zero.");
                        return NULL;
                    }
                    return TMM_CreateNumberValue(p_LeftValue->m_Number / p_RightValue->m_Number);
                case TMM_TOKEN_ASSIGN_MODULO:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        TM_error("Encountered modulo with attempted division by zero.");
                        return NULL;
                    }
                    return TMM_CreateNumberValue(fmod(p_LeftValue->m_Number, p_RightValue->m_Number));
                case TMM_TOKEN_ASSIGN_EXPONENT:
                    return TMM_CreateNumberValue(pow(p_LeftValue->m_Number, p_RightValue->m_Number));
                case TMM_TOKEN_ASSIGN_BITWISE_AND:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart & p_RightValue->m_IntegerPart);
                case TMM_TOKEN_ASSIGN_BITWISE_OR:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart | p_RightValue->m_IntegerPart);
                case TMM_TOKEN_ASSIGN_BITWISE_XOR:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart ^ p_RightValue->m_IntegerPart);
                case TMM_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart << p_RightValue->m_IntegerPart);
                case TMM_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart >> p_RightValue->m_IntegerPart);
                default:
                    TM_error("Invalid operator type for number-vs-number assignment operation.");
                    return NULL;
            }
        }
        else
        {
            TM_error("Invalid righthand value type for assignment operation.");
            return NULL;
        }
    }
    else if (p_LeftValue->m_Type == TMM_VT_STRING)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == TMM_VT_STRING)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case TMM_TOKEN_ASSIGN_PLUS:
                {
                    return TMM_ConcatenateStringValues(p_LeftValue, p_RightValue);
                }
                default:
                    TM_error("Invalid operator type for string-vs-string assignment operation.");
                    return NULL;
            }
        }
        else
        {
            TM_error("Invalid righthand value type for assignment operation.");
            return NULL;
        }
    }
    else
    {
        TM_error("Invalid lefthand value type for assignment operation.");
        return NULL;
    }
}

// Static Functions - Binary Operations ////////////////////////////////////////////////////////////

static TMM_Value* TMM_PerformBinaryOperation (const TMM_Value* p_LeftValue,
    const TMM_Value* p_RightValue, TMM_TokenType p_Operator)
{
    // Check the type of the lefthand value.
    if (p_LeftValue->m_Type == TMM_VT_NUMBER)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == TMM_VT_NUMBER)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case TMM_TOKEN_PLUS:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number + p_RightValue->m_Number);
                case TMM_TOKEN_MINUS:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number - p_RightValue->m_Number);
                case TMM_TOKEN_MULTIPLY:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number * p_RightValue->m_Number);
                case TMM_TOKEN_DIVIDE:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        TM_error("Encountered attempted division by zero.");
                        return NULL;
                    }
                    return TMM_CreateNumberValue(p_LeftValue->m_Number / p_RightValue->m_Number);
                case TMM_TOKEN_MODULO:
                    if (p_RightValue->m_IntegerPart == 0 && p_RightValue->m_FractionalPart == 0)
                    {
                        TM_error("Encountered modulo with attempted division by zero.");
                        return NULL;
                    }

                    return TMM_CreateNumberValue(fmod(p_LeftValue->m_Number, p_RightValue->m_Number));
                case TMM_TOKEN_EXPONENT:
                    return TMM_CreateNumberValue(pow(p_LeftValue->m_Number, p_RightValue->m_Number));
                case TMM_TOKEN_BITWISE_AND:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart & p_RightValue->m_IntegerPart);
                case TMM_TOKEN_BITWISE_OR:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart | p_RightValue->m_IntegerPart);
                case TMM_TOKEN_BITWISE_XOR:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart ^ p_RightValue->m_IntegerPart);
                case TMM_TOKEN_BITWISE_SHIFT_LEFT:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart << p_RightValue->m_IntegerPart);
                case TMM_TOKEN_BITWISE_SHIFT_RIGHT:
                    return TMM_CreateNumberValue(p_LeftValue->m_IntegerPart >> p_RightValue->m_IntegerPart);
                case TMM_TOKEN_LOGICAL_AND:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number && p_RightValue->m_Number);
                case TMM_TOKEN_LOGICAL_OR:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number || p_RightValue->m_Number);
                case TMM_TOKEN_COMPARE_EQUAL:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number == p_RightValue->m_Number);
                case TMM_TOKEN_COMPARE_NOT_EQUAL:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number != p_RightValue->m_Number);
                case TMM_TOKEN_COMPARE_LESS:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number < p_RightValue->m_Number);
                case TMM_TOKEN_COMPARE_LESS_EQUAL:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number <= p_RightValue->m_Number);
                case TMM_TOKEN_COMPARE_GREATER:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number > p_RightValue->m_Number);
                case TMM_TOKEN_COMPARE_GREATER_EQUAL:
                    return TMM_CreateNumberValue(p_LeftValue->m_Number >= p_RightValue->m_Number);
                default:
                    TM_error("Invalid operator type for number-vs-number binary operation.");
                    return NULL;
            }
        }
        else if (p_RightValue->m_Type == TMM_VT_STRING)
        {
            // Convert the number to a string.
            char l_Buffer[32];
            if (p_LeftValue->m_FractionalPart == 0)
            {
                snprintf(l_Buffer, sizeof(l_Buffer), "%ld", p_LeftValue->m_IntegerPart);
            }
            else
            {
                snprintf(l_Buffer, sizeof(l_Buffer), "%lf", p_LeftValue->m_Number);
            }

            switch (p_Operator)
            {
                case TMM_TOKEN_PLUS:
                {
                    TMM_Value* l_LeftValueString = TMM_CreateStringValue(l_Buffer);
                    TMM_Value* l_Result = TMM_ConcatenateStringValues(l_LeftValueString, p_RightValue);
                    TMM_DestroyValue(l_LeftValueString);
                    return l_Result;
                }
                default:
                    TM_error("Invalid operator type for number-vs-string binary operation.");
                    return NULL;
            }
        }
        else
        {
            TM_error("Invalid righthand value type for binary operation.");
            return NULL;
        }
    }
    else if (p_LeftValue->m_Type == TMM_VT_STRING)
    {
        // Check the type of the righthand value.
        if (p_RightValue->m_Type == TMM_VT_STRING)
        {
            // Check the operator type and perform the proper operation.
            switch (p_Operator)
            {
                case TMM_TOKEN_PLUS:
                {
                    return TMM_ConcatenateStringValues(p_LeftValue, p_RightValue);
                }
                default:
                    TM_error("Invalid operator type for string-vs-string binary operation.");
                    return NULL;
            }
        }
        else if (p_RightValue->m_Type == TMM_VT_NUMBER)
        {
            // Convert the number to a string.
            char l_Buffer[32];
            if (p_RightValue->m_FractionalPart == 0)
            {
                snprintf(l_Buffer, sizeof(l_Buffer), "%ld", p_RightValue->m_IntegerPart);
            }
            else
            {
                snprintf(l_Buffer, sizeof(l_Buffer), "%lf", p_RightValue->m_Number);
            }

            switch (p_Operator)
            {
                case TMM_TOKEN_PLUS:
                {
                    TMM_Value* l_RightValueString = TMM_CreateStringValue(l_Buffer);
                    TMM_Value* l_Result = TMM_ConcatenateStringValues(p_LeftValue, l_RightValueString);
                    TMM_DestroyValue(l_RightValueString);
                    return l_Result;
                }
                default:
                    TM_error("Invalid operator type for string-vs-number binary operation.");
                    return NULL;
            }
        }
        else
        {
            TM_error("Invalid righthand value type for binary operation.");
            return NULL;
        }
    }
    else
    {
        TM_error("Invalid lefthand value type for binary operation.");
        return NULL;
    }
}

// Static Functions - Unary Operations /////////////////////////////////////////////////////////////

static TMM_Value* TMM_PerformUnaryOperation (const TMM_Value* p_Value, TMM_TokenType p_Operator)
{
    // Check the type of the value.
    if (p_Value->m_Type == TMM_VT_NUMBER)
    {
        // Check the operator type and perform the proper operation.
        switch (p_Operator)
        {
            case TMM_TOKEN_PLUS:
                return TMM_CreateNumberValue(p_Value->m_Number);
            case TMM_TOKEN_MINUS:
                return TMM_CreateNumberValue(-p_Value->m_Number);
            case TMM_TOKEN_LOGICAL_NOT:
                return TMM_CreateNumberValue(!p_Value->m_Number);
            case TMM_TOKEN_BITWISE_NOT:
                return TMM_CreateNumberValue(~p_Value->m_IntegerPart);
            default:
                TM_error("Invalid operator type for number unary operation.");
                return NULL;
        }
    }
    else
    {
        TM_error("Invalid value type for unary operation.");
        return NULL;
    }
}

// Static Functions - Evaluation ///////////////////////////////////////////////////////////////////

static TMM_Value* TMM_EvaluateString (const TMM_Syntax* p_SyntaxNode)
{
    TMM_Value* l_Value = TMM_CreateStringValue(p_SyntaxNode->m_String);
    return l_Value;
}

static TMM_Value* TMM_EvaluateNumber (const TMM_Syntax* p_SyntaxNode)
{
    TMM_Value* l_Value = TMM_CreateNumberValue(p_SyntaxNode->m_Number);
    return l_Value;
}

static TMM_Value* TMM_EvaluateBinaryExpression (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the left-hand side of the expression.
    TMM_Value* l_LeftValue = TMM_Evaluate(p_SyntaxNode->m_LeftExpr);
    if (l_LeftValue == NULL)
    {
        return NULL;
    }

    // Evaluate the right-hand side of the expression.
    TMM_Value* l_RightValue = TMM_Evaluate(p_SyntaxNode->m_RightExpr);
    if (l_RightValue == NULL)
    {
        TMM_DestroyValue(l_LeftValue);
        return NULL;
    }

    // Perform the binary operation.
    TMM_Value* l_Result = TMM_PerformBinaryOperation(l_LeftValue, l_RightValue, p_SyntaxNode->m_Operator);
    TMM_DestroyValue(l_LeftValue);
    TMM_DestroyValue(l_RightValue);

    return l_Result;
}

static TMM_Value* TMM_EvaluateUnaryExpression (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the operand of the expression.
    TMM_Value* l_OperandValue = TMM_Evaluate(p_SyntaxNode->m_RightExpr);
    if (l_OperandValue == NULL)
    {
        return NULL;
    }

    // Perform the unary operation.
    TMM_Value* l_Result = TMM_PerformUnaryOperation(l_OperandValue, p_SyntaxNode->m_Operator);
    TMM_DestroyValue(l_OperandValue);

    return l_Result;
}

static TMM_Value* TMM_EvaluateIdentifier (const TMM_Syntax* p_SyntaxNode)
{
    // Check if the identifier is a reference to a defined value.
    for (size_t i = 0; i < s_Builder.m_DefineCount; ++i)
    {
        if (strcmp(s_Builder.m_DefineKeys[i], p_SyntaxNode->m_String) == 0)
        {
            return TMM_CopyValue(s_Builder.m_DefineValues[i]);
        }
    }

    // The identifier must be a reference to a label.
    TMM_Label* l_Label = NULL;

    // Find the label by name.
    for (size_t i = 0; i < s_Builder.m_LabelCount; ++i)
    {
        if (strcmp(s_Builder.m_Labels[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            l_Label = &s_Builder.m_Labels[i];
            break;
        }
    }

    // Was the label found?
    if (l_Label == NULL)
    {
        // Create a new unresolved label.
        TMM_ResizeLabelsArray();

        // Allocate memory for the label's string name.
        size_t  l_LabelStrlen = strlen(p_SyntaxNode->m_String);
        char* l_LabelName = TM_calloc(l_LabelStrlen + 1, char);
        TM_pexpect(l_LabelName != NULL, "Failed to allocate memory for label name string");
        strncpy(l_LabelName, p_SyntaxNode->m_String, l_LabelStrlen);

        // Allocate memory for the label's references array.
        uint16_t* l_LabelReferences = TM_calloc(TMM_BUILDER_INITIAL_CAPACITY, uint16_t);
        TM_pexpect(l_LabelReferences != NULL, "Failed to allocate memory for label references array");

        // Point to the next available label.
        l_Label = &s_Builder.m_Labels[s_Builder.m_LabelCount++];
        l_Label->m_Name = l_LabelName;
        l_Label->m_References = l_LabelReferences;
        l_Label->m_ReferenceCount = 0;
        l_Label->m_ReferenceCapacity = TMM_BUILDER_INITIAL_CAPACITY;
        l_Label->m_Address = 0;
        l_Label->m_Resolved = false;
    }

    // Add the current output size as a reference to the label.
    TMM_ResizeLabelReferences(l_Label);
    l_Label->m_References[l_Label->m_ReferenceCount++] = s_Builder.m_OutputSize;

    // Return a number value with the address of the label if it has been resolved.
    // Otherwise, return a number value with the value 0.
    return (l_Label->m_Resolved == true) ?
        TMM_CreateNumberValue((double) l_Label->m_Address) :
        TMM_CreateNumberValue(0);
}

static TMM_Value* TMM_EvaluateLabel (const TMM_Syntax* p_SyntaxNode)
{
    // Check if the label has already been defined.
    TMM_Label* l_Label = NULL;
    for (size_t i = 0; i < s_Builder.m_LabelCount; ++i)
    {
        if (strcmp(s_Builder.m_Labels[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            l_Label = &s_Builder.m_Labels[i];
            break;
        }
    }

    // Has the label been defined?
    if (l_Label == NULL)
    {
        // Resize the labels array.
        TMM_ResizeLabelsArray();

        // Allocate memory for the label's string name.
        size_t  l_LabelStrlen = strlen(p_SyntaxNode->m_String);
        char* l_LabelName = TM_calloc(l_LabelStrlen + 1, char);
        TM_pexpect(l_LabelName != NULL, "Failed to allocate memory for label name string");

        // Allocate memory for the label's references array.
        uint16_t* l_LabelReferences = TM_calloc(TMM_BUILDER_INITIAL_CAPACITY, uint16_t);
        TM_pexpect(l_LabelReferences != NULL, "Failed to allocate memory for label references array");

        // Point to the next available label.
        l_Label = &s_Builder.m_Labels[s_Builder.m_LabelCount++];
        l_Label->m_Name = l_LabelName;
        l_Label->m_References = l_LabelReferences;
        l_Label->m_ReferenceCount = 0;
        l_Label->m_ReferenceCapacity = TMM_BUILDER_INITIAL_CAPACITY;
        
        // Label is resolved to the current output size.
        l_Label->m_Address = s_Builder.m_OutputSize;
        l_Label->m_Resolved = true;
    }
    else
    {
        // Set the label's resolved address to the current output size.
        l_Label->m_Address = s_Builder.m_OutputSize;

        // Has the label not been resolved?
        if (l_Label->m_Resolved == false)
        {
            // Resolve it now.
            l_Label->m_Resolved = true;
            for (size_t i = 0; i < l_Label->m_ReferenceCount; ++i)
            {
                uint16_t l_Reference = l_Label->m_References[i];
                s_Builder.m_Output[l_Reference] = (uint8_t) (l_Label->m_Address & 0xFF);
                s_Builder.m_Output[l_Reference + 1] = (uint8_t) ((l_Label->m_Address >> 8) & 0xFF);
            }
        }
    }

    return TMM_CreateVoidValue();
}

static TMM_Value* TMM_EvaluateData (const TMM_Syntax* p_SyntaxNode)
{
    // The type of data being defined is hinted in the syntax node's stored keyword type.
    switch (p_SyntaxNode->m_KeywordType)
    {
        case TMM_KT_DB: // Define Bytes
        {
            // Evaluate each expression in the data syntax node.
            for (size_t i = 0; i < p_SyntaxNode->m_BodySize; ++i)
            {
                TMM_Value* l_Value = TMM_Evaluate(p_SyntaxNode->m_Body[i]);
                if (l_Value == NULL)
                {
                    return NULL;
                }

                // Check the type of the value. It can be a number or a string.
                if (l_Value->m_Type == TMM_VT_NUMBER)
                {
                    if (l_Value->m_IntegerPart > 0xFF)
                    {
                        TM_warn("Value '%lu' is too large to fit in a byte, and will be truncated.", l_Value->m_IntegerPart);
                    }

                    if (TMM_DefineByte(l_Value->m_IntegerPart & 0xFF) == false)
                    {
                        TMM_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else if (l_Value->m_Type == TMM_VT_STRING)
                {
                    if (TMM_DefineStringASCII(l_Value->m_String) == false)
                    {
                        TMM_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else
                {
                    TM_error("Unexpected value type in 'db' statement.");
                    TMM_DestroyValue(l_Value);
                    return NULL;
                }

                TMM_DestroyValue(l_Value);
            }
            break;
        }
        case TMM_KT_DW: // Define Words
        {
            // Evaluate each expression in the data syntax node.
            for (size_t i = 0; i < p_SyntaxNode->m_BodySize; ++i)
            {
                TMM_Value* l_Value = TMM_Evaluate(p_SyntaxNode->m_Body[i]);
                if (l_Value == NULL)
                {
                    return NULL;
                }

                // Check the type of the value. It must be a number.
                if (l_Value->m_Type == TMM_VT_NUMBER)
                {
                    if (l_Value->m_IntegerPart > 0xFFFF)
                    {
                        TM_warn("Value '%lu' is too large to fit in a word, and will be truncated.", l_Value->m_IntegerPart);
                    }

                    if (TMM_DefineWord(l_Value->m_IntegerPart & 0xFFFF) == false)
                    {
                        TMM_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else
                {
                    TM_error("Unexpected value type in 'dw' statement.");
                    TMM_DestroyValue(l_Value);
                    return NULL;
                }

                TMM_DestroyValue(l_Value);
            }
            break;
        }
        case TMM_KT_DL: // Define Longs
        {
            // Evaluate each expression in the data syntax node.
            for (size_t i = 0; i < p_SyntaxNode->m_BodySize; ++i)
            {
                TMM_Value* l_Value = TMM_Evaluate(p_SyntaxNode->m_Body[i]);
                if (l_Value == NULL)
                {
                    return NULL;
                }

                // Check the type of the value. It must be a number.
                if (l_Value->m_Type == TMM_VT_NUMBER)
                {
                    if (TMM_DefineLong(l_Value->m_IntegerPart & 0xFFFFFFFF) == false)
                    {
                        TMM_DestroyValue(l_Value);
                        return NULL;
                    }
                }
                else
                {
                    TM_error("Unexpected value type in 'dl' statement.");
                    TMM_DestroyValue(l_Value);
                    return NULL;
                }

                TMM_DestroyValue(l_Value);
            }
            break;
        }
        case TMM_KT_DS: // Define Sequence (of bytes)
        {
            // The `ds` statement requires a count expression followed by one or more data expressions.
            // It defines the sequence of bytes after the count expression a certain number of times
            // equal to the value of the count expression.

            // Evaluate the count expression.
            TMM_Value* l_CountValue = TMM_Evaluate(p_SyntaxNode->m_CountExpr);
            if (l_CountValue == NULL)
            {
                return NULL;
            }

            // Check the type of the value. It must be a number.
            if (l_CountValue->m_Type != TMM_VT_NUMBER)
            {
                TM_error("Unexpected value type for count expression in 'ds' statement.");
                TMM_DestroyValue(l_CountValue);
                return NULL;
            }
            
            // Evaluate each expression in the data syntax node. Repeat the sequence of bytes
            // as many times as the value of the count expression.
            uint64_t l_Count = l_CountValue->m_IntegerPart;
            TMM_DestroyValue(l_CountValue);

            while (l_Count > 0)
            {
                for (size_t i = 0; i < p_SyntaxNode->m_BodySize; ++i)
                {
                    TMM_Value* l_Value = TMM_Evaluate(p_SyntaxNode->m_Body[i]);
                    if (l_Value == NULL)
                    {
                        return NULL;
                    }

                    // Check the type of the value. It can be a number or a string.
                    if (l_Value->m_Type == TMM_VT_NUMBER)
                    {
                        if (l_Value->m_IntegerPart > 0xFF)
                        {
                            TM_warn("Value '%lu' is too large to fit in a byte, and will be truncated.", l_Value->m_IntegerPart);
                        }

                        if (TMM_DefineByte(l_Value->m_IntegerPart & 0xFF) == false)
                        {
                            TMM_DestroyValue(l_Value);
                            return NULL;
                        }
                    }
                    else if (l_Value->m_Type == TMM_VT_STRING)
                    {
                        if (TMM_DefineStringASCII(l_Value->m_String) == false)
                        {
                            TMM_DestroyValue(l_Value);
                            return NULL;
                        }
                    }
                    else
                    {
                        TM_error("Unexpected value type in 'ds' statement.");
                        TMM_DestroyValue(l_Value);
                        return NULL;
                    }

                    TMM_DestroyValue(l_Value);
                }

                l_Count--;
            }

            break;
        }
        default:
            TM_error("Unexpected keyword type for data syntax node.");
            return NULL;
    }

    return TMM_CreateVoidValue();
}

static TMM_Value* TMM_EvaluateDefine (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the value of the define statement.
    TMM_Value* l_Value = TMM_Evaluate(p_SyntaxNode->m_RightExpr);
    if (l_Value == NULL)
    {
        return NULL;
    }

    // Check to see if the define already exists.
    TMM_Value** l_ExistingValue = NULL;
    for (size_t i = 0; i < s_Builder.m_DefineCount; ++i)
    {
        if (strcmp(s_Builder.m_DefineKeys[i], p_SyntaxNode->m_String) == 0)
        {
            l_ExistingValue = &s_Builder.m_DefineValues[i];
            break;
        }
    }

    // If the define already exists, then perform an assignment operation.
    if (l_ExistingValue != NULL)
    {
        // Perform the assignment operation and store the result.
        TMM_Value* l_Result = TMM_PerformAssignmentOperation(*l_ExistingValue, l_Value, p_SyntaxNode->m_Operator);
        if (l_Result == NULL)
        {
            TMM_DestroyValue(l_Value);
            return NULL;
        }

        // Replace the existing value with the result of the assignment operation.
        TMM_DestroyValue(*l_ExistingValue);
        *l_ExistingValue = l_Result;
    }
    else
    {
        // Resize the defines arrays.
        TMM_ResizeDefinesArrays();

        // Allocate memory for the define key string.
        size_t  l_DefineStrlen = strlen(p_SyntaxNode->m_String);
        char* l_DefineKey = TM_calloc(l_DefineStrlen + 1, char);
        TM_pexpect(l_DefineKey != NULL, "Failed to allocate memory for define key string");
        strncpy(l_DefineKey, p_SyntaxNode->m_String, l_DefineStrlen);

        // Point to the next available define.
        s_Builder.m_DefineKeys[s_Builder.m_DefineCount] = l_DefineKey;
        s_Builder.m_DefineValues[s_Builder.m_DefineCount] = TMM_CopyValue(l_Value);
        s_Builder.m_DefineCount++;
    }

    TMM_DestroyValue(l_Value);
    return TMM_CreateVoidValue();
}

static TMM_Value* TMM_EvaluateMacroDefinition (const TMM_Syntax* p_SyntaxNode)
{
    // Check if the macro has already been defined.
    for (size_t i = 0; i < s_Builder.m_MacroCount; ++i)
    {
        if (strcmp(s_Builder.m_Macros[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            TM_error("Macro '%s' has already been defined.", p_SyntaxNode->m_String);
            return NULL;
        }
    }

    // Resize the macros array.
    TMM_ResizeMacrosArray();

    // Copy the name of the macro.
    size_t  l_NameStrlen = strlen(p_SyntaxNode->m_String);
    char* l_Name = TM_calloc(l_NameStrlen + 1, char);
    TM_pexpect(l_Name != NULL, "Failed to allocate memory for macro name string");
    strncpy(l_Name, p_SyntaxNode->m_String, l_NameStrlen);

    // Resize the macros array.
    TMM_Macro* l_Macro = &s_Builder.m_Macros[s_Builder.m_MacroCount++];
    l_Macro->m_Name = l_Name;
    l_Macro->m_Block = TMM_CopySyntax(p_SyntaxNode->m_LeftExpr);

    return TMM_CreateVoidValue();
}

static TMM_Value* TMM_EvaluateMacroCall (const TMM_Syntax* p_SyntaxNode)
{
    // Find the macro by name.
    TMM_Macro* l_Macro = NULL;
    for (size_t i = 0; i < s_Builder.m_MacroCount; ++i)
    {
        if (strcmp(s_Builder.m_Macros[i].m_Name, p_SyntaxNode->m_String) == 0)
        {
            l_Macro = &s_Builder.m_Macros[i];
            break;
        }
    }

    // Was the macro found?
    if (l_Macro == NULL)
    {
        TM_error("Macro '%s' was not found.", p_SyntaxNode->m_String);
        return NULL;
    }

    // Check the macro call stack. Is it full?
    if (s_Builder.m_MacroCallStackIndex >= TMM_BUILDER_CALL_STACK_SIZE)
    {
        TM_error("Macro call stack overflowed.");
        return NULL;
    }

    // Set up the macro call context.
    TMM_MacroCall* l_Call = TMM_CreateMacroCall(p_SyntaxNode->m_BodySize);
    s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex++] = l_Call;

    // Evaluate the macro call's arguments.
    for (size_t i = 0; i < p_SyntaxNode->m_BodySize; ++i)
    {
        TMM_Value* l_Value = TMM_Evaluate(p_SyntaxNode->m_Body[i]);
        if (l_Value == NULL)
        {
            TMM_DestroyMacroCall(l_Call);
            s_Builder.m_MacroCallStack[--s_Builder.m_MacroCallStackIndex] = NULL;
            return NULL;
        }

        l_Call->m_Arguments[i] = l_Value;
    }

    // Evaluate the macro block.
    TMM_Value* l_Result = TMM_EvaluateBlock(l_Macro->m_Block);
    if (l_Result == NULL)
    {
        TMM_DestroyMacroCall(l_Call);
        s_Builder.m_MacroCallStack[--s_Builder.m_MacroCallStackIndex] = NULL;
        return NULL;
    }

    // Clean up the macro call context.
    TMM_DestroyMacroCall(l_Call);
    s_Builder.m_MacroCallStack[--s_Builder.m_MacroCallStackIndex] = NULL;
    
    return l_Result;
}

TMM_Value* TMM_EvaluateNARGExpression (const TMM_Syntax* p_Syntax)
{
    // Make sure the macro call stack is not empty.
    if (s_Builder.m_MacroCallStackIndex == 0)
    {
        TM_error("NARG syntax outside of a macro call.");
        return NULL;
    }

    // Get the macro call context at the top of the stack.
    TMM_MacroCall* l_Call = s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex - 1];

    // Return a number value with the number of arguments in the macro call.
    return TMM_CreateNumberValue((double) l_Call->m_ArgumentCount);
}

TMM_Value* TMM_EvaluateMacroArgument (const TMM_Syntax* p_Syntax)
{
    // Make sure the macro call stack is not empty.
    if (s_Builder.m_MacroCallStackIndex == 0)
    {
        TM_error("Macro argument syntax outside of a macro call.");
        return NULL;
    }

    // Get the macro call context at the top of the stack.
    TMM_MacroCall* l_Call = s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex - 1];

    // Check the index of the argument. Macro arguments are 1-indexed ('\1' is the first argument).
    size_t l_ArgIndex = (size_t) p_Syntax->m_Number;

    // Offset the argument index by the shift offset.
    l_ArgIndex += l_Call->m_ArgumentOffset;

    if (l_ArgIndex < 1 || l_ArgIndex > l_Call->m_ArgumentCount)
    {
        TM_error("Macro argument index %zu out of range.", l_ArgIndex);
        return NULL;
    }

    // Return a copy of the argument value.
    return TMM_CopyValue(l_Call->m_Arguments[l_ArgIndex - 1]);
}

TMM_Value* TMM_EvaluateShiftStatement (const TMM_Syntax* p_SyntaxNode)
{
    // Make sure the macro call stack is not empty.
    if (s_Builder.m_MacroCallStackIndex == 0)
    {
        TM_error("Shift syntax outside of a macro call.");
        return NULL;
    }

    // Get the macro call context at the top of the stack.
    TMM_MacroCall* l_Call = s_Builder.m_MacroCallStack[s_Builder.m_MacroCallStackIndex - 1];

    // Evaluate the shift expression.
    TMM_Value* l_ShiftValue = TMM_Evaluate(p_SyntaxNode->m_CountExpr);
    if (l_ShiftValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a number.
    if (l_ShiftValue->m_Type != TMM_VT_NUMBER)
    {
        TM_error("Unexpected value type for shift expression in 'shift' statement.");
        TMM_DestroyValue(l_ShiftValue);
        return NULL;
    }

    // Shift the argument offset by the value of the shift expression.
    l_Call->m_ArgumentOffset += (size_t) l_ShiftValue->m_IntegerPart;
    TMM_DestroyValue(l_ShiftValue);

    return TMM_CreateVoidValue();
}

TMM_Value* TMM_EvaluateRepeatStatement (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the count expression.
    TMM_Value* l_CountValue = TMM_Evaluate(p_SyntaxNode->m_CountExpr);
    if (l_CountValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a number.
    if (l_CountValue->m_Type != TMM_VT_NUMBER)
    {
        TM_error("Unexpected value type for count expression in 'repeat' statement.");
        TMM_DestroyValue(l_CountValue);
        return NULL;
    }

    // Evaluate the block expression.
    TMM_Value* l_Result = NULL;
    uint64_t l_Count = l_CountValue->m_IntegerPart;
    TMM_DestroyValue(l_CountValue);

    while (l_Count > 0)
    {
        l_Result = TMM_EvaluateBlock(p_SyntaxNode->m_LeftExpr);
        if (l_Result == NULL)
        {
            return NULL;
        }

        TMM_DestroyValue(l_Result);
        l_Count--;
    }

    return TMM_CreateVoidValue();
}

TMM_Value* TMM_EvaluateIfStatement (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the condition expression.
    TMM_Value* l_ConditionValue = TMM_Evaluate(p_SyntaxNode->m_CondExpr);
    if (l_ConditionValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a number.
    if (l_ConditionValue->m_Type != TMM_VT_NUMBER)
    {
        TM_error("Unexpected value type for condition expression in 'if' statement.");
        TMM_DestroyValue(l_ConditionValue);
        return NULL;
    }

    // Evaluate the block expression.
    TMM_Value* l_Result = NULL;
    if (l_ConditionValue->m_Number != 0)
    {
        l_Result = TMM_Evaluate(p_SyntaxNode->m_LeftExpr);
    }
    else if (p_SyntaxNode->m_RightExpr != NULL)
    {
        l_Result = TMM_Evaluate(p_SyntaxNode->m_RightExpr);
    }

    TMM_DestroyValue(l_ConditionValue);
    return l_Result;
}

TMM_Value* TMM_EvaluateIncludeStatement (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the string expression.
    TMM_Value* l_StringValue = TMM_Evaluate(p_SyntaxNode->m_LeftExpr);
    if (l_StringValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a string.
    if (l_StringValue->m_Type != TMM_VT_STRING)
    {
        TM_error("Unexpected value type for string expression in 'include' statement.");
        TMM_DestroyValue(l_StringValue);
        return NULL;
    }
    
    // Prepare the lexer for the new file.
    TMM_ResetLexer();
    if (TMM_LexFile(l_StringValue->m_String) == false)
    {
        TMM_DestroyValue(l_StringValue);
        return NULL;
    }

    // Parse the lexed tokens.
    TMM_Syntax* l_Syntax = TMM_CreateSyntax(TMM_ST_BLOCK, TMM_PeekToken(0));
    if (TMM_Parse(l_Syntax) == false)
    {
        TMM_DestroySyntax(l_Syntax);
        TMM_DestroyValue(l_StringValue);
        return NULL;
    }

    // Evaluate the parsed syntax.
    TMM_Value* l_Result = TMM_Evaluate(l_Syntax);
    if (l_Result == NULL)
    {
        TMM_DestroySyntax(l_Syntax);
        TMM_DestroyValue(l_StringValue);
        return NULL;
    }

    TMM_DestroySyntax(l_Syntax);
    TMM_DestroyValue(l_Result);
    TMM_DestroyValue(l_StringValue);
    return TMM_CreateVoidValue();
}

TMM_Value* TMM_EvaluateIncbinStatement (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the string expression.
    TMM_Value* l_StringValue = TMM_Evaluate(p_SyntaxNode->m_LeftExpr);
    if (l_StringValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a string.
    if (l_StringValue->m_Type != TMM_VT_STRING)
    {
        TM_error("Unexpected value type for string expression in 'incbin' statement.");
        TMM_DestroyValue(l_StringValue);
        return NULL;
    }

    // The syntax's right expression, if provided, will contain an offset value.
    size_t l_Offset = 0;
    if (p_SyntaxNode->m_RightExpr != NULL)
    {
        // Evaluate the offset expression.
        TMM_Value* l_OffsetValue = TMM_Evaluate(p_SyntaxNode->m_RightExpr);
        if (l_OffsetValue == NULL)
        {
            TMM_DestroyValue(l_StringValue);
            return NULL;
        }

        // Check the type of the value. It must be a number.
        if (l_OffsetValue->m_Type != TMM_VT_NUMBER)
        {
            TM_error("Unexpected value type for offset expression in 'incbin' statement.");
            TMM_DestroyValue(l_OffsetValue);
            TMM_DestroyValue(l_StringValue);
            return NULL;
        }

        l_Offset = (size_t) l_OffsetValue->m_IntegerPart;
        TMM_DestroyValue(l_OffsetValue);
    }

    // The syntax's count expression, if provided, will contain a length value.
    size_t l_Length = 0;
    if (p_SyntaxNode->m_CountExpr != NULL)
    {
        // Evaluate the length expression.
        TMM_Value* l_LengthValue = TMM_Evaluate(p_SyntaxNode->m_CountExpr);
        if (l_LengthValue == NULL)
        {
            TMM_DestroyValue(l_StringValue);
            return NULL;
        }

        // Check the type of the value. It must be a number.
        if (l_LengthValue->m_Type != TMM_VT_NUMBER)
        {
            TM_error("Unexpected value type for length expression in 'incbin' statement.");
            TMM_DestroyValue(l_LengthValue);
            TMM_DestroyValue(l_StringValue);
            return NULL;
        }

        l_Length = (size_t) l_LengthValue->m_IntegerPart;
        TMM_DestroyValue(l_LengthValue);
    }

    // Load the file.
    if (TMM_DefineBinaryFile(l_StringValue->m_String, l_Offset, l_Length) == false)
    {
        TMM_DestroyValue(l_StringValue);
        return NULL;
    }

    TMM_DestroyValue(l_StringValue);
    return TMM_CreateVoidValue();
}

TMM_Value* TMM_EvaluateAssert (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the condition expression.
    TMM_Value* l_ConditionValue = TMM_Evaluate(p_SyntaxNode->m_CondExpr);
    if (l_ConditionValue == NULL)
    {
        return NULL;
    }

    // Check the type of the value. It must be a number.
    if (l_ConditionValue->m_Type != TMM_VT_NUMBER)
    {
        TM_error("Unexpected value type for condition expression in 'assert' statement.");
        TMM_DestroyValue(l_ConditionValue);
        return NULL;
    }

    // Check if the condition is false.
    if (l_ConditionValue->m_Number == 0)
    {
        if (p_SyntaxNode->m_RightExpr != NULL)
        {
            // Evaluate the error message expression.
            TMM_Value* l_ErrorMessageValue = TMM_Evaluate(p_SyntaxNode->m_RightExpr);
            if (l_ErrorMessageValue == NULL)
            {
                TMM_DestroyValue(l_ConditionValue);
                return NULL;
            }

            // Check the type of the value. It must be a string.
            if (l_ErrorMessageValue->m_Type != TMM_VT_STRING)
            {
                TM_error("Unexpected value type for error message expression in 'assert' statement.");
                TMM_DestroyValue(l_ErrorMessageValue);
                TMM_DestroyValue(l_ConditionValue);
                return NULL;
            }

            // Print the error message.
            fprintf(stderr, "Assertion failed: %s\n", l_ErrorMessageValue->m_String);
            TMM_DestroyValue(l_ErrorMessageValue);
        }
        else
        {
            // Print a generic error message.
            fprintf(stderr, "Assertion failed.\n");
        }

        TMM_DestroyValue(l_ConditionValue);
        return NULL;
    }

    TMM_DestroyValue(l_ConditionValue);
    return TMM_CreateVoidValue();
}

TMM_Value* TMM_EvaluateBlock (const TMM_Syntax* p_SyntaxNode)
{
    // Create a new value to hold the result of the block.
    TMM_Value* l_Result = TMM_CreateVoidValue();

    // Evaluate each statement in the block.
    for (size_t i = 0; i < p_SyntaxNode->m_BodySize; ++i)
    {
        TMM_Value* l_Value = TMM_Evaluate(p_SyntaxNode->m_Body[i]);
        TMM_DestroyValue(l_Result);

        if (l_Value == NULL)
        {
            return NULL;
        }

        l_Result = l_Value;
    }

    return l_Result;
}

TMM_Value* TMM_Evaluate (const TMM_Syntax* p_SyntaxNode)
{
    TMM_Value* l_Result = NULL;
    switch (p_SyntaxNode->m_Type)
    {   
        case TMM_ST_STRING:
            l_Result = TMM_EvaluateString(p_SyntaxNode);
            break;

        case TMM_ST_NUMBER:
            l_Result = TMM_EvaluateNumber(p_SyntaxNode);
            break;

        case TMM_ST_BINARY_EXP:
            l_Result = TMM_EvaluateBinaryExpression(p_SyntaxNode);
            break;

        case TMM_ST_UNARY_EXP:
            l_Result = TMM_EvaluateUnaryExpression(p_SyntaxNode);
            break;

        case TMM_ST_IDENTIFIER:
            l_Result = TMM_EvaluateIdentifier(p_SyntaxNode);
            break;

        case TMM_ST_LABEL:
            l_Result = TMM_EvaluateLabel(p_SyntaxNode);
            break;

        case TMM_ST_DATA:
            l_Result = TMM_EvaluateData(p_SyntaxNode);
            break;

        case TMM_ST_DEF:
            l_Result = TMM_EvaluateDefine(p_SyntaxNode);
            break;

        case TMM_ST_MACRO:
            l_Result = TMM_EvaluateMacroDefinition(p_SyntaxNode);
            break;

        case TMM_ST_MACRO_CALL:
            l_Result = TMM_EvaluateMacroCall(p_SyntaxNode);
            break;

        case TMM_ST_NARG:
            l_Result = TMM_EvaluateNARGExpression(p_SyntaxNode);
            break;

        case TMM_ST_ARGUMENT:
            l_Result = TMM_EvaluateMacroArgument(p_SyntaxNode);
            break;

        case TMM_ST_SHIFT:
            l_Result = TMM_EvaluateShiftStatement(p_SyntaxNode);
            break;

        case TMM_ST_REPEAT:
            l_Result = TMM_EvaluateRepeatStatement(p_SyntaxNode);
            break;

        case TMM_ST_IF:
            l_Result = TMM_EvaluateIfStatement(p_SyntaxNode);
            break;

        case TMM_ST_BLOCK:
            l_Result = TMM_EvaluateBlock(p_SyntaxNode);
            break;

        case TMM_ST_INCLUDE:
            l_Result = TMM_EvaluateIncludeStatement(p_SyntaxNode);
            break;

        case TMM_ST_INCBIN:
            l_Result = TMM_EvaluateIncbinStatement(p_SyntaxNode);
            break;

        case TMM_ST_ASSERT:
            l_Result = TMM_EvaluateAssert(p_SyntaxNode);
            break;

        default:
            TM_error("Unexpected syntax node type: %d.", p_SyntaxNode->m_Type);
            break;
    }

    if (l_Result == NULL)
    {
        fprintf(stderr, " - In file '%s:%zu:%zu.\n",
            p_SyntaxNode->m_Token.m_SourceFile,
            p_SyntaxNode->m_Token.m_Line,
            p_SyntaxNode->m_Token.m_Column
        );
    }

    return l_Result;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void TMM_InitBuilder ()
{
    // Initialize labels.
    s_Builder.m_Labels = TM_malloc(TMM_BUILDER_INITIAL_CAPACITY, TMM_Label);
    TM_pexpect(s_Builder.m_Labels != NULL, "Failed to allocate memory for the builder's address labels array");
    s_Builder.m_LabelCapacity = TMM_BUILDER_INITIAL_CAPACITY;
    s_Builder.m_LabelCount = 0;

    // Initialize macros.
    s_Builder.m_Macros = TM_malloc(TMM_BUILDER_INITIAL_CAPACITY, TMM_Macro);
    TM_pexpect(s_Builder.m_Macros != NULL, "Failed to allocate memory for the builder's macros array");
    s_Builder.m_MacroCapacity = TMM_BUILDER_INITIAL_CAPACITY;
    s_Builder.m_MacroCount = 0;

    // Initialize defines.
    s_Builder.m_DefineValues = TM_malloc(TMM_BUILDER_INITIAL_CAPACITY, TMM_Value*);
    TM_pexpect(s_Builder.m_DefineValues != NULL, "Failed to allocate memory for the builder's define values array");
    s_Builder.m_DefineKeys = TM_malloc(TMM_BUILDER_INITIAL_CAPACITY, char*);
    TM_pexpect(s_Builder.m_DefineKeys != NULL, "Failed to allocate memory for the builder's define keys array");
    s_Builder.m_DefineCapacity = TMM_BUILDER_INITIAL_CAPACITY;
    s_Builder.m_DefineCount = 0;
}

void TMM_ShutdownBuilder ()
{
    // Free defines.
    for (size_t i = 0; i < s_Builder.m_DefineCount; ++i)
    {
        TMM_DestroyValue(s_Builder.m_DefineValues[i]);
        TM_free(s_Builder.m_DefineKeys[i]);
    }
    TM_free(s_Builder.m_DefineValues);
    TM_free(s_Builder.m_DefineKeys);

    // Free macro call stack.
    for (size_t i = 0; i < s_Builder.m_MacroCallStackIndex; ++i)
    {
        TMM_DestroyMacroCall(s_Builder.m_MacroCallStack[i]);
    }

    // Free macros.
    for (size_t i = 0; i < s_Builder.m_MacroCount; ++i)
    {
        TM_free(s_Builder.m_Macros[i].m_Name);
        TMM_DestroySyntax(s_Builder.m_Macros[i].m_Block);
    }
    TM_free(s_Builder.m_Macros);

    // Free labels.
    for (size_t i = 0; i < s_Builder.m_LabelCount; ++i)
    {
        TM_free(s_Builder.m_Labels[i].m_Name);
        TM_free(s_Builder.m_Labels[i].m_References);
    }
    TM_free(s_Builder.m_Labels);

    // Free the result value.
    TMM_DestroyValue(s_Builder.m_Result);
}

bool TMM_Build (const TMM_Syntax* p_SyntaxNode)
{
    // Evaluate the syntax node.
    s_Builder.m_Result = TMM_Evaluate(p_SyntaxNode);
    return s_Builder.m_Result != NULL;
}

bool TMM_SaveBinary (const char* p_OutputPath)
{
    TM_assert(p_OutputPath);

    // Ensure the path is not blank.
    if (p_OutputPath[0] == '\0')
    {
        TM_error("Output path is blank.");
        return false;
    }

    // Attempt to open the output file for writing.
    FILE* l_File = fopen(p_OutputPath, "wb");
    if (l_File == NULL)
    {
        TM_perror("Failed to open output file '%s' for writing", p_OutputPath);
        return false;
    }

    // Write the output buffer to the file.
    size_t l_BytesWritten = fwrite(s_Builder.m_Output, sizeof(uint8_t), s_Builder.m_OutputSize, l_File);
    if (l_BytesWritten != s_Builder.m_OutputSize || ferror(l_File))
    {
        TM_perror("Failed to write output to file '%s'", p_OutputPath);
        fclose(l_File);
        return false;
    }

    // Close the file.
    fclose(l_File);
    return true;
}
