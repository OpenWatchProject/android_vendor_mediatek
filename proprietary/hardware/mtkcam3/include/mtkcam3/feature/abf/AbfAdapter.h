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

#ifndef __ABF_ADAPTER_H__
#define __ABF_ADAPTER_H__

#include <utils/RefBase.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>


class MTKABF; // in MTKABF.h
struct ABFInitInfo;
struct ABFTuningInfo;
struct ABFImageInfo;


class AbfAdapter : public virtual android::RefBase
{
public:
    enum {
        DEBUG_INPUT = 0x1,
        DEBUG_OUTPUT = 0x2,
        DEBUG_INTERMEDIATE = 0x4,
        DEBUG_PROFILE = 0x8,
        DEBUG_ALWAYS_RUN = 0x10,
        DEBUG_NEVER_RUN = 0x20,
        DEBUG_UT = 0x1000
    };

    struct ProcessParam {
        MINT32 iso;

        ProcessParam() : iso(100) { }
    };

    static android::sp<AbfAdapter> createInstance(MUINT32 openId, unsigned int debugFlags = 0);

    AbfAdapter(MUINT32 openId, MUINT32 debugFlags = 0);
    virtual ~AbfAdapter();

    bool needAbf(const ProcessParam &param);
    bool process(const ProcessParam &param, NSCam::IImageBuffer *pIOBuffer);

protected:
    MUINT32 mOpenId;
    unsigned int mDebugFlags;

    MTKABF *mpAbfAlgo;

    virtual void* getTuningData(const ProcessParam &param);
    void prepareInitInfo(const ProcessParam &param, ABFInitInfo *initInfo);
    static bool isPaddingInLines(NSCam::IImageBuffer *pIOBuffer);

    static int printInt32Array(char *buffer, int bufferSize, const MINT32 *array, int len);
    void debugPrint(const ABFTuningInfo &tuningInfo, int iso);
    void debugPrint(const ABFImageInfo &imageInfo);
    void debugSaveImage(NSCam::IImageBuffer *pBuffer, unsigned int source);
};

#endif

