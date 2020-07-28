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

#define SMI_LARB_NR     9
#define SMI_LARB_SZ		(0x1000)

#define LARB0_BASE_PA	0x14017000
#define LARB1_BASE_PA	0x14018000
#define LARB2_BASE_PA	0x16010000
#define LARB3_BASE_PA	0x17010000
#define LARB5_BASE_PA	0x1502e000
#define LARB7_BASE_PA	0x1b10f000
#define LARB8_BASE_PA	0x1b00f000
#define LARB9_BASE_PA	0x1a001000
#define LARB10_BASE_PA	0x1a002000

#define sel_larb0_id      0
#define sel_larb1_id      4
#define sel_larb2_id      8
#define sel_larb3_id      12
#define sel_larb5_id      16
#define sel_larb7_id      20
#define sel_larb8_id      21
#define sel_larb9_id      28
#define sel_larb10_id     25
#define sel_larb_com      24

/* VPU_IOMMU AXI_ID */
#define IDMA_0   0x9
#define CORE_0   0x1
#define EDMA     0x5
#define IDMA_1   0xB
#define CORE_1   0x3
#define EDMB     0x7
#define CORE_2   0x0
#define EDMC     0x2

#define COM_IDMA_0   F_MSK(3, 0)
#define COM_CORE_0   (F_MSK(3, 0) | F_MSK(9, 8))
#define COM_EDMA     (F_MSK(2, 0) | F_BIT_SET(9))
#define COM_IDMA_1   F_MSK(3, 0)
#define COM_CORE_1   (F_MSK(3, 0) | F_MSK(9, 8))
#define COM_EDMB     (F_MSK(2, 0) | F_BIT_SET(9))
#define COM_CORE_2   (F_MSK(1, 0) | F_BIT_SET(9))
#define COM_EDMC     (F_MSK(1, 0) | F_MSK(9, 8))


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
	M4U_PORT_JPGEDC_WDMA,
	M4U_PORT_JPGEDC_BSDMA,
	M4U_PORT_VENC_NBM_WDMA,
	M4U_PORT_VENC_NBM_WDMA_LITE,
	M4U_PORT_VENC_CUR_LUMA,
	M4U_PORT_VENC_CUR_CHROMA,
	M4U_PORT_VENC_REF_LUMA,
	M4U_PORT_VENC_REF_CHROMA,

	/*larb4-dummy*/

	/*larb5-IMG-26*/
	M4U_PORT_IMGI_D1,
	M4U_PORT_IMGBI_D1,
	M4U_PORT_DMGI_D1,
	M4U_PORT_DEPI_D1,
	M4U_PORT_LCEI_D1,
	M4U_PORT_SMTI_D1,
	M4U_PORT_SMTO_D2,
	M4U_PORT_SMTO_D1,
	M4U_PORT_CRZO_D1,
	M4U_PORT_IMG3O_D1,

	M4U_PORT_VIPI_D1,
	M4U_PORT_WPE_RDMA1,
	M4U_PORT_WPE_RDMA0,
	M4U_PORT_WPE_WDMA,
	M4U_PORT_TIMGO_D1,
	M4U_PORT_MFB_RDMA0,
	M4U_PORT_MFB_RDMA1,
	M4U_PORT_MFB_RDMA2,
	M4U_PORT_MFB_RDMA3,
	M4U_PORT_MFB_WDMA,

	M4U_PORT_RESERVE1,
	M4U_PORT_RESERVE2,
	M4U_PORT_RESERVE3,
	M4U_PORT_RESERVE4,
	M4U_PORT_RESERVE5,
	M4U_PORT_RESERVE6,

	/*larb7-IPESYS-4*/
	M4U_PORT_DVS_RDMA,
	M4U_PORT_DVS_WDMA,
	M4U_PORT_DVP_RDMA,
	M4U_PORT_DVP_WDMA,

	/*larb8-IPESYS-10*/
	M4U_PORT_FDVT_RDA,
	M4U_PORT_FDVT_RDB,
	M4U_PORT_FDVT_WRA,
	M4U_PORT_FDVT_WRB,
	M4U_PORT_FE_RD0,
	M4U_PORT_FE_RD1,
	M4U_PORT_FE_WR0,
	M4U_PORT_FE_WR1,
	M4U_PORT_RSC_RDMA0,
	M4U_PORT_RSC_WDMA,

	/*larb9-CAM-24*/
	M4U_PORT_CAM_IMGO_C,
	M4U_PORT_CAM_RRZO_C,
	M4U_PORT_CAM_LSCI_C,
	M4U_PORT_CAM_BPCI_C,
	M4U_PORT_CAM_YUVO_C,
	M4U_PORT_CAM_UFDI_R2_C,
	M4U_PORT_CAM_RAWI_R2_C,
	M4U_PORT_CAM_RAWI_R5_C,
	M4U_PORT_CAM_CAMSV_1,
	M4U_PORT_CAM_CAMSV_2,

	M4U_PORT_CAM_CAMSV_3,
	M4U_PORT_CAM_CAMSV_4,
	M4U_PORT_CAM_CAMSV_5,
	M4U_PORT_CAM_CAMSV_6,
	M4U_PORT_CAM_AAO_C,
	M4U_PORT_CAM_AFO_C,
	M4U_PORT_CAM_FLKO_C,
	M4U_PORT_CAM_LCESO_C,
	M4U_PORT_CAM_CRZO_C,
	M4U_PORT_CAM_LTMSO_C,

	M4U_PORT_CAM_RSSO_C,
	M4U_PORT_CAM_CCUI,
	M4U_PORT_CAM_CCUO,
	M4U_PORT_CAM_FAKE,

	/*larb10-CAM-31*/
	M4U_PORT_CAM_IMGO_A,
	M4U_PORT_CAM_RRZO_A,
	M4U_PORT_CAM_LSCI_A,
	M4U_PORT_CAM_BPCI_A,
	M4U_PORT_CAM_YUVO_A,
	M4U_PORT_CAM_UFDI_A,
	M4U_PORT_CAM_RAWI_R2_A,
	M4U_PORT_CAM_RAWI_R5_A,
	M4U_PORT_CAM_IMGO_B,
	M4U_PORT_CAM_RRZO_B,

	M4U_PORT_CAM_LSCI_B,
	M4U_PORT_CAM_BPCI_B,
	M4U_PORT_CAM_YUVO_B,
	M4U_PORT_CAM_UFDI_B,
	M4U_PORT_CAM_RAWI_R2_B,
	M4U_PORT_CAM_RAWI_R5_B,
	M4U_PORT_CAM_CAMSV_0,
	M4U_PORT_CAM_AAO_A,
	M4U_PORT_CAM_AFO_A,
	M4U_PORT_CAM_FLKO_A,

	M4U_PORT_CAM_LCESO_A,
	M4U_PORT_CAM_CRZO_A,
	M4U_PORT_CAM_AAO_B,
	M4U_PORT_CAM_AFO_B,
	M4U_PORT_CAM_FLKO_B,
	M4U_PORT_CAM_LCESO_B,
	M4U_PORT_CAM_CRZO_B,
	M4U_PORT_CAM_LTMSO_A,
	M4U_PORT_CAM_RSSO_A,
	M4U_PORT_CAM_LTMSO_B,
	M4U_PORT_CAM_RSSO_B,

	M4U_PORT_CCU0,

	M4U_PORT_UNKNOWN

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
	case ((sel_larb1_id<<7)|(0<<2)): return M4U_PORT_DISP_OVL0_2L_HDR;
	case ((sel_larb1_id<<7)|(1<<2)): return M4U_PORT_DISP_OVL1_2L_HDR;
	case ((sel_larb1_id<<7)|(2<<2)): return M4U_PORT_DISP_OVL0_2L;
	case ((sel_larb1_id<<7)|(3<<2)): return M4U_PORT_DISP_OVL1_2L;
	case ((sel_larb1_id<<7)|(4<<2)): return M4U_PORT_DISP_RDMA1;
	case ((sel_larb1_id<<7)|(5<<2)): return M4U_PORT_MDP_PVRIC0;
	case ((sel_larb1_id<<7)|(6<<2)): return M4U_PORT_MDP_PVRIC1;
	case ((sel_larb1_id<<7)|(7<<2)): return M4U_PORT_MDP_RDMA0;
	case ((sel_larb1_id<<7)|(8<<2)): return M4U_PORT_MDP_RDMA1;
	case ((sel_larb1_id<<7)|(9<<2)): return M4U_PORT_MDP_WROT0_R;
	case ((sel_larb1_id<<7)|(10<<2)): return M4U_PORT_MDP_WROT0_W;
	case ((sel_larb1_id<<7)|(11<<2)): return M4U_PORT_MDP_WROT1_R;
	case ((sel_larb1_id<<7)|(12<<2)): return M4U_PORT_MDP_WROT1_W;
	case ((sel_larb1_id<<7)|(13<<2)): return M4U_PORT_DISP_FAKE1;

	/*larb2-VDEC-12*/
	case ((sel_larb2_id<<7)|(0<<2)): return M4U_PORT_HW_VDEC_MC_EXT;
	case ((sel_larb2_id<<7)|(1<<2)): return M4U_PORT_HW_VDEC_UFO_EXT;
	case ((sel_larb2_id<<7)|(2<<2)): return M4U_PORT_HW_VDEC_PP_EXT;
	case ((sel_larb2_id<<7)|(3<<2)): return M4U_PORT_HW_VDEC_PRED_RD_EXT;
	case ((sel_larb2_id<<7)|(4<<2)): return M4U_PORT_HW_VDEC_PRED_WR_EXT;
	case ((sel_larb2_id<<7)|(5<<2)): return M4U_PORT_HW_VDEC_PPWRAP_EXT;
	case ((sel_larb2_id<<7)|(6<<2)): return M4U_PORT_HW_VDEC_TILE_EXT;
	case ((sel_larb2_id<<7)|(7<<2)): return M4U_PORT_HW_VDEC_VLD_EXT;
	case ((sel_larb2_id<<7)|(8<<2)): return M4U_PORT_HW_VDEC_VLD2_EXT;
	case ((sel_larb2_id<<7)|(9<<2)): return M4U_PORT_HW_VDEC_AVC_MV_EXT;
	case ((sel_larb2_id<<7)|(10<<2)): return M4U_PORT_HW_VDEC_UFO_ENC_EXT;
	case ((sel_larb2_id<<7)|(11<<2)): return M4U_PORT_HW_VDEC_RG_CTRL_DMA_EXT;

	/*larb3-VENC-19*/
	case ((sel_larb3_id<<7)|(0<<2)): return M4U_PORT_VENC_RCPU;
	case ((sel_larb3_id<<7)|(1<<2)): return M4U_PORT_VENC_REC;
	case ((sel_larb3_id<<7)|(2<<2)): return M4U_PORT_VENC_BSDMA;
	case ((sel_larb3_id<<7)|(3<<2)): return M4U_PORT_VENC_SV_COMV;
	case ((sel_larb3_id<<7)|(4<<2)): return M4U_PORT_VENC_RD_COMV;
	case ((sel_larb3_id<<7)|(5<<2)): return M4U_PORT_VENC_NBM_RDMA;
	case ((sel_larb3_id<<7)|(6<<2)): return M4U_PORT_VENC_NBM_RDMA_LITE;
	case ((sel_larb3_id<<7)|(7<<2)): return M4U_PORT_JPGENC_Y_RDMA;
	case ((sel_larb3_id<<7)|(8<<2)): return M4U_PORT_JPGENC_C_RDMA;
	case ((sel_larb3_id<<7)|(9<<2)): return M4U_PORT_JPGENC_Q_TABLE;
	case ((sel_larb3_id<<7)|(10<<2)): return M4U_PORT_JPGENC_BSDMA;
	case ((sel_larb3_id<<7)|(11<<2)): return M4U_PORT_JPGEDC_WDMA;
	case ((sel_larb3_id<<7)|(12<<2)): return M4U_PORT_JPGEDC_BSDMA;
	case ((sel_larb3_id<<7)|(13<<2)): return M4U_PORT_VENC_NBM_WDMA;
	case ((sel_larb3_id<<7)|(14<<2)): return M4U_PORT_VENC_NBM_WDMA_LITE;
	case ((sel_larb3_id<<7)|(15<<2)): return M4U_PORT_VENC_CUR_LUMA;
	case ((sel_larb3_id<<7)|(16<<2)): return M4U_PORT_VENC_CUR_CHROMA;
	case ((sel_larb3_id<<7)|(17<<2)): return M4U_PORT_VENC_REF_LUMA;
	case ((sel_larb3_id<<7)|(18<<2)): return M4U_PORT_VENC_REF_CHROMA;

	/*larb5-IMG-26*/
	case ((sel_larb5_id<<7)|(0<<2)): return M4U_PORT_IMGI_D1;
	case ((sel_larb5_id<<7)|(1<<2)): return M4U_PORT_IMGBI_D1;
	case ((sel_larb5_id<<7)|(2<<2)): return M4U_PORT_DMGI_D1;
	case ((sel_larb5_id<<7)|(3<<2)): return M4U_PORT_DEPI_D1;
	case ((sel_larb5_id<<7)|(4<<2)): return M4U_PORT_LCEI_D1;
	case ((sel_larb5_id<<7)|(5<<2)): return M4U_PORT_SMTI_D1;
	case ((sel_larb5_id<<7)|(6<<2)): return M4U_PORT_SMTO_D2;
	case ((sel_larb5_id<<7)|(7<<2)): return M4U_PORT_SMTO_D1;
	case ((sel_larb5_id<<7)|(8<<2)): return M4U_PORT_CRZO_D1;
	case ((sel_larb5_id<<7)|(9<<2)): return M4U_PORT_IMG3O_D1;

	case ((sel_larb5_id<<7)|(10<<2)): return M4U_PORT_VIPI_D1;
	case ((sel_larb5_id<<7)|(11<<2)): return M4U_PORT_WPE_RDMA1;
	case ((sel_larb5_id<<7)|(12<<2)): return M4U_PORT_WPE_RDMA0;
	case ((sel_larb5_id<<7)|(13<<2)): return M4U_PORT_WPE_WDMA;
	case ((sel_larb5_id<<7)|(14<<2)): return M4U_PORT_TIMGO_D1;
	case ((sel_larb5_id<<7)|(15<<2)): return M4U_PORT_MFB_RDMA0;
	case ((sel_larb5_id<<7)|(16<<2)): return M4U_PORT_MFB_RDMA1;
	case ((sel_larb5_id<<7)|(17<<2)): return M4U_PORT_MFB_RDMA2;
	case ((sel_larb5_id<<7)|(18<<2)): return M4U_PORT_MFB_RDMA3;
	case ((sel_larb5_id<<7)|(19<<2)): return M4U_PORT_MFB_WDMA;

	case ((sel_larb5_id<<7)|(20<<2)): return M4U_PORT_RESERVE1;
	case ((sel_larb5_id<<7)|(21<<2)): return M4U_PORT_RESERVE2;
	case ((sel_larb5_id<<7)|(22<<2)): return M4U_PORT_RESERVE3;
	case ((sel_larb5_id<<7)|(23<<2)): return M4U_PORT_RESERVE4;
	case ((sel_larb5_id<<7)|(24<<2)): return M4U_PORT_RESERVE5;
	case ((sel_larb5_id<<7)|(25<<2)): return M4U_PORT_RESERVE6;

	/*larb7-IPESYS-4*/
	case ((sel_larb7_id<<7)|(0<<2)): return M4U_PORT_DVS_RDMA;
	case ((sel_larb7_id<<7)|(1<<2)): return M4U_PORT_DVS_WDMA;
	case ((sel_larb7_id<<7)|(2<<2)): return M4U_PORT_DVP_RDMA;
	case ((sel_larb7_id<<7)|(3<<2)): return M4U_PORT_DVP_WDMA;

	/*larb8-IPESYS-10*/
	case ((sel_larb8_id<<7)|(0<<2)): return M4U_PORT_FDVT_RDA;
	case ((sel_larb8_id<<7)|(1<<2)): return M4U_PORT_FDVT_RDB;
	case ((sel_larb8_id<<7)|(2<<2)): return M4U_PORT_FDVT_WRA;
	case ((sel_larb8_id<<7)|(3<<2)): return M4U_PORT_FDVT_WRB;
	case ((sel_larb8_id<<7)|(4<<2)): return M4U_PORT_FE_RD0;
	case ((sel_larb8_id<<7)|(5<<2)): return M4U_PORT_FE_RD1;
	case ((sel_larb8_id<<7)|(6<<2)): return M4U_PORT_FE_WR0;
	case ((sel_larb8_id<<7)|(7<<2)): return M4U_PORT_FE_WR1;
	case ((sel_larb8_id<<7)|(8<<2)): return M4U_PORT_RSC_RDMA0;
	case ((sel_larb8_id<<7)|(9<<2)): return M4U_PORT_RSC_WDMA;

	/*larb9-CAM-24*/
	case ((sel_larb9_id<<7)|(0<<2)): return M4U_PORT_CAM_IMGO_C;
	case ((sel_larb9_id<<7)|(1<<2)): return M4U_PORT_CAM_RRZO_C;
	case ((sel_larb9_id<<7)|(2<<2)): return M4U_PORT_CAM_LSCI_C;
	case ((sel_larb9_id<<7)|(3<<2)): return M4U_PORT_CAM_BPCI_C;
	case ((sel_larb9_id<<7)|(4<<2)): return M4U_PORT_CAM_YUVO_C;
	case ((sel_larb9_id<<7)|(5<<2)): return M4U_PORT_CAM_UFDI_R2_C;
	case ((sel_larb9_id<<7)|(6<<2)): return M4U_PORT_CAM_RAWI_R2_C;
	case ((sel_larb9_id<<7)|(7<<2)): return M4U_PORT_CAM_RAWI_R5_C;
	case ((sel_larb9_id<<7)|(8<<2)): return M4U_PORT_CAM_CAMSV_1;
	case ((sel_larb9_id<<7)|(9<<2)): return M4U_PORT_CAM_CAMSV_2;

	case ((sel_larb9_id<<7)|(10<<2)): return M4U_PORT_CAM_CAMSV_3;
	case ((sel_larb9_id<<7)|(11<<2)): return M4U_PORT_CAM_CAMSV_4;
	case ((sel_larb9_id<<7)|(12<<2)): return M4U_PORT_CAM_CAMSV_5;
	case ((sel_larb9_id<<7)|(13<<2)): return M4U_PORT_CAM_CAMSV_6;
	case ((sel_larb9_id<<7)|(14<<2)): return M4U_PORT_CAM_AAO_C;
	case ((sel_larb9_id<<7)|(15<<2)): return M4U_PORT_CAM_AFO_C;
	case ((sel_larb9_id<<7)|(16<<2)): return M4U_PORT_CAM_FLKO_C;
	case ((sel_larb9_id<<7)|(17<<2)): return M4U_PORT_CAM_LCESO_C;
	case ((sel_larb9_id<<7)|(18<<2)): return M4U_PORT_CAM_CRZO_C;
	case ((sel_larb9_id<<7)|(19<<2)): return M4U_PORT_CAM_LTMSO_C;

	case ((sel_larb9_id<<7)|(20<<2)): return M4U_PORT_CAM_RSSO_C;
	case ((sel_larb9_id<<7)|(21<<2)): return M4U_PORT_CAM_CCUI;
	case ((sel_larb9_id<<7)|(22<<2)): return M4U_PORT_CAM_CCUO;
	case ((sel_larb9_id<<7)|(23<<2)): return M4U_PORT_CAM_FAKE;

	/*larb10-CAM-31*/
	case ((sel_larb10_id<<7)|(0<<2)): return M4U_PORT_CAM_IMGO_A;
	case ((sel_larb10_id<<7)|(1<<2)): return M4U_PORT_CAM_RRZO_A;
	case ((sel_larb10_id<<7)|(2<<2)): return M4U_PORT_CAM_LSCI_A;
	case ((sel_larb10_id<<7)|(3<<2)): return M4U_PORT_CAM_BPCI_A;
	case ((sel_larb10_id<<7)|(4<<2)): return M4U_PORT_CAM_YUVO_A;
	case ((sel_larb10_id<<7)|(5<<2)): return M4U_PORT_CAM_UFDI_A;
	case ((sel_larb10_id<<7)|(6<<2)): return M4U_PORT_CAM_RAWI_R2_A;
	case ((sel_larb10_id<<7)|(7<<2)): return M4U_PORT_CAM_RAWI_R5_A;
	case ((sel_larb10_id<<7)|(8<<2)): return M4U_PORT_CAM_IMGO_B;
	case ((sel_larb10_id<<7)|(9<<2)): return M4U_PORT_CAM_RRZO_B;

	case ((sel_larb10_id<<7)|(10<<2)): return M4U_PORT_CAM_LSCI_B;
	case ((sel_larb10_id<<7)|(11<<2)): return M4U_PORT_CAM_BPCI_B;
	case ((sel_larb10_id<<7)|(12<<2)): return M4U_PORT_CAM_YUVO_B;
	case ((sel_larb10_id<<7)|(13<<2)): return M4U_PORT_CAM_UFDI_B;
	case ((sel_larb10_id<<7)|(14<<2)): return M4U_PORT_CAM_RAWI_R2_B;
	case ((sel_larb10_id<<7)|(15<<2)): return M4U_PORT_CAM_RAWI_R5_B;
	case ((sel_larb10_id<<7)|(16<<2)): return M4U_PORT_CAM_CAMSV_0;
	case ((sel_larb10_id<<7)|(17<<2)): return M4U_PORT_CAM_AAO_A;
	case ((sel_larb10_id<<7)|(18<<2)): return M4U_PORT_CAM_AFO_A;
	case ((sel_larb10_id<<7)|(19<<2)): return M4U_PORT_CAM_FLKO_A;

	case ((sel_larb10_id<<7)|(20<<2)): return M4U_PORT_CAM_LCESO_A;
	case ((sel_larb10_id<<7)|(21<<2)): return M4U_PORT_CAM_CRZO_A;
	case ((sel_larb10_id<<7)|(22<<2)): return M4U_PORT_CAM_AAO_B;
	case ((sel_larb10_id<<7)|(23<<2)): return M4U_PORT_CAM_AFO_B;
	case ((sel_larb10_id<<7)|(24<<2)): return M4U_PORT_CAM_FLKO_B;
	case ((sel_larb10_id<<7)|(25<<2)): return M4U_PORT_CAM_LCESO_B;
	case ((sel_larb10_id<<7)|(26<<2)): return M4U_PORT_CAM_CRZO_B;
	case ((sel_larb10_id<<7)|(27<<2)): return M4U_PORT_CAM_LTMSO_A;
	case ((sel_larb10_id<<7)|(28<<2)): return M4U_PORT_CAM_RSSO_A;
	case ((sel_larb10_id<<7)|(29<<2)): return M4U_PORT_CAM_LTMSO_B;
	case ((sel_larb10_id<<7)|(30<<2)): return M4U_PORT_CAM_RSSO_B;

	default:
		 return M4U_PORT_UNKNOWN;
	}
}

static inline char *m4u_get_port_name(int portID)
{
	switch (portID) {
    /*larb0 -MMSYS-9*/
	case M4U_PORT_DISP_POSTMASK0:  return "DISP_POSTMASK0";
	case M4U_PORT_DISP_OVL0_HDR:  return "DISP_OVL0_HDR";
	case M4U_PORT_DISP_OVL1_HDR:  return "DISP_OVL1_HDR";
	case M4U_PORT_DISP_OVL0:  return "DISP_OVL0";
	case M4U_PORT_DISP_OVL1:  return "DISP_OVL1";
	case M4U_PORT_DISP_PVRIC0:  return "DISP_PVRIC0";
	case M4U_PORT_DISP_RDMA0:  return "DISP_RDMA0";
	case M4U_PORT_DISP_WDMA0:  return "DISP_WDMA0";
	case M4U_PORT_DISP_FAKE0:  return "DISP_FAKE0";

	/*larb1-MMSYS-14*/
	case M4U_PORT_DISP_OVL0_2L_HDR:  return "DISP_OVL0_2L_HDR";
	case M4U_PORT_DISP_OVL1_2L_HDR:  return "DISP_OVL1_2L_HDR";
	case M4U_PORT_DISP_OVL0_2L:  return "DISP_OVL0_2L";
	case M4U_PORT_DISP_OVL1_2L:  return "DISP_OVL1_2L";
	case M4U_PORT_DISP_RDMA1:  return "DISP_RDMA1";
	case M4U_PORT_MDP_PVRIC0:  return "MDP_PVRIC0";
	case M4U_PORT_MDP_PVRIC1:  return "MDP_PVRIC1";
	case M4U_PORT_MDP_RDMA0:  return "MDP_RDMA0";
	case M4U_PORT_MDP_RDMA1:  return "MDP_RDMA1";
	case M4U_PORT_MDP_WROT0_R:  return "MDP_WROT0_R";
	case M4U_PORT_MDP_WROT0_W:  return "MDP_WROT0_W";
	case M4U_PORT_MDP_WROT1_R:  return "MDP_WROT1_R";
	case M4U_PORT_MDP_WROT1_W:  return "MDP_WROT1_W";
	case M4U_PORT_DISP_FAKE1:  return "DISP_FAKE1";

	/*larb2-VDEC-12*/
	case M4U_PORT_HW_VDEC_MC_EXT:  return "VDEC_MC_EXT";
	case M4U_PORT_HW_VDEC_UFO_EXT:  return "VDEC_UFO_EXT";
	case M4U_PORT_HW_VDEC_PP_EXT:  return "VDEC_PP_EXT";
	case M4U_PORT_HW_VDEC_PRED_RD_EXT:  return "VDEC_PRED_RD_EXT";
	case M4U_PORT_HW_VDEC_PRED_WR_EXT:  return "VDEC_PRED_WR_EXT";
	case M4U_PORT_HW_VDEC_PPWRAP_EXT:  return "VDEC_PPWRAP_EXT";
	case M4U_PORT_HW_VDEC_TILE_EXT:  return "VDEC_TILE_EXT";
	case M4U_PORT_HW_VDEC_VLD_EXT:  return "VDEC_VLD_EXT";
	case M4U_PORT_HW_VDEC_VLD2_EXT:  return "VDEC_VLD2_EXT";
	case M4U_PORT_HW_VDEC_AVC_MV_EXT:  return "VDEC_AVC_MV_EXT";
	case M4U_PORT_HW_VDEC_UFO_ENC_EXT:  return "VDEC_UFO_ENC_EXT";
	case M4U_PORT_HW_VDEC_RG_CTRL_DMA_EXT:  return "VDEC_RG_CTRL_DMA_EXT";

	/*larb3-VENC-19*/
	case M4U_PORT_VENC_RCPU:  return "VENC_RCPU";
	case M4U_PORT_VENC_REC:  return "VENC_REC";
	case M4U_PORT_VENC_BSDMA:  return "VENC_BSDMA";
	case M4U_PORT_VENC_SV_COMV:  return "VENC_SV_COMV";
	case M4U_PORT_VENC_RD_COMV:  return "VENC_RD_COMV";
	case M4U_PORT_VENC_NBM_RDMA:  return "VENC_NBM_RDMA";
	case M4U_PORT_VENC_NBM_RDMA_LITE:  return "VENC_NBM_RDMA_LITE";
	case M4U_PORT_JPGENC_Y_RDMA:  return "JPGENC_Y_RDMA";
	case M4U_PORT_JPGENC_C_RDMA:  return "JPGENC_C_RDMA";
	case M4U_PORT_JPGENC_Q_TABLE:  return "JPGENC_Q_TABLE";
	case M4U_PORT_JPGENC_BSDMA:  return "JPGENC_BSDMA";
	case M4U_PORT_JPGEDC_WDMA:  return "JPGEDC_WDMA";
	case M4U_PORT_JPGEDC_BSDMA:  return "JPGEDC_BSDMA";
	case M4U_PORT_VENC_NBM_WDMA:  return "VENC_NBM_WDMA";
	case M4U_PORT_VENC_NBM_WDMA_LITE:  return "VENC_NBM_WDMA_LITE";
	case M4U_PORT_VENC_CUR_LUMA:  return "VENC_CUR_LUMA";
	case M4U_PORT_VENC_CUR_CHROMA:  return "VENC_CUR_CHROMA";
	case M4U_PORT_VENC_REF_LUMA:  return "VENC_REF_LUMA";
	case M4U_PORT_VENC_REF_CHROMA:  return "VENC_REF_CHROMA";

	/*larb5-IMG-26*/
	case M4U_PORT_IMGI_D1:  return "IMGI_D1";
	case M4U_PORT_IMGBI_D1:  return "IMGBI_D1";
	case M4U_PORT_DMGI_D1:  return "DMGI_D1";
	case M4U_PORT_DEPI_D1:  return "DEPI_D1";
	case M4U_PORT_LCEI_D1:  return "LCEI_D1";
	case M4U_PORT_SMTI_D1:  return "SMTI_D1";
	case M4U_PORT_SMTO_D2:  return "SMTO_D2";
	case M4U_PORT_SMTO_D1:  return "SMTO_D1";
	case M4U_PORT_CRZO_D1:  return "CRZO_D1";
	case M4U_PORT_IMG3O_D1:  return "IMG3O_D1";

	case M4U_PORT_VIPI_D1:  return "VIPI_D1";
	case M4U_PORT_WPE_RDMA1:  return "WPE_RDMA1";
	case M4U_PORT_WPE_RDMA0:  return "WPE_RDMA0";
	case M4U_PORT_WPE_WDMA:  return "WPE_WDMA";
	case M4U_PORT_TIMGO_D1:  return "TIMGO_D1";
	case M4U_PORT_MFB_RDMA0:  return "MFB_RDMA0";
	case M4U_PORT_MFB_RDMA1:  return "MFB_RDMA1";
	case M4U_PORT_MFB_RDMA2:  return "MFB_RDMA2";
	case M4U_PORT_MFB_RDMA3:  return "MFB_RDMA3";
	case M4U_PORT_MFB_WDMA:  return "MFB_WDMA";

	case M4U_PORT_RESERVE1:  return "RESERVE1";
	case M4U_PORT_RESERVE2:  return "RESERVE2";
	case M4U_PORT_RESERVE3:  return "RESERVE3";
	case M4U_PORT_RESERVE4:  return "RESERVE4";
	case M4U_PORT_RESERVE5:  return "RESERVE5";
	case M4U_PORT_RESERVE6:  return "RESERVE6";

	/*larb7-IPESYS-4*/
	case M4U_PORT_DVS_RDMA:  return "DVS_RDMA";
	case M4U_PORT_DVS_WDMA:  return "DVS_WDMA";
	case M4U_PORT_DVP_RDMA:  return "DVP_RDMA";
	case M4U_PORT_DVP_WDMA:  return "DVP_WDMA";

	/*larb8-IPESYS-10*/
	case M4U_PORT_FDVT_RDA:  return "FDVT_RDA";
	case M4U_PORT_FDVT_RDB:  return "FDVT_RDB";
	case M4U_PORT_FDVT_WRA:  return "FDVT_WRA";
	case M4U_PORT_FDVT_WRB:  return "FDVT_WRB";
	case M4U_PORT_FE_RD0:  return "FE_RD0";
	case M4U_PORT_FE_RD1:  return "FE_RD1";
	case M4U_PORT_FE_WR0:  return "FE_WR0";
	case M4U_PORT_FE_WR1:  return "FE_WR1";
	case M4U_PORT_RSC_RDMA0:  return "RSC_RDMA0";
	case M4U_PORT_RSC_WDMA:  return "RSC_WDMA";

	/*larb9-CAM-24*/
	case M4U_PORT_CAM_IMGO_C:  return "CAM_IMGO_C";
	case M4U_PORT_CAM_RRZO_C:  return "CAM_RRZO_C";
	case M4U_PORT_CAM_LSCI_C:  return "CAM_LSCI_C";
	case M4U_PORT_CAM_BPCI_C:  return "CAM_BPCI_C";
	case M4U_PORT_CAM_YUVO_C:  return "CAM_YUVO_C";
	case M4U_PORT_CAM_UFDI_R2_C:  return "CAM_UFDI_R2_C";
	case M4U_PORT_CAM_RAWI_R2_C:  return "CAM_RAWI_R2_C";
	case M4U_PORT_CAM_RAWI_R5_C:  return "CAM_RAWI_R5_C";
	case M4U_PORT_CAM_CAMSV_1:  return "CAM_CAMSV_1";
	case M4U_PORT_CAM_CAMSV_2:  return "CAM_CAMSV_2";

	case M4U_PORT_CAM_CAMSV_3:  return "CAM_CAMSV_3";
	case M4U_PORT_CAM_CAMSV_4:  return "CAM_CAMSV_4";
	case M4U_PORT_CAM_CAMSV_5:  return "CAM_CAMSV_5";
	case M4U_PORT_CAM_CAMSV_6:  return "CAM_CAMSV_6";
	case M4U_PORT_CAM_AAO_C:  return "CAM_AAO_C";
	case M4U_PORT_CAM_AFO_C:  return "CAM_AFO_C";
	case M4U_PORT_CAM_FLKO_C:  return "CAM_FLKO_C";
	case M4U_PORT_CAM_LCESO_C:  return "CAM_LCESO_C";
	case M4U_PORT_CAM_CRZO_C:  return "CAM_CRZO_C";
	case M4U_PORT_CAM_LTMSO_C:  return "CAM_LTMSO_C";

	case M4U_PORT_CAM_RSSO_C:  return "CAM_RSSO_C";
	case M4U_PORT_CAM_CCUI:  return "CAM_CCUI";
	case M4U_PORT_CAM_CCUO:  return "CAM_CCUO";
	case M4U_PORT_CAM_FAKE:  return "CAM_FAKE";

	/*larb10-CAM-31*/
	case M4U_PORT_CAM_IMGO_A:  return "CAM_IMGO_A";
	case M4U_PORT_CAM_RRZO_A:  return "CAM_RRZO_A";
	case M4U_PORT_CAM_LSCI_A:  return "CAM_LSCI_A";
	case M4U_PORT_CAM_BPCI_A:  return "CAM_BPCI_A";
	case M4U_PORT_CAM_YUVO_A:  return "CAM_YUVO_A";
	case M4U_PORT_CAM_UFDI_A:  return "CAM_UFDI_A";
	case M4U_PORT_CAM_RAWI_R2_A:  return "CAM_RAWI_R2_A";
	case M4U_PORT_CAM_RAWI_R5_A:  return "CAM_RAWI_R5_A";
	case M4U_PORT_CAM_IMGO_B:  return "CAM_IMGO_B";
	case M4U_PORT_CAM_RRZO_B:  return "CAM_RRZO_B";

	case M4U_PORT_CAM_LSCI_B:  return "CAM_LSCI_B";
	case M4U_PORT_CAM_BPCI_B:  return "CAM_BPCI_B";
	case M4U_PORT_CAM_YUVO_B:  return "CAM_YUVO_B";
	case M4U_PORT_CAM_UFDI_B:  return "CAM_UFDI_B";
	case M4U_PORT_CAM_RAWI_R2_B:  return "CAM_RAWI_R2_B";
	case M4U_PORT_CAM_RAWI_R5_B:  return "CAM_RAWI_R5_B";
	case M4U_PORT_CAM_CAMSV_0:  return "CAM_CAMSV_0";
	case M4U_PORT_CAM_AAO_A:  return "CAM_AAO_A";
	case M4U_PORT_CAM_AFO_A:  return "CAM_AFO_A";
	case M4U_PORT_CAM_FLKO_A:  return "CAM_FLKO_A";

	case M4U_PORT_CAM_LCESO_A:  return "CAM_LCESO_A";
	case M4U_PORT_CAM_CRZO_A:  return "CAM_CRZO_A";
	case M4U_PORT_CAM_AAO_B:  return "CAM_AAO_B";
	case M4U_PORT_CAM_AFO_B:  return "CAM_AFO_B";
	case M4U_PORT_CAM_FLKO_B:  return "CAM_FLKO_B";
	case M4U_PORT_CAM_LCESO_B:  return "CAM_LCESO_B";
	case M4U_PORT_CAM_CRZO_B:  return "CAM_CRZO_B";
	case M4U_PORT_CAM_LTMSO_A:  return "CAM_LTMSO_A";
	case M4U_PORT_CAM_RSSO_A:  return "CAM_RSSO_A";
	case M4U_PORT_CAM_LTMSO_B:  return "CAM_LTMSO_B";
	case M4U_PORT_CAM_RSSO_B:  return "CAM_RSSO_B";

	case M4U_PORT_CCU0:  return "CCU0";

	case M4U_PORT_UNKNOWN:	  return "UNKNOWN";
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
	else if ((fault_id & COM_EDMA) == EDMA)
		return "EDMA";
	else if ((fault_id & COM_IDMA_1) == IDMA_1)
		return "VPU_IDMA_1";
	else if ((fault_id & COM_CORE_1) == CORE_1)
		return "VPU_CORE_1";
	else if ((fault_id & COM_EDMB) == EDMB)
		return "EDMB";
	else if ((fault_id & COM_CORE_2) == CORE_2)
		return "MDLA_CORE_2";
	else if ((fault_id & COM_EDMC) == EDMC)
		return "EDMC";
	else
		return "VPU_UNKNOWN";
}

static uint64_t smiLarbBasePA[SMI_LARB_NR] = {LARB0_BASE_PA, LARB1_BASE_PA,
							LARB2_BASE_PA, LARB3_BASE_PA, LARB5_BASE_PA,
							LARB7_BASE_PA, LARB8_BASE_PA, LARB9_BASE_PA, LARB10_BASE_PA};

static unsigned int m4u_port0_in_larbx[SMI_LARB_NR] = {M4U_PORT_DISP_POSTMASK0,
							M4U_PORT_DISP_OVL0_2L_HDR, M4U_PORT_HW_VDEC_MC_EXT,
							M4U_PORT_VENC_RCPU, M4U_PORT_IMGI_D1, M4U_PORT_DVS_RDMA,
							M4U_PORT_FDVT_RDA,
							M4U_PORT_CAM_IMGO_C, M4U_PORT_CAM_IMGO_A};

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

