#include <xtensa/coreasm.h>
//#include "exception_handler.h"
// ***************************************************************************
// * Global symbol
// ***************************************************************************
    .global Exception_Handler
    .global Double_Exception_Handler

// ***************************************************************************
// * External symbol
// ***************************************************************************
    .extern DSP_Fault_handler
    .extern pxExceptionStack
    .extern pExceptionContext

// ***************************************************************************
// * Text Offset
// ***************************************************************************
    EXCEPTION_FRAME_SIZE                = 300
    /* General core registers */
    EXCEPTION_OFFSET_A0                 = 0
    EXCEPTION_OFFSET_A1                 = 4
    EXCEPTION_OFFSET_A2                 = 8
    EXCEPTION_OFFSET_A3                 = 12
    EXCEPTION_OFFSET_A4                 = 16
    EXCEPTION_OFFSET_A5                 = 20
    EXCEPTION_OFFSET_A6                 = 24
    EXCEPTION_OFFSET_A7                 = 28
    EXCEPTION_OFFSET_A8                 = 32
    EXCEPTION_OFFSET_A9                 = 36
    EXCEPTION_OFFSET_A10                = 40
    EXCEPTION_OFFSET_A11                = 44
    EXCEPTION_OFFSET_A12                = 48
    EXCEPTION_OFFSET_A13                = 52
    EXCEPTION_OFFSET_A14                = 56
    EXCEPTION_OFFSET_A15                = 60
    EXCEPTION_OFFSET_A16                = 64
    EXCEPTION_OFFSET_A17                = 68
    EXCEPTION_OFFSET_A18                = 72
    EXCEPTION_OFFSET_A19                = 76
    EXCEPTION_OFFSET_A20                = 80
    EXCEPTION_OFFSET_A21                = 84
    EXCEPTION_OFFSET_A22                = 88
    EXCEPTION_OFFSET_A23                = 92
    EXCEPTION_OFFSET_A24                = 96
    EXCEPTION_OFFSET_A25                = 100
    EXCEPTION_OFFSET_A26                = 104
    EXCEPTION_OFFSET_A27                = 108
    EXCEPTION_OFFSET_A28                = 112
    EXCEPTION_OFFSET_A29                = 116
    EXCEPTION_OFFSET_A30                = 120
    EXCEPTION_OFFSET_A31                = 124
    EXCEPTION_OFFSET_A32                = 128
    EXCEPTION_OFFSET_A33                = 132
    EXCEPTION_OFFSET_A34                = 136
    EXCEPTION_OFFSET_A35                = 140
    EXCEPTION_OFFSET_A36                = 144
    EXCEPTION_OFFSET_A37                = 148
    EXCEPTION_OFFSET_A38                = 152
    EXCEPTION_OFFSET_A39                = 156
    EXCEPTION_OFFSET_A40                = 160
    EXCEPTION_OFFSET_A41                = 164
    EXCEPTION_OFFSET_A42                = 168
    EXCEPTION_OFFSET_A43                = 172
    EXCEPTION_OFFSET_A44                = 176
    EXCEPTION_OFFSET_A45                = 180
    EXCEPTION_OFFSET_A46                = 184
    EXCEPTION_OFFSET_A47                = 188
    EXCEPTION_OFFSET_A48                = 192
    EXCEPTION_OFFSET_A49                = 196
    EXCEPTION_OFFSET_A50                = 200
    EXCEPTION_OFFSET_A51                = 204
    EXCEPTION_OFFSET_A52                = 208
    EXCEPTION_OFFSET_A53                = 212
    EXCEPTION_OFFSET_A54                = 216
    EXCEPTION_OFFSET_A55                = 220
    EXCEPTION_OFFSET_A56                = 224
    EXCEPTION_OFFSET_A57                = 228
    EXCEPTION_OFFSET_A58                = 232
    EXCEPTION_OFFSET_A59                = 236
    EXCEPTION_OFFSET_A60                = 240
    EXCEPTION_OFFSET_A61                = 244
    EXCEPTION_OFFSET_A62                = 248
    EXCEPTION_OFFSET_A63                = 252
    /* Window option special registers */
    EXCEPTION_OFFSET_WINDOWBASE         = 256
    EXCEPTION_OFFSET_WINDOWSTART        = 260
    /* Shift amount special registers */
    EXCEPTION_OFFSET_SAR                = 264
    /* Exception and Interrupt option special registers */
    EXCEPTION_OFFSET_PC                 = 268
    EXCEPTION_OFFSET_EXCCAUSE           = 272
    EXCEPTION_OFFSET_EXCVADDR           = 276
    EXCEPTION_OFFSET_EXCSAVE1           = 280
    EXCEPTION_OFFSET_EPC1               = 284
    EXCEPTION_OFFSET_DEPC               = 288
    EXCEPTION_OFFSET_PS                 = 292
    /* Loop option special registers */
    EXCEPTION_OFFSET_LBEG               = 296
    EXCEPTION_OFFSET_LEND               = 300
    EXCEPTION_OFFSET_LCOUNT             = 304
// ***************************************************************************
// * Exception_Handler
// * <null>
// *
// * exception handler
// ***************************************************************************

    .align 4
    .literal_position
Exception_Handler:
        /* save context to exceptionContext */
        rsr     a0 , EXCSAVE_1                                         // resume a0
        wsr     a2 , EXCSAVE_1                                         // store  a2
        movi    a2 , pExceptionContext                                 // change a2 to the address of pExceptionContext
        l32i    a2 , a2 , 0                                            // change a0 to the address of exceptionContext

        /* save general core registers */
        s32i    a0 , a2 , EXCEPTION_OFFSET_A0
        s32i    a1 , a2 , EXCEPTION_OFFSET_A1
        rsr     a0 , EXCSAVE_1                                         // resume a2 to a0
        s32i    a0 , a2 , EXCEPTION_OFFSET_A2
        s32i    a3 , a2 , EXCEPTION_OFFSET_A3
        s32i    a4 , a2 , EXCEPTION_OFFSET_A4
        s32i    a5 , a2 , EXCEPTION_OFFSET_A5
        s32i    a6 , a2 , EXCEPTION_OFFSET_A6
        s32i    a7 , a2 , EXCEPTION_OFFSET_A7

        /* Save/restore all basic ISA state */
        rsr     a3 , EPC1
        rsr     a4 , EXCCAUSE
        s32i    a3 , a2 , EXCEPTION_OFFSET_PC
        s32i    a3 , a2 , EXCEPTION_OFFSET_EPC1
        s32i    a4 , a2 , EXCEPTION_OFFSET_EXCCAUSE
        /* clear PS.EXCM because PS.EXCM is 0 when first exception happens */
        movi    a3 , 0xffffffef
        rsr     a4 , PS
        and     a4 , a4 , a3
        s32i    a4 , a2 , EXCEPTION_OFFSET_PS
        rsr     a3 , EXCVADDR
        s32i    a3 , a2 , EXCEPTION_OFFSET_EXCVADDR
        rsr     a4 , EXCSAVE1
        s32i    a4 , a2 , EXCEPTION_OFFSET_EXCSAVE1

        /* save Loop option special registers */
        rsr     a3 , LBEG
        s32i    a3 , a2 , EXCEPTION_OFFSET_LBEG
        rsr     a3 , LEND
        s32i    a3 , a2 , EXCEPTION_OFFSET_LEND
        rsr     a3 , LCOUNT
        s32i    a3 , a2 , EXCEPTION_OFFSET_LCOUNT

#ifdef __XTENSA_WINDOWED_ABI__
        /* save window special registers */
        rsr     a3 , WindowBase
        rsr     a4 , WindowStart
        s32i    a3 , a2 , EXCEPTION_OFFSET_WINDOWBASE
        s32i    a4 , a2 , EXCEPTION_OFFSET_WINDOWSTART
        /* save remainder of entire address register file */
        movi    a0 , SAR
        s32i    a0 , a2 , EXCEPTION_OFFSET_SAR
        movi    a0 , XCHAL_NUM_AREGS - 8
#endif
1:      s32i    a8 , a2 , EXCEPTION_OFFSET_A8
        s32i    a9 , a2 , EXCEPTION_OFFSET_A9
        s32i    a10 , a2 , EXCEPTION_OFFSET_A10
        s32i    a11 , a2 , EXCEPTION_OFFSET_A11
        s32i    a12 , a2 , EXCEPTION_OFFSET_A12
        s32i    a13 , a2 , EXCEPTION_OFFSET_A13
        s32i    a14 , a2 , EXCEPTION_OFFSET_A14
        s32i    a15 , a2 , EXCEPTION_OFFSET_A15

#ifdef __XTENSA_WINDOWED_ABI__
        addi    a8 , a0 , -8
        addi    a10 , a2 , 8*4
        rotw    2
        bnez    a0, 1b
        rotw    2
#endif

#ifdef __XTENSA_CALL0_ABI__
        /* clear PS.EXCM and set PS.Level to 0xf*/
        movi    a3 , 0x2f
#else
        /* clear PS.EXCM and set PS.Level to 0xf and enable window check*/
        movi    a3 , 0x0004002f
#endif
        wsr     a3 , PS
        rsync

#ifdef __XTENSA_CALL0_ABI__
        /* change sp to exception stack */
        movi    a2 , pxExceptionStack                                  // change a2 to the address of pxExceptionStack
        l32i    a1 , a2 , 0                                            // change a1 to the address of xExceptionStack
        movi    a0 , DSP_Fault_handler
        callx0  a0
#else
        /* stack init and change sp to exception stack - 16  */
        movi    a2 , pxExceptionStack                                  // change a2 to the address of pxExceptionStack
        l32i    a4 , a2 , 0                                            // change a4 to the address of xExceptionStack
        addi    a1 , a4 , -16
        addi    a4 , a1 , 32
        s32e    a4 , a1 , -12
        movi    a8 , DSP_Fault_handler
        callx8  a8
#endif

// ***************************************************************************
// * Double_Exception_Handler
// * <null>
// *
// * Double exception handler
// ***************************************************************************
    .align 4
    .literal_position
Double_Exception_Handler:
        /* save context to exceptionContext */
        rsr     a0 , EXCSAVE + XCHAL_NMILEVEL                          // resume a0
        wsr     a2 , EXCSAVE + XCHAL_NMILEVEL                          // store  a2
        movi    a2 , pExceptionContext                                 // change a2 to the address of pExceptionContext
        l32i    a2 , a2 , 0                                            // change a0 to the address of exceptionContext

        /* save general core registers */
        s32i    a0 , a2 , EXCEPTION_OFFSET_A0
        s32i    a1 , a2 , EXCEPTION_OFFSET_A1
        rsr     a0 , EXCSAVE + XCHAL_NMILEVEL                          // resume a2 to a0
        s32i    a0 , a2 , EXCEPTION_OFFSET_A2
        s32i    a3 , a2 , EXCEPTION_OFFSET_A3
        s32i    a4 , a2 , EXCEPTION_OFFSET_A4
        s32i    a5 , a2 , EXCEPTION_OFFSET_A5
        s32i    a6 , a2 , EXCEPTION_OFFSET_A6
        s32i    a7 , a2 , EXCEPTION_OFFSET_A7

        /* Save/restore all basic ISA state */
        rsr     a3 , DEPC
        rsr     a4 , EXCCAUSE
        s32i    a3 , a2 , EXCEPTION_OFFSET_PC
        s32i    a3 , a2 , EXCEPTION_OFFSET_DEPC
        rsr     a3 , EPC1
        s32i    a3 , a2 , EXCEPTION_OFFSET_EPC1
        s32i    a4 , a2 , EXCEPTION_OFFSET_EXCCAUSE
        rsr     a4 , PS
        s32i    a4 , a2 , EXCEPTION_OFFSET_PS
        rsr     a3 , EXCVADDR
        s32i    a3 , a2 , EXCEPTION_OFFSET_EXCVADDR
        rsr     a4 , EXCSAVE1
        s32i    a4 , a2 , EXCEPTION_OFFSET_EXCSAVE1

        /* save Loop option special registers */
        rsr     a3 , LBEG
        s32i    a3 , a2 , EXCEPTION_OFFSET_LBEG
        rsr     a3 , LEND
        s32i    a3 , a2 , EXCEPTION_OFFSET_LEND
        rsr     a3 , LCOUNT
        s32i    a3 , a2 , EXCEPTION_OFFSET_LCOUNT

#ifdef __XTENSA_WINDOWED_ABI__
        /* save window special registers */
        rsr     a3 , WindowBase
        rsr     a4 , WindowStart
        s32i    a3 , a2 , EXCEPTION_OFFSET_WINDOWBASE
        s32i    a4 , a2 , EXCEPTION_OFFSET_WINDOWSTART
        /* save remainder of entire address register file */
        movi    a0 , SAR
        s32i    a0 , a2 , EXCEPTION_OFFSET_SAR
        movi    a0 , XCHAL_NUM_AREGS - 8
#endif
1:      s32i    a8 , a2 , EXCEPTION_OFFSET_A8
        s32i    a9 , a2 , EXCEPTION_OFFSET_A9
        s32i    a10 , a2 , EXCEPTION_OFFSET_A10
        s32i    a11 , a2 , EXCEPTION_OFFSET_A11
        s32i    a12 , a2 , EXCEPTION_OFFSET_A12
        s32i    a13 , a2 , EXCEPTION_OFFSET_A13
        s32i    a14 , a2 , EXCEPTION_OFFSET_A14
        s32i    a15 , a2 , EXCEPTION_OFFSET_A15

#ifdef __XTENSA_WINDOWED_ABI__
        addi    a8 , a0 , -8
        addi    a10 , a2 , 8*4
        rotw    2
        bnez    a0, 1b
        rotw    2
#endif

#ifdef __XTENSA_CALL0_ABI__
        /* clear PS.EXCM and set PS.Level to 0xf*/
        movi    a3 , 0x2f
#else
        /* clear PS.EXCM and set PS.Level to 0xf and enable window check*/
        movi    a3 , 0x0004002f
#endif
        wsr     a3 , PS
        rsync
#ifdef __XTENSA_CALL0_ABI__
        /* change sp to exception stack */
        movi    a2 , pxExceptionStack                                  // change a2 to the address of pxExceptionStack
        l32i    a1 , a2 , 0                                            // change a1 to the address of xExceptionStack
        movi    a0 , DSP_Fault_handler
        callx0  a0
#else
        /* stack init and change sp to exception stack - 16  */
        movi    a2 , pxExceptionStack                                  // change a2 to the address of pxExceptionStack
        l32i    a4 , a2 , 0                                            // change a4 to the address of xExceptionStack
        addi    a1 , a4 , -16
        addi    a4 , a1 , 32
        s32e    a4 , a1 , -12
        movi    a8 , DSP_Fault_handler
        callx8  a8
#endif
