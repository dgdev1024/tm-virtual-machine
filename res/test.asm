INCLUDE "res/tomboy.inc"

VERSION         $01, $00, $0000
REQUEST_RAM     $10, $00, $00
TITLE           "TOMBOY Test Program"
AUTHOR          "Dennis W. Griffin"
DESCRIPTION     "This is a test program for the TOMBOY system."
 
MACRO IS_EVEN
    IF (\1 % 2) == 0
        RETURN "This Is Even."
    ENDIF

    RETURN "This Is Odd."
ENDM

MACRO OUT_OF
    RETURN \1 * $100 / \2
ENDM

MACRO TEST_LOOP
    DEF LOCAL_COUNT = 0
    REPT \1
        IF LOCAL_COUNT == \2
            RETURN \2
        ENDIF
        DEF LOCAL_COUNT += 1
    ENDR

    RETURN \1
ENDM

MACRO INNER_MACRO
    RETURN (20 * 2) + 2
ENDM

MACRO OUTER_MACRO
    DB INNER_MACRO()
ENDM

MACRO FIBONACCI
    IF (\1 <= 0)
        RETURN 0
    ELIF (\1 == 1)
        RETURN 1
    ENDIF

    DEF _FN = \1
    DEF _FX = 1
    DEF _FY = 0
    DEF _FT = 0

    REPT -1
        DEF _FT = _FX
        DEF _FX = _FX + _FY
        DEF _FY = _FT
        DEF _FN -= 1

        IF _FN <= 0
            RETURN _FY
        ENDC
    ENDR
ENDM

MACRO NESTED_LOOP
    DEF _T1 = 0
    REPT -1
        REPT -1
            DEF _T1 += 1
            IF _T1 == 42
                RETURN $42
            ENDIF
        ENDR
    ENDR
ENDM

ORG RAM, $0000
    CURRENT_COUNT_BYTE2: DB 1
    CURRENT_COUNT_BYTE1: DB 1
    CURRENT_COUNT_BYTE0: DB 1

ORG ROM, $5000
    DB OUT_OF(50, 100)
    DB TEST_LOOP(128, 42)
    OUTER_MACRO
    DL FIBONACCI(40)
    DB NESTED_LOOP()

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
