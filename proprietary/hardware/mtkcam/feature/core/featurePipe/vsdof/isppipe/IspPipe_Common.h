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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

/**
 * @file IspPipe_Common.h
 * @brief Common utility for IspPipe
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_COMMON_H_
#define _MTK_CAMERA_FEATURE_PIPE_ISP_PIPE_COMMON_H_

// Standard C header file
#include <string>
// Android system/core header file

// mtkcam custom header file
// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
// Module header file
#include <mtkcam/feature/stereo/pipe/IIspPipe.h>
#include <mtkcam/utils/metadata/IMetadata.h>
// Local header file

// Logging log header
#define PIPE_MODULE_TAG "IspPipe"
#define PIPE_CLASS_TAG "Common"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

using namespace NSIoPipe;
/*******************************************************************************
* Global Definition
********************************************************************************/
MBOOL isEISOn(IMetadata* const inApp);
/******************************************************************************
* Enum Definition
********************************************************************************/
/**
  * @brief Node ID inside the IspPipe
 */
typedef enum eIspPipeNodeID {
    //
    eISP_PIPE_NODEID_END
};
/**
  * @brief Data ID used in handleData inside the IspPipe
 */
enum eIspPipeDataID {
    ID_INVALID,
    ROOT_ENQUE,
    ERROR_OCCUR_NOTIFY,
    TO_DUMP_RAWS,
    //
    eISP_PIPE_DATAID_END
};
typedef MINT32 IspPipeNodeID;
typedef MINT32 IspPipeDataID;
/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IspPipeLoggingSetup
 * @brief Control the logging enable of the isp pipe
 */
class IspPipeLoggingSetup
{
public:
    static MBOOL mbProfileLog;
    static MBOOL mbDebugLog;
};

typedef enum ISPBufferDataTypes_Internal{
    BID_INVALID = PUBLIC_PBID_START,
    // ======== public id section start ==========
    BID_IN_FSRAW_MAIN1 = PBID_IN_FULL_RAW_MAIN1,
    BID_IN_FSRAW_MAIN2 = PBID_IN_FULL_RAW_MAIN2,
    BID_IN_RSRAW_MAIN1 = PBID_IN_RESIZE_RAW_MAIN1,
    BID_IN_RSRAW_MAIN2 = PBID_IN_RESIZE_RAW_MAIN2,
    BID_IN_LCSO_MAIN1 = PBID_IN_LCSO_MAIN1,
    BID_IN_LCSO_MAIN2 = PBID_IN_LCSO_MAIN2,
    // exist only in capture
    BID_IN_RESIZE_YUV_MAIN1 = PBID_IN_RESIZE_YUV_MAIN1,
    BID_IN_RESIZE_YUV_MAIN2 = PBID_IN_RESIZE_YUV_MAIN2,
    BID_IN_FULLSIZE_YUV_MAIN1 = PBID_IN_FULLSIZE_YUV_MAIN1,
    BID_IN_FULLSIZE_YUV_MAIN2 = PBID_IN_FULLSIZE_YUV_MAIN2,

    BID_META_IN_APP = PBID_IN_APP_META,
    BID_META_IN_HAL_MAIN1 = PBID_IN_HAL_META_MAIN1,
    BID_META_IN_HAL_MAIN2 = PBID_IN_HAL_META_MAIN2,
    BID_META_OUT_APP = PBID_OUT_APP_META, //10
    BID_META_OUT_HAL = PBID_OUT_HAL_META,
    //
    BID_PIPE_OUT_PV_YUV0 = PBID_OUT_PV_YUV0,
    BID_PIPE_OUT_PV_YUV1 = PBID_OUT_PV_YUV1,
    BID_PIPE_OUT_PV_FD =  PBID_OUT_PV_FD,
    BID_PIPE_OUT_BOKEH_YUV = PBID_OUT_BOKEH_YUV,
    BID_PIPE_OUT_DEPTH = PBID_OUT_THIRDPARTY_DEPTH,
    BID_MDP_OUT_THUMBNAIL_YUV = PBID_OUT_BOKEH_THUMBNAIL_YUV,
} InternalIspPipeBufferID;


#undef PIPE_LOGD
#undef PIPE_PRFLOG
// logging macro
#define PIPE_LOGD(fmt, arg...) \
    do { if(IspPipeLoggingSetup::mbDebugLog) {MY_LOGD("%d: " fmt, __LINE__, ##arg);} } while(0)

typedef std::string (*PnLoggingPrefix)(std::string tag, int reqID);

class ProfileUtil;
class AutoProfileLogging
{
public:
    AutoProfileLogging() = delete;

    AutoProfileLogging(
            const char* blockName);

    AutoProfileLogging(
            const char* blockName,
            int reqID);

    AutoProfileLogging(
            const char* blockName,
            int reqID,
            const char* fmt,
            ...);
    ~AutoProfileLogging();

    MVOID logging(const char* fmt, ...);
public:
    bool mbForceError;
    int mReqID;
    std::string msTag;
};

MVOID debugQParams(const QParams& rInputQParam);


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

MBOOL calcDstImageCrop(
    MSize const &srcSize,
    MSize const &dstSize,
    MCropRect &result
);


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif