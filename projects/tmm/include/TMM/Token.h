/**
 * @file     TMM/Token.h
 * @brief    Contains the token structure and the token type enumeration.
 */

#pragma once
#include <TMM/Keyword.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define TMM_TOKEN_MAX_LENGTH 80

// Token Type Enumeration //////////////////////////////////////////////////////////////////////////

typedef enum TMM_TokenType
{
    TMM_TOKEN_UNKNOWN = 0,

    // Keywords and Identifiers
    TMM_TOKEN_KEYWORD,
    TMM_TOKEN_IDENTIFIER,

    // String, Number, and Character Literals
    TMM_TOKEN_STRING,
    TMM_TOKEN_NUMBER,
    TMM_TOKEN_BINARY,
    TMM_TOKEN_OCTAL,
    TMM_TOKEN_HEXADECIMAL,
    TMM_TOKEN_CHARACTER,
    TMM_TOKEN_ARGUMENT,
    TMM_TOKEN_GRAPHICS,

    // Arithmetic Operators
    TMM_TOKEN_PLUS,
    TMM_TOKEN_MINUS,
    TMM_TOKEN_MULTIPLY,
    TMM_TOKEN_EXPONENT,
    TMM_TOKEN_DIVIDE,
    TMM_TOKEN_MODULO,
    TMM_TOKEN_INCREMENT,
    TMM_TOKEN_DECREMENT,

    // Bitwise Operators
    TMM_TOKEN_BITWISE_AND,
    TMM_TOKEN_BITWISE_OR,
    TMM_TOKEN_BITWISE_XOR,
    TMM_TOKEN_BITWISE_NOT,
    TMM_TOKEN_BITWISE_SHIFT_LEFT,
    TMM_TOKEN_BITWISE_SHIFT_RIGHT,

    // Comparison Operators
    TMM_TOKEN_COMPARE_EQUAL,
    TMM_TOKEN_COMPARE_NOT_EQUAL,
    TMM_TOKEN_COMPARE_LESS,
    TMM_TOKEN_COMPARE_LESS_EQUAL,
    TMM_TOKEN_COMPARE_GREATER,
    TMM_TOKEN_COMPARE_GREATER_EQUAL,

    // Logical Operators
    TMM_TOKEN_LOGICAL_AND,
    TMM_TOKEN_LOGICAL_OR,
    TMM_TOKEN_LOGICAL_NOT,

    // Assignment Operators
    TMM_TOKEN_ASSIGN_EQUAL,
    TMM_TOKEN_ASSIGN_PLUS,
    TMM_TOKEN_ASSIGN_MINUS,
    TMM_TOKEN_ASSIGN_MULTIPLY,
    TMM_TOKEN_ASSIGN_EXPONENT,
    TMM_TOKEN_ASSIGN_DIVIDE,
    TMM_TOKEN_ASSIGN_MODULO,
    TMM_TOKEN_ASSIGN_BITWISE_AND,
    TMM_TOKEN_ASSIGN_BITWISE_OR,
    TMM_TOKEN_ASSIGN_BITWISE_XOR,
    TMM_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT,
    TMM_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT,

    // Grouping Operators
    TMM_TOKEN_PARENTHESIS_OPEN,
    TMM_TOKEN_PARENTHESIS_CLOSE,
    TMM_TOKEN_BRACKET_OPEN,
    TMM_TOKEN_BRACKET_CLOSE,
    TMM_TOKEN_BRACE_OPEN,
    TMM_TOKEN_BRACE_CLOSE,

    // Punctuation
    TMM_TOKEN_COMMA,
    TMM_TOKEN_COLON,
    TMM_TOKEN_PERIOD,
    TMM_TOKEN_QUESTION,
    TMM_TOKEN_POUND,

    TMM_TOKEN_NEWLINE,
    TMM_TOKEN_EOF
} TMM_TokenType;

// Token Structure /////////////////////////////////////////////////////////////////////////////////

typedef struct TMM_Token
{
    char*                       m_Lexeme;
    TMM_TokenType               m_Type;
    const TMM_Keyword*          m_Keyword;
    const char*                 m_SourceFile;
    size_t                      m_Line;
    size_t                      m_Column;
} TMM_Token;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const char* TMM_StringifyTokenType (TMM_TokenType p_Type);
const char* TMM_StringifyToken (const TMM_Token* p_Token);
void TMM_PrintToken (const TMM_Token* p_Token);
bool TMM_IsUnaryOperator (TMM_TokenType p_Type);
bool TMM_IsMultiplicativeOperator (TMM_TokenType p_Type);
bool TMM_IsAdditiveOperator (TMM_TokenType p_Type);
bool TMM_IsShiftOperator (TMM_TokenType p_Type);
bool TMM_IsComparisonOperator (TMM_TokenType p_Type);
bool TMM_IsAssignmentOperator (TMM_TokenType p_Type);
