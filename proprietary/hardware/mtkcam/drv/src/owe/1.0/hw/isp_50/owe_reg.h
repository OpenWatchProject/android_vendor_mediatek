#ifndef _ISP_REG_H_
#define _ISP_REG_H_

//include for reg size
#include "camera_isp.h"

#ifndef MFALSE
#define MFALSE 0
#endif
#ifndef MTRUE
#define MTRUE 1
#endif
#ifndef MUINT8
typedef unsigned char MUINT8;
#endif

#ifndef MUINT32
typedef unsigned int MUINT32;
#endif
#ifndef MINT32
typedef int MINT32;
#endif
#ifndef MBOOL
typedef int MBOOL;
#endif

#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.Raw)

/**
    REG size for each module is 0x1000
*/
#define REG_SIZE        (ISP_REG_RANGE)
#define CAM_BASE_RANGE  (REG_SIZE)
#define CAM_BASE_RANGE_SPECIAL  (sizeof(cam_reg_t)) //for CQ special baseaddress
#define UNI_BASE_RANGE  (REG_SIZE)
#define DIP_BASE_RANGE_SPECIAL  (ISP_REG_PER_DIP_RANGE)
#define CAMSV_BASE_RANGE    (REG_SIZE)
#define FDVT_BASE_RANGE (REG_SIZE)
#define WPE_BASE_RANGE  (REG_SIZE)
#define RSC_BASE_RANGE  (REG_SIZE)
#define DPE_BASE_RANGE  (REG_SIZE)

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* auto insert ralf auto gen below */

typedef volatile union _OWE_REG_RST_
{
		volatile struct	/* 0x1502C000 */
		{
				FIELD  DMA_STOP                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  DMA_STOP_STATUS                       :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 :  2;		/*  2.. 3, 0x0000000C */
				FIELD  HARD_RST                              :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 : 27;		/*  5..31, 0xFFFFFFE0 */
		} Bits;
		UINT32 Raw;
}OWE_REG_RST;	/* OWE_RST */

typedef volatile union _OWE_REG_DCM_CTRL_
{
		volatile struct	/* 0x1502C004 */
		{
				FIELD  CTRL_DCM_DIS                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  OCC_DCM_DIS                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  WMFE_DCM_DIS                          :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  MEM_DCM_DIS                           :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RDMA_0_DCM_DIS                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  WDMA_0_DCM_DIS                        :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_DCM_CTRL;	/* OWE_DCM_CTRL */

typedef volatile union _OWE_REG_INT_CTRL_
{
		volatile struct	/* 0x1502C008 */
		{
				FIELD  INT_ENABLE                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 30;		/*  1..30, 0x7FFFFFFE */
				FIELD  INT_CLR_MODE                          :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_INT_CTRL;	/* OWE_INT_CTRL */

typedef volatile union _OWE_REG_VERSION_
{
		volatile struct	/* 0x1502C010 */
		{
				FIELD  DAY                                   :  8;		/*  0.. 7, 0x000000FF */
				FIELD  MONTH                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  YEAR                                  : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_VERSION;	/* OWE_VERSION */

typedef volatile union _OWE_REG_DCM_STATUS_
{
		volatile struct	/* 0x1502C014 */
		{
				FIELD  CTRL_DCM_STATUS                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  OCC_DCM_STATUS                        :  1;		/*  4.. 4, 0x00000010 */
				FIELD  rsv_5                                 :  3;		/*  5.. 7, 0x000000E0 */
				FIELD  WMFE_DCM_STATUS                       :  1;		/*  8.. 8, 0x00000100 */
				FIELD  rsv_9                                 :  3;		/*  9..11, 0x00000E00 */
				FIELD  MEM_DCM_STATUS                        :  1;		/* 12..12, 0x00001000 */
				FIELD  rsv_13                                :  3;		/* 13..15, 0x0000E000 */
				FIELD  RDMA_0_DCM_STATUS                     :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                :  3;		/* 17..19, 0x000E0000 */
				FIELD  WDMA_0_DCM_STATUS                     :  1;		/* 20..20, 0x00100000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_DCM_STATUS;	/* OWE_DCM_STATUS */

typedef volatile union _OWE_REG_INT_STATUS_
{
		volatile struct	/* 0x1502C018 */
		{
				FIELD  INT                                   :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}OWE_REG_INT_STATUS;	/* OWE_INT_STATUS */

typedef volatile union _OWE_REG_DBG_INFO_0_
{
		volatile struct	/* 0x1502C01C */
		{
				FIELD  CTRL_FSM                              :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}OWE_REG_DBG_INFO_0;	/* OWE_DBG_INFO_0 */

typedef volatile union _OWE_REG_OCC_START_
{
		volatile struct	/* 0x1502C020 */
		{
				FIELD  OCC_START                             :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}OWE_REG_OCC_START;	/* OWE_OCC_START */

typedef volatile union _OWE_REG_OCC_INT_CTRL_
{
		volatile struct	/* 0x1502C024 */
		{
				FIELD  OCC_INT_ENABLE                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 30;		/*  1..30, 0x7FFFFFFE */
				FIELD  OCC_INT_CLR_MODE                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_OCC_INT_CTRL;	/* OWE_OCC_INT_CTRL */

typedef volatile union _OWE_REG_OCC_INT_STATUS_
{
		volatile struct	/* 0x1502C028 */
		{
				FIELD  OCC_INT                               :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}OWE_REG_OCC_INT_STATUS;	/* OWE_OCC_INT_STATUS */

typedef volatile union _DPE_REG_OCC_CTRL_0_
{
		volatile struct	/* 0x1502C030 */
		{
				FIELD  occ_scan_r2l                          :  1;		/*  0.. 0, 0x00000001 */
				FIELD  occ_horz_ds4                          :  1;		/*  1.. 1, 0x00000002 */
				FIELD  occ_vert_ds4                          :  1;		/*  2.. 2, 0x00000004 */
				FIELD  occ_h_skip_mode                       :  1;		/*  3.. 3, 0x00000008 */
				FIELD  rsv_4                                 :  4;		/*  4.. 7, 0x000000F0 */
				FIELD  occ_imgi_maj_fmt                      :  2;		/*  8.. 9, 0x00000300 */
				FIELD  occ_imgi_ref_fmt                      :  2;		/* 10..11, 0x00000C00 */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_CTRL_0;	/* DPE_OCC_CTRL_0 */

typedef volatile union _DPE_REG_OCC_CTRL_1_
{
		volatile struct	/* 0x1502C034 */
		{
				FIELD  occ_hsize                             : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  occ_vsize                             : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_CTRL_1;	/* DPE_OCC_CTRL_1 */

typedef volatile union _DPE_REG_OCC_CTRL_2_
{
		volatile struct	/* 0x1502C038 */
		{
				FIELD  occ_v_crop_s                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  occ_v_crop_e                          : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_CTRL_2;	/* DPE_OCC_CTRL_2 */

typedef volatile union _DPE_REG_OCC_CTRL_3_
{
		volatile struct	/* 0x1502C03C */
		{
				FIELD  occ_h_crop_s                          : 10;		/*  0.. 9, 0x000003FF */
				FIELD  rsv_10                                :  6;		/* 10..15, 0x0000FC00 */
				FIELD  occ_h_crop_e                          : 10;		/* 16..25, 0x03FF0000 */
				FIELD  rsv_26                                :  6;		/* 26..31, 0xFC000000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_CTRL_3;	/* DPE_OCC_CTRL_3 */

typedef volatile union _DPE_REG_OCC_REF_VEC_BASE_
{
		volatile struct	/* 0x1502C040 */
		{
				FIELD  occ_ref_vec_base_addr                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_REF_VEC_BASE;	/* DPE_OCC_REF_VEC_BASE */

typedef volatile union _DPE_REG_OCC_REF_VEC_STRIDE_
{
		volatile struct	/* 0x1502C044 */
		{
				FIELD  occ_ref_vec_stride                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  occ_ref_vec_xsize                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_REF_VEC_STRIDE;	/* DPE_OCC_REF_VEC_STRIDE */

typedef volatile union _DPE_REG_OCC_REF_PXL_BASE_
{
		volatile struct	/* 0x1502C048 */
		{
				FIELD  occ_ref_pxl_base_addr                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_REF_PXL_BASE;	/* DPE_OCC_REF_PXL_BASE */

typedef volatile union _DPE_REG_OCC_REF_PXL_STRIDE_
{
		volatile struct	/* 0x1502C04C */
		{
				FIELD  occ_ref_pxl_stride                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  occ_ref_pxl_xsize                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_REF_PXL_STRIDE;	/* DPE_OCC_REF_PXL_STRIDE */

typedef volatile union _DPE_REG_OCC_MAJ_VEC_BASE_
{
		volatile struct	/* 0x1502C050 */
		{
				FIELD  occ_maj_vec_base_addr                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_MAJ_VEC_BASE;	/* DPE_OCC_MAJ_VEC_BASE */

typedef volatile union _DPE_REG_OCC_MAJ_VEC_STRIDE_
{
		volatile struct	/* 0x1502C054 */
		{
				FIELD  occ_maj_vec_stride                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  occ_maj_vec_xsize                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_MAJ_VEC_STRIDE;	/* DPE_OCC_MAJ_VEC_STRIDE */

typedef volatile union _DPE_REG_OCC_MAJ_PXL_BASE_
{
		volatile struct	/* 0x1502C058 */
		{
				FIELD  occ_maj_pxl_base_addr                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_MAJ_PXL_BASE;	/* DPE_OCC_MAJ_PXL_BASE */

typedef volatile union _DPE_REG_OCC_MAJ_PXL_STRIDE_
{
		volatile struct	/* 0x1502C05C */
		{
				FIELD  occ_maj_pxl_stride                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  occ_maj_pxl_xsize                     : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_MAJ_PXL_STRIDE;	/* DPE_OCC_MAJ_PXL_STRIDE */

typedef volatile union _DPE_REG_OCC_WDMA_BASE_
{
		volatile struct	/* 0x1502C060 */
		{
				FIELD  occ_wdma_base_addr                    : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_WDMA_BASE;	/* DPE_OCC_WDMA_BASE */

typedef volatile union _DPE_REG_OCC_WDMA_STRIDE_
{
		volatile struct	/* 0x1502C064 */
		{
				FIELD  occ_wdma_stride                       : 16;		/*  0..15, 0x0000FFFF */
				FIELD  occ_wdma_xsize                        : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_WDMA_STRIDE;	/* DPE_OCC_WDMA_STRIDE */

typedef volatile union _DPE_REG_OCC_PQ_0_
{
		volatile struct	/* 0x1502C068 */
		{
				FIELD  occ_th_luma                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  occ_th_h                              :  8;		/*  8..15, 0x0000FF00 */
				FIELD  occ_th_v                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  occ_vec_shift                         :  3;		/* 20..22, 0x00700000 */
				FIELD  rsv_23                                :  1;		/* 23..23, 0x00800000 */
				FIELD  occ_vec_offset                        :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_PQ_0;	/* DPE_OCC_PQ_0 */

typedef volatile union _DPE_REG_OCC_PQ_1_
{
		volatile struct	/* 0x1502C06C */
		{
				FIELD  occ_invalid_value                     :  8;		/*  0.. 7, 0x000000FF */
				FIELD  occ_owc_th                            :  8;		/*  8..15, 0x0000FF00 */
				FIELD  occ_owc_en                            :  1;		/* 16..16, 0x00010000 */
				FIELD  occ_depth_clip_en                     :  1;		/* 17..17, 0x00020000 */
				FIELD  rsv_18                                : 14;		/* 18..31, 0xFFFC0000 */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_PQ_1;	/* DPE_OCC_PQ_1 */

typedef volatile union _DPE_REG_OCC_SPARE_
{
		volatile struct	/* 0x1502C070 */
		{
				FIELD  occ_spare                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_SPARE;	/* DPE_OCC_SPARE */

typedef volatile union _DPE_REG_OCC_DFT_
{
		volatile struct	/* 0x1502C074 */
		{
				FIELD  occ_atpg_ct                           :  1;		/*  0.. 0, 0x00000001 */
				FIELD  occ_atpg_ob                           :  1;		/*  1.. 1, 0x00000002 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}DPE_REG_OCC_DFT;	/* DPE_OCC_DFT */

typedef volatile union _OWE_REG_WMFE_START_
{
		volatile struct	/* 0x1502C220 */
		{
				FIELD  WMFE_START                            :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_START;	/* OWE_WMFE_START */

typedef volatile union _OWE_REG_WMFE_INT_CTRL_
{
		volatile struct	/* 0x1502C224 */
		{
				FIELD  WMFE_INT_ENABLE                       :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 30;		/*  1..30, 0x7FFFFFFE */
				FIELD  WMFE_INT_CLR_MODE                     :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_INT_CTRL;	/* OWE_WMFE_INT_CTRL */

typedef volatile union _OWE_REG_WMFE_INT_STATUS_
{
		volatile struct	/* 0x1502C228 */
		{
				FIELD  WMFE_INT                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_INT_STATUS;	/* OWE_WMFE_INT_STATUS */

typedef volatile union _OWE_REG_WMFE_CTRL_0_
{
		volatile struct	/* 0x1502C230 */
		{
				FIELD  WMFE_ENABLE_0                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  WMFE_MODE_0                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WMFE_IMG_CLIP_EN_0                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  WMFE_DPND_EN_0                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  WMFE_MASK_EN_0                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  WMFE_IMGI_FMT_0                       :  2;		/*  8.. 9, 0x00000300 */
				FIELD  WMFE_DPI_FMT_0                        :  2;		/* 10..11, 0x00000C00 */
				FIELD  WMFE_FILTER_0                         :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WMFE_HORZ_SCAN_ORDER_0                :  1;		/* 16..16, 0x00010000 */
				FIELD  WMFE_VERT_SCAN_ORDER_0                :  1;		/* 17..17, 0x00020000 */
				FIELD  WMFE_MASK_MODE_0                      :  1;		/* 18..18, 0x00040000 */
				FIELD  WMFE_CHROMA_EN_0                      :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  WMFE_MASK_VALUE_0                     :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_CTRL_0;	/* OWE_WMFE_CTRL_0 */

typedef volatile union _OWE_REG_WMFE_SIZE_0_
{
		volatile struct	/* 0x1502C234 */
		{
				FIELD  WMFE_WIDTH_0                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_HEIGHT_0                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_SIZE_0;	/* OWE_WMFE_SIZE_0 */

typedef volatile union _OWE_REG_WMFE_IMGI_BASE_ADDR_0_
{
		volatile struct	/* 0x1502C238 */
		{
				FIELD  WMFE_IMGI_BASE_ADDR_0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_BASE_ADDR_0;	/* OWE_WMFE_IMGI_BASE_ADDR_0 */

typedef volatile union _OWE_REG_WMFE_IMGI_STRIDE_0_
{
		volatile struct	/* 0x1502C23C */
		{
				FIELD  WMFE_IMGI_STRIDE_0                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_STRIDE_0;	/* OWE_WMFE_IMGI_STRIDE_0 */

typedef volatile union _OWE_REG_WMFE_DPI_BASE_ADDR_0_
{
		volatile struct	/* 0x1502C240 */
		{
				FIELD  WMFE_DPI_BASE_ADDR_0                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_BASE_ADDR_0;	/* OWE_WMFE_DPI_BASE_ADDR_0 */

typedef volatile union _OWE_REG_WMFE_DPI_STRIDE_0_
{
		volatile struct	/* 0x1502C244 */
		{
				FIELD  WMFE_DPI_STRIDE_0                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_STRIDE_0;	/* OWE_WMFE_DPI_STRIDE_0 */

typedef volatile union _OWE_REG_WMFE_TBLI_BASE_ADDR_0_
{
		volatile struct	/* 0x1502C248 */
		{
				FIELD  WMFE_TBLI_BASE_ADDR_0                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_BASE_ADDR_0;	/* OWE_WMFE_TBLI_BASE_ADDR_0 */

typedef volatile union _OWE_REG_WMFE_TBLI_STRIDE_0_
{
		volatile struct	/* 0x1502C24C */
		{
				FIELD  WMFE_TBLI_STRIDE_0                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_STRIDE_0;	/* OWE_WMFE_TBLI_STRIDE_0 */

typedef volatile union _OWE_REG_WMFE_MASKI_BASE_ADDR_0_
{
		volatile struct	/* 0x1502C250 */
		{
				FIELD  WMFE_MASKI_BASE_ADDR_0                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_BASE_ADDR_0;	/* OWE_WMFE_MASKI_BASE_ADDR_0 */

typedef volatile union _OWE_REG_WMFE_MASKI_STRIDE_0_
{
		volatile struct	/* 0x1502C254 */
		{
				FIELD  WMFE_MASKI_STRIDE_0                   : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_STRIDE_0;	/* OWE_WMFE_MASKI_STRIDE_0 */

typedef volatile union _OWE_REG_WMFE_DPO_BASE_ADDR_0_
{
		volatile struct	/* 0x1502C258 */
		{
				FIELD  WMFE_DPO_BASE_ADDR_0                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_BASE_ADDR_0;	/* OWE_WMFE_DPO_BASE_ADDR_0 */

typedef volatile union _OWE_REG_WMFE_DPO_STRIDE_0_
{
		volatile struct	/* 0x1502C25C */
		{
				FIELD  WMFE_DPO_STRIDE_0                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_STRIDE_0;	/* OWE_WMFE_DPO_STRIDE_0 */

typedef volatile union _OWE_REG_WMFE_CTRL_1_
{
		volatile struct	/* 0x1502C270 */
		{
				FIELD  WMFE_ENABLE_1                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  WMFE_MODE_1                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WMFE_IMG_CLIP_EN_1                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  WMFE_DPND_EN_1                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  WMFE_MASK_EN_1                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  WMFE_IMGI_FMT_1                       :  2;		/*  8.. 9, 0x00000300 */
				FIELD  WMFE_DPI_FMT_1                        :  2;		/* 10..11, 0x00000C00 */
				FIELD  WMFE_FILTER_1                         :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WMFE_HORZ_SCAN_ORDER_1                :  1;		/* 16..16, 0x00010000 */
				FIELD  WMFE_VERT_SCAN_ORDER_1                :  1;		/* 17..17, 0x00020000 */
				FIELD  WMFE_MASK_MODE_1                      :  1;		/* 18..18, 0x00040000 */
				FIELD  WMFE_CHROMA_EN_1                      :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  WMFE_MASK_VALUE_1                     :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_CTRL_1;	/* OWE_WMFE_CTRL_1 */

typedef volatile union _OWE_REG_WMFE_SIZE_1_
{
		volatile struct	/* 0x1502C274 */
		{
				FIELD  WMFE_WIDTH_1                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_HEIGHT_1                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_SIZE_1;	/* OWE_WMFE_SIZE_1 */

typedef volatile union _OWE_REG_WMFE_IMGI_BASE_ADDR_1_
{
		volatile struct	/* 0x1502C278 */
		{
				FIELD  WMFE_IMGI_BASE_ADDR_1                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_BASE_ADDR_1;	/* OWE_WMFE_IMGI_BASE_ADDR_1 */

typedef volatile union _OWE_REG_WMFE_IMGI_STRIDE_1_
{
		volatile struct	/* 0x1502C27C */
		{
				FIELD  WMFE_IMGI_STRIDE_1                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_STRIDE_1;	/* OWE_WMFE_IMGI_STRIDE_1 */

typedef volatile union _OWE_REG_WMFE_DPI_BASE_ADDR_1_
{
		volatile struct	/* 0x1502C280 */
		{
				FIELD  WMFE_DPI_BASE_ADDR_1                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_BASE_ADDR_1;	/* OWE_WMFE_DPI_BASE_ADDR_1 */

typedef volatile union _OWE_REG_WMFE_DPI_STRIDE_1_
{
		volatile struct	/* 0x1502C284 */
		{
				FIELD  WMFE_DPI_STRIDE_1                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_STRIDE_1;	/* OWE_WMFE_DPI_STRIDE_1 */

typedef volatile union _OWE_REG_WMFE_TBLI_BASE_ADDR_1_
{
		volatile struct	/* 0x1502C288 */
		{
				FIELD  WMFE_TBLI_BASE_ADDR_1                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_BASE_ADDR_1;	/* OWE_WMFE_TBLI_BASE_ADDR_1 */

typedef volatile union _OWE_REG_WMFE_TBLI_STRIDE_1_
{
		volatile struct	/* 0x1502C28C */
		{
				FIELD  WMFE_TBLI_STRIDE_1                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_STRIDE_1;	/* OWE_WMFE_TBLI_STRIDE_1 */

typedef volatile union _OWE_REG_WMFE_MASKI_BASE_ADDR_1_
{
		volatile struct	/* 0x1502C290 */
		{
				FIELD  WMFE_MASKI_BASE_ADDR_1                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_BASE_ADDR_1;	/* OWE_WMFE_MASKI_BASE_ADDR_1 */

typedef volatile union _OWE_REG_WMFE_MASKI_STRIDE_1_
{
		volatile struct	/* 0x1502C294 */
		{
				FIELD  WMFE_MASKI_STRIDE_1                   : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_STRIDE_1;	/* OWE_WMFE_MASKI_STRIDE_1 */

typedef volatile union _OWE_REG_WMFE_DPO_BASE_ADDR_1_
{
		volatile struct	/* 0x1502C298 */
		{
				FIELD  WMFE_DPO_BASE_ADDR_1                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_BASE_ADDR_1;	/* OWE_WMFE_DPO_BASE_ADDR_1 */

typedef volatile union _OWE_REG_WMFE_DPO_STRIDE_1_
{
		volatile struct	/* 0x1502C29C */
		{
				FIELD  WMFE_DPO_STRIDE_1                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_STRIDE_1;	/* OWE_WMFE_DPO_STRIDE_1 */

typedef volatile union _OWE_REG_WMFE_CTRL_2_
{
		volatile struct	/* 0x1502C2B0 */
		{
				FIELD  WMFE_ENABLE_2                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  WMFE_MODE_2                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WMFE_IMG_CLIP_EN_2                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  WMFE_DPND_EN_2                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  WMFE_MASK_EN_2                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  WMFE_IMGI_FMT_2                       :  2;		/*  8.. 9, 0x00000300 */
				FIELD  WMFE_DPI_FMT_2                        :  2;		/* 10..11, 0x00000C00 */
				FIELD  WMFE_FILTER_2                         :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WMFE_HORZ_SCAN_ORDER_2                :  1;		/* 16..16, 0x00010000 */
				FIELD  WMFE_VERT_SCAN_ORDER_2                :  1;		/* 17..17, 0x00020000 */
				FIELD  WMFE_MASK_MODE_2                      :  1;		/* 18..18, 0x00040000 */
				FIELD  WMFE_CHROMA_EN_2                      :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  WMFE_MASK_VALUE_2                     :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_CTRL_2;	/* OWE_WMFE_CTRL_2 */

typedef volatile union _OWE_REG_WMFE_SIZE_2_
{
		volatile struct	/* 0x1502C2B4 */
		{
				FIELD  WMFE_WIDTH_2                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_HEIGHT_2                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_SIZE_2;	/* OWE_WMFE_SIZE_2 */

typedef volatile union _OWE_REG_WMFE_IMGI_BASE_ADDR_2_
{
		volatile struct	/* 0x1502C2B8 */
		{
				FIELD  WMFE_IMGI_BASE_ADDR_2                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_BASE_ADDR_2;	/* OWE_WMFE_IMGI_BASE_ADDR_2 */

typedef volatile union _OWE_REG_WMFE_IMGI_STRIDE_2_
{
		volatile struct	/* 0x1502C2BC */
		{
				FIELD  WMFE_IMGI_STRIDE_2                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_STRIDE_2;	/* OWE_WMFE_IMGI_STRIDE_2 */

typedef volatile union _OWE_REG_WMFE_DPI_BASE_ADDR_2_
{
		volatile struct	/* 0x1502C2C0 */
		{
				FIELD  WMFE_DPI_BASE_ADDR_2                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_BASE_ADDR_2;	/* OWE_WMFE_DPI_BASE_ADDR_2 */

typedef volatile union _OWE_REG_WMFE_DPI_STRIDE_2_
{
		volatile struct	/* 0x1502C2C4 */
		{
				FIELD  WMFE_DPI_STRIDE_2                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_STRIDE_2;	/* OWE_WMFE_DPI_STRIDE_2 */

typedef volatile union _OWE_REG_WMFE_TBLI_BASE_ADDR_2_
{
		volatile struct	/* 0x1502C2C8 */
		{
				FIELD  WMFE_TBLI_BASE_ADDR_2                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_BASE_ADDR_2;	/* OWE_WMFE_TBLI_BASE_ADDR_2 */

typedef volatile union _OWE_REG_WMFE_TBLI_STRIDE_2_
{
		volatile struct	/* 0x1502C2CC */
		{
				FIELD  WMFE_TBLI_STRIDE_2                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_STRIDE_2;	/* OWE_WMFE_TBLI_STRIDE_2 */

typedef volatile union _OWE_REG_WMFE_MASKI_BASE_ADDR_2_
{
		volatile struct	/* 0x1502C2D0 */
		{
				FIELD  WMFE_MASKI_BASE_ADDR_2                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_BASE_ADDR_2;	/* OWE_WMFE_MASKI_BASE_ADDR_2 */

typedef volatile union _OWE_REG_WMFE_MASKI_STRIDE_2_
{
		volatile struct	/* 0x1502C2D4 */
		{
				FIELD  WMFE_MASKI_STRIDE_2                   : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_STRIDE_2;	/* OWE_WMFE_MASKI_STRIDE_2 */

typedef volatile union _OWE_REG_WMFE_DPO_BASE_ADDR_2_
{
		volatile struct	/* 0x1502C2D8 */
		{
				FIELD  WMFE_DPO_BASE_ADDR_2                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_BASE_ADDR_2;	/* OWE_WMFE_DPO_BASE_ADDR_2 */

typedef volatile union _OWE_REG_WMFE_DPO_STRIDE_2_
{
		volatile struct	/* 0x1502C2DC */
		{
				FIELD  WMFE_DPO_STRIDE_2                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_STRIDE_2;	/* OWE_WMFE_DPO_STRIDE_2 */

typedef volatile union _OWE_REG_WMFE_CTRL_3_
{
		volatile struct	/* 0x1502C2F0 */
		{
				FIELD  WMFE_ENABLE_3                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  WMFE_MODE_3                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WMFE_IMG_CLIP_EN_3                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  WMFE_DPND_EN_3                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  WMFE_MASK_EN_3                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  WMFE_IMGI_FMT_3                       :  2;		/*  8.. 9, 0x00000300 */
				FIELD  WMFE_DPI_FMT_3                        :  2;		/* 10..11, 0x00000C00 */
				FIELD  WMFE_FILTER_3                         :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WMFE_HORZ_SCAN_ORDER_3                :  1;		/* 16..16, 0x00010000 */
				FIELD  WMFE_VERT_SCAN_ORDER_3                :  1;		/* 17..17, 0x00020000 */
				FIELD  WMFE_MASK_MODE_3                      :  1;		/* 18..18, 0x00040000 */
				FIELD  WMFE_CHROMA_EN_3                      :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  WMFE_MASK_VALUE_3                     :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_CTRL_3;	/* OWE_WMFE_CTRL_3 */

typedef volatile union _OWE_REG_WMFE_SIZE_3_
{
		volatile struct	/* 0x1502C2F4 */
		{
				FIELD  WMFE_WIDTH_3                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_HEIGHT_3                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_SIZE_3;	/* OWE_WMFE_SIZE_3 */

typedef volatile union _OWE_REG_WMFE_IMGI_BASE_ADDR_3_
{
		volatile struct	/* 0x1502C2F8 */
		{
				FIELD  WMFE_IMGI_BASE_ADDR_3                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_BASE_ADDR_3;	/* OWE_WMFE_IMGI_BASE_ADDR_3 */

typedef volatile union _OWE_REG_WMFE_IMGI_STRIDE_3_
{
		volatile struct	/* 0x1502C2FC */
		{
				FIELD  WMFE_IMGI_STRIDE_3                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_STRIDE_3;	/* OWE_WMFE_IMGI_STRIDE_3 */

typedef volatile union _OWE_REG_WMFE_DPI_BASE_ADDR_3_
{
		volatile struct	/* 0x1502C300 */
		{
				FIELD  WMFE_DPI_BASE_ADDR_3                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_BASE_ADDR_3;	/* OWE_WMFE_DPI_BASE_ADDR_3 */

typedef volatile union _OWE_REG_WMFE_DPI_STRIDE_3_
{
		volatile struct	/* 0x1502C304 */
		{
				FIELD  WMFE_DPI_STRIDE_3                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_STRIDE_3;	/* OWE_WMFE_DPI_STRIDE_3 */

typedef volatile union _OWE_REG_WMFE_TBLI_BASE_ADDR_3_
{
		volatile struct	/* 0x1502C308 */
		{
				FIELD  WMFE_TBLI_BASE_ADDR_3                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_BASE_ADDR_3;	/* OWE_WMFE_TBLI_BASE_ADDR_3 */

typedef volatile union _OWE_REG_WMFE_TBLI_STRIDE_3_
{
		volatile struct	/* 0x1502C30C */
		{
				FIELD  WMFE_TBLI_STRIDE_3                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_STRIDE_3;	/* OWE_WMFE_TBLI_STRIDE_3 */

typedef volatile union _OWE_REG_WMFE_MASKI_BASE_ADDR_3_
{
		volatile struct	/* 0x1502C310 */
		{
				FIELD  WMFE_MASKI_BASE_ADDR_3                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_BASE_ADDR_3;	/* OWE_WMFE_MASKI_BASE_ADDR_3 */

typedef volatile union _OWE_REG_WMFE_MASKI_STRIDE_3_
{
		volatile struct	/* 0x1502C314 */
		{
				FIELD  WMFE_MASKI_STRIDE_3                   : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_STRIDE_3;	/* OWE_WMFE_MASKI_STRIDE_3 */

typedef volatile union _OWE_REG_WMFE_DPO_BASE_ADDR_3_
{
		volatile struct	/* 0x1502C318 */
		{
				FIELD  WMFE_DPO_BASE_ADDR_3                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_BASE_ADDR_3;	/* OWE_WMFE_DPO_BASE_ADDR_3 */

typedef volatile union _OWE_REG_WMFE_DPO_STRIDE_3_
{
		volatile struct	/* 0x1502C31C */
		{
				FIELD  WMFE_DPO_STRIDE_3                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_STRIDE_3;	/* OWE_WMFE_DPO_STRIDE_3 */

typedef volatile union _OWE_REG_WMFE_CTRL_4_
{
		volatile struct	/* 0x1502C330 */
		{
				FIELD  WMFE_ENABLE_4                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 :  3;		/*  1.. 3, 0x0000000E */
				FIELD  WMFE_MODE_4                           :  1;		/*  4.. 4, 0x00000010 */
				FIELD  WMFE_IMG_CLIP_EN_4                    :  1;		/*  5.. 5, 0x00000020 */
				FIELD  WMFE_DPND_EN_4                        :  1;		/*  6.. 6, 0x00000040 */
				FIELD  WMFE_MASK_EN_4                        :  1;		/*  7.. 7, 0x00000080 */
				FIELD  WMFE_IMGI_FMT_4                       :  2;		/*  8.. 9, 0x00000300 */
				FIELD  WMFE_DPI_FMT_4                        :  2;		/* 10..11, 0x00000C00 */
				FIELD  WMFE_FILTER_4                         :  2;		/* 12..13, 0x00003000 */
				FIELD  rsv_14                                :  2;		/* 14..15, 0x0000C000 */
				FIELD  WMFE_HORZ_SCAN_ORDER_4                :  1;		/* 16..16, 0x00010000 */
				FIELD  WMFE_VERT_SCAN_ORDER_4                :  1;		/* 17..17, 0x00020000 */
				FIELD  WMFE_MASK_MODE_4                      :  1;		/* 18..18, 0x00040000 */
				FIELD  WMFE_CHROMA_EN_4                      :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                :  4;		/* 20..23, 0x00F00000 */
				FIELD  WMFE_MASK_VALUE_4                     :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_CTRL_4;	/* OWE_WMFE_CTRL_4 */

typedef volatile union _OWE_REG_WMFE_SIZE_4_
{
		volatile struct	/* 0x1502C334 */
		{
				FIELD  WMFE_WIDTH_4                          : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_HEIGHT_4                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_SIZE_4;	/* OWE_WMFE_SIZE_4 */

typedef volatile union _OWE_REG_WMFE_IMGI_BASE_ADDR_4_
{
		volatile struct	/* 0x1502C338 */
		{
				FIELD  WMFE_IMGI_BASE_ADDR_4                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_BASE_ADDR_4;	/* OWE_WMFE_IMGI_BASE_ADDR_4 */

typedef volatile union _OWE_REG_WMFE_IMGI_STRIDE_4_
{
		volatile struct	/* 0x1502C33C */
		{
				FIELD  WMFE_IMGI_STRIDE_4                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_IMGI_STRIDE_4;	/* OWE_WMFE_IMGI_STRIDE_4 */

typedef volatile union _OWE_REG_WMFE_DPI_BASE_ADDR_4_
{
		volatile struct	/* 0x1502C340 */
		{
				FIELD  WMFE_DPI_BASE_ADDR_4                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_BASE_ADDR_4;	/* OWE_WMFE_DPI_BASE_ADDR_4 */

typedef volatile union _OWE_REG_WMFE_DPI_STRIDE_4_
{
		volatile struct	/* 0x1502C344 */
		{
				FIELD  WMFE_DPI_STRIDE_4                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPI_STRIDE_4;	/* OWE_WMFE_DPI_STRIDE_4 */

typedef volatile union _OWE_REG_WMFE_TBLI_BASE_ADDR_4_
{
		volatile struct	/* 0x1502C348 */
		{
				FIELD  WMFE_TBLI_BASE_ADDR_4                 : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_BASE_ADDR_4;	/* OWE_WMFE_TBLI_BASE_ADDR_4 */

typedef volatile union _OWE_REG_WMFE_TBLI_STRIDE_4_
{
		volatile struct	/* 0x1502C34C */
		{
				FIELD  WMFE_TBLI_STRIDE_4                    : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_TBLI_STRIDE_4;	/* OWE_WMFE_TBLI_STRIDE_4 */

typedef volatile union _OWE_REG_WMFE_MASKI_BASE_ADDR_4_
{
		volatile struct	/* 0x1502C350 */
		{
				FIELD  WMFE_MASKI_BASE_ADDR_4                : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_BASE_ADDR_4;	/* OWE_WMFE_MASKI_BASE_ADDR_4 */

typedef volatile union _OWE_REG_WMFE_MASKI_STRIDE_4_
{
		volatile struct	/* 0x1502C354 */
		{
				FIELD  WMFE_MASKI_STRIDE_4                   : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_MASKI_STRIDE_4;	/* OWE_WMFE_MASKI_STRIDE_4 */

typedef volatile union _OWE_REG_WMFE_DPO_BASE_ADDR_4_
{
		volatile struct	/* 0x1502C358 */
		{
				FIELD  WMFE_DPO_BASE_ADDR_4                  : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_BASE_ADDR_4;	/* OWE_WMFE_DPO_BASE_ADDR_4 */

typedef volatile union _OWE_REG_WMFE_DPO_STRIDE_4_
{
		volatile struct	/* 0x1502C35C */
		{
				FIELD  WMFE_DPO_STRIDE_4                     : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DPO_STRIDE_4;	/* OWE_WMFE_DPO_STRIDE_4 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_00_
{
		volatile struct	/* 0x1502C400 */
		{
				FIELD  WMFE_CTRL_FSM                         :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 :  1;		/*  3.. 3, 0x00000008 */
				FIELD  WMFE_PRCSS_FSM                        :  2;		/*  4.. 5, 0x00000030 */
				FIELD  rsv_6                                 :  2;		/*  6.. 7, 0x000000C0 */
				FIELD  WMFE_RDMA_FSM                         :  3;		/*  8..10, 0x00000700 */
				FIELD  rsv_11                                :  1;		/* 11..11, 0x00000800 */
				FIELD  WMFE_WDMA_FSM                         :  3;		/* 12..14, 0x00007000 */
				FIELD  rsv_15                                :  1;		/* 15..15, 0x00008000 */
				FIELD  WMFE_C2Y_FSM                          :  2;		/* 16..17, 0x00030000 */
				FIELD  rsv_18                                :  2;		/* 18..19, 0x000C0000 */
				FIELD  WMFE_H2X_FSM                          :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  2;		/* 22..23, 0x00C00000 */
				FIELD  WMFE_B2B_FSM                          :  2;		/* 24..25, 0x03000000 */
				FIELD  rsv_26                                :  2;		/* 26..27, 0x0C000000 */
				FIELD  WMFE_HFLIP_FSM                        :  2;		/* 28..29, 0x30000000 */
				FIELD  rsv_30                                :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_00;	/* OWE_WMFE_DBG_INFO_00 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_01_
{
		volatile struct	/* 0x1502C404 */
		{
				FIELD  WMFE_TRIGG_CNT                        :  3;		/*  0.. 2, 0x00000007 */
				FIELD  rsv_3                                 : 13;		/*  3..15, 0x0000FFF8 */
				FIELD  WMFE_LOAD_LINE_CNT                    : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_01;	/* OWE_WMFE_DBG_INFO_01 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_02_
{
		volatile struct	/* 0x1502C408 */
		{
				FIELD  WMFE_CURR_IMG_X                       : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_CURR_DP_X                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_02;	/* OWE_WMFE_DBG_INFO_02 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_03_
{
		volatile struct	/* 0x1502C40C */
		{
				FIELD  WMFE_CURR_MASK_X                      : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_03;	/* OWE_WMFE_DBG_INFO_03 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_04_
{
		volatile struct	/* 0x1502C410 */
		{
				FIELD  WMFE_CURR_X                           : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_CURR_Y                           : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_04;	/* OWE_WMFE_DBG_INFO_04 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_05_
{
		volatile struct	/* 0x1502C414 */
		{
				FIELD  WMFE_IMG_R_CNT                        :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  WMFE_IMG_W_CNT                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_05;	/* OWE_WMFE_DBG_INFO_05 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_06_
{
		volatile struct	/* 0x1502C418 */
		{
				FIELD  WMFE_MASK_R_CNT                       :  5;		/*  0.. 4, 0x0000001F */
				FIELD  rsv_5                                 : 11;		/*  5..15, 0x0000FFE0 */
				FIELD  WMFE_MASK_W_CNT                       :  5;		/* 16..20, 0x001F0000 */
				FIELD  rsv_21                                : 11;		/* 21..31, 0xFFE00000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_06;	/* OWE_WMFE_DBG_INFO_06 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_07_
{
		volatile struct	/* 0x1502C41C */
		{
				FIELD  WMFE_DP_R_CNT                         :  8;		/*  0.. 7, 0x000000FF */
				FIELD  rsv_8                                 :  8;		/*  8..15, 0x0000FF00 */
				FIELD  WMFE_DP_W_CNT                         :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  8;		/* 24..31, 0xFF000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_07;	/* OWE_WMFE_DBG_INFO_07 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_08_
{
		volatile struct	/* 0x1502C420 */
		{
				FIELD  WMFE_IMGI_Y_CNT                       : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  WMFE_DPI_Y_CNT                        : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_08;	/* OWE_WMFE_DBG_INFO_08 */

typedef volatile union _OWE_REG_WMFE_DBG_INFO_09_
{
		volatile struct	/* 0x1502C424 */
		{
				FIELD  WMFE_MASKI_Y_CNT                      : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 20;		/* 12..31, 0xFFFFF000 */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_DBG_INFO_09;	/* OWE_WMFE_DBG_INFO_09 */

typedef volatile union _OWE_REG_WMFE_SPARE_0_
{
		volatile struct	/* 0x1502C4F8 */
		{
				FIELD  WMFE_SPARE_0                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_SPARE_0;	/* OWE_WMFE_SPARE_0 */

typedef volatile union _OWE_REG_WMFE_SPARE_1_
{
		volatile struct	/* 0x1502C4FC */
		{
				FIELD  WMFE_SPARE_1                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_WMFE_SPARE_1;	/* OWE_WMFE_SPARE_1 */

typedef volatile union _OWE_REG_DMA_DBG_
{
		volatile struct	/* 0x1502C7F4 */
		{
				FIELD  DMA_DBG_DATA                          : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_DMA_DBG;	/* OWE_DMA_DBG */

typedef volatile union _OWE_REG_DMA_REQ_STATUS_
{
		volatile struct	/* 0x1502C7F8 */
		{
				FIELD  DMA_REQ_STATUS                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_DMA_REQ_STATUS;	/* OWE_DMA_REQ_STATUS */

typedef volatile union _OWE_REG_DMA_RDY_STATUS_
{
		volatile struct	/* 0x1502C7FC */
		{
				FIELD  DMA_RDY_STATUS                        : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWE_REG_DMA_RDY_STATUS;	/* OWE_DMA_RDY_STATUS */

typedef volatile union _DMA_REG_SOFT_RSTSTAT_
{
		volatile struct	/* 0x1502C800 */
		{
				FIELD  OWEO_SOFT_RST_STAT                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  OWEI_SOFT_RST_STAT                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}DMA_REG_SOFT_RSTSTAT;	/* DMA_SOFT_RSTSTAT */

typedef volatile union _VERTICAL_REG_FLIP_EN_
{
		volatile struct	/* 0x1502C804 */
		{
				FIELD  OWEO_V_FLIP_EN                        :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  OWEI_V_FLIP_EN                        :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}VERTICAL_REG_FLIP_EN;	/* VERTICAL_FLIP_EN */

typedef volatile union _DMA_REG_SOFT_RESET_
{
		volatile struct	/* 0x1502C808 */
		{
				FIELD  OWEO_SOFT_RST                         :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  OWEI_SOFT_RST                         :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  SEPARATE_SOFT_RST_EN                  :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DMA_REG_SOFT_RESET;	/* DMA_SOFT_RESET */

typedef volatile union _LAST_REG_ULTRA_EN_
{
		volatile struct	/* 0x1502C80C */
		{
				FIELD  OWEO_LAST_ULTRA_EN                    :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  OWEI_LAST_ULTRA_EN                    :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 15;		/* 17..31, 0xFFFE0000 */
		} Bits;
		UINT32 Raw;
}LAST_REG_ULTRA_EN;	/* LAST_ULTRA_EN */

typedef volatile union _SPECIAL_REG_FUN_EN_
{
		volatile struct	/* 0x1502C810 */
		{
				FIELD  SLOW_CNT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                :  1;		/* 16..16, 0x00010000 */
				FIELD  CONTINUOUS_COM_CON                    :  2;		/* 17..18, 0x00060000 */
				FIELD  CONTINUOUS_COM_EN                     :  1;		/* 19..19, 0x00080000 */
				FIELD  rsv_20                                : 10;		/* 20..29, 0x3FF00000 */
				FIELD  INTERLACE_MODE                        :  1;		/* 30..30, 0x40000000 */
				FIELD  SLOW_EN                               :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}SPECIAL_REG_FUN_EN;	/* SPECIAL_FUN_EN */

typedef volatile union _OWEO_REG_BASE_ADDR_
{
		volatile struct	/* 0x1502C830 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWEO_REG_BASE_ADDR;	/* OWEO_BASE_ADDR */

typedef volatile union _OWEO_REG_BASE_ADDR_2_
{
		volatile struct	/* 0x1502C834 */
		{
				FIELD  BASE_ADDR_2                           :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}OWEO_REG_BASE_ADDR_2;	/* OWEO_BASE_ADDR_2 */

typedef volatile union _OWEO_REG_OFST_ADDR_
{
		volatile struct	/* 0x1502C838 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWEO_REG_OFST_ADDR;	/* OWEO_OFST_ADDR */

typedef volatile union _OWEO_REG_OFST_ADDR_2_
{
		volatile struct	/* 0x1502C83C */
		{
				FIELD  OFFSET_ADDR_2                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}OWEO_REG_OFST_ADDR_2;	/* OWEO_OFST_ADDR_2 */

typedef volatile union _OWEO_REG_XSIZE_
{
		volatile struct	/* 0x1502C840 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWEO_REG_XSIZE;	/* OWEO_XSIZE */

typedef volatile union _OWEO_REG_YSIZE_
{
		volatile struct	/* 0x1502C844 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWEO_REG_YSIZE;	/* OWEO_YSIZE */

typedef volatile union _OWEO_REG_STRIDE_
{
		volatile struct	/* 0x1502C848 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWEO_REG_STRIDE;	/* OWEO_STRIDE */

typedef volatile union _OWEO_REG_CON_
{
		volatile struct	/* 0x1502C84C */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWEO_REG_CON;	/* OWEO_CON */

typedef volatile union _OWEO_REG_CON2_
{
		volatile struct	/* 0x1502C850 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWEO_REG_CON2;	/* OWEO_CON2 */

typedef volatile union _OWEO_REG_CON3_
{
		volatile struct	/* 0x1502C854 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWEO_REG_CON3;	/* OWEO_CON3 */

typedef volatile union _OWEI_REG_BASE_ADDR_
{
		volatile struct	/* 0x1502C890 */
		{
				FIELD  BASE_ADDR                             : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWEI_REG_BASE_ADDR;	/* OWEI_BASE_ADDR */

typedef volatile union _OWEI_REG_BASE_ADDR_2_
{
		volatile struct	/* 0x1502C894 */
		{
				FIELD  BASE_ADDR_2                           :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}OWEI_REG_BASE_ADDR_2;	/* OWEI_BASE_ADDR_2 */

typedef volatile union _OWEI_REG_OFST_ADDR_
{
		volatile struct	/* 0x1502C898 */
		{
				FIELD  OFFSET_ADDR                           : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}OWEI_REG_OFST_ADDR;	/* OWEI_OFST_ADDR */

typedef volatile union _OWEI_REG_OFST_ADDR_2_
{
		volatile struct	/* 0x1502C89C */
		{
				FIELD  OFFSET_ADDR_2                         :  2;		/*  0.. 1, 0x00000003 */
				FIELD  rsv_2                                 : 30;		/*  2..31, 0xFFFFFFFC */
		} Bits;
		UINT32 Raw;
}OWEI_REG_OFST_ADDR_2;	/* OWEI_OFST_ADDR_2 */

typedef volatile union _OWEI_REG_XSIZE_
{
		volatile struct	/* 0x1502C8A0 */
		{
				FIELD  XSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWEI_REG_XSIZE;	/* OWEI_XSIZE */

typedef volatile union _OWEI_REG_YSIZE_
{
		volatile struct	/* 0x1502C8A4 */
		{
				FIELD  YSIZE                                 : 16;		/*  0..15, 0x0000FFFF */
				FIELD  rsv_16                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWEI_REG_YSIZE;	/* OWEI_YSIZE */

typedef volatile union _OWEI_REG_STRIDE_
{
		volatile struct	/* 0x1502C8A8 */
		{
				FIELD  STRIDE                                : 16;		/*  0..15, 0x0000FFFF */
				FIELD  BUS_SIZE                              :  4;		/* 16..19, 0x000F0000 */
				FIELD  FORMAT                                :  2;		/* 20..21, 0x00300000 */
				FIELD  rsv_22                                :  1;		/* 22..22, 0x00400000 */
				FIELD  FORMAT_EN                             :  1;		/* 23..23, 0x00800000 */
				FIELD  BUS_SIZE_EN                           :  1;		/* 24..24, 0x01000000 */
				FIELD  rsv_25                                :  5;		/* 25..29, 0x3E000000 */
				FIELD  SWAP                                  :  2;		/* 30..31, 0xC0000000 */
		} Bits;
		UINT32 Raw;
}OWEI_REG_STRIDE;	/* OWEI_STRIDE */

typedef volatile union _OWEI_REG_CON_
{
		volatile struct	/* 0x1502C8AC */
		{
				FIELD  FIFO_SIZE                             : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                : 16;		/* 12..27, 0x0FFFF000 */
				FIELD  MAX_BURST_LEN                         :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWEI_REG_CON;	/* OWEI_CON */

typedef volatile union _OWEI_REG_CON2_
{
		volatile struct	/* 0x1502C8B0 */
		{
				FIELD  FIFO_PRI_THRL                         : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRI_THRH                         : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWEI_REG_CON2;	/* OWEI_CON2 */

typedef volatile union _OWEI_REG_CON3_
{
		volatile struct	/* 0x1502C8B4 */
		{
				FIELD  FIFO_PRE_PRI_THRL                     : 12;		/*  0..11, 0x00000FFF */
				FIELD  rsv_12                                :  4;		/* 12..15, 0x0000F000 */
				FIELD  FIFO_PRE_PRI_THRH                     : 12;		/* 16..27, 0x0FFF0000 */
				FIELD  rsv_28                                :  4;		/* 28..31, 0xF0000000 */
		} Bits;
		UINT32 Raw;
}OWEI_REG_CON3;	/* OWEI_CON3 */

typedef volatile union _DMA_REG_ERR_CTRL_
{
		volatile struct	/* 0x1502C900 */
		{
				FIELD  OWEO_ERR                              :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 15;		/*  1..15, 0x0000FFFE */
				FIELD  OWEI_ERR                              :  1;		/* 16..16, 0x00010000 */
				FIELD  rsv_17                                : 14;		/* 17..30, 0x7FFE0000 */
				FIELD  ERR_CLR_MD                            :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DMA_REG_ERR_CTRL;	/* DMA_ERR_CTRL */

typedef volatile union _OWEO_REG_ERR_STAT_
{
		volatile struct	/* 0x1502C904 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWEO_REG_ERR_STAT;	/* OWEO_ERR_STAT */

typedef volatile union _OWEI_REG_ERR_STAT_
{
		volatile struct	/* 0x1502C908 */
		{
				FIELD  ERR_STAT                              : 16;		/*  0..15, 0x0000FFFF */
				FIELD  ERR_EN                                : 16;		/* 16..31, 0xFFFF0000 */
		} Bits;
		UINT32 Raw;
}OWEI_REG_ERR_STAT;	/* OWEI_ERR_STAT */

typedef volatile union _DMA_REG_DEBUG_ADDR_
{
		volatile struct	/* 0x1502C90C */
		{
				FIELD  DEBUG_ADDR                            : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DMA_REG_DEBUG_ADDR;	/* DMA_DEBUG_ADDR */

typedef volatile union _DMA_REG_RSV1_
{
		volatile struct	/* 0x1502C910 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DMA_REG_RSV1;	/* DMA_RSV1 */

typedef volatile union _DMA_REG_RSV2_
{
		volatile struct	/* 0x1502C914 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DMA_REG_RSV2;	/* DMA_RSV2 */

typedef volatile union _DMA_REG_RSV3_
{
		volatile struct	/* 0x1502C918 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DMA_REG_RSV3;	/* DMA_RSV3 */

typedef volatile union _DMA_REG_RSV4_
{
		volatile struct	/* 0x1502C91C */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DMA_REG_RSV4;	/* DMA_RSV4 */

typedef volatile union _DMA_REG_RSV5_
{
		volatile struct	/* 0x1502C920 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DMA_REG_RSV5;	/* DMA_RSV5 */

typedef volatile union _DMA_REG_RSV6_
{
		volatile struct	/* 0x1502C924 */
		{
				FIELD  RSV                                   : 32;		/*  0..31, 0xFFFFFFFF */
		} Bits;
		UINT32 Raw;
}DMA_REG_RSV6;	/* DMA_RSV6 */

typedef volatile union _DMA_REG_DEBUG_SEL_
{
		volatile struct	/* 0x1502C928 */
		{
				FIELD  DMA_TOP_SEL                           :  8;		/*  0.. 7, 0x000000FF */
				FIELD  R_W_DMA_TOP_SEL                       :  8;		/*  8..15, 0x0000FF00 */
				FIELD  SUB_MODULE_SEL                        :  8;		/* 16..23, 0x00FF0000 */
				FIELD  rsv_24                                :  6;		/* 24..29, 0x3F000000 */
				FIELD  ARBITER_BVALID_FULL                   :  1;		/* 30..30, 0x40000000 */
				FIELD  ARBITER_COM_FULL                      :  1;		/* 31..31, 0x80000000 */
		} Bits;
		UINT32 Raw;
}DMA_REG_DEBUG_SEL;	/* DMA_DEBUG_SEL */

typedef volatile union _DMA_REG_BW_SELF_TEST_
{
		volatile struct	/* 0x1502C92C */
		{
				FIELD  BW_SELF_TEST_EN_OWEO                  :  1;		/*  0.. 0, 0x00000001 */
				FIELD  rsv_1                                 : 31;		/*  1..31, 0xFFFFFFFE */
		} Bits;
		UINT32 Raw;
}DMA_REG_BW_SELF_TEST;	/* DMA_BW_SELF_TEST */

typedef volatile struct _owe_reg_t_	/* 0x1502C000..0x1502C97F */
{
	OWE_REG_RST                                     OWE_RST;                                         /* 0000, 0x1502C000 */
	OWE_REG_DCM_CTRL                                OWE_DCM_CTRL;                                    /* 0004, 0x1502C004 */
	OWE_REG_INT_CTRL                                OWE_INT_CTRL;                                    /* 0008, 0x1502C008 */
	UINT32                                          rsv_000C;                                        /* 000C, 0x1502C00C */
	OWE_REG_VERSION                                 OWE_VERSION;                                     /* 0010, 0x1502C010 */
	OWE_REG_DCM_STATUS                              OWE_DCM_STATUS;                                  /* 0014, 0x1502C014 */
	OWE_REG_INT_STATUS                              OWE_INT_STATUS;                                  /* 0018, 0x1502C018 */
	OWE_REG_DBG_INFO_0                              OWE_DBG_INFO_0;                                  /* 001C, 0x1502C01C */
	OWE_REG_OCC_START                               OWE_OCC_START;                                   /* 0020, 0x1502C020 */
	OWE_REG_OCC_INT_CTRL                            OWE_OCC_INT_CTRL;                                /* 0024, 0x1502C024 */
	OWE_REG_OCC_INT_STATUS                          OWE_OCC_INT_STATUS;                              /* 0028, 0x1502C028 */
	UINT32                                          rsv_002C;                                        /* 002C, 0x1502C02C */
	DPE_REG_OCC_CTRL_0                              DPE_OCC_CTRL_0;                                  /* 0030, 0x1502C030 */
	DPE_REG_OCC_CTRL_1                              DPE_OCC_CTRL_1;                                  /* 0034, 0x1502C034 */
	DPE_REG_OCC_CTRL_2                              DPE_OCC_CTRL_2;                                  /* 0038, 0x1502C038 */
	DPE_REG_OCC_CTRL_3                              DPE_OCC_CTRL_3;                                  /* 003C, 0x1502C03C */
	DPE_REG_OCC_REF_VEC_BASE                        DPE_OCC_REF_VEC_BASE;                            /* 0040, 0x1502C040 */
	DPE_REG_OCC_REF_VEC_STRIDE                      DPE_OCC_REF_VEC_STRIDE;                          /* 0044, 0x1502C044 */
	DPE_REG_OCC_REF_PXL_BASE                        DPE_OCC_REF_PXL_BASE;                            /* 0048, 0x1502C048 */
	DPE_REG_OCC_REF_PXL_STRIDE                      DPE_OCC_REF_PXL_STRIDE;                          /* 004C, 0x1502C04C */
	DPE_REG_OCC_MAJ_VEC_BASE                        DPE_OCC_MAJ_VEC_BASE;                            /* 0050, 0x1502C050 */
	DPE_REG_OCC_MAJ_VEC_STRIDE                      DPE_OCC_MAJ_VEC_STRIDE;                          /* 0054, 0x1502C054 */
	DPE_REG_OCC_MAJ_PXL_BASE                        DPE_OCC_MAJ_PXL_BASE;                            /* 0058, 0x1502C058 */
	DPE_REG_OCC_MAJ_PXL_STRIDE                      DPE_OCC_MAJ_PXL_STRIDE;                          /* 005C, 0x1502C05C */
	DPE_REG_OCC_WDMA_BASE                           DPE_OCC_WDMA_BASE;                               /* 0060, 0x1502C060 */
	DPE_REG_OCC_WDMA_STRIDE                         DPE_OCC_WDMA_STRIDE;                             /* 0064, 0x1502C064 */
	DPE_REG_OCC_PQ_0                                DPE_OCC_PQ_0;                                    /* 0068, 0x1502C068 */
	DPE_REG_OCC_PQ_1                                DPE_OCC_PQ_1;                                    /* 006C, 0x1502C06C */
	DPE_REG_OCC_SPARE                               DPE_OCC_SPARE;                                   /* 0070, 0x1502C070 */
	DPE_REG_OCC_DFT                                 DPE_OCC_DFT;                                     /* 0074, 0x1502C074 */
	UINT32                                          rsv_0078[106];                                   /* 0078..021F, 0x1502C078..1502C21F */
	OWE_REG_WMFE_START                              OWE_WMFE_START;                                  /* 0220, 0x1502C220 */
	OWE_REG_WMFE_INT_CTRL                           OWE_WMFE_INT_CTRL;                               /* 0224, 0x1502C224 */
	OWE_REG_WMFE_INT_STATUS                         OWE_WMFE_INT_STATUS;                             /* 0228, 0x1502C228 */
	UINT32                                          rsv_022C;                                        /* 022C, 0x1502C22C */
	OWE_REG_WMFE_CTRL_0                             OWE_WMFE_CTRL_0;                                 /* 0230, 0x1502C230 */
	OWE_REG_WMFE_SIZE_0                             OWE_WMFE_SIZE_0;                                 /* 0234, 0x1502C234 */
	OWE_REG_WMFE_IMGI_BASE_ADDR_0                   OWE_WMFE_IMGI_BASE_ADDR_0;                       /* 0238, 0x1502C238 */
	OWE_REG_WMFE_IMGI_STRIDE_0                      OWE_WMFE_IMGI_STRIDE_0;                          /* 023C, 0x1502C23C */
	OWE_REG_WMFE_DPI_BASE_ADDR_0                    OWE_WMFE_DPI_BASE_ADDR_0;                        /* 0240, 0x1502C240 */
	OWE_REG_WMFE_DPI_STRIDE_0                       OWE_WMFE_DPI_STRIDE_0;                           /* 0244, 0x1502C244 */
	OWE_REG_WMFE_TBLI_BASE_ADDR_0                   OWE_WMFE_TBLI_BASE_ADDR_0;                       /* 0248, 0x1502C248 */
	OWE_REG_WMFE_TBLI_STRIDE_0                      OWE_WMFE_TBLI_STRIDE_0;                          /* 024C, 0x1502C24C */
	OWE_REG_WMFE_MASKI_BASE_ADDR_0                  OWE_WMFE_MASKI_BASE_ADDR_0;                      /* 0250, 0x1502C250 */
	OWE_REG_WMFE_MASKI_STRIDE_0                     OWE_WMFE_MASKI_STRIDE_0;                         /* 0254, 0x1502C254 */
	OWE_REG_WMFE_DPO_BASE_ADDR_0                    OWE_WMFE_DPO_BASE_ADDR_0;                        /* 0258, 0x1502C258 */
	OWE_REG_WMFE_DPO_STRIDE_0                       OWE_WMFE_DPO_STRIDE_0;                           /* 025C, 0x1502C25C */
	UINT32                                          rsv_0260[4];                                     /* 0260..026F, 0x1502C260..1502C26F */
	OWE_REG_WMFE_CTRL_1                             OWE_WMFE_CTRL_1;                                 /* 0270, 0x1502C270 */
	OWE_REG_WMFE_SIZE_1                             OWE_WMFE_SIZE_1;                                 /* 0274, 0x1502C274 */
	OWE_REG_WMFE_IMGI_BASE_ADDR_1                   OWE_WMFE_IMGI_BASE_ADDR_1;                       /* 0278, 0x1502C278 */
	OWE_REG_WMFE_IMGI_STRIDE_1                      OWE_WMFE_IMGI_STRIDE_1;                          /* 027C, 0x1502C27C */
	OWE_REG_WMFE_DPI_BASE_ADDR_1                    OWE_WMFE_DPI_BASE_ADDR_1;                        /* 0280, 0x1502C280 */
	OWE_REG_WMFE_DPI_STRIDE_1                       OWE_WMFE_DPI_STRIDE_1;                           /* 0284, 0x1502C284 */
	OWE_REG_WMFE_TBLI_BASE_ADDR_1                   OWE_WMFE_TBLI_BASE_ADDR_1;                       /* 0288, 0x1502C288 */
	OWE_REG_WMFE_TBLI_STRIDE_1                      OWE_WMFE_TBLI_STRIDE_1;                          /* 028C, 0x1502C28C */
	OWE_REG_WMFE_MASKI_BASE_ADDR_1                  OWE_WMFE_MASKI_BASE_ADDR_1;                      /* 0290, 0x1502C290 */
	OWE_REG_WMFE_MASKI_STRIDE_1                     OWE_WMFE_MASKI_STRIDE_1;                         /* 0294, 0x1502C294 */
	OWE_REG_WMFE_DPO_BASE_ADDR_1                    OWE_WMFE_DPO_BASE_ADDR_1;                        /* 0298, 0x1502C298 */
	OWE_REG_WMFE_DPO_STRIDE_1                       OWE_WMFE_DPO_STRIDE_1;                           /* 029C, 0x1502C29C */
	UINT32                                          rsv_02A0[4];                                     /* 02A0..02AF, 0x1502C2A0..1502C2AF */
	OWE_REG_WMFE_CTRL_2                             OWE_WMFE_CTRL_2;                                 /* 02B0, 0x1502C2B0 */
	OWE_REG_WMFE_SIZE_2                             OWE_WMFE_SIZE_2;                                 /* 02B4, 0x1502C2B4 */
	OWE_REG_WMFE_IMGI_BASE_ADDR_2                   OWE_WMFE_IMGI_BASE_ADDR_2;                       /* 02B8, 0x1502C2B8 */
	OWE_REG_WMFE_IMGI_STRIDE_2                      OWE_WMFE_IMGI_STRIDE_2;                          /* 02BC, 0x1502C2BC */
	OWE_REG_WMFE_DPI_BASE_ADDR_2                    OWE_WMFE_DPI_BASE_ADDR_2;                        /* 02C0, 0x1502C2C0 */
	OWE_REG_WMFE_DPI_STRIDE_2                       OWE_WMFE_DPI_STRIDE_2;                           /* 02C4, 0x1502C2C4 */
	OWE_REG_WMFE_TBLI_BASE_ADDR_2                   OWE_WMFE_TBLI_BASE_ADDR_2;                       /* 02C8, 0x1502C2C8 */
	OWE_REG_WMFE_TBLI_STRIDE_2                      OWE_WMFE_TBLI_STRIDE_2;                          /* 02CC, 0x1502C2CC */
	OWE_REG_WMFE_MASKI_BASE_ADDR_2                  OWE_WMFE_MASKI_BASE_ADDR_2;                      /* 02D0, 0x1502C2D0 */
	OWE_REG_WMFE_MASKI_STRIDE_2                     OWE_WMFE_MASKI_STRIDE_2;                         /* 02D4, 0x1502C2D4 */
	OWE_REG_WMFE_DPO_BASE_ADDR_2                    OWE_WMFE_DPO_BASE_ADDR_2;                        /* 02D8, 0x1502C2D8 */
	OWE_REG_WMFE_DPO_STRIDE_2                       OWE_WMFE_DPO_STRIDE_2;                           /* 02DC, 0x1502C2DC */
	UINT32                                          rsv_02E0[4];                                     /* 02E0..02EF, 0x1502C2E0..1502C2EF */
	OWE_REG_WMFE_CTRL_3                             OWE_WMFE_CTRL_3;                                 /* 02F0, 0x1502C2F0 */
	OWE_REG_WMFE_SIZE_3                             OWE_WMFE_SIZE_3;                                 /* 02F4, 0x1502C2F4 */
	OWE_REG_WMFE_IMGI_BASE_ADDR_3                   OWE_WMFE_IMGI_BASE_ADDR_3;                       /* 02F8, 0x1502C2F8 */
	OWE_REG_WMFE_IMGI_STRIDE_3                      OWE_WMFE_IMGI_STRIDE_3;                          /* 02FC, 0x1502C2FC */
	OWE_REG_WMFE_DPI_BASE_ADDR_3                    OWE_WMFE_DPI_BASE_ADDR_3;                        /* 0300, 0x1502C300 */
	OWE_REG_WMFE_DPI_STRIDE_3                       OWE_WMFE_DPI_STRIDE_3;                           /* 0304, 0x1502C304 */
	OWE_REG_WMFE_TBLI_BASE_ADDR_3                   OWE_WMFE_TBLI_BASE_ADDR_3;                       /* 0308, 0x1502C308 */
	OWE_REG_WMFE_TBLI_STRIDE_3                      OWE_WMFE_TBLI_STRIDE_3;                          /* 030C, 0x1502C30C */
	OWE_REG_WMFE_MASKI_BASE_ADDR_3                  OWE_WMFE_MASKI_BASE_ADDR_3;                      /* 0310, 0x1502C310 */
	OWE_REG_WMFE_MASKI_STRIDE_3                     OWE_WMFE_MASKI_STRIDE_3;                         /* 0314, 0x1502C314 */
	OWE_REG_WMFE_DPO_BASE_ADDR_3                    OWE_WMFE_DPO_BASE_ADDR_3;                        /* 0318, 0x1502C318 */
	OWE_REG_WMFE_DPO_STRIDE_3                       OWE_WMFE_DPO_STRIDE_3;                           /* 031C, 0x1502C31C */
	UINT32                                          rsv_0320[4];                                     /* 0320..032F, 0x1502C320..1502C32F */
	OWE_REG_WMFE_CTRL_4                             OWE_WMFE_CTRL_4;                                 /* 0330, 0x1502C330 */
	OWE_REG_WMFE_SIZE_4                             OWE_WMFE_SIZE_4;                                 /* 0334, 0x1502C334 */
	OWE_REG_WMFE_IMGI_BASE_ADDR_4                   OWE_WMFE_IMGI_BASE_ADDR_4;                       /* 0338, 0x1502C338 */
	OWE_REG_WMFE_IMGI_STRIDE_4                      OWE_WMFE_IMGI_STRIDE_4;                          /* 033C, 0x1502C33C */
	OWE_REG_WMFE_DPI_BASE_ADDR_4                    OWE_WMFE_DPI_BASE_ADDR_4;                        /* 0340, 0x1502C340 */
	OWE_REG_WMFE_DPI_STRIDE_4                       OWE_WMFE_DPI_STRIDE_4;                           /* 0344, 0x1502C344 */
	OWE_REG_WMFE_TBLI_BASE_ADDR_4                   OWE_WMFE_TBLI_BASE_ADDR_4;                       /* 0348, 0x1502C348 */
	OWE_REG_WMFE_TBLI_STRIDE_4                      OWE_WMFE_TBLI_STRIDE_4;                          /* 034C, 0x1502C34C */
	OWE_REG_WMFE_MASKI_BASE_ADDR_4                  OWE_WMFE_MASKI_BASE_ADDR_4;                      /* 0350, 0x1502C350 */
	OWE_REG_WMFE_MASKI_STRIDE_4                     OWE_WMFE_MASKI_STRIDE_4;                         /* 0354, 0x1502C354 */
	OWE_REG_WMFE_DPO_BASE_ADDR_4                    OWE_WMFE_DPO_BASE_ADDR_4;                        /* 0358, 0x1502C358 */
	OWE_REG_WMFE_DPO_STRIDE_4                       OWE_WMFE_DPO_STRIDE_4;                           /* 035C, 0x1502C35C */
	UINT32                                          rsv_0360[40];                                    /* 0360..03FF, 0x1502C360..1502C3FF */
	OWE_REG_WMFE_DBG_INFO_00                        OWE_WMFE_DBG_INFO_00;                            /* 0400, 0x1502C400 */
	OWE_REG_WMFE_DBG_INFO_01                        OWE_WMFE_DBG_INFO_01;                            /* 0404, 0x1502C404 */
	OWE_REG_WMFE_DBG_INFO_02                        OWE_WMFE_DBG_INFO_02;                            /* 0408, 0x1502C408 */
	OWE_REG_WMFE_DBG_INFO_03                        OWE_WMFE_DBG_INFO_03;                            /* 040C, 0x1502C40C */
	OWE_REG_WMFE_DBG_INFO_04                        OWE_WMFE_DBG_INFO_04;                            /* 0410, 0x1502C410 */
	OWE_REG_WMFE_DBG_INFO_05                        OWE_WMFE_DBG_INFO_05;                            /* 0414, 0x1502C414 */
	OWE_REG_WMFE_DBG_INFO_06                        OWE_WMFE_DBG_INFO_06;                            /* 0418, 0x1502C418 */
	OWE_REG_WMFE_DBG_INFO_07                        OWE_WMFE_DBG_INFO_07;                            /* 041C, 0x1502C41C */
	OWE_REG_WMFE_DBG_INFO_08                        OWE_WMFE_DBG_INFO_08;                            /* 0420, 0x1502C420 */
	OWE_REG_WMFE_DBG_INFO_09                        OWE_WMFE_DBG_INFO_09;                            /* 0424, 0x1502C424 */
	UINT32                                          rsv_0428[52];                                    /* 0428..04F7, 0x1502C428..1502C4F7 */
	OWE_REG_WMFE_SPARE_0                            OWE_WMFE_SPARE_0;                                /* 04F8, 0x1502C4F8 */
	OWE_REG_WMFE_SPARE_1                            OWE_WMFE_SPARE_1;                                /* 04FC, 0x1502C4FC */
	UINT32                                          rsv_0500[189];                                   /* 0500..07F3, 0x1502C500..1502C7F3 */
	OWE_REG_DMA_DBG                                 OWE_DMA_DBG;                                     /* 07F4, 0x1502C7F4 */
	OWE_REG_DMA_REQ_STATUS                          OWE_DMA_REQ_STATUS;                              /* 07F8, 0x1502C7F8 */
	OWE_REG_DMA_RDY_STATUS                          OWE_DMA_RDY_STATUS;                              /* 07FC, 0x1502C7FC */
	DMA_REG_SOFT_RSTSTAT                            DMA_SOFT_RSTSTAT;                                /* 0800, 0x1502C800 */
	VERTICAL_REG_FLIP_EN                            VERTICAL_FLIP_EN;                                /* 0804, 0x1502C804 */
	DMA_REG_SOFT_RESET                              DMA_SOFT_RESET;                                  /* 0808, 0x1502C808 */
	LAST_REG_ULTRA_EN                               LAST_ULTRA_EN;                                   /* 080C, 0x1502C80C */
	SPECIAL_REG_FUN_EN                              SPECIAL_FUN_EN;                                  /* 0810, 0x1502C810 */
	UINT32                                          rsv_0814[7];                                     /* 0814..082F, 0x1502C814..1502C82F */
	OWEO_REG_BASE_ADDR                              OWEO_BASE_ADDR;                                  /* 0830, 0x1502C830 */
	OWEO_REG_BASE_ADDR_2                            OWEO_BASE_ADDR_2;                                /* 0834, 0x1502C834 */
	OWEO_REG_OFST_ADDR                              OWEO_OFST_ADDR;                                  /* 0838, 0x1502C838 */
	OWEO_REG_OFST_ADDR_2                            OWEO_OFST_ADDR_2;                                /* 083C, 0x1502C83C */
	OWEO_REG_XSIZE                                  OWEO_XSIZE;                                      /* 0840, 0x1502C840 */
	OWEO_REG_YSIZE                                  OWEO_YSIZE;                                      /* 0844, 0x1502C844 */
	OWEO_REG_STRIDE                                 OWEO_STRIDE;                                     /* 0848, 0x1502C848 */
	OWEO_REG_CON                                    OWEO_CON;                                        /* 084C, 0x1502C84C */
	OWEO_REG_CON2                                   OWEO_CON2;                                       /* 0850, 0x1502C850 */
	OWEO_REG_CON3                                   OWEO_CON3;                                       /* 0854, 0x1502C854 */
	UINT32                                          rsv_0858[14];                                    /* 0858..088F, 0x1502C858..1502C88F */
	OWEI_REG_BASE_ADDR                              OWEI_BASE_ADDR;                                  /* 0890, 0x1502C890 */
	OWEI_REG_BASE_ADDR_2                            OWEI_BASE_ADDR_2;                                /* 0894, 0x1502C894 */
	OWEI_REG_OFST_ADDR                              OWEI_OFST_ADDR;                                  /* 0898, 0x1502C898 */
	OWEI_REG_OFST_ADDR_2                            OWEI_OFST_ADDR_2;                                /* 089C, 0x1502C89C */
	OWEI_REG_XSIZE                                  OWEI_XSIZE;                                      /* 08A0, 0x1502C8A0 */
	OWEI_REG_YSIZE                                  OWEI_YSIZE;                                      /* 08A4, 0x1502C8A4 */
	OWEI_REG_STRIDE                                 OWEI_STRIDE;                                     /* 08A8, 0x1502C8A8 */
	OWEI_REG_CON                                    OWEI_CON;                                        /* 08AC, 0x1502C8AC */
	OWEI_REG_CON2                                   OWEI_CON2;                                       /* 08B0, 0x1502C8B0 */
	OWEI_REG_CON3                                   OWEI_CON3;                                       /* 08B4, 0x1502C8B4 */
	UINT32                                          rsv_08B8[18];                                    /* 08B8..08FF, 0x1502C8B8..1502C8FF */
	DMA_REG_ERR_CTRL                                DMA_ERR_CTRL;                                    /* 0900, 0x1502C900 */
	OWEO_REG_ERR_STAT                               OWEO_ERR_STAT;                                   /* 0904, 0x1502C904 */
	OWEI_REG_ERR_STAT                               OWEI_ERR_STAT;                                   /* 0908, 0x1502C908 */
	DMA_REG_DEBUG_ADDR                              DMA_DEBUG_ADDR;                                  /* 090C, 0x1502C90C */
	DMA_REG_RSV1                                    DMA_RSV1;                                        /* 0910, 0x1502C910 */
	DMA_REG_RSV2                                    DMA_RSV2;                                        /* 0914, 0x1502C914 */
	DMA_REG_RSV3                                    DMA_RSV3;                                        /* 0918, 0x1502C918 */
	DMA_REG_RSV4                                    DMA_RSV4;                                        /* 091C, 0x1502C91C */
	DMA_REG_RSV5                                    DMA_RSV5;                                        /* 0920, 0x1502C920 */
	DMA_REG_RSV6                                    DMA_RSV6;                                        /* 0924, 0x1502C924 */
	DMA_REG_DEBUG_SEL                               DMA_DEBUG_SEL;                                   /* 0928, 0x1502C928 */
	DMA_REG_BW_SELF_TEST                            DMA_BW_SELF_TEST;                                /* 092C, 0x1502C92C */
	UINT32                                          rsv_0930[20];                                    /* 0930..097F, 1502C930..1502C97F */
}owe_reg_t;

/* auto insert ralf auto gen above */

#endif // _ISP_REG_H_
