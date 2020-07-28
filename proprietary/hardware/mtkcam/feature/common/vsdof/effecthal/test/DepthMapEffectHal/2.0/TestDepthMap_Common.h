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

#include <mtkcam/utils/std/Log.h>
#include <utils/String8.h>
#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam/feature/stereo/pipe/IDepthMapEffectRequest.h>
#include <camera_custom_stereo.h>
#include <vector>


using namespace NSCam;
using namespace NSCam::NSCamFeature::NSFeaturePipe_DepthMap;

#define MY_LOGV(fmt, arg...)                  CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)                  CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)                  CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)                  CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)                  CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD_WITH_OPENID(fmt, arg...)      CAM_LOGD("[%s] (%d) " fmt, __FUNCTION__, getOpenId(), ##arg)

#define CONNECT_DATA_AND_SETUPDUMP(srcNode, tarNode, dataID)\
{\
    srcNode->connectData(dataID, dataID, tarNode);\
    property_set(srcNode->getName(), "1");\
    char prop_name[256];\
    snprintf(prop_name, 256, "%s.%s", srcNode->getName(), DepthMapDataHandler::ID2Name(dataID));\
    property_set(prop_name, "1");\
}

#define SETUP_DUMP(srcNode, dataID)\
{\
    property_set(srcNode->getName(), "1");\
    char prop_name[256];\
    snprintf(prop_name, 256, "%s.%s", srcNode->getName(), DepthMapDataHandler::ID2Name(dataID));\
    property_set(prop_name, "1");\
}

#define SETUP_DUMP_REQ_START(start, size)\
{\
    ::property_set("vendor.depthmap.pipe.dump.size", size);\
    ::property_set("vendor.depthmap.pipe.dump.start", start);\
}

enum TestReqType
{
    eREQTYPE_PV,
    eREQTYPE_VR,
    eREQTYPE_CAP
    
};

IImageBuffer* createImageBufferFromFile(const IImageBufferAllocator::ImgParam imgParam, const char* path, const char* name, MINT usage);

IImageBuffer* createEmptyImageBuffer(const IImageBufferAllocator::ImgParam imgParam, const char* name, MINT usage, MBOOL is_gb=MFALSE);


class WaitingListener
{
public:
    static android::Mutex sMutex;
    static int sCBCount;
    static int sDoneCount;
    static String8 sDoneStatus;
    static String8 sNotReadyStatus;

public:
    static MVOID CB(MVOID* tag, android::String8 status, sp<EffectRequest>& request);
    static bool waitRequest(unsigned int targetTimes, unsigned int timeout_sec);
    static bool waitRequestAtLeast(unsigned int targetTimes, unsigned int timeout_sec, float least_wait_sec);
    static void resetCounter();
};

// finish CB which will be used to notifiy the WaitingListener
MVOID finishCB_forListener(bool status);

MBOOL setupReqMetadata(MBOOL eisON, sp<IDepthMapEffectRequest> pRequest);
MVOID releaseMetadata(std::vector<sp<IDepthMapEffectRequest> > vRequestVec);
MBOOL prepareReqInputBuffer(sp<IDepthMapEffectRequest> pRequest);
MBOOL prepareReqOutputBuffer(TestReqType reqType, sp<IDepthMapEffectRequest> pRequest);


/**
 * @brief Try to get metadata value
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to retrieve
 * @param [out] rVal the metadata value to be stored.
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/**
 * @brief Try to set metadata value
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to configure
 * @param [in] rVal the metadata value to set
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/**
 * @brief update the metadata entry
 * @param [in] pMetadata IMetadata instance
 * @param [in] tag the metadata tag to update
 * @param [in] rVal the metadata entry value
 * @return
 * - MTRUE indicates success.
 * - MFALSE indicates failure.
 */
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}
