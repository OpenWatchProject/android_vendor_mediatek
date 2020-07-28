#include "DpWrapper_WPE.h"
#include "mdp_reg_rdma.h"
#include "mmsys_config.h"
#include "dip_reg.h"
#include "camera_wpe.h"


/* WPE unmapped base address macro for GCE to access */
//#define WPE_BASE_HW   0x1502a000

#define WPE_WPE_START_HW                      (WPE_BASE_HW)
#define WPE_CTL_MOD_EN_HW                     (WPE_BASE_HW + 0x0004)
#define WPE_CTL_DMA_EN_HW                     (WPE_BASE_HW + 0x0008)

#define WPE_CTL_CFG_HW                        (WPE_BASE_HW + 0x0010)
#define WPE_CTL_FMT_SEL_HW                    (WPE_BASE_HW + 0x0014)
#define WPE_CTL_INT_EN_HW                     (WPE_BASE_HW + 0x0018)

#define WPE_CTL_INT_STATUS_HW                 (WPE_BASE_HW + 0x0020)
#define WPE_CTL_INT_STATUSX_HW                (WPE_BASE_HW + 0x0024)
#define WPE_CTL_TDR_TILE_HW                   (WPE_BASE_HW + 0x0028)
#define WPE_CTL_TDR_DBG_STATUS_HW             (WPE_BASE_HW + 0x002C)
#define WPE_CTL_TDR_TCM_EN_HW                 (WPE_BASE_HW + 0x0030)
#define WPE_CTL_SW_CTL_HW                     (WPE_BASE_HW + 0x0034)
#define WPE_CTL_SPARE0_HW                     (WPE_BASE_HW + 0x0038)
#define WPE_CTL_SPARE1_HW                     (WPE_BASE_HW + 0x003C)
#define WPE_CTL_SPARE2_HW                     (WPE_BASE_HW + 0x0040)
#define WPE_CTL_DONE_SEL_HW                   (WPE_BASE_HW + 0x0044)
#define WPE_CTL_DBG_SET_HW                    (WPE_BASE_HW + 0x0048)
#define WPE_CTL_DBG_PORT_HW                   (WPE_BASE_HW + 0x004C)
#define WPE_CTL_DATE_CODE_HW                  (WPE_BASE_HW + 0x0050)
#define WPE_CTL_PROJ_CODE_HW                  (WPE_BASE_HW + 0x0054)
#define WPE_CTL_WPE_DCM_DIS_HW                (WPE_BASE_HW + 0x0058)
#define WPE_CTL_DMA_DCM_DIS_HW                (WPE_BASE_HW + 0x005C)
#define WPE_CTL_WPE_DCM_STATUS_HW             (WPE_BASE_HW + 0x0060)
#define WPE_CTL_DMA_DCM_STATUS_HW             (WPE_BASE_HW + 0x0064)
#define WPE_CTL_WPE_REQ_STATUS_HW             (WPE_BASE_HW + 0x0068)
#define WPE_CTL_DMA_REQ_STATUS_HW             (WPE_BASE_HW + 0x006C)
#define WPE_CTL_WPE_RDY_STATUS_HW             (WPE_BASE_HW + 0x0070)
#define WPE_CTL_DMA_RDY_STATUS_HW             (WPE_BASE_HW + 0x0074)

#define WPE_VGEN_CTL_HW                       (WPE_BASE_HW + 0x00C0)
#define WPE_VGEN_IN_IMG_HW                    (WPE_BASE_HW + 0x00C4)
#define WPE_VGEN_OUT_IMG_HW                   (WPE_BASE_HW + 0x00C8)
#define WPE_VGEN_HORI_STEP_HW                 (WPE_BASE_HW + 0x00CC)
#define WPE_VGEN_VERT_STEP_HW                 (WPE_BASE_HW + 0x00D0)
#define WPE_VGEN_HORI_INT_OFST_HW             (WPE_BASE_HW + 0x00D4)
#define WPE_VGEN_HORI_SUB_OFST_HW             (WPE_BASE_HW + 0x00D8)
#define WPE_VGEN_VERT_INT_OFST_HW             (WPE_BASE_HW + 0x00DC)
#define WPE_VGEN_VERT_SUB_OFST_HW             (WPE_BASE_HW + 0x00E0)

#define WPE_VGEN_POST_CTL_HW                  (WPE_BASE_HW + 0x00E8)
#define WPE_VGEN_POST_COMP_X_HW               (WPE_BASE_HW + 0x00EC)
#define WPE_VGEN_POST_COMP_Y_HW               (WPE_BASE_HW + 0x00F0)
#define WPE_VGEN_MAX_VEC_HW                   (WPE_BASE_HW + 0x00F4)
#define WPE_VFIFO_CTL_HW                      (WPE_BASE_HW + 0x00F8)

#define WPE_CFIFO_CTL_HW                      (WPE_BASE_HW + 0x0140)

#define WPE_RWCTL_CTL_HW                      (WPE_BASE_HW + 0x0150)

#define WPE_CACHI_SPECIAL_FUN_EN_HW           (WPE_BASE_HW + 0x0160)

#define WPE_C24_TILE_EDGE_HW                  (WPE_BASE_HW + 0x0170)

#define WPE_MDP_CROP_X_HW                     (WPE_BASE_HW + 0x0190)
#define WPE_MDP_CROP_Y_HW                     (WPE_BASE_HW + 0x0194)

#define WPE_ISPCROP_CON1_HW                   (WPE_BASE_HW + 0x01C0)
#define WPE_ISPCROP_CON2_HW                   (WPE_BASE_HW + 0x01C4)

#define WPE_PSP_CTL_HW                        (WPE_BASE_HW + 0x01F0)
#define WPE_PSP2_CTL_HW                       (WPE_BASE_HW + 0x01F4)

#define WPE_ADDR_GEN_SOFT_RSTSTAT_0_HW        (WPE_BASE_HW + 0x02C0)
#define WPE_ADDR_GEN_BASE_ADDR_0_HW           (WPE_BASE_HW + 0x02C4)
#define WPE_ADDR_GEN_OFFSET_ADDR_0_HW         (WPE_BASE_HW + 0x02C8)
#define WPE_ADDR_GEN_STRIDE_0_HW              (WPE_BASE_HW + 0x02CC)
#define WPE_CACHI_CON_0_HW                    (WPE_BASE_HW + 0x02D0)
#define WPE_CACHI_CON2_0_HW                   (WPE_BASE_HW + 0x02D4)
#define WPE_CACHI_CON3_0_HW                   (WPE_BASE_HW + 0x02D8)
#define WPE_ADDR_GEN_ERR_CTRL_0_HW            (WPE_BASE_HW + 0x02DC)
#define WPE_ADDR_GEN_ERR_STAT_0_HW            (WPE_BASE_HW + 0x02E0)
#define WPE_ADDR_GEN_RSV1_0_HW                (WPE_BASE_HW + 0x02E4)
#define WPE_ADDR_GEN_DEBUG_SEL_0_HW           (WPE_BASE_HW + 0x02E8)

#define WPE_ADDR_GEN_SOFT_RSTSTAT_1_HW        (WPE_BASE_HW + 0x02F0)
#define WPE_ADDR_GEN_BASE_ADDR_1_HW           (WPE_BASE_HW + 0x02F4)
#define WPE_ADDR_GEN_OFFSET_ADDR_1_HW         (WPE_BASE_HW + 0x02F8)
#define WPE_ADDR_GEN_STRIDE_1_HW              (WPE_BASE_HW + 0x02FC)
#define WPE_CACHI_CON_1_HW                    (WPE_BASE_HW + 0x0300)
#define WPE_CACHI_CON2_1_HW                   (WPE_BASE_HW + 0x0304)
#define WPE_CACHI_CON3_1_HW                   (WPE_BASE_HW + 0x0308)
#define WPE_ADDR_GEN_ERR_CTRL_1_HW            (WPE_BASE_HW + 0x030C)
#define WPE_ADDR_GEN_ERR_STAT_1_HW            (WPE_BASE_HW + 0x0310)
#define WPE_ADDR_GEN_RSV1_1_HW                (WPE_BASE_HW + 0x0314)
#define WPE_ADDR_GEN_DEBUG_SEL_1_HW           (WPE_BASE_HW + 0x0318)

#define WPE_ADDR_GEN_SOFT_RSTSTAT_2_HW        (WPE_BASE_HW + 0x0320)
#define WPE_ADDR_GEN_BASE_ADDR_2_HW           (WPE_BASE_HW + 0x0324)
#define WPE_ADDR_GEN_OFFSET_ADDR_2_HW         (WPE_BASE_HW + 0x0328)
#define WPE_ADDR_GEN_STRIDE_2_HW              (WPE_BASE_HW + 0x032C)
#define WPE_CACHI_CON_2_HW                    (WPE_BASE_HW + 0x0330)
#define WPE_CACHI_CON2_2_HW                   (WPE_BASE_HW + 0x0334)
#define WPE_CACHI_CON3_2_HW                   (WPE_BASE_HW + 0x0338)
#define WPE_ADDR_GEN_ERR_CTRL_2_HW            (WPE_BASE_HW + 0x033C)
#define WPE_ADDR_GEN_ERR_STAT_2_HW            (WPE_BASE_HW + 0x0340)
#define WPE_ADDR_GEN_RSV1_2_HW                (WPE_BASE_HW + 0x0344)
#define WPE_ADDR_GEN_DEBUG_SEL_2_HW           (WPE_BASE_HW + 0x0348)

#define WPE_ADDR_GEN_SOFT_RSTSTAT_3_HW        (WPE_BASE_HW + 0x0350)
#define WPE_ADDR_GEN_BASE_ADDR_3_HW           (WPE_BASE_HW + 0x0354)
#define WPE_ADDR_GEN_OFFSET_ADDR_3_HW         (WPE_BASE_HW + 0x0358)
#define WPE_ADDR_GEN_STRIDE_3_HW              (WPE_BASE_HW + 0x035C)
#define WPE_CACHI_CON_3_HW                    (WPE_BASE_HW + 0x0360)
#define WPE_CACHI_CON2_3_HW                   (WPE_BASE_HW + 0x0364)
#define WPE_CACHI_CON3_3_HW                   (WPE_BASE_HW + 0x0368)
#define WPE_ADDR_GEN_ERR_CTRL_3_HW            (WPE_BASE_HW + 0x036C)
#define WPE_ADDR_GEN_ERR_STAT_3_HW            (WPE_BASE_HW + 0x0370)
#define WPE_ADDR_GEN_RSV1_3_HW                (WPE_BASE_HW + 0x0374)
#define WPE_ADDR_GEN_DEBUG_SEL_3_HW           (WPE_BASE_HW + 0x0378)

#define WPE_DMA_SOFT_RSTSTAT_HW               (WPE_BASE_HW + 0x03C0)
#define WPE_TDRI_BASE_ADDR_HW                 (WPE_BASE_HW + 0x03C4)
#define WPE_TDRI_OFST_ADDR_HW                 (WPE_BASE_HW + 0x03C8)
#define WPE_TDRI_XSIZE_HW                     (WPE_BASE_HW + 0x03CC)
#define WPE_VERTICAL_FLIP_EN_HW               (WPE_BASE_HW + 0x03D0)
#define WPE_DMA_SOFT_RESET_HW                 (WPE_BASE_HW + 0x03D4)
#define WPE_LAST_ULTRA_EN_HW                  (WPE_BASE_HW + 0x03D8)
#define WPE_SPECIAL_FUN_EN_HW                 (WPE_BASE_HW + 0x03DC)

#define WPE_WPEO_BASE_ADDR_HW                 (WPE_BASE_HW + 0x03F0)

#define WPE_WPEO_OFST_ADDR_HW                 (WPE_BASE_HW + 0x03F8)

#define WPE_WPEO_XSIZE_HW                     (WPE_BASE_HW + 0x0400)
#define WPE_WPEO_YSIZE_HW                     (WPE_BASE_HW + 0x0404)
#define WPE_WPEO_STRIDE_HW                    (WPE_BASE_HW + 0x0408)
#define WPE_WPEO_CON_HW                       (WPE_BASE_HW + 0x040C)
#define WPE_WPEO_CON2_HW                      (WPE_BASE_HW + 0x0410)
#define WPE_WPEO_CON3_HW                      (WPE_BASE_HW + 0x0414)
#define WPE_WPEO_CROP_HW                      (WPE_BASE_HW + 0x0418)

#define WPE_MSKO_BASE_ADDR_HW                 (WPE_BASE_HW + 0x0420)

#define WPE_MSKO_OFST_ADDR_HW                 (WPE_BASE_HW + 0x0428)

#define WPE_MSKO_XSIZE_HW                     (WPE_BASE_HW + 0x0430)
#define WPE_MSKO_YSIZE_HW                     (WPE_BASE_HW + 0x0434)
#define WPE_MSKO_STRIDE_HW                    (WPE_BASE_HW + 0x0438)
#define WPE_MSKO_CON_HW                       (WPE_BASE_HW + 0x043C)
#define WPE_MSKO_CON2_HW                      (WPE_BASE_HW + 0x0440)
#define WPE_MSKO_CON3_HW                      (WPE_BASE_HW + 0x0444)
#define WPE_MSKO_CROP_HW                      (WPE_BASE_HW + 0x0448)

#define WPE_VECI_BASE_ADDR_HW                 (WPE_BASE_HW + 0x0450)

#define WPE_VECI_OFST_ADDR_HW                 (WPE_BASE_HW + 0x0458)

#define WPE_VECI_XSIZE_HW                     (WPE_BASE_HW + 0x0460)
#define WPE_VECI_YSIZE_HW                     (WPE_BASE_HW + 0x0464)
#define WPE_VECI_STRIDE_HW                    (WPE_BASE_HW + 0x0468)
#define WPE_VECI_CON_HW                       (WPE_BASE_HW + 0x046C)
#define WPE_VECI_CON2_HW                      (WPE_BASE_HW + 0x0470)
#define WPE_VECI_CON3_HW                      (WPE_BASE_HW + 0x0474)

#define WPE_VEC2I_BASE_ADDR_HW                (WPE_BASE_HW + 0x0480)

#define WPE_VEC2I_OFST_ADDR_HW                (WPE_BASE_HW + 0x0488)

#define WPE_VEC2I_XSIZE_HW                    (WPE_BASE_HW + 0x0490)
#define WPE_VEC2I_YSIZE_HW                    (WPE_BASE_HW + 0x0494)
#define WPE_VEC2I_STRIDE_HW                   (WPE_BASE_HW + 0x0498)
#define WPE_VEC2I_CON_HW                      (WPE_BASE_HW + 0x049C)
#define WPE_VEC2I_CON2_HW                     (WPE_BASE_HW + 0x04A0)
#define WPE_VEC2I_CON3_HW                     (WPE_BASE_HW + 0x04A4)

#define WPE_VEC3I_BASE_ADDR_HW                (WPE_BASE_HW + 0x04B0)

#define WPE_VEC3I_OFST_ADDR_HW                (WPE_BASE_HW + 0x04B8)

#define WPE_VEC3I_XSIZE_HW                    (WPE_BASE_HW + 0x04C0)
#define WPE_VEC3I_YSIZE_HW                    (WPE_BASE_HW + 0x04C4)
#define WPE_VEC3I_STRIDE_HW                   (WPE_BASE_HW + 0x04C8)
#define WPE_VEC3I_CON_HW                      (WPE_BASE_HW + 0x04CC)
#define WPE_VEC3I_CON2_HW                     (WPE_BASE_HW + 0x04D0)
#define WPE_VEC3I_CON3_HW                     (WPE_BASE_HW + 0x04D4)

#define WPE_DMA_ERR_CTRL_HW                   (WPE_BASE_HW + 0x04E0)
#define WPE_WPEO_ERR_STAT_HW                  (WPE_BASE_HW + 0x04E4)
#define WPE_MSKO_ERR_STAT_HW                  (WPE_BASE_HW + 0x04E8)
#define WPE_VECI_ERR_STAT_HW                  (WPE_BASE_HW + 0x04EC)
#define WPE_VEC2I_ERR_STAT_HW                 (WPE_BASE_HW + 0x04F0)
#define WPE_VEC3I_ERR_STAT_HW                 (WPE_BASE_HW + 0x04F4)
#define WPE_DMA_DEBUG_ADDR_HW                 (WPE_BASE_HW + 0x04F8)
#define WPE_DMA_RSV1_HW                       (WPE_BASE_HW + 0x04FC)
#define WPE_DMA_RSV2_HW                       (WPE_BASE_HW + 0x0500)
#define WPE_DMA_RSV3_HW                       (WPE_BASE_HW + 0x0504)
#define WPE_DMA_RSV4_HW                       (WPE_BASE_HW + 0x0508)
#define WPE_DMA_DEBUG_SEL_HW                  (WPE_BASE_HW + 0x050C)

#define WPE_B_WPE_START_HW                      (WPE_B_BASE_HW)
#define WPE_B_CTL_MOD_EN_HW                     (WPE_B_BASE_HW + 0x0004)
#define WPE_B_CTL_DMA_EN_HW                     (WPE_B_BASE_HW + 0x0008)

#define WPE_B_CTL_CFG_HW                        (WPE_B_BASE_HW + 0x0010)
#define WPE_B_CTL_FMT_SEL_HW                    (WPE_B_BASE_HW + 0x0014)
#define WPE_B_CTL_INT_EN_HW                     (WPE_B_BASE_HW + 0x0018)

#define WPE_B_CTL_INT_STATUS_HW                 (WPE_B_BASE_HW + 0x0020)
#define WPE_B_CTL_INT_STATUSX_HW                (WPE_B_BASE_HW + 0x0024)
#define WPE_B_CTL_TDR_TILE_HW                   (WPE_B_BASE_HW + 0x0028)
#define WPE_B_CTL_TDR_DBG_STATUS_HW             (WPE_B_BASE_HW + 0x002C)
#define WPE_B_CTL_TDR_TCM_EN_HW                 (WPE_B_BASE_HW + 0x0030)
#define WPE_B_CTL_SW_CTL_HW                     (WPE_B_BASE_HW + 0x0034)
#define WPE_B_CTL_SPARE0_HW                     (WPE_B_BASE_HW + 0x0038)
#define WPE_B_CTL_SPARE1_HW                     (WPE_B_BASE_HW + 0x003C)
#define WPE_B_CTL_SPARE2_HW                     (WPE_B_BASE_HW + 0x0040)
#define WPE_B_CTL_DONE_SEL_HW                   (WPE_B_BASE_HW + 0x0044)
#define WPE_B_CTL_DBG_SET_HW                    (WPE_B_BASE_HW + 0x0048)
#define WPE_B_CTL_DBG_PORT_HW                   (WPE_B_BASE_HW + 0x004C)
#define WPE_B_CTL_DATE_CODE_HW                  (WPE_B_BASE_HW + 0x0050)
#define WPE_B_CTL_PROJ_CODE_HW                  (WPE_B_BASE_HW + 0x0054)
#define WPE_B_CTL_WPE_DCM_DIS_HW                (WPE_B_BASE_HW + 0x0058)
#define WPE_B_CTL_DMA_DCM_DIS_HW                (WPE_B_BASE_HW + 0x005C)
#define WPE_B_CTL_WPE_DCM_STATUS_HW             (WPE_B_BASE_HW + 0x0060)
#define WPE_B_CTL_DMA_DCM_STATUS_HW             (WPE_B_BASE_HW + 0x0064)
#define WPE_B_CTL_WPE_REQ_STATUS_HW             (WPE_B_BASE_HW + 0x0068)
#define WPE_B_CTL_DMA_REQ_STATUS_HW             (WPE_B_BASE_HW + 0x006C)
#define WPE_B_CTL_WPE_RDY_STATUS_HW             (WPE_B_BASE_HW + 0x0070)
#define WPE_B_CTL_DMA_RDY_STATUS_HW             (WPE_B_BASE_HW + 0x0074)

#define WPE_B_VGEN_CTL_HW                       (WPE_B_BASE_HW + 0x00C0)
#define WPE_B_VGEN_IN_IMG_HW                    (WPE_B_BASE_HW + 0x00C4)
#define WPE_B_VGEN_OUT_IMG_HW                   (WPE_B_BASE_HW + 0x00C8)
#define WPE_B_VGEN_HORI_STEP_HW                 (WPE_B_BASE_HW + 0x00CC)
#define WPE_B_VGEN_VERT_STEP_HW                 (WPE_B_BASE_HW + 0x00D0)
#define WPE_B_VGEN_HORI_INT_OFST_HW             (WPE_B_BASE_HW + 0x00D4)
#define WPE_B_VGEN_HORI_SUB_OFST_HW             (WPE_B_BASE_HW + 0x00D8)
#define WPE_B_VGEN_VERT_INT_OFST_HW             (WPE_B_BASE_HW + 0x00DC)
#define WPE_B_VGEN_VERT_SUB_OFST_HW             (WPE_B_BASE_HW + 0x00E0)

#define WPE_B_VGEN_POST_CTL_HW                  (WPE_B_BASE_HW + 0x00E8)
#define WPE_B_VGEN_POST_COMP_X_HW               (WPE_B_BASE_HW + 0x00EC)
#define WPE_B_VGEN_POST_COMP_Y_HW               (WPE_B_BASE_HW + 0x00F0)
#define WPE_B_VGEN_MAX_VEC_HW                   (WPE_B_BASE_HW + 0x00F4)
#define WPE_B_VFIFO_CTL_HW                      (WPE_B_BASE_HW + 0x00F8)

#define WPE_B_CFIFO_CTL_HW                      (WPE_B_BASE_HW + 0x0140)

#define WPE_B_RWCTL_CTL_HW                      (WPE_B_BASE_HW + 0x0150)

#define WPE_B_CACHI_SPECIAL_FUN_EN_HW           (WPE_B_BASE_HW + 0x0160)

#define WPE_B_C24_TILE_EDGE_HW                  (WPE_B_BASE_HW + 0x0170)

#define WPE_B_MDP_CROP_X_HW                     (WPE_B_BASE_HW + 0x0190)
#define WPE_B_MDP_CROP_Y_HW                     (WPE_B_BASE_HW + 0x0194)

#define WPE_B_ISPCROP_CON1_HW                   (WPE_B_BASE_HW + 0x01C0)
#define WPE_B_ISPCROP_CON2_HW                   (WPE_B_BASE_HW + 0x01C4)

#define WPE_B_PSP_CTL_HW                        (WPE_B_BASE_HW + 0x01F0)
#define WPE_B_PSP2_CTL_HW                       (WPE_B_BASE_HW + 0x01F4)

#define WPE_B_ADDR_GEN_SOFT_RSTSTAT_0_HW        (WPE_B_BASE_HW + 0x02C0)
#define WPE_B_ADDR_GEN_BASE_ADDR_0_HW           (WPE_B_BASE_HW + 0x02C4)
#define WPE_B_ADDR_GEN_OFFSET_ADDR_0_HW         (WPE_B_BASE_HW + 0x02C8)
#define WPE_B_ADDR_GEN_STRIDE_0_HW              (WPE_B_BASE_HW + 0x02CC)
#define WPE_B_CACHI_CON_0_HW                    (WPE_B_BASE_HW + 0x02D0)
#define WPE_B_CACHI_CON2_0_HW                   (WPE_B_BASE_HW + 0x02D4)
#define WPE_B_CACHI_CON3_0_HW                   (WPE_B_BASE_HW + 0x02D8)
#define WPE_B_ADDR_GEN_ERR_CTRL_0_HW            (WPE_B_BASE_HW + 0x02DC)
#define WPE_B_ADDR_GEN_ERR_STAT_0_HW            (WPE_B_BASE_HW + 0x02E0)
#define WPE_B_ADDR_GEN_RSV1_0_HW                (WPE_B_BASE_HW + 0x02E4)
#define WPE_B_ADDR_GEN_DEBUG_SEL_0_HW           (WPE_B_BASE_HW + 0x02E8)

#define WPE_B_ADDR_GEN_SOFT_RSTSTAT_1_HW        (WPE_B_BASE_HW + 0x02F0)
#define WPE_B_ADDR_GEN_BASE_ADDR_1_HW           (WPE_B_BASE_HW + 0x02F4)
#define WPE_B_ADDR_GEN_OFFSET_ADDR_1_HW         (WPE_B_BASE_HW + 0x02F8)
#define WPE_B_ADDR_GEN_STRIDE_1_HW              (WPE_B_BASE_HW + 0x02FC)
#define WPE_B_CACHI_CON_1_HW                    (WPE_B_BASE_HW + 0x0300)
#define WPE_B_CACHI_CON2_1_HW                   (WPE_B_BASE_HW + 0x0304)
#define WPE_B_CACHI_CON3_1_HW                   (WPE_B_BASE_HW + 0x0308)
#define WPE_B_ADDR_GEN_ERR_CTRL_1_HW            (WPE_B_BASE_HW + 0x030C)
#define WPE_B_ADDR_GEN_ERR_STAT_1_HW            (WPE_B_BASE_HW + 0x0310)
#define WPE_B_ADDR_GEN_RSV1_1_HW                (WPE_B_BASE_HW + 0x0314)
#define WPE_B_ADDR_GEN_DEBUG_SEL_1_HW           (WPE_B_BASE_HW + 0x0318)

#define WPE_B_ADDR_GEN_SOFT_RSTSTAT_2_HW        (WPE_B_BASE_HW + 0x0320)
#define WPE_B_ADDR_GEN_BASE_ADDR_2_HW           (WPE_B_BASE_HW + 0x0324)
#define WPE_B_ADDR_GEN_OFFSET_ADDR_2_HW         (WPE_B_BASE_HW + 0x0328)
#define WPE_B_ADDR_GEN_STRIDE_2_HW              (WPE_B_BASE_HW + 0x032C)
#define WPE_B_CACHI_CON_2_HW                    (WPE_B_BASE_HW + 0x0330)
#define WPE_B_CACHI_CON2_2_HW                   (WPE_B_BASE_HW + 0x0334)
#define WPE_B_CACHI_CON3_2_HW                   (WPE_B_BASE_HW + 0x0338)
#define WPE_B_ADDR_GEN_ERR_CTRL_2_HW            (WPE_B_BASE_HW + 0x033C)
#define WPE_B_ADDR_GEN_ERR_STAT_2_HW            (WPE_B_BASE_HW + 0x0340)
#define WPE_B_ADDR_GEN_RSV1_2_HW                (WPE_B_BASE_HW + 0x0344)
#define WPE_B_ADDR_GEN_DEBUG_SEL_2_HW           (WPE_B_BASE_HW + 0x0348)

#define WPE_B_ADDR_GEN_SOFT_RSTSTAT_3_HW        (WPE_B_BASE_HW + 0x0350)
#define WPE_B_ADDR_GEN_BASE_ADDR_3_HW           (WPE_B_BASE_HW + 0x0354)
#define WPE_B_ADDR_GEN_OFFSET_ADDR_3_HW         (WPE_B_BASE_HW + 0x0358)
#define WPE_B_ADDR_GEN_STRIDE_3_HW              (WPE_B_BASE_HW + 0x035C)
#define WPE_B_CACHI_CON_3_HW                    (WPE_B_BASE_HW + 0x0360)
#define WPE_B_CACHI_CON2_3_HW                   (WPE_B_BASE_HW + 0x0364)
#define WPE_B_CACHI_CON3_3_HW                   (WPE_B_BASE_HW + 0x0368)
#define WPE_B_ADDR_GEN_ERR_CTRL_3_HW            (WPE_B_BASE_HW + 0x036C)
#define WPE_B_ADDR_GEN_ERR_STAT_3_HW            (WPE_B_BASE_HW + 0x0370)
#define WPE_B_ADDR_GEN_RSV1_3_HW                (WPE_B_BASE_HW + 0x0374)
#define WPE_B_ADDR_GEN_DEBUG_SEL_3_HW           (WPE_B_BASE_HW + 0x0378)

#define WPE_B_DMA_SOFT_RSTSTAT_HW               (WPE_B_BASE_HW + 0x03C0)
#define WPE_B_TDRI_BASE_ADDR_HW                 (WPE_B_BASE_HW + 0x03C4)
#define WPE_B_TDRI_OFST_ADDR_HW                 (WPE_B_BASE_HW + 0x03C8)
#define WPE_B_TDRI_XSIZE_HW                     (WPE_B_BASE_HW + 0x03CC)
#define WPE_B_VERTICAL_FLIP_EN_HW               (WPE_B_BASE_HW + 0x03D0)
#define WPE_B_DMA_SOFT_RESET_HW                 (WPE_B_BASE_HW + 0x03D4)
#define WPE_B_LAST_ULTRA_EN_HW                  (WPE_B_BASE_HW + 0x03D8)
#define WPE_B_SPECIAL_FUN_EN_HW                 (WPE_B_BASE_HW + 0x03DC)

#define WPE_B_WPEO_BASE_ADDR_HW                 (WPE_B_BASE_HW + 0x03F0)

#define WPE_B_WPEO_OFST_ADDR_HW                 (WPE_B_BASE_HW + 0x03F8)

#define WPE_B_WPEO_XSIZE_HW                     (WPE_B_BASE_HW + 0x0400)
#define WPE_B_WPEO_YSIZE_HW                     (WPE_B_BASE_HW + 0x0404)
#define WPE_B_WPEO_STRIDE_HW                    (WPE_B_BASE_HW + 0x0408)
#define WPE_B_WPEO_CON_HW                       (WPE_B_BASE_HW + 0x040C)
#define WPE_B_WPEO_CON2_HW                      (WPE_B_BASE_HW + 0x0410)
#define WPE_B_WPEO_CON3_HW                      (WPE_B_BASE_HW + 0x0414)
#define WPE_B_WPEO_CROP_HW                      (WPE_B_BASE_HW + 0x0418)

#define WPE_B_MSKO_BASE_ADDR_HW                 (WPE_B_BASE_HW + 0x0420)

#define WPE_B_MSKO_OFST_ADDR_HW                 (WPE_B_BASE_HW + 0x0428)

#define WPE_B_MSKO_XSIZE_HW                     (WPE_B_BASE_HW + 0x0430)
#define WPE_B_MSKO_YSIZE_HW                     (WPE_B_BASE_HW + 0x0434)
#define WPE_B_MSKO_STRIDE_HW                    (WPE_B_BASE_HW + 0x0438)
#define WPE_B_MSKO_CON_HW                       (WPE_B_BASE_HW + 0x043C)
#define WPE_B_MSKO_CON2_HW                      (WPE_B_BASE_HW + 0x0440)
#define WPE_B_MSKO_CON3_HW                      (WPE_B_BASE_HW + 0x0444)
#define WPE_B_MSKO_CROP_HW                      (WPE_B_BASE_HW + 0x0448)

#define WPE_B_VECI_BASE_ADDR_HW                 (WPE_B_BASE_HW + 0x0450)

#define WPE_B_VECI_OFST_ADDR_HW                 (WPE_B_BASE_HW + 0x0458)

#define WPE_B_VECI_XSIZE_HW                     (WPE_B_BASE_HW + 0x0460)
#define WPE_B_VECI_YSIZE_HW                     (WPE_B_BASE_HW + 0x0464)
#define WPE_B_VECI_STRIDE_HW                    (WPE_B_BASE_HW + 0x0468)
#define WPE_B_VECI_CON_HW                       (WPE_B_BASE_HW + 0x046C)
#define WPE_B_VECI_CON2_HW                      (WPE_B_BASE_HW + 0x0470)
#define WPE_B_VECI_CON3_HW                      (WPE_B_BASE_HW + 0x0474)

#define WPE_B_VEC2I_BASE_ADDR_HW                (WPE_B_BASE_HW + 0x0480)

#define WPE_B_VEC2I_OFST_ADDR_HW                (WPE_B_BASE_HW + 0x0488)

#define WPE_B_VEC2I_XSIZE_HW                    (WPE_B_BASE_HW + 0x0490)
#define WPE_B_VEC2I_YSIZE_HW                    (WPE_B_BASE_HW + 0x0494)
#define WPE_B_VEC2I_STRIDE_HW                   (WPE_B_BASE_HW + 0x0498)
#define WPE_B_VEC2I_CON_HW                      (WPE_B_BASE_HW + 0x049C)
#define WPE_B_VEC2I_CON2_HW                     (WPE_B_BASE_HW + 0x04A0)
#define WPE_B_VEC2I_CON3_HW                     (WPE_B_BASE_HW + 0x04A4)

#define WPE_B_VEC3I_BASE_ADDR_HW                (WPE_B_BASE_HW + 0x04B0)

#define WPE_B_VEC3I_OFST_ADDR_HW                (WPE_B_BASE_HW + 0x04B8)

#define WPE_B_VEC3I_XSIZE_HW                    (WPE_B_BASE_HW + 0x04C0)
#define WPE_B_VEC3I_YSIZE_HW                    (WPE_B_BASE_HW + 0x04C4)
#define WPE_B_VEC3I_STRIDE_HW                   (WPE_B_BASE_HW + 0x04C8)
#define WPE_B_VEC3I_CON_HW                      (WPE_B_BASE_HW + 0x04CC)
#define WPE_B_VEC3I_CON2_HW                     (WPE_B_BASE_HW + 0x04D0)
#define WPE_B_VEC3I_CON3_HW                     (WPE_B_BASE_HW + 0x04D4)

#define WPE_B_DMA_ERR_CTRL_HW                   (WPE_B_BASE_HW + 0x04E0)
#define WPE_B_WPEO_ERR_STAT_HW                  (WPE_B_BASE_HW + 0x04E4)
#define WPE_B_MSKO_ERR_STAT_HW                  (WPE_B_BASE_HW + 0x04E8)
#define WPE_B_VECI_ERR_STAT_HW                  (WPE_B_BASE_HW + 0x04EC)
#define WPE_B_VEC2I_ERR_STAT_HW                 (WPE_B_BASE_HW + 0x04F0)
#define WPE_B_VEC3I_ERR_STAT_HW                 (WPE_B_BASE_HW + 0x04F4)
#define WPE_B_DMA_DEBUG_ADDR_HW                 (WPE_B_BASE_HW + 0x04F8)
#define WPE_B_DMA_RSV1_HW                       (WPE_B_BASE_HW + 0x04FC)
#define WPE_B_DMA_RSV2_HW                       (WPE_B_BASE_HW + 0x0500)
#define WPE_B_DMA_RSV3_HW                       (WPE_B_BASE_HW + 0x0504)
#define WPE_B_DMA_RSV4_HW                       (WPE_B_BASE_HW + 0x0508)
#define WPE_B_DMA_DEBUG_SEL_HW                  (WPE_B_BASE_HW + 0x050C)

DP_STATUS_ENUM DpWrapper_WPE::onInitEngine(DpCommand &command)
{
    DP_STATUS_ENUM status;
    int32_t        size;

    status = m_pInDataPort->getParameter(&m_ISPConfig, &size);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpWrapper_WPE: get WPE tile paramter failed\n");
        return DP_STATUS_INVALID_PARAX;
    }

    // Direct link
    if (m_ISPConfig.top.wpe_mdpcrop_en)
    {
        DPLOGI("DpWrapper_WPE: SW_RST ASYNC2\n");
        // Reset MDP_DL_ASYNC2_TX
        // Bit  4: MDP_DL_ASYNC2_TX / MDP_RELAY2
        MM_REG_WRITE(command, MMSYS_SW0_RST_B,    0x0, 0x00000010);
        MM_REG_WRITE(command, MMSYS_SW0_RST_B, 1 << 4, 0x00000010);
        // Reset MDP_DL_ASYNC2_RX
        // Bit  11: MDP_DL_ASYNC2_RX
        MM_REG_WRITE(command, MMSYS_SW1_RST_B,     0x0, 0x00000800);
        MM_REG_WRITE(command, MMSYS_SW1_RST_B, 1 << 11, 0x00000800);
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::onWaitEvent(DpCommand &command)
{
	DP_STATUS_ENUM status;
	int32_t 	   size;
	WPE_Config	  *wpecmd;

	status = m_pInDataPort->getParameter(&m_ISPConfig, &size);
	if (DP_STATUS_RETURN_SUCCESS != status)
	{
		DPLOGE("DpWrapper_WPE: get WPE tile paramter failed\n");
	    return DP_STATUS_INVALID_PARAX;
	}

	//MM_REG_WRITE_MASK(command, WPE_CACHI_SPECIAL_FUN_EN_HW, wpecmd->WPE_CACHI_SPECIAL_FUN_EN, 0xFFFFFFFF);
    if (m_ISPConfig.top.wpe_mdpcrop_en == 1)
    {
        DPLOGI("[DpWrapper_WPE::onWaitEvent]Enable WPE direct link,\n");
        //Enable WPE direct link
        if(m_identifier == 0)
        	MM_REG_WRITE_MASK(command, 0x15020030, 0x00000400, 0x00000C00); // MDP2_DL_SEL: select wpe_a for Cannon/Sylvia
        else
			MM_REG_WRITE_MASK(command, 0x15020030, 0x00000200, 0x00000200); // MDP_DL_SEL: select wpe_b for Cannon/Sylvia
    }

    if(m_identifier == 0)
	{
    	MM_REG_WRITE_MASK(command, 0x1502A000, 0x0001, 0x00000001);
		MM_REG_WAIT(command, DpCommand::WPE_FRAME_DONE);
    }
	else
	{
		MM_REG_WRITE_MASK(command, 0x1502D000, 0x0001, 0x00000001);
		MM_REG_WAIT(command, DpCommand::WPE_B_FRAME_DONE);
	}
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::onFlushBuffer(FLUSH_TYPE_ENUM type)
{
    if (FLUSH_AFTER_HW_WRITE == type)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    DPLOGI("[onFlushBuffer]+,\n");
    //m_pInDataPort->flushBuffer(CLIENT_CONSUMER);

#if CONFIG_FOR_DUMP_COMMAND
    FILE *pFile;
    pFile = fopen("./out/TDRIOutput.bin", "wb");
    fwrite(m_pTDRIBufVA, 1, m_pTileDesc->used_word_no_wpe*4, pFile); //Holmes : make the correct size=>uint word=>4 bytes
    fclose(pFile);
#endif // CONFIG_FOR_DUMP_COMMAND


#if CONFIG_FOR_VERIFY_FPGA
    uint32_t i,tmpOffset;
    pFile = fopen("./out/WPEFPGAInfo.bin","wb");
    fwrite(&m_TDRITileID, sizeof(m_TDRITileID), 1, pFile); //Holmes : make the correct size=>uint word=>4 bytes
    for(i=0;i<m_TDRITileID;i++)
    {
        tmpOffset = (m_pTileDesc->tpipe_info_wpe[i].dump_offset_no) << 2;
        fwrite(&tmpOffset, sizeof(uint32_t), 1, pFile); //Holmes : make the correct size=>uint word=>4 bytes
    }
    fclose(pFile);
#endif // CONFIG_FOR_VERIFY_FPGA


    DPLOGI("[onFlushBuffer]-,\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::onConfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    WPE_Config    *wpecmd;
    DPLOGI("[DpWrapper_WPE::onConfigFrame]+,\n");

    memset(m_pTileDesc, 0x0, sizeof(ISP_TPIPE_DESCRIPTOR_STRUCT));
    //memset(m_pTileInfo, 0x0, MAX_TILE_TOT_NO * sizeof(ISP_TPIPE_INFORMATION_STRUCT));
    //memset(m_pTdrFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));
    //memset(m_pIrqFlag, 0x0, ((MAX_TILE_TOT_NO + 31) >> 5) * sizeof(unsigned int));

	wpecmd = (WPE_Config*)m_ISPConfig.drvinfo.wpecommand;

#if CONFIG_FOR_VERIFY_FPGA
    m_TDRIBufPA = m_pTDRIBufMem->mapHWAddress(tWPEI, 0);
#else
    m_TDRIBufPA = m_ISPConfig.drvinfo.tpipeTablePa_wpe;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_TDRIBufPA);

#if CONFIG_FOR_VERIFY_FPGA
    m_pTDRIBufVA = (uint32_t*)m_pTDRIBufMem->mapSWAddress();
#else
    m_pTDRIBufVA = m_ISPConfig.drvinfo.tpipeTableVa_wpe;
#endif // CONFIG_FOR_VERIFY_FPGA
    assert(0 != m_pTDRIBufVA);

    // Initialize hex dump info
    m_pTileDesc->tpipe_config_wpe       = m_pTDRIBufVA;
    m_pTileDesc->tpipe_info_wpe         = m_pTileInfo;
    m_pTileDesc->tdr_disable_flag   = m_pTdrFlag;
    m_pTileDesc->last_irq_flag      = m_pIrqFlag;
    m_pTileDesc->total_tpipe_no     = MAX_TILE_TOT_NO;
    m_pTileDesc->total_word_no_wpe      = MAX_ISP_TILE_TDR_HEX_NO_WPE;

    //m_ISPPassType = m_ISPConfig.drvinfo.cqIdx;
    //m_backup_isp_tilewitdh = m_ISPConfig.sw.tpipe_width;
    //m_backup_isp_tileheight = m_ISPConfig.sw.tpipe_height;

#if !CONFIG_FOR_VERIFY_FPGA
    if (m_inFrameWidth != m_ISPConfig.wpe.vgen_input_crop_width)
    {
        DPLOGI("Change VECI input width from %d to %d\n",m_inFrameWidth,m_ISPConfig.wpe.vgen_input_crop_width);
        m_inFrameWidth = m_ISPConfig.wpe.vgen_input_crop_width;
    }

    if (m_inFrameHeight != m_ISPConfig.wpe.vgen_input_crop_height)
    {
        DPLOGI("Change VECI input height from %d to %d\n",m_inFrameHeight,m_ISPConfig.wpe.vgen_input_crop_height);
        m_inFrameHeight = m_ISPConfig.wpe.vgen_input_crop_height;
    }

    if (0 == m_ISPConfig.top.wpe_mdpcrop_en)
    {
        DPLOGI("Change VECI output width/height from WPE config w:%d h:%d\n",m_ISPConfig.wpe.vgen_output_width, m_ISPConfig.wpe.vgen_output_height);
        m_outFrameWidth = m_ISPConfig.wpe.vgen_output_width;
        m_outFrameHeight = m_ISPConfig.wpe.vgen_output_height;
    }

    if (0 != m_ISPConfig.drvinfo.debugRegDump)
    {
        command.setISPDebugDumpRegs(m_ISPConfig.drvinfo.debugRegDump);
        DPLOGI("setISPDebugDumpRegs: %d\n",m_ISPConfig.drvinfo.debugRegDump);
    }

	if(m_identifier == 0)
	{
		MM_REG_WRITE_MASK(command, WPE_CTL_MOD_EN_HW, wpecmd->WPE_CTL_MOD_EN, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CTL_DMA_EN_HW, wpecmd->WPE_CTL_DMA_EN, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CTL_CFG_HW, wpecmd->WPE_CTL_CFG, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CTL_FMT_SEL_HW, wpecmd->WPE_CTL_FMT_SEL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CTL_INT_EN_HW, wpecmd->WPE_CTL_INT_EN, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_CTL_INT_STATUS_HW, wpecmd->WPE_CTL_INT_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_INT_STATUSX_HW, wpecmd->WPE_CTL_INT_STATUSX, 0xFFFFFFFF);
	
	
		MM_REG_WRITE_MASK(command, WPE_CTL_TDR_TILE_HW, wpecmd->WPE_CTL_TDR_TILE, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_CTL_TDR_DBG_STATUS_HW, wpecmd->WPE_CTL_TDR_DBG_STATUS, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_CTL_TDR_TCM_EN_HW, wpecmd->WPE_CTL_TDR_TCM_EN, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_SW_CTL_HW, wpecmd->WPE_CTL_SW_CTL, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_SPARE0_HW, wpecmd->WPE_CTL_SPARE0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_SPARE1_HW, wpecmd->WPE_CTL_SPARE1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_SPARE2_HW, wpecmd->WPE_CTL_SPARE2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CTL_DONE_SEL_HW, wpecmd->WPE_CTL_DONE_SEL, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_CTL_DBG_SET_HW, wpecmd->WPE_CTL_DBG_SET, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_DBG_PORT_HW, wpecmd->WPE_CTL_DBG_PORT, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_DATE_CODE_HW, wpecmd->WPE_CTL_DATE_CODE, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_PROJ_CODE_HW, wpecmd->WPE_CTL_PROJ_CODE, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_WPE_DCM_DIS_HW, 0xFFFFFFFF, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_DMA_DCM_DIS_HW, 0xFFFFFFFF, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_WPE_DCM_STATUS_HW, wpecmd->WPE_CTL_WPE_DCM_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_DMA_DCM_STATUS_HW, wpecmd->WPE_CTL_DMA_DCM_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_WPE_REQ_STATUS_HW, wpecmd->WPE_CTL_WPE_REQ_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_DMA_REQ_STATUS_HW, wpecmd->WPE_CTL_DMA_REQ_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_WPE_RDY_STATUS_HW, wpecmd->WPE_CTL_WPE_RDY_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_CTL_DMA_RDY_STATUS_HW, wpecmd->WPE_CTL_DMA_RDY_STATUS, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_CTL_HW, wpecmd->WPE_VGEN_CTL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_IN_IMG_HW, wpecmd->WPE_VGEN_IN_IMG, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_OUT_IMG_HW, wpecmd->WPE_VGEN_OUT_IMG, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_HORI_STEP_HW, wpecmd->WPE_VGEN_HORI_STEP, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_VERT_STEP_HW, wpecmd->WPE_VGEN_VERT_STEP, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_HORI_INT_OFST_HW , wpecmd->WPE_VGEN_HORI_INT_OFST, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_HORI_SUB_OFST_HW, wpecmd->WPE_VGEN_HORI_SUB_OFST, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_VERT_INT_OFST_HW, wpecmd->WPE_VGEN_VERT_INT_OFST, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_VERT_SUB_OFST_HW, wpecmd->WPE_VGEN_VERT_SUB_OFST, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VGEN_POST_CTL_HW, wpecmd->WPE_VGEN_POST_CTL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_POST_COMP_X_HW, wpecmd->WPE_VGEN_POST_COMP_X, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_POST_COMP_Y_HW, wpecmd->WPE_VGEN_POST_COMP_Y, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VGEN_MAX_VEC_HW, wpecmd->WPE_VGEN_MAX_VEC, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VFIFO_CTL_HW, wpecmd->WPE_VFIFO_CTL, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_CFIFO_CTL_HW, wpecmd->WPE_CFIFO_CTL, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_RWCTL_CTL_HW, wpecmd->WPE_RWCTL_CTL, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_CACHI_SPECIAL_FUN_EN_HW, wpecmd->WPE_CACHI_SPECIAL_FUN_EN, 0xFFFFFFFF);
		if (m_ISPConfig.top.wpe_mdpcrop_en == 1)
		{
			MM_REG_WRITE_MASK(command, WPE_C24_TILE_EDGE_HW, wpecmd->WPE_C24_TILE_EDGE, 0xFFFFFFFF);
	
			MM_REG_WRITE_MASK(command, WPE_MDP_CROP_X_HW, wpecmd->WPE_MDP_CROP_X, 0xFFFFFFFF);
			MM_REG_WRITE_MASK(command, WPE_MDP_CROP_Y_HW, wpecmd->WPE_MDP_CROP_Y, 0xFFFFFFFF);
		}
		if (m_ISPConfig.top.wpe_ispcrop_en == 1)
		{
			MM_REG_WRITE_MASK(command, WPE_ISPCROP_CON1_HW, wpecmd->WPE_ISPCROP_CON1, 0xFFFFFFFF);
			MM_REG_WRITE_MASK(command, WPE_ISPCROP_CON2_HW, wpecmd->WPE_ISPCROP_CON2, 0xFFFFFFFF);
		}
		MM_REG_WRITE_MASK(command, WPE_PSP_CTL_HW, wpecmd->WPE_PSP_CTL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_PSP2_CTL_HW, wpecmd->WPE_PSP2_CTL, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_SOFT_RSTSTAT_0_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_BASE_ADDR_0_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_OFFSET_ADDR_0_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_STRIDE_0_HW, wpecmd->WPE_ADDR_GEN_STRIDE_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON_0_HW, wpecmd->WPE_CACHI_CON_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON2_0_HW, wpecmd->WPE_CACHI_CON2_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON3_0_HW, wpecmd->WPE_CACHI_CON3_0, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_CTRL_0_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_0, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_STAT_0_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_RSV1_0_HW, wpecmd->WPE_ADDR_GEN_RSV1_0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_DEBUG_SEL_0_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_0, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_SOFT_RSTSTAT_1_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_BASE_ADDR_1_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_OFFSET_ADDR_1_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_STRIDE_1_HW, wpecmd->WPE_ADDR_GEN_STRIDE_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON_1_HW, wpecmd->WPE_CACHI_CON_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON2_1_HW , wpecmd->WPE_CACHI_CON2_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON3_1_HW, wpecmd->WPE_CACHI_CON3_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_CTRL_1_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_STAT_1_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_RSV1_1_HW, wpecmd->WPE_ADDR_GEN_RSV1_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_DEBUG_SEL_1_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_1, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_SOFT_RSTSTAT_2_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_BASE_ADDR_2_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_OFFSET_ADDR_2_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_STRIDE_2_HW, wpecmd->WPE_ADDR_GEN_STRIDE_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON_2_HW, wpecmd->WPE_CACHI_CON_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON2_2_HW , wpecmd->WPE_CACHI_CON2_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON3_2_HW, wpecmd->WPE_CACHI_CON3_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_CTRL_2_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_STAT_2_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_RSV1_2_HW, wpecmd->WPE_ADDR_GEN_RSV1_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_DEBUG_SEL_2_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_2, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_SOFT_RSTSTAT_3_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_BASE_ADDR_3_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_OFFSET_ADDR_3_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_STRIDE_3_HW, wpecmd->WPE_ADDR_GEN_STRIDE_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON_3_HW, wpecmd->WPE_CACHI_CON_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON2_3_HW , wpecmd->WPE_CACHI_CON2_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_CACHI_CON3_3_HW, wpecmd->WPE_CACHI_CON3_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_CTRL_3_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_ERR_STAT_3_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_RSV1_3_HW, wpecmd->WPE_ADDR_GEN_RSV1_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_ADDR_GEN_DEBUG_SEL_3_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_3, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_DMA_SOFT_RSTSTAT_HW, wpecmd->WPE_DMA_SOFT_RSTSTAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_TDRI_BASE_ADDR_HW, wpecmd->WPE_TDRI_BASE_ADDR, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_TDRI_OFST_ADDR_HW, wpecmd->WPE_TDRI_OFST_ADDR, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_TDRI_XSIZE_HW, wpecmd->WPE_TDRI_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VERTICAL_FLIP_EN_HW, wpecmd->WPE_VERTICAL_FLIP_EN, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_DMA_SOFT_RESET_HW, wpecmd->WPE_DMA_SOFT_RESET, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_LAST_ULTRA_EN_HW, wpecmd->WPE_LAST_ULTRA_EN, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_SPECIAL_FUN_EN_HW, wpecmd->WPE_SPECIAL_FUN_EN, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_WPEO_BASE_ADDR_HW, wpecmd->WPE_WPEO_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_WPEO_OFST_ADDR_HW, wpecmd->WPE_WPEO_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_WPEO_XSIZE_HW, wpecmd->WPE_WPEO_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_WPEO_YSIZE_HW, wpecmd->WPE_WPEO_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_WPEO_STRIDE_HW, wpecmd->WPE_WPEO_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_WPEO_CON_HW, wpecmd->WPE_WPEO_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_WPEO_CON2_HW, wpecmd->WPE_WPEO_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_WPEO_CON3_HW, wpecmd->WPE_WPEO_CON3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_WPEO_CROP_HW, wpecmd->WPE_WPEO_CROP, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_MSKO_BASE_ADDR_HW, wpecmd->WPE_MSKO_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_MSKO_OFST_ADDR_HW, wpecmd->WPE_MSKO_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_MSKO_XSIZE_HW, wpecmd->WPE_MSKO_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_MSKO_YSIZE_HW, wpecmd->WPE_MSKO_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_MSKO_STRIDE_HW, wpecmd->WPE_MSKO_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_MSKO_CON_HW, wpecmd->WPE_MSKO_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_MSKO_CON2_HW, wpecmd->WPE_MSKO_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_MSKO_CON3_HW, wpecmd->WPE_MSKO_CON3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_MSKO_CROP_HW, wpecmd->WPE_MSKO_CROP, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VECI_BASE_ADDR_HW, wpecmd->WPE_VECI_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VECI_OFST_ADDR_HW, wpecmd->WPE_VECI_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VECI_XSIZE_HW, wpecmd->WPE_VECI_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VECI_YSIZE_HW, wpecmd->WPE_VECI_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VECI_STRIDE_HW, wpecmd->WPE_VECI_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VECI_CON_HW, wpecmd->WPE_VECI_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VECI_CON2_HW, wpecmd->WPE_VECI_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VECI_CON3_HW, wpecmd->WPE_VECI_CON3, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VEC2I_BASE_ADDR_HW, wpecmd->WPE_VEC2I_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VEC2I_OFST_ADDR_HW, wpecmd->WPE_VEC2I_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VEC2I_XSIZE_HW, wpecmd->WPE_VEC2I_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC2I_YSIZE_HW, wpecmd->WPE_VEC2I_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC2I_STRIDE_HW, wpecmd->WPE_VEC2I_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC2I_CON_HW, wpecmd->WPE_VEC2I_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC2I_CON2_HW, wpecmd->WPE_VEC2I_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC2I_CON3_HW, wpecmd->WPE_VEC2I_CON3, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VEC3I_BASE_ADDR_HW, wpecmd->WPE_VEC3I_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VEC3I_OFST_ADDR_HW, wpecmd->WPE_VEC3I_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_VEC3I_XSIZE_HW, wpecmd->WPE_VEC3I_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC3I_YSIZE_HW, wpecmd->WPE_VEC3I_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC3I_STRIDE_HW, wpecmd->WPE_VEC3I_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC3I_CON_HW, wpecmd->WPE_VEC3I_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC3I_CON2_HW, wpecmd->WPE_VEC3I_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC3I_CON3_HW, wpecmd->WPE_VEC3I_CON3, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_DMA_ERR_CTRL_HW , wpecmd->WPE_DMA_ERR_CTRL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_WPEO_ERR_STAT_HW, wpecmd->WPE_WPEO_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_MSKO_ERR_STAT_HW, wpecmd->WPE_MSKO_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VECI_ERR_STAT_HW, wpecmd->WPE_VECI_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC2I_ERR_STAT_HW, wpecmd->WPE_VEC2I_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_VEC3I_ERR_STAT_HW, wpecmd->WPE_VEC3I_ERR_STAT, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_DMA_DEBUG_ADDR_HW, wpecmd->WPE_DMA_DEBUG_ADDR, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_DMA_DEBUG_SEL_HW, wpecmd->WPE_DMA_DEBUG_SEL, 0xFFFFFFFF);
	}
	else
	{
		MM_REG_WRITE_MASK(command, WPE_B_CTL_MOD_EN_HW, wpecmd->WPE_CTL_MOD_EN, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CTL_DMA_EN_HW, wpecmd->WPE_CTL_DMA_EN, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CTL_CFG_HW, wpecmd->WPE_CTL_CFG, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CTL_FMT_SEL_HW, wpecmd->WPE_CTL_FMT_SEL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CTL_INT_EN_HW, wpecmd->WPE_CTL_INT_EN, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_INT_STATUS_HW, wpecmd->WPE_CTL_INT_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_INT_STATUSX_HW, wpecmd->WPE_CTL_INT_STATUSX, 0xFFFFFFFF);
	
	
		MM_REG_WRITE_MASK(command, WPE_B_CTL_TDR_TILE_HW, wpecmd->WPE_CTL_TDR_TILE, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_TDR_DBG_STATUS_HW, wpecmd->WPE_CTL_TDR_DBG_STATUS, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_CTL_TDR_TCM_EN_HW, wpecmd->WPE_CTL_TDR_TCM_EN, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_SW_CTL_HW, wpecmd->WPE_CTL_SW_CTL, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_SPARE0_HW, wpecmd->WPE_CTL_SPARE0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_SPARE1_HW, wpecmd->WPE_CTL_SPARE1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_SPARE2_HW, wpecmd->WPE_CTL_SPARE2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CTL_DONE_SEL_HW, wpecmd->WPE_CTL_DONE_SEL, 0xFFFFFFFF);
	
	
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_DBG_SET_HW, wpecmd->WPE_CTL_DBG_SET, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_DBG_PORT_HW, wpecmd->WPE_CTL_DBG_PORT, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_DATE_CODE_HW, wpecmd->WPE_CTL_DATE_CODE, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_PROJ_CODE_HW, wpecmd->WPE_CTL_PROJ_CODE, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_WPE_DCM_DIS_HW, 0xFFFFFFFF, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_DMA_DCM_DIS_HW, 0xFFFFFFFF, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_WPE_DCM_STATUS_HW, wpecmd->WPE_CTL_WPE_DCM_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_DMA_DCM_STATUS_HW, wpecmd->WPE_CTL_DMA_DCM_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_WPE_REQ_STATUS_HW, wpecmd->WPE_CTL_WPE_REQ_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_DMA_REQ_STATUS_HW, wpecmd->WPE_CTL_DMA_REQ_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_WPE_RDY_STATUS_HW, wpecmd->WPE_CTL_WPE_RDY_STATUS, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_CTL_DMA_RDY_STATUS_HW, wpecmd->WPE_CTL_DMA_RDY_STATUS, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_CTL_HW, wpecmd->WPE_VGEN_CTL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_IN_IMG_HW, wpecmd->WPE_VGEN_IN_IMG, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_OUT_IMG_HW, wpecmd->WPE_VGEN_OUT_IMG, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_HORI_STEP_HW, wpecmd->WPE_VGEN_HORI_STEP, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_VERT_STEP_HW, wpecmd->WPE_VGEN_VERT_STEP, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_HORI_INT_OFST_HW , wpecmd->WPE_VGEN_HORI_INT_OFST, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_HORI_SUB_OFST_HW, wpecmd->WPE_VGEN_HORI_SUB_OFST, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_VERT_INT_OFST_HW, wpecmd->WPE_VGEN_VERT_INT_OFST, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_VERT_SUB_OFST_HW, wpecmd->WPE_VGEN_VERT_SUB_OFST, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_POST_CTL_HW, wpecmd->WPE_VGEN_POST_CTL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_POST_COMP_X_HW, wpecmd->WPE_VGEN_POST_COMP_X, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_POST_COMP_Y_HW, wpecmd->WPE_VGEN_POST_COMP_Y, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VGEN_MAX_VEC_HW, wpecmd->WPE_VGEN_MAX_VEC, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VFIFO_CTL_HW, wpecmd->WPE_VFIFO_CTL, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_CFIFO_CTL_HW, wpecmd->WPE_CFIFO_CTL, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_RWCTL_CTL_HW, wpecmd->WPE_RWCTL_CTL, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_B_CACHI_SPECIAL_FUN_EN_HW, wpecmd->WPE_CACHI_SPECIAL_FUN_EN, 0xFFFFFFFF);
		if (m_ISPConfig.top.wpe_mdpcrop_en == 1)
		{
			MM_REG_WRITE_MASK(command, WPE_B_C24_TILE_EDGE_HW, wpecmd->WPE_C24_TILE_EDGE, 0xFFFFFFFF);
	
			MM_REG_WRITE_MASK(command, WPE_B_MDP_CROP_X_HW, wpecmd->WPE_MDP_CROP_X, 0xFFFFFFFF);
			MM_REG_WRITE_MASK(command, WPE_B_MDP_CROP_Y_HW, wpecmd->WPE_MDP_CROP_Y, 0xFFFFFFFF);
		}
		if (m_ISPConfig.top.wpe_ispcrop_en == 1)
		{
			MM_REG_WRITE_MASK(command, WPE_B_ISPCROP_CON1_HW, wpecmd->WPE_ISPCROP_CON1, 0xFFFFFFFF);
			MM_REG_WRITE_MASK(command, WPE_B_ISPCROP_CON2_HW, wpecmd->WPE_ISPCROP_CON2, 0xFFFFFFFF);
		}
		MM_REG_WRITE_MASK(command, WPE_B_PSP_CTL_HW, wpecmd->WPE_PSP_CTL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_PSP2_CTL_HW, wpecmd->WPE_PSP2_CTL, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_SOFT_RSTSTAT_0_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_BASE_ADDR_0_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_OFFSET_ADDR_0_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_STRIDE_0_HW, wpecmd->WPE_ADDR_GEN_STRIDE_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON_0_HW, wpecmd->WPE_CACHI_CON_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON2_0_HW, wpecmd->WPE_CACHI_CON2_0, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON3_0_HW, wpecmd->WPE_CACHI_CON3_0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_CTRL_0_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_STAT_0_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_RSV1_0_HW, wpecmd->WPE_ADDR_GEN_RSV1_0, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_DEBUG_SEL_0_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_0, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_SOFT_RSTSTAT_1_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_BASE_ADDR_1_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_OFFSET_ADDR_1_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_STRIDE_1_HW, wpecmd->WPE_ADDR_GEN_STRIDE_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON_1_HW, wpecmd->WPE_CACHI_CON_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON2_1_HW , wpecmd->WPE_CACHI_CON2_1, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON3_1_HW, wpecmd->WPE_CACHI_CON3_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_CTRL_1_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_STAT_1_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_RSV1_1_HW, wpecmd->WPE_ADDR_GEN_RSV1_1, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_DEBUG_SEL_1_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_1, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_SOFT_RSTSTAT_2_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_BASE_ADDR_2_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_OFFSET_ADDR_2_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_STRIDE_2_HW, wpecmd->WPE_ADDR_GEN_STRIDE_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON_2_HW, wpecmd->WPE_CACHI_CON_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON2_2_HW , wpecmd->WPE_CACHI_CON2_2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON3_2_HW, wpecmd->WPE_CACHI_CON3_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_CTRL_2_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_STAT_2_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_RSV1_2_HW, wpecmd->WPE_ADDR_GEN_RSV1_2, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_DEBUG_SEL_2_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_2, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_SOFT_RSTSTAT_3_HW, wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_BASE_ADDR_3_HW, wpecmd->WPE_ADDR_GEN_BASE_ADDR_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_OFFSET_ADDR_3_HW, wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_STRIDE_3_HW, wpecmd->WPE_ADDR_GEN_STRIDE_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON_3_HW, wpecmd->WPE_CACHI_CON_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON2_3_HW , wpecmd->WPE_CACHI_CON2_3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_CACHI_CON3_3_HW, wpecmd->WPE_CACHI_CON3_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_CTRL_3_HW, wpecmd->WPE_ADDR_GEN_ERR_CTRL_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_ERR_STAT_3_HW, wpecmd->WPE_ADDR_GEN_ERR_STAT_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_RSV1_3_HW, wpecmd->WPE_ADDR_GEN_RSV1_3, 0xFFFFFFFF);
		// MM_REG_WRITE_MASK(command, WPE_B_ADDR_GEN_DEBUG_SEL_3_HW, wpecmd->WPE_ADDR_GEN_DEBUG_SEL_3, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_B_DMA_SOFT_RSTSTAT_HW, wpecmd->WPE_DMA_SOFT_RSTSTAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_TDRI_BASE_ADDR_HW, wpecmd->WPE_TDRI_BASE_ADDR, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_TDRI_OFST_ADDR_HW, wpecmd->WPE_TDRI_OFST_ADDR, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_TDRI_XSIZE_HW, wpecmd->WPE_TDRI_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VERTICAL_FLIP_EN_HW, wpecmd->WPE_VERTICAL_FLIP_EN, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_DMA_SOFT_RESET_HW, wpecmd->WPE_DMA_SOFT_RESET, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_LAST_ULTRA_EN_HW, wpecmd->WPE_LAST_ULTRA_EN, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_SPECIAL_FUN_EN_HW, wpecmd->WPE_SPECIAL_FUN_EN, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_BASE_ADDR_HW, wpecmd->WPE_WPEO_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_OFST_ADDR_HW, wpecmd->WPE_WPEO_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_XSIZE_HW, wpecmd->WPE_WPEO_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_YSIZE_HW, wpecmd->WPE_WPEO_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_STRIDE_HW, wpecmd->WPE_WPEO_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_CON_HW, wpecmd->WPE_WPEO_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_CON2_HW, wpecmd->WPE_WPEO_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_CON3_HW, wpecmd->WPE_WPEO_CON3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_CROP_HW, wpecmd->WPE_WPEO_CROP, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_BASE_ADDR_HW, wpecmd->WPE_MSKO_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_OFST_ADDR_HW, wpecmd->WPE_MSKO_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_XSIZE_HW, wpecmd->WPE_MSKO_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_YSIZE_HW, wpecmd->WPE_MSKO_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_STRIDE_HW, wpecmd->WPE_MSKO_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_CON_HW, wpecmd->WPE_MSKO_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_CON2_HW, wpecmd->WPE_MSKO_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_CON3_HW, wpecmd->WPE_MSKO_CON3, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_CROP_HW, wpecmd->WPE_MSKO_CROP, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VECI_BASE_ADDR_HW, wpecmd->WPE_VECI_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VECI_OFST_ADDR_HW, wpecmd->WPE_VECI_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VECI_XSIZE_HW, wpecmd->WPE_VECI_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VECI_YSIZE_HW, wpecmd->WPE_VECI_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VECI_STRIDE_HW, wpecmd->WPE_VECI_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VECI_CON_HW, wpecmd->WPE_VECI_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VECI_CON2_HW, wpecmd->WPE_VECI_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VECI_CON3_HW, wpecmd->WPE_VECI_CON3, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_BASE_ADDR_HW, wpecmd->WPE_VEC2I_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_OFST_ADDR_HW, wpecmd->WPE_VEC2I_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_XSIZE_HW, wpecmd->WPE_VEC2I_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_YSIZE_HW, wpecmd->WPE_VEC2I_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_STRIDE_HW, wpecmd->WPE_VEC2I_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_CON_HW, wpecmd->WPE_VEC2I_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_CON2_HW, wpecmd->WPE_VEC2I_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_CON3_HW, wpecmd->WPE_VEC2I_CON3, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_BASE_ADDR_HW, wpecmd->WPE_VEC3I_BASE_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_OFST_ADDR_HW, wpecmd->WPE_VEC3I_OFST_ADDR, 0xFFFFFFFF);
	
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_XSIZE_HW, wpecmd->WPE_VEC3I_XSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_YSIZE_HW, wpecmd->WPE_VEC3I_YSIZE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_STRIDE_HW, wpecmd->WPE_VEC3I_STRIDE, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_CON_HW, wpecmd->WPE_VEC3I_CON, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_CON2_HW, wpecmd->WPE_VEC3I_CON2, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_CON3_HW, wpecmd->WPE_VEC3I_CON3, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_B_DMA_ERR_CTRL_HW , wpecmd->WPE_DMA_ERR_CTRL, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_WPEO_ERR_STAT_HW, wpecmd->WPE_WPEO_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_MSKO_ERR_STAT_HW, wpecmd->WPE_MSKO_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VECI_ERR_STAT_HW, wpecmd->WPE_VECI_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC2I_ERR_STAT_HW, wpecmd->WPE_VEC2I_ERR_STAT, 0xFFFFFFFF);
		MM_REG_WRITE_MASK(command, WPE_B_VEC3I_ERR_STAT_HW, wpecmd->WPE_VEC3I_ERR_STAT, 0xFFFFFFFF);
		//MM_REG_WRITE_MASK(command, WPE_B_DMA_DEBUG_ADDR_HW, wpecmd->WPE_DMA_DEBUG_ADDR, 0xFFFFFFFF);
	
		//MM_REG_WRITE_MASK(command, WPE_B_DMA_DEBUG_SEL_HW, wpecmd->WPE_DMA_DEBUG_SEL, 0xFFFFFFFF);
	}

		DPLOGI("ConfigWPEHW Start!\n");
		DPLOGI("WPE_CTL_MOD_EN:0x%x!\n", wpecmd->WPE_CTL_MOD_EN);
		DPLOGI("WPE_CTL_DMA_EN:0x%x!\n", wpecmd->WPE_CTL_DMA_EN);
	
		DPLOGI("WPE_CTL_CFG:0x%x!\n", wpecmd->WPE_CTL_CFG);
		DPLOGI("WPE_CTL_FMT_SEL:0x%x!\n", wpecmd->WPE_CTL_FMT_SEL);
		DPLOGI("WPE_CTL_INT_EN:0x%x!\n", wpecmd->WPE_CTL_INT_EN);
	
		DPLOGI("WPE_CTL_INT_STATUS:0x%x!\n", wpecmd->WPE_CTL_INT_STATUS);
		DPLOGI("WPE_CTL_INT_STATUSX:0x%x!\n", wpecmd->WPE_CTL_INT_STATUSX);
		DPLOGI("WPE_CTL_TDR_TILE:0x%x!\n", wpecmd->WPE_CTL_TDR_TILE);
		DPLOGI("WPE_CTL_TDR_DBG_STATUS:0x%x!\n", wpecmd->WPE_CTL_TDR_DBG_STATUS);
		DPLOGI("WPE_CTL_TDR_TCM_EN:0x%x!\n", wpecmd->WPE_CTL_TDR_TCM_EN);
		DPLOGI("WPE_CTL_SW_CTL:0x%x!\n", wpecmd->WPE_CTL_SW_CTL);
		DPLOGI("WPE_CTL_SPARE0:0x%x!\n", wpecmd->WPE_CTL_SPARE0);
		DPLOGI("WPE_CTL_SPARE1:0x%x!\n", wpecmd->WPE_CTL_SPARE1);
		DPLOGI("WPE_CTL_SPARE2:0x%x!\n", wpecmd->WPE_CTL_SPARE2);
		DPLOGI("WPE_CTL_DONE_SEL:0x%x!\n", wpecmd->WPE_CTL_DONE_SEL);
		DPLOGI("WPE_CTL_DBG_SET:0x%x!\n", wpecmd->WPE_CTL_DBG_SET);
		DPLOGI("WPE_CTL_DBG_PORT:0x%x!\n", wpecmd->WPE_CTL_DBG_PORT);
		//DPLOGI("WPE_CTL_DATE_CODE:0x%x!\n", wpecmd->WPE_CTL_DATE_CODE);
		//DPLOGI("WPE_CTL_PROJ_CODE:0x%x!\n", wpecmd->WPE_CTL_PROJ_CODE);
		DPLOGI("WPE_CTL_WPE_DCM_DIS:0x%x!\n", wpecmd->WPE_CTL_WPE_DCM_DIS);
		DPLOGI("WPE_CTL_DMA_DCM_DIS:0x%x!\n", wpecmd->WPE_CTL_DMA_DCM_DIS);
		//DPLOGI("WPE_CTL_WPE_DCM_STATUS:0x%x!\n", wpecmd->WPE_CTL_WPE_DCM_STATUS);
		//DPLOGI("WPE_CTL_DMA_DCM_STATUS:0x%x!\n", wpecmd->WPE_CTL_DMA_DCM_STATUS);
		//DPLOGI("WPE_CTL_WPE_REQ_STATUS:0x%x!\n", wpecmd->WPE_CTL_WPE_REQ_STATUS);
		//DPLOGI("WPE_CTL_DMA_REQ_STATUS:0x%x!\n", wpecmd->WPE_CTL_DMA_REQ_STATUS);
		//DPLOGI("WPE_CTL_WPE_RDY_STATUS:0x%x!\n", wpecmd->WPE_CTL_WPE_RDY_STATUS);
		//DPLOGI("WPE_CTL_DMA_RDY_STATUS:0x%x!\n", wpecmd->WPE_CTL_DMA_RDY_STATUS);
	
		DPLOGI("WPE_VGEN_CTL:0x%x!\n", wpecmd->WPE_VGEN_CTL);
		DPLOGI("WPE_VGEN_IN_IMG:0x%x!\n", wpecmd->WPE_VGEN_IN_IMG);
		DPLOGI("WPE_VGEN_OUT_IMG:0x%x!\n", wpecmd->WPE_VGEN_OUT_IMG);
		DPLOGI("WPE_VGEN_HORI_STEP:0x%x!\n", wpecmd->WPE_VGEN_HORI_STEP);
		DPLOGI("WPE_VGEN_VERT_STEP:0x%x!\n", wpecmd->WPE_VGEN_VERT_STEP);
		DPLOGI("WPE_VGEN_HORI_INT_OFST_REG :0x%x!\n", wpecmd->WPE_VGEN_HORI_INT_OFST);
		DPLOGI("WPE_VGEN_HORI_SUB_OFST:0x%x!\n", wpecmd->WPE_VGEN_HORI_SUB_OFST);
		DPLOGI("WPE_VGEN_VERT_INT_OFST:0x%x!\n", wpecmd->WPE_VGEN_VERT_INT_OFST);
		DPLOGI("WPE_VGEN_VERT_SUB_OFST:0x%x!\n", wpecmd->WPE_VGEN_VERT_SUB_OFST);
	
		DPLOGI("WPE_VGEN_POST_CTL:0x%x!\n", wpecmd->WPE_VGEN_POST_CTL);
		DPLOGI("WPE_VGEN_POST_COMP_X:0x%x!\n", wpecmd->WPE_VGEN_POST_COMP_X);
		DPLOGI("WPE_VGEN_POST_COMP_Y:0x%x!\n", wpecmd->WPE_VGEN_POST_COMP_Y);
		DPLOGI("WPE_VGEN_MAX_VEC:0x%x!\n", wpecmd->WPE_VGEN_MAX_VEC);
		DPLOGI("WPE_VFIFO_CTL:0x%x!\n", wpecmd->WPE_VFIFO_CTL);
	
		DPLOGI("WPE_CFIFO_CTL:0x%x!\n", wpecmd->WPE_CFIFO_CTL);
	
		DPLOGI("WPE_RWCTL_CTL:0x%x!\n", wpecmd->WPE_RWCTL_CTL);
	
		//DPLOGI("WPE_CACHI_SPECIAL_FUN_EN:0x%x!\n", wpecmd->WPE_CACHI_SPECIAL_FUN_EN);
	
		DPLOGI("WPE_C24_TILE_EDGE:0x%x!\n", wpecmd->WPE_C24_TILE_EDGE);
	
		DPLOGI("WPE_MDP_CROP_X:0x%x!\n", wpecmd->WPE_MDP_CROP_X);
		DPLOGI("WPE_MDP_CROP_Y:0x%x!\n", wpecmd->WPE_MDP_CROP_Y);
	
		DPLOGI("WPE_ISPCROP_CON1:0x%x!\n", wpecmd->WPE_ISPCROP_CON1);
		DPLOGI("WPE_ISPCROP_CON2:0x%x!\n", wpecmd->WPE_ISPCROP_CON2);
	
		DPLOGI("WPE_PSP_CTL:0x%x!\n", wpecmd->WPE_PSP_CTL);
		DPLOGI("WPE_PSP2_CTL:0x%x!\n", wpecmd->WPE_PSP2_CTL);
	
		DPLOGI("WPE_ADDR_GEN_SOFT_RSTSTAT_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_0);
		DPLOGI("WPE_ADDR_GEN_BASE_ADDR_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_BASE_ADDR_0);
		DPLOGI("WPE_ADDR_GEN_OFFSET_ADDR_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_0);
		DPLOGI("WPE_ADDR_GEN_STRIDE_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_STRIDE_0);
		DPLOGI("WPE_CACHI_CON_0:0x%x!\n", wpecmd->WPE_CACHI_CON_0);
		DPLOGI("WPE_CACHI_CON2_0:0x%x!\n", wpecmd->WPE_CACHI_CON2_0);
		DPLOGI("WPE_CACHI_CON3_0:0x%x!\n", wpecmd->WPE_CACHI_CON3_0);
		//DPLOGI("WPE_ADDR_GEN_ERR_CTRL_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_CTRL_0);
		//DPLOGI("WPE_ADDR_GEN_ERR_STAT_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_STAT_0);
		//DPLOGI("WPE_ADDR_GEN_RSV1_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_RSV1_0);
		//DPLOGI("WPE_ADDR_GEN_DEBUG_SEL_0:0x%x!\n", wpecmd->WPE_ADDR_GEN_DEBUG_SEL_0);
	
		DPLOGI("WPE_ADDR_GEN_SOFT_RSTSTAT_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_1);
		DPLOGI("WPE_ADDR_GEN_BASE_ADDR_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_BASE_ADDR_1);
		DPLOGI("WPE_ADDR_GEN_OFFSET_ADDR_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_1);
		DPLOGI("WPE_ADDR_GEN_STRIDE_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_STRIDE_1);
		DPLOGI("WPE_CACHI_CON_1:0x%x!\n", wpecmd->WPE_CACHI_CON_1);
		DPLOGI("WPE_CACHI_CON2_1_REG :0x%x!\n", wpecmd->WPE_CACHI_CON2_1);
		DPLOGI("WPE_CACHI_CON3_1:0x%x!\n", wpecmd->WPE_CACHI_CON3_1);
		//DPLOGI("WPE_ADDR_GEN_ERR_CTRL_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_CTRL_1);
		//DPLOGI("WPE_ADDR_GEN_ERR_STAT_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_STAT_1);
		//DPLOGI("WPE_ADDR_GEN_RSV1_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_RSV1_1);
		//DPLOGI("WPE_ADDR_GEN_DEBUG_SEL_1:0x%x!\n", wpecmd->WPE_ADDR_GEN_DEBUG_SEL_1);
	
		DPLOGI("WPE_ADDR_GEN_SOFT_RSTSTAT_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_2);
		DPLOGI("WPE_ADDR_GEN_BASE_ADDR_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_BASE_ADDR_2);
		DPLOGI("WPE_ADDR_GEN_OFFSET_ADDR_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_2);
		DPLOGI("WPE_ADDR_GEN_STRIDE_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_STRIDE_2);
		DPLOGI("WPE_CACHI_CON_2:0x%x!\n", wpecmd->WPE_CACHI_CON_2);
		DPLOGI("WPE_CACHI_CON2_2_REG :0x%x!\n", wpecmd->WPE_CACHI_CON2_2);
		DPLOGI("WPE_CACHI_CON3_2:0x%x!\n", wpecmd->WPE_CACHI_CON3_2);
		//DPLOGI("WPE_ADDR_GEN_ERR_CTRL_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_CTRL_2);
		//DPLOGI("WPE_ADDR_GEN_ERR_STAT_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_STAT_2);
		//DPLOGI("WPE_ADDR_GEN_RSV1_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_RSV1_2);
		//DPLOGI("WPE_ADDR_GEN_DEBUG_SEL_2:0x%x!\n", wpecmd->WPE_ADDR_GEN_DEBUG_SEL_2);
	
		DPLOGI("WPE_ADDR_GEN_SOFT_RSTSTAT_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_SOFT_RSTSTAT_3);
		DPLOGI("WPE_ADDR_GEN_BASE_ADDR_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_BASE_ADDR_3);
		DPLOGI("WPE_ADDR_GEN_OFFSET_ADDR_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_OFFSET_ADDR_3);
		DPLOGI("WPE_ADDR_GEN_STRIDE_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_STRIDE_3);
		DPLOGI("WPE_CACHI_CON_3:0x%x!\n", wpecmd->WPE_CACHI_CON_3);
		DPLOGI("WPE_CACHI_CON2_3_REG :0x%x!\n", wpecmd->WPE_CACHI_CON2_3);
		DPLOGI("WPE_CACHI_CON3_3:0x%x!\n", wpecmd->WPE_CACHI_CON3_3);
		//DPLOGI("WPE_ADDR_GEN_ERR_CTRL_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_CTRL_3);
		//DPLOGI("WPE_ADDR_GEN_ERR_STAT_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_ERR_STAT_3);
		//DPLOGI("WPE_ADDR_GEN_RSV1_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_RSV1_3);
		//DPLOGI("WPE_ADDR_GEN_DEBUG_SEL_3:0x%x!\n", wpecmd->WPE_ADDR_GEN_DEBUG_SEL_3);
	
		DPLOGI("WPE_DMA_SOFT_RSTSTAT:0x%x!\n", wpecmd->WPE_DMA_SOFT_RSTSTAT);
		DPLOGI("WPE_TDRI_BASE_ADDR:0x%x!\n", wpecmd->WPE_TDRI_BASE_ADDR);
		DPLOGI("WPE_TDRI_OFST_ADDR:0x%x!\n", wpecmd->WPE_TDRI_OFST_ADDR);
		DPLOGI("WPE_TDRI_XSIZE:0x%x!\n", wpecmd->WPE_TDRI_XSIZE);
		DPLOGI("WPE_VERTICAL_FLIP_EN:0x%x!\n", wpecmd->WPE_VERTICAL_FLIP_EN);
		DPLOGI("WPE_DMA_SOFT_RESET:0x%x!\n", wpecmd->WPE_DMA_SOFT_RESET);
		DPLOGI("WPE_LAST_ULTRA_EN:0x%x!\n", wpecmd->WPE_LAST_ULTRA_EN);
		DPLOGI("WPE_SPECIAL_FUN_EN:0x%x!\n", wpecmd->WPE_SPECIAL_FUN_EN);
	
		DPLOGI("WPE_WPEO_BASE_ADDR:0x%x!\n", wpecmd->WPE_WPEO_BASE_ADDR);
	
		DPLOGI("WPE_WPEO_OFST_ADDR:0x%x!\n", wpecmd->WPE_WPEO_OFST_ADDR);
	
		DPLOGI("WPE_WPEO_XSIZE:0x%x!\n", wpecmd->WPE_WPEO_XSIZE);
		DPLOGI("WPE_WPEO_YSIZE:0x%x!\n", wpecmd->WPE_WPEO_YSIZE);
		DPLOGI("WPE_WPEO_STRIDE:0x%x!\n", wpecmd->WPE_WPEO_STRIDE);
		DPLOGI("WPE_WPEO_CON:0x%x!\n", wpecmd->WPE_WPEO_CON);
		DPLOGI("WPE_WPEO_CON2:0x%x!\n", wpecmd->WPE_WPEO_CON2);
		DPLOGI("WPE_WPEO_CON3:0x%x!\n", wpecmd->WPE_WPEO_CON3);
		DPLOGI("WPE_WPEO_CROP:0x%x!\n", wpecmd->WPE_WPEO_CROP);
	
		DPLOGI("WPE_MSKO_BASE_ADDR:0x%x!\n", wpecmd->WPE_MSKO_BASE_ADDR);
	
		DPLOGI("WPE_MSKO_OFST_ADDR:0x%x!\n", wpecmd->WPE_MSKO_OFST_ADDR);
	
		DPLOGI("WPE_MSKO_XSIZE:0x%x!\n", wpecmd->WPE_MSKO_XSIZE);
		DPLOGI("WPE_MSKO_YSIZE:0x%x!\n", wpecmd->WPE_MSKO_YSIZE);
		DPLOGI("WPE_MSKO_STRIDE:0x%x!\n", wpecmd->WPE_MSKO_STRIDE);
		DPLOGI("WPE_MSKO_CON:0x%x!\n", wpecmd->WPE_MSKO_CON);
		DPLOGI("WPE_MSKO_CON2:0x%x!\n", wpecmd->WPE_MSKO_CON2);
		DPLOGI("WPE_MSKO_CON3:0x%x!\n", wpecmd->WPE_MSKO_CON3);
		DPLOGI("WPE_MSKO_CROP:0x%x!\n", wpecmd->WPE_MSKO_CROP);
	
		DPLOGI("WPE_VECI_BASE_ADDR:0x%x!\n", wpecmd->WPE_VECI_BASE_ADDR);
	
		DPLOGI("WPE_VECI_OFST_ADDR:0x%x!\n", wpecmd->WPE_VECI_OFST_ADDR);
	
		DPLOGI("WPE_VECI_XSIZE:0x%x!\n", wpecmd->WPE_VECI_XSIZE);
		DPLOGI("WPE_VECI_YSIZE:0x%x!\n", wpecmd->WPE_VECI_YSIZE);
		DPLOGI("WPE_VECI_STRIDE:0x%x!\n", wpecmd->WPE_VECI_STRIDE);
		DPLOGI("WPE_VECI_CON:0x%x!\n", wpecmd->WPE_VECI_CON);
		DPLOGI("WPE_VECI_CON2:0x%x!\n", wpecmd->WPE_VECI_CON2);
		DPLOGI("WPE_VECI_CON3:0x%x!\n", wpecmd->WPE_VECI_CON3);
	
		DPLOGI("WPE_VEC2I_BASE_ADDR:0x%x!\n", wpecmd->WPE_VEC2I_BASE_ADDR);
	
		DPLOGI("WPE_VEC2I_OFST_ADDR:0x%x!\n", wpecmd->WPE_VEC2I_OFST_ADDR);
	
		DPLOGI("WPE_VEC2I_XSIZE:0x%x!\n", wpecmd->WPE_VEC2I_XSIZE);
		DPLOGI("WPE_VEC2I_YSIZE:0x%x!\n", wpecmd->WPE_VEC2I_YSIZE);
		DPLOGI("WPE_VEC2I_STRIDE:0x%x!\n", wpecmd->WPE_VEC2I_STRIDE);
		DPLOGI("WPE_VEC2I_CON:0x%x!\n", wpecmd->WPE_VEC2I_CON);
		DPLOGI("WPE_VEC2I_CON2:0x%x!\n", wpecmd->WPE_VEC2I_CON2);
		DPLOGI("WPE_VEC2I_CON3:0x%x!\n", wpecmd->WPE_VEC2I_CON3);
	
		DPLOGI("WPE_VEC3I_BASE_ADDR:0x%x!\n", wpecmd->WPE_VEC3I_BASE_ADDR);
	
		DPLOGI("WPE_VEC3I_OFST_ADDR:0x%x!\n", wpecmd->WPE_VEC3I_OFST_ADDR);
	
		DPLOGI("WPE_VEC3I_XSIZE:0x%x!\n", wpecmd->WPE_VEC3I_XSIZE);
		DPLOGI("WPE_VEC3I_YSIZE:0x%x!\n", wpecmd->WPE_VEC3I_YSIZE);
		DPLOGI("WPE_VEC3I_STRIDE:0x%x!\n", wpecmd->WPE_VEC3I_STRIDE);
		DPLOGI("WPE_VEC3I_CON:0x%x!\n", wpecmd->WPE_VEC3I_CON);
		DPLOGI("WPE_VEC3I_CON2:0x%x!\n", wpecmd->WPE_VEC3I_CON2);
		DPLOGI("WPE_VEC3I_CON3:0x%x!\n", wpecmd->WPE_VEC3I_CON3);
	
		DPLOGI("WPE_DMA_ERR_CTRL_REG :0x%x!\n", wpecmd->WPE_DMA_ERR_CTRL);
		DPLOGI("WPE_WPEO_ERR_STAT:0x%x!\n", wpecmd->WPE_WPEO_ERR_STAT);
		DPLOGI("WPE_MSKO_ERR_STAT:0x%x!\n", wpecmd->WPE_MSKO_ERR_STAT);
		DPLOGI("WPE_VECI_ERR_STAT:0x%x!\n", wpecmd->WPE_VECI_ERR_STAT);
		DPLOGI("WPE_VEC2I_ERR_STAT:0x%x!\n", wpecmd->WPE_VEC2I_ERR_STAT);
		DPLOGI("WPE_VEC3I_ERR_STAT:0x%x!\n", wpecmd->WPE_VEC3I_ERR_STAT);
		DPLOGI("WPE_DMA_DEBUG_ADDR:0x%x!\n", wpecmd->WPE_DMA_DEBUG_ADDR);
	
		DPLOGI("WPE_DMA_DEBUG_SEL:0x%x!\n", wpecmd->WPE_DMA_DEBUG_SEL);

#endif

    DPLOGI("[DpWrapper_WPE::onConfigFrame]-,\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::initTilePath(TILE_PARAM_STRUCT *p_tile_param)
{
    ISP_TILE_MESSAGE_ENUM result = ISP_MESSAGE_TILE_OK;

    /* tile core property */
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;
    m_pTileFunc = p_tile_param->ptr_tile_func_param;

    assert(NULL == p_tile_param->ptr_isp_tile_descriptor);
    p_tile_param->ptr_isp_tile_descriptor = m_pTileDesc;

    DPLOGI("[initTilePath]+,\n");

#if !CONFIG_FOR_VERIFY_FPGA
    result = tile_copy_config_by_platform(p_tile_param, &m_ISPConfig);
    if (ISP_MESSAGE_TILE_OK != result)
    {
        DPLOGE("[Error]onConfigFrame: get ISP tile paramter failed,result(%d)\n",result);
        return DP_STATUS_INVALID_PARAX;
    }

    ptr_tile_reg_map->last_irq_mode = m_ISPConfig.sw.tpipe_irq_mode; // 0 FOR WPE ONLY,2 FOR MDP directtlink
    ptr_tile_reg_map->isp_tile_width  = m_ISPConfig.sw.tpipe_width_wpe;
    ptr_tile_reg_map->isp_tile_height = m_ISPConfig.sw.tpipe_height_wpe;

    //m_ISPPassType = m_ISPConfig.drvinfo.cqIdx;

#else // CONFIG_FOR_VERIFY_FPGA
    result = tile_main_read_isp_reg_file(ptr_tile_reg_map, "tile_reg_map_frame.txt", "tile_reg_map_frame_d.txt");
    if (ISP_MESSAGE_TILE_OK != result)
    {
        DPLOGE("DpWrapper_WPE: read tile_reg_map_frame.txt failed\n");
        return DP_STATUS_INVALID_FILE;
    }

    ptr_tile_reg_map->last_irq_mode = 2; // 0 FOR ISP ONLY,2 FOR MDP directtlink
    ptr_tile_reg_map->isp_tile_width  = 512;
    ptr_tile_reg_map->isp_tile_height = MAX_SIZE;

    ptr_tile_reg_map->tdr_ctrl_en = true;
    ptr_tile_reg_map->skip_tile_mode = true;
    ptr_tile_reg_map->run_c_model_direct_link = 0;

#endif // CONFIG_FOR_VERIFY_FPGA
    ptr_tile_reg_map->max_input_width  = MAX_SIZE;  /* can modify to test */
    ptr_tile_reg_map->max_input_height = MAX_SIZE;  /* can modify to test */

    ptr_tile_reg_map->src_dump_order = m_outDumpOrder; //Normal direction
    ptr_tile_reg_map->src_stream_order = m_inStreamOrder; //Normal direction
    ptr_tile_reg_map->src_cal_order = m_inCalOrder; //Normal direction

    DPLOGI("[initTilePath]-,\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::onInitTileCalc(struct TILE_PARAM_STRUCT* p_tile_param)
{
    uint32_t index;

    /* tile core property */
    TILE_REG_MAP_STRUCT     *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param = p_tile_param->ptr_tile_func_param;
    TILE_FUNC_BLOCK_STRUCT  *ptr_func;

    for (index = 0; index < ptr_tile_func_param->used_func_no; index++)
    {
        ptr_func = &ptr_tile_func_param->func_list[index];
        if (TILE_FUNC_VECI_ID == ptr_func->func_num)
        {
            m_pVECI = ptr_func;
        }

        if (TILE_FUNC_WPE_MDP_CROP_ID == ptr_func->func_num)
        {
            m_pMDPCrop = ptr_func;
        }

		 if (TILE_FUNC_WPE_ISP_CROP_ID == ptr_func->func_num)
        {
            m_pISPCrop = ptr_func;
        }
    }

#if !CONFIG_FOR_VERIFY_FPGA
    if (m_onlyWPE && m_ISPConfig.top.wpe_mdpcrop_en)
    {
        DPLOGE("DpWrapper_WPE: MDP_Crop is enabled in ISP Pass2 only.\n");
        assert(0);
    }
#else // CONFIG_FOR_VERIFY_FPGA
    if (0 != m_pMDPCrop)
    {
        ptr_tile_reg_map->run_c_model_direct_link = 1;
    }
    assert(false == m_tileDisable); // on init frame mode
#endif // CONFIG_FOR_VERIFY_FPGA

    m_TDRITileID = 0;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::onRetrieveTileParam(struct TILE_PARAM_STRUCT* p_tile_param)
{
    if (NULL != m_pMDPCrop)
    {
        m_outTileXLeft   = m_pMDPCrop->out_pos_xs;
        m_outTileXRight  = m_pMDPCrop->out_pos_xe;
        m_outTileYTop    = m_pMDPCrop->out_pos_ys;
        m_outTileYBottom = m_pMDPCrop->out_pos_ye;
        //m_tileXEndFlag   = m_pMDPCrop->h_end_flag;
        //m_tileYEndFlag   = m_pMDPCrop->v_end_flag;
    }

    else if(NULL != m_pISPCrop)
    {

		m_outTileXLeft   = m_pISPCrop->out_pos_xs;
        m_outTileXRight  = m_pISPCrop->out_pos_xe;
        m_outTileYTop    = m_pISPCrop->out_pos_ys;
        m_outTileYBottom = m_pISPCrop->out_pos_ye;
        //m_tileXEndFlag   = m_pMDPCrop->h_end_flag;
        //m_tileYEndFlag   = m_pMDPCrop->v_end_flag;
	}

    if (NULL != m_pVECI)
    {
        m_inTileXLeft   = m_pVECI->in_pos_xs;
        m_inTileXRight  = m_pVECI->in_pos_xe;
        m_inTileYTop    = m_pVECI->in_pos_ys;
        m_inTileYBottom = m_pVECI->in_pos_ye;
    }

    m_inTileAccumulation += (m_inTileXRight - m_inTileXLeft + 1);
    m_outTileAccumulation += (m_outTileXRight - m_outTileXLeft + 1);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::onConfigTile(DpCommand &command)
{
    uint32_t tpipePa;
    uint32_t i;
	uint32_t tpipeXsize;
    uint32_t width;
    uint32_t height;

/*
#if CONFIG_FOR_VERIFY_FPGA
    MM_REG_WRITE_MASK(command, MDP_FAKE_ISP_CFG, ((m_TDRITileID&0xFF) << 24), 0xFF000000);
#endif
*/
    tpipePa = m_TDRIBufPA + m_pTileDesc->tpipe_info_wpe[m_TDRITileID].dump_offset_no*sizeof(uint32_t);
	tpipeXsize = m_pTileDesc->tpipe_info_wpe[m_TDRITileID].dump_offset_no*sizeof(uint32_t);

    DPLOGI("[onConfigTile]+,m_TDRITileID(%d),offset(0x%x),tpipePa(0x%08x),used_word_no(%d), tpipeXsize(%d)\n",
            m_TDRITileID, m_pTileDesc->tpipe_info_wpe[m_TDRITileID].dump_offset_no,tpipePa,m_pTileDesc->used_word_no_wpe,tpipeXsize);

    #if 0
    for(i=0;i<m_pTileDesc->used_word_no;i+=5){
        DPLOGI("Offset(%d),VA(0x%08x),(0x%08x)(0x%08x)(0x%08x)(0x%08x)(0x%08x)",
            i+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no,
            (m_pTDRIBufVA+i+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+0+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+1+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+2+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+3+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no),
            *(m_pTDRIBufVA+i+4+m_pTileDesc->tpipe_info[m_TDRITileID].dump_offset_no));
    }
    #endif
	if(m_identifier == 0)
	{
    	MM_REG_WRITE_MASK(command, 0x1502A3C4, tpipePa, 0xFFFFFFFF, &m_TDRITileLabel[m_TDRITileID]);
	
		//MM_REG_WRITE_MASK(command, 0x1502A3CC, tpipeXsize, 0xFFFFFFFF, &m_TDRITileLabel[m_TDRITileID]);
	}
	else
	{
		MM_REG_WRITE_MASK(command, 0x1502D3C4, tpipePa, 0xFFFFFFFF, &m_TDRITileLabel[m_TDRITileID]);
	
		//MM_REG_WRITE_MASK(command, 0x1502A3CC, tpipeXsize, 0xFFFFFFFF, &m_TDRITileLabel[m_TDRITileID]);
	}	
	m_TDRITileID++;

    width = m_outTileXRight - m_outTileXLeft + 1;
    height = m_outTileYBottom - m_outTileYTop + 1;

    if (m_ISPConfig.top.mdp_crop_en)
    {
        MM_REG_WRITE(command, MDP_ASYNC_CFG_WD, (height << 16) + (width << 0), 0xFFFFFFFF);
    }

/*
#if CONFIG_FOR_VERIFY_FPGA
    uint32_t width = m_outTileXRight - m_outTileXLeft + 1;
    uint32_t height = m_outTileYBottom - m_outTileYTop + 1;
    MM_REG_WRITE_MASK(command, MDP_FAKE_ISP_CFG, (height << 12) + width, 0x00FFFFFF);
#endif // CONFIG_FOR_VERIFY_FPGA
*/
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpWrapper_WPE::onReconfigTiles(DpCommand &command)
{
    uint32_t tpipePa;
    uint32_t i;
	uint32_t tpipeXsize;
	if(m_identifier == 0)
	{
    	for (i = 0; i < m_TDRITileID; i++)
    	{
        	tpipePa = m_TDRIBufPA + m_pTileDesc->tpipe_info_wpe[i].dump_offset_no*sizeof(uint32_t);
			tpipeXsize = m_pTileDesc->tpipe_info_wpe[m_TDRITileID].dump_offset_no*sizeof(uint32_t);

        	DPLOGI("[reConfigTile]+,m_TDRITileID(%d),offset(0x%x),tpipePa(0x%08x), , tpipeXsize(%d)\n", i, m_pTileDesc->tpipe_info_wpe[i].dump_offset_no,tpipePa, tpipeXsize);

        	MM_REG_WRITE_MASK(command, 0x1502A3C4, tpipePa, 0xFFFFFFFF, NULL, m_TDRITileLabel[i]);
		//MM_REG_WRITE_MASK(command, 0x1502A3CC, tpipeXsize, 0xFFFFFFFF, NULL, m_TDRITileLabel[i]);
    	}
	}	
	else
	{
		for (i = 0; i < m_TDRITileID; i++)
		{
			tpipePa = m_TDRIBufPA + m_pTileDesc->tpipe_info_wpe[i].dump_offset_no*sizeof(uint32_t);
			tpipeXsize = m_pTileDesc->tpipe_info_wpe[m_TDRITileID].dump_offset_no*sizeof(uint32_t);
		
			DPLOGI("[reConfigTile WPEB]+,m_TDRITileID(%d),offset(0x%x),tpipePa(0x%08x), , tpipeXsize(%d)\n", i, m_pTileDesc->tpipe_info_wpe[i].dump_offset_no,tpipePa, tpipeXsize);
		
			MM_REG_WRITE_MASK(command, 0x1502D3C4, tpipePa, 0xFFFFFFFF, NULL, m_TDRITileLabel[i]);
		}

	}

    return DP_STATUS_RETURN_SUCCESS;
}
