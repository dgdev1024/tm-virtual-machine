/**
 * @file  TOMBOY/Program.c
 */

#include <TOMBOY/Program.h>

// Program Header Structure ////////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_ProgramHeader
{
    char        m_Identifier[4];            ///< @brief Identifier for the program header, should be "TMBY".
    uint8_t     m_MajorVersion;             ///< @brief Major version of the program.
    uint8_t     m_MinorVersion;             ///< @brief Minor version of the program.
    uint16_t    m_PatchVersion;             ///< @brief Patch version of the program.
    uint32_t    m_ProgramSize;              ///< @brief Size of the program in bytes.
    uint32_t    m_ProgramWRAM;              ///< @brief Requested size of the working RAM (WRAM) in bytes.
    uint32_t    m_ProgramSRAM;              ///< @brief Requested size of the static RAM (SRAM) in bytes.
    char        m_ProgramName[32];          ///< @brief Name of the program.
    char        m_ProgramAuthor[32];        ///< @brief Author of the program.
    char        m_ProgramDescription[256];  ///< @brief Description of the program.
} TOMBOY_ProgramHeader;

// Program Structure ///////////////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_Program
{
    uint8_t*                        m_Data;             ///< @brief Pointer to the program data in memory.
    uint32_t                        m_Size;             ///< @brief Size of the program data in bytes.
    const TOMBOY_ProgramHeader*     m_Header;           ///< @brief Pointer to the program header at the start of the program data.
} TOMBOY_Program;

// Private Function Prototypes /////////////////////////////////////////////////////////////////////

static bool TOMBOY_ValidateProgram (const TOMBOY_Program* p_Program);

// Private Functions ///////////////////////////////////////////////////////////////////////////////

bool TOMBOY_ValidateProgram (const TOMBOY_Program* p_Program)
{
    // Point to the program header.
    const TOMBOY_ProgramHeader* l_Header = p_Program->m_Header;
    if (l_Header == NULL)
    {
        TM_error("Program header is NULL.");
        return false;
    }

    // Validate the program identifier.
    if (strncmp(l_Header->m_Identifier, "TMBY", 4) != 0)
    {
        TM_error("Invalid program identifier: '%s'. Expected 'TMBY'.", l_Header->m_Identifier);
        return false;
    }

    // Validate the program size.
    if (l_Header->m_ProgramSize != p_Program->m_Size)
    {
        TM_error("Program size mismatch: expected %u bytes, got %u bytes.", l_Header->m_ProgramSize, p_Program->m_Size);
        return false;
    }

    if (l_Header->m_ProgramWRAM > TOMBOY_WRAM_SIZE)
    {
        TM_error("Requested WRAM size %u exceeds maximum of %u bytes.", l_Header->m_ProgramWRAM, TOMBOY_WRAM_SIZE);
        return false;
    }

    if (l_Header->m_ProgramSRAM > TOMBOY_SRAM_SIZE)
    {
        TM_error("Requested SRAM size %u exceeds maximum of %u bytes.", l_Header->m_ProgramSRAM, TOMBOY_SRAM_SIZE);
        return false;
    }

    return true;
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_Program* TOMBOY_CreateProgram (const char* p_Filename)
{
    // Ensure the filename is not NULL or blank.
    if (p_Filename == NULL || p_Filename[0] == '\0')
    {
        TM_error("Filename is NULL or blank.");
        return NULL;
    }

    // Open the file for reading.
    FILE* l_File = fopen(p_Filename, "rb");
    if (l_File == NULL)
    {
        TM_perror("Failed to open file '%s' for reading", p_Filename);
        return NULL;
    }

    // Get and validate the file size.
    fseek(l_File, 0, SEEK_END);
    int64_t l_FileSize = ftell(l_File);
    if (l_FileSize < 0)
    {
        TM_perror("Failed to get file size for '%s'", p_Filename);
        fclose(l_File);
        return NULL;
    }
    else if (l_FileSize < 0x4000)
    {
        TM_error("File '%s' is too small to be a valid TOMBOY program.", p_Filename);
        fclose(l_File);
        return NULL;
    }
    else if (l_FileSize > TM_ROM_END)
    {
        TM_error("File '%s' is too large to be a valid TOMBOY program.", p_Filename);
        fclose(l_File);
        return NULL;
    }
    rewind(l_File);

    // Create the program instance.
    TOMBOY_Program* l_Program = TM_calloc(1, TOMBOY_Program);
    TM_pexpect(l_Program != NULL, "Failed to allocate memory for TOMBOY program");

    // Allocate memory for the program data.
    l_Program->m_Data = TM_calloc(l_FileSize, uint8_t);
    TM_pexpect(l_Program->m_Data != NULL, "Failed to allocate memory for TOMBOY program data");
    l_Program->m_Size = (uint32_t) l_FileSize;

    // Read the program data from the file.
    size_t l_ReadBytes = fread(l_Program->m_Data, 1, l_FileSize, l_File);
    if (l_ReadBytes != l_FileSize || (ferror(l_File) && !feof(l_File)))
    {
        TM_perror("Failed to read program data from '%s'", p_Filename);
        fclose(l_File);
        TOMBOY_DestroyProgram(l_Program);
        return NULL;
    }
    
    // Close the file.
    fclose(l_File);
    l_Program->m_Header = (const TOMBOY_ProgramHeader*) l_Program->m_Data;

    // Validate the program.
    if (TOMBOY_ValidateProgram(l_Program) == false)
    {
        TOMBOY_DestroyProgram(l_Program);
        return NULL;
    }

    return l_Program;
}

void TOMBOY_DestroyProgram (TOMBOY_Program* p_Program)
{
    if (p_Program != NULL)
    {
        TM_free(p_Program->m_Data);
        TM_free(p_Program);
    }
}

uint8_t TOMBOY_ReadProgramByte (const TOMBOY_Program* p_Program, uint32_t p_Address)
{
    if (p_Program == NULL)
    {
        TM_error("TOMBOY program is NULL.");
        return 0xFF;
    }

    if (p_Address >= p_Program->m_Size)
    {
        TM_error("Program relative read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    return p_Program->m_Data[p_Address];
}

uint32_t TOMBOY_GetRequestedSRAMSize (const TOMBOY_Program* p_Program)
{
    if (p_Program == NULL)
    {
        TM_error("TOMBOY program is NULL.");
        return 0;
    }

    return p_Program->m_Header->m_ProgramSRAM;
}

uint32_t TOMBOY_GetRequestedWRAMSize (const TOMBOY_Program* p_Program)
{
    if (p_Program == NULL)
    {
        TM_error("TOMBOY program is NULL.");
        return 0;
    }

    return p_Program->m_Header->m_ProgramWRAM;
}
