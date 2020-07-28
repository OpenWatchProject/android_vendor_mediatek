#ifndef __TILE_MDP_REG_H__
#define __TILE_MDP_REG_H__

#include "DpTileScaler.h"
class DpRingBufferPool;
typedef int32_t (*pullBufferDataCB) (DpRingBufferPool *pBuf,
                                     int32_t bufID,
                                     int32_t left,
                                     int32_t right,
                                     int32_t top,
                                     int32_t bottom);

typedef int32_t (*fitConstraintCB) (DpRingBufferPool *pBuf,
                                    int32_t orgLeft,
                                    int32_t orgRight,
                                    int32_t orgTop,
                                    int32_t orgBottom,
                                    int32_t &newLeft ,
                                    int32_t &newRight,
                                    int32_t &newTop,
                                    int32_t &newBottom);

/* id enum */
#define MDP_TILE_FUNC_ID_ENUM_DECLARE \
    TILE_FUNC_MDP_BASE = (10000),\
    TILE_FUNC_CAMIN_ID = TILE_FUNC_MDP_BASE + 3, /* tCAMIN */\
    TILE_FUNC_RDMA0_ID,\
    TILE_FUNC_PRZ0_ID,\
    TILE_FUNC_PRZ1_ID,\
    TILE_FUNC_TDSHP0_ID,\
    TILE_FUNC_WROT0_ID,\
    TILE_FUNC_WDMA_ID,\

/* error enum */
#define MDP_TILE_ERROR_MESSAGE_ENUM(n, CMD) \
    /* Ring Buffer control */\
    CMD(n, MDP_MESSAGE_BUFFER_EMPTY, ISP_TPIPE_MESSAGE_FAIL)\
    /* RDMA check */\
    CMD(n, MDP_MESSAGE_RDMA_NULL_DATA, ISP_TPIPE_MESSAGE_FAIL)\
    /* PRZ check */\
    CMD(n, MDP_MESSAGE_PRZ_NULL_DATA, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, MDP_MESSAGE_RESIZER_SCALING_ERROR, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, MDP_MESSAGE_RESIZER_INVALID_OUTPUT, ISP_TPIPE_MESSAGE_FAIL)\
    /* TDSHP check */\
    CMD(n, MDP_MESSAGE_TDSHP_BACK_LT_FORWARD, ISP_TPIPE_MESSAGE_FAIL)\
    /* WROT check */\
    CMD(n, MDP_MESSAGE_WROT_NULL_DATA, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, MDP_MESSAGE_WROT_INVALID_FORMAT, ISP_TPIPE_MESSAGE_FAIL)\
    /* WDMA check */\
    CMD(n, MDP_MESSAGE_WDMA_NULL_DATA, ISP_TPIPE_MESSAGE_FAIL)\
    /* JPEGENC check */\
    CMD(n, MDP_MESSAGE_JPEGENC_NULL_DATA, ISP_TPIPE_MESSAGE_FAIL)\
    /* General status */\
    CMD(n, MDP_MESSAGE_INVALID_STATE, ISP_TPIPE_MESSAGE_FAIL)\
    CMD(n, MDP_MESSAGE_UNKNOWN_ERROR, ISP_TPIPE_MESSAGE_FAIL)\

#define MDP_TILE_INTERNAL_TDR_REG_LUT(CMD, a, b, c, d, e)\
    CMD(a, b, c, d, e, int, BACKUP_RDMA_RING_BUF_READ,, BACKUP_RDMA_RING_BUF_READ,, false,,)

#define MDP_TILE_INTERNAL_REG_LUT(CMD, a, b, c, d, e)
#define MDP_TILE_CMODEL_REG_LUT(CMD, a, b, c, d, e)
#define MDP_TILE_PLATFORM_REG_LUT(CMD, a, b, c, d, e) \
    /* to add register only support by platform */\
	/* force to disable isp tdr direct-link */\
	CMD(a, b, c, d, e, bool, disable_isp_direct_link,, disable_isp_direct_link,, true,,)\

#define MDP_TILE_PLATFORM_DEBUG_REG_LUT(CMD, a, b, c, d, e)\
    /* to add register only support by platform */\
    CMD(a, b, c, d, e, int,,, platform_mdp_hex_no_per_tpipe,,,,)\


#define MDP_TILE_CMODEL_PATH_LUT(CMD, a, b, c, d, e)

/* register table (Cmodel, platform, tile driver) for SW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
#define MDP_TILE_SW_REG_LUT(CMD, a, b, c, d, e) \
    /* RDMA input width & height */\
    CMD(a, b, c, d, e, int, mdp_src_width, , mdp_tile_src_width,, true,,)\
    CMD(a, b, c, d, e, int, mdp_src_height, , mdp_tile_src_height,, true,,)\
    CMD(a, b, c, d, e, int, mdp_tpipe_width, , mdp_tile_width,, true,,)\
    CMD(a, b, c, d, e, int, mdp_tpipe_height, , mdp_tile_height,, true,,)\


/* register table (Cmodel, platform, tile driver) for HW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* register name of current c model */
/* register name of HW ISP & platform parameters */
/* internal variable name of tile */
/* array bracket [xx] */
/* valid condition by tdr_en to print platform log with must string, default: false */
/* isp_reg.h reg name */
/* isp_reg.h field name */
/* direct-link param 0: must be equal, 1: replaced by MDP, 2: don't care, 4: shold compare isp_reg and reg map in program */
#define MDP_TILE_HW_REG_LUT(CMD, a, b, c, d, e) \
    /* Common */\
    /* module enable register */\
    CMD(a, b, c, d, e, int, enable_ISP,, CAMIN_EN,, true,,)\
    CMD(a, b, c, d, e, int, enable_MDP_RDMA0,, RDMA0_EN,, true,,)\
    CMD(a, b, c, d, e, int, enable_MDP_PRZ0,, PRZ0_EN,, true,,)\
    CMD(a, b, c, d, e, int, enable_MDP_PRZ1,, PRZ1_EN,, true,,)\
    CMD(a, b, c, d, e, int, enable_MDP_TDSHP0,, TDSHP0_EN,, true,,)\
    CMD(a, b, c, d, e, int, enable_MDP_WROT0,, WROT0_EN,, true,,)\
    CMD(a, b, c, d, e, int, enable_MDP_WDMA,, WDMA_EN,, true,,)\
    /* MUX - mout */\
    CMD(a, b, c, d, e, int, DISP_isp_mout_en, MMSYS_ISP_MOUT_EN, CAMIN_OUT,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_rdma0_mout_en, MMSYS_MDP_RDMA0_MOUT_EN, RDMA0_OUT,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_prz0_mout_en, MMSYS_MDP_PRZ0_MOUT_EN, PRZ0_OUT,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_prz1_mout_en, MMSYS_MDP_PRZ1_MOUT_EN, PRZ1_OUT,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_tdshp0_mout_en, MMSYS_MDP_TDSHP0_MOUT_EN, TDSHP0_OUT,, true,,)\
    /* MUX - sel in */\
    CMD(a, b, c, d, e, int, DISP_mdp_prz0_sel, MMSYS_MDP_PRZ0_SEL_IN, PRZ0_SEL,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_prz1_sel, MMSYS_MDP_PRZ1_SEL_IN, PRZ1_SEL,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_tdshp0_sel, MMSYS_MDP_TDSHP0_SEL_IN, TDSHP0_SEL,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_wdma_sel, MMSYS_MDP_WDMA_SEL_IN, WDMA_SEL,, true,,)\
    CMD(a, b, c, d, e, int, DISP_mdp_wrot0_sel, MMSYS_MDP_WROT0_SEL_IN, WROT0_SEL,, true,,)\
	CMD(a, b, c, d, e, int, RDMA0_ring_buf_read, , RDMA_RING_BUF_READ,, true,,)\


typedef struct TILE_FUNC_DATA_STRUCT
{
} TILE_FUNC_DATA_STRUCT;



typedef struct MDP_RDMA_DATA : public TILE_FUNC_DATA_STRUCT
{
    DpColorFormat       m_sourceColorFormat;
    bool                m_isRingBufferMode;
    DpRingBufferPool    *m_pRingBufferPool;
    pullBufferDataCB    m_pPullDataFunc;
    fitConstraintCB     m_pFitConstraintFunc;
    bool                m_isVideoBlockMode;
    uint32_t            m_videoBlockShiftW;
    uint32_t            m_videoBlockShiftH;
    uint32_t            m_cropOffsetX;
    uint32_t            m_cropOffsetY;
    uint32_t            m_cropWidth;
    uint32_t            m_cropHeight;
    uint32_t            m_identifier;

    MDP_RDMA_DATA()
        : m_sourceColorFormat(DP_COLOR_RGB888),
          m_isRingBufferMode(false),
          m_pRingBufferPool(NULL),
          m_pPullDataFunc(NULL),
          m_pFitConstraintFunc(NULL),
          m_isVideoBlockMode(false),
          m_videoBlockShiftW(0),
          m_videoBlockShiftH(0),
          m_cropOffsetX(0),
          m_cropOffsetY(0),
          m_cropWidth(0),
          m_cropHeight(0),
          m_identifier(0)
    {
    }
} MDP_RDMA_DATA;


typedef struct MDP_PRZ_DATA : public TILE_FUNC_DATA_STRUCT
{
    bool                        m_use121filter;
    uint32_t                    m_coeffStepX;
    uint32_t                    m_coeffStepY;
    uint32_t                    m_precisionX;
    uint32_t                    m_precisionY;

    int32_t                     m_cropOffsetX;
    int32_t                     m_cropSubpixX;
    int32_t                     m_cropWidth;
    int32_t                     m_cropOffsetY;
    int32_t                     m_cropSubpixY;
    int32_t                     m_cropHeight;

    bool                        m_horDirScale;
    DP_TILE_SCALER_ALGO_ENUM    m_horAlgorithm;
    bool                        m_verDirScale;
    DP_TILE_SCALER_ALGO_ENUM    m_verAlgorithm;
    int32_t                     m_C42OutFrameW;
    int32_t                     m_C24InFrameW;
    int32_t                     m_rszOutTileW;
    int32_t                     m_rszBackXLeft;
    int32_t                     m_rszBackXRight;

    MDP_PRZ_DATA()
        : m_use121filter(true),
          m_coeffStepX(0),
          m_coeffStepY(0),
          m_precisionX(0),
          m_precisionY(0),
          m_cropOffsetX(0),
          m_cropSubpixX(0),
          m_cropWidth(0),
          m_cropOffsetY(0),
          m_cropSubpixY(0),
          m_cropHeight(0),
          m_horDirScale(false),
          m_horAlgorithm(static_cast<DP_TILE_SCALER_ALGO_ENUM>(0)),
          m_verDirScale(false),
          m_verAlgorithm(static_cast<DP_TILE_SCALER_ALGO_ENUM>(0)),
          m_C42OutFrameW(0),
          m_C24InFrameW(0),
          m_rszOutTileW(0),
          m_rszBackXLeft(0),
          m_rszBackXRight(0)
    {
    }
} MDP_PRZ_DATA;

typedef struct MDP_TDSHP_DATA : public TILE_FUNC_DATA_STRUCT
{
    bool m_HFGEnable;

    MDP_TDSHP_DATA()
        : m_HFGEnable(false)
    {
    }
}MDP_TDSHP_DATA;

typedef struct MDP_WROT_DATA : public TILE_FUNC_DATA_STRUCT
{
    DpColorFormat   m_targetFormat;
    uint32_t        m_rotationAngle;
    uint32_t        m_FIFOMaxSize;
    uint32_t        m_maxLineCount;
    bool            m_enAlphaRot;
    uint32_t        m_identifier;
    bool            m_flipHorizontal;

    MDP_WROT_DATA()
        : m_targetFormat(DP_COLOR_RGB888),
          m_rotationAngle(0),
          m_FIFOMaxSize(0),
          m_maxLineCount(0),
          m_enAlphaRot(false),
          m_identifier(0),
          m_flipHorizontal(false)
    {
    }
} MDP_WROT_DATA;


typedef struct MDP_WDMA_DATA : public TILE_FUNC_DATA_STRUCT
{
    DpColorFormat   m_targetFormat;

    MDP_WDMA_DATA()
        : m_targetFormat(DP_COLOR_RGB888)
    {
    }
} MDP_WDMA_DATA;


#define JENC_MCU_WIDTH (16)

#define JENC_MAX_TILE_COL_NUM (4096)
#define JENC_MAX_TILE_ROW_NUM (8)
#define JENC_MIN_TILE_ROW_NUM (3)

#define JENC_MAX_TILE_WIDTH (768)
#define JENC_MIN_TILE_WIDTH (256)
#define JENC_WORST_CR (3/2)

#define JENC_TBS_DATA_UNIT_SIZE (2)   /*YUV*/
#define JENC_TSI_DATA_UNIT_SIZE (16)

#define JENC_WRITE_DATA_UNIT (128)
#define JENC_ADDR_ALIGN_BYTE (16)

#define IS_R0(x)   ((x) == 0x00)
#define IS_R90(x)  ((x) == 0x01)
#define IS_R180(x) ((x) == 0x02)
#define IS_R270(x) ((x) == 0x03)

#define IS_ROT_HOR(x)   (  IS_R0(x) || IS_R180(x) )
#define IS_ROT_VER(x)   ( IS_R90(x) || IS_R270(x) )

#define IS_ROT_R0_R90(x)   (  IS_R0(x) || IS_R90(x) )
#define IS_ROT_R180_R270(x)   (  IS_R180(x) || IS_R270(x) )

#define TO_CEIL(x,a) ( ( (unsigned int)(x) + ((a)-1)) & ~((a)-1) )

#define TO_FLOOR(x,a) ( (unsigned int)(x) & ~((a)-1) )

//#define TO_DIV_CEIL(frame,first_tile) (( (frame)/(first_tile) ) + ( (frame)%(first_tile) )? 1 : 0 )
#define TO_DIV_CEIL(x,a) ( ( (x)/(a) ) + ( ( (x)%(a) )? 1 : 0 ) )

#define TO_MASK(x,a) ( (x) & (a) )

typedef struct MDP_JPEGENC_DATA : public TILE_FUNC_DATA_STRUCT
{
    DpColorFormat   m_targetFormat;
    uint32_t jenc_REAL_tbs_buf_size;
    uint32_t jenc_rotate;
    uint32_t jenc_tbs_compress_ratio;
    uint32_t jenc_cfg_ttp1_mcu_height;

    MDP_JPEGENC_DATA()
        : m_targetFormat(DP_COLOR_I420),
          jenc_REAL_tbs_buf_size(0),
          jenc_rotate(0),
          jenc_tbs_compress_ratio(0),
          jenc_cfg_ttp1_mcu_height(0)
    {
    }
} MDP_JPEGENC_DATA;

#endif

