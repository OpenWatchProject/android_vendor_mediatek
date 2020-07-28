#ifndef _SENINF_REG_H_
#define _SENINF_REG_H_

// ----------------- seninf_top Bit Field Definitions -------------------




#define SENINF_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define SENINF_READ_REG(RegBase, RegName) (RegBase->RegName.Raw)

//#define SENINF_WRITE_BITS(RegBase, RegName, FieldName, Value)
//    do {
//        (RegBase->RegName.Bits.FieldName) = (Value);
//    } while (0)

#define SENINF_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
    } while (0)

#define mt65xx_reg_writel(v, a) \
        do {    \
            *(volatile unsigned int *)(a) = (v);    \
        } while (0)



//#define SENINF_BASE_HW     0x15008000
#define SENINF_BASE_HW     0x1a040000
#define SENINF_BASE_RANGE  0x4000
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* start MT6593_SENINF_TOP_CODA.xml*/
typedef volatile union _REG_SENINF_TOP_CTRL_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD SENINF1_PCLK_SEL          : 1;
        FIELD SENINF2_PCLK_SEL          : 1;
        FIELD SENINF1_PCLK_EN           : 1;
        FIELD SENINF2_PCLK_EN           : 1;
        FIELD rsv_12                    : 4;
        FIELD SENINF_TOP_N3D_SW_RST     : 1;
        FIELD rsv_17                    : 14;
        FIELD SENINF_TOP_DBG_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_CTRL;

typedef volatile union _REG_SENINF_TOP_CMODEL_PAR_
{
    volatile struct
    {
        FIELD SENINF1_EN                : 1;
        FIELD SENINF2_EN                : 1;
        FIELD SENINF3_EN                : 1;
        FIELD SENINF4_EN                : 1;
        FIELD SENINF1_FORMAT            : 4;
        FIELD SENINF2_FORMAT            : 4;
        FIELD SENINF3_FORMAT            : 4;
        FIELD SENINF4_FORMAT            : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_CMODEL_PAR;

typedef volatile union _REG_SENINF_TOP_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF1_MUX_SRC_SEL       : 4;
        FIELD SENINF2_MUX_SRC_SEL       : 4;
        FIELD SENINF3_MUX_SRC_SEL       : 4;
        FIELD SENINF4_MUX_SRC_SEL       : 4;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_MUX_CTRL;

/* end MT6593_SENINF_TOP_CODA.xml*/

/* start MT6593_seninf_n3d.xml*/
typedef volatile union _REG_N3D_CTL_
{
    volatile struct
    {
        FIELD MODE                      : 2;
        FIELD I2C1_EN                   : 1;
        FIELD I2C2_EN                   : 1;
        FIELD I2C1_INT_EN               : 1;
        FIELD I2C2_INT_EN               : 1;
        FIELD N3D_EN                    : 1;
        FIELD W1CLR                     : 1;
        FIELD DIFF_EN                   : 1;
        FIELD DDBG_SEL                  : 3;
        FIELD MODE1_DBG                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_N3D_CTL;

typedef volatile union _REG_N3D_POS_
{
    volatile struct
    {
        FIELD N3D_POS                   : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_POS;

typedef volatile union _REG_N3D_TRIG_
{
    volatile struct
    {
        FIELD I2CA_TRIG                 : 1;
        FIELD I2CB_TRIG                 : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_N3D_TRIG;

typedef volatile union _REG_N3D_INT_
{
    volatile struct
    {
        FIELD I2C1_INT                  : 1;
        FIELD I2C2_INT                  : 1;
        FIELD DIFF_INT                  : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_N3D_INT;

typedef volatile union _REG_N3D_CNT0_
{
    volatile struct
    {
        FIELD N3D_CNT0                  : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_CNT0;

typedef volatile union _REG_N3D_CNT1_
{
    volatile struct
    {
        FIELD N3D_CNT1                  : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_CNT1;

typedef volatile union _REG_N3D_DBG_
{
    volatile struct
    {
        FIELD N3D_DBG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_DBG;

typedef volatile union _REG_N3D_DIFF_THR_
{
    volatile struct
    {
        FIELD N3D_DIFF_THR              : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_DIFF_THR;

typedef volatile union _REG_N3D_DIFF_CNT_
{
    volatile struct
    {
        FIELD N3D_DIFF_CNT              : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_DIFF_CNT;

/* end MT6593_seninf_n3d.xml*/

/* start MT6593_SENINF_CODA.xml*/
typedef volatile union _REG_SENINF1_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CTRL;

typedef volatile union _REG_SENINF2_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CTRL;

typedef volatile union _REG_SENINF3_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CTRL;

typedef volatile union _REG_SENINF4_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_CTRL;

/* end MT6593_SENINF_CODA.xml*/

/* start MT6593_SENINF_MUX_CODA.xml*/
typedef volatile union _REG_SENINF1_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_CTRL;

typedef volatile union _REG_SENINF1_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_INTEN;

typedef volatile union _REG_SENINF1_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_INTSTA;

typedef volatile union _REG_SENINF1_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_SIZE;

typedef volatile union _REG_SENINF1_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_1;

typedef volatile union _REG_SENINF1_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_2;

typedef volatile union _REG_SENINF1_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_3;

typedef volatile union _REG_SENINF1_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_4;

typedef volatile union _REG_SENINF1_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_5;

typedef volatile union _REG_SENINF1_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_6;

typedef volatile union _REG_SENINF1_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_7;

typedef volatile union _REG_SENINF1_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_SPARE;

typedef volatile union _REG_SENINF1_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DATA;

typedef volatile union _REG_SENINF1_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DATA_CNT;

typedef volatile union _REG_SENINF1_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_CROP;

typedef volatile union _REG_SENINF2_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_CTRL;

typedef volatile union _REG_SENINF2_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_INTEN;

typedef volatile union _REG_SENINF2_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_INTSTA;

typedef volatile union _REG_SENINF2_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_SIZE;

typedef volatile union _REG_SENINF2_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_1;

typedef volatile union _REG_SENINF2_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_2;

typedef volatile union _REG_SENINF2_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_3;

typedef volatile union _REG_SENINF2_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_4;

typedef volatile union _REG_SENINF2_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_5;

typedef volatile union _REG_SENINF2_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_6;

typedef volatile union _REG_SENINF2_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_7;

typedef volatile union _REG_SENINF2_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_SPARE;

typedef volatile union _REG_SENINF2_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DATA;

typedef volatile union _REG_SENINF2_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DATA_CNT;

typedef volatile union _REG_SENINF2_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_CROP;

typedef volatile union _REG_SENINF3_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_CTRL;

typedef volatile union _REG_SENINF3_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_INTEN;

typedef volatile union _REG_SENINF3_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_INTSTA;

typedef volatile union _REG_SENINF3_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_SIZE;

typedef volatile union _REG_SENINF3_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_1;

typedef volatile union _REG_SENINF3_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_2;

typedef volatile union _REG_SENINF3_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_3;

typedef volatile union _REG_SENINF3_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_4;

typedef volatile union _REG_SENINF3_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_5;

typedef volatile union _REG_SENINF3_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_6;

typedef volatile union _REG_SENINF3_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_7;

typedef volatile union _REG_SENINF3_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_SPARE;

typedef volatile union _REG_SENINF3_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DATA;

typedef volatile union _REG_SENINF3_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DATA_CNT;

typedef volatile union _REG_SENINF3_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_CROP;

typedef volatile union _REG_SENINF4_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_CTRL;

typedef volatile union _REG_SENINF4_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_INTEN;

typedef volatile union _REG_SENINF4_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_INTSTA;

typedef volatile union _REG_SENINF4_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_SIZE;

typedef volatile union _REG_SENINF4_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_1;

typedef volatile union _REG_SENINF4_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_2;

typedef volatile union _REG_SENINF4_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_3;

typedef volatile union _REG_SENINF4_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_4;

typedef volatile union _REG_SENINF4_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_5;

typedef volatile union _REG_SENINF4_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_6;

typedef volatile union _REG_SENINF4_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_7;

typedef volatile union _REG_SENINF4_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_SPARE;

typedef volatile union _REG_SENINF4_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DATA;

typedef volatile union _REG_SENINF4_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DATA_CNT;

typedef volatile union _REG_SENINF4_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_CROP;

/* end MT6593_SENINF_MUX_CODA.xml*/

/* start MT6593_SENINF_TG_CODA.xml

typedef volatile union _SENINF_REG_TG1_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_PH_CNT;

typedef volatile union _SENINF_REG_TG1_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_SEN_CK;

typedef volatile union _SENINF_REG_TG1_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_CTL;

typedef volatile union _SENINF_REG_TG1_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_SIZE;

typedef volatile union _SENINF_REG_TG1_TM_CLK_
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
} SENINF_REG_TG1_TM_CLK;

typedef volatile union _SENINF_REG_TG2_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_PH_CNT;

typedef volatile union _SENINF_REG_TG2_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_SEN_CK;

typedef volatile union _SENINF_REG_TG2_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_CTL;

typedef volatile union _SENINF_REG_TG2_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_SIZE;

typedef volatile union _SENINF_REG_TG2_TM_CLK_
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
} SENINF_REG_TG2_TM_CLK;

typedef volatile union _SENINF_REG_TG3_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_PH_CNT;

typedef volatile union _SENINF_REG_TG3_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_SEN_CK;

typedef volatile union _SENINF_REG_TG3_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_TM_CTL;

typedef volatile union _SENINF_REG_TG3_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_TM_SIZE;

typedef volatile union _SENINF_REG_TG3_TM_CLK_
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
} SENINF_REG_TG3_TM_CLK;

typedef volatile union _SENINF_REG_TG4_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_PH_CNT;

typedef volatile union _SENINF_REG_TG4_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_SEN_CK;

typedef volatile union _SENINF_REG_TG4_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_TM_CTL;

typedef volatile union _SENINF_REG_TG4_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_TM_SIZE;

typedef volatile union _SENINF_REG_TG4_TM_CLK_
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
} SENINF_REG_TG4_TM_CLK;
 end MT6593_SENINF_TG_CODA.xml*/

/* start MT6593_SENINF_CCIR656_CODA.xml*/
typedef volatile union _REG_CCIR656_CTL_
{
    volatile struct
    {
        FIELD CCIR656_REV_0             : 1;
        FIELD CCIR656_REV_1             : 1;
        FIELD CCIR656_HS_POL            : 1;
        FIELD CCIR656_VS_POL            : 1;
        FIELD CCIR656_PT_EN             : 1;
        FIELD CCIR656_EN                : 1;
        FIELD rsv_6                     : 2;
        FIELD CCIR656_DBG_SEL           : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_CTL;

typedef volatile union _REG_CCIR656_H_
{
    volatile struct
    {
        FIELD CCIR656_HS_START          : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_HS_END            : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_H;

typedef volatile union _REG_CCIR656_PTGEN_H_1_
{
    volatile struct
    {
        FIELD CCIR656_PT_HTOTAL         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HACTIVE        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_1;

typedef volatile union _REG_CCIR656_PTGEN_H_2_
{
    volatile struct
    {
        FIELD CCIR656_PT_HWIDTH         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HSTART         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_2;

typedef volatile union _REG_CCIR656_PTGEN_V_1_
{
    volatile struct
    {
        FIELD CCIR656_PT_VTOTAL         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VACTIVE        : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_1;

typedef volatile union _REG_CCIR656_PTGEN_V_2_
{
    volatile struct
    {
        FIELD CCIR656_PT_VWIDTH         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VSTART         : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_2;

typedef volatile union _REG_CCIR656_PTGEN_CTL1_
{
    volatile struct
    {
        FIELD CCIR656_PT_TYPE           : 8;
        FIELD rsv_8                     : 8;
        FIELD CCIR656_PT_COLOR_BAR_TH   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL1;

typedef volatile union _REG_CCIR656_PTGEN_CTL2_
{
    volatile struct
    {
        FIELD CCIR656_PT_Y              : 8;
        FIELD CCIR656_PT_CB             : 8;
        FIELD CCIR656_PT_CR             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL2;

typedef volatile union _REG_CCIR656_PTGEN_CTL3_
{
    volatile struct
    {
        FIELD CCIR656_PT_BD_Y           : 8;
        FIELD CCIR656_PT_BD_CB          : 8;
        FIELD CCIR656_PT_BD_CR          : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL3;

typedef volatile union _REG_CCIR656_STATUS_
{
    volatile struct
    {
        FIELD CCIR656_IN_FIELD          : 1;
        FIELD CCIR656_IN_VS             : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_STATUS;

/* end MT6593_SENINF_CCIR656_CODA.xml*/

/* start MT6593_MIPI_RX_CONFIG_CODA.xml*/
typedef volatile union _REG_MIPI_RX_CON00_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD RG_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRC_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON00_CSI0;

typedef volatile union _REG_MIPI_RX_CON04_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD0_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON04_CSI0;

typedef volatile union _REG_MIPI_RX_CON08_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD1_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON08_CSI0;

typedef volatile union _REG_MIPI_RX_CON0C_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD2_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON0C_CSI0;

typedef volatile union _REG_MIPI_RX_CON10_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD3_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON10_CSI0;

typedef volatile union _REG_MIPI_RX_CON24_CSI0_
{
    volatile struct
    {
        FIELD CSI0_BIST_NUM             : 2;
        FIELD CSI0_BIST_EN              : 1;
        FIELD rsv_3                     : 2;
        FIELD CSI0_BIST_FIX_PAT         : 1;
        FIELD CSI0_BIST_CLK_SEL         : 1;
        FIELD CSI0_BIST_CLK4X_SEL       : 1;
        FIELD CSI0_BIST_TERM_DELAY      : 8;
        FIELD CSI0_BIST_SETTLE_DELAY    : 8;
        FIELD CSI0_BIST_LN0_MUX         : 2;
        FIELD CSI0_BIST_LN1_MUX         : 2;
        FIELD CSI0_BIST_LN2_MUX         : 2;
        FIELD CSI0_BIST_LN3_MUX         : 2;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON24_CSI0;

typedef volatile union _REG_MIPI_RX_CON28_CSI0_
{
    volatile struct
    {
        FIELD CSI0_BIST_START           : 1;
        FIELD CSI0_BIST_DATA_OK         : 1;
        FIELD CSI0_BIST_HS_FSM_OK       : 1;
        FIELD CSI0_BIST_LANE_FSM_OK     : 1;
        FIELD CSI0_BIST_CSI2_DATA_OK    : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON28_CSI0;

typedef volatile union _REG_MIPI_RX_CON34_CSI0_
{
    volatile struct
    {
        FIELD BIST_MODE                 : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON34_CSI0;

typedef volatile union _REG_MIPI_RX_CON38_CSI0_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_MODE      : 1;
        FIELD MIPI_RX_SW_CAL_MODE       : 1;
        FIELD MIPI_RX_HW_CAL_START      : 1;
        FIELD MIPI_RX_HW_CAL_OPTION     : 1;
        FIELD MIPI_RX_HW_CAL_SW_RST     : 1;
        FIELD MIPI_RX_MACRO_SRC_SEL     : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON38_CSI0;

typedef volatile union _REG_MIPI_RX_CON3C_CSI0_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_          : 32;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON3C_CSI0;

typedef volatile union _REG_MIPI_RX_CON40_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_LPRX_SWAP    : 1;
        FIELD RG_CSI0_LNRC_HSRX_INVERT  : 1;
        FIELD rsv_2                     : 2;
        FIELD RG_CSI0_LNRD0_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD0_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD1_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD1_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD2_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD2_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD3_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD3_HSRX_INVERT : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON40_CSI0;

typedef volatile union _REG_MIPI_RX_CON44_CSI0_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD DA_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON44_CSI0;

typedef volatile union _REG_MIPI_RX_CON48_CSI0_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON48_CSI0;

typedef volatile union _REG_MIPI_RX_CON50_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_BCLK_INV          : 1;
        FIELD rsv_1                     : 3;
        FIELD RG_CSI0_BCLK_MON          : 1;
        FIELD RG_CSI0_4XCLK_MON         : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON50_CSI0;

typedef volatile union _REG_MIPI_RX_CON00_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD RG_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRC_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON00_CSI1;

typedef volatile union _REG_MIPI_RX_CON04_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD0_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON04_CSI1;

typedef volatile union _REG_MIPI_RX_CON08_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD1_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON08_CSI1;

typedef volatile union _REG_MIPI_RX_CON0C_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD2_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON0C_CSI1;

typedef volatile union _REG_MIPI_RX_CON10_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD3_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON10_CSI1;

typedef volatile union _REG_MIPI_RX_CON24_CSI1_
{
    volatile struct
    {
        FIELD CSI0_BIST_NUM             : 2;
        FIELD CSI0_BIST_EN              : 1;
        FIELD rsv_3                     : 2;
        FIELD CSI0_BIST_FIX_PAT         : 1;
        FIELD CSI0_BIST_CLK_SEL         : 1;
        FIELD CSI0_BIST_CLK4X_SEL       : 1;
        FIELD CSI0_BIST_TERM_DELAY      : 8;
        FIELD CSI0_BIST_SETTLE_DELAY    : 8;
        FIELD CSI0_BIST_LN0_MUX         : 2;
        FIELD CSI0_BIST_LN1_MUX         : 2;
        FIELD CSI0_BIST_LN2_MUX         : 2;
        FIELD CSI0_BIST_LN3_MUX         : 2;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON24_CSI1;

typedef volatile union _REG_MIPI_RX_CON28_CSI1_
{
    volatile struct
    {
        FIELD CSI0_BIST_START           : 1;
        FIELD CSI0_BIST_DATA_OK         : 1;
        FIELD CSI0_BIST_HS_FSM_OK       : 1;
        FIELD CSI0_BIST_LANE_FSM_OK     : 1;
        FIELD CSI0_BIST_CSI2_DATA_OK    : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON28_CSI1;

typedef volatile union _REG_MIPI_RX_CON34_CSI1_
{
    volatile struct
    {
        FIELD BIST_MODE                 : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON34_CSI1;

typedef volatile union _REG_MIPI_RX_CON38_CSI1_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_MODE      : 1;
        FIELD MIPI_RX_SW_CAL_MODE       : 1;
        FIELD MIPI_RX_HW_CAL_START      : 1;
        FIELD MIPI_RX_HW_CAL_OPTION     : 1;
        FIELD MIPI_RX_HW_CAL_SW_RST     : 1;
        FIELD MIPI_RX_MACRO_SRC_SEL     : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON38_CSI1;

typedef volatile union _REG_MIPI_RX_CON3C_CSI1_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_          : 32;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON3C_CSI1;

typedef volatile union _REG_MIPI_RX_CON40_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_LPRX_SWAP    : 1;
        FIELD RG_CSI0_LNRC_HSRX_INVERT  : 1;
        FIELD rsv_2                     : 2;
        FIELD RG_CSI0_LNRD0_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD0_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD1_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD1_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD2_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD2_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD3_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD3_HSRX_INVERT : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON40_CSI1;

typedef volatile union _REG_MIPI_RX_CON44_CSI1_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD DA_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON44_CSI1;

typedef volatile union _REG_MIPI_RX_CON48_CSI1_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON48_CSI1;

typedef volatile union _REG_MIPI_RX_CON50_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_BCLK_INV          : 1;
        FIELD rsv_1                     : 3;
        FIELD RG_CSI0_BCLK_MON          : 1;
        FIELD RG_CSI0_4XCLK_MON         : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON50_CSI1;

typedef volatile union _REG_MIPI_RX_CON00_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD RG_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRC_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON00_CSI2;

typedef volatile union _REG_MIPI_RX_CON04_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD0_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON04_CSI2;

typedef volatile union _REG_MIPI_RX_CON08_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD1_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON08_CSI2;

typedef volatile union _REG_MIPI_RX_CON0C_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD2_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON0C_CSI2;

typedef volatile union _REG_MIPI_RX_CON10_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD3_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON10_CSI2;

typedef volatile union _REG_MIPI_RX_CON24_CSI2_
{
    volatile struct
    {
        FIELD CSI0_BIST_NUM             : 2;
        FIELD CSI0_BIST_EN              : 1;
        FIELD rsv_3                     : 2;
        FIELD CSI0_BIST_FIX_PAT         : 1;
        FIELD CSI0_BIST_CLK_SEL         : 1;
        FIELD CSI0_BIST_CLK4X_SEL       : 1;
        FIELD CSI0_BIST_TERM_DELAY      : 8;
        FIELD CSI0_BIST_SETTLE_DELAY    : 8;
        FIELD CSI0_BIST_LN0_MUX         : 2;
        FIELD CSI0_BIST_LN1_MUX         : 2;
        FIELD CSI0_BIST_LN2_MUX         : 2;
        FIELD CSI0_BIST_LN3_MUX         : 2;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON24_CSI2;

typedef volatile union _REG_MIPI_RX_CON28_CSI2_
{
    volatile struct
    {
        FIELD CSI0_BIST_START           : 1;
        FIELD CSI0_BIST_DATA_OK         : 1;
        FIELD CSI0_BIST_HS_FSM_OK       : 1;
        FIELD CSI0_BIST_LANE_FSM_OK     : 1;
        FIELD CSI0_BIST_CSI2_DATA_OK    : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON28_CSI2;

typedef volatile union _REG_MIPI_RX_CON34_CSI2_
{
    volatile struct
    {
        FIELD BIST_MODE                 : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON34_CSI2;

typedef volatile union _REG_MIPI_RX_CON38_CSI2_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_MODE      : 1;
        FIELD MIPI_RX_SW_CAL_MODE       : 1;
        FIELD MIPI_RX_HW_CAL_START      : 1;
        FIELD MIPI_RX_HW_CAL_OPTION     : 1;
        FIELD MIPI_RX_HW_CAL_SW_RST     : 1;
        FIELD MIPI_RX_MACRO_SRC_SEL     : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON38_CSI2;

typedef volatile union _REG_MIPI_RX_CON3C_CSI2_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_          : 32;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON3C_CSI2;

typedef volatile union _REG_MIPI_RX_CON40_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_LPRX_SWAP    : 1;
        FIELD RG_CSI0_LNRC_HSRX_INVERT  : 1;
        FIELD rsv_2                     : 2;
        FIELD RG_CSI0_LNRD0_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD0_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD1_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD1_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD2_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD2_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD3_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD3_HSRX_INVERT : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON40_CSI2;

typedef volatile union _REG_MIPI_RX_CON44_CSI2_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD DA_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON44_CSI2;

typedef volatile union _REG_MIPI_RX_CON48_CSI2_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON48_CSI2;

typedef volatile union _REG_MIPI_RX_CON50_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_BCLK_INV          : 1;
        FIELD rsv_1                     : 3;
        FIELD RG_CSI0_BCLK_MON          : 1;
        FIELD RG_CSI0_4XCLK_MON         : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON50_CSI2;

/* end MT6593_MIPI_RX_CONFIG_CODA.xml*/

/* start MT6593_SENINF_CSI2_CODA.xml*/
typedef volatile union _REG_SENINF1_CSI2_CTRL_
{
    volatile struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CTRL;

typedef volatile union _REG_SENINF1_CSI2_DELAY_
{
    volatile struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DELAY;

typedef volatile union _REG_SENINF1_CSI2_INTEN_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTEN;

typedef volatile union _REG_SENINF1_CSI2_INTSTA_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTSTA;

typedef volatile union _REG_SENINF1_CSI2_ECCDBG_
{
    volatile struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_ECCDBG;

typedef volatile union _REG_SENINF1_CSI2_CRCDBG_
{
    volatile struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CRCDBG;

typedef volatile union _REG_SENINF1_CSI2_DBG_
{
    volatile struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DBG;

typedef volatile union _REG_SENINF1_CSI2_VER_
{
    volatile struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VER;

typedef volatile union _REG_SENINF1_CSI2_SHORT_INFO_
{
    volatile struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_SHORT_INFO;

typedef volatile union _REG_SENINF1_CSI2_LNFSM_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNFSM;

typedef volatile union _REG_SENINF1_CSI2_LNMUX_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNMUX;

typedef volatile union _REG_SENINF1_CSI2_HSYNC_CNT_
{
    volatile struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_HSYNC_CNT;

typedef volatile union _REG_SENINF1_CSI2_CAL_
{
    volatile struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CAL;

typedef volatile union _REG_SENINF1_CSI2_DS_
{
    volatile struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DS;

typedef volatile union _REG_SENINF1_CSI2_VS_
{
    volatile struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VS;

typedef volatile union _REG_SENINF1_CSI2_BIST_
{
    volatile struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_BIST;

typedef volatile union _REG_SENINF2_CSI2_CTRL_
{
    volatile struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CTRL;

typedef volatile union _REG_SENINF2_CSI2_DELAY_
{
    volatile struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DELAY;

typedef volatile union _REG_SENINF2_CSI2_INTEN_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_INTEN;

typedef volatile union _REG_SENINF2_CSI2_INTSTA_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_INTSTA;

typedef volatile union _REG_SENINF2_CSI2_ECCDBG_
{
    volatile struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_ECCDBG;

typedef volatile union _REG_SENINF2_CSI2_CRCDBG_
{
    volatile struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CRCDBG;

typedef volatile union _REG_SENINF2_CSI2_DBG_
{
    volatile struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DBG;

typedef volatile union _REG_SENINF2_CSI2_VER_
{
    volatile struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_VER;

typedef volatile union _REG_SENINF2_CSI2_SHORT_INFO_
{
    volatile struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_SHORT_INFO;

typedef volatile union _REG_SENINF2_CSI2_LNFSM_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_LNFSM;

typedef volatile union _REG_SENINF2_CSI2_LNMUX_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_LNMUX;

typedef volatile union _REG_SENINF2_CSI2_HSYNC_CNT_
{
    volatile struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_HSYNC_CNT;

typedef volatile union _REG_SENINF2_CSI2_CAL_
{
    volatile struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CAL;

typedef volatile union _REG_SENINF2_CSI2_DS_
{
    volatile struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DS;

typedef volatile union _REG_SENINF2_CSI2_VS_
{
    volatile struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_VS;

typedef volatile union _REG_SENINF2_CSI2_BIST_
{
    volatile struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_BIST;

typedef volatile union _REG_SENINF3_CSI2_CTRL_
{
    volatile struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_CTRL;

typedef volatile union _REG_SENINF3_CSI2_DELAY_
{
    volatile struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_DELAY;

typedef volatile union _REG_SENINF3_CSI2_INTEN_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_INTEN;

typedef volatile union _REG_SENINF3_CSI2_INTSTA_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_INTSTA;

typedef volatile union _REG_SENINF3_CSI2_ECCDBG_
{
    volatile struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_ECCDBG;

typedef volatile union _REG_SENINF3_CSI2_CRCDBG_
{
    volatile struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_CRCDBG;

typedef volatile union _REG_SENINF3_CSI2_DBG_
{
    volatile struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_DBG;

typedef volatile union _REG_SENINF3_CSI2_VER_
{
    volatile struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_VER;

typedef volatile union _REG_SENINF3_CSI2_SHORT_INFO_
{
    volatile struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_SHORT_INFO;

typedef volatile union _REG_SENINF3_CSI2_LNFSM_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_LNFSM;

typedef volatile union _REG_SENINF3_CSI2_LNMUX_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_LNMUX;

typedef volatile union _REG_SENINF3_CSI2_HSYNC_CNT_
{
    volatile struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_HSYNC_CNT;

typedef volatile union _REG_SENINF3_CSI2_CAL_
{
    volatile struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_CAL;

typedef volatile union _REG_SENINF3_CSI2_DS_
{
    volatile struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_DS;

typedef volatile union _REG_SENINF3_CSI2_VS_
{
    volatile struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_VS;

typedef volatile union _REG_SENINF3_CSI2_BIST_
{
    volatile struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_BIST;

/* end MT6593_SENINF_CSI2_CODA.xml*/

/* start MT6593_ncsi2.xml*/
typedef volatile union _REG_SENINF1_NCSI2_CTL_
{
    volatile struct /* 0x150083A0 */
    {
        FIELD  DATA_LANE0_EN            :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DATA_LANE1_EN            :  1;      /*  1.. 1, 0x00000002 */
        FIELD  DATA_LANE2_EN            :  1;      /*  2.. 2, 0x00000004 */
        FIELD  DATA_LANE3_EN            :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CLOCK_LANE_EN            :  1;      /*  4.. 4, 0x00000010 */
        FIELD  ECC_EN                   :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CRC_EN                   :  1;      /*  6.. 6, 0x00000040 */
        FIELD  HSRX_DET_EN              :  1;      /*  7.. 7, 0x00000080 */
        FIELD  HS_PRPR_EN               :  1;      /*  8.. 8, 0x00000100 */
        FIELD  HS_END_EN                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  rsv_10                   :  2;      /* 10..11, 0x00000C00 */
        FIELD  GENERIC_LONG_PACKET_EN   :  1;      /* 12..12, 0x00001000 */
        FIELD  IMAGE_PACKET_EN          :  1;      /* 13..13, 0x00002000 */
        FIELD  BYTE2PIXEL_EN            :  1;      /* 14..14, 0x00004000 */
        FIELD  VS_TYPE                  :  1;      /* 15..15, 0x00008000 */
        FIELD  ED_SEL                   :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                   :  1;      /* 17..17, 0x00020000 */
        FIELD  FLUSH_MODE               :  2;      /* 18..19, 0x000C0000 */
        FIELD  SYNC_DET_SCHEME          :  1;      /* 20..20, 0x00100000 */
        FIELD  SYNC_DET_EN              :  1;      /* 21..21, 0x00200000 */
        FIELD  SYNC_DET_BITSWAP_EN      :  1;      /* 22..22, 0x00400000 */
        FIELD  ASYNC_FIFO_RST_SCH       :  2;      /* 23..24, 0x01800000 */
        FIELD  HS_TRAIL_EN              :  1;      /* 25..25, 0x02000000 */
        FIELD  REF_SYNC_DET_EN          :  1;      /* 26..26, 0x04000000 */
        FIELD  CLOCK_HS_OPTION          :  1;      /* 27..27, 0x08000000 */
        FIELD  VS_OUT_CYCLE_NUMBER      :  2;      /* 28..29, 0x30000000 */
        FIELD  rsv_30                   :  2;      /* 30..31, 0xC0000000 */
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_CTL;

typedef volatile union _REG_SENINF1_NCSI2_LNRC_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_TIMING;

typedef volatile union _REG_SENINF1_NCSI2_LNRD_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRD_TIMING;

typedef volatile union _REG_SENINF1_NCSI2_DPCM_
{
    volatile struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 3;
        FIELD DI_30_DPCM_EN             : 1;
        FIELD DI_31_DPCM_EN             : 1;
        FIELD DI_32_DPCM_EN             : 1;
        FIELD DI_33_DPCM_EN             : 1;
        FIELD DI_34_DPCM_EN             : 1;
        FIELD DI_35_DPCM_EN             : 1;
        FIELD DI_36_DPCM_EN             : 1;
        FIELD DI_37_DPCM_EN             : 1;
        FIELD DI_2A_DPCM_EN             : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DPCM;

typedef volatile union _REG_SENINF1_NCSI2_INT_EN_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 16;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_INT_EN;

typedef volatile union _REG_SENINF1_NCSI2_INT_STATUS_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_INT_STATUS;

typedef volatile union _REG_SENINF1_NCSI2_DGB_SEL_
{
    volatile struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DGB_SEL;

typedef volatile union _REG_SENINF1_NCSI2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DBG_PORT;

typedef volatile union _REG_SENINF1_NCSI2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE0;

typedef volatile union _REG_SENINF1_NCSI2_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE1;

typedef volatile union _REG_SENINF1_NCSI2_LNRC_FSM_
{
    volatile struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_FSM;

typedef volatile union _REG_SENINF1_NCSI2_LNRD_FSM_
{
    volatile struct
    {
        FIELD LNRD0_RX_FSM              : 7;
        FIELD rsv_7                     : 1;
        FIELD LNRD1_RX_FSM              : 7;
        FIELD rsv_15                    : 1;
        FIELD LNRD2_RX_FSM              : 7;
        FIELD rsv_23                    : 1;
        FIELD LNRD3_RX_FSM              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRD_FSM;

typedef volatile union _REG_SENINF1_NCSI2_FRAME_LINE_NUM_
{
    volatile struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_FRAME_LINE_NUM;

typedef volatile union _REG_SENINF1_NCSI2_GENERIC_SHORT_
{
    volatile struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_GENERIC_SHORT;

typedef volatile union _REG_SENINF1_NCSI2_HSRX_DBG_
{
    volatile struct
    {
        FIELD DATA_LANE0_HSRX_EN        : 1;
        FIELD DATA_LANE1_HSRX_EN        : 1;
        FIELD DATA_LANE2_HSRX_EN        : 1;
        FIELD DATA_LANE3_HSRX_EN        : 1;
        FIELD CLOCK_LANE_HSRX_EN        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_HSRX_DBG;

typedef volatile union _REG_SENINF1_NCSI2_DI_
{
    volatile struct
    {
        FIELD VC0                       : 2;
        FIELD DT0                       : 6;
        FIELD VC1                       : 2;
        FIELD DT1                       : 6;
        FIELD VC2                       : 2;
        FIELD DT2                       : 6;
        FIELD VC3                       : 2;
        FIELD DT3                       : 6;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DI;

typedef volatile union _REG_SENINF1_NCSI2_HS_TRAIL_
{
    volatile struct
    {
        FIELD HS_TRAIL_PARAMETER        : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_HS_TRAIL;

typedef volatile union _REG_SENINF1_NCSI2_DI_CTRL_
{
    volatile struct
    {
        FIELD VC0_INTERLEAVING          : 1;
        FIELD DT0_INTERLEAVING          : 2;
        FIELD rsv_3                     : 5;
        FIELD VC1_INTERLEAVING          : 1;
        FIELD DT1_INTERLEAVING          : 2;
        FIELD rsv_11                    : 5;
        FIELD VC2_INTERLEAVING          : 1;
        FIELD DT2_INTERLEAVING          : 2;
        FIELD rsv_19                    : 5;
        FIELD VC3_INTERLEAVING          : 1;
        FIELD DT3_INTERLEAVING          : 2;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DI_CTRL;

typedef volatile union _REG_SENINF2_NCSI2_CTL_
{
    volatile struct/* 0x150087A0 */
    {
        FIELD  DATA_LANE0_EN            :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DATA_LANE1_EN            :  1;      /*  1.. 1, 0x00000002 */
        FIELD  DATA_LANE2_EN            :  1;      /*  2.. 2, 0x00000004 */
        FIELD  DATA_LANE3_EN            :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CLOCK_LANE_EN            :  1;      /*  4.. 4, 0x00000010 */
        FIELD  ECC_EN                   :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CRC_EN                   :  1;      /*  6.. 6, 0x00000040 */
        FIELD  HSRX_DET_EN              :  1;      /*  7.. 7, 0x00000080 */
        FIELD  HS_PRPR_EN               :  1;      /*  8.. 8, 0x00000100 */
        FIELD  HS_END_EN                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  rsv_10                   :  2;      /* 10..11, 0x00000C00 */
        FIELD  GENERIC_LONG_PACKET_EN   :  1;      /* 12..12, 0x00001000 */
        FIELD  IMAGE_PACKET_EN          :  1;      /* 13..13, 0x00002000 */
        FIELD  BYTE2PIXEL_EN            :  1;      /* 14..14, 0x00004000 */
        FIELD  VS_TYPE                  :  1;      /* 15..15, 0x00008000 */
        FIELD  ED_SEL                   :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                   :  1;      /* 17..17, 0x00020000 */
        FIELD  FLUSH_MODE               :  2;      /* 18..19, 0x000C0000 */
        FIELD  SYNC_DET_SCHEME          :  1;      /* 20..20, 0x00100000 */
        FIELD  SYNC_DET_EN              :  1;      /* 21..21, 0x00200000 */
        FIELD  SYNC_DET_BITSWAP_EN      :  1;      /* 22..22, 0x00400000 */
        FIELD  ASYNC_FIFO_RST_SCH       :  2;      /* 23..24, 0x01800000 */
        FIELD  HS_TRAIL_EN              :  1;      /* 25..25, 0x02000000 */
        FIELD  REF_SYNC_DET_EN          :  1;      /* 26..26, 0x04000000 */
        FIELD  CLOCK_HS_OPTION          :  1;      /* 27..27, 0x08000000 */
        FIELD  VS_OUT_CYCLE_NUMBER      :  2;      /* 28..29, 0x30000000 */
        FIELD  rsv_30                   :  2;      /* 30..31, 0xC0000000 */
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_CTL;

typedef volatile union _REG_SENINF2_NCSI2_LNRC_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRC_TIMING;

typedef volatile union _REG_SENINF2_NCSI2_LNRD_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRD_TIMING;

typedef volatile union _REG_SENINF2_NCSI2_DPCM_
{
    volatile struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 3;
        FIELD DI_30_DPCM_EN             : 1;
        FIELD DI_31_DPCM_EN             : 1;
        FIELD DI_32_DPCM_EN             : 1;
        FIELD DI_33_DPCM_EN             : 1;
        FIELD DI_34_DPCM_EN             : 1;
        FIELD DI_35_DPCM_EN             : 1;
        FIELD DI_36_DPCM_EN             : 1;
        FIELD DI_37_DPCM_EN             : 1;
        FIELD DI_2A_DPCM_EN             : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DPCM;

typedef volatile union _REG_SENINF2_NCSI2_INT_EN_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 16;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_INT_EN;

typedef volatile union _REG_SENINF2_NCSI2_INT_STATUS_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_INT_STATUS;

typedef volatile union _REG_SENINF2_NCSI2_DGB_SEL_
{
    volatile struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DGB_SEL;

typedef volatile union _REG_SENINF2_NCSI2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DBG_PORT;

typedef volatile union _REG_SENINF2_NCSI2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_SPARE0;

typedef volatile union _REG_SENINF2_NCSI2_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_SPARE1;

typedef volatile union _REG_SENINF2_NCSI2_LNRC_FSM_
{
    volatile struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRC_FSM;

typedef volatile union _REG_SENINF2_NCSI2_LNRD_FSM_
{
    volatile struct
    {
        FIELD LNRD0_RX_FSM              : 7;
        FIELD rsv_7                     : 1;
        FIELD LNRD1_RX_FSM              : 7;
        FIELD rsv_15                    : 1;
        FIELD LNRD2_RX_FSM              : 7;
        FIELD rsv_23                    : 1;
        FIELD LNRD3_RX_FSM              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRD_FSM;

typedef volatile union _REG_SENINF2_NCSI2_FRAME_LINE_NUM_
{
    volatile struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_FRAME_LINE_NUM;

typedef volatile union _REG_SENINF2_NCSI2_GENERIC_SHORT_
{
    volatile struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_GENERIC_SHORT;

typedef volatile union _REG_SENINF2_NCSI2_HSRX_DBG_
{
    volatile struct
    {
        FIELD DATA_LANE0_HSRX_EN        : 1;
        FIELD DATA_LANE1_HSRX_EN        : 1;
        FIELD DATA_LANE2_HSRX_EN        : 1;
        FIELD DATA_LANE3_HSRX_EN        : 1;
        FIELD CLOCK_LANE_HSRX_EN        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_HSRX_DBG;

typedef volatile union _REG_SENINF2_NCSI2_DI_
{
    volatile struct
    {
        FIELD VC0                       : 2;
        FIELD DT0                       : 6;
        FIELD VC1                       : 2;
        FIELD DT1                       : 6;
        FIELD VC2                       : 2;
        FIELD DT2                       : 6;
        FIELD VC3                       : 2;
        FIELD DT3                       : 6;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DI;

typedef volatile union _REG_SENINF2_NCSI2_HS_TRAIL_
{
    volatile struct
    {
        FIELD HS_TRAIL_PARAMETER        : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_HS_TRAIL;

typedef volatile union _REG_SENINF2_NCSI2_DI_CTRL_
{
    volatile struct
    {
        FIELD VC0_INTERLEAVING          : 1;
        FIELD DT0_INTERLEAVING          : 2;
        FIELD rsv_3                     : 5;
        FIELD VC1_INTERLEAVING          : 1;
        FIELD DT1_INTERLEAVING          : 2;
        FIELD rsv_11                    : 5;
        FIELD VC2_INTERLEAVING          : 1;
        FIELD DT2_INTERLEAVING          : 2;
        FIELD rsv_19                    : 5;
        FIELD VC3_INTERLEAVING          : 1;
        FIELD DT3_INTERLEAVING          : 2;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DI_CTRL;

typedef volatile union _REG_SENINF3_NCSI2_CTL_
{
    volatile struct
    {
        FIELD  DATA_LANE0_EN            :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DATA_LANE1_EN            :  1;      /*  1.. 1, 0x00000002 */
        FIELD  DATA_LANE2_EN            :  1;      /*  2.. 2, 0x00000004 */
        FIELD  DATA_LANE3_EN            :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CLOCK_LANE_EN            :  1;      /*  4.. 4, 0x00000010 */
        FIELD  ECC_EN                   :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CRC_EN                   :  1;      /*  6.. 6, 0x00000040 */
        FIELD  HSRX_DET_EN              :  1;      /*  7.. 7, 0x00000080 */
        FIELD  HS_PRPR_EN               :  1;      /*  8.. 8, 0x00000100 */
        FIELD  HS_END_EN                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  rsv_10                   :  2;      /* 10..11, 0x00000C00 */
        FIELD  GENERIC_LONG_PACKET_EN   :  1;      /* 12..12, 0x00001000 */
        FIELD  IMAGE_PACKET_EN          :  1;      /* 13..13, 0x00002000 */
        FIELD  BYTE2PIXEL_EN            :  1;      /* 14..14, 0x00004000 */
        FIELD  VS_TYPE                  :  1;      /* 15..15, 0x00008000 */
        FIELD  ED_SEL                   :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                   :  1;      /* 17..17, 0x00020000 */
        FIELD  FLUSH_MODE               :  2;      /* 18..19, 0x000C0000 */
        FIELD  SYNC_DET_SCHEME          :  1;      /* 20..20, 0x00100000 */
        FIELD  SYNC_DET_EN              :  1;      /* 21..21, 0x00200000 */
        FIELD  SYNC_DET_BITSWAP_EN      :  1;      /* 22..22, 0x00400000 */
        FIELD  ASYNC_FIFO_RST_SCH       :  2;      /* 23..24, 0x01800000 */
        FIELD  HS_TRAIL_EN              :  1;      /* 25..25, 0x02000000 */
        FIELD  REF_SYNC_DET_EN          :  1;      /* 26..26, 0x04000000 */
        FIELD  CLOCK_HS_OPTION          :  1;      /* 27..27, 0x08000000 */
        FIELD  VS_OUT_CYCLE_NUMBER      :  2;      /* 28..29, 0x30000000 */
        FIELD  rsv_30                   :  2;      /* 30..31, 0xC0000000 */
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_CTL;

typedef volatile union _REG_SENINF3_NCSI2_LNRC_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRC_TIMING;

typedef volatile union _REG_SENINF3_NCSI2_LNRD_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRD_TIMING;

typedef volatile union _REG_SENINF3_NCSI2_DPCM_
{
    volatile struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 3;
        FIELD DI_30_DPCM_EN             : 1;
        FIELD DI_31_DPCM_EN             : 1;
        FIELD DI_32_DPCM_EN             : 1;
        FIELD DI_33_DPCM_EN             : 1;
        FIELD DI_34_DPCM_EN             : 1;
        FIELD DI_35_DPCM_EN             : 1;
        FIELD DI_36_DPCM_EN             : 1;
        FIELD DI_37_DPCM_EN             : 1;
        FIELD DI_2A_DPCM_EN             : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DPCM;

typedef volatile union _REG_SENINF3_NCSI2_INT_EN_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 16;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_INT_EN;

typedef volatile union _REG_SENINF3_NCSI2_INT_STATUS_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_INT_STATUS;

typedef volatile union _REG_SENINF3_NCSI2_DGB_SEL_
{
    volatile struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DGB_SEL;

typedef volatile union _REG_SENINF3_NCSI2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DBG_PORT;

typedef volatile union _REG_SENINF3_NCSI2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_SPARE0;

typedef volatile union _REG_SENINF3_NCSI2_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_SPARE1;

typedef volatile union _REG_SENINF3_NCSI2_LNRC_FSM_
{
    volatile struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRC_FSM;

typedef volatile union _REG_SENINF3_NCSI2_LNRD_FSM_
{
    volatile struct
    {
        FIELD LNRD0_RX_FSM              : 7;
        FIELD rsv_7                     : 1;
        FIELD LNRD1_RX_FSM              : 7;
        FIELD rsv_15                    : 1;
        FIELD LNRD2_RX_FSM              : 7;
        FIELD rsv_23                    : 1;
        FIELD LNRD3_RX_FSM              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRD_FSM;

typedef volatile union _REG_SENINF3_NCSI2_FRAME_LINE_NUM_
{
    volatile struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_FRAME_LINE_NUM;

typedef volatile union _REG_SENINF3_NCSI2_GENERIC_SHORT_
{
    volatile struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_GENERIC_SHORT;

typedef volatile union _REG_SENINF3_NCSI2_HSRX_DBG_
{
    volatile struct
    {
        FIELD DATA_LANE0_HSRX_EN        : 1;
        FIELD DATA_LANE1_HSRX_EN        : 1;
        FIELD DATA_LANE2_HSRX_EN        : 1;
        FIELD DATA_LANE3_HSRX_EN        : 1;
        FIELD CLOCK_LANE_HSRX_EN        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_HSRX_DBG;

typedef volatile union _REG_SENINF3_NCSI2_DI_
{
    volatile struct
    {
        FIELD VC0                       : 2;
        FIELD DT0                       : 6;
        FIELD VC1                       : 2;
        FIELD DT1                       : 6;
        FIELD VC2                       : 2;
        FIELD DT2                       : 6;
        FIELD VC3                       : 2;
        FIELD DT3                       : 6;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DI;

typedef volatile union _REG_SENINF3_NCSI2_HS_TRAIL_
{
    volatile struct
    {
        FIELD HS_TRAIL_PARAMETER        : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_HS_TRAIL;

typedef volatile union _REG_SENINF3_NCSI2_DI_CTRL_
{
    volatile struct
    {
        FIELD VC0_INTERLEAVING          : 1;
        FIELD DT0_INTERLEAVING          : 2;
        FIELD rsv_3                     : 5;
        FIELD VC1_INTERLEAVING          : 1;
        FIELD DT1_INTERLEAVING          : 2;
        FIELD rsv_11                    : 5;
        FIELD VC2_INTERLEAVING          : 1;
        FIELD DT2_INTERLEAVING          : 2;
        FIELD rsv_19                    : 5;
        FIELD VC3_INTERLEAVING          : 1;
        FIELD DT3_INTERLEAVING          : 2;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DI_CTRL;

typedef volatile union _SCAM_REG1_CFG_
{
    volatile struct /* 0x1500_8e20*/
    {
        FIELD  INTEN0                                :  1;      /*  0.. 0, 0x00000001 */
        FIELD  INTEN1                                :  1;      /*  1.. 1, 0x00000002 */
        FIELD  INTEN2                                :  1;      /*  2.. 2, 0x00000004 */
        FIELD  INTEN3                                :  1;      /*  3.. 3, 0x00000008 */
        FIELD  INTEN4                                :  1;      /*  4.. 4, 0x00000010 */
        FIELD  INTEN5                                :  1;      /*  5.. 5, 0x00000020 */
        FIELD  INTEN6                                :  1;      /*  6.. 6, 0x00000040 */
        FIELD  INTEN7                                :  1;      /*  7.. 7, 0x00000080 */
        FIELD  rsv_8                                 :  3;      /*  8..10, 0x00000700 */
        FIELD  SPCLK_INV                             :  1;      /* 11..11, 0x00000800 */
        FIELD  Clock_inverse                         :  1;      /* 12..12, 0x00001000 */
        FIELD  INTEN8                                :  1;      /* 13..13, 0x00002000 */
        FIELD  TIME_OUT_CNT_EN                       :  1;      /* 14..14, 0x00004000 */
        FIELD  rsv_15                                :  2;      /* 15..16, 0x00018000 */
        FIELD  Continuous_mode                       :  1;      /* 17..17, 0x00020000 */
        FIELD  LANECON_EN                            :  1;      /* 18..18, 0x00040000 */
        FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
        FIELD  Debug_mode                            :  1;      /* 20..20, 0x00100000 */
        FIELD  CAL_MD                                :  1;      /* 21..21, 0x00200000 */
        FIELD  CAL_CRC_ON                            :  1;      /* 22..22, 0x00400000 */
        FIELD  INTEN9                                :  1;      /* 23..23, 0x00800000 */
        FIELD  CSD_NUM                               :  3;      /* 24..26, 0x07000000 */
        FIELD  rsv_27                                :  1;      /* 27..27, 0x08000000 */
        FIELD  Warning_mask                          :  1;      /* 28..28, 0x10000000 */
        FIELD  C_DATA_PACKET_MD                      :  2;      /* 29..30, 0x60000000 */
        FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
    } Bits;
    UINT32 Raw;
}SCAM_REG1_CFG; /* SCAM_CFG */


typedef volatile union _SCAM_REG1_CON_
{
    volatile struct
    {
        FIELD Enable                    : 1;
        FIELD rsv_1                     : 15;
        FIELD Reset                     : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_CON;

typedef volatile union _SCAM_REG1_INT_
{
    volatile struct
    {
        FIELD INT0                      : 1;
        FIELD INT1                      : 1;
        FIELD INT2                      : 1;
        FIELD INT3                      : 1;
        FIELD INT4                      : 1;
        FIELD INT5                      : 1;
        FIELD INT6                      : 1;
        FIELD INT7                      : 1;
        FIELD INT8                      : 1;
        FIELD INT9                      : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INT;

typedef volatile union _SCAM_REG1_SIZE_
{
    volatile struct
    {
        FIELD WIDTH                     : 12;
        FIELD rsv_12                    : 4;
        FIELD HEIGHT                    : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_SIZE;

typedef volatile union _SCAM_REG1_CFG2_
{
    volatile struct
    {
        FIELD DIS_GATED_CLK             : 1;
        FIELD Reserved                  : 31;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_CFG2;

typedef volatile union _SCAM_REG1_INFO0_
{
    volatile struct
    {
        FIELD LINE_ID                   : 16;
        FIELD PACKET_SIZE               : 16;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO0;

typedef volatile union _SCAM_REG1_INFO1_
{
    volatile struct
    {
        FIELD Reserved                  : 8;
        FIELD DATA_ID                   : 6;
        FIELD CRC_ON                    : 1;
        FIELD ACTIVE                    : 1;
        FIELD DATA_CNT                  : 16;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO1;

typedef volatile union _SCAM_REG1_INFO2_
{
    volatile struct
    {
        FIELD INFO2                     : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO2;

typedef volatile union _SCAM_REG1_INFO3_
{
    volatile struct
    {
        FIELD INFO3                     : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO3;

typedef volatile union _SCAM_REG1_INFO4_
{
    volatile struct
    {
        FIELD FEND_CNT                  : 4;
        FIELD W_CRC_CNT                 : 4;
        FIELD W_SYNC_CNT                : 4;
        FIELD W_PID_CNT                 : 4;
        FIELD W_LID_CNT                 : 4;
        FIELD W_DID_CNT                 : 4;
        FIELD W_SIZE_CNT                : 4;
        FIELD C_CRC_CNT                 : 4;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO4;

typedef volatile union _SCAM_REG1_INFO5_
{
    volatile struct
    {
        FIELD INFO5                     : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO5;

typedef volatile union _SCAM_REG1_INFO6_
{
    volatile struct
    {
        FIELD W_TIME_OUT_CNT            : 4;
        FIELD C_DATA_PACKET_CNT         : 4;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO6;

typedef volatile union _SCAM_REG1_DDR_CTRL_
{
    volatile struct
    {
        FIELD CSK_SEL                   : 1;
        FIELD CSK_0_INV                 : 1;
        FIELD CSK_1_INV                 : 1;
        FIELD CSD_0_SWAP                : 1;
        FIELD CSD_1_SWAP                : 1;
        FIELD CSD_2_SWAP                : 1;
        FIELD CSD_3_SWAP                : 1;
        FIELD DDR_MODE                  : 1;
        FIELD CSD_0_DLY                 : 5;
        FIELD CSD_1_DLY                 : 5;
        FIELD CSD_2_DLY                 : 5;
        FIELD CSD_3_DLY                 : 5;
        FIELD PAD_CSD_NUM               : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_DDR_CTRL;

typedef volatile union _SCAM_REG1_TIME_OUT_
{
    volatile struct
    {
        FIELD TIME_OUT_VALUE            : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_TIME_OUT;

typedef volatile union _SCAM_REG1_LINE_ID_START_
{
    volatile struct
    {
        FIELD LINE_ID_START             : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_LINE_ID_START;

/* end MT6593_ncsi2.xml*/

// ----------------- seninf_top  Grouping Definitions -------------------
// ----------------- seninf_top Register Definition -------------------
#if 1
typedef volatile union _SENINF_REG_TOP_CTRL_
{
		volatile struct	/* 0x1A040000 */
		{
				FIELD  rsv_0                                 :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SENINF1_PCLK_SEL                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF2_PCLK_SEL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF1_PCLK_EN                       :  1;		/* 10..10, 0x00000400 */
				FIELD  SENINF2_PCLK_EN                       :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  SENINF_TOP_N3D_SW_RST                 :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  SENINF_TOP_DBG_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_CTRL;	/* SENINF_TOP_CTRL */

typedef volatile union _SENINF_REG_TOP_CMODEL_PAR_
{
		volatile struct	/* 0x1A040004 */
		{
				FIELD  SENINF1_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF2_EN                            :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF3_EN                            :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF4_EN                            :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF1_FORMAT                        :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SENINF2_FORMAT                        :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF3_FORMAT                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  SENINF4_FORMAT                        :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_CMODEL_PAR;	/* SENINF_TOP_CMODEL_PAR */

typedef volatile union _SENINF_REG_TOP_MUX_CTRL_
{
		volatile struct	/* 0x1A040008 */
		{
				FIELD  SENINF1_MUX_SRC_SEL                   :  4;		/*  0.. 3, 0x0000000F */
				FIELD  SENINF2_MUX_SRC_SEL                   :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  SENINF3_MUX_SRC_SEL                   :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF4_MUX_SRC_SEL                   :  4;		/* 12..15, 0x0000F000 */
				FIELD  SENINF5_MUX_SRC_SEL                   :  4;		/* 16..19, 0x000F0000 */
				FIELD  SENINF6_MUX_SRC_SEL                   :  4;		/* 20..23, 0x00F00000 */
				FIELD  SENINF7_MUX_SRC_SEL                   :  4;		/* 24..27, 0x0F000000 */
				FIELD  SENINF8_MUX_SRC_SEL                   :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TOP_MUX_CTRL;	/* SENINF_TOP_MUX_CTRL */

typedef volatile union _SENINF_REG_N3D_CTL_
{
		volatile struct	/* 0x1A0400C0 */
		{
				FIELD  MODE                                  :  2;		/*  0.. 1, 0x00000003 */
				FIELD  I2C1_EN                               :  1;		/*  2.. 2, 0x00000004 */
				FIELD  I2C2_EN                               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  I2C1_INT_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  I2C2_INT_EN                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  N3D_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  W1CLR                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DIFF_EN                               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DDBG_SEL                              :  3;		/*  9..11, 0x00000E00 */
				FIELD  MODE1_DBG                             :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  SEN1_TIM_EN                           :  1;		/* 16..16, 0x00010000 */
				FIELD  SEN2_TIM_EN                           :  1;		/* 17..17, 0x00020000 */
				FIELD  SEN1_OV_VS_INT_EN                     :  1;		/* 18..18, 0x00040000 */
				FIELD  SEN2_OV_VS_INT_EN                     :  1;		/* 19..19, 0x00080000 */
				FIELD  HW_SYNC_MODE                          :  1;		/* 20..20, 0x00100000 */
				FIELD  VALID_TG_EN                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SYNC_PIN_A_EN                         :  1;		/* 22..22, 0x00400000 */
				FIELD  SYNC_PIN_A_POLARITY                   :  1;		/* 23..23, 0x00800000 */
				FIELD  SYNC_PIN_B_EN                         :  1;		/* 24..24, 0x01000000 */
				FIELD  SYNC_PIN_B_POLARITY                   :  1;		/* 25..25, 0x02000000 */
				FIELD  I2C_MUX_SEL                           :  1;		/* 26..26, 0x04000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_CTL;	/* SENINF_N3D_CTL */

typedef volatile union _SENINF_REG_N3D_POS_
{
		volatile struct	/* 0x1A0400C4 */
		{
				FIELD  N3D_POS                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_POS;	/* SENINF_N3D_POS */

typedef volatile union _SENINF_REG_N3D_TRIG_
{
		volatile struct	/* 0x1A0400C8 */
		{
				FIELD  I2CA_TRIG                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  I2CB_TRIG                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_TRIG;	/* SENINF_N3D_TRIG */

typedef volatile union _SENINF_REG_N3D_INT_
{
		volatile struct	/* 0x1A0400CC */
		{
				FIELD  I2C1_INT                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  I2C2_INT                              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DIFF_INT                              :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_INT;	/* SENINF_N3D_INT */

typedef volatile union _SENINF_REG_N3D_CNT0_
{
		volatile struct	/* 0x1A0400D0 */
		{
				FIELD  N3D_CNT0                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_CNT0;	/* SENINF_N3D_CNT0 */

typedef volatile union _SENINF_REG_N3D_CNT1_
{
		volatile struct	/* 0x1A0400D4 */
		{
				FIELD  N3D_CNT1                              : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_CNT1;	/* SENINF_N3D_CNT1 */

typedef volatile union _SENINF_REG_N3D_DBG_
{
		volatile struct	/* 0x1A0400D8 */
		{
				FIELD  N3D_DBG                               : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_DBG;	/* SENINF_N3D_DBG */

typedef volatile union _SENINF_REG_N3D_DIFF_THR_
{
		volatile struct	/* 0x1A0400DC */
		{
				FIELD  N3D_DIFF_THR                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_DIFF_THR;	/* SENINF_N3D_DIFF_THR */

typedef volatile union _SENINF_REG_N3D_DIFF_CNT_
{
		volatile struct	/* 0x1A0400E0 */
		{
				FIELD  N3D_DIFF_CNT                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_DIFF_CNT;	/* SENINF_N3D_DIFF_CNT */

typedef volatile union _SENINF_REG_N3D_DBG_1_
{
		volatile struct	/* 0x1A0400E4 */
		{
				FIELD  N3D_DBG_1                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_DBG_1;	/* SENINF_N3D_DBG_1 */

typedef volatile union _SENINF_REG_N3D_VALID_TG_CNT_
{
		volatile struct	/* 0x1A0400E8 */
		{
				FIELD  N3D_VALID_TG_CNT                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_VALID_TG_CNT;	/* SENINF_N3D_VALID_TG_CNT */

typedef volatile union _SENINF_REG_N3D_SYNC_A_PERIOD_
{
		volatile struct	/* 0x1A0400EC */
		{
				FIELD  N3D_SYNC_A_PERIOD                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_SYNC_A_PERIOD;	/* SENINF_N3D_SYNC_A_PERIOD */

typedef volatile union _SENINF_REG_N3D_SYNC_B_PERIOD_
{
		volatile struct	/* 0x1A0400F0 */
		{
				FIELD  N3D_SYNC_B_PERIOD                     : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_SYNC_B_PERIOD;	/* SENINF_N3D_SYNC_B_PERIOD */

typedef volatile union _SENINF_REG_N3D_SYNC_A_PULSE_LEN_
{
		volatile struct	/* 0x1A0400F4 */
		{
				FIELD  N3D_SYNC_A_PULSE_LEN                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_SYNC_A_PULSE_LEN;	/* SENINF_N3D_SYNC_A_PULSE_LEN */

typedef volatile union _SENINF_REG_N3D_SYNC_B_PULSE_LEN_
{
		volatile struct	/* 0x1A0400F8 */
		{
				FIELD  N3D_SYNC_B_PULSE_LEN                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_N3D_SYNC_B_PULSE_LEN;	/* SENINF_N3D_SYNC_B_PULSE_LEN */

typedef volatile union _SENINF1_REG_CTRL_
{
		volatile struct	/* 0x1A040100 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI2_SW_RST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CTRL;	/* SENINF1_CTRL */

typedef volatile union _SENINF1_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A040120 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_CTRL;	/* SENINF1_MUX_CTRL */

typedef volatile union _SENINF1_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A040124 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_INTEN;	/* SENINF1_MUX_INTEN */

typedef volatile union _SENINF1_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A040128 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_INTSTA;	/* SENINF1_MUX_INTSTA */

typedef volatile union _SENINF1_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A04012C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_SIZE;	/* SENINF1_MUX_SIZE */

typedef volatile union _SENINF1_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A040130 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_1;	/* SENINF1_MUX_DEBUG_1 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A040134 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_2;	/* SENINF1_MUX_DEBUG_2 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A040138 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_3;	/* SENINF1_MUX_DEBUG_3 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A04013C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_4;	/* SENINF1_MUX_DEBUG_4 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A040140 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_5;	/* SENINF1_MUX_DEBUG_5 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A040144 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_6;	/* SENINF1_MUX_DEBUG_6 */

typedef volatile union _SENINF1_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A040148 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DEBUG_7;	/* SENINF1_MUX_DEBUG_7 */

typedef volatile union _SENINF1_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A04014C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_SPARE;	/* SENINF1_MUX_SPARE */

typedef volatile union _SENINF1_REG_MUX_DATA_
{
		volatile struct	/* 0x1A040150 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DATA;	/* SENINF1_MUX_DATA */

typedef volatile union _SENINF1_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A040154 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_DATA_CNT;	/* SENINF1_MUX_DATA_CNT */

typedef volatile union _SENINF1_REG_MUX_CROP_
{
		volatile struct	/* 0x1A040158 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_MUX_CROP;	/* SENINF1_MUX_CROP */

typedef volatile union _SENINF_REG_TG1_PH_CNT_
{
		volatile struct	/* 0x1A040200 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_PH_CNT;	/* SENINF_TG1_PH_CNT */

typedef volatile union _SENINF_REG_TG1_SEN_CK_
{
		volatile struct	/* 0x1A040204 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_SEN_CK;	/* SENINF_TG1_SEN_CK */

typedef volatile union _SENINF_REG_TG1_TM_CTL_
{
		volatile struct	/* 0x1A040208 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_CTL;	/* SENINF_TG1_TM_CTL */

typedef volatile union _SENINF_REG_TG1_TM_SIZE_
{
		volatile struct	/* 0x1A04020C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_SIZE;	/* SENINF_TG1_TM_SIZE */

typedef volatile union _SENINF_REG_TG1_TM_CLK_
{
		volatile struct	/* 0x1A040210 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_CLK;	/* SENINF_TG1_TM_CLK */

typedef volatile union _SENINF_REG_TG1_TM_STP_
{
		volatile struct	/* 0x1A040214 */
		{
				FIELD  TG_TM_STP                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG1_TM_STP;	/* SENINF_TG1_TM_STP */

typedef volatile union _MIPI_REG_RX_CON00_CSI0_
{
		volatile struct	/* 0x1A040300 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_CAL_APPLY           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_CAL_EN              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_OFFSET_CODE         :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRC_HSRX_OFFSET_OUT          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON00_CSI0;	/* MIPI_RX_CON00_CSI0 */

typedef volatile union _MIPI_REG_RX_CON04_CSI0_
{
		volatile struct	/* 0x1A040304 */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD0_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON04_CSI0;	/* MIPI_RX_CON04_CSI0 */

typedef volatile union _MIPI_REG_RX_CON08_CSI0_
{
		volatile struct	/* 0x1A040308 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD1_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON08_CSI0;	/* MIPI_RX_CON08_CSI0 */

typedef volatile union _MIPI_REG_RX_CON0C_CSI0_
{
		volatile struct	/* 0x1A04030C */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD2_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON0C_CSI0;	/* MIPI_RX_CON0C_CSI0 */

typedef volatile union _MIPI_REG_RX_CON10_CSI0_
{
		volatile struct	/* 0x1A040310 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD3_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON10_CSI0;	/* MIPI_RX_CON10_CSI0 */

typedef volatile union _MIPI_REG_RX_CON24_CSI0_
{
		volatile struct	/* 0x1A040324 */
		{
				FIELD  CSI0_BIST_NUM                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI0_BIST_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  2;		/*  3.. 4, 0x00000018 */
				FIELD  CSI0_BIST_FIX_PAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI0_BIST_CLK_SEL                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI0_BIST_CLK4X_SEL                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI0_BIST_TERM_DELAY                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSI0_BIST_SETTLE_DELAY                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI0_BIST_LN0_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN1_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN2_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN3_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON24_CSI0;	/* MIPI_RX_CON24_CSI0 */

typedef volatile union _MIPI_REG_RX_CON28_CSI0_
{
		volatile struct	/* 0x1A040328 */
		{
				FIELD  CSI0_BIST_START                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI0_BIST_DATA_OK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI0_BIST_HS_FSM_OK                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI0_BIST_LANE_FSM_OK                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI0_BIST_CSI2_DATA_OK                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON28_CSI0;	/* MIPI_RX_CON28_CSI0 */

typedef volatile union _MIPI_REG_RX_CON34_CSI0_
{
		volatile struct	/* 0x1A040334 */
		{
				FIELD  BIST_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON34_CSI0;	/* MIPI_RX_CON34_CSI0 */

typedef volatile union _MIPI_REG_RX_CON38_CSI0_
{
		volatile struct	/* 0x1A040338 */
		{
				FIELD  MIPI_RX_SW_CTRL_MODE                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MIPI_RX_SW_CAL_MODE                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MIPI_RX_HW_CAL_START                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MIPI_RX_HW_CAL_OPTION                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MIPI_RX_HW_CAL_SW_RST                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MIPI_RX_MACRO_SRC_SEL                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON38_CSI0;	/* MIPI_RX_CON38_CSI0 */

typedef volatile union _MIPI_REG_RX_CON3C_CSI0_
{
		volatile struct	/* 0x1A04033C */
		{
				FIELD  MIPI_RX_SW_CTRL_                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON3C_CSI0;	/* MIPI_RX_CON3C_CSI0 */

typedef volatile union _MIPI_REG_RX_CON40_CSI0_
{
		volatile struct	/* 0x1A040340 */
		{
				FIELD  RG_CSI0_LNRC_LPRX_SWAP                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_INVERT              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0_LNRD0_LPRX_SWAP               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0_LNRD0_HSRX_INVERT             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI0_LNRD1_LPRX_SWAP               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RG_CSI0_LNRD1_HSRX_INVERT             :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0_LNRD2_LPRX_SWAP               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD2_HSRX_INVERT             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD3_LPRX_SWAP               :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD3_HSRX_INVERT             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON40_CSI0;	/* MIPI_RX_CON40_CSI0 */

typedef volatile union _MIPI_REG_RX_CON44_CSI0_
{
		volatile struct	/* 0x1A040344 */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DA_CSI0_LNRC_HSRX_CAL_APPLY           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DA_CSI0_LNRC_HSRX_CAL_EN              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DA_CSI0_LNRC_HSRX_OFFSET_CODE         :  5;		/* 10..14, 0x00007C00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DA_CSI0_LNRD1_HSRX_CAL_APPLY          :  1;		/* 16..16, 0x00010000 */
				FIELD  DA_CSI0_LNRD1_HSRX_CAL_EN             :  1;		/* 17..17, 0x00020000 */
				FIELD  DA_CSI0_LNRD1_HSRX_OFFSET_CODE        :  5;		/* 18..22, 0x007C0000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON44_CSI0;	/* MIPI_RX_CON44_CSI0 */

typedef volatile union _MIPI_REG_RX_CON48_CSI0_
{
		volatile struct	/* 0x1A040348 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DA_CSI0_LNRD3_HSRX_CAL_APPLY          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DA_CSI0_LNRD3_HSRX_CAL_EN             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DA_CSI0_LNRD3_HSRX_OFFSET_CODE        :  5;		/* 10..14, 0x00007C00 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON48_CSI0;	/* MIPI_RX_CON48_CSI0 */

typedef volatile union _MIPI_REG_RX_CON50_CSI0_
{
		volatile struct	/* 0x1A040350 */
		{
				FIELD  RG_CSI0_BCLK_INV                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  RG_CSI0_BCLK_MON                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0_4XCLK_MON                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON50_CSI0;	/* MIPI_RX_CON50_CSI0 */

typedef volatile union _SENINF1_REG_CSI2_CTRL_
{
		volatile struct	/* 0x1A040360 */
		{
				FIELD  CSI2_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DLANE1_EN                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DLANE2_EN                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DLANE3_EN                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI2_ECC_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CSI2_ED_SEL                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI2_CLK_MISS_EN                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI2_LP11_RST_EN                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI2_SYNC_RST_EN                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CSI2_ESC_EN                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CSI2_SCLK_SEL                         :  1;		/* 10..10, 0x00000400 */
				FIELD  CSI2_SCLK4X_SEL                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CSI2_SW_RST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  CSI2_VSYNC_TYPE                       :  1;		/* 13..13, 0x00002000 */
				FIELD  CSI2_HSRXEN_PFOOT_CLR                 :  1;		/* 14..14, 0x00004000 */
				FIELD  CSI2_SYNC_CLR_EXTEND                  :  1;		/* 15..15, 0x00008000 */
				FIELD  CSI2_ASYNC_OPTION                     :  1;		/* 16..16, 0x00010000 */
				FIELD  CSI2_DATA_FLOW                        :  2;		/* 17..18, 0x00060000 */
				FIELD  CSI2_BIST_ERROR_COUNT                 :  8;		/* 19..26, 0x07F80000 */
				FIELD  CSI2_BIST_START                       :  1;		/* 27..27, 0x08000000 */
				FIELD  CSI2_BIST_DATA_OK                     :  1;		/* 28..28, 0x10000000 */
				FIELD  CSI2_HS_FSM_OK                        :  1;		/* 29..29, 0x20000000 */
				FIELD  CSI2_LANE_FSM_OK                      :  1;		/* 30..30, 0x40000000 */
				FIELD  CSI2_BIST_CSI2_DATA_OK                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CTRL;	/* SENINF1_CSI2_CTRL */

typedef volatile union _SENINF1_REG_CSI2_DELAY_
{
		volatile struct	/* 0x1A040364 */
		{
				FIELD  LP2HS_CLK_TERM_DELAY                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LP2HS_DATA_SETTLE_DELAY               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  LP2HS_DATA_TERM_DELAY                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DELAY;	/* SENINF1_CSI2_DELAY */

typedef volatile union _SENINF1_REG_CSI2_INTEN_
{
		volatile struct	/* 0x1A040368 */
		{
				FIELD  CRC_ERR_IRQ_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ECC_ERR_IRQ_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ECC_CORRECT_IRQ_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI2SYNC_NONSYNC_IRQ_EN               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  CSI2_WC_NUMBER                        : 16;		/*  8..23, 0x00FFFF00 */
				FIELD  CSI2_DATA_TYPE                        :  6;		/* 24..29, 0x3F000000 */
				FIELD  VCHANNEL_ID                           :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_INTEN;	/* SENINF1_CSI2_INTEN */

typedef volatile union _SENINF1_REG_CSI2_INTSTA_
{
		volatile struct	/* 0x1A04036C */
		{
				FIELD  CRC_ERR_IRQ                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ECC_ERR_IRQ                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ECC_CORRECT_IRQ                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI2SYNC_NONSYNC_IRQ                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI2_IRQ_CLR_SEL                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CSI2_SPARE                            :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  rsv_8                                 : 12;		/*  8..19, 0x000FFF00 */
				FIELD  CSI2OUT_HSYNC                         :  1;		/* 20..20, 0x00100000 */
				FIELD  CSI2OUT_VSYNC                         :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_INTSTA;	/* SENINF1_CSI2_INTSTA */

typedef volatile union _SENINF1_REG_CSI2_ECCDBG_
{
		volatile struct	/* 0x1A040370 */
		{
				FIELD  CSI2_ECCDB_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSI2_ECCDB_BSEL                       : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_ECCDBG;	/* SENINF1_CSI2_ECCDBG */

typedef volatile union _SENINF1_REG_CSI2_CRCDBG_
{
		volatile struct	/* 0x1A040374 */
		{
				FIELD  CSI2_CRCDB_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_SPARE                            :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSI2_CRCDB_WSEL                       : 16;		/*  8..23, 0x00FFFF00 */
				FIELD  CSI2_CRCDB_BSEL                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CRCDBG;	/* SENINF1_CSI2_CRCDBG */

typedef volatile union _SENINF1_REG_CSI2_DBG_
{
		volatile struct	/* 0x1A040378 */
		{
				FIELD  CSI2_DEBUG_ON                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_DBG_SRC_SEL                      :  4;		/*  1.. 4, 0x0000001E */
				FIELD  CSI2_DATA_HS_CS                       :  6;		/*  5..10, 0x000007E0 */
				FIELD  CSI2_CLK_LANE_CS                      :  5;		/* 11..15, 0x0000F800 */
				FIELD  VCHANNEL0_ID                          :  2;		/* 16..17, 0x00030000 */
				FIELD  VCHANNEL1_ID                          :  2;		/* 18..19, 0x000C0000 */
				FIELD  VCHANNEL_ID_EN                        :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  LNC_LPRXDB_EN                         :  1;		/* 22..22, 0x00400000 */
				FIELD  LN0_LPRXDB_EN                         :  1;		/* 23..23, 0x00800000 */
				FIELD  LN1_LPRXDB_EN                         :  1;		/* 24..24, 0x01000000 */
				FIELD  LN2_LPRXDB_EN                         :  1;		/* 25..25, 0x02000000 */
				FIELD  LN3_LPRXDB_EN                         :  1;		/* 26..26, 0x04000000 */
				FIELD  LNC_HSRXDB_EN                         :  1;		/* 27..27, 0x08000000 */
				FIELD  LN0_HSRXDB_EN                         :  1;		/* 28..28, 0x10000000 */
				FIELD  LN1_HSRXDB_EN                         :  1;		/* 29..29, 0x20000000 */
				FIELD  LN2_HSRXDB_EN                         :  1;		/* 30..30, 0x40000000 */
				FIELD  LN3_HSRXDB_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DBG;	/* SENINF1_CSI2_DBG */

typedef volatile union _SENINF1_REG_CSI2_VER_
{
		volatile struct	/* 0x1A04037C */
		{
				FIELD  DATE                                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MONTH                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  YEAR                                  : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_VER;	/* SENINF1_CSI2_VER */

typedef volatile union _SENINF1_REG_CSI2_SHORT_INFO_
{
		volatile struct	/* 0x1A040380 */
		{
				FIELD  CSI2_LINE_NO                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CSI2_FRAME_NO                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_SHORT_INFO;	/* SENINF1_CSI2_SHORT_INFO */

typedef volatile union _SENINF1_REG_CSI2_LNFSM_
{
		volatile struct	/* 0x1A040384 */
		{
				FIELD  CSI2_DATA_LN0_CS                      :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI2_DATA_LN1_CS                      :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  CSI2_DATA_LN2_CS                      :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CSI2_DATA_LN3_CS                      :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_LNFSM;	/* SENINF1_CSI2_LNFSM */

typedef volatile union _SENINF1_REG_CSI2_LNMUX_
{
		volatile struct	/* 0x1A040388 */
		{
				FIELD  CSI2_DATA_LN0_MUX                     :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI2_DATA_LN1_MUX                     :  2;		/*  2.. 3, 0x0000000C */
				FIELD  CSI2_DATA_LN2_MUX                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  CSI2_DATA_LN3_MUX                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_LNMUX;	/* SENINF1_CSI2_LNMUX */

typedef volatile union _SENINF1_REG_CSI2_HSYNC_CNT_
{
		volatile struct	/* 0x1A04038C */
		{
				FIELD  CSI2_HSYNC_CNT                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_HSYNC_CNT;	/* SENINF1_CSI2_HSYNC_CNT */

typedef volatile union _SENINF1_REG_CSI2_CAL_
{
		volatile struct	/* 0x1A040390 */
		{
				FIELD  CSI2_CAL_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CSI2_CAL_STATE                        :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  CSI2_CAL_CNT_1                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI2_CAL_CNT_2                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_CAL;	/* SENINF1_CSI2_CAL */

typedef volatile union _SENINF1_REG_CSI2_DS_
{
		volatile struct	/* 0x1A040394 */
		{
				FIELD  CSI2_DS_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_DS_CTRL                          :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_DS;	/* SENINF1_CSI2_DS */

typedef volatile union _SENINF1_REG_CSI2_VS_
{
		volatile struct	/* 0x1A040398 */
		{
				FIELD  CSI2_VS_CTRL                          :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_VS;	/* SENINF1_CSI2_VS */

typedef volatile union _SENINF1_REG_CSI2_BIST_
{
		volatile struct	/* 0x1A04039C */
		{
				FIELD  CSI2_BIST_LNR0_DATA_OK                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_BIST_LNR1_DATA_OK                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI2_BIST_LNR2_DATA_OK                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI2_BIST_LNR3_DATA_OK                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_CSI2_BIST;	/* SENINF1_CSI2_BIST */

typedef volatile union _SENINF1_REG_NCSI2_CTL_
{
		volatile struct	/* 0x1A0403A0 */
		{
				FIELD  DATA_LANE0_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ECC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CRC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  HSRX_DET_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  HS_PRPR_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HS_END_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  GENERIC_LONG_PACKET_EN                :  1;		/* 12..12, 0x00001000 */
				FIELD  IMAGE_PACKET_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BYTE2PIXEL_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  VS_TYPE                               :  1;		/* 15..15, 0x00008000 */
				FIELD  ED_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  FLUSH_MODE                            :  2;		/* 18..19, 0x000C0000 */
				FIELD  SYNC_DET_SCHEME                       :  1;		/* 20..20, 0x00100000 */
				FIELD  SYNC_DET_EN                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SYNC_DET_BITSWAP_EN                   :  1;		/* 22..22, 0x00400000 */
				FIELD  ASYNC_FIFO_RST_SCH                    :  2;		/* 23..24, 0x01800000 */
				FIELD  HS_TRAIL_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  REF_SYNC_DET_EN                       :  1;		/* 26..26, 0x04000000 */
				FIELD  CLOCK_HS_OPTION                       :  1;		/* 27..27, 0x08000000 */
				FIELD  VS_OUT_CYCLE_NUMBER                   :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_CTL;	/* SENINF1_NCSI2_CTL */

typedef volatile union _SENINF1_REG_NCSI2_LNRC_TIMING_
{
		volatile struct	/* 0x1A0403A4 */
		{
				FIELD  TERM_PARAMETER                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SETTLE_PARAMETER                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_LNRC_TIMING;	/* SENINF1_NCSI2_LNRC_TIMING */

typedef volatile union _SENINF1_REG_NCSI2_LNRD_TIMING_
{
		volatile struct	/* 0x1A0403A8 */
		{
				FIELD  TERM_PARAMETER                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SETTLE_PARAMETER                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_LNRD_TIMING;	/* SENINF1_NCSI2_LNRD_TIMING */

typedef volatile union _SENINF1_REG_NCSI2_DPCM_
{
		volatile struct	/* 0x1A0403AC */
		{
				FIELD  DPCM_MODE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DI_30_DPCM_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DI_31_DPCM_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DI_32_DPCM_EN                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DI_33_DPCM_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  DI_34_DPCM_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  DI_35_DPCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  DI_36_DPCM_EN                         :  1;		/* 13..13, 0x00002000 */
				FIELD  DI_37_DPCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  DI_2A_DPCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_DPCM;	/* SENINF1_NCSI2_DPCM */

typedef volatile union _SENINF1_REG_NCSI2_INT_EN_
{
		volatile struct	/* 0x1A0403B0 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_AFIFO                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  9;		/* 22..30, 0x7FC00000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_INT_EN;	/* SENINF1_NCSI2_INT_EN */

typedef volatile union _SENINF1_REG_NCSI2_INT_STATUS_
{
		volatile struct	/* 0x1A0403B4 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_AFIFO                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_INT_STATUS;	/* SENINF1_NCSI2_INT_STATUS */

typedef volatile union _SENINF1_REG_NCSI2_DGB_SEL_
{
		volatile struct	/* 0x1A0403B8 */
		{
				FIELD  DEBUG_SEL                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_DGB_SEL;	/* SENINF1_NCSI2_DGB_SEL */

typedef volatile union _SENINF1_REG_NCSI2_DBG_PORT_
{
		volatile struct	/* 0x1A0403BC */
		{
				FIELD  CTL_DBG_PORT                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_DBG_PORT;	/* SENINF1_NCSI2_DBG_PORT */

typedef volatile union _SENINF1_REG_NCSI2_SPARE0_
{
		volatile struct	/* 0x1A0403C0 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_SPARE0;	/* SENINF1_NCSI2_SPARE0 */

typedef volatile union _SENINF1_REG_NCSI2_SPARE1_
{
		volatile struct	/* 0x1A0403C4 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_SPARE1;	/* SENINF1_NCSI2_SPARE1 */

typedef volatile union _SENINF1_REG_NCSI2_LNRC_FSM_
{
		volatile struct	/* 0x1A0403C8 */
		{
				FIELD  LNRC_RX_FSM                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_LNRC_FSM;	/* SENINF1_NCSI2_LNRC_FSM */

typedef volatile union _SENINF1_REG_NCSI2_LNRD_FSM_
{
		volatile struct	/* 0x1A0403CC */
		{
				FIELD  LNRD0_RX_FSM                          :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LNRD1_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LNRD2_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  LNRD3_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_LNRD_FSM;	/* SENINF1_NCSI2_LNRD_FSM */

typedef volatile union _SENINF1_REG_NCSI2_FRAME_LINE_NUM_
{
		volatile struct	/* 0x1A0403D0 */
		{
				FIELD  FRAME_NUM                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LINE_NUM                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_FRAME_LINE_NUM;	/* SENINF1_NCSI2_FRAME_LINE_NUM */

typedef volatile union _SENINF1_REG_NCSI2_GENERIC_SHORT_
{
		volatile struct	/* 0x1A0403D4 */
		{
				FIELD  GENERIC_SHORT_PACKET_DT               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  GENERIC_SHORT_PACKET_DATA             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_GENERIC_SHORT;	/* SENINF1_NCSI2_GENERIC_SHORT */

typedef volatile union _SENINF1_REG_NCSI2_HSRX_DBG_
{
		volatile struct	/* 0x1A0403D8 */
		{
				FIELD  DATA_LANE0_HSRX_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_HSRX_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_HSRX_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_HSRX_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_HSRX_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_HSRX_DBG;	/* SENINF1_NCSI2_HSRX_DBG */

typedef volatile union _SENINF1_REG_NCSI2_DI_
{
		volatile struct	/* 0x1A0403DC */
		{
				FIELD  VC0                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT0                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC1                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT1                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  VC2                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  DT2                                   :  6;		/* 18..23, 0x00FC0000 */
				FIELD  VC3                                   :  2;		/* 24..25, 0x03000000 */
				FIELD  DT3                                   :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_DI;	/* SENINF1_NCSI2_DI */

typedef volatile union _SENINF1_REG_NCSI2_HS_TRAIL_
{
		volatile struct	/* 0x1A0403E0 */
		{
				FIELD  HS_TRAIL_PARAMETER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_HS_TRAIL;	/* SENINF1_NCSI2_HS_TRAIL */

typedef volatile union _SENINF1_REG_NCSI2_DI_CTRL_
{
		volatile struct	/* 0x1A0403E4 */
		{
				FIELD  VC0_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT0_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC1_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT1_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VC2_INTERLEAVING                      :  1;		/* 16..16, 0x00010000 */
				FIELD  DT2_INTERLEAVING                      :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  VC3_INTERLEAVING                      :  1;		/* 24..24, 0x01000000 */
				FIELD  DT3_INTERLEAVING                      :  2;		/* 25..26, 0x06000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_DI_CTRL;	/* SENINF1_NCSI2_DI_CTRL */

typedef volatile union _SENINF1_REG_NCSI2_DI_1_
{
		volatile struct	/* 0x1A0403E8 */
		{
				FIELD  VC4                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT4                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC5                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT5                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_DI_1;	/* SENINF1_NCSI2_DI_1 */

typedef volatile union _SENINF1_REG_NCSI2_DI_CTRL_1_
{
		volatile struct	/* 0x1A0403EC */
		{
				FIELD  VC4_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT4_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC5_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT5_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_DI_CTRL_1;	/* SENINF1_NCSI2_DI_CTRL_1 */

typedef volatile union _SENINF1_REG_NCSI2_RAW10_DI_CTRL_
{
		volatile struct	/* 0x1A0403F0 */
		{
				FIELD  RAW10_INTERLEAVING_DT_0               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RAW10_INTERLEAVING_EN_0               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RAW10_INTERLEAVING_DT_1               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  RAW10_INTERLEAVING_EN_1               :  1;		/* 15..15, 0x00008000 */
				FIELD  RAW10_INTERLEAVING_DT_2               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  RAW10_INTERLEAVING_EN_2               :  1;		/* 23..23, 0x00800000 */
				FIELD  RAW10_INTERLEAVING_DT_3               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  RAW10_INTERLEAVING_EN_3               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_RAW10_DI_CTRL;	/* SENINF1_NCSI2_RAW10_DI_CTRL */

typedef volatile union _SENINF1_REG_NCSI2_RAW12_DI_CTRL_
{
		volatile struct	/* 0x1A0403F4 */
		{
				FIELD  RAW12_INTERLEAVING_DT_0               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RAW12_INTERLEAVING_EN_0               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RAW12_INTERLEAVING_DT_1               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  RAW12_INTERLEAVING_EN_1               :  1;		/* 15..15, 0x00008000 */
				FIELD  RAW12_INTERLEAVING_DT_2               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  RAW12_INTERLEAVING_EN_2               :  1;		/* 23..23, 0x00800000 */
				FIELD  RAW12_INTERLEAVING_DT_3               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  RAW12_INTERLEAVING_EN_3               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_RAW12_DI_CTRL;	/* SENINF1_NCSI2_RAW12_DI_CTRL */

typedef volatile union _SENINF1_REG_NCSI2_RAW14_DI_CTRL_
{
		volatile struct	/* 0x1A0403F8 */
		{
				FIELD  RAW14_INTERLEAVING_DT_0               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RAW14_INTERLEAVING_EN_0               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RAW14_INTERLEAVING_DT_1               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  RAW14_INTERLEAVING_EN_1               :  1;		/* 15..15, 0x00008000 */
				FIELD  RAW14_INTERLEAVING_DT_2               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  RAW14_INTERLEAVING_EN_2               :  1;		/* 23..23, 0x00800000 */
				FIELD  RAW14_INTERLEAVING_DT_3               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  RAW14_INTERLEAVING_EN_3               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF1_REG_NCSI2_RAW14_DI_CTRL;	/* SENINF1_NCSI2_RAW14_DI_CTRL */

typedef volatile union _SENINF2_REG_CTRL_
{
		volatile struct	/* 0x1A040500 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI2_SW_RST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CTRL;	/* SENINF2_CTRL */

typedef volatile union _SENINF2_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A040520 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_CTRL;	/* SENINF2_MUX_CTRL */

typedef volatile union _SENINF2_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A040524 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_INTEN;	/* SENINF2_MUX_INTEN */

typedef volatile union _SENINF2_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A040528 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_INTSTA;	/* SENINF2_MUX_INTSTA */

typedef volatile union _SENINF2_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A04052C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_SIZE;	/* SENINF2_MUX_SIZE */

typedef volatile union _SENINF2_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A040530 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_1;	/* SENINF2_MUX_DEBUG_1 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A040534 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_2;	/* SENINF2_MUX_DEBUG_2 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A040538 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_3;	/* SENINF2_MUX_DEBUG_3 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A04053C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_4;	/* SENINF2_MUX_DEBUG_4 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A040540 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_5;	/* SENINF2_MUX_DEBUG_5 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A040544 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_6;	/* SENINF2_MUX_DEBUG_6 */

typedef volatile union _SENINF2_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A040548 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DEBUG_7;	/* SENINF2_MUX_DEBUG_7 */

typedef volatile union _SENINF2_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A04054C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_SPARE;	/* SENINF2_MUX_SPARE */

typedef volatile union _SENINF2_REG_MUX_DATA_
{
		volatile struct	/* 0x1A040550 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DATA;	/* SENINF2_MUX_DATA */

typedef volatile union _SENINF2_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A040554 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_DATA_CNT;	/* SENINF2_MUX_DATA_CNT */

typedef volatile union _SENINF2_REG_MUX_CROP_
{
		volatile struct	/* 0x1A040558 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_MUX_CROP;	/* SENINF2_MUX_CROP */

typedef volatile union _SENINF_REG_TG2_PH_CNT_
{
		volatile struct	/* 0x1A040600 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_PH_CNT;	/* SENINF_TG2_PH_CNT */

typedef volatile union _SENINF_REG_TG2_SEN_CK_
{
		volatile struct	/* 0x1A040604 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_SEN_CK;	/* SENINF_TG2_SEN_CK */

typedef volatile union _SENINF_REG_TG2_TM_CTL_
{
		volatile struct	/* 0x1A040608 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_CTL;	/* SENINF_TG2_TM_CTL */

typedef volatile union _SENINF_REG_TG2_TM_SIZE_
{
		volatile struct	/* 0x1A04060C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_SIZE;	/* SENINF_TG2_TM_SIZE */

typedef volatile union _SENINF_REG_TG2_TM_CLK_
{
		volatile struct	/* 0x1A040610 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_CLK;	/* SENINF_TG2_TM_CLK */

typedef volatile union _SENINF_REG_TG2_TM_STP_
{
		volatile struct	/* 0x1A040614 */
		{
				FIELD  TG_TM_STP                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG2_TM_STP;	/* SENINF_TG2_TM_STP */

typedef volatile union _MIPI_REG_RX_CON00_CSI1_
{
		volatile struct	/* 0x1A040700 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_CAL_APPLY           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_CAL_EN              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_OFFSET_CODE         :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRC_HSRX_OFFSET_OUT          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON00_CSI1;	/* MIPI_RX_CON00_CSI1 */

typedef volatile union _MIPI_REG_RX_CON04_CSI1_
{
		volatile struct	/* 0x1A040704 */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD0_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON04_CSI1;	/* MIPI_RX_CON04_CSI1 */

typedef volatile union _MIPI_REG_RX_CON08_CSI1_
{
		volatile struct	/* 0x1A040708 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD1_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON08_CSI1;	/* MIPI_RX_CON08_CSI1 */

typedef volatile union _MIPI_REG_RX_CON0C_CSI1_
{
		volatile struct	/* 0x1A04070C */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD2_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON0C_CSI1;	/* MIPI_RX_CON0C_CSI1 */

typedef volatile union _MIPI_REG_RX_CON10_CSI1_
{
		volatile struct	/* 0x1A040710 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  AD_CSI0_LNRD3_HSRX_OFFSET_OUT         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON10_CSI1;	/* MIPI_RX_CON10_CSI1 */

typedef volatile union _MIPI_REG_RX_CON24_CSI1_
{
		volatile struct	/* 0x1A040724 */
		{
				FIELD  CSI0_BIST_NUM                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI0_BIST_EN                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  2;		/*  3.. 4, 0x00000018 */
				FIELD  CSI0_BIST_FIX_PAT                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI0_BIST_CLK_SEL                     :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI0_BIST_CLK4X_SEL                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI0_BIST_TERM_DELAY                  :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CSI0_BIST_SETTLE_DELAY                :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI0_BIST_LN0_MUX                     :  2;		/* 24..25, 0x03000000 */
				FIELD  CSI0_BIST_LN1_MUX                     :  2;		/* 26..27, 0x0C000000 */
				FIELD  CSI0_BIST_LN2_MUX                     :  2;		/* 28..29, 0x30000000 */
				FIELD  CSI0_BIST_LN3_MUX                     :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON24_CSI1;	/* MIPI_RX_CON24_CSI1 */

typedef volatile union _MIPI_REG_RX_CON28_CSI1_
{
		volatile struct	/* 0x1A040728 */
		{
				FIELD  CSI0_BIST_START                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI0_BIST_DATA_OK                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI0_BIST_HS_FSM_OK                   :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI0_BIST_LANE_FSM_OK                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI0_BIST_CSI2_DATA_OK                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON28_CSI1;	/* MIPI_RX_CON28_CSI1 */

typedef volatile union _MIPI_REG_RX_CON34_CSI1_
{
		volatile struct	/* 0x1A040734 */
		{
				FIELD  BIST_MODE                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON34_CSI1;	/* MIPI_RX_CON34_CSI1 */

typedef volatile union _MIPI_REG_RX_CON38_CSI1_
{
		volatile struct	/* 0x1A040738 */
		{
				FIELD  MIPI_RX_SW_CTRL_MODE                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  MIPI_RX_SW_CAL_MODE                   :  1;		/*  1.. 1, 0x00000002 */
				FIELD  MIPI_RX_HW_CAL_START                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  MIPI_RX_HW_CAL_OPTION                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  MIPI_RX_HW_CAL_SW_RST                 :  1;		/*  4.. 4, 0x00000010 */
				FIELD  MIPI_RX_MACRO_SRC_SEL                 :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON38_CSI1;	/* MIPI_RX_CON38_CSI1 */

typedef volatile union _MIPI_REG_RX_CON3C_CSI1_
{
		volatile struct	/* 0x1A04073C */
		{
				FIELD  MIPI_RX_SW_CTRL_                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON3C_CSI1;	/* MIPI_RX_CON3C_CSI1 */

typedef volatile union _MIPI_REG_RX_CON40_CSI1_
{
		volatile struct	/* 0x1A040740 */
		{
				FIELD  RG_CSI0_LNRC_LPRX_SWAP                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_INVERT              :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0_LNRD0_LPRX_SWAP               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0_LNRD0_HSRX_INVERT             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI0_LNRD1_LPRX_SWAP               :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RG_CSI0_LNRD1_HSRX_INVERT             :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0_LNRD2_LPRX_SWAP               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD2_HSRX_INVERT             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD3_LPRX_SWAP               :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD3_HSRX_INVERT             :  1;		/* 11..11, 0x00000800 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON40_CSI1;	/* MIPI_RX_CON40_CSI1 */

typedef volatile union _MIPI_REG_RX_CON44_CSI1_
{
		volatile struct	/* 0x1A040744 */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DA_CSI0_LNRC_HSRX_CAL_APPLY           :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DA_CSI0_LNRC_HSRX_CAL_EN              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DA_CSI0_LNRC_HSRX_OFFSET_CODE         :  5;		/* 10..14, 0x00007C00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DA_CSI0_LNRD1_HSRX_CAL_APPLY          :  1;		/* 16..16, 0x00010000 */
				FIELD  DA_CSI0_LNRD1_HSRX_CAL_EN             :  1;		/* 17..17, 0x00020000 */
				FIELD  DA_CSI0_LNRD1_HSRX_OFFSET_CODE        :  5;		/* 18..22, 0x007C0000 */
				FIELD  rsv_23                                :  9;		/* 23..31, 0xFF800000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON44_CSI1;	/* MIPI_RX_CON44_CSI1 */

typedef volatile union _MIPI_REG_RX_CON48_CSI1_
{
		volatile struct	/* 0x1A040748 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_CAL_APPLY          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_CAL_EN             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_OFFSET_CODE        :  5;		/*  2.. 6, 0x0000007C */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DA_CSI0_LNRD3_HSRX_CAL_APPLY          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DA_CSI0_LNRD3_HSRX_CAL_EN             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DA_CSI0_LNRD3_HSRX_OFFSET_CODE        :  5;		/* 10..14, 0x00007C00 */
				FIELD  rsv_15                                : 17;		/* 15..31, 0xFFFF8000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON48_CSI1;	/* MIPI_RX_CON48_CSI1 */

typedef volatile union _MIPI_REG_RX_CON50_CSI1_
{
		volatile struct	/* 0x1A040750 */
		{
				FIELD  RG_CSI0_BCLK_INV                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  RG_CSI0_BCLK_MON                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0_4XCLK_MON                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_CON50_CSI1;	/* MIPI_RX_CON50_CSI1 */

typedef volatile union _SENINF2_REG_CSI2_CTRL_
{
		volatile struct	/* 0x1A040760 */
		{
				FIELD  CSI2_EN                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DLANE1_EN                             :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DLANE2_EN                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DLANE3_EN                             :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI2_ECC_EN                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CSI2_ED_SEL                           :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CSI2_CLK_MISS_EN                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CSI2_LP11_RST_EN                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI2_SYNC_RST_EN                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  CSI2_ESC_EN                           :  1;		/*  9.. 9, 0x00000200 */
				FIELD  CSI2_SCLK_SEL                         :  1;		/* 10..10, 0x00000400 */
				FIELD  CSI2_SCLK4X_SEL                       :  1;		/* 11..11, 0x00000800 */
				FIELD  CSI2_SW_RST                           :  1;		/* 12..12, 0x00001000 */
				FIELD  CSI2_VSYNC_TYPE                       :  1;		/* 13..13, 0x00002000 */
				FIELD  CSI2_HSRXEN_PFOOT_CLR                 :  1;		/* 14..14, 0x00004000 */
				FIELD  CSI2_SYNC_CLR_EXTEND                  :  1;		/* 15..15, 0x00008000 */
				FIELD  CSI2_ASYNC_OPTION                     :  1;		/* 16..16, 0x00010000 */
				FIELD  CSI2_DATA_FLOW                        :  2;		/* 17..18, 0x00060000 */
				FIELD  CSI2_BIST_ERROR_COUNT                 :  8;		/* 19..26, 0x07F80000 */
				FIELD  CSI2_BIST_START                       :  1;		/* 27..27, 0x08000000 */
				FIELD  CSI2_BIST_DATA_OK                     :  1;		/* 28..28, 0x10000000 */
				FIELD  CSI2_HS_FSM_OK                        :  1;		/* 29..29, 0x20000000 */
				FIELD  CSI2_LANE_FSM_OK                      :  1;		/* 30..30, 0x40000000 */
				FIELD  CSI2_BIST_CSI2_DATA_OK                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CTRL;	/* SENINF2_CSI2_CTRL */

typedef volatile union _SENINF2_REG_CSI2_DELAY_
{
		volatile struct	/* 0x1A040764 */
		{
				FIELD  LP2HS_CLK_TERM_DELAY                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  LP2HS_DATA_SETTLE_DELAY               :  8;		/* 16..23, 0x00FF0000 */
				FIELD  LP2HS_DATA_TERM_DELAY                 :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DELAY;	/* SENINF2_CSI2_DELAY */

typedef volatile union _SENINF2_REG_CSI2_INTEN_
{
		volatile struct	/* 0x1A040768 */
		{
				FIELD  CRC_ERR_IRQ_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ECC_ERR_IRQ_EN                        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ECC_CORRECT_IRQ_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI2SYNC_NONSYNC_IRQ_EN               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  CSI2_WC_NUMBER                        : 16;		/*  8..23, 0x00FFFF00 */
				FIELD  CSI2_DATA_TYPE                        :  6;		/* 24..29, 0x3F000000 */
				FIELD  VCHANNEL_ID                           :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_INTEN;	/* SENINF2_CSI2_INTEN */

typedef volatile union _SENINF2_REG_CSI2_INTSTA_
{
		volatile struct	/* 0x1A04076C */
		{
				FIELD  CRC_ERR_IRQ                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ECC_ERR_IRQ                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ECC_CORRECT_IRQ                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI2SYNC_NONSYNC_IRQ                  :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CSI2_IRQ_CLR_SEL                      :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CSI2_SPARE                            :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  rsv_8                                 : 12;		/*  8..19, 0x000FFF00 */
				FIELD  CSI2OUT_HSYNC                         :  1;		/* 20..20, 0x00100000 */
				FIELD  CSI2OUT_VSYNC                         :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_INTSTA;	/* SENINF2_CSI2_INTSTA */

typedef volatile union _SENINF2_REG_CSI2_ECCDBG_
{
		volatile struct	/* 0x1A040770 */
		{
				FIELD  CSI2_ECCDB_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSI2_ECCDB_BSEL                       : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_ECCDBG;	/* SENINF2_CSI2_ECCDBG */

typedef volatile union _SENINF2_REG_CSI2_CRCDBG_
{
		volatile struct	/* 0x1A040774 */
		{
				FIELD  CSI2_CRCDB_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_SPARE                            :  7;		/*  1.. 7, 0x000000FE */
				FIELD  CSI2_CRCDB_WSEL                       : 16;		/*  8..23, 0x00FFFF00 */
				FIELD  CSI2_CRCDB_BSEL                       :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CRCDBG;	/* SENINF2_CSI2_CRCDBG */

typedef volatile union _SENINF2_REG_CSI2_DBG_
{
		volatile struct	/* 0x1A040778 */
		{
				FIELD  CSI2_DEBUG_ON                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_DBG_SRC_SEL                      :  4;		/*  1.. 4, 0x0000001E */
				FIELD  CSI2_DATA_HS_CS                       :  6;		/*  5..10, 0x000007E0 */
				FIELD  CSI2_CLK_LANE_CS                      :  5;		/* 11..15, 0x0000F800 */
				FIELD  VCHANNEL0_ID                          :  2;		/* 16..17, 0x00030000 */
				FIELD  VCHANNEL1_ID                          :  2;		/* 18..19, 0x000C0000 */
				FIELD  VCHANNEL_ID_EN                        :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  LNC_LPRXDB_EN                         :  1;		/* 22..22, 0x00400000 */
				FIELD  LN0_LPRXDB_EN                         :  1;		/* 23..23, 0x00800000 */
				FIELD  LN1_LPRXDB_EN                         :  1;		/* 24..24, 0x01000000 */
				FIELD  LN2_LPRXDB_EN                         :  1;		/* 25..25, 0x02000000 */
				FIELD  LN3_LPRXDB_EN                         :  1;		/* 26..26, 0x04000000 */
				FIELD  LNC_HSRXDB_EN                         :  1;		/* 27..27, 0x08000000 */
				FIELD  LN0_HSRXDB_EN                         :  1;		/* 28..28, 0x10000000 */
				FIELD  LN1_HSRXDB_EN                         :  1;		/* 29..29, 0x20000000 */
				FIELD  LN2_HSRXDB_EN                         :  1;		/* 30..30, 0x40000000 */
				FIELD  LN3_HSRXDB_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DBG;	/* SENINF2_CSI2_DBG */

typedef volatile union _SENINF2_REG_CSI2_VER_
{
		volatile struct	/* 0x1A04077C */
		{
				FIELD  DATE                                  :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MONTH                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  YEAR                                  : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_VER;	/* SENINF2_CSI2_VER */

typedef volatile union _SENINF2_REG_CSI2_SHORT_INFO_
{
		volatile struct	/* 0x1A040780 */
		{
				FIELD  CSI2_LINE_NO                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  CSI2_FRAME_NO                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_SHORT_INFO;	/* SENINF2_CSI2_SHORT_INFO */

typedef volatile union _SENINF2_REG_CSI2_LNFSM_
{
		volatile struct	/* 0x1A040784 */
		{
				FIELD  CSI2_DATA_LN0_CS                      :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  CSI2_DATA_LN1_CS                      :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  CSI2_DATA_LN2_CS                      :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  CSI2_DATA_LN3_CS                      :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_LNFSM;	/* SENINF2_CSI2_LNFSM */

typedef volatile union _SENINF2_REG_CSI2_LNMUX_
{
		volatile struct	/* 0x1A040788 */
		{
				FIELD  CSI2_DATA_LN0_MUX                     :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CSI2_DATA_LN1_MUX                     :  2;		/*  2.. 3, 0x0000000C */
				FIELD  CSI2_DATA_LN2_MUX                     :  2;		/*  4.. 5, 0x00000030 */
				FIELD  CSI2_DATA_LN3_MUX                     :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_LNMUX;	/* SENINF2_CSI2_LNMUX */

typedef volatile union _SENINF2_REG_CSI2_HSYNC_CNT_
{
		volatile struct	/* 0x1A04078C */
		{
				FIELD  CSI2_HSYNC_CNT                        : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                : 19;		/* 13..31, 0xFFFFE000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_HSYNC_CNT;	/* SENINF2_CSI2_HSYNC_CNT */

typedef volatile union _SENINF2_REG_CSI2_CAL_
{
		volatile struct	/* 0x1A040790 */
		{
				FIELD  CSI2_CAL_EN                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  CSI2_CAL_STATE                        :  3;		/*  4.. 6, 0x00000070 */
				FIELD  rsv_7                                 :  9;		/*  7..15, 0x0000FF80 */
				FIELD  CSI2_CAL_CNT_1                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  CSI2_CAL_CNT_2                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_CAL;	/* SENINF2_CSI2_CAL */

typedef volatile union _SENINF2_REG_CSI2_DS_
{
		volatile struct	/* 0x1A040794 */
		{
				FIELD  CSI2_DS_EN                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_DS_CTRL                          :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 : 29;		/*  3..31, 0xFFFFFFF8 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_DS;	/* SENINF2_CSI2_DS */

typedef volatile union _SENINF2_REG_CSI2_VS_
{
		volatile struct	/* 0x1A040798 */
		{
				FIELD  CSI2_VS_CTRL                          :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_VS;	/* SENINF2_CSI2_VS */

typedef volatile union _SENINF2_REG_CSI2_BIST_
{
		volatile struct	/* 0x1A04079C */
		{
				FIELD  CSI2_BIST_LNR0_DATA_OK                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CSI2_BIST_LNR1_DATA_OK                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI2_BIST_LNR2_DATA_OK                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CSI2_BIST_LNR3_DATA_OK                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_CSI2_BIST;	/* SENINF2_CSI2_BIST */

typedef volatile union _SENINF2_REG_NCSI2_CTL_
{
		volatile struct	/* 0x1A0407A0 */
		{
				FIELD  DATA_LANE0_EN                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_EN                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_EN                         :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_EN                         :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ECC_EN                                :  1;		/*  5.. 5, 0x00000020 */
				FIELD  CRC_EN                                :  1;		/*  6.. 6, 0x00000040 */
				FIELD  HSRX_DET_EN                           :  1;		/*  7.. 7, 0x00000080 */
				FIELD  HS_PRPR_EN                            :  1;		/*  8.. 8, 0x00000100 */
				FIELD  HS_END_EN                             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  rsv_10                                :  2;		/* 10..11, 0x00000C00 */
				FIELD  GENERIC_LONG_PACKET_EN                :  1;		/* 12..12, 0x00001000 */
				FIELD  IMAGE_PACKET_EN                       :  1;		/* 13..13, 0x00002000 */
				FIELD  BYTE2PIXEL_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  VS_TYPE                               :  1;		/* 15..15, 0x00008000 */
				FIELD  ED_SEL                                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  1;		/* 17..17, 0x00020000 */
				FIELD  FLUSH_MODE                            :  2;		/* 18..19, 0x000C0000 */
				FIELD  SYNC_DET_SCHEME                       :  1;		/* 20..20, 0x00100000 */
				FIELD  SYNC_DET_EN                           :  1;		/* 21..21, 0x00200000 */
				FIELD  SYNC_DET_BITSWAP_EN                   :  1;		/* 22..22, 0x00400000 */
				FIELD  ASYNC_FIFO_RST_SCH                    :  2;		/* 23..24, 0x01800000 */
				FIELD  HS_TRAIL_EN                           :  1;		/* 25..25, 0x02000000 */
				FIELD  REF_SYNC_DET_EN                       :  1;		/* 26..26, 0x04000000 */
				FIELD  CLOCK_HS_OPTION                       :  1;		/* 27..27, 0x08000000 */
				FIELD  VS_OUT_CYCLE_NUMBER                   :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_CTL;	/* SENINF2_NCSI2_CTL */

typedef volatile union _SENINF2_REG_NCSI2_LNRC_TIMING_
{
		volatile struct	/* 0x1A0407A4 */
		{
				FIELD  TERM_PARAMETER                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SETTLE_PARAMETER                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_LNRC_TIMING;	/* SENINF2_NCSI2_LNRC_TIMING */

typedef volatile union _SENINF2_REG_NCSI2_LNRD_TIMING_
{
		volatile struct	/* 0x1A0407A8 */
		{
				FIELD  TERM_PARAMETER                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  SETTLE_PARAMETER                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_LNRD_TIMING;	/* SENINF2_NCSI2_LNRD_TIMING */

typedef volatile union _SENINF2_REG_NCSI2_DPCM_
{
		volatile struct	/* 0x1A0407AC */
		{
				FIELD  DPCM_MODE                             :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DI_30_DPCM_EN                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DI_31_DPCM_EN                         :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DI_32_DPCM_EN                         :  1;		/*  9.. 9, 0x00000200 */
				FIELD  DI_33_DPCM_EN                         :  1;		/* 10..10, 0x00000400 */
				FIELD  DI_34_DPCM_EN                         :  1;		/* 11..11, 0x00000800 */
				FIELD  DI_35_DPCM_EN                         :  1;		/* 12..12, 0x00001000 */
				FIELD  DI_36_DPCM_EN                         :  1;		/* 13..13, 0x00002000 */
				FIELD  DI_37_DPCM_EN                         :  1;		/* 14..14, 0x00004000 */
				FIELD  DI_2A_DPCM_EN                         :  1;		/* 15..15, 0x00008000 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_DPCM;	/* SENINF2_NCSI2_DPCM */

typedef volatile union _SENINF2_REG_NCSI2_INT_EN_
{
		volatile struct	/* 0x1A0407B0 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_AFIFO                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  ERR_FRAME_SYNC_S4                     :  1;		/* 20..20, 0x00100000 */
				FIELD  ERR_FRAME_SYNC_S5                     :  1;		/* 21..21, 0x00200000 */
				FIELD  rsv_22                                :  9;		/* 22..30, 0x7FC00000 */
				FIELD  INT_WCLR_EN                           :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_INT_EN;	/* SENINF2_NCSI2_INT_EN */

typedef volatile union _SENINF2_REG_NCSI2_INT_STATUS_
{
		volatile struct	/* 0x1A0407B4 */
		{
				FIELD  ERR_FRAME_SYNC                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  ERR_ID                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  ERR_ECC_NO_ERROR                      :  1;		/*  2.. 2, 0x00000004 */
				FIELD  ERR_ECC_CORRECTED                     :  1;		/*  3.. 3, 0x00000008 */
				FIELD  ERR_ECC_DOUBLE                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  ERR_CRC                               :  1;		/*  5.. 5, 0x00000020 */
				FIELD  ERR_AFIFO                             :  1;		/*  6.. 6, 0x00000040 */
				FIELD  ERR_MULTI_LANE_SYNC                   :  1;		/*  7.. 7, 0x00000080 */
				FIELD  ERR_SOT_SYNC_HS_LNRD0                 :  1;		/*  8.. 8, 0x00000100 */
				FIELD  ERR_SOT_SYNC_HS_LNRD1                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  ERR_SOT_SYNC_HS_LNRD2                 :  1;		/* 10..10, 0x00000400 */
				FIELD  ERR_SOT_SYNC_HS_LNRD3                 :  1;		/* 11..11, 0x00000800 */
				FIELD  FS                                    :  1;		/* 12..12, 0x00001000 */
				FIELD  LS                                    :  1;		/* 13..13, 0x00002000 */
				FIELD  GS                                    :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  ERR_FRAME_SYNC_S0                     :  1;		/* 16..16, 0x00010000 */
				FIELD  ERR_FRAME_SYNC_S1                     :  1;		/* 17..17, 0x00020000 */
				FIELD  ERR_FRAME_SYNC_S2                     :  1;		/* 18..18, 0x00040000 */
				FIELD  ERR_FRAME_SYNC_S3                     :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_INT_STATUS;	/* SENINF2_NCSI2_INT_STATUS */

typedef volatile union _SENINF2_REG_NCSI2_DGB_SEL_
{
		volatile struct	/* 0x1A0407B8 */
		{
				FIELD  DEBUG_SEL                             :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_DGB_SEL;	/* SENINF2_NCSI2_DGB_SEL */

typedef volatile union _SENINF2_REG_NCSI2_DBG_PORT_
{
		volatile struct	/* 0x1A0407BC */
		{
				FIELD  CTL_DBG_PORT                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_DBG_PORT;	/* SENINF2_NCSI2_DBG_PORT */

typedef volatile union _SENINF2_REG_NCSI2_SPARE0_
{
		volatile struct	/* 0x1A0407C0 */
		{
				FIELD  SPARE0                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_SPARE0;	/* SENINF2_NCSI2_SPARE0 */

typedef volatile union _SENINF2_REG_NCSI2_SPARE1_
{
		volatile struct	/* 0x1A0407C4 */
		{
				FIELD  SPARE1                                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_SPARE1;	/* SENINF2_NCSI2_SPARE1 */

typedef volatile union _SENINF2_REG_NCSI2_LNRC_FSM_
{
		volatile struct	/* 0x1A0407C8 */
		{
				FIELD  LNRC_RX_FSM                           :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_LNRC_FSM;	/* SENINF2_NCSI2_LNRC_FSM */

typedef volatile union _SENINF2_REG_NCSI2_LNRD_FSM_
{
		volatile struct	/* 0x1A0407CC */
		{
				FIELD  LNRD0_RX_FSM                          :  7;		/*  0.. 6, 0x0000007F */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  LNRD1_RX_FSM                          :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  LNRD2_RX_FSM                          :  7;		/* 16..22, 0x007F0000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  LNRD3_RX_FSM                          :  7;		/* 24..30, 0x7F000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_LNRD_FSM;	/* SENINF2_NCSI2_LNRD_FSM */

typedef volatile union _SENINF2_REG_NCSI2_FRAME_LINE_NUM_
{
		volatile struct	/* 0x1A0407D0 */
		{
				FIELD  FRAME_NUM                             : 16;		/*  0..15, 0x0000FFFF */
				FIELD  LINE_NUM                              : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_FRAME_LINE_NUM;	/* SENINF2_NCSI2_FRAME_LINE_NUM */

typedef volatile union _SENINF2_REG_NCSI2_GENERIC_SHORT_
{
		volatile struct	/* 0x1A0407D4 */
		{
				FIELD  GENERIC_SHORT_PACKET_DT               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 10;		/*  6..15, 0x0000FFC0 */
				FIELD  GENERIC_SHORT_PACKET_DATA             : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_GENERIC_SHORT;	/* SENINF2_NCSI2_GENERIC_SHORT */

typedef volatile union _SENINF2_REG_NCSI2_HSRX_DBG_
{
		volatile struct	/* 0x1A0407D8 */
		{
				FIELD  DATA_LANE0_HSRX_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DATA_LANE1_HSRX_EN                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DATA_LANE2_HSRX_EN                    :  1;		/*  2.. 2, 0x00000004 */
				FIELD  DATA_LANE3_HSRX_EN                    :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CLOCK_LANE_HSRX_EN                    :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_HSRX_DBG;	/* SENINF2_NCSI2_HSRX_DBG */

typedef volatile union _SENINF2_REG_NCSI2_DI_
{
		volatile struct	/* 0x1A0407DC */
		{
				FIELD  VC0                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT0                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC1                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT1                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  VC2                                   :  2;		/* 16..17, 0x00030000 */
				FIELD  DT2                                   :  6;		/* 18..23, 0x00FC0000 */
				FIELD  VC3                                   :  2;		/* 24..25, 0x03000000 */
				FIELD  DT3                                   :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_DI;	/* SENINF2_NCSI2_DI */

typedef volatile union _SENINF2_REG_NCSI2_HS_TRAIL_
{
		volatile struct	/* 0x1A0407E0 */
		{
				FIELD  HS_TRAIL_PARAMETER                    :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_HS_TRAIL;	/* SENINF2_NCSI2_HS_TRAIL */

typedef volatile union _SENINF2_REG_NCSI2_DI_CTRL_
{
		volatile struct	/* 0x1A0407E4 */
		{
				FIELD  VC0_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT0_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC1_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT1_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                :  5;		/* 11..15, 0x0000F800 */
				FIELD  VC2_INTERLEAVING                      :  1;		/* 16..16, 0x00010000 */
				FIELD  DT2_INTERLEAVING                      :  2;		/* 17..18, 0x00060000 */
				FIELD  rsv_19                                :  5;		/* 19..23, 0x00F80000 */
				FIELD  VC3_INTERLEAVING                      :  1;		/* 24..24, 0x01000000 */
				FIELD  DT3_INTERLEAVING                      :  2;		/* 25..26, 0x06000000 */
				FIELD  rsv_27                                :  5;		/* 27..31, 0xF8000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_DI_CTRL;	/* SENINF2_NCSI2_DI_CTRL */

typedef volatile union _SENINF2_REG_NCSI2_DI_1_
{
		volatile struct	/* 0x1A0407E8 */
		{
				FIELD  VC4                                   :  2;		/*  0.. 1, 0x00000003 */
				FIELD  DT4                                   :  6;		/*  2.. 7, 0x000000FC */
				FIELD  VC5                                   :  2;		/*  8.. 9, 0x00000300 */
				FIELD  DT5                                   :  6;		/* 10..15, 0x0000FC00 */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_DI_1;	/* SENINF2_NCSI2_DI_1 */

typedef volatile union _SENINF2_REG_NCSI2_DI_CTRL_1_
{
		volatile struct	/* 0x1A0407EC */
		{
				FIELD  VC4_INTERLEAVING                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DT4_INTERLEAVING                      :  2;		/*  1.. 2, 0x00000006 */
				FIELD  rsv_3                                 :  5;		/*  3.. 7, 0x000000F8 */
				FIELD  VC5_INTERLEAVING                      :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DT5_INTERLEAVING                      :  2;		/*  9..10, 0x00000600 */
				FIELD  rsv_11                                : 21;		/* 11..31, 0xFFFFF800 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_DI_CTRL_1;	/* SENINF2_NCSI2_DI_CTRL_1 */

typedef volatile union _SENINF2_REG_NCSI2_RAW10_DI_CTRL_
{
		volatile struct	/* 0x1A0407F0 */
		{
				FIELD  RAW10_INTERLEAVING_DT_0               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RAW10_INTERLEAVING_EN_0               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RAW10_INTERLEAVING_DT_1               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  RAW10_INTERLEAVING_EN_1               :  1;		/* 15..15, 0x00008000 */
				FIELD  RAW10_INTERLEAVING_DT_2               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  RAW10_INTERLEAVING_EN_2               :  1;		/* 23..23, 0x00800000 */
				FIELD  RAW10_INTERLEAVING_DT_3               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  RAW10_INTERLEAVING_EN_3               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_RAW10_DI_CTRL;	/* SENINF2_NCSI2_RAW10_DI_CTRL */

typedef volatile union _SENINF2_REG_NCSI2_RAW12_DI_CTRL_
{
		volatile struct	/* 0x1A0407F4 */
		{
				FIELD  RAW12_INTERLEAVING_DT_0               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RAW12_INTERLEAVING_EN_0               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RAW12_INTERLEAVING_DT_1               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  RAW12_INTERLEAVING_EN_1               :  1;		/* 15..15, 0x00008000 */
				FIELD  RAW12_INTERLEAVING_DT_2               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  RAW12_INTERLEAVING_EN_2               :  1;		/* 23..23, 0x00800000 */
				FIELD  RAW12_INTERLEAVING_DT_3               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  RAW12_INTERLEAVING_EN_3               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_RAW12_DI_CTRL;	/* SENINF2_NCSI2_RAW12_DI_CTRL */

typedef volatile union _SENINF2_REG_NCSI2_RAW14_DI_CTRL_
{
		volatile struct	/* 0x1A0407F8 */
		{
				FIELD  RAW14_INTERLEAVING_DT_0               :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RAW14_INTERLEAVING_EN_0               :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RAW14_INTERLEAVING_DT_1               :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  1;		/* 14..14, 0x00004000 */
				FIELD  RAW14_INTERLEAVING_EN_1               :  1;		/* 15..15, 0x00008000 */
				FIELD  RAW14_INTERLEAVING_DT_2               :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  RAW14_INTERLEAVING_EN_2               :  1;		/* 23..23, 0x00800000 */
				FIELD  RAW14_INTERLEAVING_DT_3               :  6;		/* 24..29, 0x3F000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  RAW14_INTERLEAVING_EN_3               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF2_REG_NCSI2_RAW14_DI_CTRL;	/* SENINF2_NCSI2_RAW14_DI_CTRL */

typedef volatile union _SENINF3_REG_CTRL_
{
		volatile struct	/* 0x1A040900 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI2_SW_RST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_CTRL;	/* SENINF3_CTRL */

typedef volatile union _SENINF3_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A040920 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_CTRL;	/* SENINF3_MUX_CTRL */

typedef volatile union _SENINF3_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A040924 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_INTEN;	/* SENINF3_MUX_INTEN */

typedef volatile union _SENINF3_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A040928 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_INTSTA;	/* SENINF3_MUX_INTSTA */

typedef volatile union _SENINF3_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A04092C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_SIZE;	/* SENINF3_MUX_SIZE */

typedef volatile union _SENINF3_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A040930 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_1;	/* SENINF3_MUX_DEBUG_1 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A040934 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_2;	/* SENINF3_MUX_DEBUG_2 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A040938 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_3;	/* SENINF3_MUX_DEBUG_3 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A04093C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_4;	/* SENINF3_MUX_DEBUG_4 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A040940 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_5;	/* SENINF3_MUX_DEBUG_5 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A040944 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_6;	/* SENINF3_MUX_DEBUG_6 */

typedef volatile union _SENINF3_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A040948 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DEBUG_7;	/* SENINF3_MUX_DEBUG_7 */

typedef volatile union _SENINF3_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A04094C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_SPARE;	/* SENINF3_MUX_SPARE */

typedef volatile union _SENINF3_REG_MUX_DATA_
{
		volatile struct	/* 0x1A040950 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DATA;	/* SENINF3_MUX_DATA */

typedef volatile union _SENINF3_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A040954 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_DATA_CNT;	/* SENINF3_MUX_DATA_CNT */

typedef volatile union _SENINF3_REG_MUX_CROP_
{
		volatile struct	/* 0x1A040958 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF3_REG_MUX_CROP;	/* SENINF3_MUX_CROP */

typedef volatile union _SENINF_REG_TG3_PH_CNT_
{
		volatile struct	/* 0x1A040A00 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_PH_CNT;	/* SENINF_TG3_PH_CNT */

typedef volatile union _SENINF_REG_TG3_SEN_CK_
{
		volatile struct	/* 0x1A040A04 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_SEN_CK;	/* SENINF_TG3_SEN_CK */

typedef volatile union _SENINF_REG_TG3_TM_CTL_
{
		volatile struct	/* 0x1A040A08 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_CTL;	/* SENINF_TG3_TM_CTL */

typedef volatile union _SENINF_REG_TG3_TM_SIZE_
{
		volatile struct	/* 0x1A040A0C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_SIZE;	/* SENINF_TG3_TM_SIZE */

typedef volatile union _SENINF_REG_TG3_TM_CLK_
{
		volatile struct	/* 0x1A040A10 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_CLK;	/* SENINF_TG3_TM_CLK */

typedef volatile union _SENINF_REG_TG3_TM_STP_
{
		volatile struct	/* 0x1A040A14 */
		{
				FIELD  TG_TM_STP                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG3_TM_STP;	/* SENINF_TG3_TM_STP */

typedef volatile union _SENINF4_REG_CTRL_
{
		volatile struct	/* 0x1A040D00 */
		{
				FIELD  SENINF_EN                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  NCSI2_SW_RST                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CSI2_SW_RST                           :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR_SW_RST                           :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CKGEN_SW_RST                          :  1;		/*  4.. 4, 0x00000010 */
				FIELD  TEST_MODEL_SW_RST                     :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SCAM_SW_RST                           :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_DEBUG_SEL                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  PAD2CAM_DATA_SEL                      :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_CTRL;	/* SENINF4_CTRL */

typedef volatile union _SENINF4_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A040D20 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_CTRL;	/* SENINF4_MUX_CTRL */

typedef volatile union _SENINF4_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A040D24 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_INTEN;	/* SENINF4_MUX_INTEN */

typedef volatile union _SENINF4_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A040D28 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_INTSTA;	/* SENINF4_MUX_INTSTA */

typedef volatile union _SENINF4_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A040D2C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_SIZE;	/* SENINF4_MUX_SIZE */

typedef volatile union _SENINF4_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A040D30 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_1;	/* SENINF4_MUX_DEBUG_1 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A040D34 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_2;	/* SENINF4_MUX_DEBUG_2 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A040D38 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_3;	/* SENINF4_MUX_DEBUG_3 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A040D3C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_4;	/* SENINF4_MUX_DEBUG_4 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A040D40 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_5;	/* SENINF4_MUX_DEBUG_5 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A040D44 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_6;	/* SENINF4_MUX_DEBUG_6 */

typedef volatile union _SENINF4_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A040D48 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DEBUG_7;	/* SENINF4_MUX_DEBUG_7 */

typedef volatile union _SENINF4_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A040D4C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_SPARE;	/* SENINF4_MUX_SPARE */

typedef volatile union _SENINF4_REG_MUX_DATA_
{
		volatile struct	/* 0x1A040D50 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DATA;	/* SENINF4_MUX_DATA */

typedef volatile union _SENINF4_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A040D54 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_DATA_CNT;	/* SENINF4_MUX_DATA_CNT */

typedef volatile union _SENINF4_REG_MUX_CROP_
{
		volatile struct	/* 0x1A040D58 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF4_REG_MUX_CROP;	/* SENINF4_MUX_CROP */

typedef volatile union _SENINF_REG_TG4_PH_CNT_
{
		volatile struct	/* 0x1A040E00 */
		{
				FIELD  TGCLK_SEL                             :  2;		/*  0.. 1, 0x00000003 */
				FIELD  CLKFL_POL                             :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  EXT_RST                               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  EXT_PWRDN                             :  1;		/*  5.. 5, 0x00000020 */
				FIELD  PAD_PCLK_INV                          :  1;		/*  6.. 6, 0x00000040 */
				FIELD  CAM_PCLK_INV                          :  1;		/*  7.. 7, 0x00000080 */
				FIELD  rsv_8                                 : 20;		/*  8..27, 0x0FFFFF00 */
				FIELD  CLKPOL                                :  1;		/* 28..28, 0x10000000 */
				FIELD  ADCLK_EN                              :  1;		/* 29..29, 0x20000000 */
				FIELD  rsv_30                                :  1;		/* 30..30, 0x40000000 */
				FIELD  PCEN                                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_PH_CNT;	/* SENINF_TG4_PH_CNT */

typedef volatile union _SENINF_REG_TG4_SEN_CK_
{
		volatile struct	/* 0x1A040E04 */
		{
				FIELD  CLKFL                                 :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CLKRS                                 :  6;		/*  8..13, 0x00003F00 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  CLKCNT                                :  6;		/* 16..21, 0x003F0000 */
				FIELD  rsv_22                                : 10;		/* 22..31, 0xFFC00000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_SEN_CK;	/* SENINF_TG4_SEN_CK */

typedef volatile union _SENINF_REG_TG4_TM_CTL_
{
		volatile struct	/* 0x1A040E08 */
		{
				FIELD  TM_EN                                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  TM_RST                                :  1;		/*  1.. 1, 0x00000002 */
				FIELD  TM_FMT                                :  1;		/*  2.. 2, 0x00000004 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  TM_PAT                                :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  TM_VSYNC                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  TM_DUMMYPXL                           :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_CTL;	/* SENINF_TG4_TM_CTL */

typedef volatile union _SENINF_REG_TG4_TM_SIZE_
{
		volatile struct	/* 0x1A040E0C */
		{
				FIELD  TM_PXL                                : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  TM_LINE                               : 14;		/* 16..29, 0x3FFF0000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_SIZE;	/* SENINF_TG4_TM_SIZE */

typedef volatile union _SENINF_REG_TG4_TM_CLK_
{
		volatile struct	/* 0x1A040E10 */
		{
				FIELD  TM_CLK_CNT                            :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 12;		/*  4..15, 0x0000FFF0 */
				FIELD  TM_CLRBAR_OFT                         : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  TM_CLRBAR_IDX                         :  3;		/* 28..30, 0x70000000 */
				FIELD  rsv_31                                :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_CLK;	/* SENINF_TG4_TM_CLK */

typedef volatile union _SENINF_REG_TG4_TM_STP_
{
		volatile struct	/* 0x1A040E14 */
		{
				FIELD  TG_TM_STP                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF_REG_TG4_TM_STP;	/* SENINF_TG4_TM_STP */

typedef volatile union _CCIR656_REG_CTL_
{
		volatile struct	/* 0x1A040EC0 */
		{
				FIELD  CCIR656_REV_0                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CCIR656_REV_1                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  CCIR656_HS_POL                        :  1;		/*  2.. 2, 0x00000004 */
				FIELD  CCIR656_VS_POL                        :  1;		/*  3.. 3, 0x00000008 */
				FIELD  CCIR656_PT_EN                         :  1;		/*  4.. 4, 0x00000010 */
				FIELD  CCIR656_EN                            :  1;		/*  5.. 5, 0x00000020 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  CCIR656_DBG_SEL                       :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_CTL;	/* CCIR656_CTL */

typedef volatile union _CCIR656_REG_H_
{
		volatile struct	/* 0x1A040EC4 */
		{
				FIELD  CCIR656_HS_START                      : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CCIR656_HS_END                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_H;	/* CCIR656_H */

typedef volatile union _CCIR656_REG_PTGEN_H_1_
{
		volatile struct	/* 0x1A040EC8 */
		{
				FIELD  CCIR656_PT_HTOTAL                     : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CCIR656_PT_HACTIVE                    : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_PTGEN_H_1;	/* CCIR656_PTGEN_H_1 */

typedef volatile union _CCIR656_REG_PTGEN_H_2_
{
		volatile struct	/* 0x1A040ECC */
		{
				FIELD  CCIR656_PT_HWIDTH                     : 13;		/*  0..12, 0x00001FFF */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  CCIR656_PT_HSTART                     : 13;		/* 16..28, 0x1FFF0000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_PTGEN_H_2;	/* CCIR656_PTGEN_H_2 */

typedef volatile union _CCIR656_REG_PTGEN_V_1_
{
		volatile struct	/* 0x1A040ED0 */
		{
				FIELD  CCIR656_PT_VTOTAL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CCIR656_PT_VACTIVE                    : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_PTGEN_V_1;	/* CCIR656_PTGEN_V_1 */

typedef volatile union _CCIR656_REG_PTGEN_V_2_
{
		volatile struct	/* 0x1A040ED4 */
		{
				FIELD  CCIR656_PT_VWIDTH                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  CCIR656_PT_VSTART                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_PTGEN_V_2;	/* CCIR656_PTGEN_V_2 */

typedef volatile union _CCIR656_REG_PTGEN_CTL1_
{
		volatile struct	/* 0x1A040ED8 */
		{
				FIELD  CCIR656_PT_TYPE                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCIR656_PT_COLOR_BAR_TH               : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_PTGEN_CTL1;	/* CCIR656_PTGEN_CTL1 */

typedef volatile union _CCIR656_REG_PTGEN_CTL2_
{
		volatile struct	/* 0x1A040EDC */
		{
				FIELD  CCIR656_PT_Y                          :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCIR656_PT_CB                         :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCIR656_PT_CR                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_PTGEN_CTL2;	/* CCIR656_PTGEN_CTL2 */

typedef volatile union _CCIR656_REG_PTGEN_CTL3_
{
		volatile struct	/* 0x1A040EE0 */
		{
				FIELD  CCIR656_PT_BD_Y                       :  8;		/*  0.. 7, 0x000000FF */
				FIELD  CCIR656_PT_BD_CB                      :  8;		/*  8..15, 0x0000FF00 */
				FIELD  CCIR656_PT_BD_CR                      :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_PTGEN_CTL3;	/* CCIR656_PTGEN_CTL3 */

typedef volatile union _CCIR656_REG_STATUS_
{
		volatile struct	/* 0x1A040EE4 */
		{
				FIELD  CCIR656_IN_FIELD                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  CCIR656_IN_VS                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}CCIR656_REG_STATUS;	/* CCIR656_STATUS */

typedef volatile union _SENINF5_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A041120 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_CTRL;	/* SENINF5_MUX_CTRL */

typedef volatile union _SENINF5_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A041124 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_INTEN;	/* SENINF5_MUX_INTEN */

typedef volatile union _SENINF5_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A041128 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_INTSTA;	/* SENINF5_MUX_INTSTA */

typedef volatile union _SENINF5_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A04112C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_SIZE;	/* SENINF5_MUX_SIZE */

typedef volatile union _SENINF5_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A041130 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_1;	/* SENINF5_MUX_DEBUG_1 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A041134 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_2;	/* SENINF5_MUX_DEBUG_2 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A041138 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_3;	/* SENINF5_MUX_DEBUG_3 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A04113C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_4;	/* SENINF5_MUX_DEBUG_4 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A041140 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_5;	/* SENINF5_MUX_DEBUG_5 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A041144 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_6;	/* SENINF5_MUX_DEBUG_6 */

typedef volatile union _SENINF5_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A041148 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DEBUG_7;	/* SENINF5_MUX_DEBUG_7 */

typedef volatile union _SENINF5_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A04114C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_SPARE;	/* SENINF5_MUX_SPARE */

typedef volatile union _SENINF5_REG_MUX_DATA_
{
		volatile struct	/* 0x1A041150 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DATA;	/* SENINF5_MUX_DATA */

typedef volatile union _SENINF5_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A041154 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_DATA_CNT;	/* SENINF5_MUX_DATA_CNT */

typedef volatile union _SENINF5_REG_MUX_CROP_
{
		volatile struct	/* 0x1A041158 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF5_REG_MUX_CROP;	/* SENINF5_MUX_CROP */

typedef volatile union _SENINF6_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A041520 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_CTRL;	/* SENINF6_MUX_CTRL */

typedef volatile union _SENINF6_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A041524 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_INTEN;	/* SENINF6_MUX_INTEN */

typedef volatile union _SENINF6_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A041528 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_INTSTA;	/* SENINF6_MUX_INTSTA */

typedef volatile union _SENINF6_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A04152C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_SIZE;	/* SENINF6_MUX_SIZE */

typedef volatile union _SENINF6_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A041530 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_1;	/* SENINF6_MUX_DEBUG_1 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A041534 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_2;	/* SENINF6_MUX_DEBUG_2 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A041538 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_3;	/* SENINF6_MUX_DEBUG_3 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A04153C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_4;	/* SENINF6_MUX_DEBUG_4 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A041540 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_5;	/* SENINF6_MUX_DEBUG_5 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A041544 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_6;	/* SENINF6_MUX_DEBUG_6 */

typedef volatile union _SENINF6_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A041548 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DEBUG_7;	/* SENINF6_MUX_DEBUG_7 */

typedef volatile union _SENINF6_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A04154C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_SPARE;	/* SENINF6_MUX_SPARE */

typedef volatile union _SENINF6_REG_MUX_DATA_
{
		volatile struct	/* 0x1A041550 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DATA;	/* SENINF6_MUX_DATA */

typedef volatile union _SENINF6_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A041554 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_DATA_CNT;	/* SENINF6_MUX_DATA_CNT */

typedef volatile union _SENINF6_REG_MUX_CROP_
{
		volatile struct	/* 0x1A041558 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF6_REG_MUX_CROP;	/* SENINF6_MUX_CROP */

typedef volatile union _SENINF7_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A041920 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_CTRL;	/* SENINF7_MUX_CTRL */

typedef volatile union _SENINF7_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A041924 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_INTEN;	/* SENINF7_MUX_INTEN */

typedef volatile union _SENINF7_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A041928 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_INTSTA;	/* SENINF7_MUX_INTSTA */

typedef volatile union _SENINF7_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A04192C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_SIZE;	/* SENINF7_MUX_SIZE */

typedef volatile union _SENINF7_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A041930 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_1;	/* SENINF7_MUX_DEBUG_1 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A041934 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_2;	/* SENINF7_MUX_DEBUG_2 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A041938 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_3;	/* SENINF7_MUX_DEBUG_3 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A04193C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_4;	/* SENINF7_MUX_DEBUG_4 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A041940 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_5;	/* SENINF7_MUX_DEBUG_5 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A041944 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_6;	/* SENINF7_MUX_DEBUG_6 */

typedef volatile union _SENINF7_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A041948 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DEBUG_7;	/* SENINF7_MUX_DEBUG_7 */

typedef volatile union _SENINF7_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A04194C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_SPARE;	/* SENINF7_MUX_SPARE */

typedef volatile union _SENINF7_REG_MUX_DATA_
{
		volatile struct	/* 0x1A041950 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DATA;	/* SENINF7_MUX_DATA */

typedef volatile union _SENINF7_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A041954 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_DATA_CNT;	/* SENINF7_MUX_DATA_CNT */

typedef volatile union _SENINF7_REG_MUX_CROP_
{
		volatile struct	/* 0x1A041958 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF7_REG_MUX_CROP;	/* SENINF7_MUX_CROP */

typedef volatile union _SENINF8_REG_MUX_CTRL_
{
		volatile struct	/* 0x1A041D20 */
		{
				FIELD  SENINF_MUX_SW_RST                     :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_IRQ_SW_RST                     :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  5;		/*  2.. 6, 0x0000007C */
				FIELD  SENINF_HSYNC_MASK                     :  1;		/*  7.. 7, 0x00000080 */
				FIELD  SENINF_PIX_SEL                        :  1;		/*  8.. 8, 0x00000100 */
				FIELD  SENINF_VSYNC_POL                      :  1;		/*  9.. 9, 0x00000200 */
				FIELD  SENINF_HSYNC_POL                      :  1;		/* 10..10, 0x00000400 */
				FIELD  OVERRUN_RST_EN                        :  1;		/* 11..11, 0x00000800 */
				FIELD  SENINF_SRC_SEL                        :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PUSH_EN                          :  6;		/* 16..21, 0x003F0000 */
				FIELD  FIFO_FLUSH_EN                         :  6;		/* 22..27, 0x0FC00000 */
				FIELD  FIFO_FULL_WR_EN                       :  2;		/* 28..29, 0x30000000 */
				FIELD  CROP_EN                               :  1;		/* 30..30, 0x40000000 */
				FIELD  SENINF_MUX_EN                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_CTRL;	/* SENINF8_MUX_CTRL */

typedef volatile union _SENINF8_REG_MUX_INTEN_
{
		volatile struct	/* 0x1A041D24 */
		{
				FIELD  SENINF_OVERRUN_IRQ_EN                 :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_EN                  :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_EN                  :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_EN                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_EN                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_EN         :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_EN         :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 24;		/*  7..30, 0x7FFFFF80 */
				FIELD  SENINF_IRQ_CLR_SEL                    :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_INTEN;	/* SENINF8_MUX_INTEN */

typedef volatile union _SENINF8_REG_MUX_INTSTA_
{
		volatile struct	/* 0x1A041D28 */
		{
				FIELD  SENINF_OVERRUN_IRQ_STA                :  1;		/*  0.. 0, 0x00000001 */
				FIELD  SENINF_CRCERR_IRQ_STA                 :  1;		/*  1.. 1, 0x00000002 */
				FIELD  SENINF_FSMERR_IRQ_STA                 :  1;		/*  2.. 2, 0x00000004 */
				FIELD  SENINF_VSIZEERR_IRQ_STA               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  SENINF_HSIZEERR_IRQ_STA               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  SENINF_SENSOR_VSIZEERR_IRQ_STA        :  1;		/*  5.. 5, 0x00000020 */
				FIELD  SENINF_SENSOR_HSIZEERR_IRQ_STA        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 : 25;		/*  7..31, 0xFFFFFF80 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_INTSTA;	/* SENINF8_MUX_INTSTA */

typedef volatile union _SENINF8_REG_MUX_SIZE_
{
		volatile struct	/* 0x1A041D2C */
		{
				FIELD  SENINF_VSIZE                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_HSIZE                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_SIZE;	/* SENINF8_MUX_SIZE */

typedef volatile union _SENINF8_REG_MUX_DEBUG_1_
{
		volatile struct	/* 0x1A041D30 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_1;	/* SENINF8_MUX_DEBUG_1 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_2_
{
		volatile struct	/* 0x1A041D34 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_2;	/* SENINF8_MUX_DEBUG_2 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_3_
{
		volatile struct	/* 0x1A041D38 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_3;	/* SENINF8_MUX_DEBUG_3 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_4_
{
		volatile struct	/* 0x1A041D3C */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_4;	/* SENINF8_MUX_DEBUG_4 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_5_
{
		volatile struct	/* 0x1A041D40 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_5;	/* SENINF8_MUX_DEBUG_5 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_6_
{
		volatile struct	/* 0x1A041D44 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_6;	/* SENINF8_MUX_DEBUG_6 */

typedef volatile union _SENINF8_REG_MUX_DEBUG_7_
{
		volatile struct	/* 0x1A041D48 */
		{
				FIELD  DEBUG_INFO                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DEBUG_7;	/* SENINF8_MUX_DEBUG_7 */

typedef volatile union _SENINF8_REG_MUX_SPARE_
{
		volatile struct	/* 0x1A041D4C */
		{
				FIELD  rsv_0                                 :  9;		/*  0.. 8, 0x000001FF */
				FIELD  SENINF_CRC_SEL                        :  2;		/*  9..10, 0x00000600 */
				FIELD  SENINF_VCNT_SEL                       :  2;		/* 11..12, 0x00001800 */
				FIELD  SENINF_FIFO_FULL_SEL                  :  1;		/* 13..13, 0x00002000 */
				FIELD  SENINF_SPARE                          :  6;		/* 14..19, 0x000FC000 */
				FIELD  rsv_20                                : 12;		/* 20..31, 0xFFF00000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_SPARE;	/* SENINF8_MUX_SPARE */

typedef volatile union _SENINF8_REG_MUX_DATA_
{
		volatile struct	/* 0x1A041D50 */
		{
				FIELD  SENINF_DATA0                          : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_DATA1                          : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DATA;	/* SENINF8_MUX_DATA */

typedef volatile union _SENINF8_REG_MUX_DATA_CNT_
{
		volatile struct	/* 0x1A041D54 */
		{
				FIELD  SENINF_DATA_CNT                       : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_DATA_CNT;	/* SENINF8_MUX_DATA_CNT */

typedef volatile union _SENINF8_REG_MUX_CROP_
{
		volatile struct	/* 0x1A041D58 */
		{
				FIELD  SENINF_CROP_X1                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  SENINF_CROP_X2                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}SENINF8_REG_MUX_CROP;	/* SENINF8_MUX_CROP */
#endif

typedef volatile union _SENINF1_REG_MUX_CTRL_EXT_
{
        volatile struct /* 0x1A040D3C */
        {
                FIELD  SENINF_SRC_SEL_EXT                    :  2;      /*  0.. 1, 0x00000003 */
                FIELD  rsv_2                                 :  2;      /*  2.. 3, 0x0000000C */
                FIELD  SENINF_PIX_SEL_EXT                    :  1;      /*  4.. 4, 0x00000010 */
                FIELD  rsv_5                                 : 27;      /*  5..31, 0xFFFFFFE0 */
        } Bits;
        UINT32 Raw;
}SENINF1_REG_MUX_CTRL_EXT;  /* SENINF1_MUX_CTRL_EXT */

#if 1
typedef volatile union _MIPI_REG_RX_ANA00_CSI0_
{
		volatile struct	/* 0x10217000 */
		{
				FIELD  RG_CSI0_LNRC_LDO_OUT_EN               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRC_INPUT_SEL                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRC_LPRX_IPLUS               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRC_LPRX_IMINUS              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRC_HSRX_IPLUS               :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRC_HSRX_IMINUS              :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRC_RT_CODE                  :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                :  3;		/* 16..18, 0x00070000 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI0;	/* MIPI_RX_ANA00_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA04_CSI0_
{
		volatile struct	/* 0x10217004 */
		{
				FIELD  RG_CSI0_LNRD0_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD0_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD0_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD0_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD0_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD0_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD0_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD0_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD0_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD0_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD0_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI0;	/* MIPI_RX_ANA04_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA08_CSI0_
{
		volatile struct	/* 0x10217008 */
		{
				FIELD  RG_CSI0_LNRD1_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD1_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD1_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD1_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD1_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD1_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD1_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD1_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD1_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD1_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD1_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI0;	/* MIPI_RX_ANA08_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI0_
{
		volatile struct	/* 0x1021700C */
		{
				FIELD  RG_CSI0_LNRD2_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD2_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD2_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD2_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD2_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD2_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD2_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD2_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD2_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD2_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD2_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI0;	/* MIPI_RX_ANA0C_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA10_CSI0_
{
		volatile struct	/* 0x10217010 */
		{
				FIELD  RG_CSI0_LNRD3_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD3_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD3_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD3_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD3_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD3_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD3_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD3_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD3_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD3_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD3_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI0;	/* MIPI_RX_ANA10_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA20_CSI0_
{
		volatile struct	/* 0x10217020 */
		{
				FIELD  RG_CSI0_LDO_CORE_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD_HSRX_BCLK_INVERT         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_BCLK_SEL                      :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0_HSDET_GATING_EN               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0_4XCLK_INVERT                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI0_4XCLK_DISABLE                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0_PRESERVE                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0_DELAY_TSTEN                   :  1;		/* 12..12, 0x00001000 */
				FIELD  RG_CSI0_DELAY_POSDIV                  :  1;		/* 13..13, 0x00002000 */
				FIELD  RG_CSI0_DELAY_LDOEN                   :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI0_SPARE                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA20_CSI0;	/* MIPI_RX_ANA20_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA24_CSI0_
{
		volatile struct	/* 0x10217024 */
		{
				FIELD  RG_CSI0_BG_CORE_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_BG_CKEN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_BG_DIV                        :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0_BG_FAST_CHARGE                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0_BG_R1_TRIM                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0_BG_R2_TRIM                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_V12_SEL                       :  3;		/* 16..18, 0x00070000 */
				FIELD  RG_CSI0_V10_SEL                       :  3;		/* 19..21, 0x00380000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0_V072_SEL                      :  3;		/* 24..26, 0x07000000 */
				FIELD  RG_CSI0_V04_SEL                       :  3;		/* 27..29, 0x38000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI0;	/* MIPI_RX_ANA24_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA28_CSI0_
{
		volatile struct	/* 0x10217028 */
		{
				FIELD  RG_CSI0_V032_SEL                      :  3;		/*  0.. 2, 0x00000007 */
				FIELD  RG_CSI0_V02_SEL                       :  3;		/*  3.. 5, 0x00000038 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA28_CSI0;	/* MIPI_RX_ANA28_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA30_CSI0_
{
		volatile struct	/* 0x10217030 */
		{
				FIELD  DA_CSI0_LNRD_HSRX_SYNC_INIT           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AD_CSI0_LNRD_HSDET                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  DA_CSI0_LNRC_LPRX_EN                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AD_CSI0_LNRC_LPRX_DP                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  AD_CSI0_LNRC_LPRX_DN                  :  1;		/*  6.. 6, 0x00000040 */
				FIELD  DA_CSI0_LNRC_HSRX_EN                  :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AD_CSI0_LNRC_HSRX_OFFSET_OUT          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DA_CSI0_LNRD0_LPRX_EN                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AD_CSI0_LNRD0_LPRX_DP                 :  1;		/* 10..10, 0x00000400 */
				FIELD  AD_CSI0_LNRD0_LPRX_DN                 :  1;		/* 11..11, 0x00000800 */
				FIELD  DA_CSI0_LNRD0_HSRX_EN                 :  1;		/* 12..12, 0x00001000 */
				FIELD  AD_CSI0_LNRD0_HSRX_OFFSET_OUT         :  1;		/* 13..13, 0x00002000 */
				FIELD  DA_CSI0_LNRD1_LPRX_EN                 :  1;		/* 14..14, 0x00004000 */
				FIELD  AD_CSI0_LNRD1_LPRX_DP                 :  1;		/* 15..15, 0x00008000 */
				FIELD  AD_CSI0_LNRD1_LPRX_DN                 :  1;		/* 16..16, 0x00010000 */
				FIELD  DA_CSI0_LNRD1_HSRX_EN                 :  1;		/* 17..17, 0x00020000 */
				FIELD  AD_CSI0_LNRD1_HSRX_OFFSET_OUT         :  1;		/* 18..18, 0x00040000 */
				FIELD  DA_CSI0_LNRD2_LPRX_EN                 :  1;		/* 19..19, 0x00080000 */
				FIELD  AD_CSI0_LNRD2_LPRX_DP                 :  1;		/* 20..20, 0x00100000 */
				FIELD  AD_CSI0_LNRD2_LPRX_DN                 :  1;		/* 21..21, 0x00200000 */
				FIELD  DA_CSI0_LNRD2_HSRX_EN                 :  1;		/* 22..22, 0x00400000 */
				FIELD  AD_CSI0_LNRD2_HSRX_OFFSET_OUT         :  1;		/* 23..23, 0x00800000 */
				FIELD  DA_CSI0_LNRD3_LPRX_EN                 :  1;		/* 24..24, 0x01000000 */
				FIELD  AD_CSI0_LNRD3_LPRX_DP                 :  1;		/* 25..25, 0x02000000 */
				FIELD  AD_CSI0_LNRD3_LPRX_DN                 :  1;		/* 26..26, 0x04000000 */
				FIELD  DA_CSI0_LNRD3_HSRX_EN                 :  1;		/* 27..27, 0x08000000 */
				FIELD  AD_CSI0_LNRD3_HSRX_OFFSET_OUT         :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA30_CSI0;	/* MIPI_RX_ANA30_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA44_CSI0_
{
		volatile struct	/* 0x10217044 */
		{
				FIELD  MIPI_RX_MON_MUX                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA44_CSI0;	/* MIPI_RX_ANA44_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA4C_CSI0_
{
		volatile struct	/* 0x1021704C */
		{
				FIELD  RG_MIPI_GPI0_IES                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_MIPI_GPI0_SMT                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_MIPI_GPI0_PU                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_MIPI_GPI0_PD                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_MIPI_GPI0_RDSEL                    :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_MIPI_GPI1_IES                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RG_MIPI_GPI1_SMT                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_MIPI_GPI1_PU                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_MIPI_GPI1_PD                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_MIPI_GPI1_RDSEL                    :  2;		/* 10..11, 0x00000C00 */
				FIELD  RG_MIPI_GPI2_IES                      :  1;		/* 12..12, 0x00001000 */
				FIELD  RG_MIPI_GPI2_SMT                      :  1;		/* 13..13, 0x00002000 */
				FIELD  RG_MIPI_GPI2_PU                       :  1;		/* 14..14, 0x00004000 */
				FIELD  RG_MIPI_GPI2_PD                       :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_MIPI_GPI2_RDSEL                    :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_MIPI_GPI3_IES                      :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_MIPI_GPI3_SMT                      :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_MIPI_GPI3_PU                       :  1;		/* 20..20, 0x00100000 */
				FIELD  RG_MIPI_GPI3_PD                       :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_MIPI_GPI3_RDSEL                    :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_MIPI_GPI4_IES                      :  1;		/* 24..24, 0x01000000 */
				FIELD  RG_MIPI_GPI4_SMT                      :  1;		/* 25..25, 0x02000000 */
				FIELD  RG_MIPI_GPI4_PU                       :  1;		/* 26..26, 0x04000000 */
				FIELD  RG_MIPI_GPI4_PD                       :  1;		/* 27..27, 0x08000000 */
				FIELD  RG_MIPI_GPI4_RDSEL                    :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA4C_CSI0;	/* MIPI_RX_ANA4C_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA50_CSI0_
{
		volatile struct	/* 0x10217050 */
		{
				FIELD  RG_MIPI_GPI5_IES                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_MIPI_GPI5_SMT                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_MIPI_GPI5_PU                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_MIPI_GPI5_PD                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_MIPI_GPI5_RDSEL                    :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_MIPI_GPI6_IES                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RG_MIPI_GPI6_SMT                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_MIPI_GPI6_PU                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_MIPI_GPI6_PD                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_MIPI_GPI6_RDSEL                    :  2;		/* 10..11, 0x00000C00 */
				FIELD  RG_MIPI_GPI7_IES                      :  1;		/* 12..12, 0x00001000 */
				FIELD  RG_MIPI_GPI7_SMT                      :  1;		/* 13..13, 0x00002000 */
				FIELD  RG_MIPI_GPI7_PU                       :  1;		/* 14..14, 0x00004000 */
				FIELD  RG_MIPI_GPI7_PD                       :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_MIPI_GPI7_RDSEL                    :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_MIPI_GPI8_IES                      :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_MIPI_GPI8_SMT                      :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_MIPI_GPI8_PU                       :  1;		/* 20..20, 0x00100000 */
				FIELD  RG_MIPI_GPI8_PD                       :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_MIPI_GPI8_RDSEL                    :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_MIPI_GPI9_IES                      :  1;		/* 24..24, 0x01000000 */
				FIELD  RG_MIPI_GPI9_SMT                      :  1;		/* 25..25, 0x02000000 */
				FIELD  RG_MIPI_GPI9_PU                       :  1;		/* 26..26, 0x04000000 */
				FIELD  RG_MIPI_GPI9_PD                       :  1;		/* 27..27, 0x08000000 */
				FIELD  RG_MIPI_GPI9_RDSEL                    :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA50_CSI0;	/* MIPI_RX_ANA50_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA54_CSI0_
{
		volatile struct	/* 0x10217054 */
		{
				FIELD  APB_ASYNC_CNT_EN                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  APB_ASYNC_ERR                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  APB_ASYNC_ERR_ADDR                    : 10;		/*  8..17, 0x0003FF00 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA54_CSI0;	/* MIPI_RX_ANA54_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA58_CSI0_
{
		volatile struct	/* 0x10217058 */
		{
				FIELD  csi0_byteclk_cnt                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA58_CSI0;	/* MIPI_RX_ANA58_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA78_CSI0_
{
		volatile struct	/* 0x10217078 */
		{
				FIELD  DA_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRC_HSRX_DELAY_CODE          :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA78_CSI0;	/* MIPI_RX_ANA78_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA7C_CSI0_
{
		volatile struct	/* 0x1021707C */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA7C_CSI0;	/* MIPI_RX_ANA7C_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA80_CSI0_
{
		volatile struct	/* 0x10217080 */
		{
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA80_CSI0;	/* MIPI_RX_ANA80_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA84_CSI0_
{
		volatile struct	/* 0x10217084 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA84_CSI0;	/* MIPI_RX_ANA84_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA88_CSI0_
{
		volatile struct	/* 0x10217088 */
		{
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA88_CSI0;	/* MIPI_RX_ANA88_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA8C_CSI0_
{
		volatile struct	/* 0x1021708C */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA8C_CSI0;	/* MIPI_RX_ANA8C_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA90_CSI0_
{
		volatile struct	/* 0x10217090 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA90_CSI0;	/* MIPI_RX_ANA90_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA94_CSI0_
{
		volatile struct	/* 0x10217094 */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA94_CSI0;	/* MIPI_RX_ANA94_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA98_CSI0_
{
		volatile struct	/* 0x10217098 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA98_CSI0;	/* MIPI_RX_ANA98_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAA0_CSI0_
{
		volatile struct	/* 0x102170A0 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA0_CSI0;	/* MIPI_RX_ANAA0_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAB0_CSI0_
{
		volatile struct	/* 0x102170B0 */
		{
				FIELD  Delay_APPLY_MODE                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DESKEW_SW_RST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DESKEW_TRIGGER_MODE                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DESKEW_ACC_MODE                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_CSI2_RST_ENABLE                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  DESKEW_IP_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  DESKEW_ENABLE                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAB0_CSI0;	/* MIPI_RX_ANAB0_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAB4_CSI0_
{
		volatile struct	/* 0x102170B4 */
		{
				FIELD  SYNC_CODE_MASK                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  EXPECTED_SYNC_CODE                    : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAB4_CSI0;	/* MIPI_RX_ANAB4_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAB8_CSI0_
{
		volatile struct	/* 0x102170B8 */
		{
				FIELD  DESKEW_SETUP_TIME                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_HOLD_TIME                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_TIME_OUT                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DESKEW_TIME_OUT_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAB8_CSI0;	/* MIPI_RX_ANAB8_CSI0 */

typedef volatile union _MIPI_REG_RX_ANABC_CSI0_
{
		volatile struct	/* 0x102170BC */
		{
				FIELD  DESKEW_DETECTION_MODE                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_DETECTION_CNT                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DESKEW_DELAY_APPLY_MODE               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  DESKEW_LANE_NUMBER                    :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANABC_CSI0;	/* MIPI_RX_ANABC_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAC0_CSI0_
{
		volatile struct	/* 0x102170C0 */
		{
				FIELD  DESKEW_INTERRUPT_ENABLE               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 23;		/*  8..30, 0x7FFFFF00 */
				FIELD  DESKEW_INTERRUPT_W1C_EN               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAC0_CSI0;	/* MIPI_RX_ANAC0_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAC4_CSI0_
{
		volatile struct	/* 0x102170C4 */
		{
				FIELD  DESKEW_INTERRUPT_STATUS               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAC4_CSI0;	/* MIPI_RX_ANAC4_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAC8_CSI0_
{
		volatile struct	/* 0x102170C8 */
		{
				FIELD  DESKEW_DEBUG_MUX_SELECT               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAC8_CSI0;	/* MIPI_RX_ANAC8_CSI0 */

typedef volatile union _MIPI_REG_RX_ANACC_CSI0_
{
		volatile struct	/* 0x102170CC */
		{
				FIELD  DESKEW_DEBUG_OUTPUTS                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANACC_CSI0;	/* MIPI_RX_ANACC_CSI0 */

typedef volatile union _MIPI_REG_RX_ANAD0_CSI0_
{
		volatile struct	/* 0x102170D0 */
		{
				FIELD  DESKEW_DELAY_LENGTH                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAD0_CSI0;	/* MIPI_RX_ANAD0_CSI0 */

typedef volatile union _MIPI_REG_RX_ANA00_CSI1_
{
		volatile struct	/* 0x10218000 */
		{
				FIELD  RG_CSI0_LNRC_LDO_OUT_EN               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRC_INPUT_SEL                :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRC_LPRX_IPLUS               :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRC_LPRX_IMINUS              :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRC_HSRX_IPLUS               :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRC_HSRX_IMINUS              :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRC_RT_CODE                  :  4;		/* 12..15, 0x0000F000 */
				FIELD  rsv_16                                :  3;		/* 16..18, 0x00070000 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  4;		/* 20..23, 0x00F00000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA00_CSI1;	/* MIPI_RX_ANA00_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA04_CSI1_
{
		volatile struct	/* 0x10218004 */
		{
				FIELD  RG_CSI0_LNRD0_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD0_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD0_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD0_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD0_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD0_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD0_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD0_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD0_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD0_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD0_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA04_CSI1;	/* MIPI_RX_ANA04_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA08_CSI1_
{
		volatile struct	/* 0x10218008 */
		{
				FIELD  RG_CSI0_LNRD1_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD1_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD1_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD1_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD1_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD1_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD1_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD1_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD1_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD1_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD1_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA08_CSI1;	/* MIPI_RX_ANA08_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA0C_CSI1_
{
		volatile struct	/* 0x1021800C */
		{
				FIELD  RG_CSI0_LNRD2_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD2_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD2_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD2_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD2_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD2_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD2_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD2_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD2_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD2_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD2_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA0C_CSI1;	/* MIPI_RX_ANA0C_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA10_CSI1_
{
		volatile struct	/* 0x10218010 */
		{
				FIELD  RG_CSI0_LNRD3_LDO_OUT_EN              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  2;		/*  1.. 2, 0x00000006 */
				FIELD  RG_CSI0_LNRD3_INPUT_SEL               :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  RG_CSI0_LNRD3_LPRX_IPLUS              :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_CSI0_LNRD3_LPRX_IMINUS             :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_CSI0_LNRD3_HSRX_IPLUS              :  1;		/* 10..10, 0x00000400 */
				FIELD  RG_CSI0_LNRD3_HSRX_IMINUS             :  1;		/* 11..11, 0x00000800 */
				FIELD  RG_CSI0_LNRD3_RT_CODE                 :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_LNRD3_HSRX_EDGE_SEL           :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_CSI0_LNRD3_HSRX_FORCE_SYNC         :  2;		/* 18..19, 0x000C0000 */
				FIELD  RG_CSI0_LNRD3_HSRX_DET_SEL            :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_CSI0_LNRD3_HSRX_BYPASS_SYNC        :  1;		/* 22..22, 0x00400000 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/* 23..23, 0x00800000 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  4;		/* 24..27, 0x0F000000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA10_CSI1;	/* MIPI_RX_ANA10_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA20_CSI1_
{
		volatile struct	/* 0x10218020 */
		{
				FIELD  RG_CSI0_LDO_CORE_EN                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD_HSRX_BCLK_INVERT         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_BCLK_SEL                      :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0_HSDET_GATING_EN               :  1;		/*  4.. 4, 0x00000010 */
				FIELD  RG_CSI0_4XCLK_INVERT                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  RG_CSI0_4XCLK_DISABLE                 :  1;		/*  6.. 6, 0x00000040 */
				FIELD  rsv_7                                 :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_CSI0_PRESERVE                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0_DELAY_TSTEN                   :  1;		/* 12..12, 0x00001000 */
				FIELD  RG_CSI0_DELAY_POSDIV                  :  1;		/* 13..13, 0x00002000 */
				FIELD  RG_CSI0_DELAY_LDOEN                   :  1;		/* 14..14, 0x00004000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_CSI0_SPARE                         : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA20_CSI1;	/* MIPI_RX_ANA20_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA24_CSI1_
{
		volatile struct	/* 0x10218024 */
		{
				FIELD  RG_CSI0_BG_CORE_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_BG_CKEN                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_BG_DIV                        :  2;		/*  2.. 3, 0x0000000C */
				FIELD  RG_CSI0_BG_FAST_CHARGE                :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  RG_CSI0_BG_R1_TRIM                    :  4;		/*  8..11, 0x00000F00 */
				FIELD  RG_CSI0_BG_R2_TRIM                    :  4;		/* 12..15, 0x0000F000 */
				FIELD  RG_CSI0_V12_SEL                       :  3;		/* 16..18, 0x00070000 */
				FIELD  RG_CSI0_V10_SEL                       :  3;		/* 19..21, 0x00380000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_CSI0_V072_SEL                      :  3;		/* 24..26, 0x07000000 */
				FIELD  RG_CSI0_V04_SEL                       :  3;		/* 27..29, 0x38000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA24_CSI1;	/* MIPI_RX_ANA24_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA28_CSI1_
{
		volatile struct	/* 0x10218028 */
		{
				FIELD  RG_CSI0_V032_SEL                      :  3;		/*  0.. 2, 0x00000007 */
				FIELD  RG_CSI0_V02_SEL                       :  3;		/*  3.. 5, 0x00000038 */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA28_CSI1;	/* MIPI_RX_ANA28_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA30_CSI1_
{
		volatile struct	/* 0x10218030 */
		{
				FIELD  DA_CSI0_LNRD_HSRX_SYNC_INIT           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  AD_CSI0_LNRD_HSDET                    :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  DA_CSI0_LNRC_LPRX_EN                  :  1;		/*  4.. 4, 0x00000010 */
				FIELD  AD_CSI0_LNRC_LPRX_DP                  :  1;		/*  5.. 5, 0x00000020 */
				FIELD  AD_CSI0_LNRC_LPRX_DN                  :  1;		/*  6.. 6, 0x00000040 */
				FIELD  DA_CSI0_LNRC_HSRX_EN                  :  1;		/*  7.. 7, 0x00000080 */
				FIELD  AD_CSI0_LNRC_HSRX_OFFSET_OUT          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  DA_CSI0_LNRD0_LPRX_EN                 :  1;		/*  9.. 9, 0x00000200 */
				FIELD  AD_CSI0_LNRD0_LPRX_DP                 :  1;		/* 10..10, 0x00000400 */
				FIELD  AD_CSI0_LNRD0_LPRX_DN                 :  1;		/* 11..11, 0x00000800 */
				FIELD  DA_CSI0_LNRD0_HSRX_EN                 :  1;		/* 12..12, 0x00001000 */
				FIELD  AD_CSI0_LNRD0_HSRX_OFFSET_OUT         :  1;		/* 13..13, 0x00002000 */
				FIELD  DA_CSI0_LNRD1_LPRX_EN                 :  1;		/* 14..14, 0x00004000 */
				FIELD  AD_CSI0_LNRD1_LPRX_DP                 :  1;		/* 15..15, 0x00008000 */
				FIELD  AD_CSI0_LNRD1_LPRX_DN                 :  1;		/* 16..16, 0x00010000 */
				FIELD  DA_CSI0_LNRD1_HSRX_EN                 :  1;		/* 17..17, 0x00020000 */
				FIELD  AD_CSI0_LNRD1_HSRX_OFFSET_OUT         :  1;		/* 18..18, 0x00040000 */
				FIELD  DA_CSI0_LNRD2_LPRX_EN                 :  1;		/* 19..19, 0x00080000 */
				FIELD  AD_CSI0_LNRD2_LPRX_DP                 :  1;		/* 20..20, 0x00100000 */
				FIELD  AD_CSI0_LNRD2_LPRX_DN                 :  1;		/* 21..21, 0x00200000 */
				FIELD  DA_CSI0_LNRD2_HSRX_EN                 :  1;		/* 22..22, 0x00400000 */
				FIELD  AD_CSI0_LNRD2_HSRX_OFFSET_OUT         :  1;		/* 23..23, 0x00800000 */
				FIELD  DA_CSI0_LNRD3_LPRX_EN                 :  1;		/* 24..24, 0x01000000 */
				FIELD  AD_CSI0_LNRD3_LPRX_DP                 :  1;		/* 25..25, 0x02000000 */
				FIELD  AD_CSI0_LNRD3_LPRX_DN                 :  1;		/* 26..26, 0x04000000 */
				FIELD  DA_CSI0_LNRD3_HSRX_EN                 :  1;		/* 27..27, 0x08000000 */
				FIELD  AD_CSI0_LNRD3_HSRX_OFFSET_OUT         :  1;		/* 28..28, 0x10000000 */
				FIELD  rsv_29                                :  3;		/* 29..31, 0xE0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA30_CSI1;	/* MIPI_RX_ANA30_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA44_CSI1_
{
		volatile struct	/* 0x10218044 */
		{
				FIELD  MIPI_RX_MON_MUX                       :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 : 28;		/*  4..31, 0xFFFFFFF0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA44_CSI1;	/* MIPI_RX_ANA44_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA4C_CSI1_
{
		volatile struct	/* 0x1021804C */
		{
				FIELD  RG_MIPI_GPI0_IES                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_MIPI_GPI0_SMT                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_MIPI_GPI0_PU                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_MIPI_GPI0_PD                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_MIPI_GPI0_RDSEL                    :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_MIPI_GPI1_IES                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RG_MIPI_GPI1_SMT                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_MIPI_GPI1_PU                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_MIPI_GPI1_PD                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_MIPI_GPI1_RDSEL                    :  2;		/* 10..11, 0x00000C00 */
				FIELD  RG_MIPI_GPI2_IES                      :  1;		/* 12..12, 0x00001000 */
				FIELD  RG_MIPI_GPI2_SMT                      :  1;		/* 13..13, 0x00002000 */
				FIELD  RG_MIPI_GPI2_PU                       :  1;		/* 14..14, 0x00004000 */
				FIELD  RG_MIPI_GPI2_PD                       :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_MIPI_GPI2_RDSEL                    :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_MIPI_GPI3_IES                      :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_MIPI_GPI3_SMT                      :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_MIPI_GPI3_PU                       :  1;		/* 20..20, 0x00100000 */
				FIELD  RG_MIPI_GPI3_PD                       :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_MIPI_GPI3_RDSEL                    :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_MIPI_GPI4_IES                      :  1;		/* 24..24, 0x01000000 */
				FIELD  RG_MIPI_GPI4_SMT                      :  1;		/* 25..25, 0x02000000 */
				FIELD  RG_MIPI_GPI4_PU                       :  1;		/* 26..26, 0x04000000 */
				FIELD  RG_MIPI_GPI4_PD                       :  1;		/* 27..27, 0x08000000 */
				FIELD  RG_MIPI_GPI4_RDSEL                    :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA4C_CSI1;	/* MIPI_RX_ANA4C_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA50_CSI1_
{
		volatile struct	/* 0x10218050 */
		{
				FIELD  RG_MIPI_GPI5_IES                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_MIPI_GPI5_SMT                      :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_MIPI_GPI5_PU                       :  1;		/*  2.. 2, 0x00000004 */
				FIELD  RG_MIPI_GPI5_PD                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  RG_MIPI_GPI5_RDSEL                    :  2;		/*  4.. 5, 0x00000030 */
				FIELD  RG_MIPI_GPI6_IES                      :  1;		/*  6.. 6, 0x00000040 */
				FIELD  RG_MIPI_GPI6_SMT                      :  1;		/*  7.. 7, 0x00000080 */
				FIELD  RG_MIPI_GPI6_PU                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  RG_MIPI_GPI6_PD                       :  1;		/*  9.. 9, 0x00000200 */
				FIELD  RG_MIPI_GPI6_RDSEL                    :  2;		/* 10..11, 0x00000C00 */
				FIELD  RG_MIPI_GPI7_IES                      :  1;		/* 12..12, 0x00001000 */
				FIELD  RG_MIPI_GPI7_SMT                      :  1;		/* 13..13, 0x00002000 */
				FIELD  RG_MIPI_GPI7_PU                       :  1;		/* 14..14, 0x00004000 */
				FIELD  RG_MIPI_GPI7_PD                       :  1;		/* 15..15, 0x00008000 */
				FIELD  RG_MIPI_GPI7_RDSEL                    :  2;		/* 16..17, 0x00030000 */
				FIELD  RG_MIPI_GPI8_IES                      :  1;		/* 18..18, 0x00040000 */
				FIELD  RG_MIPI_GPI8_SMT                      :  1;		/* 19..19, 0x00080000 */
				FIELD  RG_MIPI_GPI8_PU                       :  1;		/* 20..20, 0x00100000 */
				FIELD  RG_MIPI_GPI8_PD                       :  1;		/* 21..21, 0x00200000 */
				FIELD  RG_MIPI_GPI8_RDSEL                    :  2;		/* 22..23, 0x00C00000 */
				FIELD  RG_MIPI_GPI9_IES                      :  1;		/* 24..24, 0x01000000 */
				FIELD  RG_MIPI_GPI9_SMT                      :  1;		/* 25..25, 0x02000000 */
				FIELD  RG_MIPI_GPI9_PU                       :  1;		/* 26..26, 0x04000000 */
				FIELD  RG_MIPI_GPI9_PD                       :  1;		/* 27..27, 0x08000000 */
				FIELD  RG_MIPI_GPI9_RDSEL                    :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA50_CSI1;	/* MIPI_RX_ANA50_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA54_CSI1_
{
		volatile struct	/* 0x10218054 */
		{
				FIELD  APB_ASYNC_CNT_EN                      :  1;		/*  0.. 0, 0x00000001 */
				FIELD  APB_ASYNC_ERR                         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  6;		/*  2.. 7, 0x000000FC */
				FIELD  APB_ASYNC_ERR_ADDR                    : 10;		/*  8..17, 0x0003FF00 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA54_CSI1;	/* MIPI_RX_ANA54_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA58_CSI1_
{
		volatile struct	/* 0x10218058 */
		{
				FIELD  csi0_byteclk_cnt                      : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA58_CSI1;	/* MIPI_RX_ANA58_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA78_CSI1_
{
		volatile struct	/* 0x10218078 */
		{
				FIELD  DA_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRC_HSRX_DELAY_CODE          :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA78_CSI1;	/* MIPI_RX_ANA78_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA7C_CSI1_
{
		volatile struct	/* 0x1021807C */
		{
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD0_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA7C_CSI1;	/* MIPI_RX_ANA7C_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA80_CSI1_
{
		volatile struct	/* 0x10218080 */
		{
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD1_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA80_CSI1;	/* MIPI_RX_ANA80_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA84_CSI1_
{
		volatile struct	/* 0x10218084 */
		{
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD2_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA84_CSI1;	/* MIPI_RX_ANA84_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA88_CSI1_
{
		volatile struct	/* 0x10218088 */
		{
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  DA_CSI0_LNRD3_HSRX_DELAY_CODE         :  4;		/*  2.. 5, 0x0000003C */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA88_CSI1;	/* MIPI_RX_ANA88_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA8C_CSI1_
{
		volatile struct	/* 0x1021808C */
		{
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD0_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA8C_CSI1;	/* MIPI_RX_ANA8C_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA90_CSI1_
{
		volatile struct	/* 0x10218090 */
		{
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD1_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA90_CSI1;	/* MIPI_RX_ANA90_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA94_CSI1_
{
		volatile struct	/* 0x10218094 */
		{
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD2_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA94_CSI1;	/* MIPI_RX_ANA94_CSI1 */

typedef volatile union _MIPI_REG_RX_ANA98_CSI1_
{
		volatile struct	/* 0x10218098 */
		{
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_EN           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_APPLY        :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRD3_HSRX_DELAY_CODE         :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANA98_CSI1;	/* MIPI_RX_ANA98_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAA0_CSI1_
{
		volatile struct	/* 0x102180A0 */
		{
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_EN            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_APPLY         :  1;		/*  1.. 1, 0x00000002 */
				FIELD  RG_CSI0_LNRC_HSRX_DELAY_CODE          :  6;		/*  2.. 7, 0x000000FC */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAA0_CSI1;	/* MIPI_RX_ANAA0_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAB0_CSI1_
{
		volatile struct	/* 0x102180B0 */
		{
				FIELD  Delay_APPLY_MODE                      :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  3;		/*  4.. 6, 0x00000070 */
				FIELD  DESKEW_SW_RST                         :  1;		/*  7.. 7, 0x00000080 */
				FIELD  DESKEW_TRIGGER_MODE                   :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  DESKEW_ACC_MODE                       :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_CSI2_RST_ENABLE                :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 13;		/* 17..29, 0x3FFE0000 */
				FIELD  DESKEW_IP_SEL                         :  1;		/* 30..30, 0x40000000 */
				FIELD  DESKEW_ENABLE                         :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAB0_CSI1;	/* MIPI_RX_ANAB0_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAB4_CSI1_
{
		volatile struct	/* 0x102180B4 */
		{
				FIELD  SYNC_CODE_MASK                        : 16;		/*  0..15, 0x0000FFFF */
				FIELD  EXPECTED_SYNC_CODE                    : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAB4_CSI1;	/* MIPI_RX_ANAB4_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAB8_CSI1_
{
		volatile struct	/* 0x102180B8 */
		{
				FIELD  DESKEW_SETUP_TIME                     :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_HOLD_TIME                      :  4;		/*  8..11, 0x00000F00 */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  DESKEW_TIME_OUT                       :  8;		/* 16..23, 0x00FF0000 */
				FIELD  DESKEW_TIME_OUT_EN                    :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  7;		/* 25..31, 0xFE000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAB8_CSI1;	/* MIPI_RX_ANAB8_CSI1 */

typedef volatile union _MIPI_REG_RX_ANABC_CSI1_
{
		volatile struct	/* 0x102180BC */
		{
				FIELD  DESKEW_DETECTION_MODE                 :  4;		/*  0.. 3, 0x0000000F */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  DESKEW_DETECTION_CNT                  :  7;		/*  8..14, 0x00007F00 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  DESKEW_DELAY_APPLY_MODE               :  4;		/* 16..19, 0x000F0000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  DESKEW_LANE_NUMBER                    :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANABC_CSI1;	/* MIPI_RX_ANABC_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAC0_CSI1_
{
		volatile struct	/* 0x102180C0 */
		{
				FIELD  DESKEW_INTERRUPT_ENABLE               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 23;		/*  8..30, 0x7FFFFF00 */
				FIELD  DESKEW_INTERRUPT_W1C_EN               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAC0_CSI1;	/* MIPI_RX_ANAC0_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAC4_CSI1_
{
		volatile struct	/* 0x102180C4 */
		{
				FIELD  DESKEW_INTERRUPT_STATUS               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAC4_CSI1;	/* MIPI_RX_ANAC4_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAC8_CSI1_
{
		volatile struct	/* 0x102180C8 */
		{
				FIELD  DESKEW_DEBUG_MUX_SELECT               :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 : 24;		/*  8..31, 0xFFFFFF00 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAC8_CSI1;	/* MIPI_RX_ANAC8_CSI1 */

typedef volatile union _MIPI_REG_RX_ANACC_CSI1_
{
		volatile struct	/* 0x102180CC */
		{
				FIELD  DESKEW_DEBUG_OUTPUTS                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANACC_CSI1;	/* MIPI_RX_ANACC_CSI1 */

typedef volatile union _MIPI_REG_RX_ANAD0_CSI1_
{
		volatile struct	/* 0x102180D0 */
		{
				FIELD  DESKEW_DELAY_LENGTH                   :  6;		/*  0.. 5, 0x0000003F */
				FIELD  rsv_6                                 : 26;		/*  6..31, 0xFFFFFFC0 */
		} Bits;
		UINT32 Raw;
}MIPI_REG_RX_ANAD0_CSI1;	/* MIPI_RX_ANAD0_CSI1 */
#endif

#if 1
typedef volatile struct _seninf_reg_t_	/* 0x1A040000..0x1A041DAB */
{
	SENINF_REG_TOP_CTRL                             SENINF_TOP_CTRL;                                 /* 0000, 0x1A040000 */
	SENINF_REG_TOP_CMODEL_PAR                       SENINF_TOP_CMODEL_PAR;                           /* 0004, 0x1A040004 */
	SENINF_REG_TOP_MUX_CTRL                         SENINF_TOP_MUX_CTRL;                             /* 0008, 0x1A040008 */
	UINT32                                          rsv_000C[45];                                    /* 000C..00BF, 0x1A04000C..1A0400BF */
	SENINF_REG_N3D_CTL                              SENINF_N3D_CTL;                                  /* 00C0, 0x1A0400C0 */
	SENINF_REG_N3D_POS                              SENINF_N3D_POS;                                  /* 00C4, 0x1A0400C4 */
	SENINF_REG_N3D_TRIG                             SENINF_N3D_TRIG;                                 /* 00C8, 0x1A0400C8 */
	SENINF_REG_N3D_INT                              SENINF_N3D_INT;                                  /* 00CC, 0x1A0400CC */
	SENINF_REG_N3D_CNT0                             SENINF_N3D_CNT0;                                 /* 00D0, 0x1A0400D0 */
	SENINF_REG_N3D_CNT1                             SENINF_N3D_CNT1;                                 /* 00D4, 0x1A0400D4 */
	SENINF_REG_N3D_DBG                              SENINF_N3D_DBG;                                  /* 00D8, 0x1A0400D8 */
	SENINF_REG_N3D_DIFF_THR                         SENINF_N3D_DIFF_THR;                             /* 00DC, 0x1A0400DC */
	SENINF_REG_N3D_DIFF_CNT                         SENINF_N3D_DIFF_CNT;                             /* 00E0, 0x1A0400E0 */
	SENINF_REG_N3D_DBG_1                            SENINF_N3D_DBG_1;                                /* 00E4, 0x1A0400E4 */
	SENINF_REG_N3D_VALID_TG_CNT                     SENINF_N3D_VALID_TG_CNT;                         /* 00E8, 0x1A0400E8 */
	SENINF_REG_N3D_SYNC_A_PERIOD                    SENINF_N3D_SYNC_A_PERIOD;                        /* 00EC, 0x1A0400EC */
	SENINF_REG_N3D_SYNC_B_PERIOD                    SENINF_N3D_SYNC_B_PERIOD;                        /* 00F0, 0x1A0400F0 */
	SENINF_REG_N3D_SYNC_A_PULSE_LEN                 SENINF_N3D_SYNC_A_PULSE_LEN;                     /* 00F4, 0x1A0400F4 */
	SENINF_REG_N3D_SYNC_B_PULSE_LEN                 SENINF_N3D_SYNC_B_PULSE_LEN;                     /* 00F8, 0x1A0400F8 */
	UINT32                                          rsv_00FC;                                        /* 00FC, 0x1A0400FC */
	SENINF1_REG_CTRL                                SENINF1_CTRL;                                    /* 0100, 0x1A040100 */
	UINT32                                          rsv_0104[7];                                     /* 0104..011F, 0x1A040104..1A04011F */
	SENINF1_REG_MUX_CTRL                            SENINF1_MUX_CTRL;                                /* 0120, 0x1A040120 */
	SENINF1_REG_MUX_INTEN                           SENINF1_MUX_INTEN;                               /* 0124, 0x1A040124 */
	SENINF1_REG_MUX_INTSTA                          SENINF1_MUX_INTSTA;                              /* 0128, 0x1A040128 */
	SENINF1_REG_MUX_SIZE                            SENINF1_MUX_SIZE;                                /* 012C, 0x1A04012C */
	SENINF1_REG_MUX_DEBUG_1                         SENINF1_MUX_DEBUG_1;                             /* 0130, 0x1A040130 */
	SENINF1_REG_MUX_DEBUG_2                         SENINF1_MUX_DEBUG_2;                             /* 0134, 0x1A040134 */
	SENINF1_REG_MUX_DEBUG_3                         SENINF1_MUX_DEBUG_3;                             /* 0138, 0x1A040138 */
	SENINF1_REG_MUX_DEBUG_4                         SENINF1_MUX_DEBUG_4;                             /* 013C, 0x1A04013C */
	SENINF1_REG_MUX_DEBUG_5                         SENINF1_MUX_DEBUG_5;                             /* 0140, 0x1A040140 */
	SENINF1_REG_MUX_DEBUG_6                         SENINF1_MUX_DEBUG_6;                             /* 0144, 0x1A040144 */
	SENINF1_REG_MUX_DEBUG_7                         SENINF1_MUX_DEBUG_7;                             /* 0148, 0x1A040148 */
	SENINF1_REG_MUX_SPARE                           SENINF1_MUX_SPARE;                               /* 014C, 0x1A04014C */
	SENINF1_REG_MUX_DATA                            SENINF1_MUX_DATA;                                /* 0150, 0x1A040150 */
	SENINF1_REG_MUX_DATA_CNT                        SENINF1_MUX_DATA_CNT;                            /* 0154, 0x1A040154 */
	SENINF1_REG_MUX_CROP                            SENINF1_MUX_CROP;                                /* 0158, 0x1A040158 */
	UINT32                                          rsv_015C[41];                                    /* 015C..01FF, 0x1A04015C..1A0401FF */
	SENINF_REG_TG1_PH_CNT                           SENINF_TG1_PH_CNT;                               /* 0200, 0x1A040200 */
	SENINF_REG_TG1_SEN_CK                           SENINF_TG1_SEN_CK;                               /* 0204, 0x1A040204 */
	SENINF_REG_TG1_TM_CTL                           SENINF_TG1_TM_CTL;                               /* 0208, 0x1A040208 */
	SENINF_REG_TG1_TM_SIZE                          SENINF_TG1_TM_SIZE;                              /* 020C, 0x1A04020C */
	SENINF_REG_TG1_TM_CLK                           SENINF_TG1_TM_CLK;                               /* 0210, 0x1A040210 */
	SENINF_REG_TG1_TM_STP                           SENINF_TG1_TM_STP;                               /* 0214, 0x1A040214 */
	UINT32                                          rsv_0218[58];                                    /* 0218..02FF, 0x1A040218..1A0402FF */
	MIPI_REG_RX_CON00_CSI0                          MIPI_RX_CON00_CSI0;                              /* 0300, 0x1A040300 */
	MIPI_REG_RX_CON04_CSI0                          MIPI_RX_CON04_CSI0;                              /* 0304, 0x1A040304 */
	MIPI_REG_RX_CON08_CSI0                          MIPI_RX_CON08_CSI0;                              /* 0308, 0x1A040308 */
	MIPI_REG_RX_CON0C_CSI0                          MIPI_RX_CON0C_CSI0;                              /* 030C, 0x1A04030C */
	MIPI_REG_RX_CON10_CSI0                          MIPI_RX_CON10_CSI0;                              /* 0310, 0x1A040310 */
	UINT32                                          rsv_0314[4];                                     /* 0314..0323, 0x1A040314..1A040323 */
	MIPI_REG_RX_CON24_CSI0                          MIPI_RX_CON24_CSI0;                              /* 0324, 0x1A040324 */
	MIPI_REG_RX_CON28_CSI0                          MIPI_RX_CON28_CSI0;                              /* 0328, 0x1A040328 */
	UINT32                                          rsv_032C[2];                                     /* 032C..0333, 0x1A04032C..1A040333 */
	MIPI_REG_RX_CON34_CSI0                          MIPI_RX_CON34_CSI0;                              /* 0334, 0x1A040334 */
	MIPI_REG_RX_CON38_CSI0                          MIPI_RX_CON38_CSI0;                              /* 0338, 0x1A040338 */
	MIPI_REG_RX_CON3C_CSI0                          MIPI_RX_CON3C_CSI0;                              /* 033C, 0x1A04033C */
	MIPI_REG_RX_CON40_CSI0                          MIPI_RX_CON40_CSI0;                              /* 0340, 0x1A040340 */
	MIPI_REG_RX_CON44_CSI0                          MIPI_RX_CON44_CSI0;                              /* 0344, 0x1A040344 */
	MIPI_REG_RX_CON48_CSI0                          MIPI_RX_CON48_CSI0;                              /* 0348, 0x1A040348 */
	UINT32                                          rsv_034C;                                        /* 034C, 0x1A04034C */
	MIPI_REG_RX_CON50_CSI0                          MIPI_RX_CON50_CSI0;                              /* 0350, 0x1A040350 */
	UINT32                                          rsv_0354[3];                                     /* 0354..035F, 0x1A040354..1A04035F */
	SENINF1_REG_CSI2_CTRL                           SENINF1_CSI2_CTRL;                               /* 0360, 0x1A040360 */
	SENINF1_REG_CSI2_DELAY                          SENINF1_CSI2_DELAY;                              /* 0364, 0x1A040364 */
	SENINF1_REG_CSI2_INTEN                          SENINF1_CSI2_INTEN;                              /* 0368, 0x1A040368 */
	SENINF1_REG_CSI2_INTSTA                         SENINF1_CSI2_INTSTA;                             /* 036C, 0x1A04036C */
	SENINF1_REG_CSI2_ECCDBG                         SENINF1_CSI2_ECCDBG;                             /* 0370, 0x1A040370 */
	SENINF1_REG_CSI2_CRCDBG                         SENINF1_CSI2_CRCDBG;                             /* 0374, 0x1A040374 */
	SENINF1_REG_CSI2_DBG                            SENINF1_CSI2_DBG;                                /* 0378, 0x1A040378 */
	SENINF1_REG_CSI2_VER                            SENINF1_CSI2_VER;                                /* 037C, 0x1A04037C */
	SENINF1_REG_CSI2_SHORT_INFO                     SENINF1_CSI2_SHORT_INFO;                         /* 0380, 0x1A040380 */
	SENINF1_REG_CSI2_LNFSM                          SENINF1_CSI2_LNFSM;                              /* 0384, 0x1A040384 */
	SENINF1_REG_CSI2_LNMUX                          SENINF1_CSI2_LNMUX;                              /* 0388, 0x1A040388 */
	SENINF1_REG_CSI2_HSYNC_CNT                      SENINF1_CSI2_HSYNC_CNT;                          /* 038C, 0x1A04038C */
	SENINF1_REG_CSI2_CAL                            SENINF1_CSI2_CAL;                                /* 0390, 0x1A040390 */
	SENINF1_REG_CSI2_DS                             SENINF1_CSI2_DS;                                 /* 0394, 0x1A040394 */
	SENINF1_REG_CSI2_VS                             SENINF1_CSI2_VS;                                 /* 0398, 0x1A040398 */
	SENINF1_REG_CSI2_BIST                           SENINF1_CSI2_BIST;                               /* 039C, 0x1A04039C */
	SENINF1_REG_NCSI2_CTL                           SENINF1_NCSI2_CTL;                               /* 03A0, 0x1A0403A0 */
	SENINF1_REG_NCSI2_LNRC_TIMING                   SENINF1_NCSI2_LNRC_TIMING;                       /* 03A4, 0x1A0403A4 */
	SENINF1_REG_NCSI2_LNRD_TIMING                   SENINF1_NCSI2_LNRD_TIMING;                       /* 03A8, 0x1A0403A8 */
	SENINF1_REG_NCSI2_DPCM                          SENINF1_NCSI2_DPCM;                              /* 03AC, 0x1A0403AC */
	SENINF1_REG_NCSI2_INT_EN                        SENINF1_NCSI2_INT_EN;                            /* 03B0, 0x1A0403B0 */
	SENINF1_REG_NCSI2_INT_STATUS                    SENINF1_NCSI2_INT_STATUS;                        /* 03B4, 0x1A0403B4 */
	SENINF1_REG_NCSI2_DGB_SEL                       SENINF1_NCSI2_DGB_SEL;                           /* 03B8, 0x1A0403B8 */
	SENINF1_REG_NCSI2_DBG_PORT                      SENINF1_NCSI2_DBG_PORT;                          /* 03BC, 0x1A0403BC */
	SENINF1_REG_NCSI2_SPARE0                        SENINF1_NCSI2_SPARE0;                            /* 03C0, 0x1A0403C0 */
	SENINF1_REG_NCSI2_SPARE1                        SENINF1_NCSI2_SPARE1;                            /* 03C4, 0x1A0403C4 */
	SENINF1_REG_NCSI2_LNRC_FSM                      SENINF1_NCSI2_LNRC_FSM;                          /* 03C8, 0x1A0403C8 */
	SENINF1_REG_NCSI2_LNRD_FSM                      SENINF1_NCSI2_LNRD_FSM;                          /* 03CC, 0x1A0403CC */
	SENINF1_REG_NCSI2_FRAME_LINE_NUM                SENINF1_NCSI2_FRAME_LINE_NUM;                    /* 03D0, 0x1A0403D0 */
	SENINF1_REG_NCSI2_GENERIC_SHORT                 SENINF1_NCSI2_GENERIC_SHORT;                     /* 03D4, 0x1A0403D4 */
	SENINF1_REG_NCSI2_HSRX_DBG                      SENINF1_NCSI2_HSRX_DBG;                          /* 03D8, 0x1A0403D8 */
	SENINF1_REG_NCSI2_DI                            SENINF1_NCSI2_DI;                                /* 03DC, 0x1A0403DC */
	SENINF1_REG_NCSI2_HS_TRAIL                      SENINF1_NCSI2_HS_TRAIL;                          /* 03E0, 0x1A0403E0 */
	SENINF1_REG_NCSI2_DI_CTRL                       SENINF1_NCSI2_DI_CTRL;                           /* 03E4, 0x1A0403E4 */
	SENINF1_REG_NCSI2_DI_1                          SENINF1_NCSI2_DI_1;                              /* 03E8, 0x1A0403E8 */
	SENINF1_REG_NCSI2_DI_CTRL_1                     SENINF1_NCSI2_DI_CTRL_1;                         /* 03EC, 0x1A0403EC */
	SENINF1_REG_NCSI2_RAW10_DI_CTRL                 SENINF1_NCSI2_RAW10_DI_CTRL;                     /* 03F0, 0x1A0403F0 */
	SENINF1_REG_NCSI2_RAW12_DI_CTRL                 SENINF1_NCSI2_RAW12_DI_CTRL;                     /* 03F4, 0x1A0403F4 */
	SENINF1_REG_NCSI2_RAW14_DI_CTRL                 SENINF1_NCSI2_RAW14_DI_CTRL;                     /* 03F8, 0x1A0403F8 */
	UINT32                                          rsv_03FC[65];                                    /* 03FC..04FF, 0x1A0403FC..1A0404FF */
	SENINF2_REG_CTRL                                SENINF2_CTRL;                                    /* 0500, 0x1A040500 */
	UINT32                                          rsv_0504[7];                                     /* 0504..051F, 0x1A040504..1A04051F */
	SENINF2_REG_MUX_CTRL                            SENINF2_MUX_CTRL;                                /* 0520, 0x1A040520 */
	SENINF2_REG_MUX_INTEN                           SENINF2_MUX_INTEN;                               /* 0524, 0x1A040524 */
	SENINF2_REG_MUX_INTSTA                          SENINF2_MUX_INTSTA;                              /* 0528, 0x1A040528 */
	SENINF2_REG_MUX_SIZE                            SENINF2_MUX_SIZE;                                /* 052C, 0x1A04052C */
	SENINF2_REG_MUX_DEBUG_1                         SENINF2_MUX_DEBUG_1;                             /* 0530, 0x1A040530 */
	SENINF2_REG_MUX_DEBUG_2                         SENINF2_MUX_DEBUG_2;                             /* 0534, 0x1A040534 */
	SENINF2_REG_MUX_DEBUG_3                         SENINF2_MUX_DEBUG_3;                             /* 0538, 0x1A040538 */
	SENINF2_REG_MUX_DEBUG_4                         SENINF2_MUX_DEBUG_4;                             /* 053C, 0x1A04053C */
	SENINF2_REG_MUX_DEBUG_5                         SENINF2_MUX_DEBUG_5;                             /* 0540, 0x1A040540 */
	SENINF2_REG_MUX_DEBUG_6                         SENINF2_MUX_DEBUG_6;                             /* 0544, 0x1A040544 */
	SENINF2_REG_MUX_DEBUG_7                         SENINF2_MUX_DEBUG_7;                             /* 0548, 0x1A040548 */
	SENINF2_REG_MUX_SPARE                           SENINF2_MUX_SPARE;                               /* 054C, 0x1A04054C */
	SENINF2_REG_MUX_DATA                            SENINF2_MUX_DATA;                                /* 0550, 0x1A040550 */
	SENINF2_REG_MUX_DATA_CNT                        SENINF2_MUX_DATA_CNT;                            /* 0554, 0x1A040554 */
	SENINF2_REG_MUX_CROP                            SENINF2_MUX_CROP;                                /* 0558, 0x1A040558 */
	UINT32                                          rsv_055C[41];                                    /* 055C..05FF, 0x1A04055C..1A0405FF */
	SENINF_REG_TG2_PH_CNT                           SENINF_TG2_PH_CNT;                               /* 0600, 0x1A040600 */
	SENINF_REG_TG2_SEN_CK                           SENINF_TG2_SEN_CK;                               /* 0604, 0x1A040604 */
	SENINF_REG_TG2_TM_CTL                           SENINF_TG2_TM_CTL;                               /* 0608, 0x1A040608 */
	SENINF_REG_TG2_TM_SIZE                          SENINF_TG2_TM_SIZE;                              /* 060C, 0x1A04060C */
	SENINF_REG_TG2_TM_CLK                           SENINF_TG2_TM_CLK;                               /* 0610, 0x1A040610 */
	SENINF_REG_TG2_TM_STP                           SENINF_TG2_TM_STP;                               /* 0614, 0x1A040614 */
	UINT32                                          rsv_0618[58];                                    /* 0618..06FF, 0x1A040618..1A0406FF */
	MIPI_REG_RX_CON00_CSI1                          MIPI_RX_CON00_CSI1;                              /* 0700, 0x1A040700 */
	MIPI_REG_RX_CON04_CSI1                          MIPI_RX_CON04_CSI1;                              /* 0704, 0x1A040704 */
	MIPI_REG_RX_CON08_CSI1                          MIPI_RX_CON08_CSI1;                              /* 0708, 0x1A040708 */
	MIPI_REG_RX_CON0C_CSI1                          MIPI_RX_CON0C_CSI1;                              /* 070C, 0x1A04070C */
	MIPI_REG_RX_CON10_CSI1                          MIPI_RX_CON10_CSI1;                              /* 0710, 0x1A040710 */
	UINT32                                          rsv_0714[4];                                     /* 0714..0723, 0x1A040714..1A040723 */
	MIPI_REG_RX_CON24_CSI1                          MIPI_RX_CON24_CSI1;                              /* 0724, 0x1A040724 */
	MIPI_REG_RX_CON28_CSI1                          MIPI_RX_CON28_CSI1;                              /* 0728, 0x1A040728 */
	UINT32                                          rsv_072C[2];                                     /* 072C..0733, 0x1A04072C..1A040733 */
	MIPI_REG_RX_CON34_CSI1                          MIPI_RX_CON34_CSI1;                              /* 0734, 0x1A040734 */
	MIPI_REG_RX_CON38_CSI1                          MIPI_RX_CON38_CSI1;                              /* 0738, 0x1A040738 */
	MIPI_REG_RX_CON3C_CSI1                          MIPI_RX_CON3C_CSI1;                              /* 073C, 0x1A04073C */
	MIPI_REG_RX_CON40_CSI1                          MIPI_RX_CON40_CSI1;                              /* 0740, 0x1A040740 */
	MIPI_REG_RX_CON44_CSI1                          MIPI_RX_CON44_CSI1;                              /* 0744, 0x1A040744 */
	MIPI_REG_RX_CON48_CSI1                          MIPI_RX_CON48_CSI1;                              /* 0748, 0x1A040748 */
	UINT32                                          rsv_074C;                                        /* 074C, 0x1A04074C */
	MIPI_REG_RX_CON50_CSI1                          MIPI_RX_CON50_CSI1;                              /* 0750, 0x1A040750 */
	UINT32                                          rsv_0754[3];                                     /* 0754..075F, 0x1A040754..1A04075F */
	SENINF2_REG_CSI2_CTRL                           SENINF2_CSI2_CTRL;                               /* 0760, 0x1A040760 */
	SENINF2_REG_CSI2_DELAY                          SENINF2_CSI2_DELAY;                              /* 0764, 0x1A040764 */
	SENINF2_REG_CSI2_INTEN                          SENINF2_CSI2_INTEN;                              /* 0768, 0x1A040768 */
	SENINF2_REG_CSI2_INTSTA                         SENINF2_CSI2_INTSTA;                             /* 076C, 0x1A04076C */
	SENINF2_REG_CSI2_ECCDBG                         SENINF2_CSI2_ECCDBG;                             /* 0770, 0x1A040770 */
	SENINF2_REG_CSI2_CRCDBG                         SENINF2_CSI2_CRCDBG;                             /* 0774, 0x1A040774 */
	SENINF2_REG_CSI2_DBG                            SENINF2_CSI2_DBG;                                /* 0778, 0x1A040778 */
	SENINF2_REG_CSI2_VER                            SENINF2_CSI2_VER;                                /* 077C, 0x1A04077C */
	SENINF2_REG_CSI2_SHORT_INFO                     SENINF2_CSI2_SHORT_INFO;                         /* 0780, 0x1A040780 */
	SENINF2_REG_CSI2_LNFSM                          SENINF2_CSI2_LNFSM;                              /* 0784, 0x1A040784 */
	SENINF2_REG_CSI2_LNMUX                          SENINF2_CSI2_LNMUX;                              /* 0788, 0x1A040788 */
	SENINF2_REG_CSI2_HSYNC_CNT                      SENINF2_CSI2_HSYNC_CNT;                          /* 078C, 0x1A04078C */
	SENINF2_REG_CSI2_CAL                            SENINF2_CSI2_CAL;                                /* 0790, 0x1A040790 */
	SENINF2_REG_CSI2_DS                             SENINF2_CSI2_DS;                                 /* 0794, 0x1A040794 */
	SENINF2_REG_CSI2_VS                             SENINF2_CSI2_VS;                                 /* 0798, 0x1A040798 */
	SENINF2_REG_CSI2_BIST                           SENINF2_CSI2_BIST;                               /* 079C, 0x1A04079C */
	SENINF2_REG_NCSI2_CTL                           SENINF2_NCSI2_CTL;                               /* 07A0, 0x1A0407A0 */
	SENINF2_REG_NCSI2_LNRC_TIMING                   SENINF2_NCSI2_LNRC_TIMING;                       /* 07A4, 0x1A0407A4 */
	SENINF2_REG_NCSI2_LNRD_TIMING                   SENINF2_NCSI2_LNRD_TIMING;                       /* 07A8, 0x1A0407A8 */
	SENINF2_REG_NCSI2_DPCM                          SENINF2_NCSI2_DPCM;                              /* 07AC, 0x1A0407AC */
	SENINF2_REG_NCSI2_INT_EN                        SENINF2_NCSI2_INT_EN;                            /* 07B0, 0x1A0407B0 */
	SENINF2_REG_NCSI2_INT_STATUS                    SENINF2_NCSI2_INT_STATUS;                        /* 07B4, 0x1A0407B4 */
	SENINF2_REG_NCSI2_DGB_SEL                       SENINF2_NCSI2_DGB_SEL;                           /* 07B8, 0x1A0407B8 */
	SENINF2_REG_NCSI2_DBG_PORT                      SENINF2_NCSI2_DBG_PORT;                          /* 07BC, 0x1A0407BC */
	SENINF2_REG_NCSI2_SPARE0                        SENINF2_NCSI2_SPARE0;                            /* 07C0, 0x1A0407C0 */
	SENINF2_REG_NCSI2_SPARE1                        SENINF2_NCSI2_SPARE1;                            /* 07C4, 0x1A0407C4 */
	SENINF2_REG_NCSI2_LNRC_FSM                      SENINF2_NCSI2_LNRC_FSM;                          /* 07C8, 0x1A0407C8 */
	SENINF2_REG_NCSI2_LNRD_FSM                      SENINF2_NCSI2_LNRD_FSM;                          /* 07CC, 0x1A0407CC */
	SENINF2_REG_NCSI2_FRAME_LINE_NUM                SENINF2_NCSI2_FRAME_LINE_NUM;                    /* 07D0, 0x1A0407D0 */
	SENINF2_REG_NCSI2_GENERIC_SHORT                 SENINF2_NCSI2_GENERIC_SHORT;                     /* 07D4, 0x1A0407D4 */
	SENINF2_REG_NCSI2_HSRX_DBG                      SENINF2_NCSI2_HSRX_DBG;                          /* 07D8, 0x1A0407D8 */
	SENINF2_REG_NCSI2_DI                            SENINF2_NCSI2_DI;                                /* 07DC, 0x1A0407DC */
	SENINF2_REG_NCSI2_HS_TRAIL                      SENINF2_NCSI2_HS_TRAIL;                          /* 07E0, 0x1A0407E0 */
	SENINF2_REG_NCSI2_DI_CTRL                       SENINF2_NCSI2_DI_CTRL;                           /* 07E4, 0x1A0407E4 */
	SENINF2_REG_NCSI2_DI_1                          SENINF2_NCSI2_DI_1;                              /* 07E8, 0x1A0407E8 */
	SENINF2_REG_NCSI2_DI_CTRL_1                     SENINF2_NCSI2_DI_CTRL_1;                         /* 07EC, 0x1A0407EC */
	SENINF2_REG_NCSI2_RAW10_DI_CTRL                 SENINF2_NCSI2_RAW10_DI_CTRL;                     /* 07F0, 0x1A0407F0 */
	SENINF2_REG_NCSI2_RAW12_DI_CTRL                 SENINF2_NCSI2_RAW12_DI_CTRL;                     /* 07F4, 0x1A0407F4 */
	SENINF2_REG_NCSI2_RAW14_DI_CTRL                 SENINF2_NCSI2_RAW14_DI_CTRL;                     /* 07F8, 0x1A0407F8 */
	UINT32                                          rsv_07FC[65];                                    /* 07FC..08FF, 0x1A0407FC..1A0408FF */
	SENINF3_REG_CTRL                                SENINF3_CTRL;                                    /* 0900, 0x1A040900 */
	UINT32                                          rsv_0904[7];                                     /* 0904..091F, 0x1A040904..1A04091F */
	SENINF3_REG_MUX_CTRL                            SENINF3_MUX_CTRL;                                /* 0920, 0x1A040920 */
	SENINF3_REG_MUX_INTEN                           SENINF3_MUX_INTEN;                               /* 0924, 0x1A040924 */
	SENINF3_REG_MUX_INTSTA                          SENINF3_MUX_INTSTA;                              /* 0928, 0x1A040928 */
	SENINF3_REG_MUX_SIZE                            SENINF3_MUX_SIZE;                                /* 092C, 0x1A04092C */
	SENINF3_REG_MUX_DEBUG_1                         SENINF3_MUX_DEBUG_1;                             /* 0930, 0x1A040930 */
	SENINF3_REG_MUX_DEBUG_2                         SENINF3_MUX_DEBUG_2;                             /* 0934, 0x1A040934 */
	SENINF3_REG_MUX_DEBUG_3                         SENINF3_MUX_DEBUG_3;                             /* 0938, 0x1A040938 */
	SENINF3_REG_MUX_DEBUG_4                         SENINF3_MUX_DEBUG_4;                             /* 093C, 0x1A04093C */
	SENINF3_REG_MUX_DEBUG_5                         SENINF3_MUX_DEBUG_5;                             /* 0940, 0x1A040940 */
	SENINF3_REG_MUX_DEBUG_6                         SENINF3_MUX_DEBUG_6;                             /* 0944, 0x1A040944 */
	SENINF3_REG_MUX_DEBUG_7                         SENINF3_MUX_DEBUG_7;                             /* 0948, 0x1A040948 */
	SENINF3_REG_MUX_SPARE                           SENINF3_MUX_SPARE;                               /* 094C, 0x1A04094C */
	SENINF3_REG_MUX_DATA                            SENINF3_MUX_DATA;                                /* 0950, 0x1A040950 */
	SENINF3_REG_MUX_DATA_CNT                        SENINF3_MUX_DATA_CNT;                            /* 0954, 0x1A040954 */
	SENINF3_REG_MUX_CROP                            SENINF3_MUX_CROP;                                /* 0958, 0x1A040958 */
	UINT32                                          rsv_095C[41];                                    /* 095C..09FF, 0x1A04095C..1A0409FF */
	SENINF_REG_TG3_PH_CNT                           SENINF_TG3_PH_CNT;                               /* 0A00, 0x1A040A00 */
	SENINF_REG_TG3_SEN_CK                           SENINF_TG3_SEN_CK;                               /* 0A04, 0x1A040A04 */
	SENINF_REG_TG3_TM_CTL                           SENINF_TG3_TM_CTL;                               /* 0A08, 0x1A040A08 */
	SENINF_REG_TG3_TM_SIZE                          SENINF_TG3_TM_SIZE;                              /* 0A0C, 0x1A040A0C */
	SENINF_REG_TG3_TM_CLK                           SENINF_TG3_TM_CLK;                               /* 0A10, 0x1A040A10 */
	SENINF_REG_TG3_TM_STP                           SENINF_TG3_TM_STP;                               /* 0A14, 0x1A040A14 */
	UINT32                                          rsv_0A18[186];                                   /* 0A18..0CFF, 0x1A040A18..1A040CFF */
	SENINF4_REG_CTRL                                SENINF4_CTRL;                                    /* 0D00, 0x1A040D00 */
	UINT32                                          rsv_0D04[7];                                     /* 0D04..0D1F, 0x1A040D04..1A040D1F */
	SENINF4_REG_MUX_CTRL                            SENINF4_MUX_CTRL;                                /* 0D20, 0x1A040D20 */
	SENINF4_REG_MUX_INTEN                           SENINF4_MUX_INTEN;                               /* 0D24, 0x1A040D24 */
	SENINF4_REG_MUX_INTSTA                          SENINF4_MUX_INTSTA;                              /* 0D28, 0x1A040D28 */
	SENINF4_REG_MUX_SIZE                            SENINF4_MUX_SIZE;                                /* 0D2C, 0x1A040D2C */
	SENINF4_REG_MUX_DEBUG_1                         SENINF4_MUX_DEBUG_1;                             /* 0D30, 0x1A040D30 */
	SENINF4_REG_MUX_DEBUG_2                         SENINF4_MUX_DEBUG_2;                             /* 0D34, 0x1A040D34 */
	SENINF4_REG_MUX_DEBUG_3                         SENINF4_MUX_DEBUG_3;                             /* 0D38, 0x1A040D38 */
	SENINF4_REG_MUX_DEBUG_4                         SENINF4_MUX_DEBUG_4;                             /* 0D3C, 0x1A040D3C */
	SENINF4_REG_MUX_DEBUG_5                         SENINF4_MUX_DEBUG_5;                             /* 0D40, 0x1A040D40 */
	SENINF4_REG_MUX_DEBUG_6                         SENINF4_MUX_DEBUG_6;                             /* 0D44, 0x1A040D44 */
	SENINF4_REG_MUX_DEBUG_7                         SENINF4_MUX_DEBUG_7;                             /* 0D48, 0x1A040D48 */
	SENINF4_REG_MUX_SPARE                           SENINF4_MUX_SPARE;                               /* 0D4C, 0x1A040D4C */
	SENINF4_REG_MUX_DATA                            SENINF4_MUX_DATA;                                /* 0D50, 0x1A040D50 */
	SENINF4_REG_MUX_DATA_CNT                        SENINF4_MUX_DATA_CNT;                            /* 0D54, 0x1A040D54 */
	SENINF4_REG_MUX_CROP                            SENINF4_MUX_CROP;                                /* 0D58, 0x1A040D58 */
	UINT32                                          rsv_0D5C[41];                                    /* 0D5C..0DFF, 0x1A040D5C..1A040DFF */
	SENINF_REG_TG4_PH_CNT                           SENINF_TG4_PH_CNT;                               /* 0E00, 0x1A040E00 */
	SENINF_REG_TG4_SEN_CK                           SENINF_TG4_SEN_CK;                               /* 0E04, 0x1A040E04 */
	SENINF_REG_TG4_TM_CTL                           SENINF_TG4_TM_CTL;                               /* 0E08, 0x1A040E08 */
	SENINF_REG_TG4_TM_SIZE                          SENINF_TG4_TM_SIZE;                              /* 0E0C, 0x1A040E0C */
	SENINF_REG_TG4_TM_CLK                           SENINF_TG4_TM_CLK;                               /* 0E10, 0x1A040E10 */
	SENINF_REG_TG4_TM_STP                           SENINF_TG4_TM_STP;                               /* 0E14, 0x1A040E14 */
	UINT32                                          rsv_0E18[42];                                    /* 0E18..0EBF, 0x1A040E18..1A040EBF */
	CCIR656_REG_CTL                                 CCIR656_CTL;                                     /* 0EC0, 0x1A040EC0 */
	CCIR656_REG_H                                   CCIR656_H;                                       /* 0EC4, 0x1A040EC4 */
	CCIR656_REG_PTGEN_H_1                           CCIR656_PTGEN_H_1;                               /* 0EC8, 0x1A040EC8 */
	CCIR656_REG_PTGEN_H_2                           CCIR656_PTGEN_H_2;                               /* 0ECC, 0x1A040ECC */
	CCIR656_REG_PTGEN_V_1                           CCIR656_PTGEN_V_1;                               /* 0ED0, 0x1A040ED0 */
	CCIR656_REG_PTGEN_V_2                           CCIR656_PTGEN_V_2;                               /* 0ED4, 0x1A040ED4 */
	CCIR656_REG_PTGEN_CTL1                          CCIR656_PTGEN_CTL1;                              /* 0ED8, 0x1A040ED8 */
	CCIR656_REG_PTGEN_CTL2                          CCIR656_PTGEN_CTL2;                              /* 0EDC, 0x1A040EDC */
	CCIR656_REG_PTGEN_CTL3                          CCIR656_PTGEN_CTL3;                              /* 0EE0, 0x1A040EE0 */
	CCIR656_REG_STATUS                              CCIR656_STATUS;                                  /* 0EE4, 0x1A040EE4 */
	UINT32                                          rsv_0EE8[142];                                   /* 0EE8..111F, 0x1A040EE8..1A04111F */
	SENINF5_REG_MUX_CTRL                            SENINF5_MUX_CTRL;                                /* 1120, 0x1A041120 */
	SENINF5_REG_MUX_INTEN                           SENINF5_MUX_INTEN;                               /* 1124, 0x1A041124 */
	SENINF5_REG_MUX_INTSTA                          SENINF5_MUX_INTSTA;                              /* 1128, 0x1A041128 */
	SENINF5_REG_MUX_SIZE                            SENINF5_MUX_SIZE;                                /* 112C, 0x1A04112C */
	SENINF5_REG_MUX_DEBUG_1                         SENINF5_MUX_DEBUG_1;                             /* 1130, 0x1A041130 */
	SENINF5_REG_MUX_DEBUG_2                         SENINF5_MUX_DEBUG_2;                             /* 1134, 0x1A041134 */
	SENINF5_REG_MUX_DEBUG_3                         SENINF5_MUX_DEBUG_3;                             /* 1138, 0x1A041138 */
	SENINF5_REG_MUX_DEBUG_4                         SENINF5_MUX_DEBUG_4;                             /* 113C, 0x1A04113C */
	SENINF5_REG_MUX_DEBUG_5                         SENINF5_MUX_DEBUG_5;                             /* 1140, 0x1A041140 */
	SENINF5_REG_MUX_DEBUG_6                         SENINF5_MUX_DEBUG_6;                             /* 1144, 0x1A041144 */
	SENINF5_REG_MUX_DEBUG_7                         SENINF5_MUX_DEBUG_7;                             /* 1148, 0x1A041148 */
	SENINF5_REG_MUX_SPARE                           SENINF5_MUX_SPARE;                               /* 114C, 0x1A04114C */
	SENINF5_REG_MUX_DATA                            SENINF5_MUX_DATA;                                /* 1150, 0x1A041150 */
	SENINF5_REG_MUX_DATA_CNT                        SENINF5_MUX_DATA_CNT;                            /* 1154, 0x1A041154 */
	SENINF5_REG_MUX_CROP                            SENINF5_MUX_CROP;                                /* 1158, 0x1A041158 */
	UINT32                                          rsv_115C[241];                                   /* 115C..151F, 0x1A04115C..1A04151F */
	SENINF6_REG_MUX_CTRL                            SENINF6_MUX_CTRL;                                /* 1520, 0x1A041520 */
	SENINF6_REG_MUX_INTEN                           SENINF6_MUX_INTEN;                               /* 1524, 0x1A041524 */
	SENINF6_REG_MUX_INTSTA                          SENINF6_MUX_INTSTA;                              /* 1528, 0x1A041528 */
	SENINF6_REG_MUX_SIZE                            SENINF6_MUX_SIZE;                                /* 152C, 0x1A04152C */
	SENINF6_REG_MUX_DEBUG_1                         SENINF6_MUX_DEBUG_1;                             /* 1530, 0x1A041530 */
	SENINF6_REG_MUX_DEBUG_2                         SENINF6_MUX_DEBUG_2;                             /* 1534, 0x1A041534 */
	SENINF6_REG_MUX_DEBUG_3                         SENINF6_MUX_DEBUG_3;                             /* 1538, 0x1A041538 */
	SENINF6_REG_MUX_DEBUG_4                         SENINF6_MUX_DEBUG_4;                             /* 153C, 0x1A04153C */
	SENINF6_REG_MUX_DEBUG_5                         SENINF6_MUX_DEBUG_5;                             /* 1540, 0x1A041540 */
	SENINF6_REG_MUX_DEBUG_6                         SENINF6_MUX_DEBUG_6;                             /* 1544, 0x1A041544 */
	SENINF6_REG_MUX_DEBUG_7                         SENINF6_MUX_DEBUG_7;                             /* 1548, 0x1A041548 */
	SENINF6_REG_MUX_SPARE                           SENINF6_MUX_SPARE;                               /* 154C, 0x1A04154C */
	SENINF6_REG_MUX_DATA                            SENINF6_MUX_DATA;                                /* 1550, 0x1A041550 */
	SENINF6_REG_MUX_DATA_CNT                        SENINF6_MUX_DATA_CNT;                            /* 1554, 0x1A041554 */
	SENINF6_REG_MUX_CROP                            SENINF6_MUX_CROP;                                /* 1558, 0x1A041558 */
	UINT32                                          rsv_155C[241];                                   /* 155C..191F, 0x1A04155C..1A04191F */
	SENINF7_REG_MUX_CTRL                            SENINF7_MUX_CTRL;                                /* 1920, 0x1A041920 */
	SENINF7_REG_MUX_INTEN                           SENINF7_MUX_INTEN;                               /* 1924, 0x1A041924 */
	SENINF7_REG_MUX_INTSTA                          SENINF7_MUX_INTSTA;                              /* 1928, 0x1A041928 */
	SENINF7_REG_MUX_SIZE                            SENINF7_MUX_SIZE;                                /* 192C, 0x1A04192C */
	SENINF7_REG_MUX_DEBUG_1                         SENINF7_MUX_DEBUG_1;                             /* 1930, 0x1A041930 */
	SENINF7_REG_MUX_DEBUG_2                         SENINF7_MUX_DEBUG_2;                             /* 1934, 0x1A041934 */
	SENINF7_REG_MUX_DEBUG_3                         SENINF7_MUX_DEBUG_3;                             /* 1938, 0x1A041938 */
	SENINF7_REG_MUX_DEBUG_4                         SENINF7_MUX_DEBUG_4;                             /* 193C, 0x1A04193C */
	SENINF7_REG_MUX_DEBUG_5                         SENINF7_MUX_DEBUG_5;                             /* 1940, 0x1A041940 */
	SENINF7_REG_MUX_DEBUG_6                         SENINF7_MUX_DEBUG_6;                             /* 1944, 0x1A041944 */
	SENINF7_REG_MUX_DEBUG_7                         SENINF7_MUX_DEBUG_7;                             /* 1948, 0x1A041948 */
	SENINF7_REG_MUX_SPARE                           SENINF7_MUX_SPARE;                               /* 194C, 0x1A04194C */
	SENINF7_REG_MUX_DATA                            SENINF7_MUX_DATA;                                /* 1950, 0x1A041950 */
	SENINF7_REG_MUX_DATA_CNT                        SENINF7_MUX_DATA_CNT;                            /* 1954, 0x1A041954 */
	SENINF7_REG_MUX_CROP                            SENINF7_MUX_CROP;                                /* 1958, 0x1A041958 */
	UINT32                                          rsv_195C[241];                                   /* 195C..1D1F, 0x1A04195C..1A041D1F */
	SENINF8_REG_MUX_CTRL                            SENINF8_MUX_CTRL;                                /* 1D20, 0x1A041D20 */
	SENINF8_REG_MUX_INTEN                           SENINF8_MUX_INTEN;                               /* 1D24, 0x1A041D24 */
	SENINF8_REG_MUX_INTSTA                          SENINF8_MUX_INTSTA;                              /* 1D28, 0x1A041D28 */
	SENINF8_REG_MUX_SIZE                            SENINF8_MUX_SIZE;                                /* 1D2C, 0x1A041D2C */
	SENINF8_REG_MUX_DEBUG_1                         SENINF8_MUX_DEBUG_1;                             /* 1D30, 0x1A041D30 */
	SENINF8_REG_MUX_DEBUG_2                         SENINF8_MUX_DEBUG_2;                             /* 1D34, 0x1A041D34 */
	SENINF8_REG_MUX_DEBUG_3                         SENINF8_MUX_DEBUG_3;                             /* 1D38, 0x1A041D38 */
	SENINF8_REG_MUX_DEBUG_4                         SENINF8_MUX_DEBUG_4;                             /* 1D3C, 0x1A041D3C */
	SENINF8_REG_MUX_DEBUG_5                         SENINF8_MUX_DEBUG_5;                             /* 1D40, 0x1A041D40 */
	SENINF8_REG_MUX_DEBUG_6                         SENINF8_MUX_DEBUG_6;                             /* 1D44, 0x1A041D44 */
	SENINF8_REG_MUX_DEBUG_7                         SENINF8_MUX_DEBUG_7;                             /* 1D48, 0x1A041D48 */
	SENINF8_REG_MUX_SPARE                           SENINF8_MUX_SPARE;                               /* 1D4C, 0x1A041D4C */
	SENINF8_REG_MUX_DATA                            SENINF8_MUX_DATA;                                /* 1D50, 0x1A041D50 */
	SENINF8_REG_MUX_DATA_CNT                        SENINF8_MUX_DATA_CNT;                            /* 1D54, 0x1A041D54 */
	SENINF8_REG_MUX_CROP                            SENINF8_MUX_CROP;                                /* 1D58, 0x1A041D58 */
	UINT32                                          rsv_1D5C[20];                                    /* 1D5C..1DAB, 1A041D5C..1A041DAB */
}seninf_reg_t;
#endif

typedef volatile struct _seninf_reg_t_base_  /* 0x1A040000..0x1A047FFF */
{
	SENINF_REG_TOP_CTRL 							SENINF_TOP_CTRL;								 /* 0000, 0x1A040000 */
	SENINF_REG_TOP_CMODEL_PAR						SENINF_TOP_CMODEL_PAR;							 /* 0004, 0x1A040004 */
	SENINF_REG_TOP_MUX_CTRL 						SENINF_TOP_MUX_CTRL;							 /* 0008, 0x1A040008 */
	UINT32											rsv_000C[45];									 /* 000C..00BF, 0x1A04000C..1A0400BF */
	SENINF_REG_N3D_CTL								SENINF_N3D_CTL; 								 /* 00C0, 0x1A0400C0 */
	SENINF_REG_N3D_POS								SENINF_N3D_POS; 								 /* 00C4, 0x1A0400C4 */
	SENINF_REG_N3D_TRIG 							SENINF_N3D_TRIG;								 /* 00C8, 0x1A0400C8 */
	SENINF_REG_N3D_INT								SENINF_N3D_INT; 								 /* 00CC, 0x1A0400CC */
	SENINF_REG_N3D_CNT0 							SENINF_N3D_CNT0;								 /* 00D0, 0x1A0400D0 */
	SENINF_REG_N3D_CNT1 							SENINF_N3D_CNT1;								 /* 00D4, 0x1A0400D4 */
	SENINF_REG_N3D_DBG								SENINF_N3D_DBG; 								 /* 00D8, 0x1A0400D8 */
	SENINF_REG_N3D_DIFF_THR 						SENINF_N3D_DIFF_THR;							 /* 00DC, 0x1A0400DC */
	SENINF_REG_N3D_DIFF_CNT 						SENINF_N3D_DIFF_CNT;							 /* 00E0, 0x1A0400E0 */
	SENINF_REG_N3D_DBG_1							SENINF_N3D_DBG_1;								 /* 00E4, 0x1A0400E4 */
	SENINF_REG_N3D_VALID_TG_CNT 					SENINF_N3D_VALID_TG_CNT;						 /* 00E8, 0x1A0400E8 */
	SENINF_REG_N3D_SYNC_A_PERIOD					SENINF_N3D_SYNC_A_PERIOD;						 /* 00EC, 0x1A0400EC */
	SENINF_REG_N3D_SYNC_B_PERIOD					SENINF_N3D_SYNC_B_PERIOD;						 /* 00F0, 0x1A0400F0 */
	SENINF_REG_N3D_SYNC_A_PULSE_LEN 				SENINF_N3D_SYNC_A_PULSE_LEN;					 /* 00F4, 0x1A0400F4 */
	SENINF_REG_N3D_SYNC_B_PULSE_LEN 				SENINF_N3D_SYNC_B_PULSE_LEN;					 /* 00F8, 0x1A0400F8 */
	UINT32											rsv_00FC;										 /* 00FC, 0x1A0400FC */
	SENINF1_REG_CTRL								SENINF1_CTRL;									 /* 0100, 0x1A040100 */
	UINT32											rsv_0104[7];									 /* 0104..011F, 0x1A040104..1A04011F */
	SENINF1_REG_MUX_CTRL							SENINF1_MUX_CTRL;								 /* 0120, 0x1A040120 */
	SENINF1_REG_MUX_INTEN							SENINF1_MUX_INTEN;								 /* 0124, 0x1A040124 */
	SENINF1_REG_MUX_INTSTA							SENINF1_MUX_INTSTA; 							 /* 0128, 0x1A040128 */
	SENINF1_REG_MUX_SIZE							SENINF1_MUX_SIZE;								 /* 012C, 0x1A04012C */
	SENINF1_REG_MUX_DEBUG_1 						SENINF1_MUX_DEBUG_1;							 /* 0130, 0x1A040130 */
	SENINF1_REG_MUX_DEBUG_2 						SENINF1_MUX_DEBUG_2;							 /* 0134, 0x1A040134 */
	SENINF1_REG_MUX_DEBUG_3 						SENINF1_MUX_DEBUG_3;							 /* 0138, 0x1A040138 */
	SENINF1_REG_MUX_DEBUG_4 						SENINF1_MUX_DEBUG_4;							 /* 013C, 0x1A04013C */
	SENINF1_REG_MUX_DEBUG_5 						SENINF1_MUX_DEBUG_5;							 /* 0140, 0x1A040140 */
	SENINF1_REG_MUX_DEBUG_6 						SENINF1_MUX_DEBUG_6;							 /* 0144, 0x1A040144 */
	SENINF1_REG_MUX_DEBUG_7 						SENINF1_MUX_DEBUG_7;							 /* 0148, 0x1A040148 */
	SENINF1_REG_MUX_SPARE							SENINF1_MUX_SPARE;								 /* 014C, 0x1A04014C */
	SENINF1_REG_MUX_DATA							SENINF1_MUX_DATA;								 /* 0150, 0x1A040150 */
	SENINF1_REG_MUX_DATA_CNT						SENINF1_MUX_DATA_CNT;							 /* 0154, 0x1A040154 */
	SENINF1_REG_MUX_CROP							SENINF1_MUX_CROP;								 /* 0158, 0x1A040158 */
	UINT32											rsv_015C[41];									 /* 015C..01FF, 0x1A04015C..1A0401FF */
	SENINF_REG_TG1_PH_CNT							SENINF_TG1_PH_CNT;								 /* 0200, 0x1A040200 */
	SENINF_REG_TG1_SEN_CK							SENINF_TG1_SEN_CK;								 /* 0204, 0x1A040204 */
	SENINF_REG_TG1_TM_CTL							SENINF_TG1_TM_CTL;								 /* 0208, 0x1A040208 */
	SENINF_REG_TG1_TM_SIZE							SENINF_TG1_TM_SIZE; 							 /* 020C, 0x1A04020C */
	SENINF_REG_TG1_TM_CLK							SENINF_TG1_TM_CLK;								 /* 0210, 0x1A040210 */
	SENINF_REG_TG1_TM_STP							SENINF_TG1_TM_STP;								 /* 0214, 0x1A040214 */
	UINT32											rsv_0218[58];									 /* 0218..02FF, 0x1A040218..1A0402FF */
	MIPI_REG_RX_CON00_CSI0							MIPI_RX_CON00_CSI0; 							 /* 0300, 0x1A040300 */
	MIPI_REG_RX_CON04_CSI0							MIPI_RX_CON04_CSI0; 							 /* 0304, 0x1A040304 */
	MIPI_REG_RX_CON08_CSI0							MIPI_RX_CON08_CSI0; 							 /* 0308, 0x1A040308 */
	MIPI_REG_RX_CON0C_CSI0							MIPI_RX_CON0C_CSI0; 							 /* 030C, 0x1A04030C */
	MIPI_REG_RX_CON10_CSI0							MIPI_RX_CON10_CSI0; 							 /* 0310, 0x1A040310 */
	UINT32											rsv_0314[4];									 /* 0314..0323, 0x1A040314..1A040323 */
	MIPI_REG_RX_CON24_CSI0							MIPI_RX_CON24_CSI0; 							 /* 0324, 0x1A040324 */
	MIPI_REG_RX_CON28_CSI0							MIPI_RX_CON28_CSI0; 							 /* 0328, 0x1A040328 */
	UINT32											rsv_032C[2];									 /* 032C..0333, 0x1A04032C..1A040333 */
	MIPI_REG_RX_CON34_CSI0							MIPI_RX_CON34_CSI0; 							 /* 0334, 0x1A040334 */
	MIPI_REG_RX_CON38_CSI0							MIPI_RX_CON38_CSI0; 							 /* 0338, 0x1A040338 */
	MIPI_REG_RX_CON3C_CSI0							MIPI_RX_CON3C_CSI0; 							 /* 033C, 0x1A04033C */
	MIPI_REG_RX_CON40_CSI0							MIPI_RX_CON40_CSI0; 							 /* 0340, 0x1A040340 */
	MIPI_REG_RX_CON44_CSI0							MIPI_RX_CON44_CSI0; 							 /* 0344, 0x1A040344 */
	MIPI_REG_RX_CON48_CSI0							MIPI_RX_CON48_CSI0; 							 /* 0348, 0x1A040348 */
	UINT32											rsv_034C;										 /* 034C, 0x1A04034C */
	MIPI_REG_RX_CON50_CSI0							MIPI_RX_CON50_CSI0; 							 /* 0350, 0x1A040350 */
	UINT32											rsv_0354[3];									 /* 0354..035F, 0x1A040354..1A04035F */
	SENINF1_REG_CSI2_CTRL							SENINF1_CSI2_CTRL;								 /* 0360, 0x1A040360 */
	SENINF1_REG_CSI2_DELAY							SENINF1_CSI2_DELAY; 							 /* 0364, 0x1A040364 */
	SENINF1_REG_CSI2_INTEN							SENINF1_CSI2_INTEN; 							 /* 0368, 0x1A040368 */
	SENINF1_REG_CSI2_INTSTA 						SENINF1_CSI2_INTSTA;							 /* 036C, 0x1A04036C */
	SENINF1_REG_CSI2_ECCDBG 						SENINF1_CSI2_ECCDBG;							 /* 0370, 0x1A040370 */
	SENINF1_REG_CSI2_CRCDBG 						SENINF1_CSI2_CRCDBG;							 /* 0374, 0x1A040374 */
	SENINF1_REG_CSI2_DBG							SENINF1_CSI2_DBG;								 /* 0378, 0x1A040378 */
	SENINF1_REG_CSI2_VER							SENINF1_CSI2_VER;								 /* 037C, 0x1A04037C */
	SENINF1_REG_CSI2_SHORT_INFO 					SENINF1_CSI2_SHORT_INFO;						 /* 0380, 0x1A040380 */
	SENINF1_REG_CSI2_LNFSM							SENINF1_CSI2_LNFSM; 							 /* 0384, 0x1A040384 */
	SENINF1_REG_CSI2_LNMUX							SENINF1_CSI2_LNMUX; 							 /* 0388, 0x1A040388 */
	SENINF1_REG_CSI2_HSYNC_CNT						SENINF1_CSI2_HSYNC_CNT; 						 /* 038C, 0x1A04038C */
	SENINF1_REG_CSI2_CAL							SENINF1_CSI2_CAL;								 /* 0390, 0x1A040390 */
	SENINF1_REG_CSI2_DS 							SENINF1_CSI2_DS;								 /* 0394, 0x1A040394 */
	SENINF1_REG_CSI2_VS 							SENINF1_CSI2_VS;								 /* 0398, 0x1A040398 */
	SENINF1_REG_CSI2_BIST							SENINF1_CSI2_BIST;								 /* 039C, 0x1A04039C */
	SENINF1_REG_NCSI2_CTL							SENINF1_NCSI2_CTL;								 /* 03A0, 0x1A0403A0 */
	SENINF1_REG_NCSI2_LNRC_TIMING					SENINF1_NCSI2_LNRC_TIMING;						 /* 03A4, 0x1A0403A4 */
	SENINF1_REG_NCSI2_LNRD_TIMING					SENINF1_NCSI2_LNRD_TIMING;						 /* 03A8, 0x1A0403A8 */
	SENINF1_REG_NCSI2_DPCM							SENINF1_NCSI2_DPCM; 							 /* 03AC, 0x1A0403AC */
	SENINF1_REG_NCSI2_INT_EN						SENINF1_NCSI2_INT_EN;							 /* 03B0, 0x1A0403B0 */
	SENINF1_REG_NCSI2_INT_STATUS					SENINF1_NCSI2_INT_STATUS;						 /* 03B4, 0x1A0403B4 */
	SENINF1_REG_NCSI2_DGB_SEL						SENINF1_NCSI2_DGB_SEL;							 /* 03B8, 0x1A0403B8 */
	SENINF1_REG_NCSI2_DBG_PORT						SENINF1_NCSI2_DBG_PORT; 						 /* 03BC, 0x1A0403BC */
	SENINF1_REG_NCSI2_SPARE0						SENINF1_NCSI2_SPARE0;							 /* 03C0, 0x1A0403C0 */
	SENINF1_REG_NCSI2_SPARE1						SENINF1_NCSI2_SPARE1;							 /* 03C4, 0x1A0403C4 */
	SENINF1_REG_NCSI2_LNRC_FSM						SENINF1_NCSI2_LNRC_FSM; 						 /* 03C8, 0x1A0403C8 */
	SENINF1_REG_NCSI2_LNRD_FSM						SENINF1_NCSI2_LNRD_FSM; 						 /* 03CC, 0x1A0403CC */
	SENINF1_REG_NCSI2_FRAME_LINE_NUM				SENINF1_NCSI2_FRAME_LINE_NUM;					 /* 03D0, 0x1A0403D0 */
	SENINF1_REG_NCSI2_GENERIC_SHORT 				SENINF1_NCSI2_GENERIC_SHORT;					 /* 03D4, 0x1A0403D4 */
	SENINF1_REG_NCSI2_HSRX_DBG						SENINF1_NCSI2_HSRX_DBG; 						 /* 03D8, 0x1A0403D8 */
	SENINF1_REG_NCSI2_DI							SENINF1_NCSI2_DI;								 /* 03DC, 0x1A0403DC */
	SENINF1_REG_NCSI2_HS_TRAIL						SENINF1_NCSI2_HS_TRAIL; 						 /* 03E0, 0x1A0403E0 */
	SENINF1_REG_NCSI2_DI_CTRL						SENINF1_NCSI2_DI_CTRL;							 /* 03E4, 0x1A0403E4 */
	SENINF1_REG_NCSI2_DI_1							SENINF1_NCSI2_DI_1; 							 /* 03E8, 0x1A0403E8 */
	SENINF1_REG_NCSI2_DI_CTRL_1 					SENINF1_NCSI2_DI_CTRL_1;						 /* 03EC, 0x1A0403EC */
	SENINF1_REG_NCSI2_RAW10_DI_CTRL 				SENINF1_NCSI2_RAW10_DI_CTRL;					 /* 03F0, 0x1A0403F0 */
	SENINF1_REG_NCSI2_RAW12_DI_CTRL 				SENINF1_NCSI2_RAW12_DI_CTRL;					 /* 03F4, 0x1A0403F4 */
	SENINF1_REG_NCSI2_RAW14_DI_CTRL 				SENINF1_NCSI2_RAW14_DI_CTRL;					 /* 03F8, 0x1A0403F8 */
	UINT32											rsv_03FC[65];									 /* 03FC..04FF, 0x1A0403FC..1A0404FF */
}seninf_reg_t_base;


typedef volatile struct _seninf_mux_reg_t_base_  /* 0x1A040d00..0x1A040d3c */
{
	SENINF1_REG_MUX_CTRL							SENINF1_MUX_CTRL;								 /* 0120, 0x1A040120 */
	SENINF1_REG_MUX_INTEN							SENINF1_MUX_INTEN;								 /* 0124, 0x1A040124 */
	SENINF1_REG_MUX_INTSTA							SENINF1_MUX_INTSTA; 							 /* 0128, 0x1A040128 */
	SENINF1_REG_MUX_SIZE							SENINF1_MUX_SIZE;								 /* 012C, 0x1A04012C */
	SENINF1_REG_MUX_DEBUG_1 						SENINF1_MUX_DEBUG_1;							 /* 0130, 0x1A040130 */
	SENINF1_REG_MUX_DEBUG_2 						SENINF1_MUX_DEBUG_2;							 /* 0134, 0x1A040134 */
	SENINF1_REG_MUX_DEBUG_3 						SENINF1_MUX_DEBUG_3;							 /* 0138, 0x1A040138 */
	SENINF1_REG_MUX_DEBUG_4 						SENINF1_MUX_DEBUG_4;							 /* 013C, 0x1A04013C */
	SENINF1_REG_MUX_DEBUG_5 						SENINF1_MUX_DEBUG_5;							 /* 0140, 0x1A040140 */
	SENINF1_REG_MUX_DEBUG_6 						SENINF1_MUX_DEBUG_6;							 /* 0144, 0x1A040144 */
	SENINF1_REG_MUX_DEBUG_7 						SENINF1_MUX_DEBUG_7;							 /* 0148, 0x1A040148 */
	SENINF1_REG_MUX_SPARE							SENINF1_MUX_SPARE;								 /* 014C, 0x1A04014C */
	SENINF1_REG_MUX_DATA							SENINF1_MUX_DATA;								 /* 0150, 0x1A040150 */
	SENINF1_REG_MUX_DATA_CNT						SENINF1_MUX_DATA_CNT;							 /* 0154, 0x1A040154 */
	SENINF1_REG_MUX_CROP							SENINF1_MUX_CROP;								 /* 0158, 0x1A040158 */
}seninf_mux_reg_t_base;

#if 1
typedef volatile struct _mipi_ana_reg_t_	/* 0x10217000..0x1021814F */
{
	MIPI_REG_RX_ANA00_CSI0                          MIPI_RX_ANA00_CSI0;                              /* 0000, 0x10217000 */
	MIPI_REG_RX_ANA04_CSI0                          MIPI_RX_ANA04_CSI0;                              /* 0004, 0x10217004 */
	MIPI_REG_RX_ANA08_CSI0                          MIPI_RX_ANA08_CSI0;                              /* 0008, 0x10217008 */
	MIPI_REG_RX_ANA0C_CSI0                          MIPI_RX_ANA0C_CSI0;                              /* 000C, 0x1021700C */
	MIPI_REG_RX_ANA10_CSI0                          MIPI_RX_ANA10_CSI0;                              /* 0010, 0x10217010 */
	UINT32                                          rsv_0014[3];                                     /* 0014..001F, 0x10217014..1021701F */
	MIPI_REG_RX_ANA20_CSI0                          MIPI_RX_ANA20_CSI0;                              /* 0020, 0x10217020 */
	MIPI_REG_RX_ANA24_CSI0                          MIPI_RX_ANA24_CSI0;                              /* 0024, 0x10217024 */
	MIPI_REG_RX_ANA28_CSI0                          MIPI_RX_ANA28_CSI0;                              /* 0028, 0x10217028 */
	UINT32                                          rsv_002C;                                        /* 002C, 0x1021702C */
	MIPI_REG_RX_ANA30_CSI0                          MIPI_RX_ANA30_CSI0;                              /* 0030, 0x10217030 */
	UINT32                                          rsv_0034[4];                                     /* 0034..0043, 0x10217034..10217043 */
	MIPI_REG_RX_ANA44_CSI0                          MIPI_RX_ANA44_CSI0;                              /* 0044, 0x10217044 */
	UINT32                                          rsv_0048;                                        /* 0048, 0x10217048 */
	MIPI_REG_RX_ANA4C_CSI0                          MIPI_RX_ANA4C_CSI0;                              /* 004C, 0x1021704C */
	MIPI_REG_RX_ANA50_CSI0                          MIPI_RX_ANA50_CSI0;                              /* 0050, 0x10217050 */
	MIPI_REG_RX_ANA54_CSI0                          MIPI_RX_ANA54_CSI0;                              /* 0054, 0x10217054 */
	MIPI_REG_RX_ANA58_CSI0                          MIPI_RX_ANA58_CSI0;                              /* 0058, 0x10217058 */
	UINT32                                          rsv_005C[7];                                     /* 005C..0077, 0x1021705C..10217077 */
	MIPI_REG_RX_ANA78_CSI0                          MIPI_RX_ANA78_CSI0;                              /* 0078, 0x10217078 */
	MIPI_REG_RX_ANA7C_CSI0                          MIPI_RX_ANA7C_CSI0;                              /* 007C, 0x1021707C */
	MIPI_REG_RX_ANA80_CSI0                          MIPI_RX_ANA80_CSI0;                              /* 0080, 0x10217080 */
	MIPI_REG_RX_ANA84_CSI0                          MIPI_RX_ANA84_CSI0;                              /* 0084, 0x10217084 */
	MIPI_REG_RX_ANA88_CSI0                          MIPI_RX_ANA88_CSI0;                              /* 0088, 0x10217088 */
	MIPI_REG_RX_ANA8C_CSI0                          MIPI_RX_ANA8C_CSI0;                              /* 008C, 0x1021708C */
	MIPI_REG_RX_ANA90_CSI0                          MIPI_RX_ANA90_CSI0;                              /* 0090, 0x10217090 */
	MIPI_REG_RX_ANA94_CSI0                          MIPI_RX_ANA94_CSI0;                              /* 0094, 0x10217094 */
	MIPI_REG_RX_ANA98_CSI0                          MIPI_RX_ANA98_CSI0;                              /* 0098, 0x10217098 */
	UINT32                                          rsv_009C;                                        /* 009C, 0x1021709C */
	MIPI_REG_RX_ANAA0_CSI0                          MIPI_RX_ANAA0_CSI0;                              /* 00A0, 0x102170A0 */
	UINT32                                          rsv_00A4[3];                                     /* 00A4..00AF, 0x102170A4..102170AF */
	MIPI_REG_RX_ANAB0_CSI0                          MIPI_RX_ANAB0_CSI0;                              /* 00B0, 0x102170B0 */
	MIPI_REG_RX_ANAB4_CSI0                          MIPI_RX_ANAB4_CSI0;                              /* 00B4, 0x102170B4 */
	MIPI_REG_RX_ANAB8_CSI0                          MIPI_RX_ANAB8_CSI0;                              /* 00B8, 0x102170B8 */
	MIPI_REG_RX_ANABC_CSI0                          MIPI_RX_ANABC_CSI0;                              /* 00BC, 0x102170BC */
	MIPI_REG_RX_ANAC0_CSI0                          MIPI_RX_ANAC0_CSI0;                              /* 00C0, 0x102170C0 */
	MIPI_REG_RX_ANAC4_CSI0                          MIPI_RX_ANAC4_CSI0;                              /* 00C4, 0x102170C4 */
	MIPI_REG_RX_ANAC8_CSI0                          MIPI_RX_ANAC8_CSI0;                              /* 00C8, 0x102170C8 */
	MIPI_REG_RX_ANACC_CSI0                          MIPI_RX_ANACC_CSI0;                              /* 00CC, 0x102170CC */
	MIPI_REG_RX_ANAD0_CSI0                          MIPI_RX_ANAD0_CSI0;                              /* 00D0, 0x102170D0 */
	UINT32                                          rsv_00D4[971];                                   /* 00D4..0FFF, 0x102170D4..10217FFF */
	MIPI_REG_RX_ANA00_CSI1                          MIPI_RX_ANA00_CSI1;                              /* 1000, 0x10218000 */
	MIPI_REG_RX_ANA04_CSI1                          MIPI_RX_ANA04_CSI1;                              /* 1004, 0x10218004 */
	MIPI_REG_RX_ANA08_CSI1                          MIPI_RX_ANA08_CSI1;                              /* 1008, 0x10218008 */
	MIPI_REG_RX_ANA0C_CSI1                          MIPI_RX_ANA0C_CSI1;                              /* 100C, 0x1021800C */
	MIPI_REG_RX_ANA10_CSI1                          MIPI_RX_ANA10_CSI1;                              /* 1010, 0x10218010 */
	UINT32                                          rsv_1014[3];                                     /* 1014..101F, 0x10218014..1021801F */
	MIPI_REG_RX_ANA20_CSI1                          MIPI_RX_ANA20_CSI1;                              /* 1020, 0x10218020 */
	MIPI_REG_RX_ANA24_CSI1                          MIPI_RX_ANA24_CSI1;                              /* 1024, 0x10218024 */
	MIPI_REG_RX_ANA28_CSI1                          MIPI_RX_ANA28_CSI1;                              /* 1028, 0x10218028 */
	UINT32                                          rsv_102C;                                        /* 102C, 0x1021802C */
	MIPI_REG_RX_ANA30_CSI1                          MIPI_RX_ANA30_CSI1;                              /* 1030, 0x10218030 */
	UINT32                                          rsv_1034[4];                                     /* 1034..1043, 0x10218034..10218043 */
	MIPI_REG_RX_ANA44_CSI1                          MIPI_RX_ANA44_CSI1;                              /* 1044, 0x10218044 */
	UINT32                                          rsv_1048;                                        /* 1048, 0x10218048 */
	MIPI_REG_RX_ANA4C_CSI1                          MIPI_RX_ANA4C_CSI1;                              /* 104C, 0x1021804C */
	MIPI_REG_RX_ANA50_CSI1                          MIPI_RX_ANA50_CSI1;                              /* 1050, 0x10218050 */
	MIPI_REG_RX_ANA54_CSI1                          MIPI_RX_ANA54_CSI1;                              /* 1054, 0x10218054 */
	MIPI_REG_RX_ANA58_CSI1                          MIPI_RX_ANA58_CSI1;                              /* 1058, 0x10218058 */
	UINT32                                          rsv_105C[7];                                     /* 105C..1077, 0x1021805C..10218077 */
	MIPI_REG_RX_ANA78_CSI1                          MIPI_RX_ANA78_CSI1;                              /* 1078, 0x10218078 */
	MIPI_REG_RX_ANA7C_CSI1                          MIPI_RX_ANA7C_CSI1;                              /* 107C, 0x1021807C */
	MIPI_REG_RX_ANA80_CSI1                          MIPI_RX_ANA80_CSI1;                              /* 1080, 0x10218080 */
	MIPI_REG_RX_ANA84_CSI1                          MIPI_RX_ANA84_CSI1;                              /* 1084, 0x10218084 */
	MIPI_REG_RX_ANA88_CSI1                          MIPI_RX_ANA88_CSI1;                              /* 1088, 0x10218088 */
	MIPI_REG_RX_ANA8C_CSI1                          MIPI_RX_ANA8C_CSI1;                              /* 108C, 0x1021808C */
	MIPI_REG_RX_ANA90_CSI1                          MIPI_RX_ANA90_CSI1;                              /* 1090, 0x10218090 */
	MIPI_REG_RX_ANA94_CSI1                          MIPI_RX_ANA94_CSI1;                              /* 1094, 0x10218094 */
	MIPI_REG_RX_ANA98_CSI1                          MIPI_RX_ANA98_CSI1;                              /* 1098, 0x10218098 */
	UINT32                                          rsv_109C;                                        /* 109C, 0x1021809C */
	MIPI_REG_RX_ANAA0_CSI1                          MIPI_RX_ANAA0_CSI1;                              /* 10A0, 0x102180A0 */
	UINT32                                          rsv_10A4[3];                                     /* 10A4..10AF, 0x102180A4..102180AF */
	MIPI_REG_RX_ANAB0_CSI1                          MIPI_RX_ANAB0_CSI1;                              /* 10B0, 0x102180B0 */
	MIPI_REG_RX_ANAB4_CSI1                          MIPI_RX_ANAB4_CSI1;                              /* 10B4, 0x102180B4 */
	MIPI_REG_RX_ANAB8_CSI1                          MIPI_RX_ANAB8_CSI1;                              /* 10B8, 0x102180B8 */
	MIPI_REG_RX_ANABC_CSI1                          MIPI_RX_ANABC_CSI1;                              /* 10BC, 0x102180BC */
	MIPI_REG_RX_ANAC0_CSI1                          MIPI_RX_ANAC0_CSI1;                              /* 10C0, 0x102180C0 */
	MIPI_REG_RX_ANAC4_CSI1                          MIPI_RX_ANAC4_CSI1;                              /* 10C4, 0x102180C4 */
	MIPI_REG_RX_ANAC8_CSI1                          MIPI_RX_ANAC8_CSI1;                              /* 10C8, 0x102180C8 */
	MIPI_REG_RX_ANACC_CSI1                          MIPI_RX_ANACC_CSI1;                              /* 10CC, 0x102180CC */
	MIPI_REG_RX_ANAD0_CSI1                          MIPI_RX_ANAD0_CSI1;                              /* 10D0, 0x102180D0 */
	UINT32                                          rsv_10D4[31];                                    /* 10D4..114F, 102180D4..1021814F */
}mipi_ana_reg_t;
#endif

typedef volatile struct _mipi_ana_reg_t_base_    /* 0x10217000..0x10219057 */
{
	MIPI_REG_RX_ANA00_CSI0                          MIPI_RX_ANA00_CSI0;                              /* 0000, 0x10217000 */
	MIPI_REG_RX_ANA04_CSI0                          MIPI_RX_ANA04_CSI0;                              /* 0004, 0x10217004 */
	MIPI_REG_RX_ANA08_CSI0                          MIPI_RX_ANA08_CSI0;                              /* 0008, 0x10217008 */
	MIPI_REG_RX_ANA0C_CSI0                          MIPI_RX_ANA0C_CSI0;                              /* 000C, 0x1021700C */
	MIPI_REG_RX_ANA10_CSI0                          MIPI_RX_ANA10_CSI0;                              /* 0010, 0x10217010 */
	UINT32                                          rsv_0014[3];                                     /* 0014..001F, 0x10217014..1021701F */
	MIPI_REG_RX_ANA20_CSI0                          MIPI_RX_ANA20_CSI0;                              /* 0020, 0x10217020 */
	MIPI_REG_RX_ANA24_CSI0                          MIPI_RX_ANA24_CSI0;                              /* 0024, 0x10217024 */
	MIPI_REG_RX_ANA28_CSI0                          MIPI_RX_ANA28_CSI0;                              /* 0028, 0x10217028 */
	UINT32                                          rsv_002C;                                        /* 002C, 0x1021702C */
	MIPI_REG_RX_ANA30_CSI0                          MIPI_RX_ANA30_CSI0;                              /* 0030, 0x10217030 */
	UINT32                                          rsv_0034[4];                                     /* 0034..0043, 0x10217034..10217043 */
	MIPI_REG_RX_ANA44_CSI0                          MIPI_RX_ANA44_CSI0;                              /* 0044, 0x10217044 */
	UINT32                                          rsv_0048;                                        /* 0048, 0x10217048 */
	MIPI_REG_RX_ANA4C_CSI0                          MIPI_RX_ANA4C_CSI0;                              /* 004C, 0x1021704C */
	MIPI_REG_RX_ANA50_CSI0                          MIPI_RX_ANA50_CSI0;                              /* 0050, 0x10217050 */
	MIPI_REG_RX_ANA54_CSI0                          MIPI_RX_ANA54_CSI0;                              /* 0054, 0x10217054 */
	MIPI_REG_RX_ANA58_CSI0                          MIPI_RX_ANA58_CSI0;                              /* 0058, 0x10217058 */
	UINT32                                          rsv_005C[7];                                     /* 005C..0077, 0x1021705C..10217077 */
	MIPI_REG_RX_ANA78_CSI0                          MIPI_RX_ANA78_CSI0;                              /* 0078, 0x10217078 */
	MIPI_REG_RX_ANA7C_CSI0                          MIPI_RX_ANA7C_CSI0;                              /* 007C, 0x1021707C */
	MIPI_REG_RX_ANA80_CSI0                          MIPI_RX_ANA80_CSI0;                              /* 0080, 0x10217080 */
	MIPI_REG_RX_ANA84_CSI0                          MIPI_RX_ANA84_CSI0;                              /* 0084, 0x10217084 */
	MIPI_REG_RX_ANA88_CSI0                          MIPI_RX_ANA88_CSI0;                              /* 0088, 0x10217088 */
	MIPI_REG_RX_ANA8C_CSI0                          MIPI_RX_ANA8C_CSI0;                              /* 008C, 0x1021708C */
	MIPI_REG_RX_ANA90_CSI0                          MIPI_RX_ANA90_CSI0;                              /* 0090, 0x10217090 */
	MIPI_REG_RX_ANA94_CSI0                          MIPI_RX_ANA94_CSI0;                              /* 0094, 0x10217094 */
	MIPI_REG_RX_ANA98_CSI0                          MIPI_RX_ANA98_CSI0;                              /* 0098, 0x10217098 */
	UINT32                                          rsv_009C;                                        /* 009C, 0x1021709C */
	MIPI_REG_RX_ANAA0_CSI0                          MIPI_RX_ANAA0_CSI0;                              /* 00A0, 0x102170A0 */
	UINT32                                          rsv_00A4[3];                                     /* 00A4..00AF, 0x102170A4..102170AF */
	MIPI_REG_RX_ANAB0_CSI0                          MIPI_RX_ANAB0_CSI0;                              /* 00B0, 0x102170B0 */
	MIPI_REG_RX_ANAB4_CSI0                          MIPI_RX_ANAB4_CSI0;                              /* 00B4, 0x102170B4 */
	MIPI_REG_RX_ANAB8_CSI0                          MIPI_RX_ANAB8_CSI0;                              /* 00B8, 0x102170B8 */
	MIPI_REG_RX_ANABC_CSI0                          MIPI_RX_ANABC_CSI0;                              /* 00BC, 0x102170BC */
	MIPI_REG_RX_ANAC0_CSI0                          MIPI_RX_ANAC0_CSI0;                              /* 00C0, 0x102170C0 */
	MIPI_REG_RX_ANAC4_CSI0                          MIPI_RX_ANAC4_CSI0;                              /* 00C4, 0x102170C4 */
	MIPI_REG_RX_ANAC8_CSI0                          MIPI_RX_ANAC8_CSI0;                              /* 00C8, 0x102170C8 */
	MIPI_REG_RX_ANACC_CSI0                          MIPI_RX_ANACC_CSI0;                              /* 00CC, 0x102170CC */
	MIPI_REG_RX_ANAD0_CSI0                          MIPI_RX_ANAD0_CSI0;                              /* 00D0, 0x102170D0 */
	UINT32                                          rsv_00D4[971];                                   /* 00D4..0FFF, 0x102170D4..10217FFF */
}mipi_ana_reg_t_base;

#endif // _SENINF_REG_H_
