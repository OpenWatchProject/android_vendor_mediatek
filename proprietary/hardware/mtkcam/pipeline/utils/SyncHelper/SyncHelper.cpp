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

#define LOG_TAG "MtkCam/SyncHelper"
//
#include <mtkcam/utils/std/Log.h>

#include "SyncHelper.h"

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

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

using namespace NSCam::v3::Utils::Imp;

/******************************************************************************
 *
 ******************************************************************************/

android::sp<ISyncHelper> ISyncHelper::createInstance() {

    return new SyncHelper();
}

/******************************************************************************
 *
 ******************************************************************************/

status_t SyncHelper::syncEnqHW(int CamId,  IMetadata* HalControl) {
    /*Check SyncID MTK_FRAMESYNC_ID*/
    status_t err = NO_ERROR;
    MINT32 syncCamId = 0;
    MINT32 syncCam = 0;
    SyncParam syncParam;

    if (HalControl == NULL) {
        return false;
    }
    //
    IMetadata::IEntry entry = HalControl->entryFor(MTK_FRAMESYNC_ID);

    syncCam = entry.count();

    for (int i=0; i<syncCam ; i++) {
        syncParam.mSyncCams.push_back(entry.itemAt(i, Type2Type<MINT32>()));
    }

    if (tryGetMetadata<MINT32>(HalControl, MTK_FRAMESYNC_ID, CamId)) {
        syncParam.mCamId = CamId;
        SyncHelperBase::syncEnqHW(syncParam);
    }

    return err;

}
/******************************************************************************
 *
 ******************************************************************************/
bool SyncHelper::syncResultCheck(int CamId, IMetadata* HalControl, IMetadata* HalDynamic) {
    bool syncResult = true;
    MINT32 syncCam = 0;
    SyncParam syncParam;

    IMetadata::IEntry entry = HalControl->entryFor(MTK_FRAMESYNC_ID);

    syncCam = entry.count();

    if (syncCam > 0) {
        syncParam.mCamId = CamId;

        for (int i=0; i<syncCam; i++)
            syncParam.mSyncCams.push_back(entry.itemAt(i, Type2Type<MINT32>()));

        tryGetMetadata<MINT64>(HalControl, MTK_FRAMESYNC_TOLERANCE, syncParam.mSyncTolerance);
        tryGetMetadata<MUINT8>(HalControl, MTK_FRAMESYNC_FAILHANDLE, syncParam.mSyncFailHandle);

        SyncHelperBase::syncResultCheck(syncParam);

        if (syncParam.mSyncFailHandle == MTK_FRAMESYNC_FAILHANDLE_CONTINUE)  {
            if (syncParam.mSyncResult)
                trySetMetadata<MUINT8>(HalDynamic, MTK_FRAMESYNC_RESULT, MTK_FRAMESYNC_RESULT_PASS);
            else {
                trySetMetadata<MUINT8>(HalDynamic, MTK_FRAMESYNC_RESULT, MTK_FRAMESYNC_RESULT_FAIL_CONTINUE);
                syncResult = false;
            }
        }
        else if (syncParam.mSyncFailHandle == MTK_FRAMESYNC_FAILHANDLE_DROP) {
            if (syncParam.mSyncResult)
                trySetMetadata<MUINT8>(HalDynamic, MTK_FRAMESYNC_RESULT, MTK_FRAMESYNC_RESULT_PASS);
            else {
                trySetMetadata<MUINT8>(HalDynamic, MTK_FRAMESYNC_RESULT, MTK_FRAMESYNC_RESULT_FAIL_DROP);
                syncResult = false;
            }
        }

        trySetMetadata<MINT64>(HalDynamic, MTK_FRAMESYNC_RESULT, syncParam.mSyncResult);
    }

    return syncResult;
}



/******************************************************************************
 *
 ******************************************************************************/
