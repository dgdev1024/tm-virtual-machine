/**
 * @file     TMM/Arguments.h
 * @brief    Provides functions used for processing and parsing command line arguments.
 */

#pragma once

#include <TM/Common.h>

void TMM_CaptureArguments (int p_Argc, char** p_Argv);
void TMM_ReleaseArguments ();
bool TMM_HasArgument (const char* p_Longform, const char p_Shortform);
const char* TMM_GetArgumentValue (const char* p_Longform, const char p_Shortform);