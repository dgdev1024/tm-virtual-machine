/**
 * @file      TMM/Token.c
 */

#include <TMM/Token.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

const char* TMM_StringifyTokenType (TMM_TokenType p_Type)
{
    switch (p_Type)
    {
        // Keywords and Identifiers
        case TMM_TOKEN_KEYWORD: return "Keyword";
        case TMM_TOKEN_IDENTIFIER: return "Identifier";

        // String, Number, and Character Literals
        case TMM_TOKEN_STRING: return "String";
        case TMM_TOKEN_NUMBER: return "Number";
        case TMM_TOKEN_BINARY: return "Binary";
        case TMM_TOKEN_OCTAL: return "Octal";
        case TMM_TOKEN_HEXADECIMAL: return "Hexadecimal";
        case TMM_TOKEN_CHARACTER: return "Character";
        case TMM_TOKEN_ARGUMENT: return "Argument";
        case TMM_TOKEN_GRAPHICS: return "Graphics";

        // Arithmetic Operators
        case TMM_TOKEN_PLUS: return "Plus";
        case TMM_TOKEN_MINUS: return "Minus";
        case TMM_TOKEN_MULTIPLY: return "Multiply";
        case TMM_TOKEN_EXPONENT: return "Exponent";
        case TMM_TOKEN_DIVIDE: return "Divide";
        case TMM_TOKEN_MODULO: return "Modulo";
        case TMM_TOKEN_INCREMENT: return "Increment";
        case TMM_TOKEN_DECREMENT: return "Decrement";

        // Bitwise Operators
        case TMM_TOKEN_BITWISE_AND: return "Bitwise And";
        case TMM_TOKEN_BITWISE_OR: return "Bitwise Or";
        case TMM_TOKEN_BITWISE_XOR: return "Bitwise Xor";
        case TMM_TOKEN_BITWISE_NOT: return "Bitwise Not";
        case TMM_TOKEN_BITWISE_SHIFT_LEFT: return "Bitwise Shift Left";
        case TMM_TOKEN_BITWISE_SHIFT_RIGHT: return "Bitwise Shift Right";

        // Comparison Operators
        case TMM_TOKEN_COMPARE_EQUAL: return "Compare Equal";
        case TMM_TOKEN_COMPARE_NOT_EQUAL: return "Compare Not Equal";
        case TMM_TOKEN_COMPARE_LESS: return "Compare Less";
        case TMM_TOKEN_COMPARE_LESS_EQUAL: return "Compare Less Equal";
        case TMM_TOKEN_COMPARE_GREATER: return "Compare Greater";
        case TMM_TOKEN_COMPARE_GREATER_EQUAL: return "Compare Greater Equal";

        // Logical Operators
        case TMM_TOKEN_LOGICAL_AND: return "Logical And";
        case TMM_TOKEN_LOGICAL_OR: return "Logical Or";
        case TMM_TOKEN_LOGICAL_NOT: return "Logical Not";

        // Assignment Operators
        case TMM_TOKEN_ASSIGN_EQUAL: return "Assign Equal";
        case TMM_TOKEN_ASSIGN_PLUS: return "Assign Plus";
        case TMM_TOKEN_ASSIGN_MINUS: return "Assign Minus";
        case TMM_TOKEN_ASSIGN_MULTIPLY: return "Assign Multiply";
        case TMM_TOKEN_ASSIGN_EXPONENT: return "Assign Exponent";
        case TMM_TOKEN_ASSIGN_DIVIDE: return "Assign Divide";
        case TMM_TOKEN_ASSIGN_MODULO: return "Assign Modulo";
        case TMM_TOKEN_ASSIGN_BITWISE_AND: return "Assign Bitwise And";
        case TMM_TOKEN_ASSIGN_BITWISE_OR: return "Assign Bitwise Or";
        case TMM_TOKEN_ASSIGN_BITWISE_XOR: return "Assign Bitwise Xor";
        case TMM_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT: return "Assign Bitwise Shift Left";
        case TMM_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT: return "Assign Bitwise Shift Right";

        // Grouping Operators
        case TMM_TOKEN_PARENTHESIS_OPEN: return "Parenthesis Open";
        case TMM_TOKEN_PARENTHESIS_CLOSE: return "Parenthesis Close";
        case TMM_TOKEN_BRACKET_OPEN: return "Bracket Open";
        case TMM_TOKEN_BRACKET_CLOSE: return "Bracket Close";
        case TMM_TOKEN_BRACE_OPEN: return "Brace Open";
        case TMM_TOKEN_BRACE_CLOSE: return "Brace Close";

        // Punctuation
        case TMM_TOKEN_COMMA: return "Comma";
        case TMM_TOKEN_COLON: return "Colon";
        case TMM_TOKEN_PERIOD: return "Period";
        case TMM_TOKEN_QUESTION: return "Question Mark";
        case TMM_TOKEN_POUND: return "Pound";

        case TMM_TOKEN_NEWLINE: return "Newline";
        case TMM_TOKEN_EOF: return "End of File";

        default: return "Unknown";
    }
}

const char* TMM_StringifyToken (const TMM_Token* p_Token)
{
    return (p_Token != NULL) ? TMM_StringifyTokenType(p_Token->m_Type) : "Null";
}

void TMM_PrintToken (const TMM_Token* p_Token)
{
    printf("  Token '%s'", TMM_StringifyToken(p_Token));
    if (p_Token->m_Lexeme != NULL && p_Token->m_Lexeme[0] != '\0')
    {
        printf(" = '%s'", p_Token->m_Lexeme);
    }
    printf("\n");
}

bool TMM_IsUnaryOperator (TMM_TokenType p_Type)
{
    switch (p_Type)
    {
        case TMM_TOKEN_PLUS:
        case TMM_TOKEN_MINUS:
        case TMM_TOKEN_LOGICAL_NOT:
        case TMM_TOKEN_BITWISE_NOT:
            return true;

        default:
            return false;
    }
}

bool TMM_IsMultiplicativeOperator (TMM_TokenType p_Type)
{
    switch (p_Type)
    {
        case TMM_TOKEN_MULTIPLY:
        case TMM_TOKEN_DIVIDE:
        case TMM_TOKEN_MODULO:
            return true;

        default:
            return false;
    }
}

bool TMM_IsAdditiveOperator (TMM_TokenType p_Type)
{
    switch (p_Type)
    {
        case TMM_TOKEN_PLUS:
        case TMM_TOKEN_MINUS:
            return true;

        default:
            return false;
    }
}

bool TMM_IsShiftOperator (TMM_TokenType p_Type)
{
    switch (p_Type)
    {
        case TMM_TOKEN_BITWISE_SHIFT_LEFT:
        case TMM_TOKEN_BITWISE_SHIFT_RIGHT:
            return true;

        default:
            return false;
    }
}

bool TMM_IsComparisonOperator (TMM_TokenType p_Type)
{
    switch (p_Type)
    {
        case TMM_TOKEN_COMPARE_EQUAL:
        case TMM_TOKEN_COMPARE_NOT_EQUAL:
        case TMM_TOKEN_COMPARE_LESS:
        case TMM_TOKEN_COMPARE_LESS_EQUAL:
        case TMM_TOKEN_COMPARE_GREATER:
        case TMM_TOKEN_COMPARE_GREATER_EQUAL:
            return true;

        default:
            return false;
    }
}

bool TMM_IsAssignmentOperator (TMM_TokenType p_Type)
{
    switch (p_Type)
    {
        case TMM_TOKEN_ASSIGN_EQUAL:
        case TMM_TOKEN_ASSIGN_PLUS:
        case TMM_TOKEN_ASSIGN_MINUS:
        case TMM_TOKEN_ASSIGN_MULTIPLY:
        case TMM_TOKEN_ASSIGN_EXPONENT:
        case TMM_TOKEN_ASSIGN_DIVIDE:
        case TMM_TOKEN_ASSIGN_MODULO:
        case TMM_TOKEN_ASSIGN_BITWISE_AND:
        case TMM_TOKEN_ASSIGN_BITWISE_OR:
        case TMM_TOKEN_ASSIGN_BITWISE_XOR:
        case TMM_TOKEN_ASSIGN_BITWISE_SHIFT_LEFT:
        case TMM_TOKEN_ASSIGN_BITWISE_SHIFT_RIGHT:
            return true;

        default:
            return false;
    }
}
