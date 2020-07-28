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

#ifndef _MTK_AIDEPTH_H
#define _MTK_AIDEPTH_H

#include "MTKAIDepthType.h"
#include "MTKAIDepthErrCode.h"

typedef enum DRVAIDepthObject_s {
    DRV_AIDEPTH_OBJ_NONE = 0,
    DRV_AIDEPTH_OBJ_SW,
    DRV_AIDEPTH_OBJ_UNKNOWN = 0xFF,
} DrvAIDepthObject_e;

//#define AIDEPTH_RUN_MDLA
#define AIDEPTH_TIME_PROF
#define AIDEPTH_MAX_CORE_NUM (1)
#define AIDEPTH_CUSTOM_PARAM
/*****************************************************************************
    AIDEPTH Define and State Machine
******************************************************************************/

/*****************************************************************************
    Process Control
******************************************************************************/
typedef enum AIDEPTH_STATE_ENUM
{
    AIDEPTH_STATE_STANDBY=0,            // After Create Obj or Reset
    AIDEPTH_STATE_INIT,                 // After Called AIDEPTHInit
    AIDEPTH_STATE_PREPROCESS,           // After Called AIDEPTHMain
    AIDEPTH_STATE_1ST_MODEL,            // In AIDEPTHMain, start to generate depth map
	AIDEPTH_STATE_MIDDLEPROCESS,        // Warp (optional)
	AIDEPTH_STATE_2ND_MODEL,            // In AIDEPTHMain, 2nd model (optional)
    AIDEPTH_STATE_POSTPROCESS,          // In AIDEPTHMain, start to post process
    AIDEPTH_STATE_PROCESS_DONE,         // After Finish AIDEPTHMain
} AIDEPTH_STATE_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    AIDEPTH_FEATURE_BEGIN,              // minimum of feature id
	AIDEPTH_FEATURE_ADD_IMGS,
    AIDEPTH_FEATURE_SET_PARAM,
    AIDEPTH_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    AIDEPTH_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    AIDEPTH_FEATURE_GET_RESULT,         // feature id to get result
    AIDEPTH_FEATURE_GET_LOG,            // feature id to get debugging information
	AIDEPTH_FEATURE_GET_VERSION,

    #ifdef AIDEPTH_ABORT_SUPPORT
    AIDEPTH_FEATURE_SET_ABORT,
    AIDEPTH_FEATURE_SET_ABORT_POS,
    AIDEPTH_FEATURE_GET_ABORT_INFO,
    #endif
    AIDEPTH_FEATURE_MAX                 // maximum of feature id
}   AIDEPTH_FEATURE_ENUM;

typedef enum
{
    AIDEPTH_FEATURE_9_16,              // width:height = 9:16
	AIDEPTH_FEATURE_4_3,              // width:height = 3:4
    //AIDEPTH_FEATURE_16_9,              //width:height = 16:9
    AIDEPTH_MODEL_ALL                   // list all model
}   AIDEPTH_MODEL_TYPE;


typedef struct AIDepthParam
{
    MINT32 ISOValue;
    MINT32 ExposureTime;
    char* prefix;
	
}AIDepthParam;
#ifdef AIDEPTH_CUSTOM_PARAM
typedef struct AIDepthCustomParam
{
    char* key;
    MINT32 value;
}AIDepthCustomParam;
#endif
typedef struct AIDepthTuningInfo
{
    MUINT32 CoreNumber;                 // valid value = {1 ~ AIDEPTH_MAX_CORE_NUM}
	//for pre-process
	MINT32 imgWidth;
	MINT32 imgHeight;
	MINT32 flipFlag;
	
	//for middle process
	MINT32 warpFlag;

    //for post process
    MINT32 imgFinalWidth;
    MINT32 dispGain;
    MINT32 convergenceOffset;

#ifdef AIDEPTH_CUSTOM_PARAM
    MUINT32 NumOfParam;
    AIDepthCustomParam* params;
#endif

} AIDepthTuningInfo;

typedef struct AIDepthInitInfo
{
    MUINT8*                 WorkingBuffAddr;                // Working buffer start address
    MUINT32                 WorkingBuffSize;                // Working buffer size
    AIDepthTuningInfo       *pTuningInfo;                   // Tuning info
} AIDepthInitInfo;

typedef struct AIDepthModelInfo
{

	MUINT32                 modelIdx;
    MUINT32                 modelWidth;
    MUINT32                 modelHeight;
	
	MUINT32                 modelNum;
	MUINT32*                 modelListW;
	MUINT32*                 modelListH;

} AIDepthModelInfo;

typedef struct AIDepthImageInfo
{
    MUINT32                     Width;                          // (depth)input image width
    MUINT32                     Height;                         // (depth)input image height
	MUINT32                     Stride;                         // (depth)input image stride
    MUINT8*                     ImgLAddr;   // (depth)input image address array
	MUINT8*                     ImgRAddr;   // (depth)input image address array

} AIDepthImageInfo;

// AIDEPTH_FEATURE_GET_RESULT
// Input    : NONE
// Output   : AIDepthResultInfo
typedef struct AIDepthResultInfo
{
    MUINT32     DepthImageWidth;                   // output image width for AIDepth
    MUINT32     DepthImageHeight;                  // output image height for AIDepth
    MUINT8*     DepthImageAddr;          // depth image address

    MRESULT     RetCode;                        // returned status
}AIDepthResultInfo;

/*******************************************************************************
*
********************************************************************************/
class MTKAIDepth {
public:
    static MTKAIDepth* createInstance(DrvAIDepthObject_e eobject);
    virtual void   destroyInstance(MTKAIDepth* obj) = 0;

    virtual ~MTKAIDepth(){}
    // Process Control
    virtual MRESULT AIDepthInit(MUINT32 *InitInData, MUINT32 *InitOutData);    // Env/Cb setting
    virtual MRESULT AIDepthMain(void);                                         // START
    virtual MRESULT AIDepthReset(void);                                        // RESET

    // Feature Control
    virtual MRESULT AIDepthFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
	virtual MRESULT AIDepthGetModelInfo(MUINT32 FeatureID, void* pParaOut);
private:

};

class AppAIDepthTmp : public MTKAIDepth {
public:
    //
    static MTKAIDepth* getInstance();
    virtual void destroyInstance(){};
    //
    AppAIDepthTmp() {};
    virtual ~AppAIDepthTmp() {};
};

#endif

