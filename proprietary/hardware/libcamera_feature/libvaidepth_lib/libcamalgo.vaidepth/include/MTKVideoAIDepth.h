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

#ifndef _MTK_VIDEOAIDEPTH_H
#define _MTK_VIDEOAIDEPTH_H

#include "MTKVideoAIDepthType.h"
#include "MTKVideoAIDepthErrCode.h"

typedef enum DRVVideoAIDepthObject_s {
    DRV_VIDEOAIDEPTH_OBJ_NONE = 0,
    DRV_VIDEOAIDEPTH_OBJ_SW,
    DRV_VIDEOAIDEPTH_OBJ_UNKNOWN = 0xFF,
} DrvVideoAIDepthObject_e;

//#define VideoAIDEPTH_RUN_MDLA
#define VIDEOAIDEPTH_TIME_PROF
#define VIDEOAIDEPTH_MAX_CORE_NUM (1)
#define VIDEOAIDEPTH_CUSTOM_PARAM
/*****************************************************************************
    VideoAIDEPTH Define and State Machine
******************************************************************************/

/*****************************************************************************
    Process Control
******************************************************************************/
typedef enum VIDEOAIDEPTH_STATE_ENUM
{
    VIDEOAIDEPTH_STATE_STANDBY=0,            // After Create Obj or Reset
    VIDEOAIDEPTH_STATE_INIT,                 // After Called AIDEPTHInit
    VIDEOAIDEPTH_STATE_PREPROCESS,           // After Called AIDEPTHMain
    VIDEOAIDEPTH_STATE_1ST_MODEL,            // In AIDEPTHMain, start to generate depth map
    VIDEOAIDEPTH_STATE_MIDDLEPROCESS,        // Warp (optional)
    VIDEOAIDEPTH_STATE_2ND_MODEL,            // In AIDEPTHMain, 2nd model (optional)
    VIDEOAIDEPTH_STATE_POSTPROCESS,          // In AIDEPTHMain, start to post process
    VIDEOAIDEPTH_STATE_PROCESS_DONE,         // After Finish AIDEPTHMain
} VIDEOAIDEPTH_STATE_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    VIDEOAIDEPTH_FEATURE_BEGIN,              // minimum of feature id
    VIDEOAIDEPTH_FEATURE_ADD_IMGS,
    VIDEOAIDEPTH_FEATURE_SET_PARAM,
    VIDEOAIDEPTH_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    VIDEOAIDEPTH_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    VIDEOAIDEPTH_FEATURE_GET_RESULT,         // feature id to get result
    VIDEOAIDEPTH_FEATURE_GET_LOG,            // feature id to get debugging information
    VIDEOAIDEPTH_FEATURE_GET_VERSION,

    #ifdef VIDEOAIDEPTH_ABORT_SUPPORT
    VIDEOAIDEPTH_FEATURE_SET_ABORT,
    VIDEOAIDEPTH_FEATURE_SET_ABORT_POS,
    VIDEOAIDEPTH_FEATURE_GET_ABORT_INFO,
    #endif
    VIDEOAIDEPTH_FEATURE_MAX                 // maximum of feature id
}   VIDEOAIDEPTH_FEATURE_ENUM;

typedef enum
{
    VIDEOAIDEPTH_FEATURE_9_16,              // width:height = 9:16
    VIDEOAIDEPTH_FEATURE_4_3,              // width:height = 4:3
    //VIDEOAIDEPTH_FEATURE_16_9,              //width:height = 16:9
    VIDEOAIDEPTH_MODEL_ALL                   // list all model
}   VIDEOAIDEPTH_MODEL_TYPE;


typedef struct VideoAIDepthParam
{
    MINT32 ISOValue;
    MINT32 ExposureTime;
    char* prefix;
    
}VideoAIDepthParam;
#ifdef VIDEOAIDEPTH_CUSTOM_PARAM
typedef struct VideoAIDepthCustomParam
{
    char* key;
    MINT32 value;
}VideoAIDepthCustomParam;
#endif
typedef struct VideoAIDepthTuningInfo
{
    MUINT32 CoreNumber;                 // valid value = {1 ~ AIDEPTH_MAX_CORE_NUM}
    //for pre-process
    MINT32 imgWidth;
    MINT32 imgHeight;
    MINT32 flipFlag;
    
    //for middle process 
    MINT32 warpFlag; // currently not used

    //for post process
    MINT32 imgFinalWidth;
    MINT32 dispGain;
    MINT32 convergenceOffset;

#ifdef VIDEOAIDEPTH_CUSTOM_PARAM
    MUINT32 NumOfParam;
    VideoAIDepthCustomParam* params;
#endif

} VideoAIDepthTuningInfo;

typedef struct VideoAIDepthBuffStruc
{
    MUINT32 Size; // buffer size
    MINT32 BuffFD; // buffer FD 
    void* prBuff; // Buff VA address
                  // constructor
    VideoAIDepthBuffStruc()
        : Size(0), BuffFD(-1), prBuff(NULL)
    {}
} VideoAIDepthBuffStruc;

typedef struct VideoAIDepthInitInfo
{
    VideoAIDepthBuffStruc          WorkingBuff[4];                // 0-1 for ion input , 2 for ion output, 3 for Final Output
    VideoAIDepthTuningInfo       *pTuningInfo;                   // Tuning info
} VideoAIDepthInitInfo;

typedef struct VideoAIDepthModelInfo
{

    MUINT32                 modelIdx;
    MUINT32                 modelWidth;
    MUINT32                 modelHeight;
    
    MUINT32                 modelNum;
    MUINT32*                 modelListW;
    MUINT32*                 modelListH;

} VideoAIDepthModelInfo;

typedef struct VideoAIDepthImageInfo
{
    MUINT32                     Width;                          // (depth)input image width
    MUINT32                     Height;                         // (depth)input image height
    MUINT32                     Stride;                         // (depth)input image stride
    MUINT8*                     ImgLAddr;   // (depth)input image address array
    MUINT8*                     ImgRAddr;    // (depth)input image address array
    MUINT8*                     Disparity;   // (depth)input image address array DPE – Disparity
    MUINT8*                     HoleMap;    // (depth)input image address array DPE – Hole Map

} VideoAIDepthImageInfo;

// VIDEOAIDEPTH_FEATURE_GET_RESULT
// Input    : NONE
// Output   : VideoAIDepthResultInfo
typedef struct VideoAIDepthResultInfo
{
    MUINT32     DepthImageWidth;                   // output image width for AIDepth
    MUINT32     DepthImageHeight;                  // output image height for AIDepth
    MUINT8*     DepthImageAddr;          // depth image address

    MRESULT     RetCode;                        // returned status
}VideoAIDepthResultInfo;

/*******************************************************************************
*
********************************************************************************/
class MTKVideoAIDepth {
public:
    static MTKVideoAIDepth* createInstance(DrvVideoAIDepthObject_e eobject);
    virtual void   destroyInstance(MTKVideoAIDepth* obj) = 0;

    virtual ~MTKVideoAIDepth(){}
    // Process Control
    virtual MRESULT VideoAIDepthInit(MUINT32 *InitInData, MUINT32 *InitOutData);    // Env/Cb setting
    virtual MRESULT VideoAIDepthMain(void);                                         // START
    virtual MRESULT VideoAIDepthReset(void);                                        // RESET

    // Feature Control
    virtual MRESULT VideoAIDepthFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
    virtual MRESULT VideoAIDepthGetModelInfo(MUINT32 FeatureID, void* pParaOut);
private:

};

class AppVideoAIDepthTmp : public MTKVideoAIDepth {
public:
    //
    static MTKVideoAIDepth* getInstance();
    virtual void destroyInstance(){};
    //
    AppVideoAIDepthTmp() {};
    virtual ~AppVideoAIDepthTmp() {};
};

#endif

