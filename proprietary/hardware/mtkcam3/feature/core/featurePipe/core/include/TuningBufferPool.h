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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_TUNING_BUFFER_POOL_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_TUNING_BUFFER_POOL_H_

#include <utils/Mutex.h>
#include "./BufferPool.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class TuningBufferPool;

enum TuningBufSyncCtrl
{
    eTUNINGBUF_CACHECTRL_FLUSH,
    eTUNINGBUF_CACHECTRL_SYNC
};

class TuningBufferHandle: public BufferHandle<TuningBufferHandle>
{
public:
    TuningBufferHandle(const android::sp<BufferPool<TuningBufferHandle> > &pool);
public:
    MVOID* mpVA;

private:
    friend class TuningBufferPool;
};

typedef sb<TuningBufferHandle> SmartTuningBuffer;

class TuningBufferPool : public BufferPool<TuningBufferHandle>
{
public:
    enum BufProtect
    {
        BUF_PROTECT_NONE,
        BUF_PROTECT_RUN,
    };
    static android::sp<TuningBufferPool> create(const char *name, MUINT32 size, BufProtect bufProtect = BUF_PROTECT_NONE);
    static MVOID destroy(android::sp<TuningBufferPool> &pool);
    virtual ~TuningBufferPool();
    MUINT32 getBufSize() {return miBufSize;}

protected:
    TuningBufferPool(const char *name, MBOOL bufProtect);
    MBOOL init(MUINT32 size);
    MVOID uninit();
    virtual android::sp<TuningBufferHandle> doAllocate();
    virtual MBOOL doRelease(TuningBufferHandle* handle);

private:
#if 0
    MERROR ionAllocate(android::sp<TuningBufferHandle>& buffHandle);
#endif
    android::Mutex mMutex;
    MUINT32 miBufSize;
    MBOOL   mIsBufProtect;
};

} //NSFeaturePipe
} //NSCamFeature
} //NSCam


#endif
