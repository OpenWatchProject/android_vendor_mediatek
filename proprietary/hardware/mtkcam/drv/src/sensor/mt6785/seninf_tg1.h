#ifndef __SENINF_TG1_REGS_H__
#define __SENINF_TG1_REGS_H__

// ----------------- SENINF_TG1 Bit Field Definitions -------------------
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

typedef volatile union
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD TM_PAT                    : 5;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_TG1_REG_TM_CTL, *PSENINF_TG1_REG_TM_CTL;

typedef volatile union
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} SENINF_TG1_REG_TM_SIZE, *PSENINF_TG1_REG_TM_SIZE;

typedef volatile union
{
    volatile struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_TG1_REG_TM_CLK, *PSENINF_TG1_REG_TM_CLK;

typedef volatile union
{
    volatile struct
    {
        FIELD STAGGER_MODE_EN           : 1;
        FIELD rsv_1                     : 3;
        FIELD EXP_NUM                   : 3;
        FIELD rsv_7                     : 1;
        FIELD EXP_TAG0                  : 4;
        FIELD EXP_TAG1                  : 4;
        FIELD EXP_TAG2                  : 4;
        FIELD EXP_TAG3                  : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_TG1_REG_TM_STAGGER_CON, *PSENINF_TG1_REG_TM_STAGGER_CON;

// ----------------- SENINF_TG1 Register Definition -------------------
typedef volatile struct /*0x1a004600*/
{
    UINT32                          rsv_4600[2];      // 4600..4604
    SENINF_TG1_REG_TM_CTL           TM_CTL;           // 4608
    SENINF_TG1_REG_TM_SIZE          TM_SIZE;          // 460C
    SENINF_TG1_REG_TM_CLK           TM_CLK;           // 4610
    UINT32                          rsv_4614;         // 4614
    SENINF_TG1_REG_TM_STAGGER_CON   TM_STAGGER_CON;   // 4618
    UINT32                          rsv_461C[14328];  // 461C..25F8
    UINT8                           rsv_25FC;         // 25FC
    UINT16                          rsv_25FD;         // 25FD
    UINT8                           rsv_26FF;         // 26FF
    UINT32                          rsv_2700[959];    // 2700..35F8
    UINT8                           rsv_35FC;         // 35FC
    UINT16                          rsv_35FD;         // 35FD
}SENINF_TG1_REGS, *PSENINF_TG1_REGS;

#endif // __SENINF_TG1_REGS_H__
