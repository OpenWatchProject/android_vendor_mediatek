/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/SyncHelperBase"
//
#include <mtkcam/utils/std/Log.h>

#include "SyncHelperBase.h"

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//
#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

using namespace NSCam::v3::Utils::Imp;

/******************************************************************************
 *
 ******************************************************************************/

android::sp<ISyncHelperBase> ISyncHelperBase::createInstance() {

    return new SyncHelperBase();
}

/******************************************************************************
 *
 ******************************************************************************/

SyncHelperBase::SyncHelperBase()
    :mUserCounter(0)
{}

SyncHelperBase::~SyncHelperBase()
{
}


/******************************************************************************
 *
 ******************************************************************************/
status_t SyncHelperBase::start(int CamId) {
    status_t err = NO_ERROR;
    mUserCounter++;
    mContextMap.add(CamId, shared_ptr<SyncContext>(new SyncContext()));
    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t SyncHelperBase::stop(int CamId) {
    status_t err = NO_ERROR;
    mUserCounter--;
    if (mUserCounter == 0)
        mContextMap.clear();
    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t SyncHelperBase::init(int CamId) {
    status_t err = NO_ERROR;

    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t SyncHelperBase::uninit(int CamId) {
    status_t err = NO_ERROR;

    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t SyncHelperBase::syncEnqHW(SyncParam &sParam) {
    status_t err = NO_ERROR;
    size_t index = 0;
    int  waitCount = 0;

mSyncQLock.lock();
    for (std::vector<int>::iterator it = sParam.mSyncCams.begin();
        it != sParam.mSyncCams.end(); ++it) {
        index = 0;
        for (int cid : mSyncQueue) {

            if (*it == cid)
                break;
            index++;
        }

        if (index == mSyncQueue.size()) {
            if (waitCount == 0)
                mSyncQueue.push_back(sParam.mCamId);
            waitCount++;
        }


    }
    if (waitCount == 0) {
            for (std::vector<int>::iterator it = sParam.mSyncCams.begin() ;
            it != sParam.mSyncCams.end(); ++it) {
            shared_ptr<SyncContext> context = mContextMap.valueFor(*it);
            index = 0;
            for (int cid : mSyncQueue) {

                if (*it == cid) {
                    mSyncQueue.erase(mSyncQueue.begin()+index);
                    MY_LOGD("SyncQ Erase CamID = %d!\n", *it);
                    break;
                }
                index++;
            }
        }
    }
mSyncQLock.unlock();

    if (waitCount != 0) {
        shared_ptr<SyncContext> context = mContextMap.valueFor(sParam.mCamId);
        //printf("%" PRIu64 "\n", t);
        MY_LOGD("CamID = %d wait+\n", sParam.mCamId);
        sem_wait(&(context->mSyncSem));
        MY_LOGD("CamID = %d wait-\n", sParam.mCamId);
    }
    else {
        for (std::vector<int>::iterator it = sParam.mSyncCams.begin() ;
            it != sParam.mSyncCams.end(); ++it) {
            shared_ptr<SyncContext> context = mContextMap.valueFor(*it);
            sem_post(&(context->mSyncSem));
            MY_LOGD("CamID = %d, post = %d!\n", sParam.mCamId, *it);
        }

    }

    return err;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t SyncHelperBase::syncResultCheck(SyncParam &sParam) {
    status_t err = NO_ERROR;
    size_t index = 0;
    bool syncResult = true;

    shared_ptr<SyncContext> context1;
    shared_ptr<SyncContext> context2;

    int  waitCount = 0;

    auto getResult = [] (int64_t time1, int64_t time2, int64_t tolerance) -> bool {

        uint64_t diff = 0;
        if (time1 > time2)
            diff = time1 - time2;
        else
            diff = time2 - time1;

        if (diff<=tolerance)
            return true;
        else
            return false;

    };

mResultQLock.lock();
    for (std::vector<int>::iterator it = sParam.mSyncCams.begin();
        it != sParam.mSyncCams.end(); ++it) {
        index = 0;
        for (int cid : mResultQueue) {

            if (*it == cid)
                break;
            index++;
        }

        if (index == mResultQueue.size()) {
            if (waitCount == 0)
                mResultQueue.push_back(sParam.mCamId);
            waitCount++;
        }
    }
    if (waitCount == 0) {
            for (std::vector<int>::iterator it = sParam.mSyncCams.begin() ;
            it != sParam.mSyncCams.end(); ++it) {
            shared_ptr<SyncContext> context = mContextMap.valueFor(*it);
            index = 0;
            for (int cid : mResultQueue) {

                if (*it == cid) {
                    mResultQueue.erase(mResultQueue.begin()+index);
                    MY_LOGD("SyncQ Erase CamID = %d!\n", *it);
                    break;
                }
                index++;
            }
        }
    }
mResultQLock.unlock();

    context1 = mContextMap.valueFor(sParam.mCamId);
    context1->mResultTimeStamp = sParam.mReslutTimeStamp;

    if (waitCount != 0) {

        MY_LOGD("CamID = %d wait+\n", sParam.mCamId);
        sem_wait(&(context1->mResultSem));
        MY_LOGD("CamID = %d wait-\n", sParam.mCamId);

        for (std::vector<int>::iterator it = sParam.mSyncCams.begin() ;
            it != sParam.mSyncCams.end(); ++it) {
            context2 = mContextMap.valueFor(*it);
            syncResult &= getResult(context1->mResultTimeStamp,
                context2->mResultTimeStamp, sParam.mSyncTolerance);

            MY_LOGD("CamID = %d, time1=%" PRIu64 ", ret = %d, synID = %d, time2=%" PRIu64 ",\
                tolerance=%" PRIu64 "\n", sParam.mCamId, context1->mResultTimeStamp, syncResult,
                *it, context2->mResultTimeStamp, sParam.mSyncTolerance);
        }

    }
    else {
        for (std::vector<int>::iterator it = sParam.mSyncCams.begin() ;
            it != sParam.mSyncCams.end(); ++it) {
            context2 = mContextMap.valueFor(*it);
            syncResult &= getResult(context1->mResultTimeStamp,
                context2->mResultTimeStamp, sParam.mSyncTolerance);
            sem_post(&(context2->mResultSem));

            MY_LOGD("CamID = %d, time1=%" PRIu64 ", ret = %d, synID = %d, time2=%" PRIu64 "\
                tolerance=%" PRIu64 "\n", sParam.mCamId, context1->mResultTimeStamp, syncResult,
                *it, context2->mResultTimeStamp, sParam.mSyncTolerance);
        }
    }

    sParam.mSyncResult = syncResult;

    return err;
}
/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *
 ******************************************************************************/
