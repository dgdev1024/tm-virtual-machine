/**
 * @file  TOMBOY/RAM.c
 */

#include <TOMBOY/RAM.h>

// RAM Context Structure ///////////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_RAM
{
    uint8_t* m_WRAM;         ///< @brief Pointer to the working RAM (WRAM) buffer.
    uint8_t* m_SRAM;         ///< @brief Pointer to the static RAM (SRAM) buffer.
    uint8_t* m_QRAM;         ///< @brief Pointer to the quick RAM (QRAM) buffer.
    uint32_t m_WRAMSize;    ///< @brief Size of the working RAM (WRAM) in bytes.
    uint32_t m_SRAMSize;    ///< @brief Size of the static RAM (SRAM) in bytes.
} TOMBOY_RAM;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_RAM* TOMBOY_CreateRAM (uint32_t p_WRAMSize, uint32_t p_SRAMSize)
{
    // Allocate memory for the RAM context.
    TOMBOY_RAM* l_RAM = TM_calloc(1, TOMBOY_RAM);
    TM_pexpect(l_RAM != NULL, "Failed to allocate memory for TOMBOY RAM context");

    // Allocate memory for the working RAM (WRAM).
    if (p_WRAMSize > 0)
    {
        l_RAM->m_WRAM = TM_calloc(p_WRAMSize, uint8_t);
        TM_pexpect(l_RAM->m_WRAM != NULL, "Failed to allocate memory for TOMBOY WRAM");
        l_RAM->m_WRAMSize = p_WRAMSize;
    }

    // Allocate memory for the static RAM (SRAM).
    if (p_SRAMSize > 0)
    {
        l_RAM->m_SRAM = TM_calloc(p_SRAMSize, uint8_t);
        TM_pexpect(l_RAM->m_SRAM != NULL, "Failed to allocate memory for TOMBOY SRAM");
        l_RAM->m_SRAMSize = p_SRAMSize;
    }

    // Allocate memory for the quick RAM (QRAM).
    l_RAM->m_QRAM = TM_calloc(0x10000, uint8_t);
    TM_pexpect(l_RAM->m_QRAM != NULL, "Failed to allocate memory for TOMBOY QRAM");

    return l_RAM;
}

void TOMBOY_DestroyRAM (TOMBOY_RAM* p_RAM)
{
    if (p_RAM != NULL)
    {
        TM_free(p_RAM->m_WRAM);
        TM_free(p_RAM->m_SRAM);
        TM_free(p_RAM->m_QRAM);
        TM_free(p_RAM);
    }
}

void TOMBOY_ResetRAM (TOMBOY_RAM* p_RAM)
{
    if (p_RAM != NULL)
    {
        if (p_RAM->m_WRAM != NULL && p_RAM->m_WRAMSize > 0)
        {
            memset(p_RAM->m_WRAM, 0x00, p_RAM->m_WRAMSize);
        }

        if (p_RAM->m_SRAM != NULL && p_RAM->m_SRAMSize > 0)
        {
            memset(p_RAM->m_SRAM, 0x00, p_RAM->m_SRAMSize);
        }
        
        memset(p_RAM->m_QRAM, 0x00, 0x10000);
    }
}

bool TOMBOY_LoadSRAM (TOMBOY_RAM* p_RAM, const char* p_Filename)
{
    // Ensure the RAM context is not NULL.
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return false;
    }

    // Ensure the filename is not NULL or blank.
    if (p_Filename == NULL || p_Filename[0] == '\0')
    {
        TM_error("Filename is NULL or blank.");
        return false;
    }

    // Ensure the SRAM size is valid.
    if (p_RAM->m_SRAMSize == 0)
    {
        TM_error("Cannot load SRAM file '%s': Reserved SRAM size is zero.", p_Filename);
        return false;
    }

    // Open the file for reading.
    FILE* l_File = fopen(p_Filename, "rb");
    if (l_File == NULL)
    {
        TM_perror("Failed to open file '%s' for reading", p_Filename);
        return false;
    }

    // Get and validate the file size.
    fseek(l_File, 0, SEEK_END);
    int64_t l_FileSize = ftell(l_File);
    if (l_FileSize < 0)
    {
        TM_perror("Failed to get file size for '%s'", p_Filename);
        fclose(l_File);
        return false;
    }
    else if (l_FileSize > p_RAM->m_SRAMSize)
    {
        TM_error("File '%s' is too large to fit in the allocated SRAM buffer.", p_Filename);
        fclose(l_File);
        return false;
    }

    // Read the SRAM data from the file.
    size_t l_ReadBytes = fread(p_RAM->m_SRAM, 1, p_RAM->m_SRAMSize, l_File);
    if (l_ReadBytes != p_RAM->m_SRAMSize || (ferror(l_File) && !feof(l_File)))
    {
        TM_perror("Failed to read SRAM data from '%s'", p_Filename);
        fclose(l_File);
        return false;
    }

    // Close the file.
    fclose(l_File);
    return true;
}

bool TOMBOY_SaveSRAM (const TOMBOY_RAM* p_RAM, const char* p_Filename)
{
    // Ensure the RAM context is not NULL.
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return false;
    }

    // Ensure the progrma has SRAM.
    if (p_RAM->m_SRAMSize == 0)
    {
        TM_error("Cannot save SRAM file '%s': Reserved SRAM size is zero.", p_Filename);
        return false;
    }

    // Ensure the filename is not NULL or blank.
    if (p_Filename == NULL || p_Filename[0] == '\0')
    {
        TM_error("Filename is NULL or blank.");
        return false;
    }

    // Open the file for writing.
    FILE* l_File = fopen(p_Filename, "wb");
    if (l_File == NULL)
    {
        TM_perror("Failed to open file '%s' for writing", p_Filename);
        return false;
    }

    // Write the SRAM data to the file.
    size_t l_WrittenBytes = fwrite(p_RAM->m_SRAM, 1, p_RAM->m_SRAMSize, l_File);
    if (l_WrittenBytes != p_RAM->m_SRAMSize || (ferror(l_File)))
    {
        TM_perror("Failed to write SRAM data to '%s'", p_Filename);
        fclose(l_File);
        return false;
    }

    // Close the file.
    fclose(l_File);
    return true;
}

// Public Function Prototypes - Memory Access //////////////////////////////////////////////////////

uint8_t TOMBOY_ReadWRAMByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address)
{
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return 0xFF;
    }

    if (p_Address >= TOMBOY_WRAM_SIZE)
    {
        TM_error("WRAM read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    if (p_RAM->m_WRAM == NULL || p_Address >= p_RAM->m_WRAMSize)
    {
        return 0xFF;
    }

    return p_RAM->m_WRAM[p_Address];
}

void TOMBOY_WriteWRAMByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value)
{
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return;
    }

    if (p_Address >= TOMBOY_WRAM_SIZE)
    {
        TM_error("WRAM write address $%08X is out of bounds.", p_Address);
        return;
    }

    if (p_RAM->m_WRAM == NULL || p_Address >= p_RAM->m_WRAMSize)
    {
        return;
    }

    p_RAM->m_WRAM[p_Address] = p_Value;
}

uint8_t TOMBOY_ReadSRAMByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address)
{
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return 0xFF;
    }

    if (p_Address >= TOMBOY_SRAM_SIZE)
    {
        TM_error("SRAM read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    if (p_RAM->m_SRAM == NULL || p_Address >= p_RAM->m_SRAMSize)
    {
        return 0xFF;
    }

    return p_RAM->m_SRAM[p_Address];
}

void TOMBOY_WriteSRAMByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value)
{
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return;
    }

    if (p_Address >= TOMBOY_SRAM_SIZE)
    {
        TM_error("SRAM write address $%08X is out of bounds.", p_Address);
        return;
    }

    if (p_RAM->m_SRAM == NULL || p_Address >= p_RAM->m_SRAMSize)
    {
        return;
    }

    p_RAM->m_SRAM[p_Address] = p_Value;
}

uint8_t TOMBOY_ReadQRAMByte (const TOMBOY_RAM* p_RAM, uint32_t p_Address)
{
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return 0xFF;
    }

    if (p_Address >= 0x10000)
    {
        TM_error("QRAM read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    return p_RAM->m_QRAM[p_Address];
}

void TOMBOY_WriteQRAMByte (TOMBOY_RAM* p_RAM, uint32_t p_Address, uint8_t p_Value)
{
    if (p_RAM == NULL)
    {
        TM_error("RAM context is NULL.");
        return;
    }

    if (p_Address >= 0x10000)
    {
        TM_error("QRAM write address $%08X is out of bounds.", p_Address);
        return;
    }

    p_RAM->m_QRAM[p_Address] = p_Value;
}
