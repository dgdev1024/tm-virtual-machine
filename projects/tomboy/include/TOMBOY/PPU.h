/**
 * @file  TOMBOY/PPU.h
 * @brief Contains the declaration of the TOMBOY pixel processing unit (PPU) structure and its
 *        associated functions.
 * 
 * The TOMBOY Engine's PPU component seeks to simulate the Game Boy's internal pixel-processing unit
 * (PPU) hardware. The PPU is responsible for rendering graphics consisting of background and window
 * tilemaps and free-roaming objects to a 160x144 pixel screen buffer. The PPU also manages buffers
 * for storing tile, tilemap and tile attribute data, a buffer for storing object attribute data and
 * buffers for storing color palette data.
 * 
 * The PPU is responsible for the following memory buffers:
 * 
 * - The Screen Buffer: A 160x144 pixel buffer for storing the rendered graphics output. Each pixel
 *   is represented by a 32-bit unsigned integer, in RGBA format (`0xRRGGBBAA`).
 * - The Video Memory (`VRAM`): A 16 KB buffer for storing tile data, tilemap data and tile
 *   attribute data. The buffer is split into two 8 KB banks, which can be swapped in and out of
 *   memory by writing to the `VBK` hardware register. The buffer is accessible from addresses
 *   `0x8000` to `0x9FFF` in the engine's memory map. Each VRAM bank is partitioned as follows:
 *       - Relative address `0x0000` to `0x17FF`: Tile data buffer - contains data for 384 8x8 pixel
 *         tiles, each of which is represented by 16 bytes of data. The means by which the PPU
 *         addresses this buffer for fetching tiles for the background and window layers is
 *         determined by bit 4 of the display control (`LCDC`) register.
 *       - In VRAM bank 0, relative address `0x1800` to `0x1FFF`: Tilemap data buffer - contains
 *         data for two 32x32-byte tilemaps, used to index tiles in the tile data buffer to be
 *         rendered to the background and window layers. The region of the buffer used for selecting
 *         tiles for the background and window layers are determined by bits 3 and 6 of the `LCDC`
 *         register, respectively.
 *       - In VRAM bank 1, relative address `0x1800` to `0x1FFF`: Tile attribute data buffer -
 *         contains data for two 32x32-byte tile attribute maps, representing the attributes of the
 *         respective tile indices in the tilemap data buffer. This bank is not accessible if the
 *         `GRPM` register is set to 0, indicating that the PPU is in DMG graphics mode.
 * - The Object Attribute Memory (`OAM`): A 160-byte buffer for storing object attribute data. Each
 *   object attribute occupies 4 bytes of data, and the buffer is accessible from addresses `0xFE00`
 *   to `0xFE9F` in the engine's memory map. The PPU uses this buffer to determine which objects are
 *   visible on the current scanline. The buffer is divided into 40 4-byte object attribute entries,
 *   with each byte therein representing the following:
 *       - Byte 0: The object's Y-coordinate on the screen.
 *       - Byte 1: The object's X-coordinate on the screen, minus 8 pixels.
 *       - Byte 2: The object's tile index in the tile data buffer.
 *       - Byte 3: The object's attributes. More on tile attributes later.
 * - The Color RAM (`CRAM`) buffers: Two 64-byte buffers for storing color palette data, one for the
 *   background layer and one for the object layer. The buffers each store eight distinct color
 *   palettes, with each palette containing four colors. Each color is represented two bytes of data,
 *   arranged in bitwise little-endian format as follows: `0bRRRRRGGG`, `0bGGBBBBB0`. The buffers are
 *   accessible by using the palette specification registers `BGPI` and `OBPI` to select an index
 *   within the buffer, and the palette data registers `BGPD` and `OBPD` to access the color data
 *   at that index in the CRAM buffer. These buffers are not accessible if the `GRPM` register is set
 *   to 0, indicating that the PPU is in DMG graphics mode.
 * 
 * The PPU component comes equipped with the following hardware registers:
 * 
 * - `LCDC` (Display Control): The display control register is used to control the PPU's display
 *   settings. The bits of this register control the following settings:
 *       - Bit 0 - BG/Window Display or Master Priority: This setting has different effects depending
 *         on the PPU's graphics mode (set in the `GRPM` register):
 *             - In DMG graphics mode (GRPM = 0), this bit controls whether the background and window
 *               layers are displayed at all.
 *             - In CGB graphics mode (GRPM = 1), this bit controls whether the background and window
 *               layers may have priority over the object layer.
 *       - Bit 1 - Object Display: Set to display objects on the screen; clear otherwise.
 *       - Bit 2 - Object Size: Set to use 8x16 pixel objects; clear to use 8x8 pixel objects.
 *       - Bit 3 - Background Tilemap Address: If set, the PPU will select tiles from the second
 *         tilemap in VRAM (relative address `0x1C00` to `0x1FFF`) for rendering the background layer;
 *         otherwise, it will select tiles from the first tilemap in VRAM (relative address `0x1800`
 *         to `0x1BFF`).
 *       - Bit 4 - Background/Window Tile Data Address: If set, the PPU map tile indices 0-127 to
 *         tiles in block 0 of the tile data area in the current VRAM bank (relative address `0x0000`
 *         to `0x07FF`); otherwise, it maps tile indices 0-127 to tiles in block 2 of the tile data
 *         area in the current VRAM bank (relative address `0x1000` to `0x17FF`). Indices 128-255
 *         are always mapped to block 1 (relative address `0x0800` to `0x0FFF`). The object layer
 *         always uses block 0 and 1.
 *       - Bit 5 - Window Display: Set to display the window layer; clear otherwise.
 *       - Bit 6 - Window Tilemap Address: If set, the PPU will select tiles from the second tilemap
 *         in VRAM (relative address `0x1C00` to `0x1FFF`) for rendering the window layer; otherwise,
 *         it will select tiles from the first tilemap in VRAM (relative address `0x1800` to `0x1BFF`).
 *       - Bit 7 - Display Enable: Set to enable the PPU; clear to disable the PPU.
 *             - If the PPU is disabled, the screen buffer is cleared to white.
 *             - If the PPU is enabled, it cannot be disabled outside of VBLANK mode.
 * - `STAT` (Display Status): The display status register is used to indicate the PPU's current
 *   display mode, and to control the request of the `LCD_STAT` interrupt. The bits of this register
 *   control the following settings:
 *       - Bits 0-1 - Display Mode: Indicates the PPU's current display mode. More on display modes
 *         later. Read-only.
 *       - Bit 2 - Line Coincidence: Set if the current scanline (`LY`) matches the value in the
 *         line compare (`LYC`) register; clear otherwise. Read-only.
 *       - Bit 3 - Mode 0 Interrupt Request: Set to request the `LCD_STAT` interrupt when the PPU
 *         enters Mode 0 (`HBLANK`); clear otherwise.
 *       - Bit 4 - Mode 1 Interrupt Request: Set to request the `LCD_STAT` interrupt when the PPU
 *         enters Mode 1 (`VBLANK`); clear otherwise.
 *       - Bit 5 - Mode 2 Interrupt Request: Set to request the `LCD_STAT` interrupt when the PPU
 *         enters Mode 2 (`OAM_SCAN`); clear otherwise.
 *       - Bit 6 - Line Coincidence Interrupt Request: Set to request the `LCD_STAT` interrupt when
 *         the line coincidence bit is set; clear otherwise.
 * - `SCY` (Scroll Y): The scroll Y register is used to set the background scroll position on the
 *   screen. The value in this register is used to determine the Y-coordinate of the top-left corner
 *   of the visible portion of the background layer.
 * - `SCX` (Scroll X): The scroll X register is used to set the background scroll position on the
 *   screen. The value in this register is used to determine the X-coordinate of the top-left corner
 *   of the visible portion of the background layer.
 * - `LY` (LCDC Y-Coordinate, Current Scanline): The LCDC Y-coordinate register is used to store the
 *   current scanline being rendered by the PPU. The value in this register is incremented every time
 *   the PPU finishes rendering a scanline, and is reset to 0 when the PPU exits VBLANK mode. Read-only.
 * - `LYC` (LY Compare, Line Compare): The LY compare register is used to set the value against which
 *   the PPU compares the current scanline (`LY`) to determine if the line coincidence bit in the
 *   `STAT` register should be set.
 * - `DMA1`, `DMA2` and `DMA3` (DMA Start Address, Bytes 3, 2 and 1): The high three bytes of the
 *   32-bit source address for the OAM DMA transfer. The value in these registers is used to determine
 *   the source address from which data is copied to the OAM buffer.
 * - `DMA` (OAM DMA Initiate): The OAM DMA register is written to in order to initiate an OAM DMA
 *   transfer. Writing a byte to this register triggers the transfer, copying data from an address
 *   in which the bytes set in `DMA1`, `DMA2` and `DMA3` are the high three bytes, and the low byte
 *   starts at `0x00` and ends at `0x9F`.
 * - `BGP` (Background Palette Data): The background palette data register is used to set the color
 *   palette for the background layer. This register is ignored if the `GRPM` register is set to 1,
 *   indicating that the PPU is in CGB graphics mode. The bits of this register control the following
 *   settings:
 *       - Bits 0-1 - Color 0: The color of palette index 0.
 *       - Bits 2-3 - Color 1: The color of palette index 1.
 *       - Bits 4-5 - Color 2: The color of palette index 2.
 *       - Bits 6-7 - Color 3: The color of palette index 3.
 * - `OBP0` and `OBP1` (Object Palette Data): The object palette data registers are used to set the
 *   color palettes for the object layer. These registers are ignored if the `GRPM` register is set to
 *   1, indicating that the PPU is in CGB graphics mode. The bits of these registers control the
 *   following settings:
 *       - Bits 0-1 - Color 0: The color of palette index 0.
 *       - Bits 2-3 - Color 1: The color of palette index 1.
 *       - Bits 4-5 - Color 2: The color of palette index 2.
 *       - Bits 6-7 - Color 3: The color of palette index 3.
 * - `WY` (Window Position Y): The window position Y register is used to set the Y-coordinate of the
 *   top-left corner of the visible portion of the window layer.
 * - `WX` (Window Position X): The window position X register is used to set the X-coordinate of the
 *   top-left corner of the visible portion of the window layer. The value in this register is offset
 *   by 7 pixels to the right when rendering the window layer.
 * - `VBK` (VRAM Bank Number): The VRAM bank number register is used to select the current VRAM bank
 *   to access. The value in this register is used to determine which VRAM bank is accessible from
 *   addresses `0x8000` to `0x9FFF`. The value in this register is masked with 0x01 to determine the
 *   bank number.
 * - `HDMA1`, `HDMA2`, `HDMA3` and `HDMA4` (GDMA / HDMA Source Address, Bytes 3, 2, 1 and 0): The
 *   four bytes making up the 32-bit source address for a GDMA / HDMA transfer. The value in these
 *   registers is used to determine the source address from which data is copied to the VRAM buffer.
 *   Write-only.
 * - `HDMA5`, `HDMA6` (GDMA / HDMA Destination Address, Bytes 1 and 0): The two bytes making up
 *   the 16-bit destination address for a GDMA / HDMA transfer, relative to the start of the VRAM
 *   space. The value in these registers is used to determine the destination address to which data is
 *   copied in the VRAM buffer. Write-only.
 * - `HDMA7` (GDMA / HDMA Transfer, Mode, Start): The GDMA / HDMA transfer register is used to start
 *   a GDMA / HDMA transfer. More on direct-memory access processes later. The bits of this register 
 *   control the following settings:
 *       - Bits 0-6 - Transfer Length: The number of 16-byte blocks to transfer from the source to
 *         the destination. The value in this register is incremented by 1 before each transfer.
 *       - Bit 7 - Transfer Mode: Set to enable HDMA mode; clear to enable GDMA mode.
 *       - Writing to this register triggers the transfer.
 * - `BGPI` and `OBPI` (Background and Object Palette Specification): The background and object
 *   palette specification registers are used to set the index of the color in the background and
 *   object color RAM buffers to access, as well as whether or not the index is auto-incremented
 *   with each attempted access. These registers are ignored if the `GRPM` register is set to 0,
 *   indicating that the PPU is in DMG graphics mode. The bits of these registers control the
 *   following settings:
 *      - Bits 0-5 - Palette Index: The index of the color in the color RAM buffer to access.
 *      - Bit 7 - Auto-Increment: Set to increment the palette index with each attempted access;
 *        clear otherwise.
 * - `BGPD` and `OBPD` (Background and Object Palette Data): The background and object palette data
 *   registers are used to access a byte in the background and object color RAM buffers. The index of
 *   the byte accessed is determined by the index setting in the `BGPI` and `OBPI` registers, respectively.
 *   The value in these registers is masked with 0x3F to determine the index of the byte to access.
 *   These registers are ignored if the `GRPM` register is set to 0, indicating that the PPU is in DMG
 *   graphics mode.
 * - `OPRI` (Object Priority): The object priority register is used to set the priority of the object
 *   layer. If zero, then an object's priority is determined by its X position (the smaller the X
 *   position, the higher the priority); otherwise, its priority is determined by its index in the
 *   OAM buffer (the lower the index, the higher the priority). If the `GRPM` register is set to 0,
 *   indicating that the PPU is in DMG graphics mode, this register is ignored, with the former
 *   priority setting being used.
 * - `GRPM` (PPU Graphics Mode): The PPU graphics mode register is used to set the PPU's graphics mode.
 *   Setting this register to 0 indicates that the PPU is in DMG graphics mode, while setting it to
 *   any non-zero value indicates that the PPU is in CGB graphics mode.
 * 
 * The PPU component is responsible for the following hardware interrupts:
 * 
 * - `VBLANK` (Vertical Blank Interrupt): The PPU requests this interrupt when its display mode
 *   changes to Mode 1 (`VBLANK`). This interrupt is typically used to update the game's engine state
 *   and game logic.
 * - `LCD_STAT` (LCD Status Interrupt): The PPU requests this interrupt when certain conditions are
 *   met, as determined by the `STAT` register. This interrupt is typically used to implement
 *   real-time graphics effects and special rendering techniques.
 * 
 * The process of the PPU rendering graphics to the screen buffer is carried out over a number of
 * engine cycles (a rate of 4 MHz), a unit of time which will be referred to in the PPU as "dots".
 * The process of the PPU rendering graphics to the screen buffer is carried out over four distinct
 * display modes, each of which is responsible for a different aspect of the rendering process. The
 * modes are listed as follows, in order of execution:
 * 
 * - Mode 2: Object Scan (`OAM_SCAN`): The PPU scans the Object Attribute Memory (OAM) buffer to
 *   determine which objects are visible on the current scanline. This mode lasts for 80 dots, with
 *   each object taking 2 dots to process. Up to ten objects may be visible on a single scanline.
 * - Mode 3: Pixel Transfer (`PIXEL_TRANSFER`): The PPU transfers pixel data from the tile data
 *   buffers to the screen buffer. This mode lasts anywhere from 172 to 289 dots, depending on the
 *   number of visible objects on the current scanline. The actual duration is determined by the
 *   following factors:
 *       - The PPU outputs 1 pixel to the screen buffer every dot. Since a visible scanline is 160
 *         pixels wide, this accounts for 160 dots of the mode's minimum duration.
 *       - At the start of the mode, the PPU performs two tile fetches, one of which is for the
 *         first tile on the scanline, and the other of which is discarded. These fetches account
 *         for the other 12 dots of the mode's minimum duration.
 *       - At the start of the mode, a number of dots equal to `SCX` % 8 are spent idling, accounting 
 *         for the number of pixels on the leftmost tile which are not visible. This accounts adds 
 *         up to 7 dots of the mode's duration.
 *       - After the last non-window pixel is output before encountering the visible portion of the
 *         window layer, the PPU spends 6 additional dots in this mode preparing the pixel fetcher 
 *         to render the visible portion of the window layer.
 *       - An additional 6 to 11 dots are spent for each object drawn on the scanline - even partial
 *         objects. This accounts for the remaining dots of the mode's duration. The process of
 *         determining the number of dots spent on each object is as follows:
 *             - For this process, only the leftmost pixel of the object is accounted for; we'll
 *               call this "The Pixel".
 *             - Determine if The Pixel is within a background layer tile or a window layer tile.
 *               This is affected by the background scroll and window position registers.
 *             - If The Pixel is not already occupied by another object:
 *                - Incur a dot-penalty equal to the number of pixels strictly to the right of The
 *                  Pixel in its tile, minus 2. Do not incur a dot-penalty if the result is negative.
 *             - Spend the minimum of 6 dots on fetching the object's tile data.
 * - Mode 0: Horizontal Blank (`HBLANK`): At this point, the PPU has finished rendering the current
 *   scanline. The PPU enters this mode to idle for the remaining dots of the scanline. This mode
 *   lasts for the remaining dots of the scanline after the end of Mode 3. The total duration of
 *   rendering a scanline is 456 dots. Once this mode is complete:
 *       - If the scanline just rendered was the last visible scanline of the frame, the PPU enters
 *         the Vertical Blank mode.
 *       - Otherwise, the PPU advances to the next scanline and enters Mode 2.
 * - Mode 1: Vertical Blank (`VBLANK`): The PPU has finished rendering the last visible scanline of
 *   the frame. The PPU enters this mode to idle for the remaining dots of the frame (ten scanlines'
 *   worth of dots, 4,560 dots). During this mode, the entirety of VRAM, OAM and CRAM are accessible
 *   to the engine. Once this mode is complete, the PPU resets to the first scanline of the frame and
 *   enters Mode 2. The entire process of rendering a frame takes 456 dots per scanline * 144 visible
 *   scanlines, plus 4,560 dots of VBLANK, for a total of 70,224 dots.
 * 
 * The PPU component comes equipped with a pixel-fetcher unit, which is responsible for fetching
 * pixel data from the tile data buffers and preparing them for rendering to the screen buffer. The
 * pixel-fetcher unit places these pixels into one of two first-in-first-out (FIFO) buffers: one for
 * the background/window layer and one for the object (sprite) layer. The pixel-fetcher unit operates
 * in the following steps:
 * 
 * 1. **Get Tile Number**:
 *    - The PPU fetches the tile number from the background or window tile map. The tile number is used
 *      to identify which tile's pixel data to fetch next. The tile map is a 32x32 grid of tile numbers,
 *      each representing an 8x8 pixel tile.
 * 
 * 2. **Get Tile Data Low**:
 *    - The PPU fetches the low byte of the tile data for the current tile and line. Each tile is 8x8
 *      pixels, and each line of the tile is represented by two bytes (low and high). The low byte contains
 *      the least significant bits (LSBs) of the color indices for the pixels in the current line.
 * 
 * 3. **Get Tile Data High**:
 *    - The PPU fetches the high byte of the tile data for the current tile and line. The high byte contains
 *      the most significant bits (MSBs) of the color indices for the pixels in the current line.
 * 
 * 4. **Push Pixels**:
 *    - The PPU combines the low and high bytes of the tile data to form the final color indices for the pixels.
 *      These indices are then pushed into the pixel FIFO (First-In-First-Out) buffer. The pixel FIFO holds the
 *      pixel data temporarily before it is rendered to the screen. There are two FIFO buffers: one for the
 *      background/window layer and one for the object (sprite) layer.
 * 
 * 5. **Sleep**:
 *    - The PPU enters a sleep state for a few cycles to allow the pixel FIFO to process the fetched pixels.
 *      This step ensures that the pixel FIFO has enough time to push the pixels to the screen buffer, preventing
 *      any delays or glitches in the rendering process.
 * 
 * The pixel-fetcher unit operates in synchronization with the PPU's rendering cycles, ensuring that the pixel
 * data is fetched and processed in time for rendering. Each step in the pixel fetching process takes a specific
 * number of cycles to complete, resulting in smooth and accurate graphics on the Game Boy's screen:
 *    - Get Tile Number: 2 cycles
 *    - Get Tile Data Low: 2 cycles
 *    - Get Tile Data High: 2 cycles
 *    - Push Pixels: 2 cycles
 *    - Sleep: 2 cycles
 * 
 * The PPU component also manages a number of direct-memory access (DMA) processes, which are used to transfer
 * data from one memory location to another without involving the parent engine. The PPU's DMA processes 
 * are as follows:
 * 
 * - OAM DMA Transfer: The OAM DMA transfer is used to copy data from the engine's memory to the Object
 *   Attribute Memory (OAM) buffer. This process is prepared by writing the high two bytes of the source
 *   address to the `DMA1` through `DMA3` registers, then writing the second low byte of the source address
 *   to the `DMA1` register. Writing to the `DMA1` register triggers the transfer. The OAM DMA process
 *   copies data from the source address to the OAM buffer in a non-blocking manner, with the transfer
 *   taking 160 bytes of data from the source address and copying it to the OAM buffer. The OAM DMA
 *   process is used to quickly transfer object attribute data to the OAM buffer, such as sprite data
 *   or object attribute data, without having to wait for a `VBLANK` period.
 * 
 * - General-Purpose DMA (GDMA): The GDMA process is used to instantly copy data from a memory location
 *   into the Video RAM (VRAM) buffer. This process is initiated by first writing the source address to the
 *   `HDMA1` and `HDMA2` registers, the destination address to the `HDMA3` and `HDMA4` registers, the
 *   transfer length to bits 0-6 of the `HDMA5` register, and 0 to the transfer mode bit (bit 7) of the
 *   `HDMA5` register. Writing to the `HDMA5` register triggers the transfer. The GDMA process copies data
 *   from the source address to the destination address all at once, in a blocking manner. The GDMA process
 *   is used to quickly transfer large amounts of data to the VRAM buffer, such as tile data or tilemap data
 *   without having to wait for a `VBLANK` period.
 * 
 * - Horizontal Blank DMA (HDMA): The HDMA process is used to copy data from a memory location into the VRAM
 *   buffer during the horizontal blank period (`HBLANK`). As with the GDMA process, the HDMA process is
 *   prepared by writing the source and destination addresses to `HDMA1` through `HDMA6`, the transfer
 *   length to bits 0-6 of the `HDMA7` register, and 1 to the transfer mode bit (bit 7) of the `HDMA7` register.
 *   Writing to the `HDMA7` register triggers the transfer. The HDMA process copies data from the source
 *   address to the destination address in blocks of 16 bytes, with the transfer length determining the number
 *   of blocks to transfer (with 0 = 1 block, up to 0x7F blocks for a maximum of 0x7F * 16 = 2,048 bytes).
 *   The HDMA process transfers one block of data at the start of each `HBLANK` period, allowing for a
 *   non-blocking transfer of data during the frame rendering process.
 */

#pragma once
#include <TOMBOY/Common.h>

// Constants ///////////////////////////////////////////////////////////////////////////////////////

/** @brief The width, height and size of the PPU's screen buffer, each in pixels. */
#define TOMBOY_PPU_SCREEN_WIDTH 160
#define TOMBOY_PPU_SCREEN_HEIGHT 144
#define TOMBOY_PPU_SCREEN_PIXEL_SIZE (TOMBOY_PPU_SCREEN_WIDTH * TOMBOY_PPU_SCREEN_HEIGHT)
#define TOMBOY_PPU_SCREEN_BUFFER_SIZE (TOMBOY_PPU_SCREEN_PIXEL_SIZE * sizeof(uint32_t))

/** @brief The size of a bank of video RAM, and its partitions, each in bytes. */
#define TOMBOY_PPU_VRAM_BANK_SIZE 0x2000
#define TOMBOY_PPU_TDATA_PARTITION_SIZE 0x1800
#define TOMBOY_PPU_TMAP_PARTITION_SIZE 0x800
#define TOMBOY_PPU_TDATA_BLOCK_SIZE 0x800
#define TOMBOY_PPU_TMAP_BLOCK_SIZE 0x400
#define TOMBOY_PPU_TILES_PER_BLOCK 384

/** @brief The size of the OAM buffer, in object entries and bytes. */
#define TOMBOY_PPU_OBJECT_COUNT 40
#define TOMBOY_PPU_OAM_SIZE (TOMBOY_PPU_OBJECT_COUNT * 4)

/** @brief The size of the CRAM buffer, and the color palettes therein, in bytes. */
#define TOMBOY_PPU_PALETTE_COUNT 8
#define TOMBOY_PPU_PALETTE_COLOR_COUNT 4
#define TOMBOY_PPU_PALETTE_COLOR_SIZE 2
#define TOMBOY_PPU_PALETTE_SIZE (TOMBOY_PPU_PALETTE_COLOR_COUNT * TOMBOY_PPU_PALETTE_COLOR_SIZE)
#define TOMBOY_PPU_CRAM_SIZE (TOMBOY_PPU_PALETTE_COUNT * TOMBOY_PPU_PALETTE_SIZE)

/** @brief The size of the PPU's pixel FIFO buffers. */
#define TOMBOY_PPU_FIFO_SIZE 32

/** @brief The number of scanlines, visible and not, in a frame, and the number of dots therein. */
#define TOMBOY_PPU_VISIBLE_SCANLINE_COUNT 144
#define TOMBOY_PPU_SCANLINE_COUNT 154
#define TOMBOY_PPU_DOTS_PER_SCANLINE 456
#define TOMBOY_PPU_DOTS_PER_FRAME (TOMBOY_PPU_SCANLINE_COUNT * TOMBOY_PPU_DOTS_PER_SCANLINE)

// Typedefs and Forward Declarations ///////////////////////////////////////////////////////////////

/**
 * @brief A forward-declaration of the TOMBOY emulator engine structure.
 */
typedef struct TOMBOY_Engine TOMBOY_Engine;

/**
 * @brief A forward-declaration of the TOMBOY pixel processing unit (PPU) structure.
 */
typedef struct TOMBOY_PPU TOMBOY_PPU;

/**
 * @brief A pointer to a function which is called when the PPU finishes rendering a frame.
 * 
 * This function is called by the PPU when it finishes rendering a frame, and will be called even
 * when the VBLANK interrupt and/or the PPU is disabled. The function is passed a pointer to the
 * PPU structure.
 * 
 * @param p_PPU A pointer to the PPU structure.
 */
typedef void (*TOMBOY_FrameRenderedCallback) (TOMBOY_PPU*);

// Display Mode Enumeration ////////////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the PPU's display modes.
 */
typedef enum TOMBOY_DisplayMode
{
    TOMBOY_DM_HORIZONTAL_BLANK = 0,  ///< @brief The PPU is in horizontal blank mode (Mode 0, `HBLANK`).
    TOMBOY_DM_VERTICAL_BLANK = 1,    ///< @brief The PPU is in vertical blank mode (Mode 1, `VBLANK`).
    TOMBOY_DM_OBJECT_SCAN = 2,       ///< @brief The PPU is scanning for objects on the current scanline (Mode 2, `OAM_SCAN`).
    TOMBOY_DM_PIXEL_TRANSFER = 3     ///< @brief The PPU is transferring pixel data to the screen buffer (Mode 3, `PIXEL_TRANSFER`).
} TOMBOY_DisplayMode;

// `LCD_STAT` Source Enumeration ///////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the sources from which the `LCD_STAT` interrupt may be requested.
 */
typedef enum TOMBOY_DisplayStatusSource
{
    TOMBOY_DSS_HORIZONTAL_BLANK = 1,  ///< @brief The `LCD_STAT` interrupt is requested when the PPU enters Mode 0 (`HBLANK`).
    TOMBOY_DSS_VERTICAL_BLANK,        ///< @brief The `LCD_STAT` interrupt is requested when the PPU enters Mode 1 (`VBLANK`).
    TOMBOY_DSS_OBJECT_SCAN,           ///< @brief The `LCD_STAT` interrupt is requested when the PPU enters Mode 2 (`OAM_SCAN`).
    TOMBOY_DSS_LINE_COINCIDENCE       ///< @brief The `LCD_STAT` interrupt is requested when the line coincidence bit changes from low to high.
} TOMBOY_DisplayStatusSource;

// Pixel Fetcher Mode Enumeration //////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the modes of the PPU's internal pixel-fetcher unit.
 */
typedef enum TOMBOY_PixelFetchMode
{
    TOMBOY_PFM_TILE_NUMBER,           ///< @brief The pixel-fetcher is fetching the tile number for the current pixel.
    TOMBOY_PFM_TILE_DATA_LOW,         ///< @brief The pixel-fetcher is fetching the low byte of the tile data for the current pixel.
    TOMBOY_PFM_TILE_DATA_HIGH,        ///< @brief The pixel-fetcher is fetching the high byte of the tile data for the current pixel.
    TOMBOY_PFM_PUSH_PIXELS,           ///< @brief The pixel-fetcher is pushing the pixel data to the pixel FIFO buffer.
    TOMBOY_PFM_SLEEP                  ///< @brief The pixel-fetcher is idling to allow the pixel FIFO to process the fetched pixels.
} TOMBOY_PixelFetchMode;

// Object Priority Mode Enumeration ////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the priority modes for objects in the object attribute memory (OAM).
 */
typedef enum TOMBOY_ObjectPriorityMode
{
    TOMBOY_OPM_OAM_INDEX = 0,            ///< @brief The PPU uses the object's index in the OAM buffer to determine its priority. Objects with a lower index in OAM have higher priority over other objects.
    TOMBOY_OPM_X_POSITION                ///< @brief The PPU uses the object's X position to determine its priority; objects with a smaller X position have higher priority over other objects, with ties broken by the object's index in OAM.
} TOMBOY_ObjectPriorityMode;

// Graphics Mode Enumeration ///////////////////////////////////////////////////////////////////////

/**
 * @brief An enumeration representing the PPU's graphics modes.
 */
typedef enum TOMBOY_GraphicsMode
{
    TOMBOY_GM_DMG = 0,  ///< @brief The PPU is in DMG graphics mode.
    TOMBOY_GM_CGB       ///< @brief The PPU is in CGB graphics mode.
} TOMBOY_GraphicsMode;

// Display Control Union ///////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `LCDC` display control register.
 */
typedef union TOMBOY_DisplayControl
{
    struct
    {
        /**
         * @brief This bit has different effects depending on the PPU's graphics mode (set in the `GRPM`
         * register):
         * 
         * - In DMG graphics mode (GRPM = 0), this bit controls whether the background and window layers
         *   are displayed at all.
         * 
         * - In CGB graphics mode (GRPM = 1), this bit controls whether the background and window layers
         *   may have priority over the object layer.
         */
        uint8_t m_BGWEnableOrPriority : 1;
        uint8_t m_ObjectEnable : 1;       ///< @brief Set to display objects on the screen; clear otherwise.
        uint8_t m_ObjectSize : 1;         ///< @brief Set to use 8x16 pixel objects ("tall objects"); clear to use 8x8 pixel objects.

        /**
         * @brief This bit controls which tilemap in VRAM the PPU selects tiles from for rendering
         *        the background layer:
         * 
         * - If set, the PPU will select tiles from the second tilemap in VRAM (relative address
         *   `0x1C00` to `0x1FFF`).
         * 
         * - Otherwise, it will select tiles from the first tilemap in VRAM (relative address `0x1800`
         *   to `0x1BFF`).
         */
        uint8_t m_BGTilemapAddress : 1;

        /**
         * @brief This bit controls which tile data area ("block") in VRAM the PPU maps tile indices
         *        0-127 to for rendering the background and window layers:
         * 
         * - If set, the PPU maps tile indices 0-127 to tiles in block 0 of the tile data area in the
         *   current VRAM bank (relative address `0x0000` to `0x07FF`).
         * 
         * - Otherwise, it maps tile indices 0-127 to tiles in block 2 of the tile data area in the
         *   current VRAM bank (relative address `0x1000` to `0x17FF`).
         * 
         * - Indices 128-255 are always mapped to block 1 (relative address `0x0800` to `0x0FFF`).
         * 
         * - The object layer always uses block 0 and 1.
         */
        uint8_t m_BGWindowTileDataAddress : 1;

        uint8_t m_WindowEnable : 1;       ///< @brief Set to display the window layer; clear otherwise.

        /**
         * @brief This bit controls which tilemap in VRAM the PPU selects tiles from for rendering
         *        the window layer:
         * 
         * - If set, the PPU will select tiles from the second tilemap in VRAM (relative address
         *   `0x1C00` to `0x1FFF`).
         * 
         * - Otherwise, it will select tiles from the first tilemap in VRAM (relative address `0x1800`
         *   to `0x1BFF`).
         */
        uint8_t m_WindowTilemapAddress : 1;

        /**
         * @brief This bit controls whether the PPU is enabled:
         * 
         * - If set, the PPU is enabled and may render graphics to the screen buffer.
         * 
         * - If clear, the PPU is disabled and the screen buffer is cleared to white.
         * 
         * @warning If the PPU is currently enabled, it cannot be disabled outside of VBLANK mode!
         */
        uint8_t m_DisplayEnable : 1;
    };

    uint8_t m_Register;  ///< @brief The raw register value.
} TOMBOY_DisplayControl;

// Display Status Union ////////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `STAT` display status register.
 */
typedef union TOMBOY_DisplayStatus
{
    struct
    {
        uint8_t m_DisplayMode : 2;  ///< @brief The PPU's current display mode. Read-only.

        /**
         * @brief This bit indicates whether the current scanline (`LY`) register equals the value
         *        in the line compare (`LYC`) register. Read-only.
         */
        uint8_t m_LineCoincidence : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the PPU enters Mode 0 (`HBLANK`);
         *        clear otherwise.
         */
        uint8_t m_HorizontalBlankStatSource : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the PPU enters Mode 1 (`VBLANK`);
         *        clear otherwise.
         */
        uint8_t m_VerticalBlankStatSource : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the PPU enters Mode 2 (`OAM_SCAN`);
         *        clear otherwise.
         */
        uint8_t m_ObjectScanStatSource : 1;

        /**
         * @brief Set to request the `LCD_STAT` interrupt when the line coincidence bit changes from
         *        low to high; clear otherwise.
         */
        uint8_t m_LineCoincidenceStatSource : 1;

    };

    uint8_t m_Register;  ///< @brief The raw register value.
} TOMBOY_DisplayStatus;

// GDMA / HDMA Control Union ///////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `HDMA7` GDMA / HDMA transfer control register.
 */
typedef union TOMBOY_HDMAControl
{
    struct
    {
        uint8_t m_TransferLength : 7;  ///< @brief The number of 16-byte blocks to transfer from the source to the destination.
        
        /**
         * @brief This bit determines how the transfer is carried out:
         * 
         * - Set to initiate a Horizontal Blank DMA (HDMA) transfer. In this mode, the PPU transfers
         *   one 16-byte block of data at the start of each `HBLANK` period.
         * 
         * - Clear to initiate a General DMA (GDMA) transfer. In this mode, the PPU transfers all
         *   data from the source to the destination at once, in a blocking manner.
         */
        uint8_t m_TransferMode : 1;
    };

    uint8_t m_Register;  ///< @brief The raw register value.
} TOMBOY_HDMAControl;

// Palette Specification Union /////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the PPU's `BGPI` and `OBPI` background and object palette specification
 * registers.
 */
typedef union TOMBOY_PaletteSpecification
{
    struct
    {
        uint8_t m_ByteIndex : 6;  ///< @brief The index of the color in the color RAM buffer to access.
        uint8_t : 1;              ///< @brief Unused bit.

        /**
         * @brief This bit controls whether the index of the color in the color RAM buffer is
         *        auto-incremented with each attempted access.
         * 
         * @note If this bit is set, the index is incremented even if the attempted access fails.
         */
        uint8_t m_AutoIncrement : 1;
    };

    uint8_t m_Register;  ///< @brief The raw register value.
} TOMBOY_PaletteSpecification;

// Tile Attributes Union ///////////////////////////////////////////////////////////////////////////

/**
 * @brief A union representing the attributes of a tile in the tile attribute map, and the 
 *        attributes of an object in the object attribute memory (OAM).
 */
typedef union TOMBOY_TileAttributes
{
    struct
    {
        uint8_t m_PaletteIndex : 3;   ///< @brief The index of the color palette to use (CGB only).
        uint8_t m_VRAMBank : 1;       ///< @brief The VRAM bank to use for the tile data (CGB only).
        uint8_t m_DMGPalette : 1;     ///< @brief The DMG palette to use for the tile data (DMG only).
        uint8_t m_HorizontalFlip : 1; ///< @brief Whether to flip the tile horizontally.
        uint8_t m_VerticalFlip : 1;   ///< @brief Whether to flip the tile vertically.
        uint8_t m_Priority : 1;       ///< @brief Does the background/window have priority over the object or any object on this tile?
    };

    uint8_t m_Value;  ///< @brief The raw attribute value.
} TOMBOY_TileAttributes;

// Object Attribute Memory (OAM) Entry Structure ///////////////////////////////////////////////////

/**
 * @brief A structure representing an entry in the object attribute memory (OAM) buffer.
 */
typedef struct TOMBOY_Object
{
    uint8_t m_Y;                          ///< @brief The Y-coordinate of the object on the screen.
    uint8_t m_X;                          ///< @brief The X-coordinate of the object on the screen.
    uint8_t m_TileIndex;                  ///< @brief The index of the tile in the tile data buffer.
    TOMBOY_TileAttributes m_Attributes;  ///< @brief The attributes of the object.
} TOMBOY_Object;

// CRAM Color Structure ////////////////////////////////////////////////////////////////////////////

/**
 * @brief A structure representing a two-byte RGB555 color in the color RAM (CRAM) buffer.
 * 
 * @note The RGB555 color format uses 5 bits for each of the red, green, and blue color channels,
 *       laid out as follows: `0bRRRRRGGG` `0bGGBBBBB0`.
 */
typedef struct TOMBOY_ColorRGB555
{
    uint16_t m_Red   : 5;  ///< @brief The red color channel.
    uint16_t m_Green : 5;  ///< @brief The green color channel.
    uint16_t m_Blue  : 5;  ///< @brief The blue color channel.
    uint16_t : 1;          ///< @brief The unused bit at the end of the color sequence.
} TOMBOY_ColorRGB555;

// PPU Pixel Fetcher Structure /////////////////////////////////////////////////////////////////////

/**
 * @brief A structure representing the PPU's internal pixel-fetcher unit.
 */
typedef struct TOMBOY_PixelFetcher
{
    
    TOMBOY_PixelFetchMode m_Mode;  ///< @brief The current mode of the pixel-fetcher unit.
    
    // Pixel FIFO
    struct
    {
        uint32_t m_Buffer[TOMBOY_PPU_FIFO_SIZE];      ///< @brief The pixel FIFO buffer.
        uint8_t  m_Head;                              ///< @brief The index of the next available slot in the buffer.
        uint8_t  m_Tail;                              ///< @brief The index of the next slot to read from in the buffer.
        uint8_t  m_Size;                              ///< @brief The number of pixels currently in the buffer.
    } m_PixelFIFO;

    // Fetched Tile Data - Background/Window Layer
    struct
    {
        uint8_t                   m_TileIndex;        ///< @brief The number of the fetched tile in the appropriate tilemap.
        TOMBOY_TileAttributes     m_TileAttributes;   ///< @brief The attributes of the fetched tile.
        uint8_t                   m_TileDataLow;      ///< @brief The low byte of the fetched tile data.
        uint8_t                   m_TileDataHigh;     ///< @brief The high byte of the fetched tile data.
    } m_FetchedBGW;

    // Fetched Tile Data - Object Layer
    struct
    {
        uint8_t                   m_ObjectIndices[3]; ///< @brief The indices of the fetched objects' tiles in the tile data buffer.
        uint8_t                   m_TileDataLow[3];   ///< @brief The low bytes of the fetched objects' tile data.
        uint8_t                   m_TileDataHigh[3];  ///< @brief The high bytes of the fetched objects' tile data.
        uint8_t                   m_ObjectCount;      ///< @brief The number of objects fetched. Maximum of 3.
    } m_FetchedOBJ;

    // Tile Fetcher State
    uint8_t m_LineX;                                  ///< @brief The fetcher's current X-coordinate on the current scanline.
    uint8_t m_PushedX;                                ///< @brief The X-coordinate of the last pixel pushed to the screen buffer.  
    uint8_t m_FetchingX;                              ///< @brief The X-coordinate of the pixel currently being fetched.
    uint8_t m_MapY, m_MapX;                           ///< @brief The fetcher's current Y and X coordinates in the 256x256-pixel tilemap.               
    uint8_t m_TileDataOffset;                         ///< @brief The starting offset address of the tile data being fetched.
    uint8_t m_QueueX;                                 ///< @brief The X-coordinate of the last pixel pushed into the pixel FIFO?

} TOMBOY_PixelFetcher;

// Public Functions ////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a new instance of the TOMBOY pixel processing unit (PPU) component.
 * 
 * @param p_Engine  A pointer to the PPU's parent engine structure.
 * 
 * @return A pointer to the newly created PPU structure.
 */
TOMBOY_PPU* TOMBOY_CreatePPU (TOMBOY_Engine* p_Engine);

/**
 * @brief Destroys the specified instance of the TOMBOY pixel processing unit (PPU) component.
 * 
 * @param p_PPU  A pointer to the PPU structure to destroy.
 */
void TOMBOY_DestroyPPU (TOMBOY_PPU* p_PPU);

/**
 * @brief Resets the specified instance of the TOMBOY pixel processing unit (PPU) component,
 *        resetting (or initializing) all of its registers and buffers to their default values.
 * 
 * @param p_PPU  A pointer to the PPU structure to reset.
 */
void TOMBOY_ResetPPU (TOMBOY_PPU* p_PPU);

/**
 * @brief Sets a callback function to be called when the PPU finishes rendering a frame.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Callback    A pointer to the callback function to set.
 */
void TOMBOY_SetFrameRenderedCallback (TOMBOY_PPU* p_PPU, TOMBOY_FrameRenderedCallback p_Callback);

/**
 * @brief Ticks the specified instance of the TOMBOY pixel processing unit (PPU) component, updating
 *        its internal state and continuing the rendering of the current frame.
 * 
 * @param p_PPU  A pointer to the PPU structure to tick.
 * @param p_ODMA Also tick the OAM DMA process if it is currently active?
 */
void TOMBOY_TickPPU (TOMBOY_PPU* p_PPU, bool p_ODMA);

/**
 * @brief Gets the given PPU instance's screen buffer, which contains the pixels which have been
 *        rendered to the screen.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return A pointer to the PPU's screen buffer.
 */
const uint32_t* TOMBOY_GetScreenBuffer (const TOMBOY_PPU* p_PPU);

// Public Functions - Memory Access ////////////////////////////////////////////////////////////////

/**
 * @brief Reads a byte from the given PPU's currently-set VRAM bank.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to read from.
 * 
 * @return The byte read from the specified address.
 */
uint8_t TOMBOY_ReadVRAMByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address);

/**
 * @brief Writes a byte to the given PPU's currently-set VRAM bank.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to write to.
 * @param p_Value       The value to write to the specified address.
 */
void TOMBOY_WriteVRAMByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief Reads a byte from the given PPU's object attribute memory (OAM) buffer.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to read from.
 * 
 * @return The byte read from the specified address.
 */
uint8_t TOMBOY_ReadOAMByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address);

/**
 * @brief Writes a byte to the given PPU's object attribute memory (OAM) buffer.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to write to.
 * @param p_Value       The value to write to the specified address.
 */
void TOMBOY_WriteOAMByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief Reads a byte from one of the given PPU's color RAM (CRAM) buffers.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to read from.
 * 
 * @return The byte read from the specified address.
 */
uint8_t TOMBOY_ReadCRAMByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address);

/**
 * @brief Writes a byte to one of the given PPU's color RAM (CRAM) buffers.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to write to.
 * @param p_Value       The value to write to the specified address.
 */
void TOMBOY_WriteCRAMByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value);

/**
 * @brief Reads a byte from the given PPU's screen buffer.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to read from.
 * 
 * @return The byte read from the specified address.
 */
uint8_t TOMBOY_ReadScreenByte (const TOMBOY_PPU* p_PPU, uint32_t p_Address);

/**
 * @brief Writes a byte to the given PPU's screen buffer.
 * 
 * @param p_PPU         A pointer to the PPU structure.
 * @param p_Address     The relative address to write to.
 * @param p_Value       The value to write to the specified address.
 */
void TOMBOY_WriteScreenByte (TOMBOY_PPU* p_PPU, uint32_t p_Address, uint8_t p_Value);

// Public Functions - Hardware Register Getters ////////////////////////////////////////////////////

/**
 * @brief Gets the value of the given PPU's display control register, `LCDC`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `LCDC` register.
 */
uint8_t TOMBOY_ReadLCDC (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's display status register, `STAT`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `STAT` register.
 */
uint8_t TOMBOY_ReadSTAT (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's background scroll Y register, `SCY`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `SCY` register.
 */
uint8_t TOMBOY_ReadSCY (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's background scroll X register, `SCX`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `SCX` register.
 */
uint8_t TOMBOY_ReadSCX (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's line compare register, `LYC`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `LYC` register.
 */
uint8_t TOMBOY_ReadLYC (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's current scanline register, `LY`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `LY` register.
 */
uint8_t TOMBOY_ReadLY (const TOMBOY_PPU* p_PPU);

// `DMA3` through `DMA1` are write-only.

/**
 * @brief Gets the value of the given PPU's OAM DMA Initiate register, `DMA`.
 * 
 * When an OAM DMA transfer is in progress, this register is set to the low byte of the last
 * address which was transferred. This register will be set to a value less than `0xA0` if the
 * transfer is in progress, and to `0xA0` or greater if there is no transfer in progress.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `DMA` register.
 */
uint8_t TOMBOY_ReadDMA (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's DMG background palette register, `BGP`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `BGP` register.
 */
uint8_t TOMBOY_ReadBGP (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's DMG first object palette register, `OBP0`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `OBP0` register.
 */
uint8_t TOMBOY_ReadOBP0 (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's DMG second object palette register, `OBP1`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `OBP1` register.
 */
uint8_t TOMBOY_ReadOBP1 (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's window Y coordinate register, `WY`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `WY` register.
 */
uint8_t TOMBOY_ReadWY (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's window X coordinate register, `WX`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `WX` register.
 */
uint8_t TOMBOY_ReadWX (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's video RAM bank number register, `VBK`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `VBK` register.
 */
uint8_t TOMBOY_ReadVBK (const TOMBOY_PPU* p_PPU);

// `HDMA1` through `HDMA6` are write-only.

/**
 * @brief Gets the value of the given PPU's HDMA transfer length and control register, `HDMA7`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `HDMA7` register.
 */
uint8_t TOMBOY_ReadHDMA7 (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's background color palette index register, `BGPI`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `BGPI` register.
 */
uint8_t TOMBOY_ReadBGPI (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's background color palette data register, `BGPD`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `BGPD` register.
 */
uint8_t TOMBOY_ReadBGPD (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's object color palette index register, `OBPI`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `OBPI` register.
 */
uint8_t TOMBOY_ReadOBPI (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's object color palette data register, `OBPD`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `OBPD` register.
 */
uint8_t TOMBOY_ReadOBPD (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's object priority mode register, `OPRI`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `OPRI` register.
 */
uint8_t TOMBOY_ReadOPRI (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's graphics mode register, `GRPM`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `GRPM` register.
 */
uint8_t TOMBOY_ReadGRPM (const TOMBOY_PPU* p_PPU);

/**
 * @brief Gets the value of the given PPU's vertical blanking pause register, `VBP`.
 * 
 * @param p_PPU  A pointer to the PPU structure.
 * 
 * @return The value of the `VBP` register.
 */
uint8_t TOMBOY_ReadVBP (const TOMBOY_PPU* p_PPU);

// Public Functions - Hardware Register Setters ////////////////////////////////////////////////////

/**
 * @brief Sets the value of the given PPU's display control register, `LCDC`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `LCDC` register to.
 */
void TOMBOY_WriteLCDC (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's display status register, `STAT`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `STAT` register to.
 */
void TOMBOY_WriteSTAT (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's scroll Y register, `SCY`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `SCY` register to.
 */
void TOMBOY_WriteSCY (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's scroll X register, `SCX`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `SCX` register to.
 */
void TOMBOY_WriteSCX (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's line compare register, `LYC`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `LYC` register to.
 */
void TOMBOY_WriteLYC (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's OAM DMA Byte 3 register, `DMA1`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `DMA1` register to.
 */
void TOMBOY_WriteDMA1 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's OAM DMA Byte 2 register, `DMA2`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `DMA2` register to.
 */
void TOMBOY_WriteDMA2 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's OAM DMA Byte 1 register, `DMA3`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `DMA3` register to.
 */
void TOMBOY_WriteDMA3 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's OAM DMA Initiate register, `DMA`.
 * 
 * Writing to this register will initiate an OAM DMA transfer from the address comprised of the
 * `DMA3`, `DMA2`, and `DMA1` registers to the OAM buffer. The transfer will be initiated
 * immediately, and the `DMA` register will be set to the low byte of the last address which
 * starts at `0x00`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `DMA` register to.
 */
void TOMBOY_WriteDMA (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's DMG background palette register, `BGP`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `BGP` register to.
 */
void TOMBOY_WriteBGP (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's DMG first object palette register, `OBP0`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `OBP0` register to.
 */
void TOMBOY_WriteOBP0 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's DMG second object palette register, `OBP1`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `OBP1` register to.
 */
void TOMBOY_WriteOBP1 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's window Y coordinate register, `WY`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `WY` register to.
 */
void TOMBOY_WriteWY (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's window X coordinate register, `WX`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `WX` register to.
 */
void TOMBOY_WriteWX (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's video RAM bank number register, `VBK`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `VBK` register to.
 */
void TOMBOY_WriteVBK (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's HDMA source address byte 3 register, `HDMA1`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `HDMA1` register to.
 */
void TOMBOY_WriteHDMA1 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's HDMA source address byte 2 register, `HDMA2`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `HDMA2` register to.
 */
void TOMBOY_WriteHDMA2 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's HDMA source address byte 1 register, `HDMA3`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `HDMA3` register to.
 */
void TOMBOY_WriteHDMA3 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's HDMA source address byte 0 register, `HDMA4`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `HDMA4` register to.
 */
void TOMBOY_WriteHDMA4 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's HDMA destination address byte 1 register, `HDMA5`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `HDMA5` register to.
 */
void TOMBOY_WriteHDMA5 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's HDMA destination address byte 0 register, `HDMA6`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `HDMA6` register to.
 */
void TOMBOY_WriteHDMA6 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's HDMA transfer length and control register, `HDMA7`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `HDMA7` register to.
 */
void TOMBOY_WriteHDMA7 (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's background color palette index register, `BGPI`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `BGPI` register to.
 */
void TOMBOY_WriteBGPI (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's background color palette data register, `BGPD`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `BGPD` register to.
 */
void TOMBOY_WriteBGPD (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's object color palette index register, `OBPI`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `OBPI` register to.
 */
void TOMBOY_WriteOBPI (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's object color palette data register, `OBPD`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `OBPD` register to.
 */
void TOMBOY_WriteOBPD (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's object priority mode register, `OPRI`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `OPRI` register to.
 */
void TOMBOY_WriteOPRI (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's graphics mode register, `GRPM`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `GRPM` register to.
 */
void TOMBOY_WriteGRPM (TOMBOY_PPU* p_PPU, uint8_t p_Value);

/**
 * @brief Sets the value of the given PPU's vertical blanking pause register, `VBP`.
 * 
 * @param p_PPU      A pointer to the PPU structure.
 * @param p_Value    The value to set the `VBP` register to.
 */
void TOMBOY_WriteVBP (TOMBOY_PPU* p_PPU, uint8_t p_Value);
