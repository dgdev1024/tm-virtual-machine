;
; @file         tomboy.inc
; @brief        Contains constants and macros commonly used for developing applications for the TOMBOY.
; @note         Adapted from "hardware.inc", a commonly used include file in Game Boy homebrew development.
;

;; Memory Map Constants ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF _ROM            = $00000000
DEF _MDATA          = $00000000
DEF _RST            = $00001000
DEF _INT            = $00002000
DEF _CODE           = $00003000
DEF _RAM            = $80000000
DEF _DRAM           = $80000000
DEF _WRAM           = $80000000
DEF _SRAM           = $C0000000
DEF _PIXELS         = $DFFD0000
DEF _NSEND          = $DFFF0000
DEF _NRECV          = $DFFF0100
DEF _VRAM           = $DFFF8000
DEF _TDATA          = $DFFF8000
DEF _TDATA0         = $DFFF8000
DEF _TDATA1         = $DFFF8800
DEF _TDATA2         = $DFFF9000
DEF _SCRN           = $DFFF9800
DEF _SCRN0          = $DFFF9800
DEF _SCRN1          = $DFFF9C00
DEF _CRAM           = $DFFFA000
DEF _CRAM0          = $DFFFA000
DEF _CRAM1          = $DFFFA040
DEF _OAM            = $DFFFFE00
DEF _AUD3WAVERAM    = $DFFFFF30
DEF _XRAM           = $E0000000
DEF _DSTACK         = $FFFD0000
DEF _CSTACK         = $FFFE0000
DEF _QRAM           = $FFFF0000
DEF _IO             = $FFFFFF00

;; Memory Mapped Hardware Registers ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF rJOYP           = $00
DEF rP1             = $00
DEF rNTC            = $01
DEF rDIV            = $02
DEF rTIMA           = $03
DEF rTMA            = $04
DEF rTAC            = $05
DEF rRTCS           = $06
DEF rRTCM           = $07
DEF rRTCH           = $08
DEF rRTCDH          = $09
DEF rRTCDL          = $0A
DEF rRTCL           = $0B
DEF rRTCR           = $0C
DEF rIF             = $0F
DEF rNR10           = $10
DEF rNR11           = $11
DEF rNR12           = $12
DEF rNR13           = $13
DEF rNR14           = $14
DEF rNR21           = $15
DEF rNR22           = $16
DEF rNR23           = $17
DEF rNR24           = $18
DEF rNR30           = $19
DEF rNR31           = $1A
DEF rNR32           = $1B
DEF rNR33           = $1C
DEF rNR34           = $1D
DEF rNR41           = $1E
DEF rNR42           = $1F
DEF rNR43           = $20
DEF rNR44           = $21
DEF rNR50           = $22
DEF rNR51           = $23
DEF rNR52           = $24
DEF rLCDC           = $25
DEF rSTAT           = $26
DEF rSCY            = $27
DEF rSCX            = $28
DEF rLY             = $29
DEF rLYC            = $2A
DEF rDMA1           = $2B
DEF rDMA2           = $2C
DEF rDMA3           = $2D
DEF rDMA            = $2E
DEF rBGP            = $2F
DEF rOBP0           = $30
DEF rOBP1           = $31
DEF rWY             = $32
DEF rWX             = $33
DEF rKEY1           = $34
DEF rVBK            = $35
DEF rHDMA1          = $36
DEF rHDMA2          = $37
DEF rHDMA3          = $38
DEF rHDMA4          = $39
DEF rHDMA5          = $3A
DEF rHDMA6          = $3B
DEF rHDMA7          = $3C
DEF rBGPI           = $3D
DEF rBCPS           = $3D
DEF rBGPD           = $3E
DEF rBCPD           = $3E
DEF rOBPI           = $3F
DEF rOCPS           = $3F
DEF rOBPD           = $40
DEF rOCPD           = $40
DEF rOPRI           = $41
DEF rGRPM0          = $42
DEF rVBPR           = $43
DEF rPCM12          = $44
DEF rPCM34          = $45
DEF rIE             = $FF

;; Memory Mapped Hardware Register Aliases ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF rAUD1SWEEP      = rNR10
DEF rAUD1LEN        = rNR11
DEF rAUD1ENV        = rNR12
DEF rAUD1LOW        = rNR13
DEF rAUD1HIGH       = rNR14
DEF rAUD2LEN        = rNR21
DEF rAUD2ENV        = rNR22
DEF rAUD2LOW        = rNR23
DEF rAUD2HIGH       = rNR24
DEF rAUD3ENABLE     = rNR30
DEF rAUD3LEN        = rNR31
DEF rAUD3LEVEL      = rNR32
DEF rAUD3LOW        = rNR33
DEF rAUD3HIGH       = rNR34
DEF rAUD4LEN        = rNR41
DEF rAUD4ENV        = rNR42
DEF rAUD4POLY       = rNR43
DEF rAUD4GO         = rNR44
DEF rAUDVOL         = rNR50
DEF rAUDTERM        = rNR51
DEF rAUDENA         = rNR52

;; Memory Mapped Hardware Register Values ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF P1F_5               = %00100000 ; P15 out port, set to 0 to get buttons
DEF P1F_4               = %00010000 ; P14 out port, set to 0 to get dpad
DEF P1F_3               = %00001000 ; P13 in port
DEF P1F_2               = %00000100 ; P12 in port
DEF P1F_1               = %00000010 ; P11 in port
DEF P1F_0               = %00000001 ; P10 in port
DEF P1F_GET_DPAD        = P1F_5
DEF P1F_GET_BTN         = P1F_4
DEF P1F_GET_NONE        = P1F_4 | P1F_5

DEF NTCF_DIRECTION      = %00000010 ; 1 = send, 0 = receive
DEF NTCF_ENABLE         = %00000001 ; 1 = transfer in progress
DEF NTCF_STATUS         = %00001100 ; 2 bits, 00 = ready, 01 = busy, 10 = timeout, 11 = error
DEF NTCF_STATUS_READY   = %00000000 ; Network is ready for communication
DEF NTCF_STATUS_BUSY    = %00000001 ; Network is sending or receiving data
DEF NTCF_STATUS_TIMEOUT = %00000010 ; Network operation timed out
DEF NTCF_STATUS_ERROR   = %00000011 ; Network encountered an error

DEF TACF_START          = %00000100
DEF TACF_STOP           = %00000000
DEF TACF_4KHZ           = %00000000
DEF TACF_16KHZ          = %00000011
DEF TACF_65KHZ          = %00000010
DEF TACF_262KHZ         = %00000001

DEF TACB_START          = 2

DEF AUD1SWEEP_UP        = %00000000
DEF AUD1SWEEP_DOWN      = %00001000

DEF AUD3ENA_OFF         = %00000000
DEF AUD3ENA_ON          = %10000000

DEF AUD3LEVEL_MUTE      = %00000000
DEF AUD3LEVEL_100       = %00100000
DEF AUD3LEVEL_50        = %01000000
DEF AUD3LEVEL_25        = %01100000

DEF AUD4POLY_15STEP     = %00000000
DEF AUD4POLY_7STEP      = %00001000

DEF AUDVOL_VIN_LEFT     = %10000000 ; SO2
DEF AUDVOL_VIN_RIGHT    = %00001000 ; SO1

; SO2
DEF AUDTERM_4_LEFT      = %10000000
DEF AUDTERM_3_LEFT      = %01000000
DEF AUDTERM_2_LEFT      = %00100000
DEF AUDTERM_1_LEFT      = %00010000
; SO1
DEF AUDTERM_4_RIGHT     = %00001000
DEF AUDTERM_3_RIGHT     = %00000100
DEF AUDTERM_2_RIGHT     = %00000010
DEF AUDTERM_1_RIGHT     = %00000001

DEF AUDENA_ON           = %10000000
DEF AUDENA_OFF          = %00000000  ; sets all audio regs to 0!

DEF LCDCF_OFF           = %00000000 ; LCD Control Operation
DEF LCDCF_ON            = %10000000 ; LCD Control Operation
DEF LCDCF_WIN9800       = %00000000 ; Window Tile Map Display Select
DEF LCDCF_WIN9C00       = %01000000 ; Window Tile Map Display Select
DEF LCDCF_WINOFF        = %00000000 ; Window Display
DEF LCDCF_WINON         = %00100000 ; Window Display
DEF LCDCF_BLK21         = %00000000 ; BG & Window Tile Data Select
DEF LCDCF_BLK01         = %00010000 ; BG & Window Tile Data Select
DEF LCDCF_BG9800        = %00000000 ; BG Tile Map Display Select
DEF LCDCF_BG9C00        = %00001000 ; BG Tile Map Display Select
DEF LCDCF_OBJ8          = %00000000 ; OBJ Construction
DEF LCDCF_OBJ16         = %00000100 ; OBJ Construction
DEF LCDCF_OBJOFF        = %00000000 ; OBJ Display
DEF LCDCF_OBJON         = %00000010 ; OBJ Display
DEF LCDCF_BGOFF         = %00000000 ; BG Display
DEF LCDCF_BGON          = %00000001 ; BG Display

DEF LCDCB_ON            = 7 ; LCD Control Operation
DEF LCDCB_WIN9C00       = 6 ; Window Tile Map Display Select
DEF LCDCB_WINON         = 5 ; Window Display
DEF LCDCB_BLKS          = 4 ; BG & Window Tile Data Select
DEF LCDCB_BG9C00        = 3 ; BG Tile Map Display Select
DEF LCDCB_OBJ16         = 2 ; OBJ Construction
DEF LCDCB_OBJON         = 1 ; OBJ Display
DEF LCDCB_BGON          = 0 ; BG Display

DEF STATF_LYC           =  %01000000 ; LYC=LY Coincidence (Selectable)
DEF STATF_MODE10        =  %00100000 ; Mode 10
DEF STATF_MODE01        =  %00010000 ; Mode 01 (V-Blank)
DEF STATF_MODE00        =  %00001000 ; Mode 00 (H-Blank)
DEF STATF_LYCF          =  %00000100 ; Coincidence Flag
DEF STATF_HBL           =  %00000000 ; H-Blank
DEF STATF_VBL           =  %00000001 ; V-Blank
DEF STATF_OAM           =  %00000010 ; OAM-RAM is used by system
DEF STATF_LCD           =  %00000011 ; Both OAM and VRAM used by system
DEF STATF_BUSY          =  %00000010 ; When set, VRAM access is unsafe

DEF STATB_LYC           =  6
DEF STATB_MODE10        =  5
DEF STATB_MODE01        =  4
DEF STATB_MODE00        =  3
DEF STATB_LYCF          =  2
DEF STATB_BUSY          =  1

DEF WX_OFS      = 7 ; add this to a screen position to get a WX position

DEF KEY1F_DBLSPEED      = %00000001 ; Double Speed Mode (0 = off, Nonzero = on)

DEF HDMA7F_MODE_GP      = %00000000 ; General Purpose DMA (W)
DEF HDMA7F_MODE_HBL     = %10000000 ; HBlank DMA (W)
DEF HDMA7F_BUSY         = %10000000 ; HDMA Transfer in Progress (R) (0 = busy, 1 = complete)
DEF HDMA7B_MODE         = 7 ; DMA mode select (W)

DEF BCPSF_AUTOINC       = %10000000 ; Auto Increment (0=Disabled, 1=Increment after Writing)
DEF BCPSB_AUTOINC       = 7
DEF BGPIF_AUTOINC       = BCPSF_AUTOINC
DEF BGPIB_AUTOINC       = BCPSB_AUTOINC

DEF OCPSF_AUTOINC       = %10000000 ; Auto Increment (0=Disabled, 1=Increment after Writing)
DEF OCPSB_AUTOINC       = 7
DEF OBPIF_AUTOINC       = OCPSF_AUTOINC
DEF OBPIB_AUTOINC       = OCPSB_AUTOINC

DEF OPRI_OAM            = 0 ; Prioritize objects by location in OAM (CGB Mode default)
DEF OPRI_COORD          = 1 ; Prioritize objects by x-coordinate (Non-CGB Mode default)

DEF GRPM_DMG            = 0
DEF GRPM_CGB            = 1

DEF VBP_PAUSE           = 1
DEF VBP_RESUME          = 0

DEF IEF_HILO            = %00010000 ; Transition from High to Low of Pin number P10-P13
DEF IEF_SERIAL          = %00001000 ; Serial I/O transfer end
DEF IEF_TIMER           = %00000100 ; Timer Overflow
DEF IEF_STAT            = %00000010 ; STAT
DEF IEF_VBLANK          = %00000001 ; V-Blank

DEF IEB_HILO            = 4
DEF IEB_SERIAL          = 3
DEF IEB_TIMER           = 2
DEF IEB_STAT            = 1
DEF IEB_VBLANK          = 0

;; Common Sound Channel Flags ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; --
; -- Square wave duty cycle
; --
; -- Can be used with AUD1LEN and AUD2LEN
; -- See AUD1LEN for more info
; --
DEF AUDLEN_DUTY_12_5    = %00000000 ; 12.5%
DEF AUDLEN_DUTY_25      = %01000000 ; 25%
DEF AUDLEN_DUTY_50      = %10000000 ; 50%
DEF AUDLEN_DUTY_75      = %11000000 ; 75%


; --
; -- Audio envelope flags
; --
; -- Can be used with AUD1ENV, AUD2ENV, AUD4ENV
; -- See AUD1ENV for more info
; --
DEF AUDENV_UP           = %00001000
DEF AUDENV_DOWN         = %00000000


; --
; -- Audio trigger flags
; --
; -- Can be used with AUD1HIGH, AUD2HIGH, AUD3HIGH
; -- See AUD1HIGH for more info
; --
DEF AUDHIGH_RESTART     = %10000000
DEF AUDHIGH_LENGTH_ON   = %01000000
DEF AUDHIGH_LENGTH_OFF  = %00000000

;; Interrupt Handler Addresses ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF INT_HANDLER_VBLANK  = $2000
DEF INT_HANDLER_LCDSTAT = $2100
DEF INT_HANDLER_TIMER   = $2200
DEF INT_HANDLER_SERIAL  = $2300
DEF INT_HANDLER_NETWORK = $2300
DEF INT_HANDLER_JOYPAD  = $2400
DEF INT_HANDLER_RTC     = $2500

;; Keypad Constants ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF PADF_DOWN   = $80
DEF PADF_UP     = $40
DEF PADF_LEFT   = $20
DEF PADF_RIGHT  = $10
DEF PADF_START  = $08
DEF PADF_SELECT = $04
DEF PADF_B      = $02
DEF PADF_A      = $01

DEF PADB_DOWN   = $7
DEF PADB_UP     = $6
DEF PADB_LEFT   = $5
DEF PADB_RIGHT  = $4
DEF PADB_START  = $3
DEF PADB_SELECT = $2
DEF PADB_B      = $1
DEF PADB_A      = $0

;; Screen Constants ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF SCRN_X    = 160 ; Width of screen in pixels
DEF SCRN_Y    = 144 ; Height of screen in pixels. Also corresponds to the value in LY at the beginning of VBlank.
DEF SCRN_X_B  = 20  ; Width of screen in bytes
DEF SCRN_Y_B  = 18  ; Height of screen in bytes

DEF SCRN_VX   = 256 ; Virtual width of screen in pixels
DEF SCRN_VY   = 256 ; Virtual height of screen in pixels
DEF SCRN_VX_B = 32  ; Virtual width of screen in bytes
DEF SCRN_VY_B = 32  ; Virtual height of screen in bytes

;; OAM Constants ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DEF ODMA_Y          = 0 ; Y coordinate of sprite
DEF ODMA_X          = 1 ; X coordinate of sprite
DEF ODMA_TILEID     = 2 ; Tile ID of sprite
DEF ODMA_FLAGS      = 3 ; Flags of sprite

DEF OAM_Y_OFS       = 16 ; add this to a screen-relative Y position to get an OAM Y position
DEF OAM_X_OFS       = 8  ; add this to a screen-relative X position to get an OAM X position

DEF OAM_COUNT       = 40  ; number of OAM entries in OAM RAM

; flags
DEF OAMF_PRI        = %10000000 ; Priority
DEF OAMF_YFLIP      = %01000000 ; Y flip
DEF OAMF_XFLIP      = %00100000 ; X flip
DEF OAMF_PAL0       = %00000000 ; Palette number; 0,1 (DMG)
DEF OAMF_PAL1       = %00010000 ; Palette number; 0,1 (DMG)
DEF OAMF_BANK0      = %00000000 ; Bank number; 0,1 (GBC)
DEF OAMF_BANK1      = %00001000 ; Bank number; 0,1 (GBC)

DEF OAMF_PALMASK    = %00000111 ; Palette (GBC)

DEF OAMB_PRI        = 7 ; Priority
DEF OAMB_YFLIP      = 6 ; Y flip
DEF OAMB_XFLIP      = 5 ; X flip
DEF OAMB_PAL1       = 4 ; Palette number; 0,1 (DMG)
DEF OAMB_BANK1      = 3 ; Bank number; 0,1 (GBC)

;; Helper Macros ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MACRO VERSION
    ORG ROM, $0000
        DB $54, $4D, $42, $59               ; "TMBY"
        DB \1, \2                           ; Major and minor version
        DW \3                               ; Patch version
ENDM

MACRO REQUEST_RAM
    ORG ROM, $0008
        DW \1, \2, \3                       ; Requested WRAM, SRAM and XRAM size, respectively
ENDM

MACRO TITLE
    ORG ROM, $0020
        DB \1, 0                            ; Program title
ENDM

MACRO AUTHOR
    ORG ROM, $0040
        DB \1, 0                            ; Program author
ENDM

MACRO DESCRIPTION
    ORG ROM, $0060
        DB \1, 0                            ; Program description
ENDM
