/**
 * @file     TMM/Syntax.h
 * @brief    Contains enumerations and structures for syntax nodes extracted from the lexer.
 */

#pragma once
#include <TMM/Token.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define TMM_SYNTAX_BODY_INITIAL_CAPACITY 8
#define TMM_STRING_CAPACITY 80

// Syntax Type Enumeration /////////////////////////////////////////////////////////////////////////

typedef enum TMM_SyntaxType
{
    TMM_ST_BLOCK,                   ///< @brief A block of syntax nodes, including the root node.

    // Statement Nodes
    TMM_ST_LABEL,                   ///< @brief Label Statement (eg. `label:`).
    TMM_ST_DATA,                    ///< @brief Data Statement (eg. `db 0x00`, `db "Hello, World!"`, `dw 0x0000`).
    TMM_ST_DEF,                     ///< @brief Define Statement (eg. `def x = 0x00`).
    TMM_ST_MACRO,                   ///< @brief Macro Statement (eg. `macro x ... endm`).
    TMM_ST_MACRO_CALL,              ///< @brief Macro Call Statement (eg. `x`, `place_byte $42`).
    TMM_ST_SHIFT,                   ///< @brief Shift Statement (eg. `shift 2`).
    TMM_ST_REPEAT,                  ///< @brief Repeat Statement (eg. `repeat 3`, `rept 5`).
    TMM_ST_IF,                      ///< @brief If Statement (eg. `if x == 0`).
    TMM_ST_INCLUDE,                 ///< @brief Include Statement (eg. `include "file.asm"`).
    TMM_ST_INCBIN,                  ///< @brief Include Binary Statement (eg. `incbin "file.bin"`).
    TMM_ST_ASSERT,                  ///< @brief Assert Statement (eg. `assert x == 0`).

    // Expression Nodes
    TMM_ST_BINARY_EXP,              ///< @brief Binary Expression (eg. `1 + 2`, `3 * 4`).
    TMM_ST_UNARY_EXP,               ///< @brief Unary Expression (eg. `-1`, `~2`).
    TMM_ST_NARG,                    ///< @brief Number of Arguments Expression (eg. `_narg`).
    TMM_ST_IDENTIFIER,              ///< @brief Identifier (eg. `x`, `y`).
    TMM_ST_NUMBER,                  ///< @brief Number (eg. `0`, `1`, `2`).
    TMM_ST_ARGUMENT,                ///< @brief Argument Placeholder (eg. `@0`, `@1`).
    TMM_ST_STRING,                  ///< @brief String (eg. `"Hello, World!"`).

} TMM_SyntaxType;

// Syntax Node Structure ///////////////////////////////////////////////////////////////////////////

typedef struct TMM_Syntax
{
    TMM_SyntaxType           m_Type;         ///< @brief Syntax Node Type
    TMM_Token                m_Token;        ///< @brief Token Associated with the Syntax Node

    // Syntax Node Specific Data ///////////////////////////////////////////////////////////////////

    // Some nodes keep a string of text.
    // - `TMM_ST_LABEL` nodes have a string of text to hold the label name.
    // - `TMM_ST_DEF` nodes have a string of text to hold the variable name.
    // - `TMM_ST_MACRO` nodes have a string of text to hold the macro name.
    // - `TMM_ST_MACRO_CALL` nodes have a string of text to hold the macro name.
    // - `TMM_ST_IDENTIFIER` nodes have a string of text to hold the symbol name.
    // - `TMM_ST_STRING` nodes have a string of text to hold the string.
    // - `TMM_ST_NUMBER` nodes have a string of text to hold the number in string form.
    char*                        m_String;       ///< @brief String of Text

    // Some nodes hold a number.
    // - `TMM_ST_MACRO_CALL` nodes have a number value to hold the argument count.
    // - `TMM_ST_ARGUMENT` nodes have a number value to hold the argument index.
    // - `TMM_ST_NARG` nodes have a number value to hold the number of arguments passed to a macro.
    // - `TMM_ST_NUMBER` nodes have a number value.
    double                       m_Number;       ///< @brief Number Value

    // Some nodes may need to keep track of the keyword type of its lead token.
    // - `TMM_ST_DATA` nodes have a keyword type to hold the data type.
    TMM_KeywordType              m_KeywordType;  ///< @brief Keyword Type

    // Some nodes have a body of child nodes, such as a block of statements, or a macro definition.
    // - `TMM_ST_BLOCK` nodes have a body of child nodes.
    // - `TMM_ST_DATA` nodes have a body of child nodes to hold parameters passed to the data statement.
    // - `TMM_ST_MACRO_CALL` nodes have a body of child nodes to hold arguments passed to the macro.
    struct TMM_Syntax**          m_Body;         ///< @brief Array of Child Syntax Nodes
    size_t                       m_BodySize;     ///< @brief Number of Child Syntax Nodes
    size_t                       m_BodyCapacity; ///< @brief Capacity of Child Syntax Nodes

    // Some nodes keep track of a count.
    // - `TMM_ST_DATA` nodes have a count of the number of parameters.
    // - `TMM_ST_SHIFT` nodes have a count of the number of arguments to shift.
    // - `TMM_ST_REPEAT` nodes have a count of the number of times to repeat the block.
    struct TMM_Syntax*       m_CountExpr;    ///< @brief Count Expression

    // Some nodes have a conditional expression.
    // - `TMM_ST_IF` nodes have a conditional expression.
    // - `TMM_ST_ASSERT` nodes have a conditional expression.
    struct TMM_Syntax*       m_CondExpr;     ///< @brief Conditional Expression

    // Some nodes are unary and binary expression nodes, with left and/or right child nodes 
    // and an operator token.
    // - `TMM_ST_MACRO` nodes contain their body in the left child node.
    // - `TMM_ST_REPEAT` nodes contain their body in the left child node.
    // - `TMM_ST_IF` nodes contain their true block in the left child node and their false block in the right child node.
    // - `TMM_ST_ASSERT` nodes contain their false block in the right child node.
    // - `TMM_ST_BINARY_EXP` nodes have a left and right child node.
    // - `TMM_ST_UNARY_EXP` nodes have a right child node.
    struct TMM_Syntax*       m_LeftExpr;     ///< @brief Left Expression
    struct TMM_Syntax*       m_RightExpr;    ///< @brief Right Expression
    TMM_TokenType            m_Operator;     ///< @brief Operator Token Type

} TMM_Syntax;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TMM_Syntax* TMM_CreateSyntax (TMM_SyntaxType p_Type, const TMM_Token* p_Token);
TMM_Syntax* TMM_CopySyntax (const TMM_Syntax* p_Syntax);
void TMM_DestroySyntax (TMM_Syntax* p_Syntax);
void TMM_PushToSyntaxBody (TMM_Syntax* p_Parent, TMM_Syntax* p_Child);
