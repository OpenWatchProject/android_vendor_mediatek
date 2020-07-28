#ifndef __seninf1_REGS_H__
#define __seninf1_REGS_H__

// ----------------- seninf1 Bit Field Definitions -------------------

typedef unsigned int FIELD;
typedef unsigned int UINT32;

typedef PACKING union
{
    PACKING struct
    {
        FIELD seninf_en                 : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CTRL, *PREG_SENINF_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_dbg_sel         : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF_DBG, *PREG_SENINF_DBG;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_csi2_en         : 1;
        FIELD rsv_1                     : 3;
        FIELD seninf_csi2_sw_rst        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CSI2_CTRL, *PREG_SENINF_CSI2_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_testmdl_en      : 1;
        FIELD rsv_1                     : 3;
        FIELD seninf_testmdl_sw_rst     : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TESTMDL_CTRL, *PREG_SENINF_TESTMDL_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 4;
        FIELD seninf_tg_sw_rst          : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TG_CTRL, *PREG_SENINF_TG_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_scam_en         : 1;
        FIELD rsv_1                     : 3;
        FIELD seninf_scam_sw_rst        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF_SCAM_CTRL, *PREG_SENINF_SCAM_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_pcam_data_sel   : 3;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF_PCAM_CTRL, *PREG_SENINF_PCAM_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rsv_0                     : 4;
        FIELD seninf_ccir_sw_rst        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF_CCIR_CTRL, *PREG_SENINF_CCIR_CTRL;

typedef PACKING union
{
    PACKING struct
    {
        FIELD rg_seninf_spare_0         : 8;
        FIELD rsv_8                     : 8;
        FIELD rg_seninf_spare_1         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF_SPARE, *PREG_SENINF_SPARE;

// ----------------- seninf1 Register Definition -------------------
typedef volatile struct /*0x1A004200*/
{
    REG_SENINF_CTRL                 SENINF_CTRL;      // 4200
    REG_SENINF_DBG                  SENINF_DBG;       // 4204
    UINT32                          rsv_4208[2];      // 4208..420C
    REG_SENINF_CSI2_CTRL            SENINF_CSI2_CTRL; // 4210
    UINT32                          rsv_4214[3];      // 4214..421C
    REG_SENINF_TESTMDL_CTRL         SENINF_TESTMDL_CTRL; // 4220
    UINT32                          rsv_4224[3];      // 4224..422C
    REG_SENINF_TG_CTRL              SENINF_TG_CTRL;   // 4230
    UINT32                          rsv_4234[3];      // 4234..423C
    REG_SENINF_SCAM_CTRL            SENINF_SCAM_CTRL; // 4240
    UINT32                          rsv_4244[3];      // 4244..424C
    REG_SENINF_PCAM_CTRL            SENINF_PCAM_CTRL; // 4250
    UINT32                          rsv_4254[3];      // 4254..425C
    REG_SENINF_CCIR_CTRL            SENINF_CCIR_CTRL; // 4260
    UINT32                          rsv_4264[37];     // 4264..42F4
    REG_SENINF_SPARE                SENINF_SPARE;     // 42F8
}seninf1_REGS, *Pseninf1_REGS;

#endif // __seninf1_REGS_H__
