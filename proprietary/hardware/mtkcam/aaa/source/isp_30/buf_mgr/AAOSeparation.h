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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _AAO_SEPARATION_H_
#define _AAO_SEPARATION_H_

#include <aaa_types.h>

#define AAO_STT_DATA_SIZE (10)                    // byte
#define AAO_STT_PLINE_NUM (90)
#define AAO_STT_BLOCK_NUM (120)
#define AAO_STT_HIST_BIN_NUM (128)
#define AAO_STT_HIST_DATA_SIZE (3)                // byte
#define AAO_STT_HIST_NUM (4)
#define AAO_BUF_SIZE (AAO_STT_DATA_SIZE * AAO_STT_PLINE_NUM * AAO_STT_BLOCK_NUM + AAO_STT_HIST_BIN_NUM * AAO_STT_HIST_DATA_SIZE * AAO_STT_HIST_NUM)//109536
#define STAT_OPT(hdr, over, tsf) (((hdr)?4:0)|((over)?2:0)|((tsf)?1:0))

enum AAOType
{
    AAOCType = 0,
    AAONeonType
};

typedef struct
{
    MINT32                  m_i4AwbLineSize;//aaoNoAWBSz
    MINT32                  m_i4AeSize     ;// in byte
    MINT32                  m_i4HdrSize    ;// in byte
    MINT32                  m_i4AeOverSize ;// in byte
    MINT32                  m_i4TsfSize    ;// in byte
    MINT32                  m_i4LineSize   ;// lineByte
    MINT32                  m_i4SkipSize   ;
    MINT32                  m_i4BlkNumX;//m_rTsfEnvInfo.ImgWidth;
    MINT32                  m_i4BlkNumY;//m_rTsfEnvInfo.ImgHeight;
    MBOOL                   m_bEnableHDRYConfig;
    MBOOL                   m_bEnableOverCntConfig;
    MBOOL                   m_bEnableTSFConfig;
    MBOOL                   m_bDisablePixelHistConfig;
    MUINT32                 m_u4AEOverExpCntShift;
} AAO_BUF_CONFIG_T;

typedef struct
{
    MVOID*                  m_pAAOSepAWBBuf;
    MVOID*                  m_pAAOSepAEBuf;
    MVOID*                  m_pAAOSepHistBuf;
    MVOID*                  m_pAAOSepOverCntBuf;
    MVOID*                  m_pAAOSepLSCBuf;
} AAO_BUF_T;

MBOOL aaoSeparation12(const MVOID* AAOSrc , const AAO_BUF_CONFIG_T& rAAOBufConfig, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType);
MBOOL aaoSeparation14(const MVOID* AAOSrc , const AAO_BUF_CONFIG_T& rAAOBufConfig, AAO_BUF_T& rAAOBuf, const MINT32 AAOSepType);

#endif //_AAO_SEPARATION_H_

