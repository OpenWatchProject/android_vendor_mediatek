/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Acodec_vilte.cpp
 *
 * Project:
 * --------
 *   MT6573
 *
 * Description:
 * ------------
 *   Vilte support
 *
 ****************************************************************************/
//#define LOG_NDEBUG 0
#ifdef MTK_ROI_SUPPORT
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "ACodec_roi"

#include <utils/Log.h>
#include <ui/GraphicBuffer.h>
#include <ui/Fence.h>
#include <media/openmax/OMX_Core.h>
#include <media/openmax/OMX_IndexExt.h>
#include <media/openmax/OMX_Video.h>
#include <media/openmax/OMX_VideoExt.h>

#include <media/stagefright/ACodec.h>
#include <media/stagefright/omx/OMXUtils.h>
#include "Utils_MTK.h"
#include "stagefright/MediaDefs_MTK.h"

#include "MtkRoi.h"

namespace android {

    enum {
        kPortIndexInput  = 0,
        kPortIndexOutput = 1
    };

    status_t setRoiOn(const sp<IOMXNode> &spNode, const sp<AMessage> &msg, int32_t &mRoiOnMode)
    {
        status_t err = OK;

        // don't repeat set roi-on
        if(mRoiOnMode != 0) return err;

        int32_t thirdPartyRoiOn = 0, platformRoiOn = 0;
        if(msg->findInt32("roi-on", &thirdPartyRoiOn))
        {
            ALOGI("Get roi-on %d", thirdPartyRoiOn);
        }

        if(msg->findInt32("proi-on", &platformRoiOn))
        {
            ALOGI("Get platform-roi-on %d", platformRoiOn);
        }

        /*0:disable, 1:platform solution (platform first) 2~3:apk solution*/
        if(thirdPartyRoiOn >= 1) mRoiOnMode = thirdPartyRoiOn+1;
        if(platformRoiOn == 1) mRoiOnMode = 1;

        // Roi is disabled
        if(mRoiOnMode == 0) return err;

        OMX_INDEXTYPE index = OMX_IndexVendorMtkOmxVencRoiSwitch;
        err = spNode->getExtensionIndex(
            "OMX.MTK.index.param.video.roi.switch",
            &index);

        if(err == OK)
        {
            OMX_PARAM_U32TYPE mRoiSwitch;
            InitOMXParams(&mRoiSwitch);
            mRoiSwitch.nPortIndex = kPortIndexOutput;
            mRoiSwitch.nU32 = mRoiOnMode;

            err = spNode->setParameter(
                index,
                &mRoiSwitch,
                sizeof(mRoiSwitch));
        }

        if(err != OK)
        {
            mRoiOnMode = 0;
        }

        return err;
    }

    status_t setRoiLease(const sp<IOMXNode> &spNode, const sp<AMessage> &msg, int32_t &mRoiOnMode)
    {
        status_t err = OK;

        if(mRoiOnMode != 1) return err;

        OMX_INDEXTYPE indexSize, indexLease;
        AString mLicenseString;

        err = spNode->getExtensionIndex(
            "OMX.MTK.index.param.video.roi.license.size",
            &indexSize);

        err = spNode->getExtensionIndex(
            "OMX.MTK.index.param.video.roi.license",
            &indexLease);

        if(err != OK) return err;

        if(msg->findString("roi-lease", &mLicenseString))
        {
            ALOGI("Get roi-lease");
            mLicenseString.trim();

            OMX_PARAM_U32TYPE mLicenseStringSize;
            InitOMXParams(&mLicenseStringSize);
            mLicenseStringSize.nPortIndex = kPortIndexOutput;
            mLicenseStringSize.nU32 = mLicenseString.size();

            err = spNode->setParameter(
                indexSize,
                &mLicenseStringSize,
                sizeof(mLicenseStringSize));

            err = spNode->setParameter(
                indexLease,
                mLicenseString.c_str(),
                mLicenseString.size()+1);
        }

        return err;
    }

    status_t setRoiInfo(const sp<IOMXNode> &spNode, const sp<AMessage> &msg, int32_t &mRoiOnMode)
    {
        status_t err = OK;

        if(mRoiOnMode != 2 && mRoiOnMode != 3) return err;

        int32_t count = 0;
        if(msg->findInt32("roi-count", &count))
        {
            ALOGI("Get roi-count %d", count);
        }

        if(count < 0 || count > 32) return err;

        AString mAstring;

        if(msg->findString("roi-rect", &mAstring))
        {
            mAstring.trim();

            ALOGI("Get buffer roi-rect: %s %zu", mAstring.c_str(), mAstring.size());

            OMX_VIDEO_CONFIG_ROI_INFO mRoiInfo;
            InitOMXParams(&mRoiInfo);
            mRoiInfo.nRoiInfoSize = count;
            mRoiInfo.nRoiStringSize = mAstring.size();
            strncpy((char*)mRoiInfo.pRoiInfoCheck, (char*)mAstring.c_str(), sizeof(mRoiInfo.pRoiInfoCheck));

            err = spNode->setConfig(
                OMX_IndexVendorMtkOmxVencRoiSize,
                &mRoiInfo,
                sizeof(mRoiInfo));

            err = spNode->setConfig(
                OMX_IndexVendorMtkOmxVencRoiInfo,
                mAstring.c_str(),
                mAstring.size()+1);
        }

        return err;
    }

    MtkRoi::MtkRoi()
    {
        mRoiOnMode = 0;
    }

    status_t MtkRoi::setRoiParameters(const sp<IOMXNode> &spNode, const sp<AMessage> &msg)
    {
        status_t err = OK;

        ALOGI("+ %s %d", __func__, __LINE__);

        setRoiOn(spNode, msg, mRoiOnMode);
        setRoiLease(spNode, msg, mRoiOnMode);
        setRoiInfo(spNode, msg, mRoiOnMode);

        ALOGI("- %s %d", __func__, __LINE__);
        return err;
    }

}  // namespace android

#endif
