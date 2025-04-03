/**
 * @file     TMM/Lexer.h
 * @brief    Contains functions for extracting tokens from a source file.
 */

#pragma once
#include <TMM/Token.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void TMM_InitLexer ();
void TMM_ShutdownLexer ();
bool TMM_LexFile (const char* p_FilePath);
bool TMM_HasMoreTokens ();
const TMM_Token* TMM_AdvanceToken ();
const TMM_Token* TMM_AdvanceTokenIfType (TMM_TokenType p_Type);
const TMM_Token* TMM_AdvanceTokenIfKeyword (TMM_KeywordType p_Type);
const TMM_Token* TMM_PeekToken (size_t p_Offset);
void TMM_PrintTokens ();
void TMM_ResetLexer ();
