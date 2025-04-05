/**
 * @file  Main.c
 */

#include <SDL2/SDL.h>
#include <TOMBOY/Tomboy.h>

// Static Members //////////////////////////////////////////////////////////////////////////////////

static TOMBOY_Program* s_Program = NULL;

// Private Functions - Start and Exit //////////////////////////////////////////////////////////////

static void TOMBOY_AtStart (const char* p_ProgramFilename)
{
    // Create and load the program.
    s_Program = TOMBOY_CreateProgram(p_ProgramFilename);
    if (s_Program == NULL)
    {
        fprintf(stderr, "Failed to create program from file '%s'.\n", p_ProgramFilename);
        exit(EXIT_FAILURE);
    }
}

static void TOMBOY_AtExit ()
{
    if (s_Program != NULL)                  { TOMBOY_DestroyProgram(s_Program); }
}

// Main Function ///////////////////////////////////////////////////////////////////////////////////

int main (int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <ROM file>\n", argv[0]);
        return 0;
    }

    // Set the exit function, then run the start function.
    atexit(TOMBOY_AtExit);
    TOMBOY_AtStart(argv[1]);

    return 0;
}