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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file lmv_hal_imp.h
*
* EIS Hal Implementation Header File
*
*/


#ifndef _LMV_HAL_IMP_H_
#define _LMV_HAL_IMP_H_

#include <Mutex.h>
#include <Condition.h>

#include <mtkcam/feature/eis/lmv_hal.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

using android::Mutex;
using android::Condition;


#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

#define MAX_MEMORY_SIZE (40)

#define TSRECORD_MAXSIZE    (108000)
#define EIS_ACCELEROMETER   (1)
#define EIS_GYROSCOPE          (2)
#define GYRO_DATA_PER_FRAME     (100)

typedef android::sp<IImageBuffer> spIImageBuffer;

typedef std::queue<spIImageBuffer>  vecSPIImageBuffer;

namespace NSCam{
namespace Utils{
class SensorProvider;
}
}

/**
  *@class EisHalImp
  *@brief Implementation of LMVHal class
*/
class LMVHalImp : public LMVHal
{

    template<const unsigned int> friend class LMVHalObj;
private:
    LMVHalImp(const LMVHalImp&);
    LMVHalImp &operator = (const LMVHalImp&);
    /**
         *@brief LMVHalImp constructor
       */
    LMVHalImp(const MUINT32 &aSensorIdx);

    /**
         *@brief LMVHalImp destructor
       */
    ~LMVHalImp() {}

public:

    /**
         *@brief Create EisHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EisHal object
       */
    static LMVHal *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy EisHal object
         *@param[in] userName : user name,i.e. who destroy EIS object
       */
    virtual MVOID DestroyInstance(char const *userName);

    /**
         *@brief Initialization function
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init(const MUINT32 eisFactor);

    /**
         *@brief Unitialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Configure LMV
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] aEisConfig : EIS config data
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLMV(const LMV_HAL_CONFIG_DATA &aEisConfig);

    /**
         *@brief Execute LMV
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] apEisConfig : EIS config data, mainly for pass2
         *@param[in] aTimeStamp : time stamp of pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoLMVCalc(const EIS_PASS_ENUM &aEisPass, QBufInfo const &pBufInfo);

    /**
         *@brief Get EIS algorithm result (CMV)
         *@param[out] a_CMV_X_Int : EIS algo result of X-direction integer part
         *@param[out] a_CMV_X_Flt  : EIS algo result of X-direction float part
         *@param[out] a_CMV_Y_Int : EIS algo result of Y-direction integer part
         *@param[out] a_CMV_Y_Flt  : EIS algo result of Y-direction float part
         *@param[out] a_TarWidth    : EIS width crop size
         *@param[out] a_TarHeight   : EIS height crop size
       */
    virtual MVOID GetLMVResult(MUINT32 &a_CMV_X_Int,
                                   MUINT32 &a_CMV_X_Flt,
                                   MUINT32 &a_CMV_Y_Int,
                                   MUINT32 &a_CMV_Y_Flt,
                                   MUINT32 &a_TarWidth,
                                   MUINT32 &a_TarHeight,
                                   MINT32  &a_MVtoCenterX,
                                   MINT32  &a_MVtoCenterY,
                                   MUINT32 &a_isFromRRZ);

    /**
         *@brief Get EIS GMV
         *@details The value is 256x
         *@param[out] aGMV_X : x-direction global motion vector between two frames
         *@param[out] aGMV_Y  : y-direction global motion vector between two frames
         *@param[out] aEisInW  : width of EIS input image (optional)
         *@param[out] aEisInH  : height of EIS input image (optional)
         *@param[out] MAX_GMV  : max gmv search range (optional)
       */
    virtual MVOID GetGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *confX=NULL,MUINT32 *confY=NULL,
        MUINT32 *MAX_GMV=NULL);

    /**
         *@brief Get EIS HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates EIS HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetLMVSupportInfo(const MUINT32 &aSensorIdx);

    /**
         *@brief Get EIS plus crop ratio
         *@return
         *-EIS plus crop ratio
       */
    virtual MUINT32 GetEisPlusCropRatio();

    virtual NSCam::MSize QueryMinSize(MBOOL isEISOn, NSCam::MSize sensorSize, NSCam::MSize outputSize, NSCam::MSize requestSize, NSCam::MSize FovMargin);

    virtual MINT32 GetBufLMV(android::sp<IImageBuffer>& spBuf);

    virtual MINT32 NotifyLMV(QBufInfo&  pBufInfo);

    virtual MINT32 NotifyLMV(android::sp<NSCam::IImageBuffer>& spBuf);

#if EIS_ALGO_READY

    /**
         *@brief Return EIS HW statistic result
         *@param[in,out] a_pEIS_Stat : EIS_STATISTIC_STRUCT
       */
    virtual MVOID GetEisStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat);

#endif

/**
     *@brief  Get input width/height of LMV HW
   */
    MVOID GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight);

/**
     *@brief  Return  EIS HW setting of EOS_OP_HORI
     *@return
     *-EOS_OP_HORI
   */
    MUINT32 GetEisDivH();

/**
     *@brief  Return  EIS HW setting of EIS_OP_VERT
     *@return
     *-EIS_OP_VERT
   */
    MUINT32 GetEisDivV();

    MUINT32 GetLMVStatus();

    MVOID GetRegSetting(void *data);

    MVOID EnableGyroSensor();
private:

    /**
         *@brief Get sensor info
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorInfo();

#if EIS_ALGO_READY

    /**
         *@brief Get EIS customize info
         *@param[out] a_pDataOut : EIS_TUNING_PARA_STRUCT
       */
    MVOID GetEisCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut);

    /**
         *@brief Dump EIS HW statistic info
         *@param[in] aEisStat : EIS_STATISTIC_T
       */
    MVOID DumpStatistic(const EIS_STATISTIC_STRUCT &aEisStat);

#endif

    /**
         *@brief Create IMem buffer
         *@param[in,out] memSize : memory size, will align to L1 cache
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMemBuf(MUINT32 memSize, android::sp<IImageBuffer>& spImageBuf);

    /**
         *@brief Create IMem buffer
         *@param[in,out] memSize : memory size, will align to L1 cache
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMultiMemBuf(MUINT32 memSize, MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_MEMORY_SIZE]);

    /**
         *@brief Destroy IMem buffer
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMemBuf(android::sp<IImageBuffer>& spImageBuf);

    /**
         *@brief Destroy IMem buffer
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMultiMemBuf(MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_MEMORY_SIZE]);

    /**
         *@brief Prepare EIS pass1 result
         *@param[in] cmvX : EIS result
         *@param[in] cmvY : EIS result
         *@param[in] aGmvConfidX : gmvX confidence level
         *@param[in] aGmvConfidY : gmvY confidence level
         *@param[in] aTimeStamp : time stamp of pass1 image
       */
    MVOID PrepareEisResult(const MINT32 &cmvX,const MINT32 &cmvY);

    /***************************************************************************************/

    volatile MINT32 mUsers;
    mutable Mutex mLock;
    mutable Mutex mP1Lock;
    mutable Mutex mP2Lock;

    // LMV driver object
    LMVDrv *m_pLMVDrv;

#if EIS_ALGO_READY

    // EIS algo object
    MTKEis *m_pEisAlg;
    EIS_SET_PROC_INFO_STRUCT mEisAlgoProcData;

#endif

    android::sp<IImageBuffer> m_pEisDbgBuf;

    // EIS member variable
    MUINT32 mEisInput_W;
    MUINT32 mEisInput_H;
    MUINT32 mP1Target_W;
    MUINT32 mP1Target_H;

    MUINT32 mVideoW;
    MUINT32 mVideoH;

    // EIS result
    MUINT32 mDoEisCount;    //Vent@20140427: Add for EIS GMV Sync Check.
    MUINT32 mCmvX_Int;
    MUINT32 mCmvX_Flt;
    MUINT32 mCmvY_Int;
    MUINT32 mCmvY_Flt;
    MINT32  mMVtoCenterX;
    MINT32  mMVtoCenterY;

    MINT32 mGMV_X;
    MINT32 mGMV_Y;
    MINT32 mMAX_GMV;
    EIS_GET_PLUS_INFO_STRUCT mEisLastData2EisPlus;
    queue<MUINT64> mEis2EisPlusGmvTS;
    queue<MFLOAT> mEis2EisPlusGmvX;
    queue<MFLOAT> mEis2EisPlusGmvY;
    queue<MINT32> mEis2EisPlusConfX;
    queue<MINT32> mEis2EisPlusConfY;

    // EISPlus result
    EIS_PLUS_RESULT_INFO_STRUCT mEisPlusResult;

    // member variable
    MUINT32 mFrameCnt;
    MUINT32 mEisPass1Enabled;
    MUINT32 mIsEisConfig;
    MUINT32 mMemAlignment;
    MUINT32 mEisPlusCropRatio;
    MBOOL   mEisSupport;

    // sensor
    IHalSensorList *m_pHalSensorList;
    IHalSensor *m_pHalSensor;
    MUINT32 mSensorIdx;
    MUINT32 mSensorDev;
    SensorStaticInfo mSensorStaticInfo;
    SensorDynamicInfo mSensorDynamicInfo;
    MUINT64 mTsForAlgoDebug;

    android::sp<IImageBuffer> m_pEISOMainBuffer;
    android::sp<IImageBuffer> m_pEISOSliceBuffer[MAX_MEMORY_SIZE];

    mutable Mutex               mEISOBufferListLock;
    vecSPIImageBuffer           mEISOBufferList;
    MUINT32 mBufIndex;

    android::sp<SensorProvider> mpSensorProvider;

public:
    static MINT32 mDebugDump;
};

/**
  *@class EisHalObj
  *@brief singleton object for each LMVHal which is seperated by sensor index
*/
template<const unsigned int aSensorIdx>
class LMVHalObj : public LMVHalImp
{
private:
    static LMVHalObj<aSensorIdx>* spInstance;
    static Mutex    s_instMutex;

    LMVHalObj() : LMVHalImp(aSensorIdx) {}
    ~LMVHalObj() {}
public:
    static LMVHal *GetInstance()
    {
        if (0 == spInstance)
        {
            AutoMutex lock(s_instMutex);
            if (0 == spInstance)
            {
                spInstance = new LMVHalObj();
            }

        }
        return spInstance;
    }


    static void destroyInstance(void)
    {
        AutoMutex lock(s_instMutex);
        if (0 != spInstance)
        {
            delete spInstance;
            spInstance = 0;
        }
    }

};


#endif

