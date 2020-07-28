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

#ifndef _MTK_RWB_H
#define _MTK_RWB_H

#include "MTKRWBType.h"
#include "MTKRWBErrCode.h"
#include "MTKCoreType.h"
//#include "MTKFeatureSet.h"

//#define TIME_PROF

typedef enum DRVRWBObject_s {
    DRV_RWB_OBJ_NONE = 0,
    DRV_RWB_OBJ_GLES,
    DRV_RWB_OBJ_CL,

    DRV_RWB_OBJ_UNKNOWN = 0xFF,
} DrvRWBObject_e;

/*****************************************************************************
    RWB Define and State Machine
******************************************************************************/
#define RWB_MAX_IMG_NUM            (5)        // maximum image number
#define RWB_BUFFER_UNSET           (0xFFFF)
//#define DEBUG

typedef enum
{
    RWB_STATE_STANDBY=0,   // After Create Obj or Reset
    RWB_STATE_INIT,        // After Called RWBInit
    RWB_STATE_PROC,        // After Called RWBMain
    RWB_STATE_PROC_READY,  // After Finish RWBMain
} RWB_STATE_ENUM;

/*****************************************************************************
    Process Control
******************************************************************************/
typedef enum MTK_RWB_IMAGE_ENUM
{
    RWB_IMAGE_RGB565=1,
    RWB_IMAGE_BGR565,
    RWB_IMAGE_RGB888,
    RWB_IMAGE_BGR888,
    RWB_IMAGE_ARGB888,
    RWB_IMAGE_ABGR888,
    RWB_IMAGE_BGRA8888,
    RWB_IMAGE_RGBA8888,
    RWB_IMAGE_YUV444,
    RWB_IMAGE_YUV422,
    RWB_IMAGE_YUV420,
    RWB_IMAGE_YUV411,
    RWB_IMAGE_YUV400,
    RWB_IMAGE_PACKET_UYVY422,
    RWB_IMAGE_PACKET_YUY2,
    RWB_IMAGE_PACKET_YVYU,
    RWB_IMAGE_NV21,
    RWB_IMAGE_YV12,

    RWB_IMAGE_RAW8=100,
    RWB_IMAGE_RAW10,
    RWB_IMAGE_EXT_RAW8,
    RWB_IMAGE_EXT_RAW10,
    RWB_IMAGE_JPEG=200
} MTK_RWB_IMAGE_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    RWB_FEATURE_BEGIN,         // minimum of feature id
    RWB_FEATURE_ADD_IMAGE,     // feature id to add image information
    RWB_FEATURE_GET_RESULT,    // feature id to get result
    RWB_FEATURE_GET_LOG,       // feature id to get debugging information
    RWB_FEATURE_GET_WORKBUF_SIZE,// feature id to query buffer size
    RWB_FEATURE_SET_WORKBUF_ADDR,  // feature id to set working buffer address
    RWB_FEATURE_MAX            // maximum of feature id
}   RWB_FEATURE_ENUM;

//GPU tuning
typedef struct GpuTune
{
    MUINT32 GLESVersion;
    MUINT32 CLVersion;
    MUINT32 CL_CPU; //1: yes
}GpuTuning;

typedef struct
{
    MINT32 DeGamma[256*3];
    MUINT8 Gamma[4096*3];
    MINT32 CCM[3*3];
}RWBTune_T;

//RWB tuning
typedef struct RWBTune
{
        MINT32*                                DeGammaTableAddr; //256R->256G->256B
        MUINT8*                                GammaTableAddr;   //4096R->4096G->4096B
        MINT32*                                CCMTableAddr;            //Row major 3x3 matrix
        MINT32                                OFF_GAMMA;                 //1: disable gamma correction

    RWBTune(RWBTune_T& rwb)
    : DeGammaTableAddr(&rwb.DeGamma[0])
       , GammaTableAddr(&rwb.Gamma[0])
       , CCMTableAddr(&rwb.CCM[0])
       , OFF_GAMMA(0)
       {}
}RWBTuning;



typedef struct RWBImageInfoStruct
{
        MUINT8*                                    ImgAddr[RWB_MAX_IMG_NUM];      // input image address array, (cpu)
    MUINT32                 ImgNum;                     // input image number
    MTK_RWB_IMAGE_ENUM      InputImgFmt;                // input image format
        MTK_RWB_IMAGE_ENUM      OutputImgFmt;                  // output image format
    MUINT32                 InputWidth;                 // input image width
    MUINT32                 InputHeight;                // input image height
    MUINT32                 OutputWidth;                // image result width
    MUINT32                 OutputHeight;               // image result height

        MUINT8*                                    WorkingBuffAddr;               // Working buffer start address

        void*                                     SrcGraphicBuffer;                        // input GB
        void*                                        DstGraphicBuffer;                        // output GB
        MUINT16                 GB_Width;                      // input GB width
    MUINT16                 GB_Height;                     // input GB height
        MINT32                                     InputGBNum;                                    //Ring buffer size
        MINT32                                     OutputGBNum;                                //Ring buffer size

        GpuTuning* pTuningParaGPU;
        RWBTuning* pTuningParaRWB;
        //MUINT32 Features;                                                                    // Current feature combination
        //MUINT32 feature_order[MTK_MAX_FEATURE_NUM];

}RWBImageInfo;

// RWB_FEATURE_GET_RESULT
// Input    : NONE
// Output   : RWBResultInfo
struct RWBResultInfo
{
    MUINT32                 Width;                      // output image width
    MUINT32                 Height;                     // output image hieght
    MINT32                  ElapseTime[3];              // record execution time
    CORE_ERRCODE_ENUM       RetCode;                    // returned status
};

/*******************************************************************************
*
********************************************************************************/
class MTKRWB {
public:
    static MTKRWB* createInstance(DrvRWBObject_e);
    virtual void   destroyInstance(MTKRWB* obj) = 0;

    virtual ~MTKRWB();
    // Process Control
    virtual MRESULT RWBInit(MUINT32 *InitInData, MUINT32 *InitOutData);    // Env/Cb setting
    virtual MRESULT RWBMain(void);                                         // START
    virtual MRESULT RWBReset(void);                                        // RESET

    // Feature Control
    virtual MRESULT RWBFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppRWBTmp : public MTKRWB {
public:
    //
    static MTKRWB* getInstance();
    virtual void destroyInstance(MTKRWB* obj);
    //
    AppRWBTmp() {};
    virtual ~AppRWBTmp() {};
};

#endif

