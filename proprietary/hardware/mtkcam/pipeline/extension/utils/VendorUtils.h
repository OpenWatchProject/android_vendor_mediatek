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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_VENDORUTILS_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_VENDORUTILS_H_

// MTKCAM
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
namespace NSCam {
namespace plugin {

using namespace NSCam;

/******************************************************************************
 *
 *****************************************************************************/
android::sp<NSCam::v3::IImageStreamInfo>
    createRawImageStreamInfo(
        char const*         streamName,
        v3::StreamId_T      streamId,
        MUINT32             streamType,
        size_t              maxBufNum,
        size_t              minInitBufNum,
        MUINT               usageForAllocator,
        MINT                imgFormat,
        MSize const&        imgSize,
        size_t const        stride
    );

android::sp<NSCam::v3::IImageStreamInfo>
    createImageStreamInfo(
        char const*         streamName,
        v3::StreamId_T      streamId,
        MUINT32             streamType,
        size_t              maxBufNum,
        size_t              minInitBufNum,
        MUINT               usageForAllocator,
        MINT                imgFormat,
        MSize const&        imgSize,
        MUINT32             transform
    );

/******************************************************************************
 *
 *****************************************************************************/
template <class T>
inline MBOOL
updateEntry(
        IMetadata* pMetadata,
        MUINT32 const tag,
        T const& val)
{
    if (pMetadata == NULL)
    {
        return MFALSE;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
    return MTRUE;
}

//
template <class T>
inline MBOOL
tryGetMetadata(
        IMetadata const *pMetadata,
        MUINT32 tag,
        T& rVal)
{
    if (pMetadata == NULL) {
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }

    return MFALSE;
}

template <class T>
inline MBOOL
trySetMetadata(
        IMetadata& metadata,
        MUINT32 const tag,
        T const& val)
{
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    if (0 == metadata.update(entry.tag(), entry)) {
        return MTRUE;
    }
    //
    return MFALSE;
}


/**
 * Function to retrieve the rect of cropping region that application asked,
 * which means the ROI (Region Of Intreset) of zoom.
 *  @param [in] pHalMetadata        Hal Metadata to query
 *  @param [in] pAppMetadata        App Metadata to query
 *  @param [out] outRgn             Output region.
 */
MBOOL
retrieveScalerCropRgn(
        const IMetadata* pAppMetadata,
        const IMetadata* pHalMetadata,
        MRect& outRgn
        );

MBOOL
retrieveScalerCropRgn(
        const IMetadata* pAppMetadata,
        MRect& outRgn
        );


/**
 * Function to retrieve GMV (Global Motion Vector) from the metadata.
 * Notice that, the unit of vector is based on RRZO domain.
 *  @param [in] pMetadata           Metadata to query.
 *  @param [out] x                  Horizontal motion vector
 *  @param [out] y                  Vertical motion vector
 *  @param [out] size               RRZO image size
 */
MBOOL
retrieveGmvInfo(
        const IMetadata* pMetadata,
        int& x,
        int& y,
        MSize& size
        );


/**
 *  Function to calculate ROI based on source rect and destination rect.
 *  The rule that won't make distortion between source and destination rects,
 *  but a cropping will be applied.
 *
 *  e.g.:
 *
 *  <--- src width -->
 *  +----------------+   ^
 *  |                |   |
 *  |================|   |            ^
 *  |      ROI       | src height     |  dst height
 *  |================|   |            V
 *  |                |   |
 *  +----------------+   V
 *  <--- dst width -->
 *
 *  and the return rectangle will be with a offset (x,y) and resolution of
 *  this ROI.
 *
 *  @param rSrc             Rectangle describes size of source rect
 *  @param rDst             Rectangle describes size of destination rect
 *  @return                 ROI
 */

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

inline MRect inv_transform(simpleTransform const& trans, MRect const& r) {
    return MRect(inv_transform(trans, r.p), inv_transform(trans, r.s));
};

MRect calCropRegin(const IMetadata* pAppMetadata, MSize dstSize, int openId);
MRect calCropRegin(const IMetadata* pAppMetadata, const IMetadata* pHalMetadata, MSize dstSize, int openId);
MRect calCropRegin(const IMetadata* pAppMetadata, const IMetadata* pHalMetadata, MSize srcSize, MSize dstSize, int openId);
MRect calCrop( const MRect& rSrc, const MRect& rDst );


class Cropper {
public:
    struct CropInfo : public android::RefBase {
        // port
        MBOOL isResized;
        //
        MSize sensor_size;
        // p1 crop infos
        MRect crop_p1_sensor;
        MSize dstsize_resizer;
        MRect crop_dma;
        MRect activeArray;
        //
        //simpleTransform      tranActive2Sensor;
        NSCamHW::HwMatrix matActive2Sensor;
        NSCamHW::HwMatrix matSensor2Active;
        simpleTransform tranSensor2Resized;
        //
        // target crop: cropRegion
        // not applied eis's mv yet, but the crop area is already reduced by
        // EIS ratio.
        // _a: active array coordinates
        // _s: sensor coordinates
        // active array coordinates
        MRect crop_a;
        //MPoint               crop_a_p;
        //MSize                crop_a_size;
        // sensor coordinates
        //MPoint               crop_s_p;
        //MSize                crop_s_size;
        // resized coordinates
        //
        MBOOL isEisEabled;
        vector_f eis_mv_a; //active array coor.
        vector_f eis_mv_s; //sensor coor.
        vector_f eis_mv_r; //resized coor.

    };

    static MERROR getCropInfo(
            const IMetadata* inApp,
            const IMetadata* inHal,
            MBOOL const isResized,
            CropInfo &cropInfo,
            int openId
    );
    static MVOID queryCropRegion(
            const IMetadata* inApp,
            const IMetadata* inHal,
            MBOOL const isEisOn,
            MRect &cropRegion,
            int openId
    );

    static MVOID calcBufferCrop(
            MSize const &srcSize,
            MSize const &dstSize,
            MRect &viewCrop
    );

    static MVOID calcViewAngle(
            MBOOL bEnableLog,
            CropInfo const &cropInfo,
            MSize const &dstSize,
            NSIoPipe::MCropRect &result
    );

    static MVOID calcViewAngle(
            MBOOL bEnableLog,
            CropInfo const &cropInfo,
            MSize const &srcSize,
            MSize const &dstSize,
            NSIoPipe::MCropRect &result
    );

    static MBOOL refineBoundary(
            MSize const &bufSize,
            NSIoPipe::MCropRect &crop
    );

    static MVOID dump(
            CropInfo const &cropInfo
    );
};

}; //namespace plugin
}; //namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_VENDORUTILS_H_
