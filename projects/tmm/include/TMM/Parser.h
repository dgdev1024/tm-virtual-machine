/**
 * @file  TMM/Parser.h
 * @brief Contains functions for parsing tokens to produce syntax nodes.
 */

#pragma once
#include <TMM/Syntax.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void TMM_InitParser ();
void TMM_ShutdownParser ();
bool TMM_Parse (TMM_Syntax* p_SyntaxBlock);
const TMM_Syntax* TMM_GetRootSyntax ();
