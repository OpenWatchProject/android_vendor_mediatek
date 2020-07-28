/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __TLDAPC_INDEX_H__
#define __TLDAPC_INDEX_H__

/* Master's Index */
#define DAPC_MASTER_INDEX_SPI0      9
#define DAPC_MASTER_INDEX_SPI1      1
#define DAPC_MASTER_INDEX_SPI2      28
#define DAPC_MASTER_INDEX_SPI3      29
#define DAPC_MASTER_INDEX_SPI4      19
#define DAPC_MASTER_INDEX_SPI5      20

/* Slave's Domain Setting */
#define DOMAIN_AP                   0
#define DOMAIN_MD                   1
#define DOMAIN2                     2
#define DOMAIN3                     3
#define DOMAIN4                     4
#define DOMAIN5                     5
#define DOMAIN6                     6
#define DOMAIN7                     7

/* Master's Transaction */
#define SECURE_TRANSACTION          1
#define NON_SECURE_TRANSACTION      0

/* Slave's Access Permission Setting */
#define NO_PROTECTION                     0
#define SECURE_RW_ONLY                    1
#define SECURE_RW_AND_NON_SECURE_R_ONLY   2
#define NO_ACCESSIBLE                     3

/* Slave's Index */
#define DAPC_INDEX_SPI0       84
#define DAPC_INDEX_SPI1       90
#define DAPC_INDEX_SPI2       92
#define DAPC_INDEX_SPI3       93
#define DAPC_INDEX_SPI4       98
#define DAPC_INDEX_SPI5       99

#define DAPC_INDEX_I2C0       81
#define DAPC_INDEX_I2C1       82
#define DAPC_INDEX_I2C2       83
#define DAPC_INDEX_I2C3       89
#define DAPC_INDEX_I2C4       91
#define DAPC_INDEX_I2C5       96

#define DAPC_INDEX_I2C1_IMM       94
#define DAPC_INDEX_I2C2_IMM       95
#define DAPC_INDEX_I2C5_IMM       97

#define DAPC_INDEX_MMSYS_CONFIG        123
#define DAPC_INDEX_MDP_RDMA0           124
#define DAPC_INDEX_MDP_RDMA1           125
#define DAPC_INDEX_MDP_RSZ1            127
#define DAPC_INDEX_MDP_RSZ2            128
#define DAPC_INDEX_MDP_WROT0           130
#define DAPC_INDEX_MDP_WROT1           131
#define DAPC_INDEX_MDP_TDSHP           132
#define DAPC_INDEX_MDP_COLOR           133
#define DAPC_INDEX_DISP_OVL0           134
#define DAPC_INDEX_DISP_OVL1           135
#define DAPC_INDEX_DISP_OVL0_2L        136
#define DAPC_INDEX_DISP_OVL1_2L        137
#define DAPC_INDEX_DISP_RDMA0          138
#define DAPC_INDEX_DISP_RDMA1          139
#define DAPC_INDEX_DISP_WDMA0          141
#define DAPC_INDEX_DISP_WDMA1          142
#define DAPC_INDEX_DISP_COLOR0         143
#define DAPC_INDEX_DISP_CCORR0         145
#define DAPC_INDEX_DISP_AAL0           147
#define DAPC_INDEX_DISP_GAMMA0         149
#define DAPC_INDEX_DISP_DITHER0        152
#define DAPC_INDEX_DSI_UFOE            154
#define DAPC_INDEX_DSI_SPLIT           156
#define DAPC_INDEX_DSI0                157
#define DAPC_INDEX_DSI1                158
#define DAPC_INDEX_DPI0                159
#define DAPC_INDEX_MM_MUTEX            160
#define DAPC_INDEX_SMI_LARB0           161
#define DAPC_INDEX_SMI_LARB4           162
#define DAPC_INDEX_SMI_COMMON          163
#define DAPC_INDEX_MMSYS_VAD           164
#define DAPC_INDEX_MMSYS_DFP           165

#define DAPC_INDEX_IMGSYS_CONFIG       166
#define DAPC_INDEX_SMI_LARB5           167
#define DAPC_INDEX_DIP_A0              168
#define DAPC_INDEX_DIP_A1              169
#define DAPC_INDEX_DIP_A2              170
#define DAPC_INDEX_VAD                 173
#define DAPC_INDEX_DPE                 174
#define DAPC_INDEX_RSC                 175
#define DAPC_INDEX_FDVT                177
#define DAPC_INDEX_GEPF                178
#define DAPC_INDEX_DFP                 180

#define DAPC_INDEX_VDEC_GLOBAL_CON     182
#define DAPC_INDEX_SMI_LARB1           183
#define DAPC_INDEX_VDEC_FULL_TOP       184

#define DAPC_INDEX_VENC_GLOBAL_CON     185
#define DAPC_INDEX_SMI_LARB3           186
#define DAPC_INDEX_VENC                187
#define DAPC_INDEX_JPGENC              188
#define DAPC_INDEX_JPGDEC              189

#define DAPC_INDEX_CAMSYS_TOP          190
#define DAPC_INDEX_SMI_LARB2           191
#define DAPC_INDEX_CAM_TOP             192
#define DAPC_INDEX_CAM_A               193
#define DAPC_INDEX_CAM_B               194
#define DAPC_INDEX_CAM_TOP_SET         195
#define DAPC_INDEX_CAM_A_SET           196
#define DAPC_INDEX_CAM_B_SET           197
#define DAPC_INDEX_CAM_TOP_INNER       198
#define DAPC_INDEX_CAM_A_INNER         199
#define DAPC_INDEX_CAM_B_INNER         200
#define DAPC_INDEX_CAM_TOP_CLR         201
#define DAPC_INDEX_CAM_A_CLR           202
#define DAPC_INDEX_CAM_B_CLR           203
#define DAPC_INDEX_SENINF_A            204
#define DAPC_INDEX_SENINF_B            205
#define DAPC_INDEX_SENINF_C            206
#define DAPC_INDEX_SENINF_D            207
#define DAPC_INDEX_SENINF_E            208
#define DAPC_INDEX_SENINF_F            209
#define DAPC_INDEX_SENINF_G            210
#define DAPC_INDEX_SENINF_H            211
#define DAPC_INDEX_CAMSV_A             212
#define DAPC_INDEX_CAMSV_B             213
#define DAPC_INDEX_CAMSV_C             214
#define DAPC_INDEX_CAMSV_D             215
#define DAPC_INDEX_CAMSV_E             216
#define DAPC_INDEX_CAMSV_F             217
#define DAPC_INDEX_TSF                 218

#endif // __TLDAPC_INDEX_H__
