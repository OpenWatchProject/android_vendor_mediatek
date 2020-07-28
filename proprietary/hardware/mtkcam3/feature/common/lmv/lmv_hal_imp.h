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
* LMV Hal Implementation Header File
*
*/


#ifndef _LMV_HAL_IMP_H_
#define _LMV_HAL_IMP_H_

#include <Mutex.h>
#include <Condition.h>

#include <mtkcam3/feature/lmv/lmv_hal.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

using android::Mutex;
using android::Condition;


#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

#define MAX_LMV_MEMORY_SIZE (80)

#define TSRECORD_MAXSIZE    (108000)
#define GYRO_DATA_PER_FRAME     (100)

#define LMV_EIS_FE_MAX_INPUT_W  (2400)
#define LMV_EIS_FE_MAX_INPUT_H  (1360)

typedef android::sp<IImageBuffer> spIImageBuffer;

typedef std::queue<spIImageBuffer>  vecSPIImageBuffer;

namespace NSCam{
namespace Utils{
class SensorProvider;
}
}

/**
  *@class LmvHalImp
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
         *@brief Create LMVHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-LMVHal object
       */
    static LMVHal *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy LMVHal object
         *@param[in] userName : user name,i.e. who destroy LMV object
       */
    virtual MVOID DestroyInstance(char const *userName);

    /**
         *@brief Initialization function
         *@param[in] aSensorIdx : sensor index
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init(const MUINT32 eisFactor);

    /**
         *@brief Unitialization function
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Configure LMV
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aLmvConfig : LMV config data
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLMV(const LMV_HAL_CONFIG_DATA &aLmvConfig);

    virtual MINT32 PrepareLMV(const LMV_HAL_ENQUEUE_DATA &aLmvEnqueData);

    /**
         *@brief Execute LMV
         *@param[in] QBufInfo : LMV result data
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoLMVCalc(QBufInfo const &pBufInfo, const MUINT32 rrzIdx, const MUINT32 lmvIdx);

    /**
         *@brief Get LMV algorithm result (CMV)
         *@param[out] a_CMV_X_Int : LMV algo result of X-direction integer part
         *@param[out] a_CMV_X_Flt  : LMV algo result of X-direction float part
         *@param[out] a_CMV_Y_Int : LMV algo result of Y-direction integer part
         *@param[out] a_CMV_Y_Flt  : LMV algo result of Y-direction float part
         *@param[out] a_TarWidth    : LMV width crop size
         *@param[out] a_TarHeight   : LMV height crop size
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
         *@brief Get LMV GMV
         *@details The value is 256x
         *@param[out] aGMV_X : x-direction global motion vector between two frames
         *@param[out] aGMV_Y  : y-direction global motion vector between two frames
         *@param[out] aLMVInW  : width of LMV input image (optional)
         *@param[out] aLMVInH  : height of LMV input image (optional)
         *@param[out] MAX_GMV  : max gmv search range (optional)
       */
    virtual MVOID GetGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *confX=NULL,MUINT32 *confY=NULL,
        MUINT32 *MAX_GMV=NULL);

    /**
         *@brief Get LMV HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates LMV HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetLMVSupportInfo(const MUINT32 &aSensorIdx);

    virtual MINT32 GetBufLMV(android::sp<IImageBuffer>& spBuf);

    virtual MINT32 NotifyLMV(QBufInfo&  pBufInfo);

    virtual MINT32 NotifyLMV(android::sp<NSCam::IImageBuffer>& spBuf);

    /**
         *@brief Return LMV HW statistic result
         *@param[in,out] a_pLMV_Stat : EIS_STATISTIC_STRUCT
       */
    virtual MVOID GetLMVStatistic(EIS_STATISTIC_STRUCT *a_pLMV_Stat);


/**
     *@brief  Get input width/height of LMV HW
   */
    MVOID GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight);

    MUINT32 GetLMVStatus();

    MVOID GetRegSetting(void *data);

    MVOID EnableGyroSensor();
private:

    /**
         *@brief Get sensor info
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorInfo();


    /**
         *@brief Get EIS customize info
         *@param[out] a_pDataOut : EIS_TUNING_PARA_STRUCT
       */
    MVOID GetEisCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut);

    /**
         *@brief Dump EIS HW statistic info
         *@param[in] aLmvStat : EIS_STATISTIC_T
       */
    MVOID DumpStatistic(const EIS_STATISTIC_STRUCT &aLmvStat);


    /**
         *@brief Create IMem buffer
         *@param[in,out] memSize : memory size, will align to L1 cache
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMemBuf(MUINT32 memSize, android::sp<IImageBuffer>& spImageBuf);

    /**
         *@brief Create IMem buffer
         *@param[in,out] memSize : memory size, will align to L1 cache
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMultiMemBuf(MUINT32 memSize, MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_LMV_MEMORY_SIZE]);

    /**
         *@brief Destroy IMem buffer
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMemBuf(android::sp<IImageBuffer>& spImageBuf);

    /**
         *@brief Destroy IMem buffer
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMultiMemBuf(MUINT32 num, android::sp<IImageBuffer>& spMainImageBuf, android::sp<IImageBuffer> spImageBuf[MAX_LMV_MEMORY_SIZE]);

    /**
         *@brief Prepare LMV pass1 result
         *@param[in] cmvX : LMV result
         *@param[in] cmvY : LMV result
         *@param[in] aGmvConfidX : gmvX confidence level
         *@param[in] aGmvConfidY : gmvY confidence level
         *@param[in] aTimeStamp : time stamp of pass1 image
       */
    MVOID PrepareLmvResult(const MINT32 &cmvX,const MINT32 &cmvY);

    LMVData getDefaultData(QBufInfo const &pBufInfo, const MUINT32 rrzIdx);

    /***************************************************************************************/

private:

    volatile MINT32 mUsers = 0;

    mutable Mutex mLock;
    mutable Mutex mP1Lock;
    mutable Mutex mP2Lock; //use?
    mutable Mutex mLMVOBufferListLock;

private:

    EIS_SET_PROC_INFO_STRUCT mLmvAlgoProcData;//no use?

    // LMV member variable
    MUINT32 mLmvInput_W = 0;
    MUINT32 mLmvInput_H = 0;
    MUINT32 mP1ResizeIn_W = 0;
    MUINT32 mP1ResizeIn_H = 0;
    MUINT32 mP1ResizeOut_W = 0;
    MUINT32 mP1ResizeOut_H = 0;
    MUINT32 mP1Target_W = 0;
    MUINT32 mP1Target_H = 0;
    MUINT32 mVideoW = 0;
    MUINT32 mVideoH = 0;

    // LMV result
    MUINT32 mDoLmvCount = 0;
    MUINT32 mCmvX_Int = 0;
    MUINT32 mCmvX_Flt = 0;
    MUINT32 mCmvY_Int = 0;
    MUINT32 mCmvY_Flt = 0;
    MUINT32 mCmvTmpX_Int = 0;
    MUINT32 mCmvTmpX_Flt = 0;
    MUINT32 mCmvTmpY_Int = 0;
    MUINT32 mCmvTmpY_Flt = 0;
    MINT32  mMVtoCenterX = 0;
    MINT32  mMVtoCenterY = 0;
    MINT32 mGMV_X = 0;
    MINT32 mGMV_Y = 0;
    MINT32 mMAX_GMV = LMV_MAX_GMV_DEFAULT;
    EIS_GET_PLUS_INFO_STRUCT mLmvLastData2EisPlus;

    // member variable
    MUINT32 mFrameCnt = 0;
    MUINT32 mEisPass1Enabled = 0;
    MUINT32 mIsLmvConfig = 0;
    MUINT32 mMemAlignment = 0;
    MUINT32 mEisPlusCropRatio = 100;
    MBOOL   mLmvSupport = MTRUE;

    MUINT32 mSensorIdx = 0;
    MUINT32 mSensorDev = 0;
    MUINT64 mTsForAlgoDebug = 0;
    MUINT32 mBufIndex  = 0;

    float mPrev_X = 0;
    float mPrev_Y = 0;
    float mCur_X = 0;
    float mCur_Y = 0;

private:

    IHalSensorList *m_pHalSensorList             = NULL;
    IHalSensor *m_pHalSensor                     = NULL;
    SensorStaticInfo mSensorStaticInfo;
    SensorDynamicInfo mSensorDynamicInfo;

    android::sp<SensorProvider> mpSensorProvider = NULL;

    LMVDrv* m_pLMVDrv = NULL;
    MTKEis* m_pEisAlg = NULL;

private:
    android::sp<IImageBuffer> m_pLmvDbgBuf       = NULL;
    android::sp<IImageBuffer> m_pLMVOMainBuffer  = NULL;
    android::sp<IImageBuffer> m_pLMVOSliceBuffer[MAX_LMV_MEMORY_SIZE];
    vecSPIImageBuffer         mLMVOBufferList;

private:
    std::string mName;
    static MINT32 mDebugDump;
    FILE* mDumpFile = NULL;
};

/**
  *@class LmvHalObj
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

