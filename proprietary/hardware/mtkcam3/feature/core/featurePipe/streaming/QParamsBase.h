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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_QPARAMS_BASE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_QPARAMS_BASE_H_

#include "QParamsBase_t.h"

#include <featurePipe/core/include/PipeLogHeaderBegin.h>
#include "DebugControl.h"
#define PIPE_TRACE TRACE_QPARAMS_BASE
#define PIPE_CLASS_TAG "QParamsBase"
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

template <typename T>
QParamsBase<T>::QParamsBase()
    : mCount(0)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

template <typename T>
QParamsBase<T>::~QParamsBase()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

template <typename T>
MBOOL QParamsBase<T>::onQParamsFailCB(const NSCam::NSIoPipe::QParams &param, const T &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret;
    ret = onQParamsCB(param, data);
    TRACE_FUNC_EXIT();
    return ret;
}

template <typename T>
MBOOL QParamsBase<T>::onQParamsBlockCB(const NSCam::NSIoPipe::QParams &param, const T &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret;
    ret = onQParamsCB(param, data);
    TRACE_FUNC_EXIT();
    return ret;
}

template <typename T>
void QParamsBase<T>::processCB(NSCam::NSIoPipe::QParams param, CB_TYPE type)
{
    TRACE_FUNC_ENTER();
    BACKUP_DATA_TYPE *backup = NULL;

    backup = checkOutBackup(param.mpCookie);
    if( !backup || !backup->mParent )
    {
        CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_STREAMING, "Cannot retrieve QParams data from backup=%p", backup);
    }
    else
    {
        backup->restore(param);
        switch(type)
        {
        case CB_DONE:
            backup->mParent->onQParamsCB(param, backup->mData);
            break;
        case CB_FAIL:
            param.mDequeSuccess = MFALSE;
            backup->mParent->onQParamsFailCB(param, backup->mData);
            break;
        case CB_BLOCK:
            param.mDequeSuccess = MFALSE;
            backup->mParent->onQParamsBlockCB(param, backup->mData);
            break;
        default:
            CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_STREAMING, "Unknown CB type = %d", type);
          break;
        }
        backup->mParent->signalDone();
    }
    if( backup )
    {
        delete backup;
        backup = NULL;
    }
    TRACE_FUNC_EXIT();
}

template <typename T>
void QParamsBase<T>::staticQParamsCB(NSCam::NSIoPipe::QParams &param)
{
    TRACE_FUNC_ENTER();
    processCB(param, CB_DONE);
    TRACE_FUNC_EXIT();
}

template <typename T>
void QParamsBase<T>::staticQParamsFailCB(NSCam::NSIoPipe::QParams &param)
{
    TRACE_FUNC_ENTER();
    processCB(param, CB_FAIL);
    TRACE_FUNC_EXIT();
}

template <typename T>
void QParamsBase<T>::staticQParamsBlockCB(NSCam::NSIoPipe::QParams &param)
{
    TRACE_FUNC_ENTER();
    processCB(param, CB_BLOCK);
    TRACE_FUNC_EXIT();
}

template <typename T>
MBOOL QParamsBase<T>::enqueQParams(NSCam::NSIoPipe::NSPostProc::INormalStream *stream, NSCam::NSIoPipe::QParams param, const T &data)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;
    BACKUP_DATA_TYPE *backup = NULL;
    if( stream == NULL )
    {
        CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_STREAMING, "Invalid stream: NULL");
        ret = MFALSE;
    }
    else if( (backup = new BACKUP_DATA_TYPE(this, param, data)) == NULL )
    {
        CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_STREAMING, "OOM: cannot allocate memory for backup data");
        param.mDequeSuccess = MFALSE;
        this->onQParamsFailCB(param, data);
    }
    else
    {
        param.mpCookie = (void*)backup;
        param.mpfnCallback = QParamsBase<T>::staticQParamsCB;
        param.mpfnEnQFailCallback = QParamsBase<T>::staticQParamsFailCB;
        param.mpfnEnQBlockCallback = QParamsBase<T>::staticQParamsBlockCB;
        signalEnque();
        if( !stream->enque(param) )
        {
            CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_STREAMING, "normal stream enque failed, backup=%p", backup);
            staticQParamsFailCB(param);
        }
    }
    TRACE_FUNC_EXIT();
    return ret;
}

template <typename T>
MVOID QParamsBase<T>::waitEnqueQParamsDone()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    while( mCount )
    {
        mCondition.wait(mMutex);
    }
    TRACE_FUNC_EXIT();
}

template <typename T>
MVOID QParamsBase<T>::signalDone()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    --mCount;
    mCondition.broadcast();
    TRACE_FUNC_EXIT();
}

template <typename T>
MVOID QParamsBase<T>::signalEnque()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock lock(mMutex);
    ++mCount;
    TRACE_FUNC_EXIT();
}

template <typename T>
typename QParamsBase<T>::BACKUP_DATA_TYPE* QParamsBase<T>::checkOutBackup(MVOID *handle)
{
    TRACE_FUNC_ENTER();
    BACKUP_DATA_TYPE *backup;
    backup = reinterpret_cast<BACKUP_DATA_TYPE*>(handle);
    if( backup )
    {
        if( backup->mMagic != MAGIC_VALID )
        {
            CAM_ULOGE(NSCam::Utils::ULog::MOD_FPIPE_STREAMING, "Backup data is corrupted: backup=%p magic=0x%x", backup, backup->mMagic);
        }
        else
        {
            backup->mMagic = MAGIC_USED;
        }
    }
    TRACE_FUNC("get backup=%p", backup);
    TRACE_FUNC_EXIT();
    return backup;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#include <featurePipe/core/include/PipeLogHeaderEnd.h>
#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_QPARAMS_BASE_H_
