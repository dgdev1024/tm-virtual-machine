/**
 * @file  TMM/Syntax.c
 */

#include <TMM/Syntax.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TMM_Syntax* TMM_CreateSyntax (TMM_SyntaxType p_Type, const TMM_Token* p_Token)
{
    TM_assert(p_Token != NULL)

    TMM_Syntax* l_Syntax = TM_calloc(1, TMM_Syntax);
    TM_pexpect(l_Syntax, "Could not allocate memory for syntax node");

    l_Syntax->m_Type = p_Type;
    l_Syntax->m_Token.m_Type = p_Token->m_Type;
    l_Syntax->m_Token.m_Line = p_Token->m_Line;
    l_Syntax->m_Token.m_Column = p_Token->m_Column;
    l_Syntax->m_Token.m_SourceFile = p_Token->m_SourceFile;
    l_Syntax->m_Token.m_Keyword = p_Token->m_Keyword;

    if (p_Token->m_Lexeme != NULL && p_Token->m_Lexeme[0] != '\0')
    {
        size_t l_LexemeStrlen = strlen(p_Token->m_Lexeme);
        l_Syntax->m_Token.m_Lexeme = TM_calloc(l_LexemeStrlen + 1, char);
        TM_pexpect(l_Syntax->m_Token.m_Lexeme, "Could not allocate memory for token lexeme");
        strncpy(l_Syntax->m_Token.m_Lexeme, p_Token->m_Lexeme, l_LexemeStrlen);
    }

    // If the syntax node calls for a string, allocate it.
    if (
        p_Type == TMM_ST_LABEL ||
        p_Type == TMM_ST_DEF ||
        p_Type == TMM_ST_MACRO ||
        p_Type == TMM_ST_MACRO_CALL ||
        p_Type == TMM_ST_INCLUDE ||
        p_Type == TMM_ST_IDENTIFIER ||
        p_Type == TMM_ST_STRING
    )
    {
        l_Syntax->m_String = TM_calloc(TMM_STRING_CAPACITY, char);
        TM_pexpect(l_Syntax->m_String, "Could not allocate memory for syntax node string");
    }

    // If the syntax node calls for a body of child nodes, allocate it.
    if (
        p_Type == TMM_ST_BLOCK ||
        p_Type == TMM_ST_DATA ||
        p_Type == TMM_ST_MACRO_CALL
    )
    {
        l_Syntax->m_Body = TM_calloc(TMM_SYNTAX_BODY_INITIAL_CAPACITY, TMM_Syntax*);
        TM_pexpect(l_Syntax->m_Body, "Could not allocate memory for syntax node body");
        l_Syntax->m_BodyCapacity = TMM_SYNTAX_BODY_INITIAL_CAPACITY;
    }

    return l_Syntax;
}

TMM_Syntax* TMM_CopySyntax (const TMM_Syntax* p_Syntax)
{
    if (p_Syntax == NULL)
    {
        return NULL;
    }

    TMM_Syntax* l_Copy = TMM_CreateSyntax(p_Syntax->m_Type, &p_Syntax->m_Token);
    
    // Copy the string, if it exists.
    if (p_Syntax->m_String != NULL)
    {
        strncpy(l_Copy->m_String, p_Syntax->m_String, TMM_STRING_CAPACITY);
    }

    // Copy the body of child nodes, if it exists.
    if (p_Syntax->m_Body != NULL)
    {
        for (size_t i = 0; i < p_Syntax->m_BodySize; ++i)
        {
            TMM_PushToSyntaxBody(l_Copy, TMM_CopySyntax(p_Syntax->m_Body[i]));
        }
    }

    // Copy the other child syntax nodes, if they exist.
    l_Copy->m_CountExpr = TMM_CopySyntax(p_Syntax->m_CountExpr);
    l_Copy->m_CondExpr = TMM_CopySyntax(p_Syntax->m_CondExpr);
    l_Copy->m_LeftExpr = TMM_CopySyntax(p_Syntax->m_LeftExpr);
    l_Copy->m_RightExpr = TMM_CopySyntax(p_Syntax->m_RightExpr);
    l_Copy->m_Operator = p_Syntax->m_Operator;
    l_Copy->m_Number = p_Syntax->m_Number;
    l_Copy->m_KeywordType = p_Syntax->m_KeywordType;

    return l_Copy;
}

void TMM_DestroySyntax (TMM_Syntax* p_Syntax)
{
    if (p_Syntax != NULL)
    {
        // Destroy the syntax lead token's lexeme, if it exists.
        if (p_Syntax->m_Token.m_Lexeme != NULL)
        {
            TM_free(p_Syntax->m_Token.m_Lexeme);
        }

        // Destroy the string, if it exists.
        if (p_Syntax->m_String != NULL)
        {
            TM_free(p_Syntax->m_String);
        }

        // Destroy the body of child nodes, if it exists.
        if (p_Syntax->m_Body != NULL)
        {
            for (size_t i = 0; i < p_Syntax->m_BodySize; ++i)
            {
                TMM_DestroySyntax(p_Syntax->m_Body[i]);
            }

            TM_free(p_Syntax->m_Body);
        }

        // Destroy the other child syntax nodes, if they exist.
        TMM_DestroySyntax(p_Syntax->m_CountExpr);
        TMM_DestroySyntax(p_Syntax->m_CondExpr);
        TMM_DestroySyntax(p_Syntax->m_LeftExpr);
        TMM_DestroySyntax(p_Syntax->m_RightExpr);

        TM_free(p_Syntax);
    }
}

void TMM_PushToSyntaxBody (TMM_Syntax* p_Parent, TMM_Syntax* p_Child)
{
    TM_assert(p_Parent != NULL)
    TM_assert(p_Child != NULL)

    // Do nothing if the syntax body is not allocated.
    if (p_Parent->m_Body == NULL)
    {
        return;
    }

    // Resize the body array if necessary.
    if (p_Parent->m_BodySize + 1 >= p_Parent->m_BodyCapacity)
    {
        size_t l_NewCapacity = p_Parent->m_BodyCapacity * 2;
        TMM_Syntax** l_NewBody = TM_realloc(p_Parent->m_Body, l_NewCapacity, TMM_Syntax*);
        TM_pexpect(l_NewBody, "Could not reallocate memory for syntax body");

        p_Parent->m_Body = l_NewBody;
        p_Parent->m_BodyCapacity = l_NewCapacity;
    }

    // Add the child to the body.
    p_Parent->m_Body[p_Parent->m_BodySize++] = p_Child;
}