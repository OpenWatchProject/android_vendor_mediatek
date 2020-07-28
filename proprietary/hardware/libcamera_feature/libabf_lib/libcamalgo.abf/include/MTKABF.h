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

#ifndef _MTK_ABF_H_
#define _MTK_ABF_H_

#include "MTKABFType.h"
#include "MTKABFErrCode.h"

typedef enum DRVABFObject_s {
    DRV_ABF_OBJ_NONE = 0,
    DRV_ABF_OBJ_IMAGE,
    DRV_ABF_OBJ_VIDEO,
    DRV_ABF_OBJ_UNKNOWN = 0xFF,
} DrvABFObject_e;

#define ABF_TIME_PROF

/*****************************************************************************
    ABF Define and State Machine
******************************************************************************/
#define ABF_MAX_IMG_NUM         (25)
#define ABF_BUFFER_UNSET        (0xFFFF)
#define ABF_BUFFER_SCALE        (2.5f)
#define ABF_BUFFER_SIZE(w, h)   ((w) * (h) * ABF_BUFFER_SCALE)
#define ABF_C_OPT               (1)
#define ABF_NEON_OPT            (1)
#define ABF_NEON_OPT2           (1)     // optimization without flexibility
#define ABF_MULTI_CORE_OPT      (1)
#define ABF_MAX_CORE_NO         (10)
#define ABF_PARTITION_NUM       (32)

/*****************************************************************************
    Performance Control
******************************************************************************/
#define ABF_MAX_GEAR_NUM    (10)
#define ABF_MAX_CLUSTER_NUM (3)
#define ABF_MAX_GROUP_NUM   (ABF_MAX_CLUSTER_NUM * (2))
static const MUINT32 ABFPerfGearOption[ABF_MAX_GEAR_NUM][ABF_MAX_GROUP_NUM] =
{
    // LL, L, B
    {4, 2000000, 4, 2500000, 2, 2600000},
    {4, 2000000, 4, 2500000, 2, 2600000},
    {4, 2000000, 4, 2500000, 2, 2600000},
    {4, 1391000, 4, 1794000, 0, 2288000},   // reserved
    {4, 1391000, 4, 1794000, 0, 2288000},   // reserved
    {4, 1391000, 4, 1794000, 0, 2288000},   // reserved
    {4, 1391000, 4, 1794000, 0, 2288000},   // reserved
    {4, 1391000, 4, 1794000, 0, 2288000},   // reserved
    {4, 1391000, 4, 1794000, 0, 2288000},   // reserved
    {4, 1391000, 4, 1794000, 0, 2288000}    // reserved
};

/*****************************************************************************
    Process Control
******************************************************************************/
typedef enum ABF_STATE_ENUM
{
    ABF_STATE_STANDBY=0,            // After Create Obj or Reset
    ABF_STATE_INIT,                 // After Called ABFInit
    ABF_STATE_PROCESS,              // After Called ABFMain
    ABF_STATE_PROCESS_DONE,         // After Finish ABFMain
} ABF_STATE_ENUM;

typedef enum MTK_ABF_IMAGE_FMT_ENUM
{
    ABF_IMAGE_RGB565=1,
    ABF_IMAGE_BGR565,
    ABF_IMAGE_RGB888,
    ABF_IMAGE_BGR888,
    ABF_IMAGE_ARGB888,
    ABF_IMAGE_ABGR888,
    ABF_IMAGE_BGRA8888,
    ABF_IMAGE_RGBA8888,
    ABF_IMAGE_YUV444,
    ABF_IMAGE_YUV422,
    ABF_IMAGE_YUV420,
    ABF_IMAGE_YUV411,
    ABF_IMAGE_YUV400,
    ABF_IMAGE_PACKET_UYVY422,
    ABF_IMAGE_PACKET_YUY2,
    ABF_IMAGE_PACKET_YVYU,
    ABF_IMAGE_NV21,
    ABF_IMAGE_YV12,

    ABF_IMAGE_RAW8=100,
    ABF_IMAGE_RAW10,
    ABF_IMAGE_EXT_RAW8,
    ABF_IMAGE_EXT_RAW10,
    ABF_IMAGE_JPEG=200
} MTK_ABF_IMAGE_FMT_ENUM;

/*****************************************************************************
    Feature Control Enum and Structure
******************************************************************************/
typedef enum
{
    ABF_FEATURE_BEGIN,              // minimum of feature id
    ABF_FEATURE_CHECK_ENABLE,       // feature id to check whether abf is enabled
    ABF_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    ABF_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    ABF_FEATURE_ADD_IMG,            // feature id to set source image info
    ABF_FEATURE_SET_RESULT,         // feature id to set result image info
    ABF_FEATURE_GET_RESULT,         // feature id to get result
    ABF_FEATURE_GET_LOG,            // feature id to get debugging information
    ABF_FEATURE_GET_VERSION,        // feature id to get current version
    ABF_FEATURE_MAX                 // maximum of feature id
}   ABF_FEATURE_ENUM;

typedef struct ABFTuningInfo
{
    // tuning parameters
    MINT32 ABF_STHRE_R;
    MINT32 ABF_STHRE_G;
    MINT32 ABF_STHRE_B;
    MINT32 ABF_NSR_R1;
    MINT32 ABF_NSR_R2;
    MINT32 ABF_R1;
    MINT32 ABF_R2;
    MINT32 ABF_Y0;
    MINT32 ABF_Y1;
    MINT32 ABF_Y2;
    MINT32 ABF_Y3;
    MINT32 ABF_CX0;
    MINT32 ABF_CX1;
    MINT32 ABF_CX2;
    MINT32 ABF_CX3;
    MINT32 ABF_CY0;
    MINT32 ABF_CY1;
    MINT32 ABF_CY2;
    MINT32 ABF_CY3;
    MINT32 ABF_CX_SP0;
    MINT32 ABF_CX_SP1;
    MINT32 ABF_CY_SP0;
    MINT32 ABF_CY_SP1;
    MINT32 ABF_Y_SP0;
    MINT32 ABF_Y_SP1;
    MINT32 ABF_CX_IDX;
    MINT32 ABF_CY_IDX;
    MINT32 ABF_BF_WIDTH;
    MINT32 ABF_BF_MAX_DIS;
    MINT32 ABF_BF_U_OFST;
    MINT32 ABF_NR_LV;
    MINT32 ABF_THRE_LO;
    MINT32 ABF_THRE_HI;
    MINT32 ABF_CHECK;
    MINT32 ABF_ON;
} ABFTuningInfo;

typedef enum ABF_PERF_GEAR_ENUM
{
    ABF_PERF_GEAR_PERF_FIRST,    // fastest
    ABF_PERF_GEAR_CUSTOM_0,      // trade-off between fastest & slowest
    ABF_PERF_GEAR_POWER_FIRST,   // slowest
} ABF_PERF_FEAR_ENUM;

typedef struct ABFPerfGear
{
    MBOOL enable;
    ABF_PERF_GEAR_ENUM option;
} ABFPerfGear;

typedef struct ABFInitInfo
{
    void*               pWorkingBuff;       // Working buffer start address
    MUINT32             WorkingBuffSize;
    MUINT32             CoreNumber;         // valid value = {1 ~ ABF_MAX_CORE_NO}
    MUINT32             NumOfExecution;     // profiling purpose
    ABFTuningInfo       *pTuningInfo;
    MUINT32             ForcedFreq;
} ABFInitInfo;

// Input    : ABFImageInfo
// Output   : NONE
typedef struct ABFImageInfo
{
    void*                       pImg[ABF_MAX_IMG_NUM];   // input image address array
    MUINT32                     ImgNum;                  // input image number
    MTK_ABF_IMAGE_FMT_ENUM      ImgFmt;                  // input image format
    MUINT32                     Width;                   // input image width
    MUINT32                     Height;                  // input image height

} ABFImageInfo;

// ABF_FEATURE_GET_RESULT
// Input    : NONE
// Output   : ABFResultInfo
typedef struct ABFResultInfo
{
    void*                   pImg[ABF_MAX_IMG_NUM];      // output mage array
    MUINT32                 ImgNum;                     // output image number
    MTK_ABF_IMAGE_FMT_ENUM  ImgFmt;                     // output image format
    MUINT32                 Width;                      // output image width
    MUINT32                 Height;                     // output image hieght
    MRESULT                 RetCode;                    // returned status
} ABFResultInfo;
/*******************************************************************************
*
********************************************************************************/
class MTKABF {
public:
    static MTKABF* createInstance(DrvABFObject_e eobject);
    virtual void   destroyInstance(MTKABF* obj) = 0;

    virtual ~MTKABF(){}
    // Process Control
    virtual MRESULT ABFInit(void *InitInData);  // Env/Cb setting
    virtual MRESULT ABFMain(void);              // START
    virtual MRESULT ABFReset(void);             // RESET

    // Feature Control
    virtual MRESULT ABFFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppABFTmp : public MTKABF {
public:
    //
    static MTKABF* getInstance();
    virtual void destroyInstance(MTKABF* obj) = 0;
    //
    AppABFTmp() {};
    virtual ~AppABFTmp() {};
};

#endif

