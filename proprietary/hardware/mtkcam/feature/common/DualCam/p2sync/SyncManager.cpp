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

#define LOG_TAG "MtkCam/SyncManager"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <utils/RWLock.h>
#include <utils/Mutex.h>
#include <utils/String8.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <libladder.h>

//
#include <cutils/properties.h>

#include "SyncManager.h"
//

#include "DualCamStreamingFeaturePipe.h"
//
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT==1) || (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT==1) || (MTKCAM_HAVE_VSDOF_SUPPORT == 1) || (MTKCAM_HAVE_STEREO_CAMERA_SUPPORT == 1)
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#define DUAL_CAM 1
#else
#define DUAL_CAM 0
#endif
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
#include <camera_custom_dualzoom.h>
#endif

using namespace android;
using namespace std;
using namespace NSCam;

namespace NSCam
{

/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN(id)             CAM_LOGI("[%d]id:%d[%s]+", mInstanceRefCount, id, __FUNCTION__)
#define FUNCTION_OUT(id)            CAM_LOGI("[%d]id:%d[%s]-", mInstanceRefCount, id, __FUNCTION__)

/******************************************************************************
 * Initialize
 ******************************************************************************/
MINT32 SyncManagerReqList::totalRefCount = 0;
MINT32 SyncManager::totalRefCount = 0;
wp<SyncManager> SyncManager::mThiz = NULL;
static Mutex& SyncManagerCreateLock = *new Mutex();


/******************************************************************************
 *
 ******************************************************************************/
android::sp<ISyncManager> ISyncManager::createInstance(MINT32 const i4OpenId)
{
    return SyncManager::createInstance(i4OpenId);
}

/******************************************************************************
 *
 ******************************************************************************/
android::sp<ISyncManager> ISyncManager::getInstance(MINT32 const i4OpenId)
{
    return SyncManager::getInstance(i4OpenId);
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<SyncManager> SyncManager::getInstance(MINT32 const i4OpenId)
{
    // only one Synchronizer instance
    sp<SyncManager> inst = SyncManager::mThiz.promote();

    //CAM_LOGD("[%s] getInstance() current %p", __FUNCTION__, (inst.get()));

    if (inst.get())
    {
        return inst;
    }
    else
    {
        //return SyncManager::createInstance(i4OpenId);
        CAM_LOGW("[%s] = null", __FUNCTION__);
        return NULL;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
android::sp<SyncManager> SyncManager::createInstance(MINT32 const i4OpenId)
{
    Mutex::Autolock _l(SyncManagerCreateLock);

    // only one Synchronizer instance
    sp<SyncManager> inst = SyncManager::mThiz.promote();

    CAM_LOGD("[%s] createInstance() current %p", __FUNCTION__, (inst.get()));

    if (inst.get())
    {
        inst->addOpenID(i4OpenId);
        return inst;
    }
    else
    {
        SyncManager::mThiz = new SyncManager(i4OpenId);
        inst = SyncManager::mThiz.promote();
        CAM_LOGW("[%s] createInstance() new %p", __FUNCTION__, (inst.get()));
        return inst;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
SyncManager::SyncManager(MINT32 const i4OpenId):
    mFlush(false), mInstanceRefCount(SyncManager::totalRefCount)
{
    MY_LOGD(i4OpenId, "new SyncManager(#%d) %p id:%d", mInstanceRefCount, this, i4OpenId);
    SyncManager::totalRefCount++;
#if DUAL_CAM
    MINT32 featureMode = StereoSettingProvider::getStereoFeatureMode();
    MY_LOGD(i4OpenId, "feature mode (%d)", featureMode);
    if(v1::Stereo::E_DUALCAM_FEATURE_ZOOM == featureMode)
    {
        mIDualCamPolicy = IDualCamPolicy::createInstance(DUAL_CAM_POLICY_MTK_TK);
    }
    else if(v1::Stereo::E_STEREO_FEATURE_DENOISE == featureMode)
    {
        mIDualCamPolicy = IDualCamPolicy::createInstance(DUAL_CAM_POLICY_DENOISE);
    }
    else if((v1::Stereo::E_STEREO_FEATURE_CAPTURE | v1::Stereo::E_STEREO_FEATURE_VSDOF) == featureMode
            || (v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP) == featureMode)
    {
        mIDualCamPolicy = IDualCamPolicy::createInstance(DUAL_CAM_POLICY_BOKEH);
    }
    else
    {
        MY_LOGE(i4OpenId, "feature mode (%d) not support please check policy or syncMgr define!", featureMode);
    }
#else
    mIDualCamPolicy = IDualCamPolicy::createInstance(DUAL_CAM_POLICY_MTK_TK);
#endif // DUAL_CAM

    mIFrame3AControl = NULL;

    addOpenID(i4OpenId);
    mInFlushing = false;


    // pause() first to avoid "processFrameSync()" with incorrect data
    this->pause();
    this->run(LOG_TAG);
}

SyncManager::~SyncManager()
{
    FUNCTION_IN(mFirstOpenId);

    mIFrame3AControl = NULL;
    mIDualCamPolicy = NULL;
    mICallback = NULL;

    FUNCTION_OUT(mFirstOpenId);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 SyncManager::updateSetting(
    MINT32 const i4OpenId,
    IMetadata* appMetadata,
    IMetadata* halMetadata,
    SyncManagerParams &syncParams
)
{
    // **** To-Do *********************************************
    // low power policy: fps change, standby mode
    // camera switch policy: UX behavior, Wide/Tele behavior
    // ********************************************************
    Mutex::Autolock _l(mSettingLock);
    //FUNCTION_IN(i4OpenId);

    if (mIDualCamPolicy.get() != NULL)
    {
        mIDualCamPolicy->updateSetting(i4OpenId, appMetadata, halMetadata, syncParams);
    }
    //FUNCTION_OUT(i4OpenId);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID SyncManager::lock(void* arg1)
{
    // **** To-Do lock ****************************************
    // low power policy: fps change, standby mode
    // camera switch policy: UX behavior, Wide/Tele behavior
    // ********************************************************
    Mutex::Autolock _l(mSettingLock);

    if (mIDualCamPolicy.get() != NULL)
    {
        mIDualCamPolicy->lock(arg1);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID SyncManager::unlock(void* arg1)
{
    // **** To-Do unlock **************************************
    // low power policy: fps change, standby mode
    // camera switch policy: UX behavior, Wide/Tele behavior
    // ********************************************************
    Mutex::Autolock _l(mSettingLock);

    if (mIDualCamPolicy.get() != NULL)
    {
        mIDualCamPolicy->unlock(arg1);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID SyncManager::registerMgrCb(
    sp<ISyncManagerCallback> cb
)
{
    mICallback = cb;
    if (mIDualCamPolicy != NULL)
    {
        mIDualCamPolicy->addCallBack(cb);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID SyncManager::setEvent(MUINT32 key, void * arg)
{
    MY_LOGW(-1, "Key:%u", key);

    switch (key)
    {
        // the size of openid is still 2 in flushAndRemoveOpenId()
        // due to destroy order (P1->P2) ... so move to this called from DualCam1Device
        //
        // stop hwsync if mOpenIds size ==1
        //if (mOpenIds.size() == 1)
        case 0:
        {
            Mutex::Autolock _l(mSettingLock);
            Frame3ASetting_t setting =
            {
                .openId   = -1,
                .hwSyncMode = 3,
            };
            if (mIFrame3AControl != NULL)
            {
                mIFrame3AControl->set(&setting, F3A_TYPE_FRAME_SYNC, NULL, NULL);
            }
            break;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
char const* SyncManager::getName()
{
    return "SyncManager";
}


/******************************************************************************
 *
 ******************************************************************************/
status_t SyncManager::queue(
    MINT32  const               openId,
    MUINT32 const               requestNo,
    FeaturePipeParam            param,
    void*                       cb)
{
    Mutex::Autolock _l(mRequsetLock);
    CAM_LOGI("[%d]id:%d[%s] frameNo:%d +", mInstanceRefCount, openId, __FUNCTION__, requestNo);
    /*if (mFlush)
    {
        MY_LOGW(openId, "queue() failed due to flush");
        return -ENODEV;
    }*/

    MBOOL isAdded = false;

    if (mInFlushing)
    {
        param.mVarMap.set<MINT32>(VAR_DUALCAM_DROP_REQ, MTK_DUALZOOM_DROP_NEED_SYNCMGR);
    }
    // add to the list
    for (MUINT i = 0; i < mOpenIds.size() ; i++)
    {
        if (openId == mOpenIds[i])
        {
            mSyncManagerReqLists[i]->add(openId, requestNo, param, cb);
            isAdded = true;
            break;
        }
    }

    if (!isAdded)
    {
        MY_LOGW(openId, "queue() failed");
    }


    // dump
    for (MUINT i = 0; i < mSyncManagerReqLists.size() ; i++)
    {
        android::String8 msg = String8::format(",");
        for (MUINT j = 0 ; j < mSyncManagerReqLists[i]->getListItemCount() ; j++)
        {
            sp<SyncManagerReqItem> frame = mSyncManagerReqLists[i]->getItem(j);
            msg += String8::format("%d ", frame->mRequestNo);
        }
        MY_LOGD(openId, "id:%d => %s", mOpenIds[i], msg.string());

    }

    // signal to process frame sync
    resume();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t SyncManager::flushAndRemoveOpenId(MINT32 openId)
{
    FUNCTION_IN(openId);

    { // Begin Mutex::Autolock _l(mRequsetLock);
        Mutex::Autolock _l(mRequsetLock);
        mFlush = true;

        for (MUINT i = 0; i < mOpenIds.size(); i++)
        {
            if (mOpenIds[i] == openId)
            {
                for (MUINT j = 0; j < mSyncManagerReqLists[i]->getListItemCount(); j++)
                {
                    sp<SyncManagerReqItem> item = mSyncManagerReqLists[i]->getItem(j);
                    markDropFrame(item);
                    mCallBackLists[i]->add(item);
                }
                mSyncManagerReqLists[i]->doClear();

                processFrameCallback(i, false);

                if (mCallBackLists[i]->getListItemCount() > 0)
                {
                    MY_LOGW(openId, "callback list is not empty !");
                    exit(1);
                }

                mOpenIds.erase(mOpenIds.begin() + i);
                mSyncManagerReqLists.erase(mSyncManagerReqLists.begin() + i);
                mCallBackLists.erase(mCallBackLists.begin() + i);
                break;
            }
        }

        if (mOpenIds.size() < 1)
        {
            requestExit();
        }
        if(mIDualCamPolicy.get() != NULL)
        {
            Mutex::Autolock _l(mSettingLock);
            mIDualCamPolicy->setCameraOpenIds(mOpenIds);
        }
    } // End Mutex::Autolock _l(mRequsetLock);

    FUNCTION_OUT(openId);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL SyncManager::flushOnly(MINT32 openId)
{
    FUNCTION_IN(openId);

    Mutex::Autolock _l(mRequsetLock);

    for (MUINT i = 0; i < mOpenIds.size(); i++)
    {
        if (mOpenIds[i] == openId)
        {
            for (MUINT j = 0; j < mSyncManagerReqLists[i]->getListItemCount(); j++)
            {
                sp<SyncManagerReqItem> item = mSyncManagerReqLists[i]->getItem(j);
                markDropFrame(item);
                mCallBackLists[i]->add(item);
            }
            mSyncManagerReqLists[i]->doClear();
            processFrameCallback(i, false);
            break;
        }
    }
    //if (openId == DUALZOOM_WIDE_CAM_ID)
    {
        mInFlushing = true;
    }

    FUNCTION_OUT(openId);
    return true;
}



/******************************************************************************
 *
 ******************************************************************************/
status_t SyncManager::dump(MINT32 openId)
{
    FUNCTION_IN(openId);


    FUNCTION_OUT(openId);
    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL SyncManager::isFirstId(MINT32 id)
{
    if (mFirstOpenId == id)
    {
        return true;
    }
    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 SyncManager::getIndexOfOpenIds(MINT32 openid)
{
    for (MUINT i = 0 ; i < mOpenIds.size() ; i++)
    {
        if (mOpenIds[i] == openid)
        {
            return i;
        }
    }
    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 SyncManager::getOpenedIdCount()
{
    Mutex::Autolock _l(mRequsetLock);

    MINT32 count = mOpenIds.size();

    return count;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL SyncManager::isOpenedId(MINT32 id)
{
    MBOOL result = false;

    for (MUINT i = 0 ; i < mOpenIds.size() ; i++)
    {
        if (mOpenIds[i] == id)
        {
            //MY_LOGI("duplicated OpenId:%d", id);
            result = true;
        }
    }

    return result;
}


/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::addOpenID(MINT32 id)
{
    {
        Mutex::Autolock _l(mRequsetLock);

        MY_LOGI(id, "addOpenID() OpenId:%d", id);

        if (isOpenedId(id))
        {
            MY_LOGD(id, "duplicated OpenId:%d", id);
            return;
        }

        if (mOpenIds.size() == 0)
        {
            mFirstOpenId = id;
            mPreviewID = id;
            mPreviewNewID = id;
        }

        mSyncManagerReqLists.push_back(new SyncManagerReqList());
        mCallBackLists.push_back(new SyncManagerReqList());
        mOpenIds.push_back(id);

        if (mOpenIds.size() == 2 && mIFrame3AControl.get() == NULL)
        {
            mIFrame3AControl = IFrame3AControl::createInstance(mOpenIds[0], mOpenIds[1]);
            MY_LOGD(id, "new IFrame3AControl %p", mIFrame3AControl.get());
            mInFlushing = false;

            if (mIFrame3AControl.get() != NULL)
            {
                MY_LOGD(id, "init IFrame3AControl");
                mIFrame3AControl->init();
                if(mIDualCamPolicy != nullptr)
                    mIDualCamPolicy->setObject((void*)mIFrame3AControl.get(), 0);
            }
        }
    }

    if(mIDualCamPolicy != nullptr)
    {
        Mutex::Autolock _l(mSettingLock);
        mIDualCamPolicy->setCameraOpenIds(mOpenIds);
    }
    else
    {
        MY_LOGE(id, "mIDualCamPolicy is nullptr");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
status_t SyncManager::processFrameDrop()
{
    MINT reqListSize = mSyncManagerReqLists.size();

    MINT32 doDrop = 0;
    if (mIDualCamPolicy.get())
    {
        mPreviewNewID = mIDualCamPolicy->getPreviewCameraId(&doDrop);
        /*if (mPreviewID != mPreviewNewID)
        {
            // switch detected but wait new frame from new id
            MY_LOGD(-1, "cam switch %d -> %d", mPreviewID, mPreviewNewID);
        }*/
    }

    for (MINT i = 0; i < reqListSize; i++)
    {
        // remove in-order until notmal item
        while (true)
        {
            sp<SyncManagerReqItem> frame = mSyncManagerReqLists[i]->getItem(0);
            if (frame.get() == NULL)
            {
                break;
            }

            if (isDropFrame(frame, doDrop))
            {
                markDropFrame(frame);
                mSyncManagerReqLists[i]->rm(0);
                mCallBackLists[i]->add(frame);
            }
            else
            {
                break;
            }
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t SyncManager::processFrameSync(MINT32 const openId)
{
    Mutex::Autolock _l(mRequsetLock);
    bool ignoreSync = true;
    bool skipSync = false;

    //MY_LOGD(openId, "+");

    status_t result = UNKNOWN_ERROR;

    MINT main1_idx = 0;
    MINT main2_idx = 0;

    // check do sync or not
    int nosync = property_get_int32("vendor.debug.dualcam.nosync", 0);

    int forcesync = property_get_int32("vendor.debug.dualcam.forcesync", 0);

    int skipsync = property_get_int32("vendor.debug.dualcam.skipsync", 0);
    if(skipsync)
    {
        skipSync = true;
    }

    MUINT32 reqIndex = 0;
    // check if need to drop request
    {
        // add to the list
        for (MUINT i = 0; i < mOpenIds.size() ; i++)
        {
            // Limit the entry count to request list
            // or it will block pipe via display buffer when frame sync applied with timing issue
            while (mSyncManagerReqLists[i]->getListItemCount() >= 2)
            {
                MY_LOGD(-1, "openid(%d) item count(%d)", i, mSyncManagerReqLists[i]->getListItemCount());
                sp<SyncManagerReqItem> frame = mSyncManagerReqLists[i]->getItem(0);
                if (frame.get()) {
                    MINT32 dropReq = frame->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DROP_REQ, -1);
                    if(dropReq == MTK_DUALZOOM_DROP_NEVER_DROP)
                    {
                        markEnqueFrame(frame, NULL, false);
                        mSyncManagerReqLists[i]->rm(0);
                        mCallBackLists[i]->add(frame);
                    }
                    else
                    {
                        markDropFrame(frame);
                        mSyncManagerReqLists[i]->rm(0);
                        mCallBackLists[i]->add(frame);
                    }
                }
            }
        }
    }

    // remove the dropped frame first
    processFrameDrop();

    do
    {
        // remove the dropped frame first
        processFrameDrop();

        // *********************************************************************
        if (!nosync)
        {
            if (mSyncManagerReqLists.size() == 2)
            {
                sp<SyncManagerReqItem> frameidx0 = mSyncManagerReqLists[0]->getItem(0);
                sp<SyncManagerReqItem> frameidx1 = mSyncManagerReqLists[1]->getItem(0);
                MINT32 doSync = 0;
                if (NULL != frameidx0.get())
                {
                    doSync |= frameidx0->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
                }
                if (NULL != frameidx1.get())
                {
                    doSync |= frameidx1->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
                }
                {
                    MY_LOGD(openId, "doSync : %d", doSync);
                    ignoreSync = (doSync == 0);
                }
            }
            if (forcesync)
            {
                ignoreSync = false;
            }
        }
        if (ignoreSync || mOpenIds.size() == 1)
        {
            if (reqIndex >= mSyncManagerReqLists.size())
            {
                break;
            }

            sp<SyncManagerReqItem> frame1 = mSyncManagerReqLists[reqIndex]->getItem(main1_idx);
            if (frame1.get())
            {
                MINT32 DropReq = frame1->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DROP_REQ, -1);
                // get QParam, if out is zero, does not enque to pipe
                if(frame1->mFeaturePipeParam.mQParams.mvFrameParams.size() == 0 ||
                   frame1->mFeaturePipeParam.mQParams.mvFrameParams[0].mvOut.size() == 0 ||
                   DropReq)
                {
                    // drop
                    MY_LOGW(frame1->mOpenId , "mFeaturePipeParam.mQParams.mvFrameParams is 0. dorp(%d)", DropReq);
                    markDropFrame(frame1);
                    mSyncManagerReqLists[reqIndex]->rm(main1_idx);
                    mCallBackLists[reqIndex]->add(frame1);
                }
                else
                {
                    markEnqueFrame(frame1, NULL, false);
                    mSyncManagerReqLists[reqIndex]->rm(main1_idx);
                    mCallBackLists[reqIndex]->add(frame1);
                }
            }
            else
            {
                reqIndex++;
            }

            continue;
        }
        // *********************************************************************

        if (mOpenIds.size() < 2 && mSyncManagerReqLists.size() < 2)
        {
            MY_LOGD(openId, "mOpenIds:%d, mIVSyncImpReqLists:%d", (MUINT32)mOpenIds.size(),
                    (MUINT32)mSyncManagerReqLists.size());
            break;
        }

        // choice oldest one
        sp<SyncManagerReqItem> frame1 = mSyncManagerReqLists[0]->getItem(main1_idx);
        sp<SyncManagerReqItem> frame2 = mSyncManagerReqLists[1]->getItem(main2_idx);

        if (NULL == frame1.get() || NULL == frame2.get())
        {
            MY_LOGD(openId, "null frame (%p, %p)", frame1.get(), frame2.get());
            break;
        }

        /*IMetadata* main1 = frame1->mFeaturePipeParam.mVarMap.get<IMetadata*>("meta.inhal", NULL);
        IMetadata* main2 = frame2->mFeaturePipeParam.mVarMap.get<IMetadata*>("meta.inhal", NULL);

        if (NULL == main1 || NULL == main2)
        {
            MY_LOGD(openId, "null IMetadata (%p, %p)", main1, main2);
            break;
        }*/

        MINT64 timestamp_a = -1;
        MINT64 timestamp_b = -1;
        MINT32 DropReq1 = 0, DropReq2 = 0;
        timestamp_a = frame1->mFeaturePipeParam.mVarMap.get<MINT64>(VAR_DUALCAM_TIMESTAMP, -1);
        timestamp_b = frame2->mFeaturePipeParam.mVarMap.get<MINT64>(VAR_DUALCAM_TIMESTAMP, -1);
        DropReq1    = frame1->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DROP_REQ, -1);
        DropReq2    = frame2->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DROP_REQ, -1);

        MINT32 result = SYNC_MANAGER_SYNC_FAIL;
        if(skipSync)
        {
            result = SYNC_MANAGER_SYNC_OK;
        }
        else
        {
            MY_LOGD(openId,"frame1: [%d] frame2: [%d]", frame1->mRequestNo, frame2->mRequestNo);
            result = isSyncFrame(timestamp_a, timestamp_b);
        }

        switch (result)
        {
            case SYNC_MANAGER_SYNC_FAIL:
                result = BAD_VALUE;
                break;

            case SYNC_MANAGER_SYNC_FAIL_A_NO_TIMESTAMP:
            case SYNC_MANAGER_SYNC_FAIL_A:
                MY_LOGD(openId, "FAIL_A markDropFrame frameNo:%d", frame1->mRequestNo);
                //markDropFrame(frame1);
                if (!DropReq1)
                {
                    markEnqueFrame(frame1, NULL, false);
                }
                else
                {
                    markDropFrame(frame1);
                }
                mSyncManagerReqLists[0]->rm(main1_idx);
                mCallBackLists[0]->add(frame1);
                continue;

            case SYNC_MANAGER_SYNC_FAIL_B_NO_TIMESTAMP:
            case SYNC_MANAGER_SYNC_FAIL_B:
                MY_LOGD(openId, "FAIL_B markDropFrame frameNo:%d", frame2->mRequestNo);
                //markDropFrame(frame2);
                if (!DropReq2)
                {
                    markEnqueFrame(frame2, NULL, false);
                }
                else
                {
                    markDropFrame(frame2);
                }
                mSyncManagerReqLists[1]->rm(main2_idx);
                mCallBackLists[1]->add(frame2);
                continue;


            case SYNC_MANAGER_SYNC_OK:
            default:
                // To-Do wrap Tele FeaturePipeParam to Wide FeaturePipeParam's mVarMap
                MINT value = property_get_int32("vendor.debug.dualcam.pipmode", 0);
                if (value)
                {
                    markEnqueFrame(frame1, NULL, false);
                    markEnqueFrame(frame2, NULL, false);
                }
                else
                {
                    if (!DropReq1 && !DropReq2)
                    {
                        markEnqueFrame(frame1, NULL, false);

                        #if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
                        MINT32 zoomratio =
                            frame1->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_ZOOM_RATIO, 100);
                        if (zoomratio < DUALZOOM_SWICH_CAM_ZOOM_RATIO)
                        {
                            MY_LOGW(openId, "Drop !! zoomRatio:%d/%d is not good at FOV for display",
                                zoomratio, DUALZOOM_SWICH_CAM_ZOOM_RATIO);
                            frame2->mFeaturePipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
                            markDropFrame(frame2);
                        }
                        else
                        #endif
                        {
                            markEnqueFrame(frame2, NULL, false);
                        }
                    }
                    else if(DropReq1 && DropReq2)
                    {
                        MY_LOGW(openId, "two cam drop req same time");
                        frame1->mFeaturePipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
                        frame2->mFeaturePipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
                        markDropFrame(frame1);
                        markDropFrame(frame2);
                    }
                    else
                    {
                        if (forcesync)
                        {
                            frame1->mFeaturePipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DO_SYNC, 1);
                        }
                        if (DropReq2)
                        {
                            markEnqueFrame(frame1, frame2, true);
                        }
                        else
                        {
                            markEnqueFrame(frame2, frame1, true);
                        }
                    }
                }
                mSyncManagerReqLists[0]->rm(main1_idx);
                mSyncManagerReqLists[1]->rm(main2_idx);

                mCallBackLists[0]->add(frame1);
                mCallBackLists[1]->add(frame2);
                break;
        }

        result = OK;
        break;
    }
    while (1);

    //MY_LOGD(openId, "return %d", result);
    return result;
}


/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::processFrameCallback(MINT32 const openIndex, MBOOL isForceFlush)
{
    //MY_LOGD(openId, "+");

    if (0 <= openIndex && openIndex < (MINT32)mCallBackLists.size())
    {
        android::String8 msg = String8::format(",");
        for (MUINT i = 0 ; i < mCallBackLists[openIndex]->getListItemCount() ; i++)
        {
            sp<SyncManagerReqItem> frame = mCallBackLists[openIndex]->getItem(i);
            msg += String8::format("%d(%d) ", frame->mRequestNo, frame->mState);
        }
        MY_LOGD(-1, "id:%d => %s", mOpenIds[openIndex], msg.string());

        do
        {
            sp<SyncManagerReqItem> frame = mCallBackLists[openIndex]->getItem(0);

            if (frame.get() && (isForceFlush || SyncManagerReqItem::FINAL_CALLBACK <= frame->mState))
            {
                DualCamStreamingFeaturePipe* ptr = NULL;
                ptr = (DualCamStreamingFeaturePipe*)(frame->mCb);
                if (ptr)
                {
                    ptr->onCallback(frame->mOpenId, frame->mRequestNo, frame->mFeaturePipeParam,
                                    frame->mCallbackType,
                                    SyncManagerReqItem::FINAL_CALLBACK_WITH_DROP == frame->mState);
                }
                mCallBackLists[openIndex]->rm(0);
            }
            else
            {
                break;
            }
        }
        while (1);
    }
    else
    {
        MY_LOGW(-1, "openIndex:%d incorrect", openIndex);
        exit(1);
    }

    //MY_LOGD(openId, "-");
}

/******************************************************************************
 *
 ******************************************************************************/
status_t SyncManager::processFrameCallback()
{
    Mutex::Autolock _l(mRequsetLock);

    for (MUINT i = 0 ; i < mCallBackLists.size() ; i++)
    {
        processFrameCallback(i, false);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL SyncManager::isDropFrame(sp<SyncManagerReqItem> frame, MINT32 doDrop)
{
    MINT32 dropReq =
        frame->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DROP_REQ, 0);

    if (MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE > dropReq && (dropReq > MTK_DUALZOOM_DROP_NONE || dropReq != MTK_DUALZOOM_DROP_NEVER_DROP))
    {
        CAM_LOGW("[%d]id:%d frameNo:%d (%d)(drop_by_meta)", mInstanceRefCount,
            frame->mOpenId, frame->mRequestNo, dropReq);
        return true;
    }


    // check policy to drop frame which is not decided to display
    if (MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE == dropReq)
    {
        // do nothing
    }
    else if (doDrop)
    {
        MINT32 doSync = frame->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
        if (0 == doSync && (mPreviewNewID != frame->mOpenId) && (mPreviewID == mPreviewNewID))
        {
            // camera switched, another camera frame should be dropped
            CAM_LOGW("[%d]id:%d frameNo:%d, PrevId:%d (drop_by_policy)", mInstanceRefCount,
                frame->mOpenId, frame->mRequestNo, mPreviewNewID);
            return true;
        }
    }

    if ((dropReq == MTK_DUALZOOM_DROP_NONE||dropReq == MTK_DUALZOOM_DROP_NEVER_DROP) &&
        ((mPreviewID != mPreviewNewID) && (frame->mOpenId == mPreviewNewID)))
    {
        // got new frame from new id
        mPreviewID = mPreviewNewID;
        CAM_LOGD("[%d]id:%d frameNo:%d got_new_frame from new id(%d)", mInstanceRefCount,
                frame->mOpenId, frame->mRequestNo, mPreviewNewID);
    }

    return false;
}



/******************************************************************************
 *
 ******************************************************************************/
MINT32 SyncManager::isSyncFrame(MINT64 timestamp_a, MINT64 timestamp_b)
{
    MINT32 ret = SYNC_MANAGER_SYNC_FAIL;

    MINT32 timestamp_a_ms = timestamp_a / 1000000;
    MINT32 timestamp_b_ms = timestamp_b / 1000000;
    MINT32 timestamp_diff = timestamp_a_ms - timestamp_b_ms;

    if (timestamp_diff < -SYNC_MANAGER_MIN_ABS_VALUE)
    {
        ret = SYNC_MANAGER_SYNC_FAIL_A;
    }
    else if (timestamp_diff > SYNC_MANAGER_MIN_ABS_VALUE)
    {
        ret = SYNC_MANAGER_SYNC_FAIL_B;
    }
    else
    {
        ret = SYNC_MANAGER_SYNC_OK;
    }

    MY_LOGD(-1, "TS:(%09d/%09d/%09d)(ms)",
            timestamp_a_ms, timestamp_b_ms, timestamp_diff);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 SyncManager::isSyncFrame(IMetadata* a, IMetadata* b)
{
    MY_LOGD(-1, "%p, %p", a, b);

    MINT32 ret = SYNC_MANAGER_SYNC_FAIL;

    MINT64 timestamp_a = -1;
    MINT64 timestamp_b = -1;

    // fetch the metadata with timestamp
    if (!getMetadata<MINT64>(a, MTK_SENSOR_TIMESTAMP, timestamp_a))
    {
        ret = SYNC_MANAGER_SYNC_FAIL_A_NO_TIMESTAMP;
    }

    if (!getMetadata<MINT64>(b, MTK_SENSOR_TIMESTAMP, timestamp_b))
    {
        ret = SYNC_MANAGER_SYNC_FAIL_B_NO_TIMESTAMP;
    }

    if (timestamp_a == -1 || timestamp_b == -1)
    {
        MY_LOGE(-1, "can not get timestamp meta");
        MY_LOGD_IF(timestamp_a == -1, -1, "timestamp_a is -1");
        MY_LOGD_IF(timestamp_b == -1, -1, "timestamp_b is -1");
        return ret;
    }

    ret = isSyncFrame(timestamp_a, timestamp_b);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadata* SyncManager::getIMetadata(/*Vector<sp<MetaHandle>> metaVector*/)
{
    IMetadata* result = NULL;

    /*Vector<sp<MetaHandle>>::iterator it = metaVector.begin();
    Vector<sp<MetaHandle>>::iterator end = metaVector.end();
    MINT32 count = 0;
    while (it != end)
    {
        result = (*it)->getMetadata();
        if (NULL != result)
        {
            break;
        }
    }*/

    /*for (size_t i = 0; i < metaVector.size(); i++)
    {
        result = metaVector[i]->getMetadata();
        if (NULL != result)
        {
            break;
        }
    }*/

    return result;
}


/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::onCallback(FeaturePipeParam::MSG_TYPE type,
                             sp<SyncManagerReqItem> frame, FeaturePipeParam& org_callback_param)
{
    MY_LOGD(frame->mOpenId, "frameNo:%d type:%d +", frame->mRequestNo, type);

    Mutex::Autolock _l(mRequsetLock);

    // ************ for online FOVNode state notify ************
    MINT32 onLineFOV =
        org_callback_param.mVarMap.get<MINT32>(VAR_DUALCAM_FOV_ONLINE, -1);
    if (onLineFOV < 0)
    {
        onLineFOV = frame->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_FOV_ONLINE, -1);
    }

    if (onLineFOV > 0 && mIDualCamPolicy != NULL)
    {
        mIDualCamPolicy->setParameter(VAR_DUALCAM_FOV_ONLINE);
    }
    // ************ for bokeh deque result ************
    frame->mFeaturePipeParam.mQParams.mDequeSuccess =
                    org_callback_param.mQParams.mDequeSuccess;
    // ************ for bokeh deque result ************
    // ************ for online FOVNode state notify ************

    if (FeaturePipeParam::MSG_FRAME_DONE == type)
    {
        frame->mState = SyncManagerReqItem::FINAL_CALLBACK;
        frame->mCallbackType = type;

        // ignore enque's param, replace to the callback one
        //frame->mFeaturePipeParam = org_callback_param;

        MINT32 dropReq =
            frame->mFeaturePipeParam.mVarMap.get<MINT32>(VAR_DUALCAM_DROP_REQ, 0);
        if (dropReq == MTK_DUALZOOM_DROP_NONE)
        {
            dropReq = org_callback_param.mVarMap.get<MINT32>(VAR_DUALCAM_DROP_REQ, 0);
        }

        if (dropReq > MTK_DUALZOOM_DROP_NONE && dropReq != MTK_DUALZOOM_DROP_NEVER_DROP)
        {
            frame->mState = SyncManagerReqItem::FINAL_CALLBACK_WITH_DROP;
        }

        processFrameCallback(getIndexOfOpenIds(frame->mOpenId), false);
        //MY_LOGD(frame->mOpenId, "frameNo:%d -", frame->mRequestNo);
    }
    else
    {
        // just callback directly for partialRelease callback
        DualCamStreamingFeaturePipe* ptr = NULL;
        ptr = (DualCamStreamingFeaturePipe*)(frame->mCb);
        if (ptr)
        {
            // write shot mode if exist
            MUINT8 shotMode = 0;
            if(org_callback_param.tryGetVar<MUINT8>(VAR_N3D_SHOTMODE, shotMode))
            {
                frame->mFeaturePipeParam.setVar<MUINT8>(VAR_N3D_SHOTMODE, shotMode);
            }
            ptr->onCallback(frame->mOpenId, frame->mRequestNo, frame->mFeaturePipeParam,
                            type, false);
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL SyncManagerCallback(FeaturePipeParam::MSG_TYPE type, FeaturePipeParam& param)
{
    MBOOL result = false;

    wp<SyncManager> wpsync =
        param.mVarMap.get<wp<SyncManager>>(SYNC_SYNCMGR, NULL);
    wp<SyncManagerReqItem> wpframe =
        param.mVarMap.get<wp<SyncManagerReqItem>>(SYNC_REQITEM, NULL);
    wp<SyncManagerReqItem> wpslave =
        param.mVarMap.get<wp<SyncManagerReqItem>>(SYNC_REQITEM_SLAVE, NULL);


    sp<SyncManager> sync = wpsync.promote();
    sp<SyncManagerReqItem> frame = wpframe.promote();
    sp<SyncManagerReqItem> slave = wpslave.promote();


    // trigger callback to P2FeatureNode in-order
    if (sync.get() && frame.get())
    {
        sync->onCallback(type, frame, param);
    }
    else
    {
        CAM_LOGW("no wp<SyncManager> instance");
        exit(1);
    }


    // slave
    // trigger callback to P2FeatureNode in-order
    if (sync.get() && slave.get())
    {
        sync->onCallback(type, slave, slave->mFeaturePipeParam);
    }

    return result;
}

/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::markEnqueFrame(sp<SyncManagerReqItem> frame, sp<SyncManagerReqItem> slave,
                                 MBOOL isCombine)
{
    MY_LOGD(-1, "isCombine(%d)", isCombine);
    frame->mState = SyncManagerReqItem::PROCESSED_AND_WAIT_CALLBACK;

    // master camera
    // back up related parameter, use wp<> to avoid StreamFeaturePipe keep these and leak
    frame->mFeaturePipeParam.mVarMap.set<wp<SyncManager>>(SYNC_SYNCMGR, this);
    frame->mFeaturePipeParam.mVarMap.set<wp<SyncManagerReqItem>>(SYNC_REQITEM, frame);
    frame->mFeaturePipeParam.mVarMap.set<FeaturePipeParam::CALLBACK_T>(SYNC_CALLBACK,
            frame->mFeaturePipeParam.mCallback);
    frame->mFeaturePipeParam.mCallback = SyncManagerCallback;
    frame->mFeaturePipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_FOV_MASTER_ID, frame->mOpenId);

    if (isCombine && slave.get())
    {
        // attend slave's FeaturePipeParam to mVarMap *******************
        frame->mFeaturePipeParam.mVarMap.set<wp<SyncManagerReqItem>>(SYNC_REQITEM_SLAVE, slave);
        frame->mFeaturePipeParam.mVarMap.set<FeaturePipeParam>(VAR_DUALCAM_FOV_SLAVE_PARAM,
                slave->mFeaturePipeParam);
        // add slave id to master VarMap
        frame->mFeaturePipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_FOV_SLAVE_ID, slave->mOpenId);


        // change the frame state to wait master camera callback
        slave->mState = SyncManagerReqItem::PROCESSED_AND_WAIT_CALLBACK;

        /*if(mIDualCamPolicy != nullptr)
        {
            mIDualCamPolicy->updateFeatureMask(frame->mFeaturePipeParam.mFeatureMask);
        }*/

        // slave camera
        // back up related parameter, use wp<> to avoid StreamFeaturePipe keep these and leak
        slave->mFeaturePipeParam.mVarMap.set<wp<SyncManager>>(SYNC_SYNCMGR, this);
        slave->mFeaturePipeParam.mVarMap.set<wp<SyncManagerReqItem>>(SYNC_REQITEM, slave);
        slave->mFeaturePipeParam.mVarMap.set<FeaturePipeParam::CALLBACK_T>(SYNC_CALLBACK,
                slave->mFeaturePipeParam.mCallback);
        slave->mFeaturePipeParam.mCallback = SyncManagerCallback;
    }
    else
    {
        frame->mFeaturePipeParam.mVarMap.set<MINT32>(VAR_DUALCAM_DO_SYNC, 0);
    }


    // update to Policy before enque to StreamFeaturePipe
    mIDualCamPolicy->updateAfterP1(frame->mOpenId, frame->mRequestNo, &(frame->mFeaturePipeParam), 0);
    if (isCombine && slave.get())
    {
        mIDualCamPolicy->updateAfterP1(slave->mOpenId, slave->mRequestNo, &(slave->mFeaturePipeParam), 0);
    }


    // trigger StreamingFeaturePipe->enque(param);
    DualCamStreamingFeaturePipe* ptr = NULL;
    ptr = (DualCamStreamingFeaturePipe*)(frame->mCb);
    if (ptr)
    {
        ptr->onSynced(frame->mOpenId, frame->mRequestNo, frame->mFeaturePipeParam);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::markDropFrame(sp<SyncManagerReqItem> frame)
{
    frame->mState = SyncManagerReqItem::FINAL_CALLBACK_WITH_DROP;
    frame->mCallbackType = FeaturePipeParam::MSG_FRAME_DONE;
    frame->mFeaturePipeParam.mVarMap.set<FeaturePipeParam::CALLBACK_T>(SYNC_CALLBACK,
            frame->mFeaturePipeParam.mCallback);

    // update to Policy before call back to P2FeatureNode
    mIDualCamPolicy->updateAfterP1(frame->mOpenId, frame->mRequestNo, &(frame->mFeaturePipeParam), 0);


    // just callback directly for partialRelease (release RROZ/display) to avoid P1 blocking
    DualCamStreamingFeaturePipe* ptr = (DualCamStreamingFeaturePipe*)(frame->mCb);
    if (ptr)
    {
        ptr->onCallback(frame->mOpenId, frame->mRequestNo, frame->mFeaturePipeParam,
                        FeaturePipeParam::MSG_DISPLAY_DONE, true);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::requestExit()
{
    MY_LOGD(-1, "requestExit()");
    mIsExit = true;
    // must be in this order to avoid a race condition
    resume();
    Thread::requestExit();
}


/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::pause()
{
    MY_LOGD(-1, "pause()");
    mIsPaused = true;
}


/******************************************************************************
 *
 ******************************************************************************/
void SyncManager::resume()
{
    MY_LOGD(-1, "resume() ****************************************");
    mMyCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
bool SyncManager::threadLoop()
{
    AutoMutex _l(mMyLock);

    if (mIsExit)
    {
        MY_LOGW(-1, "mIsExit:true");
        return false;
    }

    if (mIsPaused)
    {
        MY_LOGD(-1, "mMyCond.wait()");
        mMyCond.wait(mMyLock);
        mIsPaused = false;
        return true;
    }

    // process frmae sync
    status_t result = processFrameSync(-1);

    processFrameCallback();

    if (result != OK)
    {
        //MY_LOGD(-1, "pause()");
        mIsPaused = true;
        return true;
    }

    return true;
}

} ///NSCam

