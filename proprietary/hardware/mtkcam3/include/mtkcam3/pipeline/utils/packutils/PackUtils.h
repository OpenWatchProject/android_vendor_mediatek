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

#ifndef _MTKCAM_ISPPACK_UTILIIIES_H_
#define _MTKCAM_ISPPACK_UTILIIIES_H_

#include <cutils/properties.h>
#include <utils/String8.h>
#include <utils/Vector.h>
#include <set>
#include <stdio.h>
#include <string.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Sync.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
//

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam3/feature/bsscore/IBssCore.h>
#include <mtkcam3/feature/ainr/IAinrFeFm.h>
#include <mtkcam/utils/std/Trace.h>

using namespace android;
using namespace NSCam;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

/******************************************************************************
 *  IspTuningDataFilter
 ******************************************************************************/
class IspTuningDataFilter
{
public:
    ///////////////////////////////////////////////////////////
    // common meta white list define
    std::set<mtk_camera_metadata_tag> const appControlMetaCommonWhiteList = {
            MTK_CONTROL_CAPTURE_INTENT,
            MTK_JPEG_GPS_COORDINATES,
            MTK_JPEG_GPS_PROCESSING_METHOD,
            MTK_JPEG_GPS_TIMESTAMP,
            MTK_JPEG_ORIENTATION,
            MTK_JPEG_QUALITY,
            MTK_JPEG_THUMBNAIL_QUALITY,
            MTK_JPEG_AVAILABLE_THUMBNAIL_SIZES,
            MTK_JPEG_MAX_SIZE,
            MTK_JPEG_SIZE,
            MTK_CONTROL_CAPTURE_PACKED_RAW_ENABLE,
            MTK_CONTROL_CAPTURE_RAW_BPP,
            MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING,
            MTK_NOISE_REDUCTION_MODE
        };
    std::set<mtk_camera_metadata_tag> const appResultMetaCommonWhiteList = {
            //TODO: add the metadata that needed be reserved
        };
    std::set<mtk_platform_metadata_tag_t> const halResultMetaCommonWhiteList = {
            //TODO: add the metadata that needed be reserved
        };
    //
    IspTuningDataFilter();
    ~IspTuningDataFilter();
    //
    MERROR getTuningDataWhiteList(
            std::set<IMetadata::Tag_t>& outCtrlAppMetaFinalWhiteList,
            std::set<IMetadata::Tag_t>& outResultAppMetaFinalWhiteList,
            std::set<IMetadata::Tag_t>& outResultHalMetaFinalWhiteList,
            std::set<IMetadata::Tag_t> const& inIspTuningCtrlAppWhiteList = std::set<IMetadata::Tag_t>(),
            std::set<IMetadata::Tag_t> const& inIspTuningResultAppWhiteList = std::set<IMetadata::Tag_t>(),
            std::set<IMetadata::Tag_t> const& inIspTuningResultHalWhiteList = std::set<IMetadata::Tag_t>()
            );
};

/******************************************************************************
 *  ISP Hal Reprocessing Utility
 ******************************************************************************/
class IspTuningDataPackUtil
{
public:
    typedef struct
    {
        MUINT32 requestNo = 0;
        MUINT32 frameNo = 0;
        int64_t timestamp = 0;
        int64_t haltimestamp = 0;
        IMetadata appControlMeta;
        IMetadata appResultMeta;
        IMetadata halResultMeta;
        android::sp<IImageBufferHeap> pSttoImageBufferHeap;
        android::sp<IImageBufferHeap> pRrzoImageBufferHeap;
        android::sp<IImageStreamInfo> pRrzoStreamInfo;
        MUINT8 *pFDData;
        MUINT32 FDDataSize;
        MUINT8 *pSTTData;
        MUINT32 STTDataSize;
        //std::set<mtk_platform_metadata_tag_t> halResultMetaWhiteList; //reserved
    } PackInputParam;

    typedef struct
    {
        int64_t haltimestamp = 0;
        IMetadata appControlMeta;
        IMetadata appResultMeta;
        IMetadata halResultMeta;
        android::sp<IImageBuffer> pSttoImageBuffer = NULL;
        android::sp<IImageBuffer> pRrzoImageBuffer = NULL;
        std::shared_ptr<std::vector<MUINT8>> pFDData = NULL;
        std::shared_ptr<std::vector<MUINT8>> pSTTData = NULL;
    } UnpackOutputParam;

    enum META_TYPE {
        CONTROL_APP,
        APP_RESULT,
        HAL_RESULT
    };
    #define META_TYPE_NUM 3
    //
    enum RAW_TYPE {
        STTO_RAW,
        RRZO_RAW
    };
    #define RAW_TYPE_NUM 2
    //
    enum PRIVATE_TYPE {
        FD_DATA
    };
    #define PRIVATE_TYPE_NUM 1
    //
    typedef struct
    {
        size_t offset = 0;
        size_t length = 0;
        MSize size = 0;
        MINT format = 0;
        size_t stride_in_bytes[3] = {0,0,0};
        size_t plane_size[3] = {0,0,0};
        size_t planeCount = 0;
        MUINT32 checkSum = 0;
    } raw_info;
    //
    typedef struct
    {
        size_t offset = 0;
        size_t length = 0;
        MUINT32 checkSum = 0;
    } meta_info;
    //
    typedef struct
    {
        size_t offset = 0;
        size_t length = 0;
        MUINT32 checkSum = 0;
    } private_info;
    //
    typedef struct
    {
        MUINT32 requestNo = 0;
        MUINT32 frameNo = 0;
        int64_t timestamp = 0;
        int64_t haltimestamp = 0;
        MUINT32 totalUsedBytes = 0;
        meta_info metaInfo[META_TYPE_NUM];
        private_info privateInfo[PRIVATE_TYPE_NUM];
        raw_info rawInfo[RAW_TYPE_NUM];
    } isp_tuning_data_info;

    IspTuningDataPackUtil() {};
    ~IspTuningDataPackUtil() {};

    MUINT32 calcuCheckSum(char *pSrc, size_t length);
    MERROR packFeoDataToHalMeta(android::sp<IImageBufferHeap> pInputBufferHeap,IMetadata &appControlMeta, IMetadata &halResultMeta);
    MERROR downSampleImageBufferHeap(android::sp<IImageBufferHeap> pInputRrzoBufferHeap,
                                             android::sp<IImageStreamInfo> pInputRrzoStreamInfo,
                                             android::sp<IImageBufferHeap> &pOutputBufferHeap,
                                             IMetadata &appControlMeta,
                                             IMetadata &halResultMeta);

    ///////////////////////////////////////
    // ISP tuning data Stream Format:
    // (1)Header
    // (2)Control APP Metadata
    // (3)App Result Metadata
    // (4)Hal Result Metadata
    // (5)STTO buffer
    // (6)RRZO buffer
    ///////////////////////////////////////
    MERROR packIspTuningDataToHeap(
            android::sp<IImageBufferHeap> pOututImageBufferHeap,    //output
            MINT32& totalUsedBytes,    //output
            PackInputParam & inputParam
            );

    MERROR unpackIspTuningDataFromHeap(
            android::sp<IImageBufferHeap> const pInputImageBufferHeap,    //input
            UnpackOutputParam& outputParam
            );

    size_t getInsufficientBufferSize() {return mInsufficientBufferSize;}
protected:
    size_t mInsufficientBufferSize = 0;

};

};
};

#endif //_MTKCAM_ISPPACK_UTILIIIES_H_

