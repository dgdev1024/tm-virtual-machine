/**
 * @file  TMM/Builder.h
 * @brief Contains functions for building the output file from the syntax tree.
 */

#pragma once
#include <TMM/Syntax.h>
#include <TMM/Value.h>

// Public Functions ////////////////////////////////////////////////////////////////////////////////

void TMM_InitBuilder ();
void TMM_ShutdownBuilder ();
bool TMM_Build (const TMM_Syntax* p_SyntaxNode);
bool TMM_SaveBinary (const char* p_OutputPath);
