#ifndef __CCU_LTM_INTERFACE__
#define __CCU_LTM_INTERFACE__

#include "ccu_ext_interface/ccu_types.h"
#include "algorithm/ccu_ltm_param.h"
#include "algorithm/ccu_ltm_nvram.h"
#include "algorithm/ccu_hlr_nvram.h"

#define CCU_LTM_INSTANCE_MAX 2

/******************************************************************************
* LTM Command ID definition
******************************************************************************/
enum ccu_msg_id_ltm
{
    /*To identify ccu control msg count*/
    DELIMITER_LTM_MSG_MIN = 0,
    /*Receive by CCU*/
    MSG_TO_CCU_LTM_INIT = DELIMITER_LTM_MSG_MIN,
    MSG_TO_CCU_LTM_PERFRAME_CTRL,
    MSG_TO_CCU_LTM_START,
    MSG_TO_CCU_LTM_STOP,
    MSG_TO_CCU_LTM_GET_INT_IDX,
    MSG_TO_CCU_HLR_START,
    MSG_TO_CCU_HLR_STOP,
    MSG_TO_CCU_FLR_START,
    MSG_TO_CCU_FLR_STOP,
    /*To identify ccu control msg count*/
    DELIMITER_LTM_MSG_1,
    /*CCU internal task generated in HW isr*/
    MSG_CCU_INTERNAL_LTM_VSYNC = DELIMITER_LTM_MSG_1,
    MSG_CCU_INTERNAL_LTM_NS_VSYNC,
    MSG_CCU_INTERNAL_LTM_SUB_FUNC1,
    MSG_CCU_INTERNAL_LTM_SUB_FUNC2,
    MSG_CCU_INTERNAL_LTM_SUB_FUNC3,
    MSG_CCU_INTERNAL_LTM_SUB_FUNC4,
    MSG_CCU_INTERNAL_LTM_CQ_DONE,
    MSG_CCU_INTERNAL_LTM_P1_DONE,
    MSG_CCU_INTERNAL_LTM_DUMP,

    /*To identify ccu control msg count*/
    DELIMITER_LTM_MSG_MAX
};

/******************************************************************************
* Interface between CCU and LTM_mgr
******************************************************************************/
#define CCU_LTMSO_RING_SIZE 6
#define CCU_LTMSO_MAX_BUF_SIZE 9600

// Should sync with ISP_MGR
#define CCU_LTM_NVRAM_NUM_MAX 60
#define CCU_LTM_NVRAM_SIZE sizeof(struct ccu_ltm_nvram)
#define CCU_HLR_NVRAM_NUM_MAX 60
#define CCU_HLR_NVRAM_SIZE sizeof(struct ccu_hlr_nvram)


#define CCU_LTM_DBG_RING_SIZE 2
#define CCU_LTM_BLK_X_NUM 12
#define CCU_LTM_BLK_Y_NUM 9
#define LTM_BIN 33

enum LTM_HW_REG
{
    LTM_HW_REG_MIN = 0,
    LTM_CTRL = LTM_HW_REG_MIN,
    LTM_BLK_NUM,
    LTM_BLK_SZ,
    LTM_BLK_DIVX,
    LTM_BLK_DIVY,
    LTM_MAX_DIV,
    LTM_CLIP,
    LTM_TILE_NUM,
    LTM_TILE_CNTX,
    LTM_TILE_CNTY,
    LTM_CFG,
    LTM_RESET,
    LTM_INTEN,
    LTM_INTSTA,
    LTM_STATUS,
    LTM_INPUT_COUNT,
    LTM_OUTPUT_COUNT,
    LTM_CHKSUM,
    LTM_TILE_SIZE,
    LTM_TILE_EDGE,
    LTM_TILE_CROP,
    LTM_DUMMY_REG,
    LTM_SRAM_CFG,
    LTM_SRAM_STATUS,
    LTM_ATPG,
    LTM_SHADOW_CTRL,
    LTM_SELRGB_GRAD0,
    LTM_SELRGB_GRAD1,
    LTM_SELRGB_GRAD2,
    LTM_SELRGB_GRAD3,
    LTM_SELRGB_TH0,
    LTM_SELRGB_TH1,
    LTM_SELRGB_TH2,
    LTM_SELRGB_TH3,
    LTM_SELRGB_SLP0,
    LTM_SELRGB_SLP1,
    LTM_SELRGB_SLP2,
    LTM_SELRGB_SLP3,
    LTM_SELRGB_SLP4,
    LTM_SELRGB_SLP5,
    LTM_SELRGB_SLP6,
    LTM_OUT_STR,
    LTM_SRAM_PINGPONG,
    LTM_HW_REG_MAX
};

enum LTMS_HW_REG
{
    LTMS_HW_REG_MIN = 0,
    LTMS_R1A_LTMS_CTRL = LTM_HW_REG_MIN,
    LTMS_R1A_LTMS_BLK_NUM,
    LTMS_R1A_LTMS_BLK_SZ,
    LTMS_R1A_LTMS_BLK_AREA,
    LTMS_R1A_LTMS_DETAIL,
    LTMS_R1A_LTMS_HIST,
    LTMS_R1A_LTMS_FLTLINE,
    LTMS_R1A_LTMS_FLTBLK,
    LTMS_R1A_LTMS_CLIP,
    LTMS_R1A_LTMS_MAX_DIV,
    LTMS_R1A_LTMS_CFG,
    LTMS_R1A_LTMS_RESET,
    LTMS_R1A_LTMS_INTEN,
    LTMS_R1A_LTMS_INTSTA,
    LTMS_R1A_LTMS_STATUS,
    LTMS_R1A_LTMS_INPUT_COUNT,
    LTMS_R1A_LTMS_OUTPUT_COUNT,
    LTMS_R1A_LTMS_CHKSUM,
    LTMS_R1A_LTMS_IN_SIZE,
    LTMS_R1A_LTMS_OUT_SIZE,
    LTMS_R1A_LTMS_ACT_WINDOW_X,
    LTMS_R1A_LTMS_ACT_WINDOW_Y,
    LTMS_R1A_LTMS_OUT_DATA_NUM,
    LTMS_R1A_LTMS_DUMMY_REG,
    LTMS_R1A_LTMS_SRAM_CFG,
    LTMS_R1A_LTMS_ATPG,
    LTMS_R1A_LTMS_SHADOW_CTRL,
    LTMS_R1A_LTMS_HIST_R,
    LTMS_R1A_LTMS_HIST_B,
    LTMS_R1A_LTMS_HIST_C,
    LTMS_R1A_LTMS_FLATLINE_R,
    LTMS_R1A_LTMS_FLATBLK_B,
    LTMS_R1A_LTMS_BLK_R_AREA,
    LTMS_R1A_LTMS_BLK_B_AREA,
    LTMS_R1A_LTMS_BLK_C_AREA,
    LTMS_HW_REG_MAX
};

struct ccu_ltm_init_data_in
{
    //Overwrite by CCU drv.
    uint32_t ltmso_ring_buf_addr[CCU_LTMSO_RING_SIZE];  //REG_LTMSO_R1_LTMSO_BASE_ADDR

    uint32_t ltm_en;
    uint32_t hlr_en;
    uint32_t ltm_nvram_size;
    uint32_t hlr_nvram_size;
    uint32_t ltm_nvram_num;
    uint32_t hlr_nvram_num;
    void *ltm_nvram_addr;
    void *hlr_nvram_addr;
};

// Structure of MSG_TO_CCU_LTM_PERFRAME_CTRL
struct ccu_ltm_perframe_data_in
{
    // Only used in Custom AE
    struct ccu_ltm_ae_info_t ae_info;

    uint32_t magic_number;
    uint32_t request_number;
    uint32_t ltm_nvram_idx;
    uint32_t hlr_nvram_idx;
    uint32_t ltm_en;
    uint32_t hlr_en;
    
    // HLR parameter
    uint32_t hlr_cct;
};

struct ccu_ltm_info_isp
{
    uint32_t ltm_curve[CCU_LTM_BLK_Y_NUM][CCU_LTM_BLK_X_NUM][LTM_BIN];
    uint32_t frame_idx;
    uint32_t ltm_en;
    uint32_t ltmso_buffer_addr;
    uint32_t ltmso_size;
    uint32_t ltm_ct;
    uint32_t ltm_hw_reg[2][LTM_HW_REG_MAX];
    uint32_t ltms_hw_reg[LTMS_HW_REG_MAX];
    void *ltmso_buffer_addr_va;
};

struct ccu_hlr_info_isp
{
    uint32_t frame_idx;
    uint32_t hlr_en;
    struct hlr_default hlr_out;
};

struct ccu_ltm_debug_info_handle
{
    uint32_t ltm_info_exif_addr[CCU_LTM_DBG_RING_SIZE];
    uint32_t ltm_info_isp_addr[CCU_LTM_DBG_RING_SIZE];
    uint32_t ltm_info_widx;
    uint32_t ltm_info_ridx;
    uint32_t hlr_info_exif_addr[CCU_LTM_DBG_RING_SIZE];
    uint32_t hlr_info_isp_addr[CCU_LTM_DBG_RING_SIZE];
    uint32_t hlr_info_widx;
    uint32_t hlr_info_ridx;
};

struct ccu_ltm_instance_mapping_handle
{
    uint32_t instance_idx;
};

#endif
