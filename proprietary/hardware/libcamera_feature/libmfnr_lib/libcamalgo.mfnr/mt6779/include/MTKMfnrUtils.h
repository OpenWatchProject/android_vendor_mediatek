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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MTK_MFNRUTILS_H
#define _MTK_MFNRUTILS_H

#include "MTKMfnrUtilsType.h"
#include "MTKMfnrUtilsErrCode.h"

typedef enum DRVMfnrUtilsObject_s
{
    DRV_MFNRUTILS_OBJ_NONE = 0,
    DRV_MFNRUTILS_OBJ_SW,
    DRV_MFNRUTILS_OBJ_UNKNOWN = 0xFF,
} DrvMfnrUtilsObject_e;

/*****************************************************************************
  Main Module
******************************************************************************/
typedef enum MFNRUTILS_PROC_ENUM
{
    MFNRUTILS_PROC1 = 0,
    MFNRUTILS_UNKNOWN_PROC
} MFNRUTILS_PROC_ENUM;

typedef enum MFNRUTILS_FTCTRL_ENUM
{
    MFNRUTILS_FTCTRL_PROC_AEVC_COMP_GAIN,
    MFNRUTILS_FTCTRL_PROC_AEVC_COMP_LCSO,
    MFNRUTILS_FTCTRL_GET_VERSION,        // feature id to get Version
    MFNRUTILS_FTCTRL_MAX
} MFNRUTILS_FTCTRL_ENUM;

struct MFNRUTILS_PROC_AEVC_COMP_GAIN_IN_STRUCT
{
    MUINT16 * pu2IspGain;  // number of i4FrameNum
    MUINT16 * pu2AGain;    // number of i4FrameNum
    MUINT32 * pu4Shutter;  // number of i4FrameNum
    MINT32 i4BaseIdx;
    MINT32 * pu4Valid;     // number of i4FrameNum
    MINT32 i4FrameNum;
    MUINT16 i2MinIspGain;
};

struct MFNRUTILS_PROC_AEVC_COMP_GAIN_OUT_STRUCT
{
    MUINT16 * pu2NewIspGain;
};

struct MFNRUTILS_PROC_AEVC_COMP_LCSO_IN_STRUCT
{
    MUINT16 ** pu2Lcso;      // number of i4FrameNum x i4LcsoSize
    MUINT16 * pu2OrIspGain;  // number of i4FrameNum
    MUINT16 * pu2NewIspGain; // number of i4FrameNum
    MINT32 i4LcsoSize;
    MINT32 i4FrameNum;
};

struct MFNRUTILS_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;                      // get Version by FeatureCtrl(GET_VERSION)

/*******************************************************************************
*
********************************************************************************/
class MTKMfnrUtils
{
public:
    static MTKMfnrUtils* createInstance(DrvMfnrUtilsObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKMfnrUtils(){}
    virtual MRESULT MfnrUtilsInit(void* pParaIn, void* pParaOut);
    virtual MRESULT MfnrUtilsReset(void);
    virtual MRESULT MfnrUtilsMain(MFNRUTILS_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT MfnrUtilsFeatureCtrl(MFNRUTILS_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppMfnrUtilsTmp : public MTKMfnrUtils
{
public:

    static MTKMfnrUtils* getInstance();
    virtual void destroyInstance();

    AppMfnrUtilsTmp() {};
    virtual ~AppMfnrUtilsTmp() {};
};
#endif

