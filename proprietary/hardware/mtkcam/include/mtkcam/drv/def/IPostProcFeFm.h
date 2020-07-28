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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_IPOSTPROCFEFM_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_IPOSTPROCFEFM_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {


/******************************************************************************
 * @struct FEInfo
 *
 * @brief FEInfo.
 *
 * @param[in] CmdIdx: specific command index: EPIPE_FE_INFO_CMD
 *
 * @param[in] moduleStruct: specific structure: FEInfo
 *
 * @param[in] s: size (i.e. width and height) in pixels.
 *  
 ******************************************************************************/
struct FEInfo
{
    MUINT32 mFEDSCR_SBIT;
    MUINT32 mFETH_C;
    MUINT32 mFETH_G;
    MUINT32 mFEFLT_EN;
    MUINT32 mFEPARAM;
    MUINT32 mFEMODE;
    MUINT32 mFEYIDX;
    MUINT32 mFEXIDX;
    MUINT32 mFESTART_X;
    MUINT32 mFESTART_Y;
    MUINT32 mFEIN_HT;
    MUINT32 mFEIN_WD;
    FEInfo()
        : mFEDSCR_SBIT(0)
        , mFETH_C(0)
        , mFETH_G(0)
        , mFEFLT_EN(0)
        , mFEPARAM(0)
        , mFEMODE(0)
        , mFEYIDX(0)
        , mFEXIDX(0)
        , mFESTART_X(0)
        , mFESTART_Y(0)
        , mFEIN_HT(0)
        , mFEIN_WD(0)
    {
    }
};


/******************************************************************************
 * @struct FMInfo
 *
 * @brief FMInfo.
 *
 * @param[in] CmdIdx: specific command index: EPIPE_FM_INFO_CMD
 *
 * @param[in] moduleStruct: specific structure: FMInfo
 *
 * @param[in] s: size (i.e. width and height) in pixels.
 *  
 ******************************************************************************/
struct FMInfo
{
    MUINT32 mFMHEIGHT;
    MUINT32 mFMWIDTH;
    MUINT32 mFMSR_TYPE;
    MUINT32 mFMOFFSET_X;
    MUINT32 mFMOFFSET_Y;
    MUINT32 mFMRES_TH;
    MUINT32 mFMSAD_TH;
    MUINT32 mFMMIN_RATIO;
    FMInfo()
        : mFMHEIGHT(0)
        , mFMWIDTH(0)
        , mFMSR_TYPE(0)
        , mFMOFFSET_X(0)
        , mFMOFFSET_Y(0)
        , mFMRES_TH(0)
        , mFMSAD_TH(0)
        , mFMMIN_RATIO(0)
    {
    }
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_IPOSTPROCFEFM_H_

