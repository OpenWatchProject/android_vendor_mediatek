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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/CBMgr"
//
#include <list>
#include <utils/threads.h>
#include <sys/time.h>
using namespace android;
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <vector>
using namespace std;
//
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;
//
#include <mtkcam/camshot/CapBufMgr.h>
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s]" fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s]" fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s]" fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s]" fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s]" fmt, __func__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#endif
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot
{
//
class CapBufMgrImp : public CapBufMgr
{
    public:
        CapBufMgrImp(MINT32 idx);
        ~CapBufMgrImp();
        //
        virtual MVOID   destroyInstance(void);
        virtual MBOOL   setMaxDequeNum(MUINT32 maxNum);
        virtual MBOOL   dequeBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize = MTRUE);
        virtual MBOOL   enqueBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize = MTRUE);
        virtual MBOOL   popBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize = MTRUE);
        virtual MBOOL   pushBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize = MTRUE);
        virtual MBOOL   getBufLen(MUINT32& bufLen, bufLenType lenType = CAP_BUF_LEN_CURRENT, MBOOL const isFullSize = MTRUE);
        MINT32          getSensorIdx() const { return mSensorIdx; }
        virtual MVOID   enableCompareShotTime(MBOOL enable = MTRUE);
        //
    private:
        MINT32                      mSensorIdx;
        mutable Mutex               mLock;
        mutable Condition           mCond_Full;
        mutable Condition           mCond_Prv;
        mutable Condition           mCondDequeBuf_Full;
        mutable Condition           mCondDequeBuf_Prv;
        list<IImageBuffer*>         mlpImgBuf_Full;     // full-size
        list<IImageBuffer*>         mlpImgBuf_Prv;      // preview-size
        MBOOL                       mbUse_Full;
        MBOOL                       mbUse_Prv;
        MUINT32                     mDequeBufCnt_Full;
        MUINT32                     mDequeBufCnt_Prv;
        MUINT32                     mDequeBufMaxNum;
        MUINT32                     mPushedBufferCount_Full;
        MBOOL                       compareShotTime;
        MINT32                      shotTimeDiff; // ms
};
/*******************************************************************************
*
********************************************************************************/
static Mutex                gCapBufMgrImpLock;
static list<CapBufMgrImp*>  glpCapBufMgrImpList;
//-----------------------------------------------------------------------------
CapBufMgr*
CapBufMgr::
createInstance(MINT32 idx)
{
    CapBufMgrImp* pCapBufMgrImp = new CapBufMgrImp(idx);
    return pCapBufMgrImp;
}
//-----------------------------------------------------------------------------
MVOID
CapBufMgrImp::
destroyInstance(void)
{
    delete this;
}
//------------------------------------------------------------------------------
CapBufMgrImp::
CapBufMgrImp(MINT32 idx)
    : mSensorIdx(idx)
    , mbUse_Full(MFALSE)
    , mbUse_Prv(MFALSE)
    , mDequeBufCnt_Full(0)
    , mDequeBufCnt_Prv(0)
    , mDequeBufMaxNum(0)
    , mPushedBufferCount_Full(0)
    , compareShotTime(MFALSE)
    , shotTimeDiff(150)
{
    FUNC_NAME;
    mlpImgBuf_Full.clear();
    mlpImgBuf_Prv.clear();
}
//-----------------------------------------------------------------------------
CapBufMgrImp::
~CapBufMgrImp()
{
    FUNC_NAME;
    mlpImgBuf_Full.clear();
    mlpImgBuf_Prv.clear();
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
setMaxDequeNum(MUINT32 maxNum)
{
    mDequeBufMaxNum = maxNum;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
dequeBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    //
    list<IImageBuffer*>::iterator it;
    if ( isFullSize )
    {
        MINT64 timestamp = 0;
        if(compareShotTime)
        {
            struct timespec t;

            t.tv_sec = t.tv_nsec = 0;
            clock_gettime(CLOCK_MONOTONIC, &t);
            timestamp = ((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000000; // from nano to ms

            MY_LOGD("deque timestamp = %d", timestamp);
        }
        while(1)
        {
            if(mDequeBufCnt_Full >= mDequeBufMaxNum)
            {
                MY_LOGW("DBC(%d) < DBMN(%d), Wait E", mDequeBufCnt_Full, mDequeBufMaxNum);
                mCondDequeBuf_Full.wait(mLock);
                MY_LOGW("DBC(%d), Wait X",mDequeBufCnt_Full);
            }
            else
            {
                for(it = mlpImgBuf_Full.begin(); it != mlpImgBuf_Full.end(); it++)
                {
                    if((*it)->getTimestamp() != 0)
                    {
                        if (compareShotTime != MTRUE)
                        {
                            break;
                        }
                        else
                        {
                            MINT64 bufTime = (MINT64)(((*it)->getTimestamp()))/1000000;
                            bufTime = bufTime + shotTimeDiff;
                            if (bufTime >= timestamp)
                            {
                                break;
                            }
                            else
                            {
                                (*it)->setTimestamp(0); // set as used
                            }
                        }
                    }
                }
                //
                if(it == mlpImgBuf_Full.end())
                {
                    MY_LOGW("No full-buf is ready, Wait E");
                    mCond_Full.wait(mLock);
                    MY_LOGW("Wait X");
                }
                else
                {
                    break;
                }
            }
        }
        mDequeBufCnt_Full++;
    }
    else
    {
        while(1)
        {
            if(mDequeBufCnt_Prv >= mDequeBufMaxNum)
            {
                MY_LOGW("DBC-Prv(%d) < DBMN(%d), Wait E", mDequeBufCnt_Prv, mDequeBufMaxNum);
                mCondDequeBuf_Prv.wait(mLock);
                MY_LOGW("DBC-Prv(%d), Wait X",mDequeBufCnt_Prv);
            }
            else
            {
                for(it = mlpImgBuf_Prv.begin(); it != mlpImgBuf_Prv.end(); it++)
                {
                    if((*it)->getTimestamp() != 0)
                    {
                        break;
                    }
                }
                //
                if(it == mlpImgBuf_Prv.end())
                {
                    MY_LOGW("No prv-buf is ready, Wait E");
                    mCond_Prv.wait(mLock);
                    MY_LOGW("Wait X");
                }
                else
                {
                    break;
                }
            }
        }
        mDequeBufCnt_Prv++;
    }
    //
    compareShotTime = MFALSE;
    //
    MUINT32 dequeBufCnt = (isFullSize) ? mDequeBufCnt_Full : mDequeBufCnt_Prv;
    MY_LOGD("idx(%d),full(%d),Buf(0x%X),TS(%d.%06d),DBC(%d)",
            getSensorIdx(),
            isFullSize,
            (MUINTPTR)*it,
            (MUINT32)(((*it)->getTimestamp()/1000)/1000000),
            (MUINT32)(((*it)->getTimestamp()/1000)%1000000),
            dequeBufCnt);
    //
    pImageBuffer = *it;
    if ( isFullSize )
    {
        mlpImgBuf_Full.erase(it);
    }
    else
    {
        mlpImgBuf_Prv.erase(it);
    }
    //
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
enqueBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    //
    pImageBuffer->setTimestamp(0);
    //
    if ( isFullSize )
    {
        mlpImgBuf_Full.push_back(pImageBuffer);
        if(mDequeBufCnt_Full == 0)
        {
            MY_LOGE("mDequeBufCnt_Full is 0");
        }
        else
        {
            mDequeBufCnt_Full--;
        }
        //
        mCondDequeBuf_Full.signal();
    }
    else
    {
        mlpImgBuf_Prv.push_back(pImageBuffer);
        if(mDequeBufCnt_Prv== 0)
        {
            MY_LOGE("mDequeBufCnt_Prv is 0");
        }
        else
        {
            mDequeBufCnt_Prv--;
        }
        //
        mCondDequeBuf_Prv.signal();
    }
    //
    MUINT32 dequeBufCnt = (isFullSize) ? mDequeBufCnt_Full : mDequeBufCnt_Prv;
    MY_LOGD("idx(%d),full(%d),Buf(0x%X),TS(%d.%06d),DBC(%d)",
            getSensorIdx(),
            isFullSize,
            (MUINTPTR)pImageBuffer,
            (MUINT32)((pImageBuffer->getTimestamp()/1000)/1000000),
            (MUINT32)((pImageBuffer->getTimestamp()/1000)%1000000),
            dequeBufCnt);
    //
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
popBuf(IImageBuffer*& pImageBuffer, MBOOL const isFullSize)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    //
    if ( isFullSize )
    {
        if(mlpImgBuf_Full.empty())
        {
            if(mbUse_Full)
            {
                MY_LOGW("buf_full is empty");
            }
            return MFALSE;
        }
    }
    else
    {
        if(mlpImgBuf_Prv.empty())
        {
            if(mbUse_Prv)
            {
                MY_LOGW("buf_prv is empty");
            }
            return MFALSE;
        }
    }
    //
    list<IImageBuffer*>::iterator it = (isFullSize) ? mlpImgBuf_Full.begin() : mlpImgBuf_Prv.begin();
    //
    MY_LOGD("idx(%d),full(%d),Buf(0x%X),TS(%d.%06d)",
            getSensorIdx(),
            isFullSize,
            (MUINTPTR)*it,
            (MUINT32)(((*it)->getTimestamp()/1000)/1000000),
            (MUINT32)(((*it)->getTimestamp()/1000)%1000000));
    //
    pImageBuffer = *it;
    if ( isFullSize )
    {
        mlpImgBuf_Full.erase(it);
        mPushedBufferCount_Full--;
    }
    else
    {
        mlpImgBuf_Prv.erase(it);
    }
    //
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
pushBuf(IImageBuffer* pImageBuffer, MBOOL const isFullSize)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    //
    MY_LOGD("idx(%d),full(%d),Buf(0x%X),TS(%d.%06d)",
            getSensorIdx(),
            isFullSize,
            (MUINTPTR)pImageBuffer,
            (MUINT32)((pImageBuffer->getTimestamp()/1000)/1000000),
            (MUINT32)((pImageBuffer->getTimestamp()/1000)%1000000));
    //
    if ( isFullSize )
    {
        mlpImgBuf_Full.push_back(pImageBuffer);
        mPushedBufferCount_Full++;
        if(pImageBuffer->getTimestamp() > 0)
        {
            mCond_Full.signal();
        }
        if(!mbUse_Full)
        {
            mbUse_Full = MTRUE;
        }
    }
    else
    {
        mlpImgBuf_Prv.push_back(pImageBuffer);
        if(pImageBuffer->getTimestamp() > 0)
        {
            mCond_Prv.signal();
        }
        if(!mbUse_Prv)
        {
            mbUse_Prv = MTRUE;
        }
    }
    //
    FUNC_END;
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL
CapBufMgrImp::
getBufLen(MUINT32& bufLen, bufLenType lenType, MBOOL const isFullSize)
{
    MBOOL ret=MTRUE;
    MY_LOGD("lenType = %d", lenType);
    switch(lenType)
    {
        case CAP_BUF_LEN_CURRENT:
            if ( isFullSize )
            {
                bufLen = mlpImgBuf_Full.size();
            }
            else
            {
                bufLen = mlpImgBuf_Prv.size();
            }
            break;

        case CAP_BUF_LEN_MAX:
            bufLen = mDequeBufMaxNum;
            break;

        case CAP_BUF_LEN_LEFT:
            if ( isFullSize )
            {
                bufLen = mDequeBufMaxNum - mlpImgBuf_Full.size();
            }
            else
            {
                bufLen = mDequeBufMaxNum - mlpImgBuf_Prv.size();
            }
            break;

        case CAP_BUF_LEN_USED:

            if (isFullSize)
            {
                list<IImageBuffer*>::iterator it;
                bufLen = 0;
                for(it = mlpImgBuf_Full.begin(); it != mlpImgBuf_Full.end(); it++)
                {
                    if((*it)->getTimestamp() == 0)
                    {
                        bufLen++;
                    }
                }
            }
            else
            {
                list<IImageBuffer*>::iterator it;
                bufLen = 0;
                for(it = mlpImgBuf_Prv.begin(); it != mlpImgBuf_Prv.end(); it++)
                {
                    if((*it)->getTimestamp() == 0)
                    {
                        bufLen++;
                    }
                }
            }
            break;

        case CAP_BUF_LEN_PUSHED:
            if (isFullSize)
            {
                bufLen = mPushedBufferCount_Full;
            }
            else
            {
                bufLen = 0; // not implement for preview part
            }
            break;

        default:
            MY_LOGE("Not support this Length type (%d)", lenType);
            ret = MFALSE;
            break;

    }
    MY_LOGD("bufLen = %d", bufLen);
    return ret;
}
//-----------------------------------------------------------------------------
MVOID
CapBufMgrImp::
enableCompareShotTime(MBOOL enable)
{
    compareShotTime = enable;
}
//-----------------------------------------------------------------------------
};

