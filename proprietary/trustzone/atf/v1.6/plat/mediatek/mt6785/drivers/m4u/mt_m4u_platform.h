/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MT_M4U_PLATFORM_H_
#define _MT_M4U_PLATFORM_H_

#define SMI_LARB_NR     6
#define SMI_LARB_SZ		(0x1000)

#define LARB0_BASE_PA	0x14017000
#define LARB1_BASE_PA	0x14018000
#define LARB2_BASE_PA	0x16010000
#define LARB3_BASE_PA	0x17010000
#define LARB5_BASE_PA	0x15021000
#define LARB6_BASE_PA	0x1a001000

#define sel_larb0_id      (0<<2)
#define sel_larb1_id      (1<<2)
#define sel_larb2_id      (2<<2)
#define sel_larb3_id      (3<<2)
#define sel_larb5_id      (4<<2)
#define sel_larb6_id      (7<<2)
#define sel_larb_com      (6<<2)

/* VPU_IOMMU AXI_ID */
#define IDMA_0   0x4
#define CORE_0   0x0
#define IDMA_1   0x5
#define CORE_1   0x1
/*
 * #define EDMA     0x5
 * #define EDMB     0x7
 * #define CORE_2   0x0
 * #define EDMC     0x2
 */

#define COM_IDMA_0   F_MSK(3, 0)
#define COM_CORE_0   F_MSK(3, 0)
#define COM_IDMA_1   F_MSK(3, 0)
#define COM_CORE_1   F_MSK(3, 0)
/*
 * #define COM_EDMA     (F_MSK(2, 0) | F_BIT_SET(9))
 * #define COM_EDMB     (F_MSK(2, 0) | F_BIT_SET(9))
 * #define COM_CORE_2   (F_MSK(1, 0) | F_BIT_SET(9))
 * #define COM_EDMC     (F_MSK(1, 0) | F_MSK(9, 8))
*/


enum {
	/*larb0 -MMSYS-9*/
	M4U_PORT_DISP_POSTMASK0,
	M4U_PORT_DISP_OVL0_HDR,
	M4U_PORT_DISP_OVL1_HDR,
	M4U_PORT_DISP_OVL0,
	M4U_PORT_DISP_OVL1,
	M4U_PORT_DISP_PVRIC0,
	M4U_PORT_DISP_RDMA0,
	M4U_PORT_DISP_WDMA0,
	M4U_PORT_DISP_FAKE0,

	/*larb1-MMSYS-14*/
	M4U_PORT_DISP_OVL0_2L_HDR,
	M4U_PORT_DISP_OVL1_2L_HDR,
	M4U_PORT_DISP_OVL0_2L,
	M4U_PORT_DISP_OVL1_2L,
	M4U_PORT_DISP_RDMA1,
	M4U_PORT_MDP_PVRIC0,
	M4U_PORT_MDP_PVRIC1,
	M4U_PORT_MDP_RDMA0,
	M4U_PORT_MDP_RDMA1,
	M4U_PORT_MDP_WROT0_R,
	M4U_PORT_MDP_WROT0_W,
	M4U_PORT_MDP_WROT1_R,
	M4U_PORT_MDP_WROT1_W,
	M4U_PORT_DISP_FAKE1,

	/*larb2-VDEC-12*/
	M4U_PORT_HW_VDEC_MC_EXT,
	M4U_PORT_HW_VDEC_UFO_EXT,
	M4U_PORT_HW_VDEC_PP_EXT,
	M4U_PORT_HW_VDEC_PRED_RD_EXT,
	M4U_PORT_HW_VDEC_PRED_WR_EXT,
	M4U_PORT_HW_VDEC_PPWRAP_EXT,
	M4U_PORT_HW_VDEC_TILE_EXT,
	M4U_PORT_HW_VDEC_VLD_EXT,
	M4U_PORT_HW_VDEC_VLD2_EXT,
	M4U_PORT_HW_VDEC_AVC_MV_EXT,
	M4U_PORT_HW_VDEC_UFO_ENC_EXT,
	M4U_PORT_HW_VDEC_RG_CTRL_DMA_EXT,

	/*larb3-VENC-19*/
	M4U_PORT_VENC_RCPU,
	M4U_PORT_VENC_REC,
	M4U_PORT_VENC_BSDMA,
	M4U_PORT_VENC_SV_COMV,
	M4U_PORT_VENC_RD_COMV,
	M4U_PORT_VENC_NBM_RDMA,
	M4U_PORT_VENC_NBM_RDMA_LITE,
	M4U_PORT_JPGENC_Y_RDMA,
	M4U_PORT_JPGENC_C_RDMA,
	M4U_PORT_JPGENC_Q_TABLE,
	M4U_PORT_JPGENC_BSDMA,
	M4U_PORT_JPGDEC_WDMA,
	M4U_PORT_JPGDEC_BSDMA,
	M4U_PORT_VENC_NBM_WDMA,
	M4U_PORT_VENC_NBM_WDMA_LITE,
	M4U_PORT_VENC_CUR_LUMA,
	M4U_PORT_VENC_CUR_CHROMA,
	M4U_PORT_VENC_REF_LUMA,
	M4U_PORT_VENC_REF_CHROMA,

	/* larb4-IMG-3
	 * HW disconnected
	 * Jolin Chou
	 * M4U_PORT_IPUO_LARB4,
	 * M4U_PORT_IPU3O_LARB4,
	 * M4U_PORT_IPUI_LARB4,
	 */

	/*larb5-CAM-26*/
	M4U_PORT_IMGI,
	M4U_PORT_IMG2O,
	M4U_PORT_IMG3O,
	M4U_PORT_VIPI,
	M4U_PORT_LCEI,
	M4U_PORT_SMXI,
	M4U_PORT_SMXO,
	M4U_PORT_WPE0_RDMA1,
	M4U_PORT_WPE0_RDMA0,
	M4U_PORT_WPE0_WDMA,
	M4U_PORT_FDVT_RDB,
	M4U_PORT_FDVT_WRA,
	M4U_PORT_FDVT_RDA,
	M4U_PORT_WPE1_RDMA0,
	M4U_PORT_WPE1_RDMA1,
	M4U_PORT_WPE1_WDMA,
	M4U_PORT_DPE_RDMA,
	M4U_PORT_DPE_WDMA,
	M4U_PORT_MFB_RDMA0,
	M4U_PORT_MFB_RDMA1,
	M4U_PORT_MFB_WDMA,
	M4U_PORT_RSC_RDMA0,
	M4U_PORT_RSC_WDMA,
	M4U_PORT_OWE_RDMA,
	M4U_PORT_OWE_WDMA,
	M4U_PORT_FDVT_WRB,

	/*larb6-CAM-31*/
	M4U_PORT_IMGO,
	M4U_PORT_RRZO,
	M4U_PORT_AAO,
	M4U_PORT_AFO,
	M4U_PORT_LSCI_0,
	M4U_PORT_LSCI_1,
	M4U_PORT_PDO,
	M4U_PORT_BPCI,
	M4U_PORT_LSCO,
	M4U_PORT_AFO_1,
	M4U_PORT_PSO,
	M4U_PORT_LSCI_2,
	M4U_PORT_SOCO,
	M4U_PORT_SOC1,
	M4U_PORT_SOC2,
	M4U_PORT_CCUI,
	M4U_PORT_CCUO,
	M4U_PORT_UFEO,
	M4U_PORT_RAWI_A,
	M4U_PORT_RSSO_A,
	M4U_PORT_CCUG,
	M4U_PORT_PDI,
	M4U_PORT_FLKO,
	M4U_PORT_LMVO,
	M4U_PORT_UFGO,
	M4U_PORT_SPARE,
	M4U_PORT_SPARE_2,
	M4U_PORT_SPARE_3,
	M4U_PORT_SPARE_4,
	M4U_PORT_SPARE_5,
	FAKE_ENGINE,

	/* larb7-CAM-5
	 * M4U_PORT_IPUO,
	 * M4U_PORT_IPU2O,
	 * M4U_PORT_IPU3O,
	 * M4U_PORT_IPUI,
	 * M4U_PORT_IPU2I,
	*/

	/* smi common */
	M4U_PORT_CCU0,
	M4U_PORT_CCU1,

	M4U_PORT_VPU,

	M4U_PORT_UNKNOWN,
};

static int m4u_get_port_id(int portID)
{
	portID &= F_MMU0_INT_ID_TF_MSK_SEC;

	/* smi common */
	if ((portID >> 7) == sel_larb_com)
		return M4U_PORT_CCU0;

	switch (portID) {
	/*larb0 -MMSYS-9*/
	case ((sel_larb0_id<<7)|(0<<2)): return M4U_PORT_DISP_POSTMASK0;
	case ((sel_larb0_id<<7)|(1<<2)): return M4U_PORT_DISP_OVL0_HDR;
	case ((sel_larb0_id<<7)|(2<<2)): return M4U_PORT_DISP_OVL1_HDR;
	case ((sel_larb0_id<<7)|(3<<2)): return M4U_PORT_DISP_OVL0;
	case ((sel_larb0_id<<7)|(4<<2)): return M4U_PORT_DISP_OVL1;
	case ((sel_larb0_id<<7)|(5<<2)): return M4U_PORT_DISP_PVRIC0;
	case ((sel_larb0_id<<7)|(6<<2)): return M4U_PORT_DISP_RDMA0;
	case ((sel_larb0_id<<7)|(7<<2)): return M4U_PORT_DISP_WDMA0;
	case ((sel_larb0_id<<7)|(8<<2)): return M4U_PORT_DISP_FAKE0;

	/*larb1-MMSYS-14*/
	case ((sel_larb1_id<<7)|(0<<2)):  return M4U_PORT_DISP_OVL0_2L_HDR;
	case ((sel_larb1_id<<7)|(1<<2)):  return M4U_PORT_DISP_OVL1_2L_HDR;
	case ((sel_larb1_id<<7)|(2<<2)):  return M4U_PORT_DISP_OVL0_2L;
	case ((sel_larb1_id<<7)|(3<<2)):  return M4U_PORT_DISP_OVL1_2L;
	case ((sel_larb1_id<<7)|(4<<2)):  return M4U_PORT_DISP_RDMA1;
	case ((sel_larb1_id<<7)|(5<<2)):  return M4U_PORT_MDP_PVRIC0;
	case ((sel_larb1_id<<7)|(6<<2)):  return M4U_PORT_MDP_PVRIC1;
	case ((sel_larb1_id<<7)|(7<<2)):  return M4U_PORT_MDP_RDMA0;
	case ((sel_larb1_id<<7)|(8<<2)):  return M4U_PORT_MDP_RDMA1;
	case ((sel_larb1_id<<7)|(9<<2)):  return M4U_PORT_MDP_WROT0_R;
	case ((sel_larb1_id<<7)|(10<<2)): return M4U_PORT_MDP_WROT0_W;
	case ((sel_larb1_id<<7)|(11<<2)): return M4U_PORT_MDP_WROT1_R;
	case ((sel_larb1_id<<7)|(12<<2)): return M4U_PORT_MDP_WROT1_W;
	case ((sel_larb1_id<<7)|(13<<2)): return M4U_PORT_DISP_FAKE1;

	/*larb2-VDEC-12*/
	case ((sel_larb2_id<<7)|(0<<2)):  return M4U_PORT_HW_VDEC_MC_EXT;
	case ((sel_larb2_id<<7)|(1<<2)):  return M4U_PORT_HW_VDEC_UFO_EXT;
	case ((sel_larb2_id<<7)|(2<<2)):  return M4U_PORT_HW_VDEC_PP_EXT;
	case ((sel_larb2_id<<7)|(3<<2)):  return M4U_PORT_HW_VDEC_PRED_RD_EXT;
	case ((sel_larb2_id<<7)|(4<<2)):  return M4U_PORT_HW_VDEC_PRED_WR_EXT;
	case ((sel_larb2_id<<7)|(5<<2)):  return M4U_PORT_HW_VDEC_PPWRAP_EXT;
	case ((sel_larb2_id<<7)|(6<<2)):  return M4U_PORT_HW_VDEC_TILE_EXT;
	case ((sel_larb2_id<<7)|(7<<2)):  return M4U_PORT_HW_VDEC_VLD_EXT;
	case ((sel_larb2_id<<7)|(8<<2)):  return M4U_PORT_HW_VDEC_VLD2_EXT;
	case ((sel_larb2_id<<7)|(9<<2)):  return M4U_PORT_HW_VDEC_AVC_MV_EXT;
	case ((sel_larb2_id<<7)|(10<<2)): return M4U_PORT_HW_VDEC_UFO_ENC_EXT;
	case ((sel_larb2_id<<7)|(11<<2)): return M4U_PORT_HW_VDEC_RG_CTRL_DMA_EXT;

	/*larb3-VENC-19*/
	case ((sel_larb3_id<<7)|(0<<2)):  return M4U_PORT_VENC_RCPU;
	case ((sel_larb3_id<<7)|(1<<2)):  return M4U_PORT_VENC_REC;
	case ((sel_larb3_id<<7)|(2<<2)):  return M4U_PORT_VENC_BSDMA;
	case ((sel_larb3_id<<7)|(3<<2)):  return M4U_PORT_VENC_SV_COMV;
	case ((sel_larb3_id<<7)|(4<<2)):  return M4U_PORT_VENC_RD_COMV;
	case ((sel_larb3_id<<7)|(5<<2)):  return M4U_PORT_VENC_NBM_RDMA;
	case ((sel_larb3_id<<7)|(6<<2)):  return M4U_PORT_VENC_NBM_RDMA_LITE;
	case ((sel_larb3_id<<7)|(7<<2)):  return M4U_PORT_JPGENC_Y_RDMA;
	case ((sel_larb3_id<<7)|(8<<2)):  return M4U_PORT_JPGENC_C_RDMA;
	case ((sel_larb3_id<<7)|(9<<2)):  return M4U_PORT_JPGENC_Q_TABLE;
	case ((sel_larb3_id<<7)|(10<<2)): return M4U_PORT_JPGENC_BSDMA;
	case ((sel_larb3_id<<7)|(11<<2)): return M4U_PORT_JPGDEC_WDMA;
	case ((sel_larb3_id<<7)|(12<<2)): return M4U_PORT_JPGDEC_BSDMA;
	case ((sel_larb3_id<<7)|(13<<2)): return M4U_PORT_VENC_NBM_WDMA;
	case ((sel_larb3_id<<7)|(14<<2)): return M4U_PORT_VENC_NBM_WDMA_LITE;
	case ((sel_larb3_id<<7)|(15<<2)): return M4U_PORT_VENC_CUR_LUMA;
	case ((sel_larb3_id<<7)|(16<<2)): return M4U_PORT_VENC_CUR_CHROMA;
	case ((sel_larb3_id<<7)|(17<<2)): return M4U_PORT_VENC_REF_LUMA;
	case ((sel_larb3_id<<7)|(18<<2)): return M4U_PORT_VENC_REF_CHROMA;

	/*larb5-IMG-26*/
	case ((sel_larb5_id<<7)|(0<<2)):  return M4U_PORT_IMGI;
	case ((sel_larb5_id<<7)|(1<<2)):  return M4U_PORT_IMG2O;
	case ((sel_larb5_id<<7)|(2<<2)):  return M4U_PORT_IMG3O;
	case ((sel_larb5_id<<7)|(3<<2)):  return M4U_PORT_VIPI;
	case ((sel_larb5_id<<7)|(4<<2)):  return M4U_PORT_LCEI;
	case ((sel_larb5_id<<7)|(5<<2)):  return M4U_PORT_SMXI;
	case ((sel_larb5_id<<7)|(6<<2)):  return M4U_PORT_SMXO;
	case ((sel_larb5_id<<7)|(7<<2)):  return M4U_PORT_WPE0_RDMA1;
	case ((sel_larb5_id<<7)|(8<<2)):  return M4U_PORT_WPE0_RDMA0;
	case ((sel_larb5_id<<7)|(9<<2)):  return M4U_PORT_WPE0_WDMA;
	case ((sel_larb5_id<<7)|(10<<2)): return M4U_PORT_FDVT_RDB;
	case ((sel_larb5_id<<7)|(11<<2)): return M4U_PORT_FDVT_WRA;
	case ((sel_larb5_id<<7)|(12<<2)): return M4U_PORT_FDVT_RDA;
	case ((sel_larb5_id<<7)|(13<<2)): return M4U_PORT_WPE1_RDMA0;
	case ((sel_larb5_id<<7)|(14<<2)): return M4U_PORT_WPE1_RDMA1;
	case ((sel_larb5_id<<7)|(15<<2)): return M4U_PORT_WPE1_WDMA;
	case ((sel_larb5_id<<7)|(16<<2)): return M4U_PORT_DPE_RDMA;
	case ((sel_larb5_id<<7)|(17<<2)): return M4U_PORT_DPE_WDMA;
	case ((sel_larb5_id<<7)|(18<<2)): return M4U_PORT_MFB_RDMA0;
	case ((sel_larb5_id<<7)|(19<<2)): return M4U_PORT_MFB_RDMA1;
	case ((sel_larb5_id<<7)|(20<<2)): return M4U_PORT_MFB_WDMA;
	case ((sel_larb5_id<<7)|(21<<2)): return M4U_PORT_RSC_RDMA0;
	case ((sel_larb5_id<<7)|(22<<2)): return M4U_PORT_RSC_WDMA;
	case ((sel_larb5_id<<7)|(23<<2)): return M4U_PORT_OWE_RDMA;
	case ((sel_larb5_id<<7)|(24<<2)): return M4U_PORT_OWE_WDMA;
	case ((sel_larb5_id<<7)|(25<<2)): return M4U_PORT_FDVT_WRB;

	default:
		 return M4U_PORT_UNKNOWN;
	}
}

static inline char *m4u_get_port_name(int portID)
{
	switch (portID) {
    /*larb0 -MMSYS-9*/
	case M4U_PORT_DISP_POSTMASK0:           return "DISP_POSTMASK0";
	case M4U_PORT_DISP_OVL0_HDR:            return "DISP_OVL0_HDR";
	case M4U_PORT_DISP_OVL1_HDR:            return "DISP_OVL1_HDR";
	case M4U_PORT_DISP_OVL0:                return "DISP_OVL0";
	case M4U_PORT_DISP_OVL1:                return "DISP_OVL1";
	case M4U_PORT_DISP_PVRIC0:              return "DISP_PVRIC0";
	case M4U_PORT_DISP_RDMA0:               return "DISP_RDMA0";
	case M4U_PORT_DISP_WDMA0:               return "DISP_WDMA0";
	case M4U_PORT_DISP_FAKE0:               return "DISP_FAKE0";

	/*larb1-MMSYS-14*/
	case M4U_PORT_DISP_OVL0_2L_HDR:         return "DISP_OVL0_2L_HDR";
	case M4U_PORT_DISP_OVL1_2L_HDR:         return "DISP_OVL1_2L_HDR";
	case M4U_PORT_DISP_OVL0_2L:             return "DISP_OVL0_2L";
	case M4U_PORT_DISP_OVL1_2L:             return "DISP_OVL1_2L";
	case M4U_PORT_DISP_RDMA1:               return "DISP_RDMA1";
	case M4U_PORT_MDP_PVRIC0:               return "MDP_PVRIC0";
	case M4U_PORT_MDP_PVRIC1:               return "MDP_PVRIC1";
	case M4U_PORT_MDP_RDMA0:                return "MDP_RDMA0";
	case M4U_PORT_MDP_RDMA1:                return "MDP_RDMA1";
	case M4U_PORT_MDP_WROT0_R:              return "MDP_WROT0_R";
	case M4U_PORT_MDP_WROT0_W:              return "MDP_WROT0_W";
	case M4U_PORT_MDP_WROT1_R:              return "MDP_WROT1_R";
	case M4U_PORT_MDP_WROT1_W:              return "MDP_WROT1_W";
	case M4U_PORT_DISP_FAKE1:               return "DISP_FAKE1";

	/*larb2-VDEC-12*/
	case M4U_PORT_HW_VDEC_MC_EXT:           return "HW_VDEC_MC_EXT";
	case M4U_PORT_HW_VDEC_UFO_EXT:          return "HW_VDEC_UFO_EXT";
	case M4U_PORT_HW_VDEC_PP_EXT:           return "HW_VDEC_PP_EXT";
	case M4U_PORT_HW_VDEC_PRED_RD_EXT:      return "HW_VDEC_PRED_RD_EXT";
	case M4U_PORT_HW_VDEC_PRED_WR_EXT:      return "HW_VDEC_PRED_WR_EXT";
	case M4U_PORT_HW_VDEC_PPWRAP_EXT:       return "HW_VDEC_PPWRAP_EXT";
	case M4U_PORT_HW_VDEC_TILE_EXT:         return "HW_VDEC_TILE_EXT";
	case M4U_PORT_HW_VDEC_VLD_EXT:          return "HW_VDEC_VLD_EXT";
	case M4U_PORT_HW_VDEC_VLD2_EXT:         return "HW_VDEC_VLD2_EXT";
	case M4U_PORT_HW_VDEC_AVC_MV_EXT:       return "HW_VDEC_AVC_MV_EXT";
	case M4U_PORT_HW_VDEC_UFO_ENC_EXT:      return "HW_VDEC_UFO_ENC_EXT";
	case M4U_PORT_HW_VDEC_RG_CTRL_DMA_EXT:  return "HW_VDEC_RG_CTRL_DMA_EXT";

	/*larb3-VENC-19*/
	case M4U_PORT_VENC_RCPU:                return "VENC_RCPU";
	case M4U_PORT_VENC_REC:                 return "VENC_REC";
	case M4U_PORT_VENC_BSDMA:               return "VENC_BSDMA";
	case M4U_PORT_VENC_SV_COMV:             return "VENC_SV_COMV";
	case M4U_PORT_VENC_RD_COMV:             return "VENC_RD_COMV";
	case M4U_PORT_VENC_NBM_RDMA:            return "VENC_NBM_RDMA";
	case M4U_PORT_VENC_NBM_RDMA_LITE:       return "VENC_NBM_RDMA_LITE";
	case M4U_PORT_JPGENC_Y_RDMA:            return "JPGENC_Y_RDMA";
	case M4U_PORT_JPGENC_C_RDMA:            return "JPGENC_C_RDMA";
	case M4U_PORT_JPGENC_Q_TABLE:           return "JPGENC_Q_TABLE";
	case M4U_PORT_JPGENC_BSDMA:             return "JPGENC_BSDMA";
	case M4U_PORT_JPGDEC_WDMA:              return "JPGDEC_WDMA";
	case M4U_PORT_JPGDEC_BSDMA:             return "JPGDEC_BSDMA";
	case M4U_PORT_VENC_NBM_WDMA:            return "VENC_NBM_WDMA";
	case M4U_PORT_VENC_NBM_WDMA_LITE:       return "VENC_NBM_WDMA_LITE";
	case M4U_PORT_VENC_CUR_LUMA:            return "VENC_CUR_LUMA";
	case M4U_PORT_VENC_CUR_CHROMA:          return "VENC_CUR_CHROMA";
	case M4U_PORT_VENC_REF_LUMA:            return "VENC_REF_LUMA";
	case M4U_PORT_VENC_REF_CHROMA:          return "VENC_REF_CHROMA";

	/*larb5-IMG-26*/
	case M4U_PORT_IMGI:                     return "IMGI";
	case M4U_PORT_IMG2O:                    return "IMG2O";
	case M4U_PORT_IMG3O:                    return "IMG3O";
	case M4U_PORT_VIPI:                     return "VIPI";
	case M4U_PORT_LCEI:                     return "LECI";
	case M4U_PORT_SMXI:                     return "SMX1";
	case M4U_PORT_SMXO:                     return "SMXO";
	case M4U_PORT_WPE0_RDMA1:               return "WPE0_RDMA1";
	case M4U_PORT_WPE0_RDMA0:               return "WPE0_RDMA0";
	case M4U_PORT_WPE0_WDMA:                return "WPE0_WDMA";
	case M4U_PORT_FDVT_RDB:                 return "FDVT_RDB";
	case M4U_PORT_FDVT_WRA:                 return "FDVT_WRA";
	case M4U_PORT_FDVT_RDA:                 return "FDVT_RDA";
	case M4U_PORT_WPE1_RDMA0:               return "WPE1_RDMA0";
	case M4U_PORT_WPE1_RDMA1:               return "WPE1_RDMA1";
	case M4U_PORT_WPE1_WDMA:                return "WPE1_WDMA";
	case M4U_PORT_DPE_RDMA:                 return "DPE_RDMA:";
	case M4U_PORT_DPE_WDMA:                 return "DPE_WDMA";
	case M4U_PORT_MFB_RDMA0:                return "MFB_RDMA0";
	case M4U_PORT_MFB_RDMA1:                return "MFB_RDMA1";
	case M4U_PORT_MFB_WDMA:                 return "MFB_WDMA";
	case M4U_PORT_RSC_RDMA0:                return "RSC_RDMA0";
	case M4U_PORT_RSC_WDMA:                 return "RSC_WDMA";
	case M4U_PORT_OWE_RDMA:                 return "OWE_RDMA";
	case M4U_PORT_OWE_WDMA:                 return "OWE_WDMA";
	case M4U_PORT_FDVT_WRB:                 return "FDVT_WRB";

	/*larb6-CAM-31*/
	case M4U_PORT_IMGO:                     return "IMGO";
	case M4U_PORT_RRZO:                     return "RRZO";
	case M4U_PORT_AAO:                      return "AAO";
	case M4U_PORT_AFO:                      return "AFO";
	case M4U_PORT_LSCI_0:                   return "LSCI_0";
	case M4U_PORT_LSCI_1:                   return "LSCI_1";
	case M4U_PORT_PDO:                      return "PDO";
	case M4U_PORT_BPCI:                     return "BPCI";
	case M4U_PORT_LSCO:                     return "LSCO";
	case M4U_PORT_AFO_1:                    return "AFO_1";
	case M4U_PORT_PSO:                      return "PSO";
	case M4U_PORT_LSCI_2:                   return "LSCI_2";
	case M4U_PORT_SOCO:                     return "SOC0";
	case M4U_PORT_SOC1:                     return "SOC1";
	case M4U_PORT_SOC2:                     return "SOC2";
	case M4U_PORT_CCUI:                     return "CCUI";
	case M4U_PORT_CCUO:                     return "CCUO";
	case M4U_PORT_UFEO:                     return "UFEO";
	case M4U_PORT_RAWI_A:                   return "RAWI_A";
	case M4U_PORT_RSSO_A:                   return "RSSO_A";
	case M4U_PORT_CCUG:                     return "CCUG";
	case M4U_PORT_PDI:                      return "PDI";
	case M4U_PORT_FLKO:                     return "FLKO";
	case M4U_PORT_LMVO:                     return "LMVO";
	case M4U_PORT_UFGO:                     return "UFGO";
	case M4U_PORT_SPARE:                    return "SPARE";
	case M4U_PORT_SPARE_2:                  return "SPARE_2";
	case M4U_PORT_SPARE_3:                  return "SPARE_3";
	case M4U_PORT_SPARE_4:                  return "SPARE_4";
	case M4U_PORT_SPARE_5:                  return "SPARE_5";
	case FAKE_ENGINE:                       return "FAKE_ENGINE";


	case M4U_PORT_CCU0:                     return "CCU0";
	case M4U_PORT_CCU1:                     return "CCU1";

	case M4U_PORT_UNKNOWN:                  return "UNKNOWN";
	default:
		 return "INVALID";
	}

}

static inline char *m4u_get_vpu_port_name(int fault_id)
{
	fault_id &= F_MSK(9, 0);

	if ((fault_id & COM_IDMA_0) == IDMA_0)
		return "VPU_IDMA_0";
	else if ((fault_id & COM_CORE_0) == CORE_0)
		return "VPU_CORE_0";
	else if ((fault_id & COM_IDMA_1) == IDMA_1)
		return "VPU_IDMA_1";
	else if ((fault_id & COM_CORE_1) == CORE_1)
		return "VPU_CORE_1";
	else
		return "VPU_UNKNOWN";
}

#if 0
static uint64_t smiLarbBasePA[SMI_LARB_NR] = {
							LARB0_BASE_PA,
							LARB1_BASE_PA,
							LARB2_BASE_PA,
							LARB3_BASE_PA,
							LARB5_BASE_PA,
							LARB6_BASE_PA};

static unsigned int m4u_port0_in_larbx[SMI_LARB_NR] = {
							M4U_PORT_DISP_POSTMASK0,
							M4U_PORT_DISP_OVL0_2L_HDR,
							M4U_PORT_HW_VDEC_MC_EXT,
							M4U_PORT_VENC_RCPU,
							M4U_PORT_IMGI,
							M4U_PORT_IMGO};

static inline int m4u_port_2_larb_port(unsigned int port)
{
	int i;

	for (i = SMI_LARB_NR-1; i >= 0; i--) {
		if (port >= m4u_port0_in_larbx[i])
			return (port-m4u_port0_in_larbx[i]);
	}
	return 0;
}

static inline int m4u_port_2_larb_id(unsigned int port)
{
	int i;

	for (i = SMI_LARB_NR-1; i >= 0; i--) {
		if (port >= m4u_port0_in_larbx[i])
			return i;
	}
	return 0;
}
#endif

#endif

