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

#ifndef _MTKCAM_HWNODE_UTILIIIES_H_
#define _MTKCAM_HWNODE_UTILIIIES_H_

#include <cutils/properties.h>
#include <utils/String8.h>
#include <utils/Vector.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Sync.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
//
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/IHalISP.h>

#include <isp_tuning/isp_tuning.h>     // EIspProfile_*

#include <camera_custom_nvram.h>
#include <mtkcam/aaa/INvBufUtil.h>
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
#include <mtkcam/drv/IHalSensor.h>     // MAKE_HalSensorList

using namespace NSIspTuning;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

// utilities for crop

inline MINT32 div_round(MINT32 const numerator, MINT32 const denominator) {
    return (( numerator < 0 ) ^ (denominator < 0 )) ?
        (( numerator - denominator/2)/denominator) : (( numerator + denominator/2)/denominator);
}

struct vector_f //vector with floating point
{
    MPoint  p;
    MPoint  pf;

                                vector_f(
                                        MPoint const& rP = MPoint(),
                                        MPoint const& rPf = MPoint()
                                        )
                                    : p(rP)
                                    , pf(rPf)
                                {}
};

struct simpleTransform
{
    // just support translation than scale, not a general formulation
    // translation
    MPoint    tarOrigin;
    // scale
    MSize     oldScale;
    MSize     newScale;

                                simpleTransform(
                                        MPoint rOrigin = MPoint(),
                                        MSize  rOldScale = MSize(),
                                        MSize  rNewScale = MSize()
                                        )
                                    : tarOrigin(rOrigin)
                                    , oldScale(rOldScale)
                                    , newScale(rNewScale)
                                {}
};

// transform MPoint
inline MPoint transform(simpleTransform const& trans, MPoint const& p) {
    return MPoint(
            div_round( (p.x - trans.tarOrigin.x) * trans.newScale.w, trans.oldScale.w),
            div_round( (p.y - trans.tarOrigin.y) * trans.newScale.h, trans.oldScale.h)
            );
};

inline MPointF transform(simpleTransform const& trans, MPointF const& p) {
    return MPointF(
            ( (p.x - trans.tarOrigin.x) * trans.newScale.w / trans.oldScale.w),
            ( (p.y - trans.tarOrigin.y) * trans.newScale.h / trans.oldScale.h)
            );
};

inline MPoint inv_transform(simpleTransform const& trans, MPoint const& p) {
    return MPoint(
            div_round( p.x * trans.oldScale.w, trans.newScale.w) + trans.tarOrigin.x,
            div_round( p.y * trans.oldScale.h, trans.newScale.h) + trans.tarOrigin.y
            );
};

inline int int_floor(float x) {
    int i = (int)x;
    return i - (i > x);
}

// transform vector_f
inline vector_f transform(simpleTransform const& trans, vector_f const& p) {
    MFLOAT const x = (p.p.x + (p.pf.x/(MFLOAT)(1u<<31))) * trans.newScale.w / trans.oldScale.w;
    MFLOAT const y = (p.p.y + (p.pf.y/(MFLOAT)(1u<<31))) * trans.newScale.h / trans.oldScale.h;
    int const x_int = int_floor(x);
    int const y_int = int_floor(y);
    return vector_f(
            MPoint(x_int, y_int),
            MPoint((x - x_int) * (1u<<31), (y - y_int) * (1u<<31))
            );
};

inline vector_f inv_transform(simpleTransform const& trans, vector_f const& p) {
    MFLOAT const x = (p.p.x + (p.pf.x/(MFLOAT)(1u<<31))) * trans.oldScale.w / trans.newScale.w;
    MFLOAT const y = (p.p.y + (p.pf.y/(MFLOAT)(1u<<31))) * trans.oldScale.h / trans.newScale.h;
    int const x_int = int_floor(x);
    int const y_int = int_floor(y);
    return vector_f(
            MPoint(x_int, y_int),
            MPoint((x - x_int) * (1u<<31), (y - y_int) * (1u<<31))
            );
};

// transform MSize
inline MSize transform(simpleTransform const& trans, MSize const& s) {
    return MSize(
            div_round( s.w * trans.newScale.w, trans.oldScale.w),
            div_round( s.h * trans.newScale.h, trans.oldScale.h)
            );
};

inline MSizeF transform(simpleTransform const& trans, MSizeF const& s) {
    return MSizeF(
            ( s.w * trans.newScale.w / trans.oldScale.w),
            ( s.h * trans.newScale.h / trans.oldScale.h)
            );
};

inline MSize inv_transform(simpleTransform const& trans, MSize const& s) {
    return MSize(
            div_round( s.w * trans.oldScale.w, trans.newScale.w),
            div_round( s.h * trans.oldScale.h, trans.newScale.h)
            );
};

// transform MRect
inline MRect transform(simpleTransform const& trans, MRect const& r) {
    return MRect(transform(trans, r.p), transform(trans, r.s));
};

inline MRectF transform(simpleTransform const& trans, MRectF const& r) {
    return MRectF(transform(trans, r.p), transform(trans, r.s));
};

inline MRect inv_transform(simpleTransform const& trans, MRect const& r) {
    return MRect(inv_transform(trans, r.p), inv_transform(trans, r.s));
};


/******************************************************************************
 *  Metadata Access
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if (pMetadata == NULL) {
        CAM_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    //
    return MFALSE;
}

template <typename T>
inline MBOOL
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if (pMetadata == NULL) {
        CAM_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    if (0 == pMetadata->update(tag, entry)) {
        return MTRUE;
    }
    //
    return MFALSE;
}

/******************************************************************************
 *  Simulate HAL 3A.
 ******************************************************************************/

using namespace NS3Av3;
using namespace android;


/******************************************************************************
 *  Hardware Related
 ******************************************************************************/
inline MBOOL isPostProcRawSupported() {
    MBOOL bSupport = MTRUE;
    #if (MTKCAM_HW_NODE_WITH_LEGACY_SUPPORT > 0)
    bSupport = MFALSE;
    #endif
    return bSupport;
};


/******************************************************************************
 *  Opaque Reprocessing Utility
 ******************************************************************************/
#define MAX_METADATA_SIZE        (200000)

class OpaqueReprocUtil
{
private:
    typedef struct
    {
        MINT8 aligned_byte;
        MSize raw_size;
        MINT raw_format;
        size_t stride_in_bytes;
        size_t payload_offset;
        size_t payload_length;
        size_t app_meta_offset;
        size_t app_meta_length;
        size_t hal_meta_offset;
        size_t hal_meta_length;

        MVOID dump()
        {
            CAM_LOGD( "[opaque] aligned_byte(0x%x) raw_size(%d,%d) raw_format(0x%x) stride_in_bytes(%zu) "
                    "payload(%zu-%zu) app_meta(%zu-%zu) hal_meta(%zu-%zu)",
                  aligned_byte, raw_size.w, raw_size.h, raw_format, stride_in_bytes,
                  payload_offset, payload_length,
                  app_meta_offset, app_meta_length,
                  hal_meta_offset, hal_meta_length);
        }
    } opaque_reproc_info_t;

public:

    static MERROR setOpaqueInfoToHeap(
            android::sp<IImageBufferHeap> &pImageBufferHeap,
            MSize const rawSize,
            MINT const rawFormat,
            size_t const rawStrideInBytes,
            size_t const rawSizeInBytes)
    {
        size_t infoOffset = (ssize_t)pImageBufferHeap->getBufSizeInBytes(0) - sizeof(opaque_reproc_info_t);
        if (infoOffset < rawSizeInBytes)
            return -EINVAL;

        MINTPTR ptrOpaqueBuf = (MINTPTR) pImageBufferHeap->getBufVA(0);
        opaque_reproc_info_t *pInfo = (opaque_reproc_info_t *) (ptrOpaqueBuf + infoOffset);

        pInfo->aligned_byte = 0x00;
        pInfo->raw_size = rawSize;
        pInfo->raw_format = rawFormat;
        pInfo->stride_in_bytes = rawStrideInBytes;
        pInfo->payload_offset = rawSizeInBytes;
        pInfo->payload_length = infoOffset - rawSizeInBytes;
        pInfo->app_meta_offset = 0;
        pInfo->app_meta_length = 0;
        pInfo->hal_meta_offset = 0;
        pInfo->hal_meta_length = 0;

        return 0;
    }

    static MERROR getImageBufferFromHeap(
            android::sp<IImageBufferHeap> pImageBufferHeap,
            android::sp<IImageBuffer>& rpImageBuffer)
    {
        size_t infoOffset = pImageBufferHeap->getBufSizeInBytes(0) - sizeof(opaque_reproc_info_t);
        MINTPTR ptrOpaqueBuf = (MINTPTR) pImageBufferHeap->getBufVA(0);
        opaque_reproc_info_t *pInfo = (opaque_reproc_info_t *) (ptrOpaqueBuf + infoOffset);

        if (pInfo->aligned_byte != 0x00)
            return NO_INIT;

        size_t strideInBytes[3]{pInfo->stride_in_bytes, 0, 0};

        rpImageBuffer = pImageBufferHeap->createImageBuffer_FromBlobHeap(
                0, pInfo->raw_format, pInfo->raw_size, strideInBytes);

        // pInfo->dump();
        return 0;
    }

    static MERROR setAppMetadataToHeap(
            android::sp<IImageBufferHeap>& pImageBufferHeap,
            IMetadata& appMeta)
    {
        size_t infoOffset = pImageBufferHeap->getBufSizeInBytes(0) - sizeof(opaque_reproc_info_t);
        MINTPTR ptrOpaqueBuf = (MINTPTR) pImageBufferHeap->getBufVA(0);
        opaque_reproc_info_t *pInfo = (opaque_reproc_info_t *) (ptrOpaqueBuf + infoOffset);

        if (pInfo->aligned_byte != 0x00)
            return NO_INIT;
        else if (pInfo->app_meta_length != 0)
            return ALREADY_EXISTS;

        // calculate the offset of app meta where hal meta is probably empty
        pInfo->app_meta_offset = pInfo->payload_offset + pInfo->hal_meta_length;

        void *pAppMetaBuf = (void *)(ptrOpaqueBuf + pInfo->app_meta_offset);
        MINT32 max_size = pInfo->payload_length - pInfo->hal_meta_length;
        ssize_t ret = appMeta.flatten(pAppMetaBuf, max_size);

        if (ret < 0) {
            CAM_LOGE("[opaque] oversized payload: ret=%zd, hal_meta_length=%zu",
                    ret, pInfo->hal_meta_length);
            return ret;
        }
        pInfo->app_meta_length = ret;

        // pInfo->dump();
        return 0;
    }

    static MERROR setHalMetadataToHeap(
            android::sp<IImageBufferHeap>& pImageBufferHeap,
            IMetadata& halMeta)
    {
        size_t infoOffset = pImageBufferHeap->getBufSizeInBytes(0) - sizeof(opaque_reproc_info_t);
        MINTPTR ptrOpaqueBuf = (MINTPTR) pImageBufferHeap->getBufVA(0);
        opaque_reproc_info_t *pInfo = (opaque_reproc_info_t *) (ptrOpaqueBuf + infoOffset);

        if (pInfo->aligned_byte != 0x00)
            return NO_INIT;
        else if (pInfo->hal_meta_length != 0)
            return ALREADY_EXISTS;

        // calculate the offset of app meta where app meta is probably empty
        pInfo->hal_meta_offset = pInfo->payload_offset + pInfo->app_meta_length;

        void *pHalMetaBuf = (void *)(ptrOpaqueBuf + pInfo->hal_meta_offset);
        MINT32 max_size = pInfo->payload_length - pInfo->app_meta_length;
        ssize_t ret = halMeta.flatten(pHalMetaBuf, max_size);

        if (ret < 0) {
            CAM_LOGE("[opaque] oversized payload: ret =%zd, app_meta_length=%zu",
                    ret, pInfo->app_meta_length);
            return ret;
        }
        pInfo->hal_meta_length = ret;

        // pInfo->dump();
        return 0;
    }

    static MERROR getAppMetadataFromHeap(
            android::sp<IImageBufferHeap> const& pImageBufferHeap,
            IMetadata& appMeta)
    {
        size_t infoOffset = pImageBufferHeap->getBufSizeInBytes(0) - sizeof(opaque_reproc_info_t);
        MINTPTR ptrOpaqueBuf = (MINTPTR) pImageBufferHeap->getBufVA(0);
        opaque_reproc_info_t *pInfo = (opaque_reproc_info_t *) (ptrOpaqueBuf + infoOffset);

        if (pInfo->aligned_byte != 0x00)
            return NO_INIT;

        void *pAppMetaBuf = (void *)(ptrOpaqueBuf + pInfo->app_meta_offset);
        ssize_t ret = appMeta.unflatten(pAppMetaBuf, pInfo->app_meta_length);
        CAM_LOGD_IF(0, "[opaque] app meta unflatten from size: %zd", ret);
        if (ret < 0)
            return ret;

        // pInfo->dump();
        return 0;
    }


    static MERROR getHalMetadataFromHeap(
            android::sp<IImageBufferHeap> const& pImageBufferHeap,
            IMetadata& halMeta)
    {
        size_t infoOffset = pImageBufferHeap->getBufSizeInBytes(0) - sizeof(opaque_reproc_info_t);
        MINTPTR ptrOpaqueBuf = (MINTPTR) pImageBufferHeap->getBufVA(0);
        opaque_reproc_info_t *pInfo = (opaque_reproc_info_t *)(ptrOpaqueBuf + infoOffset);

        if (pInfo->aligned_byte != 0x00)
            return NO_INIT;

        void *pHalMetaBuf = (void *)(ptrOpaqueBuf + pInfo->hal_meta_offset);
        ssize_t ret = halMeta.unflatten(pHalMetaBuf, pInfo->hal_meta_length);
        CAM_LOGD_IF(0, "[opaque] hal meta unflatten from size: %zd", ret);
        if (ret < 0)
            return ret;

        // pInfo->dump();
        return 0;
    }
};

enum NVRAM_TYPE
{
    NVRAM_TYPE_DRE,
    NVRAM_TYPE_CZ,
    NVRAM_TYPE_SWNR_THRES
};

static const void *getTuningFromNvram(MUINT32 openId, MUINT32& idx, MINT32 magicNo, MINT32 type, MBOOL enableLog)
{
#if MTK_CAM_NEW_NVRAM_SUPPORT
    CAM_TRACE_BEGIN("getNvram");
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    const void *pNRNvram = nullptr;

    if (idx >= EISO_NUM) {
        CAM_LOGE("wrong nvram idx %d", idx);
        return NULL;
    }

    // load some setting from nvram
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);

    auto pNvBufUtil = MAKE_NvBufUtil();
    if (pNvBufUtil == NULL) {
        CAM_LOGE("pNvBufUtil==0");
        return NULL;
    }

    auto result = pNvBufUtil->getBufAndRead(
        CAMERA_NVRAM_DATA_FEATURE,
        sensorDev, (void*&)pNvram);
    if (result != 0) {
        CAM_LOGE("read buffer chunk fail");
        return NULL;
    }

    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    CAM_IDX_QRY_COMB rMapping_Info;
    pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);

    switch (type)
    {
        case NVRAM_TYPE_DRE:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_CA_LTM, rMapping_Info, __FUNCTION__);
            pNRNvram = &(pNvram->CA_LTM[idx]);
            break;
        }
        case NVRAM_TYPE_CZ:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_ClearZoom, rMapping_Info, __FUNCTION__);
            pNRNvram = &(pNvram->ClearZoom[idx]);
            break;
        }
        case NVRAM_TYPE_SWNR_THRES:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_SWNR_THRES, rMapping_Info, __FUNCTION__);
            pNRNvram = &(pNvram->SWNR_THRES[idx]);
            break;
        }
        default:
        {
            CAM_LOGW("not support nvram type: %d", type);
            break;
        }
    }

    CAM_LOGD_IF(enableLog, "[%s] query nvram type(%d) magic(%d) mappingInfo index: %d",__FUNCTION__, type, magicNo, idx);
    CAM_TRACE_END();
    return pNRNvram;
#else
    return NULL;
#endif
}



};
};

#endif //_MTKCAM_HWNODE_UTILIIIES_H_

