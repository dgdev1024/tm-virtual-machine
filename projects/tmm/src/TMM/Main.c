/**
 * @file     TMM/Main.c
 */

#include <TMM/Arguments.h>
#include <TMM/Lexer.h>
#include <TMM/Parser.h>
#include <TMM/Builder.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

#define TMM_VERSION "0.1.0"

// Static Functions ////////////////////////////////////////////////////////////////////////////////

static void TMM_AtExit ()
{
    TMM_ShutdownBuilder();
    TMM_ShutdownParser();
    TMM_ShutdownLexer();
    TMM_ReleaseArguments();
}

static void TMM_PrintVersion ()
{
    printf("TMM - GABLE Asset BUILDer - Version %s\n", TMM_VERSION);
    printf("By: Dennis Griffin\n");
}

static void TMM_PrintHelp (FILE* p_Stream, const char* p_ProgramName)
{
    fprintf(p_Stream, "Usage: %s [options]\n", p_ProgramName);
    fprintf(p_Stream, "Options:\n");
    fprintf(p_Stream, "  -i, --input-file <file>    Input source file\n");
    fprintf(p_Stream, "  -o, --output-file <file>   Output binary file\n");
    fprintf(p_Stream, "  -l, --lex-only             Only perform lexical analysis\n");
    fprintf(p_Stream, "  -h, --help                 Print this help message\n");
    fprintf(p_Stream, "  -v, --version              Print version information\n");
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

int main (int argc, char** argv)
{
    atexit(TMM_AtExit);
    TMM_CaptureArguments(argc, argv);

    const char* l_InputFile     = TMM_GetArgumentValue("input-file", 'i');
    const char* l_OutputFile    = TMM_GetArgumentValue("output-file", 'o');
    bool        l_LexOnly       = TMM_HasArgument("lex-only", 'l');
    bool        l_Help          = TMM_HasArgument("help", 'h');
    bool        l_Version       = TMM_HasArgument("version", 'v');

    if (l_Help == true)
    {
        TMM_PrintVersion();
        TMM_PrintHelp(stdout, argv[0]);
        return 0;
    }

    if (l_Version == true)
    {
        TMM_PrintVersion();
        return 0;
    }

    if (l_InputFile == NULL)
    {
        fprintf(stderr, "Error: No input file specified\n\n");
        TMM_PrintHelp(stderr, argv[0]);
        return 1;
    }

    if (l_OutputFile == NULL && l_LexOnly == false)
    {
        fprintf(stderr, "Error: No output file specified\n\n");
        TMM_PrintHelp(stderr, argv[0]);
        return 1;
    }

    TMM_InitLexer();
    if (TMM_LexFile(l_InputFile) == false)
    {
        return 1;
    }

    if (l_LexOnly == true)
    {
        TMM_PrintTokens();
        return 0;
    }

    TMM_InitParser();
    if (TMM_Parse(NULL) == false)
    {
        return 1;
    }

    TMM_InitBuilder();
    if (TMM_Build(TMM_GetRootSyntax()) == false)
    {
        return 1;
    }

    if (TMM_SaveBinary(l_OutputFile) == false)
    {
        return 1;
    }

    return 0;
}
