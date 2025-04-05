/**
 * @file  TOMBOY/PPU.c
 */

#include <TOMBOY/Engine.h>
#include <TOMBOY/PPU.h>

// Static Constants ////////////////////////////////////////////////////////////////////////////////

static const uint32_t TOMBOY_PPU_DMG_PALETTE[4] =
{
    0xFFFFFFFF,
    0xC0C0C0FF, 
    0x808080FF, 
    0x000000FF 
};

static const uint8_t TOMBOY_PPU_DMG_PALETTE_RGB555[8] = {
    // White (RGBA8888: 0xFFFFFFFF, RGB555 LE: 0b11111111 0b11111110)
    0b11111111, 0b11111110,

    // Light Gray (RGBA8888: 0xC0C0C0FF, RGB555 LE: 0b11000110 0b00110000)
    0b11000110, 0b00110000,

    // Dark Gray (RGBA8888: 0x808080FF, RGB555 LE: 0b10000100 0b00100000)
    0b10000100, 0b00100000,

    // Black (RGBA8888: 0x000000FF, RGB555 LE: 0b00000000 0b00000000)
    0b00000000, 0b00000000
};

// TOMBOY PPU Context Structure ////////////////////////////////////////////////////////////////////

typedef struct TOMBOY_PPU
{

    // Parent Engine
    TOMBOY_Engine*  m_ParentEngine;     ///< @brief A pointer to the PPU's parent engine context.

    // Callback Functions
    TOMBOY_FrameRenderedCallback m_OnFrameRendered; ///< @brief The callback function to call when a frame is rendered.

    // Pixel Fetcher
    TOMBOY_PixelFetcher m_PixelFetcher;     ///< @brief The pixel fetcher.

    // Memory Buffers
    uint32_t        m_ScreenBuffer[TOMBOY_PPU_SCREEN_PIXEL_SIZE];   ///< @brief The screen buffer.
    uint8_t         m_VRAM0[TOMBOY_PPU_VRAM_BANK_SIZE];             ///< @brief The first VRAM bank.
    uint8_t         m_VRAM1[TOMBOY_PPU_VRAM_BANK_SIZE];             ///< @brief The second VRAM bank.
    TOMBOY_Object   m_OAM[TOMBOY_PPU_OAM_SIZE];                     ///< @brief The object attribute memory (OAM) buffer.
    uint8_t         m_BgCRAM[TOMBOY_PPU_CRAM_SIZE];                 ///< @brief The background color RAM (CRAM) buffer.
    uint8_t         m_ObjCRAM[TOMBOY_PPU_CRAM_SIZE];                ///< @brief The object color RAM (CRAM) buffer.
    uint8_t*        m_VRAM;                                         ///< @brief A pointer to the currently-selected VRAM bank.

    // Hardware Registers
    TOMBOY_DisplayControl           m_LCDC;     ///< @brief The display control register.
    TOMBOY_DisplayStatus            m_STAT;     ///< @brief The display status register.
    uint8_t                         m_SCY;      ///< @brief The background scroll Y-coordinate register.
    uint8_t                         m_SCX;      ///< @brief The background scroll X-coordinate register.
    uint8_t                         m_LY;       ///< @brief The LCDC Y-coordinate register.
    uint8_t                         m_LYC;      ///< @brief The line compare register.
    uint8_t                         m_DMA3;     ///< @brief The OAM DMA Byte 3 register.
    uint8_t                         m_DMA2;     ///< @brief The OAM DMA Byte 2 register.
    uint8_t                         m_DMA1;     ///< @brief The OAM DMA Byte 1 register.
    uint8_t                         m_DMA;      ///< @brief The OAM DMA status register.
    uint8_t                         m_BGP;      ///< @brief The background palette register.
    uint8_t                         m_OBP0;     ///< @brief The object palette 0 register.
    uint8_t                         m_OBP1;     ///< @brief The object palette 1 register.
    uint8_t                         m_WY;       ///< @brief The window Y-coordinate register.
    uint8_t                         m_WX;       ///< @brief The window X-coordinate register.
    uint8_t                         m_VBK;      ///< @brief The VRAM bank register.
    uint8_t                         m_HDMA1;    ///< @brief The HDMA source address byte 3 register.
    uint8_t                         m_HDMA2;    ///< @brief The HDMA source address byte 2 register.
    uint8_t                         m_HDMA3;    ///< @brief The HDMA source address byte 1 register.
    uint8_t                         m_HDMA4;    ///< @brief The HDMA source address byte 0 register.
    uint8_t                         m_HDMA5;    ///< @brief The HDMA destination address byte 1 register.
    uint8_t                         m_HDMA6;    ///< @brief The HDMA destination address byte 0 register.
    TOMBOY_HDMAControl              m_HDMA7;    ///< @brief The HDMA transfer length and control register.
    TOMBOY_PaletteSpecification     m_BGPI;     ///< @brief The background palette index register.
    TOMBOY_PaletteSpecification     m_OBPI;     ///< @brief The object palette index register.
    uint8_t                         m_OPRI;     ///< @brief The object priority register.
    uint8_t                         m_GRPM;     ///< @brief The graphics mode register.

    // Internal State
    uint8_t     m_WindowLine;                   ///< @brief The current line of the window layer.
    uint16_t    m_CurrentDot;                   ///< @brief The current dot in the current scanline.
    uint32_t    m_ODMASource;                   ///< @brief The source address of the OAM DMA transfer.
    uint8_t     m_ODMATicks;                    ///< @brief The number of ticks elapsed in the OAM DMA transfer, also indicates the low byte of the destination address.
    uint8_t     m_ODMADelay;                    ///< @brief The number of ticks to wait until initiating the OAM DMA transfer.
    uint32_t    m_HDMASource;                   ///< @brief The source address of the HDMA transfer.
    uint16_t    m_HDMADestination;              ///< @brief The destination address of the HDMA transfer, relative to the start of VRAM.
    uint8_t     m_HDMABlocksLeft;               ///< @brief The number of 16-byte blocks left to transfer in the HDMA transfer.
    uint8_t     m_LineObjectIndices[10];        ///< @brief The indices of the objects residing on the current scanline.
    uint8_t     m_LineObjectCount;              ///< @brief The number of objects residing on the current scanline.
    uint32_t    m_InactiveDivider;              ///< @brief A divider to increment when the PPU is disabled.

} TOMBOY_PPU;

// Static Function Prototypes - Misc. Helper Functions /////////////////////////////////////////////

static bool TOMBOY_IsWindowVisible (TOMBOY_PPU* p_PPU);
static void TOMBOY_IncrementLY (TOMBOY_PPU* p_PPU);

// Static Function Prototypes - Object Scan ////////////////////////////////////////////////////////

static void TOMBOY_ClearLineObjects (TOMBOY_PPU* p_PPU);
static void TOMBOY_FindLineObject (TOMBOY_PPU* p_PPU);

// Static Function Prototypes - Pixel Transfer /////////////////////////////////////////////////////

static uint32_t TOMBOY_GetBackgroundColorInternal (TOMBOY_PPU* p_PPU, uint8_t p_PaletteIndex, uint8_t p_ColorIndex, TOMBOY_ColorRGB555* p_RGB555);
static uint32_t TOMBOY_GetObjectColorInternal (TOMBOY_PPU* p_PPU, uint8_t p_PaletteIndex, uint8_t p_ColorIndex, TOMBOY_ColorRGB555* p_RGB555);
static void TOMBOY_PushColor (TOMBOY_PixelFetcher* p_Fetcher, uint32_t p_Color);
static void TOMBOY_PopColor (TOMBOY_PixelFetcher* p_Fetcher, uint32_t* p_Color);
static bool TOMBOY_TryAddPixel (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_ShiftNextPixel (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static uint32_t TOMBOY_FetchObjectPixel (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher, uint8_t p_Bit, uint8_t p_ColorIndex, uint32_t p_RGBAColorValue, uint8_t p_BGWindowPriority);
static void TOMBOY_FetchBackgroundTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_FetchWindowTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_FetchObjectTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_FetchObjectTileData (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher, uint8_t p_Offset);
static void TOMBOY_FetchTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_FetchTileDataLow (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_FetchTileDataHigh (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_FetchPushPixels (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_FetchSleep (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_TickPixelFetcher (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);
static void TOMBOY_ResetPixelFetcher (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher);

// Static Function Prototypes - PPU State Machine //////////////////////////////////////////////////

static void TOMBOY_TickHorizontalBlank (TOMBOY_PPU* p_PPU);
static void TOMBOY_TickVerticalBlank (TOMBOY_PPU* p_PPU);
static void TOMBOY_TickObjectScan (TOMBOY_PPU* p_PPU);
static void TOMBOY_TickPixelTransfer (TOMBOY_PPU* p_PPU);

// Static Function Prototypes - Direct Memory Access (DMA) /////////////////////////////////////////

static void TOMBOY_TickODMA (TOMBOY_PPU* p_PPU);
static void TOMBOY_TickHDMA (TOMBOY_PPU* p_PPU);

// Static Functions - Misc. Helper Functions ///////////////////////////////////////////////////////

bool TOMBOY_IsWindowVisible (TOMBOY_PPU* p_PPU)
{
    return  p_PPU->m_LCDC.m_WindowEnable == 1 && 
            p_PPU->m_WX <= 166 && 
            p_PPU->m_WY < TOMBOY_PPU_SCREEN_HEIGHT;
}

void TOMBOY_IncrementLY (TOMBOY_PPU* p_PPU)
{
    
    // Check to see if the internal window line counter needs to be incremented, as well.
    if (
        TOMBOY_IsWindowVisible(p_PPU) == true &&
        p_PPU->m_LY >= p_PPU->m_WY &&
        p_PPU->m_LY < p_PPU->m_WY + TOMBOY_PPU_SCREEN_HEIGHT
    )
    {
        p_PPU->m_WindowLine++;
    }

    // Increment the LY register. Set the line coincidence bit in the STAT register if LY matches LYC.
    p_PPU->m_STAT.m_LineCoincidence = (++p_PPU->m_LY == p_PPU->m_LYC);

    // If the coincidence bit has been set, and its STAT source is enabled, then request an `LCD_STAT`
    // interrupt.
    if (p_PPU->m_STAT.m_LineCoincidence == 1 && p_PPU->m_STAT.m_LineCoincidenceStatSource == 1)
    {
        TOMBOY_RequestInterrupt(p_PPU->m_ParentEngine, TOMBOY_IT_LCDSTAT);
    }

}

// Static Functions - Object Scan //////////////////////////////////////////////////////////////////

void TOMBOY_ClearLineObjects (TOMBOY_PPU* p_PPU)
{
    p_PPU->m_LineObjectCount = 0;
}

void TOMBOY_FindLineObject (TOMBOY_PPU* p_PPU)
{

    // There is a limit of 10 objects per scanline. Don't process more than 10 objects.
    if (p_PPU->m_LineObjectCount >= 10)
    {
        return;
    }

    // Check the `LCDC` register for the current object height.
    uint8_t l_ObjectHeight = (p_PPU->m_LCDC.m_ObjectSize == 1) ? 16 : 8;

    // The index of the object to seek in the OAM buffer depends on the current scanline dot.
    uint8_t l_ObjectIndex = (p_PPU->m_CurrentDot / 2) % TOMBOY_PPU_OBJECT_COUNT;

    // Pointer to the object in the OAM buffer.
    const TOMBOY_Object* l_Object = &p_PPU->m_OAM[l_ObjectIndex];

    // Check if the object is visible on the current scanline.
    bool l_ObjectIsVisible =
        l_Object->m_X > 0 &&
        p_PPU->m_LY + 16 >= l_Object->m_Y &&
        p_PPU->m_LY + 16 < l_Object->m_Y + l_ObjectHeight;

    // If the object is visible, add it to the list of objects on the current scanline.
    if (l_ObjectIsVisible == true)
    {
        p_PPU->m_LineObjectIndices[p_PPU->m_LineObjectCount++] = l_ObjectIndex;
    }

    // If `GRPM` is set to zero (DMG mode), or if `OPRI` is non-zero (priority by X position), then
    // the objects need to be sorted by their X position:
    // - Objects with smaller X positions have higher priority.
    // - Objects with the same X position are assigned priority based on their index in the OAM buffer.
    if (p_PPU->m_GRPM == 0 || p_PPU->m_OPRI != 0)
    {
        // Sort the objects by their X position.
        for (uint8_t i = 0; i < p_PPU->m_LineObjectCount; i++)
        {
            for (uint8_t j = i + 1; j < p_PPU->m_LineObjectCount; j++)
            {
                // Get the objects to compare.
                const TOMBOY_Object* l_ObjectA = &p_PPU->m_OAM[p_PPU->m_LineObjectIndices[i]];
                const TOMBOY_Object* l_ObjectB = &p_PPU->m_OAM[p_PPU->m_LineObjectIndices[j]];

                // Compare the X positions of the objects.
                if (l_ObjectA->m_X > l_ObjectB->m_X)
                {
                    // Swap the object indices.
                    uint8_t l_Temp = p_PPU->m_LineObjectIndices[i];
                    p_PPU->m_LineObjectIndices[i] = p_PPU->m_LineObjectIndices[j];
                    p_PPU->m_LineObjectIndices[j] = l_Temp;
                }
            }
        }
    }

}

// Static Functions - Pixel Transfer ///////////////////////////////////////////////////////////////

uint32_t TOMBOY_GetBackgroundColorInternal (TOMBOY_PPU* p_PPU, uint8_t p_PaletteIndex, uint8_t p_ColorIndex, TOMBOY_ColorRGB555* p_RGB555)
{
    // Correct the palette index to be in the range of 0-7.
    p_PaletteIndex &= 0b111;

    // Correct the color index to be in the range of 0-3.
    p_ColorIndex &= 0b11;

    // Determine the start index of the color in the CRAM buffer.
    uint8_t l_StartIndex = (p_PaletteIndex * TOMBOY_PPU_PALETTE_COLOR_COUNT * 2) + (p_ColorIndex * 2);

    // Get the color data from the CRAM buffer.
    uint8_t l_ColorData[2] = { p_PPU->m_BgCRAM[l_StartIndex], p_PPU->m_BgCRAM[l_StartIndex + 1] };

    // Extract the RGB555 color data. Remember that the color data is laid out as follows:
    // `0bRRRRRGGG` `0bGGBBBBB0`
    uint8_t l_Red   = (l_ColorData[0] & 0b11111000) >> 3;
    uint8_t l_Green = ((l_ColorData[0] & 0b00000111) << 2) | ((l_ColorData[1] & 0b11000000) >> 6);
    uint8_t l_Blue  = (l_ColorData[1] & 0b00111110) >> 1;
    
    // If a color structure was provided, store the color data in it.
    if (p_RGB555 != NULL)
    {
        p_RGB555->m_Red   = l_Red;
        p_RGB555->m_Green = l_Green;
        p_RGB555->m_Blue  = l_Blue;
    }

    // Return the RGBA color value.
    return (
        ((l_Red * 8) << 24) |
        ((l_Green * 8) << 16) |
        ((l_Blue * 8) << 8) |
        0xFF
    );
}

uint32_t TOMBOY_GetObjectColorInternal (TOMBOY_PPU* p_PPU, uint8_t p_PaletteIndex, uint8_t p_ColorIndex, TOMBOY_ColorRGB555* p_RGB555)
{
    // Correct the palette index to be in the range of 0-7.
    p_PaletteIndex &= 0b111;

    // Correct the color index to be in the range of 0-3.
    p_ColorIndex &= 0b11;

    // Determine the start index of the color in the CRAM buffer.
    uint8_t l_StartIndex = (p_PaletteIndex * TOMBOY_PPU_PALETTE_COLOR_COUNT * 2) + (p_ColorIndex * 2);

    // Get the color data from the CRAM buffer.
    uint8_t l_ColorData[2] = { p_PPU->m_ObjCRAM[l_StartIndex], p_PPU->m_ObjCRAM[l_StartIndex + 1] };

    // Extract the RGB555 color data. Remember that the color data is laid out as follows:
    // `0bRRRRRGGG` `0bGGBBBBB0`
    uint8_t l_Red   = (l_ColorData[0] & 0b11111000) >> 3;
    uint8_t l_Green = ((l_ColorData[0] & 0b00000111) << 2) | ((l_ColorData[1] & 0b11000000) >> 6);
    uint8_t l_Blue  = (l_ColorData[1] & 0b00111110) >> 1;

    // If a color structure was provided, store the color data in it.
    if (p_RGB555 != NULL)
    {
        p_RGB555->m_Red   = l_Red;
        p_RGB555->m_Green = l_Green;
        p_RGB555->m_Blue  = l_Blue;
    }

    // Return the RGBA color value.
    return (
        ((l_Red * 8) << 24) |
        ((l_Green * 8) << 16) |
        ((l_Blue * 8) << 8) |
        0xFF
    );
}

void TOMBOY_PushColor (TOMBOY_PixelFetcher* p_Fetcher, uint32_t p_Color)
{
    p_Fetcher->m_PixelFIFO.m_Buffer[p_Fetcher->m_PixelFIFO.m_Tail] = p_Color;
    p_Fetcher->m_PixelFIFO.m_Tail = (p_Fetcher->m_PixelFIFO.m_Tail + 1) % TOMBOY_PPU_FIFO_SIZE;
    p_Fetcher->m_PixelFIFO.m_Size++;
}

void TOMBOY_PopColor (TOMBOY_PixelFetcher* p_Fetcher, uint32_t* p_Color)
{
    *p_Color = p_Fetcher->m_PixelFIFO.m_Buffer[p_Fetcher->m_PixelFIFO.m_Head];
    p_Fetcher->m_PixelFIFO.m_Head = (p_Fetcher->m_PixelFIFO.m_Head + 1) % TOMBOY_PPU_FIFO_SIZE;
    p_Fetcher->m_PixelFIFO.m_Size--;
}

bool TOMBOY_TryAddPixel (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // The pixel fetcher's FIFO is considered full if it contains more than 8 pixels, enough pixels
    // to render the current line of the current tile.
    if (p_Fetcher->m_PixelFIFO.m_Size > 8)
    {
        return false;
    }

    // Get the fetched tile's attributes.
    TOMBOY_TileAttributes l_TileAttributes = p_Fetcher->m_FetchedBGW.m_TileAttributes;

    // Offset the pixel fetcher's X-coordinate by the scroll X register. Ensure that the resultant
    // X-coordinate is within the screen's bounds.
    int32_t l_OffsetX = p_Fetcher->m_FetchingX - (8 - (p_PPU->m_SCX % 8));
    if (l_OffsetX < 0) { return true; }

    // Iterate over the eight pixels which need to be shifted into the pixel FIFO.
    for (uint8_t i = 0; i < 8; ++i)
    {

        // Which bit of the tile data high and low bytes need to be added?
        uint8_t l_Bit = (l_TileAttributes.m_HorizontalFlip == 0) ? 7 - i : i;

        // Grab the proper bit from the tile data low and high bytes.
        uint8_t l_LowBit = (p_Fetcher->m_FetchedBGW.m_TileDataLow >> l_Bit) & 1;
        uint8_t l_HighBit = (p_Fetcher->m_FetchedBGW.m_TileDataHigh >> l_Bit) & 1;

        // Calculate the color index of the pixel.
        uint8_t l_ColorIndex = (l_HighBit << 1) | l_LowBit;

        // If the `GRPM` register is set to 1, then the PPU is in CGB graphics mode. Retrieve the
        // color from the background color RAM.
        uint32_t l_RGBAColorValue = 0;
        if (p_PPU->m_GRPM != 0)
        {
            l_RGBAColorValue = TOMBOY_GetBackgroundColorInternal(
                p_PPU,
                l_TileAttributes.m_PaletteIndex,
                l_ColorIndex,
                NULL
            );
        }
        
        // If the `GRPM` register is set to 0, then the PPU is in DMG graphics mode. The color
        // should not be fetched if `LCDC` bit 0 is clear.
        else if (p_PPU->m_LCDC.m_BGWEnableOrPriority == true)
        {
            // Get the proper two bits from the `BGP` register.
            uint8_t l_BitPair = (p_PPU->m_BGP >> (l_ColorIndex * 2)) & 0b11;

            // Get the RGBA color value from the DMG palette.
            l_RGBAColorValue = TOMBOY_PPU_DMG_PALETTE[l_BitPair];
        }

        // Otherwise, this is DMG mode where the background/window layer is disabled. The pixel is
        // transparent.
        else
        {
            l_RGBAColorValue = TOMBOY_PPU_DMG_PALETTE[0];
        }

        // If the object layer is enabled, and there is at least one object residing on this pixel,
        // then fetch the object pixel's color.
        if (p_PPU->m_LCDC.m_ObjectEnable == true)
        {
            l_RGBAColorValue = TOMBOY_FetchObjectPixel(
                p_PPU,
                p_Fetcher,
                l_Bit,
                l_ColorIndex,
                l_RGBAColorValue,
                p_PPU->m_LCDC.m_BGWEnableOrPriority
            );
        }

        // Shift the pixel into the pixel FIFO.
        TOMBOY_PushColor(p_Fetcher, l_RGBAColorValue);
        p_Fetcher->m_QueueX++;

    }

    return true;

}

void TOMBOY_ShiftNextPixel (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // Only shift a pixel from the FIFO if it's full.
    if (p_Fetcher->m_PixelFIFO.m_Size > 8)
    {

        // Pop the pixel's color from the FIFO.
        uint32_t l_RGBAColorValue = 0;
        TOMBOY_PopColor(p_Fetcher, &l_RGBAColorValue);

        // Ensure that the pixel is within the bounds of the screen buffer.
        if (p_Fetcher->m_LineX >= (p_PPU->m_SCX % 8))
        {

            // Determine the index of the pixel in the screen buffer.
            uint32_t l_ScreenIndex = p_Fetcher->m_PushedX + (p_PPU->m_LY * TOMBOY_PPU_SCREEN_WIDTH);

            // Emplace the pixel into the screen buffer. Advance the fetcher's pushed X-coordinate.
            p_PPU->m_ScreenBuffer[l_ScreenIndex] = l_RGBAColorValue;
            p_Fetcher->m_PushedX++;

        }

        // Move the fetcher's X-coordinate to the next pixel.
        p_Fetcher->m_LineX++;

    }

}

uint32_t TOMBOY_FetchObjectPixel (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher, uint8_t p_Bit, uint8_t p_ColorIndex, uint32_t p_RGBAColorValue, uint8_t p_BGWindowPriority)
{

    // The `p_ColorIndex` parameter contains the index of the color used to render a background-
    // or window-layer tile over which one or more of the objects fetched by the pixel fetcher may
    // be residing.
    //
    // Store that old color index here, as we may need to restore it later.
    uint8_t l_OldColorIndex = p_ColorIndex;

    // Iterate over the indices of the objects fetched by the pixel fetcher as it was processing the
    // current pixel.
    for (uint8_t i = 0; i < p_Fetcher->m_FetchedOBJ.m_ObjectCount; ++i)
    {

        // Point to the object in the OAM buffer.
        const TOMBOY_Object* l_Object = &p_PPU->m_OAM[p_Fetcher->m_FetchedOBJ.m_ObjectIndices[i]];

        // Calculate the X-coordinate of the object's left edge on the screen.
        // Ensure the object is within the bounds of the screen.
        uint8_t l_ObjectX = (l_Object->m_X - 8) + (p_PPU->m_SCX % 8);
        if (l_ObjectX + 8 < p_Fetcher->m_QueueX)
        {
            continue;
        }

        // Calculate the offset of the current pixel within the object's tile data.
        // Ensure the offset is within the bounds of the tile data.
        int8_t l_Offset = p_Fetcher->m_QueueX - l_ObjectX;
        if (l_Offset < 0 || l_Offset >= 8)
        {
            continue;
        }

        // Correct the provided `p_Bit` parameter to account for the object's X-flip attribute.
        p_Bit = (l_Object->m_Attributes.m_HorizontalFlip == 0) ? 7 - l_Offset : l_Offset;

        // Get the proper bit of the object's tile data low and high bytes.
        uint8_t l_LowBit = (p_Fetcher->m_FetchedOBJ.m_TileDataLow[i] >> p_Bit) & 1;
        uint8_t l_HighBit = (p_Fetcher->m_FetchedOBJ.m_TileDataHigh[i] >> p_Bit) & 1;

        // Calculate the color index of the pixel.
        uint8_t l_ColorIndex = (l_HighBit << 1) | l_LowBit;

        // If the color index is zero, then the pixel is transparent and does not overwrite the
        // background or window layer.
        if (l_ColorIndex == 0)
        {
            continue;
        }
        
        // Check if one of the following conditions is met:
        // - The old color index is zero.
        // - The object's BGW priority attribute is zero.
        // - The provided `p_BGWindowPriority` parameter is zero.
        if (
            l_OldColorIndex == 0 ||
            l_Object->m_Attributes.m_Priority == 0 ||
            (
                p_BGWindowPriority == 0 &&
                l_Object->m_Attributes.m_Priority == 0
            )
        )
        {
            // Is the graphics mode set to CGB mode?
            if (p_PPU->m_GRPM == 1)
            {
                p_RGBAColorValue = TOMBOY_GetObjectColorInternal(
                    p_PPU,
                    l_Object->m_Attributes.m_PaletteIndex,
                    l_ColorIndex,
                    NULL
                );
            }

            // Otherwise, the graphics mode is set to DMG mode.
            else
            {
                // Get the proper two bits from the `OBP0` or `OBP1` register.
                uint8_t l_BitPair = (l_Object->m_Attributes.m_DMGPalette == 0) ?
                    (p_PPU->m_OBP0 >> (l_ColorIndex * 2)) & 0b11 :
                    (p_PPU->m_OBP1 >> (l_ColorIndex * 2)) & 0b11;

                // Get the RGBA color value from the DMG palette.
                p_RGBAColorValue = TOMBOY_PPU_DMG_PALETTE[l_BitPair];
            }

            if (p_ColorIndex > 0) { break; }
        }

    }

    // Return the RGBA color value of the pixel.
    return p_RGBAColorValue;

}

void TOMBOY_FetchBackgroundTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // Determine the relative starting address of the tile map used to render the background layer.
    uint16_t l_TileMapAddress = (p_PPU->m_LCDC.m_BGTilemapAddress == 0) ? 0x1800 : 0x1C00;

    // Determine the source Y position of the tile.
    uint8_t l_TileY = p_Fetcher->m_MapY / 8;

    // Use the pixel fetcher's map coordinates to determine the target address offset.
    uint16_t l_TargetOffset = (l_TileY * 32) + (p_Fetcher->m_MapX / 8);
    uint16_t l_TargetAddress = l_TileMapAddress + l_TargetOffset;

    // Fetch the tile number and attributes from VRAM banks 0 and 1, respectively.
    p_Fetcher->m_FetchedBGW.m_TileIndex = p_PPU->m_VRAM0[l_TargetAddress];
    p_Fetcher->m_FetchedBGW.m_TileAttributes.m_Value = p_PPU->m_VRAM1[l_TargetAddress];
}

void TOMBOY_FetchWindowTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // Only fetch a window tile number if the following conditions are met:
    // - The window layer is currently enabled and visible.
    // - The fetcher's next X coordinate is within the bounds of the window layer.
    // - The `LY` register is within the bounds of the window layer.
    if (
        TOMBOY_IsWindowVisible(p_PPU) == true &&
        p_Fetcher->m_FetchingX + 7 >= p_PPU->m_WX &&
        p_Fetcher->m_FetchingX + 7 < (p_PPU->m_WX + TOMBOY_PPU_SCREEN_HEIGHT + 14) &&
        p_PPU->m_LY >= p_PPU->m_WY &&
        p_PPU->m_LY < p_PPU->m_WY + TOMBOY_PPU_SCREEN_WIDTH
    )
    {

        // Determine the relative starting address of the tile map used to render the window layer.
        uint16_t l_TileMapAddress = (p_PPU->m_LCDC.m_WindowTilemapAddress == 0) ? 0x1800 : 0x1C00;

        // Determine the source Y position of the tile.
        uint8_t l_TileY = p_PPU->m_WindowLine / 8;

        // Use the pixel fetcher's map coordinates to determine the target address offset.
        uint16_t l_TargetOffset = (l_TileY * 32) + ((p_Fetcher->m_FetchingX + 7 - p_PPU->m_WX) / 8);
        uint16_t l_TargetAddress = l_TileMapAddress + l_TargetOffset;
        
        // Fetch the tile number and attributes from VRAM banks 0 and 1, respectively.
        p_Fetcher->m_FetchedBGW.m_TileIndex = p_PPU->m_VRAM0[l_TargetAddress];
        p_Fetcher->m_FetchedBGW.m_TileAttributes.m_Value = p_PPU->m_VRAM1[l_TargetAddress];
    }

}

void TOMBOY_FetchObjectTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{
    // Iterate over the indices of the objects found on the current scanline.
    for (uint8_t i = 0; i < p_PPU->m_LineObjectCount; ++i)
    {

        // Point to the object in the OAM buffer.
        const TOMBOY_Object* l_Object = &p_PPU->m_OAM[p_PPU->m_LineObjectIndices[i]];

        // Calculate the object's scrolling-adjusted X-coordinate.
        int16_t l_ObjectX = (l_Object->m_X - 8) + (p_PPU->m_SCX % 8);

        // Ensure that the object's tile data will fit into the pixel fetcher's FIFO.
        if (
            (l_ObjectX >= p_Fetcher->m_FetchingX && l_ObjectX < p_Fetcher->m_FetchingX + 8) ||
            (l_ObjectX + 8 >= p_Fetcher->m_FetchingX && l_ObjectX + 8 < p_Fetcher->m_FetchingX + 8)
        )
        {

            // If it does, then add the object's index to the list of objects whose tile data needs
            // to be fetched.
            p_Fetcher->m_FetchedOBJ.m_ObjectIndices[p_Fetcher->m_FetchedOBJ.m_ObjectCount++] = 
                p_PPU->m_LineObjectIndices[i];

            // Limit 3 objects per pixel.
            if (p_Fetcher->m_FetchedOBJ.m_ObjectCount >= 3)
            {
                break;
            }

        }

    }
}

void TOMBOY_FetchObjectTileData (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher, uint8_t p_Offset)
{

    // Determine the current object size.
    uint8_t l_ObjectHeight = (p_PPU->m_LCDC.m_ObjectSize == 1) ? 16 : 8;

    // Iterate over the indices of the objects found on this pixel, whose tile data needs to be fetched.
    for (uint8_t i = 0; i < p_Fetcher->m_FetchedOBJ.m_ObjectCount; ++i)
    {

        // Point to the object in the OAM buffer.
        const TOMBOY_Object* l_Object = &p_PPU->m_OAM[p_Fetcher->m_FetchedOBJ.m_ObjectIndices[i]];

        // Get the object's Y position on the screen. Adjust the position according to the object's
        // Y-flip attribute.
        uint8_t l_ObjectY = ((p_PPU->m_LY + 16) - l_Object->m_Y) * 2;
        if (l_Object->m_Attributes.m_VerticalFlip == 1)
        {
            l_ObjectY = ((l_ObjectHeight * 2) - 2) - l_ObjectY;
        }

        // Get the object's tile index, with its low bit cleared if tall objects (8x16) are being used.
        uint8_t l_TileIndex = l_Object->m_TileIndex & ((l_ObjectHeight == 16) ? 0xFE : 0xFF);

        // Calculate the target address of the object's tile data.
        uint16_t l_TargetAddress = (l_TileIndex * 16) + (l_ObjectY + p_Offset);

        // Fetch the proper tile data (low if `p_Offset` is 0, high if `p_Offset` is non-zero).
        // The above-calculated address is relative to the start of the VRAM bank, so permission to
        // access the VRAM bank will be checked for.
        if (p_Offset == 0)
        {
            p_Fetcher->m_FetchedOBJ.m_TileDataLow[i] = p_PPU->m_VRAM[l_TargetAddress];
        }
        else
        {
            p_Fetcher->m_FetchedOBJ.m_TileDataHigh[i] = p_PPU->m_VRAM[l_TargetAddress];
        }
        
    }

}

void TOMBOY_FetchTileNumber (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // Clear the fetched object count.
    p_Fetcher->m_FetchedOBJ.m_ObjectCount = 0;

    // Check the graphics mode to determine the order in which the tile numbers are fetched.
    if (p_PPU->m_GRPM == 0)
    {
        // In DMG mode, if `LCDC` bit 0 is clear, then the background/window layer is not rendered
        // at all.
        if (p_PPU->m_LCDC.m_BGWEnableOrPriority == true)
        {
            // Fetch the tile number for the background layer.
            TOMBOY_FetchBackgroundTileNumber(p_PPU, p_Fetcher);

            // If the window layer is enabled, then fetch the tile number for the window layer.
            if (p_PPU->m_LCDC.m_WindowEnable == true)
            {
                TOMBOY_FetchWindowTileNumber(p_PPU, p_Fetcher);
            }
        }

        // If the object layer is enabled, and there are objects on the current scanline, then fetch
        // the tile number for an object.
        if (p_PPU->m_LCDC.m_ObjectEnable == true && p_PPU->m_LineObjectCount > 0)
        {
            TOMBOY_FetchObjectTileNumber(p_PPU, p_Fetcher);
        }
    }
    else
    {
        // In CGB mode, if `LCDC` bit 0 is clear, then the background/window layer does not have
        // priority over the object layer; all objects are rendered on top of the background/window
        // layer, provided they are visible on the current scanline and the object layer is enabled.
        //
        // Otherwise, the background/window layer may have priority over the object layer.
        if (p_PPU->m_LCDC.m_BGWEnableOrPriority == false)
        {
            // If the object layer is enabled, and there are objects on the current scanline, then fetch
            // the tile number for an object.
            if (p_PPU->m_LCDC.m_ObjectEnable == true && p_PPU->m_LineObjectCount > 0)
            {
                TOMBOY_FetchObjectTileNumber(p_PPU, p_Fetcher);
            }

            // Fetch the tile number for the background layer.
            TOMBOY_FetchBackgroundTileNumber(p_PPU, p_Fetcher);

            // If the window layer is enabled, then fetch the tile number for the window layer.
            if (p_PPU->m_LCDC.m_WindowEnable == true)
            {
                TOMBOY_FetchWindowTileNumber(p_PPU, p_Fetcher);
            }
        }
        else
        {
            // Fetch the tile number for the background layer.
            TOMBOY_FetchBackgroundTileNumber(p_PPU, p_Fetcher);

            // If the window layer is enabled, then fetch the tile number for the window layer.
            if (p_PPU->m_LCDC.m_WindowEnable == true)
            {
                TOMBOY_FetchWindowTileNumber(p_PPU, p_Fetcher);
            }

            // If the object layer is enabled, and there are objects on the current scanline, then fetch
            // the tile number for an object.
            if (p_PPU->m_LCDC.m_ObjectEnable == true && p_PPU->m_LineObjectCount > 0)
            {
                TOMBOY_FetchObjectTileNumber(p_PPU, p_Fetcher);
            }
        }
    }

    // Advance the pixel fetcher's X-coordinate by 8 pixels, then move to the next state.
    p_Fetcher->m_FetchingX += 8;
    p_Fetcher->m_Mode = TOMBOY_PFM_TILE_DATA_LOW;

}

void TOMBOY_FetchTileDataLow (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // Get the index of the tile which needs to be fetched.
    uint8_t l_TileIndex = p_Fetcher->m_FetchedBGW.m_TileIndex;

    // Determine the target address to begin fetching the tile data from.
    // Adjust this address, if needed, based on the `LCDC` BGW tile data address bit.
    uint16_t l_TargetAddress = (l_TileIndex * 16) + p_Fetcher->m_TileDataOffset;
    if (l_TileIndex < 128 && p_PPU->m_LCDC.m_BGWindowTileDataAddress == 0)
    {
        l_TargetAddress += 0x1000;
    }

    // Fetch the low byte of the tile data from the current bank in VRAM.
    p_Fetcher->m_FetchedBGW.m_TileDataLow = p_PPU->m_VRAM[l_TargetAddress];

    // If there is an object residing on this pixel, fetch that object's tile data as well.
    TOMBOY_FetchObjectTileData(p_PPU, p_Fetcher, 0);

    // Move to the next state.
    p_Fetcher->m_Mode = TOMBOY_PFM_TILE_DATA_HIGH;

}

void TOMBOY_FetchTileDataHigh (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // The high byte of the tile data is fetched in the same manner as the low byte.
    uint8_t l_TileIndex = p_Fetcher->m_FetchedBGW.m_TileIndex;
    uint16_t l_TargetAddress = (l_TileIndex * 16) + p_Fetcher->m_TileDataOffset + 1;
    if (l_TileIndex < 128 && p_PPU->m_LCDC.m_BGWindowTileDataAddress == 0)
    {
        l_TargetAddress += 0x1000;
    }

    // Fetch the high byte of the tile data.
    p_Fetcher->m_FetchedBGW.m_TileDataHigh = p_PPU->m_VRAM[l_TargetAddress];

    // If there is an object residing on this pixel, fetch that object's tile data as well.
    TOMBOY_FetchObjectTileData(p_PPU, p_Fetcher, 1);

    // Move to the next state.
    p_Fetcher->m_Mode = TOMBOY_PFM_SLEEP;

}

void TOMBOY_FetchPushPixels (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // In this mode, continuous attempts are made to push a pixel to the pixel FIFO buffer.
    if (TOMBOY_TryAddPixel(p_PPU, p_Fetcher) == true)
    {
        // Once successful, return to the tile number state.
        p_Fetcher->m_Mode = TOMBOY_PFM_TILE_NUMBER;
    }

}

void TOMBOY_FetchSleep (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{
    // Here, the fetcher sleeps for two dots before pushing the pixels to the screen buffer.
    p_Fetcher->m_Mode = TOMBOY_PFM_PUSH_PIXELS;
}

void TOMBOY_TickPixelFetcher (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // Calculate the absolute position of the pixel to be processed, in the 256x256 pixel tilemap.
    p_Fetcher->m_MapY = p_PPU->m_LY + p_PPU->m_SCY;
    p_Fetcher->m_MapX = p_Fetcher->m_FetchingX + p_PPU->m_SCX;

    // Calculate the offset address of the tile data to be fetched.
    p_Fetcher->m_TileDataOffset = (p_Fetcher->m_MapY % 8) * 2;

    // During each even-numbered dot in the pixel transfer mode, the pixel fetcher will work
    // to keep supplying its pixel FIFO buffer with new pixels to draw to the screen buffer.
    if (p_PPU->m_CurrentDot % 2 == 0)
    {

        // Run the appropriate function in the pixel-fetcher's state machine.
        switch (p_Fetcher->m_Mode)
        {
            case TOMBOY_PFM_TILE_NUMBER: TOMBOY_FetchTileNumber(p_PPU, p_Fetcher); break;
            case TOMBOY_PFM_TILE_DATA_LOW: TOMBOY_FetchTileDataLow(p_PPU, p_Fetcher); break;
            case TOMBOY_PFM_TILE_DATA_HIGH: TOMBOY_FetchTileDataHigh(p_PPU, p_Fetcher); break;
            case TOMBOY_PFM_PUSH_PIXELS: TOMBOY_FetchPushPixels(p_PPU, p_Fetcher); break;
            case TOMBOY_PFM_SLEEP: TOMBOY_FetchSleep(p_PPU, p_Fetcher); break;
        }

    }

    // In any event, try to shift a pixel from the FIFO buffer into the screen buffer.
    TOMBOY_ShiftNextPixel(p_PPU, p_Fetcher);

}

void TOMBOY_ResetPixelFetcher (TOMBOY_PPU* p_PPU, TOMBOY_PixelFetcher* p_Fetcher)
{

    // Reset the pixel FIFO buffer.
    p_Fetcher->m_PixelFIFO.m_Head = 0;
    p_Fetcher->m_PixelFIFO.m_Tail = 0;
    p_Fetcher->m_PixelFIFO.m_Size = 0;

}

// Static Functions - PPU State Machine ////////////////////////////////////////////////////////////

void TOMBOY_TickHorizontalBlank (TOMBOY_PPU* p_PPU)
{

    // Increment the current dot.
    p_PPU->m_CurrentDot++;

    // If the current dot is at least 456, then the horizontal blank is complete.
    if (p_PPU->m_CurrentDot >= 456)
    {

        // Increment the LY register.
        TOMBOY_IncrementLY(p_PPU);

        // If all 144 visible scanlines of the current frame have been rendered, then the vertical
        // blank period has begun.
        if (p_PPU->m_LY >= TOMBOY_PPU_SCREEN_HEIGHT)
        {
            // Move to the vertical blank state and request the `VBLANK` interrupt.
            p_PPU->m_STAT.m_DisplayMode = TOMBOY_DM_VERTICAL_BLANK;
            TOMBOY_RequestInterrupt(p_PPU->m_ParentEngine, TOMBOY_IT_VBLANK);

            // If the `LCD_STAT` interrupt source is enabled for the vertical blank period, then
            // request the `LCD_STAT` interrupt as well.
            if (p_PPU->m_STAT.m_VerticalBlankStatSource == true)
            {
                TOMBOY_RequestInterrupt(p_PPU->m_ParentEngine, TOMBOY_IT_LCDSTAT);
            }

            // If the frame rendered callback is provided, call it here.
            if (p_PPU->m_OnFrameRendered != NULL)
            {
                p_PPU->m_OnFrameRendered(p_PPU);
            }
        }

        // If there are still visible scanlines to render, then move to the object scan state.
        else
        {
            p_PPU->m_STAT.m_DisplayMode = TOMBOY_DM_OBJECT_SCAN;
            p_PPU->m_LineObjectCount = 0;

            // If its stat source is set, request the `LCD_STAT` interrupt.
            if (p_PPU->m_STAT.m_ObjectScanStatSource == true)
            {
                TOMBOY_RequestInterrupt(p_PPU->m_ParentEngine, TOMBOY_IT_LCDSTAT);
            }
        }

        // Reset the current dot.
        p_PPU->m_CurrentDot = 0;

    }

}

void TOMBOY_TickVerticalBlank (TOMBOY_PPU* p_PPU)
{
    
    // Increment the current dot.
    p_PPU->m_CurrentDot++;

    // If the current dot is at least 456, then the vertical blank is complete.
    if (p_PPU->m_CurrentDot >= 456)
    {

        // Increment the LY register.
        TOMBOY_IncrementLY(p_PPU);

        // If all 154 scanlines have been processed, then the vertical blank period is complete.
        if (p_PPU->m_LY >= TOMBOY_PPU_SCANLINE_COUNT)
        {

            // Reset the LY register and window line counter.
            p_PPU->m_LY = 0;
            p_PPU->m_WindowLine = 0;

            // Move to the object scan state and begin processing the next frame.
            p_PPU->m_STAT.m_DisplayMode = TOMBOY_DM_OBJECT_SCAN;
            p_PPU->m_LineObjectCount = 0;

            // If its stat source is set, request the `LCD_STAT` interrupt.
            if (p_PPU->m_STAT.m_ObjectScanStatSource == true)
            {
                TOMBOY_RequestInterrupt(p_PPU->m_ParentEngine, TOMBOY_IT_LCDSTAT);
            }

        }

        // Reset the current dot.
        p_PPU->m_CurrentDot = 0;

    }
}

void TOMBOY_TickObjectScan (TOMBOY_PPU* p_PPU)
{

    // Increment the current dot. Save the old dot tick.
    uint16_t l_Dot = p_PPU->m_CurrentDot++;

    // If the incremented dot is at least 80, then the object scan is complete. Move to the pixel
    // transfer state.
    if (p_PPU->m_CurrentDot >= 80)
    {
        p_PPU->m_STAT.m_DisplayMode = TOMBOY_DM_PIXEL_TRANSFER;        

        TOMBOY_PixelFetcher* l_Fetcher = &p_PPU->m_PixelFetcher;
        l_Fetcher->m_Mode = TOMBOY_PFM_TILE_NUMBER;
        l_Fetcher->m_FetchingX = 0;
        l_Fetcher->m_QueueX = 0;
        l_Fetcher->m_LineX = 0;
        l_Fetcher->m_PushedX = 0;
    }
    else if (l_Dot % 2 == 0)
    {
        // Find the next object on the current scanline.
        TOMBOY_FindLineObject(p_PPU);
    }

}

void TOMBOY_TickPixelTransfer (TOMBOY_PPU* p_PPU)
{

    // Tick the pixel fetcher.
    TOMBOY_TickPixelFetcher(p_PPU, &p_PPU->m_PixelFetcher);

    // Increment the current dot.
    p_PPU->m_CurrentDot++;

    // If the pixel fetcher has pushed enough pixels to the screen buffer to fill a scanline, then
    // the pixel transfer is complete. Move to the horizontal blank state.
    if (p_PPU->m_PixelFetcher.m_PushedX >= TOMBOY_PPU_SCREEN_WIDTH)
    {
        
        // Reset the pixel fetcher.
        TOMBOY_ResetPixelFetcher(p_PPU, &p_PPU->m_PixelFetcher);

        // Move to the horizontal blank state. If its stat source is set, request the `LCD_STAT`
        // interrupt.
        p_PPU->m_STAT.m_DisplayMode = TOMBOY_DM_HORIZONTAL_BLANK;
        if (p_PPU->m_STAT.m_HorizontalBlankStatSource == true)
        {
            TOMBOY_RequestInterrupt(p_PPU->m_ParentEngine, TOMBOY_IT_LCDSTAT);
        }

        // At the start of each H-Blank period, another block of HDMA data is transferred.
        TOMBOY_TickHDMA(p_PPU);

    }

}

// Static Functions - HDMA Transfer ////////////////////////////////////////////////////////////////

void TOMBOY_TickODMA (TOMBOY_PPU* p_PPU)
{
    // Check to see if the DMA transfer is active.
    if (p_PPU->m_ODMATicks >= 0xA0)
    {
        return;
    }

    // If the ODMA transfer has been initiated, check to see if there is a delay still before
    // actually starting the transfer.
    if (p_PPU->m_ODMADelay > 0)
    {
        p_PPU->m_ODMADelay--;
        return;
    }

    // If the ODMA transfer is active, then transfer the next byte of data.
    uint8_t l_Value = TM_ReadByte(TOMBOY_GetCPU(p_PPU->m_ParentEngine), p_PPU->m_ODMASource + p_PPU->m_ODMATicks);
    TOMBOY_WriteOAMByte(p_PPU, TOMBOY_OAM_START + (p_PPU->m_ODMATicks++), l_Value);
}

void TOMBOY_TickHDMA (TOMBOY_PPU* p_PPU)
{
    if (p_PPU->m_HDMABlocksLeft > 0)
    {
        // If the HDMA transfer is active, then decrement the number of blocks left to transfer.
        p_PPU->m_HDMABlocksLeft--;

        // Transfer the next block of data
        for (uint8_t i = 0; i < 0x10; i++)
        {
            uint8_t l_Value = TM_ReadByte(TOMBOY_GetCPU(p_PPU->m_ParentEngine), p_PPU->m_HDMASource++);
            TOMBOY_WriteVRAMByte(p_PPU, TOMBOY_VRAM_START + (p_PPU->m_HDMADestination++), l_Value);
        }
    }
}

// Public Functions ////////////////////////////////////////////////////////////////////////////////

TOMBOY_PPU* TOMBOY_CreatePPU (TOMBOY_Engine* p_Engine)
{
    // Allocate the GABLE PPU instance.
    TOMBOY_PPU* l_PPU = TM_calloc(1, TOMBOY_PPU);
    TM_pexpect(l_PPU != NULL, "Failed to allocate GABLE PPU instance");
    
    // Set the parent engine.
    l_PPU->m_ParentEngine = p_Engine;
    TOMBOY_ResetPPU(l_PPU);

    // Return the PPU instance.
    return l_PPU;
}

void TOMBOY_DestroyPPU (TOMBOY_PPU* p_PPU)
{
    if (p_PPU != NULL)
    {
        // Free the PPU instance.
        TM_free(p_PPU);
    }
}

void TOMBOY_ResetPPU (TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Clear the PPU structure's memory.
    memset(p_PPU, 0, sizeof(TOMBOY_PPU));

    // Set the default values for the PPU registers.
    /* LCDC     = 0x91 */   p_PPU->m_LCDC.m_Register    = 0x91; // 0b10010001
    /* STAT     = 0x85 */   p_PPU->m_STAT.m_Register    = 0x85; // 0b10000101
    /* SCY      = 0x00 */   p_PPU->m_SCY                = 0x00;
    /* SCX      = 0x00 */   p_PPU->m_SCX                = 0x00;
    /* LY       = 0x00 */   p_PPU->m_LY                 = 0x00;
    /* LYC      = 0x00 */   p_PPU->m_LYC                = 0x00;
    /* BGP      = 0xFC */   p_PPU->m_BGP                = 0b00011011; // Color indices: 0, 1, 2, 3
    /* OBP0     = 0xFF */   p_PPU->m_OBP0               = 0b00011011; // Color indices: 0, 1, 2, 3
    /* OBP1     = 0xFF */   p_PPU->m_OBP1               = 0b00011011; // Color indices: 0, 1, 2, 3
    /* WY       = 0x00 */   p_PPU->m_WY                 = 0x00;
    /* WX       = 0x00 */   p_PPU->m_WX                 = 0x00;
    /* DMA3     = 0x00 */   p_PPU->m_DMA3               = 0x00;
    /* DMA2     = 0x00 */   p_PPU->m_DMA2               = 0x00;
    /* DMA1     = 0x30 */   p_PPU->m_DMA1               = 0x30;
    /* DMA      = 0x00 */   p_PPU->m_DMA                = 0xFF;
    /* VBK      = 0x00 */   p_PPU->m_VBK                = 0x00;
    /* HDMA1    = 0xFF */   p_PPU->m_HDMA1              = 0xFF;
    /* HDMA2    = 0xFF */   p_PPU->m_HDMA2              = 0xFF;
    /* HDMA3    = 0xFF */   p_PPU->m_HDMA3              = 0xFF;
    /* HDMA4    = 0xFF */   p_PPU->m_HDMA4              = 0xFF;
    /* HDMA5    = 0xFF */   p_PPU->m_HDMA5              = 0xFF;
    /* HDMA6    = 0xFF */   p_PPU->m_HDMA6              = 0xFF;
    /* HDMA7    = 0xFF */   p_PPU->m_HDMA7.m_Register   = 0xFF; // 0b11111111
    /* BGPI     = 0x00 */   p_PPU->m_BGPI.m_Register    = 0x00;
    /* OBPI     = 0x00 */   p_PPU->m_OBPI.m_Register    = 0x00;
    /* OPRI     = 0x00 */   p_PPU->m_OPRI               = 0x00;
    /* GRPM     = 0x01 */   p_PPU->m_GRPM               = 0x01;

    // Initialize the Color RAM buffers using the DMG palette.
    for (uint8_t i = 0; i < TOMBOY_PPU_CRAM_SIZE; i += 8)
    {
        p_PPU->m_BgCRAM[i + 0] = TOMBOY_PPU_DMG_PALETTE_RGB555[0];
        p_PPU->m_BgCRAM[i + 1] = TOMBOY_PPU_DMG_PALETTE_RGB555[1];
        p_PPU->m_BgCRAM[i + 2] = TOMBOY_PPU_DMG_PALETTE_RGB555[2];
        p_PPU->m_BgCRAM[i + 3] = TOMBOY_PPU_DMG_PALETTE_RGB555[3];
        p_PPU->m_BgCRAM[i + 4] = TOMBOY_PPU_DMG_PALETTE_RGB555[4];
        p_PPU->m_BgCRAM[i + 5] = TOMBOY_PPU_DMG_PALETTE_RGB555[5];
        p_PPU->m_BgCRAM[i + 6] = TOMBOY_PPU_DMG_PALETTE_RGB555[6];
        p_PPU->m_BgCRAM[i + 7] = TOMBOY_PPU_DMG_PALETTE_RGB555[7];

        p_PPU->m_ObjCRAM[i + 0] = TOMBOY_PPU_DMG_PALETTE_RGB555[0];
        p_PPU->m_ObjCRAM[i + 1] = TOMBOY_PPU_DMG_PALETTE_RGB555[1];
        p_PPU->m_ObjCRAM[i + 2] = TOMBOY_PPU_DMG_PALETTE_RGB555[2];
        p_PPU->m_ObjCRAM[i + 3] = TOMBOY_PPU_DMG_PALETTE_RGB555[3];
        p_PPU->m_ObjCRAM[i + 4] = TOMBOY_PPU_DMG_PALETTE_RGB555[4];
        p_PPU->m_ObjCRAM[i + 5] = TOMBOY_PPU_DMG_PALETTE_RGB555[5];
        p_PPU->m_ObjCRAM[i + 6] = TOMBOY_PPU_DMG_PALETTE_RGB555[6];
        p_PPU->m_ObjCRAM[i + 7] = TOMBOY_PPU_DMG_PALETTE_RGB555[7];
    }

    // Point the VRAM pointer to bank 0.
    p_PPU->m_VRAM = p_PPU->m_VRAM0;

    // Reset the PPU's internal state.
    p_PPU->m_CurrentDot = 0;
    p_PPU->m_ODMATicks = 0xFF;
    p_PPU->m_ODMADelay = 0;
    p_PPU->m_ODMASource = 0;
    p_PPU->m_HDMABlocksLeft = 0;
    p_PPU->m_HDMASource = 0;
    p_PPU->m_HDMADestination = 0;
    p_PPU->m_LineObjectCount = 0;
    p_PPU->m_InactiveDivider = 0;

    // Reset the PPU's display mode and pixel fetch mode.
    p_PPU->m_STAT.m_DisplayMode = TOMBOY_DM_OBJECT_SCAN;
    p_PPU->m_PixelFetcher.m_Mode = TOMBOY_PFM_TILE_NUMBER;
}

void TOMBOY_SetFrameRenderedCallback (TOMBOY_PPU* p_PPU, TOMBOY_FrameRenderedCallback p_Callback)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the frame rendered callback.
    p_PPU->m_OnFrameRendered = p_Callback;
}

void TOMBOY_TickPPU (TOMBOY_PPU* p_PPU, bool p_ODMA)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // If the PPU is disabled, then do not tick the state machine.
    if (p_PPU->m_LCDC.m_DisplayEnable == false)
    {
        // Instead, increment the inactive divider. If the inactive divider reaches the number of
        // dots in a frame, then call the frame rendered callback, if it is set.
        p_PPU->m_InactiveDivider = (p_PPU->m_InactiveDivider + 1) % TOMBOY_PPU_DOTS_PER_FRAME;
        if (p_PPU->m_InactiveDivider == 0 && p_PPU->m_OnFrameRendered != NULL)
        {
            p_PPU->m_OnFrameRendered(p_PPU);
        }

        return;
    }

    // Check the current display mode and tick the appropriate state machine.
    switch (p_PPU->m_STAT.m_DisplayMode)
    {
        case TOMBOY_DM_HORIZONTAL_BLANK: TOMBOY_TickHorizontalBlank(p_PPU); break;
        case TOMBOY_DM_VERTICAL_BLANK: TOMBOY_TickVerticalBlank(p_PPU); break;
        case TOMBOY_DM_OBJECT_SCAN: TOMBOY_TickObjectScan(p_PPU); break;
        case TOMBOY_DM_PIXEL_TRANSFER: TOMBOY_TickPixelTransfer(p_PPU); break;
    }

    // If the ODMA transfer is active, then tick the ODMA transfer.
    if (p_ODMA == true)
    {
        TOMBOY_TickODMA(p_PPU);
    }
}

const uint32_t* TOMBOY_GetScreenBuffer (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return NULL;
    }

    // Return the screen buffer.
    return p_PPU->m_ScreenBuffer;
}

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

uint8_t TOMBOY_ReadVRAMByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Check to see if a relative address was provided (`0x0000` to `0x1FFF`). If so, then assume
    // that VRAM is being accessed from the TOMBOY's address bus. Certain checks will need to be
    // performed to ensure that the VRAM bank is accessible.
    if (p_Address < TOMBOY_PPU_VRAM_BANK_SIZE)
    {
        // VRAM is inaccessible if the following are true:
        // - The PPU is enabled (`LCDC` bit 7 is set).
        // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
        // - An OAM DMA transfer is not currently in progress (`m_ODMATicks` >= `0xA0`).
        if (
            p_PPU->m_LCDC.m_DisplayEnable == true &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER &&
            p_PPU->m_ODMATicks >= 0xA0
        )
        {
            // If the VRAM bank is inaccessible, return `0xFF`.
            return 0xFF;
        }
    }

    // Otherwise, if the address provided is an absolute address, then it can be assumed that the
    // VRAM is being accessed internally, in which case the above checks are not performed.
    //
    // Correct the address, however, to be relative to the start of VRAM.
    else if (p_Address >= TOMBOY_VRAM_START && p_Address <= TOMBOY_VRAM_END)
    {
        p_Address -= TOMBOY_VRAM_START;
    }

    // If the address is still out of bounds, return `0xFF`.
    else
    {
        TM_error("VRAM read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    // Return the byte at the specified address in the VRAM bank.
    return p_PPU->m_VRAM[p_Address];
}

uint8_t TOMBOY_ReadOAMByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Check to see if a relative address was provided (`0x0000` to `0x00FF`). If so, then assume
    // that OAM is being accessed from the TOMBOY's address bus. Certain checks will need to be
    // performed to ensure that the OAM buffer is accessible.
    if (p_Address < TOMBOY_PPU_OAM_SIZE)
    {
        // OAM is inaccessible if the following are true:
        // - The PPU is enabled (`LCDC` bit 7 is set).
        // - The PPU is not currently in H-Blank or V-Blank mode.
        // - An OAM DMA transfer is not currently in progress (`m_ODMATicks` >= `0xA0`).
        if (
            p_PPU->m_LCDC.m_DisplayEnable == true &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_VERTICAL_BLANK &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_HORIZONTAL_BLANK &&
            p_PPU->m_ODMATicks >= 0xA0
        )
        {
            // If the OAM buffer is inaccessible, return `0xFF`.
            return 0xFF;
        }
    }

    // Otherwise, if the address provided is an absolute address, then it can be assumed that the
    // OAM buffer is being accessed internally, in which case the above checks are not performed.
    //
    // Correct the address, however, to be relative to the start of OAM.
    else if (p_Address >= TOMBOY_OAM_START && p_Address <= TOMBOY_OAM_END)
    {
        p_Address -= TOMBOY_OAM_START;
    }

    // If the address is still out of bounds, return `0xFF`.
    else
    {
        TM_error("OAM read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    // Return the byte at the specified address in the OAM buffer.
    const uint8_t* l_OAM = (const uint8_t*) p_PPU->m_OAM;
    return l_OAM[p_Address];
}

uint8_t TOMBOY_ReadCRAMByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Check to see if a relative address was provided (`0x0000` to `0x1FFF`). If so, then assume
    // that CRAM is being accessed from the TOMBOY's address bus. Certain checks will need to be
    // performed to ensure that the CRAM bank is accessible.
    if (p_Address < TOMBOY_PPU_CRAM_SIZE)
    {
        // The CRAM buffers are inaccessible if the following are true:
        // - The PPU is enabled (`LCDC` bit 7 is set).
        // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
        if (
            p_PPU->m_LCDC.m_DisplayEnable == true &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
        )
        {
            // If the CRAM banks are inaccessible, return `0xFF`.
            return 0xFF;
        }
    }

    // Otherwise, if the address provided is an absolute address, then it can be assumed that the
    // CRAM bank is being accessed internally, in which case the above checks are not performed.
    //
    // Correct the address, however, to be relative to the start of CRAM.
    else if (p_Address >= TOMBOY_CRAM_START && p_Address <= TOMBOY_CRAM_END)
    {
        p_Address -= TOMBOY_CRAM_START;
    }

    // If the address is still out of bounds, return `0xFF`.
    else
    {
        TM_error("CRAM read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    // Return the byte at the specified address in the CRAM bank.
    return (p_Address < 0x40) ? p_PPU->m_BgCRAM[p_Address] : p_PPU->m_ObjCRAM[p_Address - 0x40];
}

void TOMBOY_WriteVRAMByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Check to see if a relative address was provided (`0x0000` to `0x1FFF`). If so, then assume
    // that VRAM is being accessed from the TOMBOY's address bus. Certain checks will need to be
    // performed to ensure that the VRAM bank is accessible.
    if (p_Address < TOMBOY_PPU_VRAM_BANK_SIZE)
    {
        // VRAM is inaccessible if the following are true:
        // - The PPU is enabled (`LCDC` bit 7 is set).
        // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
        // - An OAM DMA transfer is not currently in progress (`m_ODMATicks` >= `0xA0`).
        if (
            p_PPU->m_LCDC.m_DisplayEnable == true &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER &&
            p_PPU->m_ODMATicks >= 0xA0
        )
        {
            // If the VRAM bank is inaccessible, return.
            return;
        }
    }

    // Otherwise, if the address provided is an absolute address, then it can be assumed that the
    // VRAM bank is being accessed internally, in which case the above checks are not performed.
    //
    // Correct the address, however, to be relative to the start of VRAM.
    else if (p_Address >= TOMBOY_VRAM_START && p_Address <= TOMBOY_VRAM_END)
    {
        p_Address -= TOMBOY_VRAM_START;
    }

    // If the address is still out of bounds, return.
    else
    {
        TM_error("VRAM write address $%08X is out of bounds.", p_Address);
        return;
    }

    // Write the byte at the specified address in the VRAM bank.
    p_PPU->m_VRAM[p_Address] = p_Value;
}

void TOMBOY_WriteOAMByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Check to see if a relative address was provided (`0x0000` to `0x009F`). If so, then assume
    // that OAM is being accessed from the TOMBOY's address bus. Certain checks will need to be
    // performed to ensure that the OAM buffer is accessible.
    if (p_Address < TOMBOY_PPU_OAM_SIZE)
    {
        // OAM is inaccessible if the following are true:
        // - The PPU is enabled (`LCDC` bit 7 is set).
        // - The PPU is not currently in H-Blank or V-Blank mode.
        // - An OAM DMA transfer is not currently in progress (`m_ODMATicks` >= `0xA0`).
        if (
            p_PPU->m_LCDC.m_DisplayEnable == true &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_VERTICAL_BLANK &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_HORIZONTAL_BLANK &&
            p_PPU->m_ODMATicks >= 0xA0
        )
        {
            // If the OAM buffer is inaccessible.
            return;
        }
    }

    // Otherwise, if the address provided is an absolute address, then it can be assumed that the
    // OAM buffer is being accessed internally, in which case the above checks are not performed.
    //
    // Correct the address, however, to be relative to the start of OAM.
    else if (p_Address >= TOMBOY_OAM_START && p_Address <= TOMBOY_OAM_END)
    {
        p_Address -= TOMBOY_OAM_START;
    }

    // If the address is still out of bounds, return.
    else
    {
        TM_error("OAM write address $%08X is out of bounds.", p_Address);
        return;
    }

    // Write the byte at the specified address in the OAM buffer.
    uint8_t* l_OAM = (uint8_t*) p_PPU->m_OAM;
    l_OAM[p_Address] = p_Value;
}

void TOMBOY_WriteCRAMByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Check to see if a relative address was provided (`0x0000` to `0x1FFF`). If so, then assume
    // that CRAM is being accessed from the TOMBOY's address bus. Certain checks will need to be
    // performed to ensure that the CRAM bank is accessible.
    if (p_Address < TOMBOY_PPU_CRAM_SIZE)
    {
        // The CRAM buffers are inaccessible if the following are true:
        // - The PPU is enabled (`LCDC` bit 7 is set).
        // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
        if (
            p_PPU->m_LCDC.m_DisplayEnable == true &&
            p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
        )
        {
            // If the CRAM banks are inaccessible, return.
            return;
        }
    }

    // Otherwise, if the address provided is an absolute address, then it can be assumed that the
    // CRAM bank is being accessed internally, in which case the above checks are not performed.
    //
    // Correct the address, however, to be relative to the start of CRAM.
    else if (p_Address >= TOMBOY_CRAM_START && p_Address <= TOMBOY_CRAM_END)
    {
        p_Address -= TOMBOY_CRAM_START;
    }

    // If the address is still out of bounds, return.
    else
    {
        TM_error("CRAM write address $%08X is out of bounds.", p_Address);
        return;
    }

    // Write the byte at the specified address in the CRAM bank.
    if (p_Address < 0x40)
    {
        p_PPU->m_BgCRAM[p_Address] = p_Value;
    }
    else
    {
        p_PPU->m_ObjCRAM[p_Address - 0x40] = p_Value;
    }
}

uint8_t TOMBOY_ReadScreenByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    if (p_Address >= TOMBOY_PPU_SCREEN_BUFFER_SIZE)
    {
        TM_error("Screen buffer read address $%08X is out of bounds.", p_Address);
        return 0xFF;
    }

    // The screen buffer is inaccessible if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
    )
    {
        // If the screen buffer is inaccessible, return `0xFF`.
        return 0xFF;
    }

    // Return the byte at the specified address in the screen buffer.
    const uint8_t* l_Screen = (const uint8_t*) p_PPU->m_ScreenBuffer;
    return l_Screen[p_Address];
}

void TOMBOY_WriteScreenByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    if (p_Address >= TOMBOY_PPU_SCREEN_BUFFER_SIZE)
    {
        TM_error("Screen buffer write address $%08X is out of bounds.", p_Address);
        return;
    }

    // The screen buffer is inaccessible if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
    )
    {
        // If the screen buffer is inaccessible, return.
        return;
    }

    // Write the byte at the specified address in the screen buffer.
    uint8_t* l_Screen = (uint8_t*) p_PPU->m_ScreenBuffer;
    l_Screen[p_Address] = p_Value;
}

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

uint8_t TOMBOY_ReadLCDC (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the LCDC register.
    return p_PPU->m_LCDC.m_Register;
}

uint8_t TOMBOY_ReadSTAT (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the STAT register.
    return p_PPU->m_STAT.m_Register;
}

uint8_t TOMBOY_ReadSCY (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the SCY register.
    return p_PPU->m_SCY;
}

uint8_t TOMBOY_ReadSCX (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the SCX register.
    return p_PPU->m_SCX;
}

uint8_t TOMBOY_ReadLY (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the LY register.
    return p_PPU->m_LY;
}

uint8_t TOMBOY_ReadDMA (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the DMA register.
    return p_PPU->m_DMA;
}

uint8_t TOMBOY_ReadLYC (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the LYC register.
    return p_PPU->m_LYC;
}

uint8_t TOMBOY_ReadBGP (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the BGP register.
    return p_PPU->m_BGP;
}

uint8_t TOMBOY_ReadOBP0 (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the OBP0 register.
    return p_PPU->m_OBP0;
}

uint8_t TOMBOY_ReadOBP1 (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the OBP1 register.
    return p_PPU->m_OBP1;
}

uint8_t TOMBOY_ReadWY (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the WY register.
    return p_PPU->m_WY;
}

uint8_t TOMBOY_ReadWX (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the WX register.
    return p_PPU->m_WX;
}

uint8_t TOMBOY_ReadVBK (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the VBK register.
    return p_PPU->m_VBK;
}

uint8_t TOMBOY_ReadHDMA7 (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the HDMA7 register.
    return p_PPU->m_HDMA7.m_Register;
}

uint8_t TOMBOY_ReadBGPI (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the BGPI register.
    return p_PPU->m_BGPI.m_Register;
}

uint8_t TOMBOY_ReadOBPI (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the OBPI register.
    return p_PPU->m_OBPI.m_Register;
}

uint8_t TOMBOY_ReadBGPD (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }
    
    // Pallete data cannot be read if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
    )
    {
        // If the palette data is inaccessible, return `0xFF`.
        return 0xFF;
    }

    // Return the byte from BG CRAM at the address specified by the BGPI register.
    return p_PPU->m_BgCRAM[p_PPU->m_BGPI.m_ByteIndex];
}

uint8_t TOMBOY_ReadOBPD (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Pallete data cannot be read if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
    )
    {
        // If the palette data is inaccessible, return `0xFF`.
        return 0xFF;
    }

    // Return the byte from OBJ CRAM at the address specified by the OBPI register.
    return p_PPU->m_ObjCRAM[p_PPU->m_OBPI.m_ByteIndex];
}

uint8_t TOMBOY_ReadOPRI (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the OPRI register.
    return p_PPU->m_OPRI;
}

uint8_t TOMBOY_ReadGRPM (const TOMBOY_PPU* p_PPU)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return 0xFF;
    }

    // Return the GRPM register.
    return p_PPU->m_GRPM;
}

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

void TOMBOY_WriteLCDC (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // If bit 0 of the provided value is clear, then we are attempting to disable the PPU. Do not
    // do this, however, if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently in V-Blank mode.
    if (
        (p_Value & (1 << 7)) == 0 &&
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_VERTICAL_BLANK
    )
    {
        // Set the bit in the provided value to 1 to indicate the PPU should remain enabled.
        p_Value |= (1 << 7);
    }

    // Set the LCDC register.
    p_PPU->m_LCDC.m_Register = p_Value;

    // If the write to LCDC disables the PPU, then reset the inactive divider.
    if (p_PPU->m_LCDC.m_DisplayEnable == false)
    {
        p_PPU->m_InactiveDivider = 0;
    }
}

void TOMBOY_WriteSTAT (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }
    
    // Set the STAT register, but ignore the lower 3 bits as they are read-only.
    p_PPU->m_STAT.m_Register = (p_Value & 0xF8) | (p_PPU->m_STAT.m_Register & 0x07);
}

void TOMBOY_WriteSCY (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the SCY register.
    p_PPU->m_SCY = p_Value;
}

void TOMBOY_WriteSCX (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the SCX register.
    p_PPU->m_SCX = p_Value;
}

// `LY` is read-only, so no setter is provided.

void TOMBOY_WriteLYC (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the LYC register.
    p_PPU->m_LYC = p_Value;
}

void TOMBOY_WriteDMA1 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the DMA1 register.
    p_PPU->m_DMA1 = p_Value;
}

void TOMBOY_WriteDMA2 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the DMA2 register.
    p_PPU->m_DMA2 = p_Value;
}

void TOMBOY_WriteDMA3 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the DMA3 register.
    p_PPU->m_DMA3 = p_Value;
}

void TOMBOY_WriteDMA (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Value is ignored.
    (void) p_Value;
    
    // Writing to this register initiates an OAM DMA transfer.
    p_PPU->m_ODMADelay = 2;
    p_PPU->m_ODMATicks = 0;
    p_PPU->m_ODMASource =
        (((uint32_t) p_PPU->m_DMA1) << 24) |
        (((uint32_t) p_PPU->m_DMA2) << 16) |
        (((uint32_t) p_PPU->m_DMA3) << 8) |
        0x00;
}

void TOMBOY_WriteBGP (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the BGP register.
    p_PPU->m_BGP = p_Value;
}

void TOMBOY_WriteOBP0 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the OBP0 register.
    p_PPU->m_OBP0 = p_Value;
}

void TOMBOY_WriteOBP1 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the OBP1 register.
    p_PPU->m_OBP1 = p_Value;
}

void TOMBOY_WriteWY (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the WY register.
    p_PPU->m_WY = p_Value;
}

void TOMBOY_WriteWX (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the WX register.
    p_PPU->m_WX = p_Value;
}

void TOMBOY_WriteVBK (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the VBK register.
    p_PPU->m_VBK = p_Value;

    // Point the VRAM pointer to the appropriate bank.
    if (p_PPU->m_VBK & 0x01)
    {
        p_PPU->m_VRAM = p_PPU->m_VRAM1;
    }
    else
    {
        p_PPU->m_VRAM = p_PPU->m_VRAM0;
    }
}

void TOMBOY_WriteHDMA1 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the HDMA1 register.
    p_PPU->m_HDMA1 = p_Value;
}

void TOMBOY_WriteHDMA2 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the HDMA2 register.
    p_PPU->m_HDMA2 = p_Value;
}

void TOMBOY_WriteHDMA3 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the HDMA3 register.
    p_PPU->m_HDMA3 = p_Value;
}

void TOMBOY_WriteHDMA4 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the HDMA4 register.
    p_PPU->m_HDMA4 = p_Value;
}

void TOMBOY_WriteHDMA5 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the HDMA5 register.
    p_PPU->m_HDMA5 = p_Value;
}

void TOMBOY_WriteHDMA6 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the HDMA6 register.
    p_PPU->m_HDMA6 = p_Value;
}

void TOMBOY_WriteHDMA7 (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the HDMA7 register.
    p_PPU->m_HDMA7.m_Register = p_Value;

    // Writing to this register initiates an HDMA transfer.
    //
    // Set up the source address, first.
    p_PPU->m_HDMASource =
        (((uint16_t) p_PPU->m_HDMA5) << 8) |
        (((uint16_t) p_PPU->m_HDMA6) & 0xF0);

    // Set up the destination address.
    p_PPU->m_HDMADestination =
        (((uint32_t) p_PPU->m_HDMA1) << 24) |
        (((uint32_t) p_PPU->m_HDMA2) << 16) |
        (((uint32_t) p_PPU->m_HDMA3) << 8) |
        (((uint32_t) p_PPU->m_HDMA4) & 0xF0);

    // Set up the number of blocks to transfer.
    p_PPU->m_HDMABlocksLeft = (p_PPU->m_HDMA7.m_TransferLength + 1);

    // Are we initiating an instanteneous GDMA transfer?
    if (p_PPU->m_HDMA7.m_TransferMode == 0)
    {
        // If so, then perform the entire transfer immediately.
        while (p_PPU->m_HDMABlocksLeft > 0)
        {
            TOMBOY_TickHDMA(p_PPU);
        }
    }
}

void TOMBOY_WriteBGPI (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the BGPI register.
    p_PPU->m_BGPI.m_Register = p_Value;
}

void TOMBOY_WriteOBPI (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Set the OBPI register.
    p_PPU->m_OBPI.m_Register = p_Value;
}

void TOMBOY_WriteBGPD (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }
    
    // Pallete data cannot be written if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == false ||
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
    )
    {
        p_PPU->m_BgCRAM[p_PPU->m_BGPI.m_ByteIndex] = p_Value;
    }

    // Regardless, if the BGPI register's auto-increment bit is set, then increment the index.
    if (p_PPU->m_BGPI.m_AutoIncrement == true)
    {
        p_PPU->m_BGPI.m_ByteIndex = (p_PPU->m_BGPI.m_ByteIndex + 1) & 0x3F;
    }
}

void TOMBOY_WriteOBPD (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // Pallete data cannot be written if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == false ||
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
    )
    {
        p_PPU->m_ObjCRAM[p_PPU->m_OBPI.m_ByteIndex] = p_Value;
    }

    // Regardless, if the OBPI register's auto-increment bit is set, then increment the index.
    if (p_PPU->m_OBPI.m_AutoIncrement == true)
    {
        p_PPU->m_OBPI.m_ByteIndex = (p_PPU->m_OBPI.m_ByteIndex + 1) & 0x3F;
    }
}

void TOMBOY_WriteOPRI (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // The OPRI register cannot be written to if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently scanning for sprites (display mode is 'TOMBOY_DM_OBJECT_SCAN').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_OBJECT_SCAN
    )
    {
        // If the OPRI register is inaccessible, return.
        return;
    }

    // Set the OPRI register.
    p_PPU->m_OPRI = p_Value;
}

void TOMBOY_WriteGRPM (TOMBOY_PPU* p_PPU, uint8_t p_Value)
{
    if (p_PPU == NULL)
    {
        TM_error("PPU instance is NULL.");
        return;
    }

    // The GRPM register cannot be written to if the following are true:
    // - The PPU is enabled (`LCDC` bit 7 is set).
    // - The PPU is not currently drawing to the screen (display mode is 'TOMBOY_DM_PIXEL_TRANSFER').
    if (
        p_PPU->m_LCDC.m_DisplayEnable == true &&
        p_PPU->m_STAT.m_DisplayMode != TOMBOY_DM_PIXEL_TRANSFER
    )
    {
        // If the GRPM register is inaccessible, return.
        return;
    }

    // Set the GRPM register.
    p_PPU->m_GRPM = p_Value;
}
