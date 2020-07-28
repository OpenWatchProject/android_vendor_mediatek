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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_H_
//

#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/IMetadata.h>
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include "custom/Info.h"
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/RWLock.h>

#include <memory>
#include <map>
#include <vector>

using namespace NSCam;
/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace NSMetadataProvider {


/******************************************************************************
 *  Metadata Provider
 ******************************************************************************/
class MetadataProvider : public IMetadataProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    int32_t                         mLogLevel;
    Info                            mInfo;
    camera_metadata*                mpStaticCharacteristics;
    IMetadata                       mpHALMetadata;
    //
    // support hal 1
    IMetadata                       mpUpdatedHALMetadata;
    mutable android::RWLock         mRWLock;

private:
    enum {
        COMMON_COMMON       = 0x1,
        COMMON_SENSOR       = 0x1 << 1,
        PLATFORM_COMMON     = 0x1 << 2,
        PLATFORM_SENSOR     = 0x1 << 3,
        PROJECT_COMMON      = 0x1 << 4,
        PROJECT_SENSOR      = 0x1 << 5,
    };
    int32_t                         mVersion = 0;


public:     ////                    Instantiation.
    virtual                         ~MetadataProvider();
                                    MetadataProvider(
                                        int32_t const   i4OpenId
                                    );
                                    //
                                    MetadataProvider(
                                        int32_t const   i4OpenId,
                                        IMetadata&       aMeta_mtk,
                                        camera_metadata* aMeta
                                    );


    virtual uint32_t                getDeviceVersion() const;
    virtual int                     getDeviceFacing() const;
    virtual int                     getDeviceWantedOrientation() const;
    virtual int                     getDeviceSetupOrientation() const;
    virtual int                     getDeviceHasFlashLight() const;

public:
    virtual int                     setStaticData(IMetadata *pMeta);
    virtual int                     restoreStaticData();

protected:  ////                    Operations (Static Metadata).
    virtual status_t                constructStaticMetadata(
                                        sp<IMetadataConverter> pConverter,
                                        camera_metadata*&   rpMetadata,
                                        IMetadata&          mtkMetadata
                                    );

    virtual status_t                constructStaticMetadata_v1(
                                        sp<IMetadataConverter> pConverter,
                                        camera_metadata*&   rpMetadata,
                                        IMetadata&          mtkMetadata
                                    );

    virtual status_t                impConstructStaticMetadata(
                                        IMetadata &metadata
                                    );

    virtual status_t                impConstructStaticMetadata_v1(
                                        IMetadata &metadata
                                    );
    virtual status_t                impConstructStaticMetadata_v1_ext(
                                        IMetadata &metadata,
                                        char const* const staticMetadataNames[]
                                    );
    virtual status_t                impConstructStaticMetadata_v1_overwrite(
                                        IMetadata &metadata
                                    );

    virtual status_t                impConstructStaticMetadata_by_SymbolName(
                                        String8 const&      s8Symbol,
                                        IMetadata &metadata
                                    );

            void                    updateData(IMetadata &rMetadata);
            // validate the constructed metadata and raise assertion
            // if one of them does not satisfy the metadata specification
            void                    validation(IMetadata const& metadata);
            void                    updateStreamConfiguration(IMetadata &rMetadata);
            void                    updateRecommendedStreamConfiguration(IMetadata &rMetadata);
            void                    updateAfRegions(IMetadata &rMetadata);

            MBOOL                   isHdrSensor(MUINT const availVhdrEntryCount);

            MVOID                   updateHdrData(MBOOL const isHDRSensor, MBOOL const isSingleFrameSupport, MINT32 const hdrDetectionMode,
                                        IMetadata &rMetadata);

            MVOID                   updateZslData(IMetadata &rMetadata);

            MVOID                   updateHardwareLevel(IMetadata &rMetadata    __unused);
            MVOID                   updateRawCapability(IMetadata &rMetadata    __unused);
            MVOID                   updateReprocessCapabilities(IMetadata &rMetadata    __unused);
            MVOID                   updatePostRawSensitivityBoost(IMetadata &rMetadata  __unused);

public:     ////                    Operations.
    virtual status_t                onCreate();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Static Info Interface.

    virtual camera_metadata const*  getStaticCharacteristics()  const   { return mpStaticCharacteristics; }
    virtual IMetadata const&        getMtkStaticCharacteristics() const { return mpHALMetadata; }

private:

    struct StreamConfig_T {
        MINT64          mFormat = -1;
        MINT64          mWidth = -1;
        MINT64          mHeight = -1;
        MINT64          mDirection = -1;
        MINT64          mFrameDuration = -1;
        MINT64          mStallDuration = -1;
        // indicates force available in final result even if this size is larger than active array.
        bool            mMandatory = false;

        StreamConfig_T() {}

        StreamConfig_T(
            MINT64 const format,
            MINT64 const width,
            MINT64 const height,
            MINT64 const direction,
            MINT64 const frameDuration,
            MINT64 const stallDuration,
            bool   const mandatory
        )
            : mFormat(format)
            , mWidth(width)
            , mHeight(height)
            , mDirection(direction)
            , mFrameDuration(frameDuration)
            , mStallDuration(stallDuration)
            , mMandatory(mandatory)
        {}
    };
    struct RecommendedConfig_T {
        MINT32          mFormat = -1;
        MINT32          mWidth = -1;
        MINT32          mHeight = -1;
        MINT32          mDirection = -1;
        MINT32          mUsercaseID = -1;
        // indicates force available in final result even if this size is larger than active array.
        bool            mMandatory = false;

        RecommendedConfig_T() {}

        RecommendedConfig_T(
            MINT32 const format,
            MINT32 const width,
            MINT32 const height,
            MINT32 const direction,
            MINT32 const usercaseID,
            bool   const mandatory
        )
            : mFormat(format)
            , mWidth(width)
            , mHeight(height)
            , mDirection(direction)
            , mUsercaseID(usercaseID)
            , mMandatory(mandatory)
        {}
    };

    struct MSizeComp {
        bool operator() (const MSize& size1, const MSize& size2) const {
            return (size1.w>size2.w)? true :
                   (size1.w==size2.w && size1.h>size2.h)? true : false;
        }
    };

    // For each format, this map uses size as key and StreamConfig_T data as value, within
    // descending order of key comparison object.
    typedef std::map<MSize, std::shared_ptr<StreamConfig_T>, MSizeComp>  OrderedMap_T;
    typedef std::map<MSize, std::shared_ptr<RecommendedConfig_T>, MSizeComp>  OrderedMapRecommended_T;
    // key: format
    typedef std::map<MINT64, std::shared_ptr<OrderedMap_T> >             StreamConfigMap_T;
    typedef std::map<MINT64, std::shared_ptr<OrderedMapRecommended_T> >  RecommendedStreamConfigMap_T;
    StreamConfigMap_T               mStreamCfgs;
    StreamConfigMap_T               mInputStreamCfgs;
    RecommendedStreamConfigMap_T    mRecommendedCfgs;

};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSMetadataProvider
};  // namespace android
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_H_

