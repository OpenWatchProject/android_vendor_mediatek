#ifndef __TLDAPC_INDEX_H__
#define __TLDAPC_INDEX_H__

#include "tlApisec.h"

#define PLATFORM_TAG			"<6><7><8><5>"
#define DEVAPC_HAL_SUPPORT

/* Register definition for CMDQ usage */
#define DEVAPC_AO_MM_BASE		0x1000E000

/* Slave Type */
#define SLAVE_TYPE_PREFIX_INFRA_PERI	0x10000000
#define SLAVE_TYPE_PREFIX_MM		0x20000000
#define SLAVE_TYPE_PREFIX_MD		0x30000000

/* MM slaves' config index */
/* MMSYS & IMGSYS */
#define DAPC_INDEX_MMSYS_CONFIG                (SLAVE_TYPE_PREFIX_MM + 13)
#define DAPC_INDEX_MDP_RDMA0                   (SLAVE_TYPE_PREFIX_MM + 14)
#define DAPC_INDEX_MDP_RDMA1                   (SLAVE_TYPE_PREFIX_MM + 15)
#define DAPC_INDEX_MDP_RSZ0                    (SLAVE_TYPE_PREFIX_MM + 16)
#define DAPC_INDEX_MDP_RSZ1                    (SLAVE_TYPE_PREFIX_MM + 17)
#define DAPC_INDEX_MDP_WROT0                   (SLAVE_TYPE_PREFIX_MM + 18)
#define DAPC_INDEX_MDP_WDMA                    (SLAVE_TYPE_PREFIX_MM + 19)
#define DAPC_INDEX_MDP_TDSHP                   (SLAVE_TYPE_PREFIX_MM + 20)
#define DAPC_INDEX_DISP_OVL0                   (SLAVE_TYPE_PREFIX_MM + 21)
#define DAPC_INDEX_DISP_OVL0_2L                (SLAVE_TYPE_PREFIX_MM + 22)
#define DAPC_INDEX_DISP_OVL1_2L                (SLAVE_TYPE_PREFIX_MM + 23)
#define DAPC_INDEX_DISP_RDMA0                  (SLAVE_TYPE_PREFIX_MM + 24)
#define DAPC_INDEX_DISP_RDMA1                  (SLAVE_TYPE_PREFIX_MM + 25)
#define DAPC_INDEX_DISP_WDMA0                  (SLAVE_TYPE_PREFIX_MM + 26)
#define DAPC_INDEX_DISP_COLOR0                 (SLAVE_TYPE_PREFIX_MM + 27)
#define DAPC_INDEX_DISP_CCORR0                 (SLAVE_TYPE_PREFIX_MM + 28)
#define DAPC_INDEX_DISP_AAL0                   (SLAVE_TYPE_PREFIX_MM + 29)
#define DAPC_INDEX_DISP_GAMMA0                 (SLAVE_TYPE_PREFIX_MM + 30)
#define DAPC_INDEX_DISP_DITHER0                (SLAVE_TYPE_PREFIX_MM + 31)
#define DAPC_INDEX_DSI_SPLIT                   (SLAVE_TYPE_PREFIX_MM + 32)
#define DAPC_INDEX_DSI0                        (SLAVE_TYPE_PREFIX_MM + 33)
#define DAPC_INDEX_DPI                         (SLAVE_TYPE_PREFIX_MM + 34)
#define DAPC_INDEX_MM_MUTEX                    (SLAVE_TYPE_PREFIX_MM + 35)
#define DAPC_INDEX_SMI_LARB0                   (SLAVE_TYPE_PREFIX_MM + 36)
#define DAPC_INDEX_SMI_LARB1                   (SLAVE_TYPE_PREFIX_MM + 37)
#define DAPC_INDEX_SMI_COMMON                  (SLAVE_TYPE_PREFIX_MM + 38)
#define DAPC_INDEX_DISP_RSZ                    (SLAVE_TYPE_PREFIX_MM + 39)
#define DAPC_INDEX_MDP_AAL                     (SLAVE_TYPE_PREFIX_MM + 40)
#define DAPC_INDEX_MDP_CCORR                   (SLAVE_TYPE_PREFIX_MM + 41)
#define DAPC_INDEX_DBI                         (SLAVE_TYPE_PREFIX_MM + 42)
#define DAPC_INDEX_MMSYS_RESERVE               (SLAVE_TYPE_PREFIX_MM + 43)
#define DAPC_INDEX_MMSYS_RESERVE_1             (SLAVE_TYPE_PREFIX_MM + 44)
#define DAPC_INDEX_MDP_WROT1                   (SLAVE_TYPE_PREFIX_MM + 45)
#define DAPC_INDEX_DISP_POSTMASK0              (SLAVE_TYPE_PREFIX_MM + 46)
#define DAPC_INDEX_MMSYS_OTHERS                (SLAVE_TYPE_PREFIX_MM + 47)
#define DAPC_INDEX_IMGSYS_BASE                 (SLAVE_TYPE_PREFIX_MM + 48)
#define DAPC_INDEX_SMI_LARB5_BASE              (SLAVE_TYPE_PREFIX_MM + 49)
#define DAPC_INDEX_DIP1_BASE                   (SLAVE_TYPE_PREFIX_MM + 50)
#define DAPC_INDEX_DIP2_BASE                   (SLAVE_TYPE_PREFIX_MM + 51)
#define DAPC_INDEX_DIP3_BASE                   (SLAVE_TYPE_PREFIX_MM + 52)
#define DAPC_INDEX_DIP4_BASE                   (SLAVE_TYPE_PREFIX_MM + 53)
#define DAPC_INDEX_DIP5_BASE                   (SLAVE_TYPE_PREFIX_MM + 54)
#define DAPC_INDEX_DIP6_BASE                   (SLAVE_TYPE_PREFIX_MM + 55)
#define DAPC_INDEX_DPE_BASE                    (SLAVE_TYPE_PREFIX_MM + 56)
#define DAPC_INDEX_RSC_BASE                    (SLAVE_TYPE_PREFIX_MM + 57)
#define DAPC_INDEX_WPE_A_BASE                  (SLAVE_TYPE_PREFIX_MM + 58)
#define DAPC_INDEX_FDVT_BASE                   (SLAVE_TYPE_PREFIX_MM + 59)
#define DAPC_INDEX_GEPF_BASE                   (SLAVE_TYPE_PREFIX_MM + 60)
#define DAPC_INDEX_WPE_B_BASE                  (SLAVE_TYPE_PREFIX_MM + 61)
#define DAPC_INDEX_MFB_BASE                    (SLAVE_TYPE_PREFIX_MM + 62)
#define DAPC_INDEX_SMI_LARB2_BASE              (SLAVE_TYPE_PREFIX_MM + 63)
#define DAPC_INDEX_IMGSYS_OTHERS               (SLAVE_TYPE_PREFIX_MM + 64)

#endif // __TLDAPC_INDEX_H__
