/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */

 /* MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_BWDN_H
#define _MTK_BWDN_H

#include "MTKBWDNType.h"
#include "MTKBWDNErrCode.h"

/*****************************************************************************
    Configuration
******************************************************************************/


#define BWDN_MULTI_CORE_OPT (1)
#define BWDN_MAX_BUFFER_COUNT (15)
#define BWDN_MAX_THREAD  (8)

#define BWDN_MAX_FUNC_SPLIT_NUM    (4)
#define BWDN_MAX_HEIGHT_SPLIT_NUM  (64)


#define BWDN_IMG_PADDING_SIZE (16)
#define BWDN_SHADING_PADDING_SIZE (1)

#define WONLY
//#define RGB48OUT
#define LOW_FREQ_WEIGHT (900)

#if defined(__ANDROID__) || defined(ANDROID)
#define BWDN_NEON_OPT
#define GPU_HW
//#define VPU_HW
#define BayerW8 (2)
//#define BWDN_ENABLE_DYNAMIC_PRE_SERVE
#else
#define BayerW8 (1)
#endif

// HW version
#ifdef GPU_HW
//#define _IMG_GPU_
#endif

/*****************************************************************************
    Process Control
******************************************************************************/
typedef int (*PerfCB)(int);

typedef enum DRVBWDNObject_s {
    DRV_BWDN_OBJ_NONE = 0,
    DRV_BWDN_OBJ_SW,
    DRV_BWDN_OBJ_UNKNOWN = 0xFF,
} DrvBWDNObject_e;

typedef enum
{
    BWDN_FEATURE_BEGIN,              // minimum of feature id
    BWDN_FEATURE_SET_PARAMS,         // feature id to set images
    BWDN_FEATURE_SET_IMAGES,         // feature id to set images
    BWDN_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    BWDN_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    BWDN_FEATURE_GET_RESULT,         // feature id to get result
    BWDN_FEATURE_GET_LOG,            // feature id to get debugging information
} BWDN_FEATURE_ENUM;

typedef enum BWDN_MEMORY_TYPE{
    BWDN_MEMORY_VA,
    BWDN_MEMORY_GRALLOC_Y8,
    BWDN_MEMORY_GRALLOC_IMG_Y16,
    BWDN_MEMORY_GRALLOC_IMG_FLOAT,
    BWDN_MEMORY_GRALLOC_YV12
}BWDN_MEMORY_TYPE;

typedef enum BWDN_IMAGE_FMT_ENUM
{
    BWDN_IMAGE_DATA8,
    BWDN_IMAGE_DATA16,
    BWDN_IMAGE_DATA32,

    BWDN_IMAGE_RAW8,
    BWDN_IMAGE_RAW10,
    BWDN_IMAGE_EXT_RAW8,
    BWDN_IMAGE_EXT_RAW10
} BWDN_IMAGE_FMT_ENUM;

typedef enum BWDN_PROC_ENUM
{
    BWDN_PROC1 = 0,
    BWDN_PROC2,
    BWDN_UNKNOWN_PROC,
} BWDN_PROC_ENUM;

typedef enum BWDN_ACC_ENUM
{
    BWDN_DEFAULT = (1<<0),
    BWDN_USE_C = (1<<1),
    BWDN_ADD_G = (1<<2),
    BWDN_ADD_V = (1<<3)
} BWDN_ACC_ENUM;


typedef struct BWDNInitParamsIF {
    MINT32 i4CoreNum;    // ToDo: change to CPU Core Number
    MINT32 i4SplitSize;  // ToDo: decide by internal code
    //MBOOL bIsRotate;   // ToDo: remove
    MFLOAT fConvMat[3];
    BWDN_ACC_ENUM rAcc;
    PerfCB pfPerfCB;
    MUINT8* pi1DbgPath;
} BWDNInitParamsIF;

/*
typedef struct BWDNWBuffer{
    BWDN_MEMORY_TYPE eMemType;    // out:
    BWDN_IMAGE_FMT_ENUM eImgFmt;  // out:
    MINT32 i4Size;        // out: ION
    MINT32 i4Width;       // out: GRALLOC
    MINT32 i4Height;      // out: GRALLOC
    MINT32 i4BufferNum;   // out:  plane number
    void* pvBuff[4];       //in: Buff address
} BWDNWBuffer;*/

typedef struct BWDNImage{
    BWDN_MEMORY_TYPE eMemType;    // in:
    BWDN_IMAGE_FMT_ENUM eImgFmt;  // in:
    MINT32 i4Width;       // in: source width
    MINT32 i4Height;      // in: source height
    MINT32 i4Offset;      // in: source data offset (bytes)
    MINT32 i4Pitch;       // in: buffer pitch
    MINT32 i4Size;        // in: buffser size
    MINT32 i4PlaneNum;      // in:  plane number
    void* pvPlane[4];       //in: graphic buffer pointer
} BWDNImage;
typedef BWDNImage BWDNWBuffer;


typedef struct BWDNWorkingBufferInfo{
    MINT32 i4WorkingBufferNum;          // out:
    BWDNWBuffer rWorkingBuffer[BWDN_MAX_BUFFER_COUNT]; //inout
    // information for working buffer
    MINT32 i4PadWidth;             // input: pad image width
    MINT32 i4PadHeight;            // input: pad image height
} BWDNWorkingBufferInfo;

typedef struct BWDNParamsIF{
    //ISP Info
    MINT32 rOBOffsetBayer[4];
    MINT32 rOBOffsetMono[4];
    MINT32 i4SensorGainBayer;
    MINT32 i4SensorGainMono;
    MINT32 i4IspGainBayer;
    MINT32 i4IspGainMono;
    MINT32 rPreGainBayer[3];
    //MINT32 rPreGainMono[3];
    //MINT32 i4IsRotate;
    MINT32 i4BayerOrder;
    MINT32 i4RA;
    MINT32 i4BitMode;

    //MINT32 i4OffsetX;
    //MINT32 i4OffsetY;

    //ISO dependent
    MINT32 i4BWSingleRange;
    MINT32 i4BWOccRange;
    MINT32 i4BWRange;
    MINT32 i4BWKernel;
    MINT32 i4BRange;
    MINT32 i4BKernel;
    MINT32 i4WRange;
    MINT32 i4WKernel;

    //sensor dependent
    MINT32 i4VSL;
    MINT32 i4VOFT;
    MINT32 i4VGAIN;

    //information from N3D
    MFLOAT rTrans[9];
    MINT32 rDPadding[2];

    //Performance tuning
    MINT32 i4FastPreprocess;
    MINT32 i4FullSkipStep;
    MINT32 i4DblkRto;
    MINT32 i4DblkTH;
    MINT32 i4QSch;

    //  For test
    MBOOL bUnPack;  // default set  True
    MINT32 i4OutPackSize;

    #ifdef PC_UTIL // ToDo
    bool grayOnly;
    bool histEqual;
    bool unpack;
    #endif
} BWDNParamsIF;

typedef struct BWDNImageInfo{
    MINT32 i4DepthDSH;
    MINT32 i4DepthDSV;

    // Input Data
    BWDNImage* prMonoProcessedRaw;
    BWDNImage* prBayerProcessedRaw;
    BWDNImage* prBayerW;

    // depth
    BWDNImage* prDepth;

    // Full Shading Gain
    BWDNImage* prBayerGain;
    BWDNImage* prMonoGain;

    // RGB denoise output after BWDN
    BWDNImage* prOutput;            // Width x Height x 4 RGB 12-bit image
} BWDNImageInfo;


/*******************************************************************************
*
********************************************************************************/
class MTKBWDN {
public:
    static MTKBWDN* createInstance(DrvBWDNObject_e eobject);
    virtual void   destroyInstance(MTKBWDN* obj) = 0;

    virtual ~MTKBWDN(){}
    // Process Control
    virtual MRESULT BWDNInit(void *InitInData, void *InitOutData);
    virtual MRESULT BWDNMain(BWDN_PROC_ENUM eProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT BWDNReset(void);

    // Feature Control
    virtual MRESULT BWDNFeatureCtrl(BWDN_FEATURE_ENUM eFeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppBWDNTmp : public MTKBWDN {
public:
    //
    static MTKBWDN* getInstance();
    virtual void destroyInstance(MTKBWDN* obj) = 0;
    //
    AppBWDNTmp() {};
    virtual ~AppBWDNTmp() {};
};

#endif

