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

#ifndef _MTK_NR_H
#define _MTK_NR_H

#include "MTKNRType.h"
#include "MTKNRErrCode.h"

typedef enum DRVNRObject_s {
    DRV_NR_OBJ_NONE = 0,
    DRV_NR_OBJ_SW2,
    DRV_NR_OBJ_SW2_VPU,
    DRV_NR_OBJ_SW2_HIGH_PERF,
    DRV_NR_OBJ_SW2_LOW_POWER,
    DRV_NR_OBJ_UNKNOWN = 0xFF,
} DrvNRObject_e;

#define NR_TIME_PROF

/*****************************************************************************
    NR Define and State Machine
******************************************************************************/
#define NR_BUFFER_SIZE(w, h) MAX(NR2_BUFFER_SIZE(w, h), 0)

#define NR2_BUFFER_SCALE         (1.80f)
#define NR2_BUFFER_SIZE(w, h)    ((w) * (h) * NR2_BUFFER_SCALE + (w)*4 + 1024*2)

#define NR_MULTI_CORE_OPT   (1)
#define PARTITION_NUM (64)
#define PARTITION_NUM_V (8)
#define NR_MAX_BUFFER_COUNT     (10)
/*****************************************************************************
    Performance Control
******************************************************************************/
#define NR_MAX_GEAR_NUM     (10)
#define NR_MAX_CLUSTER_NUM  (2)
static const MUINT32 NRPerfGearOption[NR_MAX_GEAR_NUM][6] =
{
    // LL, L, B
    {4, 3000000, 4, 3000000, 0, 0},
    {4, 1508000, 4, 2197000, 0, 0},
    {4, 1378000, 4, 2067000, 0, 0},
    {4, 1391000, 4, 1794000, 0, 0},   // reserved
    {4, 1391000, 4, 1794000, 0, 0},   // reserved
    {4, 1391000, 4, 1794000, 0, 0},   // reserved
    {4, 1391000, 4, 1794000, 0, 0},   // reserved
    {4, 1391000, 4, 1794000, 0, 0},   // reserved
    {4, 1391000, 4, 1794000, 0, 0},   // reserved
    {4, 1391000, 4, 1794000, 0, 0}    // reserved
};


class MTKNRGPU {
public:
    static MTKNRGPU* createInstance(DrvNRObject_e eobject);
    virtual void   destroyInstance(MTKNRGPU* obj) = 0;
    virtual ~MTKNRGPU(){}                                      // RESET

    // GPU control
    virtual MRESULT NRGPUInit(void*);
    virtual MRESULT NRGPUReset();
private:

};


/*****************************************************************************
    Process Control
******************************************************************************/
typedef enum NR_STATE_ENUM
{
    NR_STATE_STANDBY=0,            // After Create Obj or Reset
    NR_STATE_INIT,                 // After Called NRInit
    NR_STATE_PROCESS,              // After Called NRMain
    NR_STATE_PROCESS_DONE,         // After Finish NRMain
} NR_STATE_ENUM;

typedef enum MTK_NR_IMAGE_FMT_ENUM
{
    NR_IMAGE_Y8 = (1<<0),
    NR_IMAGE_YUV420 = (1<<1),
    NR_IMAGE_YV12 = (1<<2),
} MTK_NR_IMAGE_FMT_ENUM;

typedef enum NR_IO_ENUM
{
    NR_INPUT,
    NR_OUTPUT,
    NR_IO_NUM
} NR_IO_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    NR_FEATURE_BEGIN,              // minimum of feature id
    NR_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    NR_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    NR_FEATURE_GET_INPUT_FMT,      // feature id to query input buffer format
    NR_FEATURE_GET_VERSION,        // feature id to get Version
    NR_FEATURE_MAX                 // maximum of feature id
}   NR_FEATURE_ENUM;

typedef struct NR2TuningInfo
{
    MINT32 NR_K;
    MINT32 NR_S;
    MINT32 NR_SD;
    MINT32 NR_BLD_W;
    MINT32 NR_BLD_TH;
    MINT32 NR_SMTH;
    MINT32 NR_NTRL_TH_1_Y;
    MINT32 NR_NTRL_TH_2_Y;
    MINT32 NR_NTRL_TH_1_UV;
    MINT32 NR_NTRL_TH_2_UV;

    // constructor
    NR2TuningInfo()
                        : NR_NTRL_TH_1_Y(255)
    {
    }
} NR2TuningInfo;

typedef struct HFGTuningInfo
{
    MINT32 HFG_ENABLE;
    MINT32 HFG_GSD;
    MINT32 HFG_SD0;
    MINT32 HFG_SD1;
    MINT32 HFG_SD2;
    MINT32 HFG_TX_S;
    MINT32 HFG_LCE_LINK_EN;
    MINT32 HFG_LUMA_CPX1;
    MINT32 HFG_LUMA_CPX2;
    MINT32 HFG_LUMA_CPX3;
    MINT32 HFG_LUMA_CPY0;
    MINT32 HFG_LUMA_CPY1;
    MINT32 HFG_LUMA_CPY2;
    MINT32 HFG_LUMA_CPY3;
    MINT32 HFG_LUMA_SP0;
    MINT32 HFG_LUMA_SP1;
    MINT32 HFG_LUMA_SP2;
    MINT32 HFG_LUMA_SP3;
} HFGTuningInfo;

typedef struct CCRTuningInfo
{
    MINT32 CCR_ENABLE;
    MINT32 CCR_CEN_U;
    MINT32 CCR_CEN_V;
    MINT32 CCR_Y_CPX1;
    MINT32 CCR_Y_CPX2;
    MINT32 CCR_Y_CPY1;
    MINT32 CCR_Y_SP1;
    MINT32 CCR_UV_X1;
    MINT32 CCR_UV_X2;
    MINT32 CCR_UV_X3;
    MINT32 CCR_UV_GAIN1;
    MINT32 CCR_UV_GAIN2;
    MINT32 CCR_UV_GAIN_SP1;
    MINT32 CCR_UV_GAIN_SP2;
    MINT32 CCR_Y_CPX3;
    MINT32 CCR_Y_CPY0;
    MINT32 CCR_Y_CPY2;
    MINT32 CCR_Y_SP0;
    MINT32 CCR_Y_SP2;
    MINT32 CCR_UV_GAIN_MODE;
    MINT32 CCR_MODE;
    MINT32 CCR_OR_MODE;
    MINT32 CCR_HUE_X1;
    MINT32 CCR_HUE_X2;
    MINT32 CCR_HUE_X3;
    MINT32 CCR_HUE_X4;
    MINT32 CCR_HUE_SP1;
    MINT32 CCR_HUE_SP2;
    MINT32 CCR_HUE_GAIN1;
    MINT32 CCR_HUE_GAIN2;
} CCRTuningInfo;

typedef enum NR_MEMORY_TYPE
{
    NR_MEMORY_ION,
    NR_MEMORY_GRALLOC_Y8,
    NR_MEMORY_GRALLOC_YV12,
}NR_MEMORY_TYPE;

typedef struct NRBuffer
{
    NR_MEMORY_TYPE MemType;    // out:
    MTK_NR_IMAGE_FMT_ENUM ImgFmt;  // out:
    MUINT32 Size;  // out: ION
    MUINT32 Width; // out: GRALLOC
    MUINT32 Height;// out: GRALLOC
    MUINT32 BufferNum; // out:  plane number
	MINT32 BuffFD[4]; // in: ION buff handle
    void* pBuff[4];// in: Buff address
}NRBuffer;

typedef struct NRInitInfo
{
    MUINT32     CoreNumber;                 // valid value = {1 ~ NR_MAX_CORE_NO}
    MUINT32     NumOfExecution; // profiling purpose
    NR2TuningInfo *pNR2TuningInfo;
    CCRTuningInfo *pCCRTuningInfo;
    HFGTuningInfo *pHFGTuningInfo;
} NRInitInfo;

typedef struct NRWorkingBufferInfo
{
    MUINT32 WorkingBufferNum;       // out:
    NRBuffer WorkingBuffer[NR_MAX_BUFFER_COUNT]; //inout
    MUINT32 Width;                     // input: image width
    MUINT32 Height;                    // input: image height
}NRWorkingBufferInfo;

// NR_STATE_GEN_DEPTH & NR_STATE_DEFOCUS
// Input    : NRImageInfo
// Output   : NONE
typedef struct NRImageInfo
{
    MUINT32 ImgSupportFmt; // out;
    MTK_NR_IMAGE_FMT_ENUM ImgFmt; // in:
    MUINT32 Width;     // in: image width
    MUINT32 Height;    // in: image height
    MUINT32 ImgNum;    // out: plane number
    NR_MEMORY_TYPE MemType[4]; // out: each buffer attribute
	MINT32 ImgFD[4];          // in: ION buff handle
    void* pImg[4];         // in: buffer pointer
    MTKNRGPU* Instance;    // in: GPU handle
} NRImageInfo;

struct NRVerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
	int MainVer;
	int SubVer;
	int PatchVer;
} ;                      // get Version by FeatureCtrl(GET_VERSION)


/*******************************************************************************
*
********************************************************************************/
class MTKNR {
public:
    static MTKNR* createInstance(DrvNRObject_e eobject);
    virtual void   destroyInstance(MTKNR* obj) = 0;

    virtual ~MTKNR(){}
    // Process Control
    virtual MRESULT NRInit(NRInitInfo *InitInData, MUINT32 *InitOutData);    // Env/Cb setting
    virtual MRESULT NRMain(NRImageInfo* InputData, MUINT32 *OutputData);  // START
    virtual MRESULT NRReset(void);                                        // RESET

    // Feature Control
    virtual MRESULT NRFeatureCtrl(NR_FEATURE_ENUM FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppNRTmp : public MTKNR {
public:
    //
    static MTKNR* getInstance();
    virtual void destroyInstance(MTKNR* obj) = 0;
    //
    AppNRTmp() {};
    virtual ~AppNRTmp() {};
};
#endif

