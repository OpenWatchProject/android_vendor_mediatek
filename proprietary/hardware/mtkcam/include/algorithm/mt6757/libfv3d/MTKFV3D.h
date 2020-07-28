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

#ifndef _MTK_FV3D_H
#define _MTK_FV3D_H

#include "MTKFV3DType.h"
#include "MTKFV3DErrCode.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

typedef enum DRVRefocusObject_s {
    DRV_FV3D_OBJ_NONE = 0,
    DRV_FV3D_OBJ_SW,
    DRV_FV3D_OBJ_UNKNOWN = 0xFF,
} DrvFV3DObject_e;

#define FV3D_TIME_PROF

/*****************************************************************************
    FV3D Define and State Machine
******************************************************************************/
#define FV3D_MAX_WIDTH_HEIGHT   (8192)

/*****************************************************************************
    Process Control
******************************************************************************/
typedef enum FV3D_STATE_ENUM
{
    FV3D_STATE_STANDBY=0,            // After Create Obj or Reset
    FV3D_STATE_INIT,                 // After Called RefocusInit
    FV3D_STATE_PROCESS,              // After Called RefocusMain
    FV3D_STATE_PROCESS_DONE,         // After Finish RefocusMain
} FV3D_STATE_ENUM;

typedef enum
{
    FV3D_FEATURE_BEGIN,
    FV3D_FEATURE_GET_WORKBUF_SIZE,
    FV3D_FEATURE_SET_WORKBUF_ADDR,
    FV3D_FEATURE_SET_INPUT_IMG,
    FV3D_FEATURE_SET_PROC_INFO,
    FV3D_FEATURE_MAX,
} FV3D_FEATURE_ENUM;

typedef struct FV3DInitInfo
{
    MUINT8* workingBufferAddr;
    MUINT32 workingBufferSize;
    MUINT32 outputWidth;
    MUINT32 outputHeight;
    MUINT32 inputWidth;
    MUINT32 inputHeight;
    MUINT32 depthWidth;
    MUINT32 depthHeight;
    MUINT32 orientation;
} FV3DInitInfo;

typedef struct FV3DImageInfo
{
    MUINT8* inputBufAddr;
    MUINT8* depthBufAddr;
}FV3DImageInfo;

typedef struct FV3DProcInfo
{
    MINT32 x_coord;
    MINT32 y_coord;
    GLuint  outputTexID;
}FV3DProcInfo;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/

/*******************************************************************************
*
********************************************************************************/
class MTKFV3D {
public:
    static MTKFV3D* createInstance(DrvFV3DObject_e eobject);
    virtual void   destroyInstance(MTKFV3D* obj) = 0;

    virtual ~MTKFV3D(){}
    // Process Control
    virtual MRESULT FV3DInit(void* InitInData, void* InitOutData);    // Env/Cb setting
    virtual MRESULT FV3DMain(void);                                         // START
    virtual MRESULT FV3DReset(void);                                        // RESET

    // Feature Control
    virtual MRESULT FV3DFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppFV3DTmp : public MTKFV3D {
public:
    //
    static MTKFV3D* getInstance();
    virtual void destroyInstance(){};
    //
    AppFV3DTmp() {};
    virtual ~AppFV3DTmp() {};
};

#endif

