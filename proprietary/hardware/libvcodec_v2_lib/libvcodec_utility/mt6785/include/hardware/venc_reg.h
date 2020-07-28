/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __VNEC_REG_H__
#define __VNEC_REG_H__

#define _VENC_HW_BASE                   u4VENC_HW_BASE              // MT8135: 0x17002000
#define _VENC_HW_BASE_RANGE             u4VENC_HW_BASE_RANGE        // MT8135: 0x1000

#define _VENC_MP4_HW_BASE               u4VENC_MP4_HW_BASE          // MT8135: 0x17002000
#define _VENC_MP4_HW_BASE_RANGE         u4VENC_MP4_HW_BASE_RANGE    // MT8135: 0x0100

//6582 VENC register definitions
#define VENC_HW_MODE_SEL                            0x0000
#define VENC_ENCODER_INFO_0                         0x0004
#define VENC_ENCODER_INFO_1                         0x0008
#define VENC_ENCODER_INFO_2                         0x000C
#define VENC_ENCODER_INFO_3                         0x0010
#define VENC_ENCODER_INFO_4                         0x0014
#define VENC_ENCODER_INFO_5                         0x0018
#define VENC_ENCODER_INFO_6                         0x001C
#define VENC_ENCODER_INFO_7                         0x0020
#define VENC_IMAGE_INFO_0                           0x0024
#define VENC_IMAGE_INFO_1                           0x0028
#define VENC_IMAGE_INFO_2                           0x002C
#define VENC_H264_ENC_INFO_0                        0x0030
#define VENC_H264_ENC_INFO_1                        0x0034
#define VENC_VP8_ENC_INFO_0                         0x0040
#define VENC_VP8_ENC_INFO_1                         0x0044
#define VENC_RATECONTROL_INFO_0                     0x0048
#define VENC_RATECONTROL_INFO_1                     0x004C
#define VENC_RATECONTROL_INFO_2                     0x0050
#define VENC_RATECONTROL_INFO_3                     0x0054
#define VENC_CODEC_CTRL                             0x0058
#define VENC_IRQ_STATUS                             0x005C
#define VENC_IRQ_ACK                                0x0060
#define VENC_BITSTREAM_BUF_DRAM_ADDR                0x0064
#define VENC_BITSTREAM_BUF_DRAM_SIZE                0x0068
#define VENC_FRM_CUR_Y_DRAM_ADDR                    0x006C
#define VENC_FRM_CUR_UV_DRAM_ADDR                   0x0070
#define VENC_FRM_REF_Y_DRAM_ADDR                    0x0074
#define VENC_FRM_REF_UV_DRAM_ADDR                   0x0078
#define VENC_FRM_REC_Y_DRAM_ADDR                    0x007C
#define VENC_FRM_REC_UV_DRAM_ADDR                   0x0080
#define VENC_LOAD_COL_INFO_DRAM_ADDR                0x0084
#define VENC_SAVE_COL_INFO_DRAM_ADDR                0x0088
#define VENC_RC_CODE_DRAM_ADDR                      0x008C
#define VENC_RC_INFO_DRAM_ADDR                      0x0090
#define VENC_CLK_CG_CTRL                            0x0094
#define VENC_PIC_BITSTREAM_BYTE_CNT                 0x0098
#define VENC_CODEC_CNT0                             0x009C
#define VENC_STUFFING_REPORT                        0x00A0
#define VENC_IRQ_MODE_SEL                           0x00A4
#define VENC_SW_HRST_N                              0x00A8
#define VENC_SW_PAUSE                               0x00AC
#define VENC_PAUSE_MODE_INFO                        0x00B0
#define VENC_CRC_BSDMA                              0x00BC
#define VENC_CRC_CUR_LUMA                           0x00C0
#define VENC_CRC_REF_LUMA                           0x00C4
#define VENC_CRC_CUR_CHROMA                         0x00C8
#define VENC_CRC_REF_CHROMA                         0x00CC
#define VENC_CRC_REC_FRM                            0x00D8
#define VENC_CRC_BSDMA_1                            0x00DC
#define VENC_CE                                     0x00EC
#define VENC_CLK_DCM_CTRL                           0x00F4
#define VENC_ROI_INDEX_0                            0x0100
#define VENC_ROI_INDEX_1                            0x0104
#define VENC_ROI_INDEX_2                            0x0108
#define VENC_ROI_INDEX_3                            0x010C
#define VENC_EIS_SAD_CUR_FRM                        0x0110
#define VENC_EIS_SAD_PREV_FRM_0                     0x0114
#define VENC_EIS_SAD_PREV_FRM_1                     0x0118
#define VENC_EIS_SAD_PREV_FRM_2                     0x011C
#define VENC_EIS_INFO_0                             0x0120
#define VENC_AVSR_ANRP_INFO_0                       0x0124
#define VENC_AVSR_ANRP_INFO_1                       0x0128
#define VENC_SMI_GULTRA_THRES                       0x012C
#define VENC_FRM_CUR_V_DRAM_ADDR                    0x0130
#define VENC_VUI_PARAMETER_INFO_0                   0x0180
#define VENC_VUI_PARAMETER_INFO_1                   0x0184
#define VENC_VUI_PARAMETER_INFO_2                   0x0188
#define VENC_VUI_PARAMETER_INFO_3                   0x018C
#define VENC_VUI_PARAMETER_INFO_4                   0x0190
#define VENC_VUI_PARAMETER_INFO_5                   0x0194
#define VENC_VUI_PARAMETER_INFO_6                   0x0198
#define VENC_VUI_PARAMETER_INFO_7                   0x019C
#define VENC_VUI_PARAMETER_INFO_8                   0x01A0
#define VENC_VUI_PARAMETER_INFO_9                   0x01A4
#define VENC_CODEC_SEL                              0x01B8
#define VENC_PRELOAD_INFO                           0x01BC

/* ROME VENC register definitions */
#define VENC_HEVC_ENC_INFO_0                        0x0038
#define VENC_HEVC_ENC_INFO_1                        0x003C
#define VENC_FRM_CUR_V_DRAM_ADDR_V2                 0x0094
#define VENC_CRC_RD_COMV                            0x00D0
#define VENC_CRC_SV_COMV                            0x00D4
#define VENC_CLK_CG_CTRL_V2                         0x00FC
#define VENC_CMDQ_EN                                0x130
#define VENC_USING_RACING_INPUT_INFO                0x0160
#define VENC_UFO_CTRL                               0x01C0
#define VENC_UFO_STREAM_SIZE                        0x01C4
#define VENC_UFOD_SMI_INTERVAL                      0x01C8
#define VENC_WFD_CHECKER_1                          0x01CC
#define VENC_WFD_CHECKER_2                          0x01D0
#define VENC_BSDMA_WRSP_CNT                         0x01D4
#define VENC_CRC_UFOD                               0x01D8
#define VENC_NBM_INFO_0                             0x01E0
#define VENC_NBM_INFO_1                             0x01E4
#define VENC_NBM_INFO_2                             0x01E8
#define VENC_NBM_INFO_3                             0x01EC
#define VENC_NBM_INFO_4                             0x01F0

/* 6592 VENC register definitions */
#define VENC_WFD_CHECKR_1                           0x01CC
#define VENC_WFD_CHECKR_2                           0x01D0
#define VENC_BSDMA_WRSP_CNT                         0x01D4

/* JADE VENC register definitions */
#define VENC_UFO_CTRL_jade                          0x0164
#define VENC_UFO_STREAM_SIZE_jade                   0x0168
#define VENC_UFOD_SMI_INTERVAL_jade                 0x016C
#define VENC_CRC_UFOD_jade                          0x0170

/* 8135 VENC register definitions */
#define VENC_MPEG4_ENC_INFO_0                       0x0038
#define VENC_MPEG4_ENC_INFO_1                       0x003C
#define VENC_CRC_RD_COMV                            0x00D0
#define VENC_CRC_SV_COMV                            0x00D4
#define VENC_MPEG4_ENC_BIT_COUNT                    0x01B0
#define VENC_MPEG4_ENC_NON_ZERO_COEF_COUNT          0x01B4


/* 6589 VENC registers definitions */
#define VENC_VP8_ENC_INFO_0                         0x0040
#define VENC_VP8_HEADER_BITSTREAM_BUF_DRAM_ADDR     0x00E0
#define VENC_VP8_HEADER_BITSTREAM_BUF_DRAM_SIZE     0x00E4
#define VENC_PIC_BITSTREAM_BYTE_CNT1                0x00E8
#define VENC_VP8_PROB_DRAM_ADDR                     0x00F0

//89 MPEG4 ENC BASE 17002600
#define VENC_MP4_FRAME_START                        0x0000
#define VENC_MP4_SLICE_START                        0x0004
#define VENC_MP4_MBX_LMT                            0x0008
#define VENC_MP4_MBY_LMT                            0x000C
#define VENC_MP4_MBX_STOP                           0x0010
#define VENC_MP4_MBY_STOP                           0x0014
#define VENC_MP4_VOP_TYPE                           0x0018
#define VENC_MP4_SHRT_VDO                           0x001C
#define VENC_MP4_FCODE                              0x0020
#define VENC_MP4_BCODE                              0x0024
#define VENC_MP4_RND_CTRL                           0x0028
#define VENC_MP4_INIT_BITH                          0x002C
#define VENC_MP4_INIT_BITL                          0x0030
#define VENC_MP4_BIT_IDX                            0x0034
#define VENC_MP4_SIDE_ADDR                          0x0038
#define VENC_MP4_SRCADR_Y                           0x003C
#define VENC_MP4_SRCADR_CB                          0x0040
#define VENC_MP4_SRCADR_CR                          0x0044
#define VENC_MP4_RECADR_Y                           0x0048
#define VENC_MP4_RECADR_CB                          0x004C
#define VENC_MP4_RECADR_CR                          0x0050
#define VENC_MP4_REFADR_Y                           0x0054
#define VENC_MP4_REFADR_CB                          0x0058
#define VENC_MP4_REFADR_CR                          0x005C
#define VENC_MP4_BITADR                             0x0060
#define VENC_MP4_ENC_STATUS                         0x0064
#define VENC_MP4_IRQ_EN                             0x0068
#define VENC_MP4_MC_CTRL                            0x006C
#define VENC_MP4_BSDMA_CTRL                         0x0070
#define VENC_MP4_ZERO_COEF_COUNT_2                  0x0074
#define VENC_MP4_IRQ_ACK                            0x0078
#define VENC_MP4_IRQ_STATUS                         0x007C
#define VENC_MP4_BYTE_COUNT                         0x0080
#define VENC_MP4_BIT_COUNT                          0x0084
#define VENC_MP4_ZERO_COEF_COUNT                    0x0088
#define VENC_MP4_QP                                 0x008C
#define VENC_MP4_RESET                              0x0090
#define VENC_MP4_CDMA_CTRL                          0x0094
#define VENC_MP4_ZERO_COEF_COUNT_3                  0x0098
#define VENC_MP4_BYTE_COUNT2                        0x009C
#define VENC_MP4_DEBUG0                             0x00A0
#define VENC_MP4_MBX_STOP2                          0x00A4
#define VENC_MP4_MBY_STOP2                          0x00A8
#define VENC_MP4_MVQP_TOTAL_CNT2                    0x00AC
#define VENC_MP4_IRQ_EN2                            0x00B0
#define VENC_MP4_AUTO_ACK2                          0x00B4
#define VENC_MP4_CHECKSUM0                          0x00B8
#define VENC_MP4_CMDQ_SET                           0x00BC
#define VENC_MP4_DRAM_CTRL2                         0x00C0
#define VENC_MP4_BYTE_COUNT3                        0x00C4
#define VENC_MP4_CHECKSUM4                          0x00C8
#define VENC_MP4_CHECKSUM5                          0x00CC
#define VENC_MP4_CDMA_STATUS                        0x00D0
#define VENC_MP4_WCDMA_STATUS                       0x00D4
#define VENC_MP4_BSDMA_STATUS                       0x00D8
#define VENC_MP4_MC0_STATUS                         0x00DC
#define VENC_MP4_MC1_STATUS                         0x00E0
#define VENC_MP4_MVQP_STATUS                        0x00E4
#define VENC_MP4_DRAM_CTRL                          0x00E8
#define VENC_MP4_MVQP_CTRL                          0x00EC
#define VENC_MP4_DCM_CTRL                           0x00F0
#define VENC_MP4_AUTO_ACK                           0x00F4
#define VENC_MP4_CHECKSUM6                          0x00F8

#define VENC_SAVE_SEGID_DRAM_ADDR                   0x102C

#endif  //__VNEC_REG_H__
