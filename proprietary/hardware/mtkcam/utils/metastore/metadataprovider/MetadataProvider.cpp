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

#define LOG_TAG "MtkCam/MetadataProvider"
//
#include "MyUtils.h"
#include <hardware/camera3.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/metadata/IMetadataTagSet.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/metadata/client/TagMap.h>
#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/mtk_metadata_types.h>

#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
//
/******************************************************************************
 *
 ******************************************************************************/
static bool setTagInfo(IMetadataTagSet &rtagInfo);


/******************************************************************************
 *
 ******************************************************************************/
IMetadataProvider*
IMetadataProvider::
create(
    int32_t const   i4OpenId
)
{
    MetadataProvider* p = new MetadataProvider(i4OpenId);
    if  ( CC_UNLIKELY( !p ) )
    {
        META_LOGE("No Memory");
        return  NULL;
    }
    //
    if  ( CC_UNLIKELY( p->onCreate()!=OK ) )
    {
        META_LOGE("onCreate");
        delete  p;
        return  NULL;
    }
    //
    return  p;
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadataProvider*
IMetadataProvider::
create(
    int32_t const    i4OpenId,
    IMetadata&       aMeta_mtk,
    camera_metadata* aMeta
)
{
    MetadataProvider* p = new MetadataProvider(i4OpenId, aMeta_mtk, aMeta);
    if  ( CC_UNLIKELY( !p ) )
    {
        META_LOGE("No Memory");
        return  NULL;
    }
    //
    return  p;
}


/******************************************************************************
 *
 ******************************************************************************/
MetadataProvider::
~MetadataProvider()
{
    META_LOGD("+ OpenId:%d", mInfo.getDeviceId());
    //
    if  ( mpStaticCharacteristics )
    {
        ::free_camera_metadata(mpStaticCharacteristics);
        mpStaticCharacteristics = NULL;
    }
    //
    META_LOGD("- OpenId:%d", mInfo.getDeviceId());
}


/******************************************************************************
 *
 ******************************************************************************/
MetadataProvider::
MetadataProvider(
    int32_t const   i4OpenId
)
    : mLogLevel(0)
    , mpStaticCharacteristics(NULL)
    , mpHALMetadata()
    , mRWLock()
    , mVersion(0)
{
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("vendor.debug.camera.log.metadataprovider", 0);
    }

    mVersion = ::property_get_int32("vendor.debug.camera.static_meta.version", 1);
    //
    IHalLogicalDeviceList* pHalDeviceList;
    pHalDeviceList = MAKE_HalLogicalDeviceList();
    if ( CC_UNLIKELY( pHalDeviceList==nullptr ) ) {
        META_LOGE("get HalLogicalDeviceList fail!");
        return;
    }
    //
    int32_t sensorType = pHalDeviceList->queryType(i4OpenId);
    int32_t sensorDev = pHalDeviceList->querySensorDevIdx(i4OpenId);
    NSCam::SensorStaticInfo sensorStaticInfo;
    pHalDeviceList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
    int32_t sensorRawFmtType = sensorStaticInfo.rawFmtType;
    const char* sensorDrvName = pHalDeviceList->queryDriverName(i4OpenId);
    META_LOGD("sensorDrvName : %s, %p", sensorDrvName, sensorDrvName);
    //
    mInfo = Info( i4OpenId, sensorDev, sensorType, sensorRawFmtType,
                  (pHalDeviceList->queryFacingDirection(i4OpenId)==0)? true : false ,
                  sensorDrvName);
    //
    showCustInfo();
}


/******************************************************************************
 *
 ******************************************************************************/
MetadataProvider::
MetadataProvider(
    int32_t const    i4OpenId,
    IMetadata&       aMeta_mtk,
    camera_metadata* aMeta
)
    : mpStaticCharacteristics(aMeta)
    , mpHALMetadata(aMeta_mtk)
{
    IHalLogicalDeviceList* pHalDeviceList;
    pHalDeviceList = MAKE_HalLogicalDeviceList();
    if ( CC_UNLIKELY( pHalDeviceList==nullptr ) ) {
        META_LOGE("get HalLogicalDeviceList fail!");
        return;
    }

    int32_t sensorType = pHalDeviceList->queryType(i4OpenId);
    int32_t sensorDev = pHalDeviceList->querySensorDevIdx(i4OpenId);
    NSCam::SensorStaticInfo sensorStaticInfo;
    pHalDeviceList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
    int32_t sensorRawFmtType = sensorStaticInfo.rawFmtType;
    const char* sensorDrvName = pHalDeviceList->queryDriverName(i4OpenId);
    //
    mInfo = Info( i4OpenId, sensorDev, sensorType, sensorRawFmtType ,
                  (pHalDeviceList->queryFacingDirection(i4OpenId)==0)? true : false ,
                  sensorDrvName);
    //
    showCustInfo();
}


/******************************************************************************
 *
 ******************************************************************************/
uint32_t
MetadataProvider::
getDeviceVersion() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mpHALMetadata.entryFor(MTK_HAL_VERSION).itemAt(0, Type2Type< MINT32 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceFacing() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mpHALMetadata.entryFor(MTK_SENSOR_INFO_FACING).itemAt(0, Type2Type< MUINT8 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceWantedOrientation() const
{
    RWLock::AutoRLock _l(mRWLock);
    if (mpHALMetadata.entryFor(MTK_SENSOR_INFO_WANTED_ORIENTATION).isEmpty())
        return mInfo.getDeviceId() == 0 ? 90 : 270;

    return mpHALMetadata.entryFor(MTK_SENSOR_INFO_WANTED_ORIENTATION).itemAt(0, Type2Type< MINT32 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceSetupOrientation() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mpHALMetadata.entryFor(MTK_SENSOR_INFO_ORIENTATION).itemAt(0, Type2Type< MINT32 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceHasFlashLight() const
{
    RWLock::AutoRLock _l(mRWLock);
    if (mpHALMetadata.entryFor(MTK_FLASH_INFO_AVAILABLE).isEmpty())
        return 0;

    return mpHALMetadata.entryFor(MTK_FLASH_INFO_AVAILABLE).itemAt(0, Type2Type< MUINT8 >());
}

/******************************************************************************
 *
 ******************************************************************************/
int MetadataProvider::
setStaticData(IMetadata *pMeta)
{
    RWLock::AutoWLock _l(mRWLock);
    mpUpdatedHALMetadata = mpHALMetadata;
    for (MUINT i = 0 ; i < pMeta->count() ; i++)
    {
        mpHALMetadata.remove(pMeta->entryAt(i).tag());
        mpHALMetadata.update(pMeta->entryAt(i).tag(), pMeta->entryAt(i));
    }
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
 int MetadataProvider::
restoreStaticData()
{
    RWLock::AutoWLock _l(mRWLock);
    if(!mpUpdatedHALMetadata.isEmpty())
    {
        mpHALMetadata.clear();
        mpHALMetadata = mpUpdatedHALMetadata;
        mpUpdatedHALMetadata.clear();
    }
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
onCreate()
{
    status_t status = OK;

    //prepare data for TagInfo
    IMetadataTagSet mtagInfo;
    setTagInfo(mtagInfo);

    //create IMetadataConverter
    sp<IMetadataConverter> pMetadataConverter = IMetadataConverter::createInstance(mtagInfo);

    {
        RWLock::AutoWLock _l(mRWLock);
        status = (mVersion==0)?
                 constructStaticMetadata(pMetadataConverter, mpStaticCharacteristics, mpHALMetadata) :
                 constructStaticMetadata_v1(pMetadataConverter, mpStaticCharacteristics, mpHALMetadata);
        if  ( CC_UNLIKELY( status!=OK ) ) {
            META_LOGE("constructStaticMetadata - status[%s(%d)]", ::strerror(-status), -status);
            return  status;
        }
    }

    return  status;
}

static
bool setTagInfo(IMetadataTagSet &rtagInfo)
{
    #define _IMP_SECTION_INFO_(...)
    #undef  _IMP_TAG_INFO_
    #define _IMP_TAG_INFO_(_tag_, _type_, _name_) \
        rtagInfo.addTag(_tag_, _name_, Type2TypeEnum<_type_>::typeEnum);
    #include <mtkcam/utils/metadata/client/mtk_metadata_tag_info.inl>
    #include <custom_metadata/custom_metadata_tag_info.inl>
    #undef  _IMP_TAG_INFO_


    #undef _IMP_TAGCONVERT_
    #define _IMP_TAGCONVERT_(_android_tag_, _mtk_tag_) \
        rtagInfo.addTagMap(_android_tag_, _mtk_tag_);
    #if (PLATFORM_SDK_VERSION >= 21)
    ADD_ALL_MEMBERS;
    #endif

    #undef _IMP_TAGCONVERT_

    return MTRUE;
}


