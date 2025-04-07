/**
 * @file  TOMBOY/Program.c
 */

#include <TOMBOY/Program.h>

// Program Header Structure ////////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_ProgramHeader
{
    uint8_t         m_Identifier[4];            ///< @brief `$0000` - Program identifier, must be "TMBY" (0x54, 0x4D, 0x42, 0x59).
    uint8_t         m_Version[4];               ///< @brief `$0004` - Program version, laid out as `0xMMNNPPPP` where `MM` is the major version, `NN` is the minor version, and `PPPP` is the patch version.
    uint8_t         m_RequestedWRAM[4];         ///< @brief `$0008` - The four bytes of the requested WRAM size, laid out in little-endian order.
    uint8_t         m_RequestedSRAM[4];         ///< @brief `$000C` - The four bytes of the requested SRAM size, laid out in little-endian order.
    uint8_t         m_RequestedXRAM[4];         ///< @brief `$0010` - The four bytes of the requested XRAM size, laid out in little-endian order.
    uint8_t         m_Padding0[12];             ///< @brief `$0014` - Padding bytes.
    uint8_t         m_ProgramName[32];          ///< @brief `$0020` - The program name, null-terminated.
    uint8_t         m_ProgramAuthor[32];        ///< @brief `$0040` - The program author, null-terminated.
    uint8_t         m_ProgramDescription[256];  ///< @brief `$0060` - The program description, null-terminated.
} TOMBOY_ProgramHeader;

// Program Structure ///////////////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_Program
{
    uint8_t*                        m_Data;             ///< @brief Pointer to the program data in memory.
    uint32_t                        m_Size;             ///< @brief Size of the program data in bytes.
    uint32_t                        m_RequestedWRAM;    ///< @brief The size of the working RAM requested by the program, in bytes.
    uint32_t                        m_RequestedSRAM;    ///< @brief The size of the static RAM requested by the program, in bytes.
    uint32_t                        m_RequestedXRAM;    ///< @brief The size of the executable RAM requested by the program, in bytes.
    const TOMBOY_ProgramHeader*     m_Header;           ///< @brief Pointer to the program header at the start of the program data.
} TOMBOY_Program;

// Private Function Prototypes /////////////////////////////////////////////////////////////////////

static bool TOMBOY_ValidateProgram (TOMBOY_Program* p_Program);

// Private Functions ///////////////////////////////////////////////////////////////////////////////

bool TOMBOY_ValidateProgram (TOMBOY_Program* p_Program)
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

    // Retrieve the program's requested WRAM and SRAM sizes.
    p_Program->m_RequestedWRAM = (uint32_t) l_Header->m_RequestedWRAM[0] |
                                 ((uint32_t) l_Header->m_RequestedWRAM[1] << 8) |
                                 ((uint32_t) l_Header->m_RequestedWRAM[2] << 16) |
                                 ((uint32_t) l_Header->m_RequestedWRAM[3] << 24);
    p_Program->m_RequestedSRAM = (uint32_t) l_Header->m_RequestedSRAM[0] |
                                 ((uint32_t) l_Header->m_RequestedSRAM[1] << 8) |
                                 ((uint32_t) l_Header->m_RequestedSRAM[2] << 16) |
                                 ((uint32_t) l_Header->m_RequestedSRAM[3] << 24);
    p_Program->m_RequestedXRAM = (uint32_t) l_Header->m_RequestedXRAM[0] |
                                 ((uint32_t) l_Header->m_RequestedXRAM[1] << 8) |
                                 ((uint32_t) l_Header->m_RequestedXRAM[2] << 16) |
                                 ((uint32_t) l_Header->m_RequestedXRAM[3] << 24);

    // Validate the requested WRAM size.
    if (p_Program->m_RequestedWRAM > TOMBOY_WRAM_SIZE)
    {
        TM_error("Requested WRAM size 0x%08X exceeds maximum of 0x%08X.", p_Program->m_RequestedWRAM, TOMBOY_WRAM_SIZE);
        return false;
    }

    // Validate the requested SRAM size.
    if (p_Program->m_RequestedSRAM > TOMBOY_SRAM_SIZE)
    {
        TM_error("Requested SRAM size 0x%08X exceeds maximum of 0x%08X.", p_Program->m_RequestedSRAM, TOMBOY_SRAM_SIZE);
        return false;
    }

    // Validate the requested XRAM size.
    if (p_Program->m_RequestedXRAM > TOMBOY_XRAM_SIZE)
    {
        TM_error("Requested XRAM size 0x%08X exceeds maximum of 0x%08X.", p_Program->m_RequestedXRAM, TOMBOY_XRAM_SIZE);
        return false;
    }

    // Ensure the program name, author, and description are null-terminated.
    if (l_Header->m_ProgramName[31] != '\0')
    {
        TM_error("Program name is not null-terminated.");
        return false;
    }

    if (l_Header->m_ProgramAuthor[31] != '\0')
    {
        TM_error("Program author is not null-terminated.");
        return false;
    }

    if (l_Header->m_ProgramDescription[255] != '\0')
    {
        TM_error("Program description is not null-terminated.");
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
    else if (l_FileSize <= 0x3002)
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

    return p_Program->m_RequestedSRAM;
}

uint32_t TOMBOY_GetRequestedWRAMSize (const TOMBOY_Program* p_Program)
{
    if (p_Program == NULL)
    {
        TM_error("TOMBOY program is NULL.");
        return 0;
    }

    return p_Program->m_RequestedWRAM;
}

uint32_t TOMBOY_GetRequestedXRAMSize (const TOMBOY_Program* p_Program)
{
    if (p_Program == NULL)
    {
        TM_error("TOMBOY program is NULL.");
        return 0;
    }

    return p_Program->m_RequestedXRAM;
}

const char* TOMBOY_GetProgramTitle (const TOMBOY_Program* p_Program)
{
    if (p_Program == NULL)
    {
        TM_error("TOMBOY program is NULL.");
        return NULL;
    }

    return (const char*) p_Program->m_Header->m_ProgramName;
}