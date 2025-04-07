INCLUDE "res/tomboy.inc"

VERSION         $01, $00, $0000
REQUEST_RAM     $10, $00, $00
TITLE           "TOMBOY Test Program"
AUTHOR          "Dennis W. Griffin"
DESCRIPTION     "This is a test program for the TOMBOY system."

ORG RAM, $0000
    CURRENT_COUNT_BYTE2: DB 1
    CURRENT_COUNT_BYTE1: DB 1
    CURRENT_COUNT_BYTE0: DB 1

ORG ROM, $3000
    MAIN:
        CALL NC, COUNTER
        STOP

    COUNTER:
        LD A, CURRENT_COUNT_BYTE0
        INC [A]
        JPB ZC, COUNTER
        LD A, CURRENT_COUNT_BYTE1
        INC [A]
        JPB ZC, COUNTER
        LD A, CURRENT_COUNT_BYTE2
        INC [A]
        RET ZS
        JPB NC, COUNTER
