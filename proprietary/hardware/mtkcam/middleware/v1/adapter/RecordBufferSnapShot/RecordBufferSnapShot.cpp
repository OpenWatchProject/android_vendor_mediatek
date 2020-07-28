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

#define LOG_TAG "MtkCam/RBSS"
//
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
//
#include <cutils/properties.h>
//
#include <list>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/exif/IBaseCamExif.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/def/common.h>
#include <mtkcam/def/ImageFormat.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>
#include <mtkcam/middleware/v1/IShot.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
using namespace NSCam::NSIoPipe::NSSImager;
#include <ExifJpegUtils.h>

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

//
#include <RecordBufferSnapShot.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::Utils;
using namespace NSShot;
using namespace std;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
#define FUNC_START_I                MY_LOGI("+")
#define FUNC_END_I                  MY_LOGI("-")

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* const pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        CAM_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        CAM_LOGE("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


/******************************************************************************
 *
 ******************************************************************************/
template<typename T>
inline MVOID
updateEntryArray(
    IMetadata* pMetadata,
    MUINT32 const tag,
    const T* array,
    MUINT32 size
)
{
    IMetadata::IEntry entry(tag);
    for (MUINT32 i = size; i != 0; i--)
    {
        entry.push_back(*array++, Type2Type< T >());
    }
    pMetadata->update(tag, entry);
}


/*******************************************************************************
*
********************************************************************************/
class RecordBufferSnapShotImp : public RecordBufferSnapShot
{
    public:
        static RecordBufferSnapShot* createInstance(
                                        MINT32                  openId);

        virtual MVOID               destroyInstance();

        RecordBufferSnapShotImp(MINT32 openId);

        virtual ~RecordBufferSnapShotImp();
    //
    public:
        virtual MBOOL   startRecording(
                            MUINT32                 recWidth,
                            MUINT32                 recHeight,
                            MUINT32                 recFormat,
                            wp<IParamsManagerV3>    wpParamsMgrV3,
                            wp<IShotCallback>       wpShotCallback);
        virtual MBOOL   stopRecording();
        virtual MBOOL   takePicture();
        virtual MBOOL   postBuffer(sp<IImageBufferHeap> spYuvBuf);
        MINT32 getOpenId() const { return mOpenId; }

        volatile MINT32         mUser;
    private:
        #define REC_IMG_STRIDE_Y    (16)
        #define REC_IMG_STRIDE_U    (16)
        #define REC_IMG_STRIDE_V    (16)

        #define ALIGN_UP_SIZE(in,align)     ((in+align-1) & ~(align-1))
        #define ALIGN_DOWN_SIZE(in,align)   (in & ~(align-1))

        #define ROUND_TO_2X(x) ((x) & (~0x1))

        typedef struct
        {
            MUINT32 planeCnt;
            MUINTPTR addr[3];
            MUINT32 size[3];
            MUINT32 stride[3];
        }REC_BUF_INFO;

        enum
        {
            BUF_YUV_TEMP,
            BUF_YUV_MAIN,
            BUF_YUV_THUMB,
            BUF_JPG,
            BUF_AMOUNT
        };

        MBOOL processJpg();
        MBOOL callbackJpg();
        MBOOL allocBuffer(
                IImageBuffer*&      pImageBuffer,
                MUINT32             width,
                MUINT32             height,
                MUINT32             format);
        MBOOL freeBuffer(IImageBuffer*& pImageBuffer);
        MVOID lockBuffer(MUINT32 type);
        MVOID unlockBuffer(MUINT32 type);
        MRect calCrop(MRect src, MRect dst);

        MBOOL getBestPicSizeForVSS();
        //
        mutable Mutex           mLock;
        mutable Condition       mCond;
        MBOOL                   mbNeedYuv;
        MBOOL                   mbRecord;
        MBOOL                   mbHasThumb;
        MINT32                  mOpenId;
        MINT32                  mLogLevel;
        MUINT32                 mRecWidth;
        MUINT32                 mRecHeight;
        MUINT32                 mRecFormat;
        MUINT32                 mRecPicSizeWidth;
        MUINT32                 mRecPicSizeHeight;
        MUINT32                 mJpgSize;
        MUINT32                 mRotation;
        MUINT32                 mJpegThumbWidth;
        MUINT32                 mJpegThumbHeight;
        MUINT32                 mJpegQuality;
        MUINT32                 mJpegQualityThumb;
        MBOOL                   mGgpsEnabled;
        MDOUBLE                 mGpsCoordinates[3];
        MINT64                  mGpsTimestamp;
        String8                 mGpsProcessingMethod;
        MINT32                  mFocalLength;
        MINT32                  mFocalLengthBase;
        REC_BUF_INFO            mRecBufInfo;
        IImageBuffer*           mpImageBuffer[BUF_AMOUNT];
        wp<IParamsManagerV3>    mwpParamsMgrV3;
        wp<IShotCallback>       mwpShotCallback;
};


/*******************************************************************************
*
********************************************************************************/
static Mutex RecordBufferSnapShotImpLock;
static list<RecordBufferSnapShotImp*> vpRecordBufferSnapShotImpSingleton;


/*******************************************************************************
*
********************************************************************************/
RecordBufferSnapShot*
RecordBufferSnapShot::
createInstance(MINT32 openId)
{
    CAM_LOGI("+");
    //
    Mutex::Autolock _l(RecordBufferSnapShotImpLock);
    //
    RecordBufferSnapShotImp* pRecordBufferSnapShotImp = NULL;
    list<RecordBufferSnapShotImp*>::const_iterator iter;
    for(iter = vpRecordBufferSnapShotImpSingleton.begin(); iter != vpRecordBufferSnapShotImpSingleton.end(); iter++)
    {
        if((*iter)->getOpenId() == openId)
            pRecordBufferSnapShotImp = (*iter);
    }
    //
    if(pRecordBufferSnapShotImp == NULL)
    {
        //create new
        pRecordBufferSnapShotImp = new RecordBufferSnapShotImp(openId);
        vpRecordBufferSnapShotImpSingleton.push_back(pRecordBufferSnapShotImp);
    }
    //
    pRecordBufferSnapShotImp->mUser++;
    CAM_LOGI("this(0x%x), userCnt(%d)",
            (MUINTPTR)(pRecordBufferSnapShotImp),
            pRecordBufferSnapShotImp->mUser);
    //
    CAM_LOGI("-");
    return pRecordBufferSnapShotImp;
}


/*******************************************************************************
*
********************************************************************************/
void
RecordBufferSnapShotImp::
destroyInstance()
{
    //FUNC_START;
    //
    Mutex::Autolock _l(RecordBufferSnapShotImpLock);
    //
    mUser--;
    MY_LOGI("this(0x%x), userCnt(%d)",
            (MUINTPTR)this,
            mUser);
    //
    if(mUser == 0)
    {
        list<RecordBufferSnapShotImp*>::iterator iter;
        for(iter = vpRecordBufferSnapShotImpSingleton.begin(); iter != vpRecordBufferSnapShotImpSingleton.end(); iter++)
        {
            if((*iter) == this)
            {
                vpRecordBufferSnapShotImpSingleton.erase(iter);
                break;
            }
        }
        FUNC_END; // marco FUNC_END invokes member hence we have to call it before delete self
        delete this;
        return;
    }
    //
    //FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
RecordBufferSnapShotImp::
RecordBufferSnapShotImp(MINT32 openId)
    : mUser(0)
    , mbNeedYuv(MFALSE)
    , mbRecord(MFALSE)
    , mbHasThumb(MTRUE)
    , mOpenId(openId)
    , mLogLevel(0)
    , mRecWidth(0)
    , mRecHeight(0)
    , mRecFormat(0)
    , mRecPicSizeWidth(0)
    , mRecPicSizeHeight(0)
    , mJpgSize(0)
    , mRotation(0)
    , mJpegThumbWidth(0)
    , mJpegThumbHeight(0)
    , mJpegQuality(0)
    , mJpegQualityThumb(0)
    , mGgpsEnabled(MTRUE)
    , mGpsTimestamp(0)
    , mGpsProcessingMethod("GPS")
    , mFocalLength(0)
    , mFocalLengthBase(1000)
    , mwpParamsMgrV3(NULL)
    , mwpShotCallback(NULL)
    , mpImageBuffer{nullptr}
{
    FUNC_START;
    //
    mLogLevel = ::property_get_int32("vendor.debug.camera.log", 0);
    if ( mLogLevel == 0 )
    {
        mLogLevel = ::property_get_int32("debug.camera.log.rbss", 1);
    }
    //
    memset((void*)(&mRecBufInfo), 0, sizeof(REC_BUF_INFO));
    //
    for(MINT i=0; i<BUF_AMOUNT; i++)
    {
        mpImageBuffer[i] = NULL;
    }
    //
    mGpsCoordinates[0]  = 0;
    mGpsCoordinates[1]  = 0;
    mGpsCoordinates[2]  = 0;
    //
    MY_LOGI("- User(%d), OpenId(%d)",
            mUser,
            mOpenId);
}


/*******************************************************************************
*
********************************************************************************/
RecordBufferSnapShotImp::
~RecordBufferSnapShotImp()
{
    FUNC_START;
    //
    //
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
startRecording(
    MUINT32                 recWidth,
    MUINT32                 recHeight,
    MUINT32                 recFormat,
    wp<IParamsManagerV3>    wpParamsMgrV3,
    wp<IShotCallback>       wpShotCallback)
{
    FUNC_START_I;
    //
    if(mbRecord)
    {
        MY_LOGW("- record has been started!");
        return MTRUE;
    }
    //
    mRecWidth       = recWidth;
    mRecHeight      = recHeight;
    mRecFormat      = recFormat;
    mwpParamsMgrV3  = wpParamsMgrV3;
    mwpShotCallback = wpShotCallback;
    getBestPicSizeForVSS();
    //
    allocBuffer(
        mpImageBuffer[BUF_YUV_TEMP],
        mRecWidth,
        mRecHeight,
        mRecFormat);
    mpImageBuffer[BUF_YUV_TEMP]->setColorProfile(eCOLORPROFILE_BT601_LIMITED);
    //
    {
        Mutex::Autolock _l(mLock);
        mbRecord = MTRUE;
    }
    //
    MY_LOGI("- User(%d), OpenId(%d), Rec(%d x %d), Fmt(%d)",
            mUser,
            mOpenId,
            mRecWidth,
            mRecHeight,
            mRecFormat);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
stopRecording()
{
    Mutex::Autolock lock(mLock);
    //
    MY_LOGI("+ Record(%d)",mbRecord);
    if(mbRecord)
    {
        MY_LOGI("freeBuffer");
        freeBuffer(mpImageBuffer[BUF_YUV_TEMP]);
        mbRecord = MFALSE;
    }
    else
    {
        MY_LOGW("record has been stopped!");
    }
    //
    FUNC_END_I;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
takePicture()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    //get information in parameter
    {
        sp<IParamsManagerV3> spParamsMgrV3 = mwpParamsMgrV3.promote();
        if( spParamsMgrV3 == NULL )
        {
            MY_LOGE("promote ParamsMgrV3 fail");
            return MFALSE;
        }
        //
        mRotation           = spParamsMgrV3->getParamsMgr()->getInt(CameraParameters::KEY_ROTATION);
        mJpegThumbWidth     = spParamsMgrV3->getParamsMgr()->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH);
        mJpegThumbHeight    = spParamsMgrV3->getParamsMgr()->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT);
        mJpegQuality        = spParamsMgrV3->getParamsMgr()->getInt(CameraParameters::KEY_JPEG_QUALITY);
        mJpegQualityThumb   = spParamsMgrV3->getParamsMgr()->getInt(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY);
        //
        MY_LOGI("thumb(%d x %d), rotation(%d), JPG(%d , %d)",
                mJpegThumbWidth,
                mJpegThumbHeight,
                mRotation,
                mJpegQuality,
                mJpegQualityThumb);
        //
        mGgpsEnabled = MTRUE;
        String8 gpsLatStr = spParamsMgrV3->getParamsMgr()->getStr(CameraParameters::KEY_GPS_LATITUDE);
        if( !gpsLatStr.isEmpty() )
        {
            String8 gpsLongStr = spParamsMgrV3->getParamsMgr()->getStr(CameraParameters::KEY_GPS_LONGITUDE);
            String8 gpsAltitudeStr = spParamsMgrV3->getParamsMgr()->getStr(CameraParameters::KEY_GPS_ALTITUDE);
            String8 gpsTimeStr = spParamsMgrV3->getParamsMgr()->getStr(CameraParameters::KEY_GPS_TIMESTAMP);
            mGpsProcessingMethod = spParamsMgrV3->getParamsMgr()->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD);
            if( gpsLongStr.isEmpty() || gpsAltitudeStr.isEmpty() || gpsTimeStr.isEmpty() || mGpsProcessingMethod.isEmpty() )
            {
                mGgpsEnabled = MFALSE;
                MY_LOGE("Incomplete set of GPS parameters provided");
            }
            else
            {
                char *endPtr;
                const char *gpsLat = gpsLatStr.string();
                mGpsCoordinates[0] = strtod(gpsLat, &endPtr);
                if( endPtr == gpsLat )
                {
                    MY_LOGE("Malformed GPS latitude: %s", gpsLat);
                    mGgpsEnabled = MFALSE;
                }
                const char *gpsLong = gpsLongStr.string();
                mGpsCoordinates[1] = strtod(gpsLong, &endPtr);
                if( endPtr == gpsLong )
                {
                    MY_LOGE("Malformed GPS longitude: %s", gpsLong);
                    mGgpsEnabled = MFALSE;
                }
                const char *gpsAltitude = gpsAltitudeStr.string();
                mGpsCoordinates[2] = strtod(gpsAltitude, &endPtr);
                if( endPtr == gpsAltitude )
                {
                    MY_LOGE("Malformed GPS altitude: %s", gpsAltitude);
                    mGgpsEnabled = MFALSE;
                }
                const char *gpsTime = gpsTimeStr.string();
                mGpsTimestamp = strtoll(gpsTime, &endPtr, 10);
                if( endPtr == gpsTime )
                {
                    MY_LOGE("Malformed GPS timestamp: %s", gpsTime);
                    mGgpsEnabled = MFALSE;
                }
            }
            MY_LOGD("GPS param: Lat(%s), Long(%s), Alt(%s), Time(%s); Result : Enable(%d), Coor(%lf,%lf,%lf), Time(%" PRId64 "), Method(%s)"
                    , gpsLatStr.string(), gpsLongStr.string(), gpsAltitudeStr.string(), gpsTimeStr.string()
                    , mGgpsEnabled, mGpsCoordinates[0], mGpsCoordinates[1], mGpsCoordinates[2], mGpsTimestamp, mGpsProcessingMethod.string());
        }
        else
        {
            MY_LOGD("KEY_GPS_LATITUDE is empty & set mGgpsEnabled to false");
            mGgpsEnabled = MFALSE;
        }
        //
        mFocalLength = spParamsMgrV3->getParamsMgr()->getFloat(MtkCameraParameters::KEY_FOCAL_LENGTH)*mFocalLengthBase;
    }
    //
    allocBuffer(
        mpImageBuffer[BUF_YUV_MAIN],
        mRecPicSizeWidth,
        mRecPicSizeHeight,
        mRecFormat);
    //allocate jpeg thumbnail buffer if need
    mbHasThumb = ( mJpegThumbWidth == 0 || mJpegThumbHeight == 0 ) ? MFALSE : MTRUE;
    if( mbHasThumb )
    {
        ret = allocBuffer(
                mpImageBuffer[BUF_YUV_THUMB],
                mJpegThumbWidth,
                mJpegThumbHeight,
                mRecFormat);
        if( !ret )
        {
            MY_LOGE("allocate YUY2 thumbnail buffer fail");
            return MFALSE;
        }
    }
    //
    if( mRotation == 0 ||
        mRotation == 180)
    {
        allocBuffer(
            mpImageBuffer[BUF_JPG],
            mRecPicSizeWidth,
            mRecPicSizeHeight,
            eImgFmt_JPEG);
    }
    else
    {
        allocBuffer(
            mpImageBuffer[BUF_JPG],
            mRecPicSizeHeight,
            mRecPicSizeWidth,
            eImgFmt_JPEG);
    }
    //wait input yuv buffer ready
    {
        Mutex::Autolock _l(mLock);
        mbNeedYuv = MTRUE;
        nsecs_t timeout = 1000LL*1000LL*1000LL;
        MY_LOGI("Wait for yuv buffer +");
        mCond.waitRelative(mLock, timeout);
        if( mbNeedYuv )
        {
            MY_LOGE("timeout cannot get yuv buffer");
            mbNeedYuv = MFALSE;
            return MFALSE;
        }
        MY_LOGI("Wait for yuv buffer -");
    }
    //
    {
        lockBuffer(BUF_YUV_TEMP);
        lockBuffer(BUF_YUV_MAIN);
        //
        ISImager *pISImager = ISImager::createInstance(mpImageBuffer[BUF_YUV_TEMP]);
        if( !pISImager )
        {
            MY_LOGE("create ISImager fail");
            unlockBuffer(BUF_YUV_TEMP);
            unlockBuffer(BUF_YUV_MAIN);
            ret = MFALSE;
            return ret;
        }
        //
        pISImager->setTargetImgBuffer(mpImageBuffer[BUF_YUV_MAIN]);
        pISImager->setTransform(0);
        //
        int imgW = mpImageBuffer[BUF_YUV_MAIN]->getImgSize().w;
        int imgH = mpImageBuffer[BUF_YUV_MAIN]->getImgSize().h;
        MRect SrcRect(MPoint(0, 0), MSize(mpImageBuffer[BUF_YUV_TEMP]->getImgSize().w, mpImageBuffer[BUF_YUV_TEMP]->getImgSize().h));
        MRect DstRect(MPoint(0, 0), MSize(imgW, imgH));
        MRect CropRect = calCrop(SrcRect, DstRect);
        MY_LOGD("src(%dx%d), dst(%dx%d), main crop(%d,%d,%dx%d)",
                SrcRect.s.w, SrcRect.s.h,
                DstRect.s.w, DstRect.s.h,
                CropRect.p.x, CropRect.p.y, CropRect.s.w, CropRect.s.h);
        pISImager->setCropROI(CropRect);
        pISImager->execute();
        pISImager->destroyInstance();
        pISImager = NULL;
        //
        unlockBuffer(BUF_YUV_TEMP);
        unlockBuffer(BUF_YUV_MAIN);
    }
    //
    if( mbHasThumb )
    {
        lockBuffer(BUF_YUV_MAIN);
        lockBuffer(BUF_YUV_THUMB);
        //
        ISImager *pISImager = ISImager::createInstance(mpImageBuffer[BUF_YUV_MAIN]);
        if( !pISImager )
        {
            MY_LOGE("create ISImager fail");
            unlockBuffer(BUF_YUV_MAIN);
            unlockBuffer(BUF_YUV_THUMB);
            ret = MFALSE;
            return ret;
        }
        //
        pISImager->setTargetImgBuffer(mpImageBuffer[BUF_YUV_THUMB]);
        pISImager->setTransform(0);
        //
        int imgW = mpImageBuffer[BUF_YUV_THUMB]->getImgSize().w;
        int imgH = mpImageBuffer[BUF_YUV_THUMB]->getImgSize().h;
        MRect SrcRect(MPoint(0, 0), MSize(mpImageBuffer[BUF_YUV_MAIN]->getImgSize().w, mpImageBuffer[BUF_YUV_MAIN]->getImgSize().h));
        MRect DstRect(MPoint(0, 0), MSize(imgW, imgH));
        MRect CropRect = calCrop(SrcRect, DstRect);
        MY_LOGD("src(%dx%d), dst(%dx%d), thumb crop(%d,%d,%dx%d)",
                SrcRect.s.w, SrcRect.s.h,
                DstRect.s.w, DstRect.s.h,
                CropRect.p.x, CropRect.p.y, CropRect.s.w, CropRect.s.h);
        pISImager->setCropROI(CropRect);
        pISImager->execute();
        pISImager->destroyInstance();
        pISImager = NULL;
        //
        unlockBuffer(BUF_YUV_MAIN);
        unlockBuffer(BUF_YUV_THUMB);
    }
    //
    ret = ret && processJpg();
    //
    freeBuffer(mpImageBuffer[BUF_YUV_MAIN]);
    if( mbHasThumb )
    {
        freeBuffer(mpImageBuffer[BUF_YUV_THUMB]);
    }
    //
    ret = ret && callbackJpg();
    //
    freeBuffer(mpImageBuffer[BUF_JPG]);
    //
    FUNC_END;
    return ret;
}

/*******************************************************************************
*
Some times, the video-size is not in picture-size values, so we need
to configure a suitable picture-size for VSS.
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
getBestPicSizeForVSS()
{
    if ( mRecHeight == 0 || mRecWidth == 0 || mwpParamsMgrV3 == NULL ) {
        MY_LOGI("something wrong.");
        return MFALSE;
    }
    sp<IParamsManagerV3> spParamsMgrV3 = mwpParamsMgrV3.promote();
    if ( spParamsMgrV3 == NULL ) {
        MY_LOGI("promote ParamsMgrV3 fail");
        return MFALSE;
    }
    CameraParameters parameters(spParamsMgrV3->getParamsMgr()->flatten());
    ::android::Vector<Size> sizes;
    parameters.getSupportedPictureSizes(sizes);

    bool find = false;
    int idx = -1;
    for ( int i = sizes.size()-1; i >= 0; i-- ) {
    //MY_LOGD("size(%d),i(%d),width(%d),height(%d)",sizes.size(),i,sizes[i].width,sizes[i].height);
        if ( sizes[i].width == mRecWidth && sizes[i].height == mRecHeight ) {
            find = true;
            break;
        }
        if ( sizes[i].width > mRecWidth && sizes[i].height > mRecHeight ) {
            idx = i;
        }
    }

    if ( find ) {
        mRecPicSizeWidth = mRecWidth;
        mRecPicSizeHeight = mRecHeight;
    }
    else
    if ( idx >= 0 ) {
        mRecPicSizeWidth = sizes[idx].width;
        mRecPicSizeHeight = sizes[idx].height;
    } else {
        MY_LOGW("something wrong for video-size.");
        int width,height;
        parameters.getPictureSize(&width,&height);
        mRecPicSizeWidth = width;
        mRecPicSizeHeight = height;
    }
    MY_LOGD("video-size(%d*%d), picture-size(%d*%d)",mRecWidth,mRecHeight,mRecPicSizeWidth,mRecPicSizeHeight);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
postBuffer(sp<IImageBufferHeap> spYuvBuf)
{
    MY_LOGD2("+");
    Mutex::Autolock _l(mLock);
    if( mbNeedYuv &&
        spYuvBuf->getImgFormat() == mpImageBuffer[BUF_YUV_TEMP]->getImgFormat() &&
        spYuvBuf->getImgSize()   == mpImageBuffer[BUF_YUV_TEMP]->getImgSize())
    {
        MY_LOGI("copy yuv buffer +");
        spYuvBuf->lockBuf(
                    LOG_TAG,
                    eBUFFER_USAGE_SW_READ_MASK);
        lockBuffer(BUF_YUV_TEMP);
        //
        for(MINT32 plane = 0 ; plane < spYuvBuf->getPlaneCount() ; plane++ )
        {
            memcpy(
                (void*)mpImageBuffer[BUF_YUV_TEMP]->getBufVA(plane),
                (void*)spYuvBuf->getBufVA(plane),
                mpImageBuffer[BUF_YUV_TEMP]->getBufSizeInBytes(plane));
            MY_LOGD("plane:%d, size:%d",
                    plane,
                    mpImageBuffer[BUF_YUV_TEMP]->getBufSizeInBytes(plane));
        }
        //
        unlockBuffer(BUF_YUV_TEMP);
        spYuvBuf->unlockBuf(LOG_TAG);
        MY_LOGI("copy yuv buffer -");
        mbNeedYuv = MFALSE;
        mCond.broadcast();
    }
    MY_LOGD2("-");
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
processJpg()
{
    FUNC_START_I;
    //
    MBOOL ret = MTRUE;
    //get metadata
    IMetadata appMeta, halMeta;
    {
        sp<IFrameInfo> pFrameInfo = IResourceContainer::getInstance(getOpenId())->queryLatestFrameInfo();
        if( pFrameInfo == NULL )
        {
            MY_LOGW("failed to queryLatestFrameInfo");
            return MFALSE;
        }
        pFrameInfo->getFrameMetadata(eSTREAMID_META_APP_DYNAMIC_P1, appMeta);
        pFrameInfo->getFrameMetadata(eSTREAMID_META_HAL_DYNAMIC_P1, halMeta);
        //
        updateEntry<MSize> (&appMeta, MTK_JPEG_THUMBNAIL_SIZE,      MSize(mJpegThumbWidth, mJpegThumbHeight));
        updateEntry<MUINT8>(&appMeta, MTK_JPEG_THUMBNAIL_QUALITY,   mJpegQualityThumb);
        updateEntry<MUINT8>(&appMeta, MTK_JPEG_QUALITY,             mJpegQuality);
        updateEntry<MINT32>(&appMeta, MTK_JPEG_ORIENTATION,         mRotation);
        //
        appMeta.remove(MTK_JPEG_GPS_COORDINATES);
        appMeta.remove(MTK_JPEG_GPS_TIMESTAMP);
        appMeta.remove(MTK_JPEG_GPS_PROCESSING_METHOD);
        if( mGgpsEnabled )
        {
            updateEntryArray(&appMeta, MTK_JPEG_GPS_COORDINATES,        mGpsCoordinates, 3);
            updateEntry<MINT64>(&appMeta, MTK_JPEG_GPS_TIMESTAMP ,      mGpsTimestamp);
            MUINT8 uGPSProcessingMethod[64]={0};
            const char *gpsProcessingMethod = mGpsProcessingMethod.string();
            for( size_t i = 0; i < mGpsProcessingMethod.size(); i++ ) {
                uGPSProcessingMethod[i] = *(gpsProcessingMethod+i);
            }
            updateEntryArray<MUINT8>(&appMeta, MTK_JPEG_GPS_PROCESSING_METHOD , uGPSProcessingMethod, 64);
        }
        //
        // 3A
        IMetadata exifMeta;
        if( !tryGetMetadata<IMetadata>(&halMeta, MTK_3A_EXIF_METADATA, exifMeta) )
        {
            MY_LOGD("no tag: MTK_3A_EXIF_METADATA, update focal-length(%d/%d)", mFocalLength, mFocalLengthBase);
            updateEntry<MINT32>(&exifMeta, MTK_3A_EXIF_FOCAL_LENGTH, mFocalLength);
            //
            MINT64 sensorExposureTime = 0;
            if(tryGetMetadata<MINT64>(&appMeta, MTK_SENSOR_EXPOSURE_TIME, sensorExposureTime))
            {
                updateEntry<MINT32> (&exifMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME, (MINT32)(sensorExposureTime/1000));
            }
            else
            {
                MY_LOGW("Cannot find MTK_SENSOR_EXPOSURE_TIME");
            }
            //
            MINT32 sensorSenstivity = 0;
            if(tryGetMetadata<MINT32>(&appMeta, MTK_SENSOR_SENSITIVITY, sensorSenstivity))
            {
                updateEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, sensorSenstivity);
            }
            else
            {
                MY_LOGW("Cannot find MTK_SENSOR_SENSITIVITY");
            }
            //
            updateEntry<IMetadata>(&halMeta, MTK_3A_EXIF_METADATA, exifMeta);
            MY_LOGI("FocalLength(%d/%d), sensorExposureTime(%d),sensorSenstivity(%d)",
                    mFocalLength,
                    mFocalLengthBase,
                    sensorExposureTime,
                    sensorSenstivity);
        }
    }
    // encode jpeg here
    {
        sp<ExifJpegUtils> pExifJpegUtils = ExifJpegUtils::createInstance(
                                                            getOpenId(),
                                                            halMeta,
                                                            appMeta,
                                                            mpImageBuffer[BUF_JPG]->getImageBufferHeap(),
                                                            mpImageBuffer[BUF_YUV_MAIN],
                                                            mbHasThumb ? mpImageBuffer[BUF_YUV_THUMB] : NULL);
        if( !pExifJpegUtils.get() )
        {
            MY_LOGE("create exif jpeg encode utils fail");
            return MFALSE;
        }
        pExifJpegUtils->setRotationMain(MTRUE);
        if( OK != pExifJpegUtils->execute() )
        {
            MY_LOGE("Exif Jpeg encode utils");
            return MFALSE;
        }
    }
    mJpgSize = mpImageBuffer[BUF_JPG]->getImageBufferHeap()->getBitstreamSize();
    //
    MY_LOGI("- JPG bitStrSize(All/Main)=(%d/%d)",
            mpImageBuffer[BUF_JPG]->getBitstreamSize(),
            mJpgSize);
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
callbackJpg()
{
    FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    sp<IShotCallback> spShotCallback = mwpShotCallback.promote();
    if( spShotCallback != NULL )
    {
        spShotCallback->onCB_Shutter(true, 0, eShotMode_VideoSnapShot);
        spShotCallback->onCB_RawImage(0, 0, NULL);
        //
        lockBuffer(BUF_JPG);
        spShotCallback->onCB_CompressedImage_packed(
                            0,
                            mJpgSize,
                            (MUINT8*)(mpImageBuffer[BUF_JPG]->getBufVA(0)),
                            0,
                            true,
                            0,
                            eShotMode_VideoSnapShot);
        unlockBuffer(BUF_JPG);
    }
    else
    {
        MY_LOGE("ShotCallback promote fail");
        ret = MFALSE;
    }
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
allocBuffer(
    IImageBuffer*&      pImageBuffer,
    MUINT32             width,
    MUINT32             height,
    MUINT32             format)
{
    FUNC_START;
    //
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    //allocate buffer
    MUINT32 bufStrideInByte[3] = {  ALIGN_UP_SIZE(width,    REC_IMG_STRIDE_Y),
                                    ALIGN_UP_SIZE(width/2,  REC_IMG_STRIDE_U),
                                    ALIGN_UP_SIZE(width/2,  REC_IMG_STRIDE_V)};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

    MY_LOGD("+ S(%dx%d), fmt(0x%x), stride(%d/%d/%d)",
            width,
            height,
            format,
            bufStrideInByte[0],
            bufStrideInByte[1],
            bufStrideInByte[2]);

#define STD_EXIF_SIZE (1280)
#define MAX_THUMBNAIL_SIZE (64*1024)
    if(format == eImgFmt_JPEG )
    {
        MUINT32 bufsize = width*height*18/10 + DBG_EXIF_SIZE + STD_EXIF_SIZE + MAX_THUMBNAIL_SIZE;
        IImageBufferAllocator::ImgParam imgParam(
                MSize(width,height),
                bufsize,
                0);
        pImageBuffer = allocator->alloc_ion(LOG_TAG, imgParam);

    }
    else
    {
        IImageBufferAllocator::ImgParam imgParam(
                format,
                MSize(width,height),
                bufStrideInByte,
                bufBoundaryInBytes,
                Format::queryPlaneCount(format));
        pImageBuffer = allocator->alloc_ion(LOG_TAG, imgParam);
    }
#undef STD_EXIF_SIZE
#undef MAX_THUMBNAIL_SIZE
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RecordBufferSnapShotImp::
freeBuffer(IImageBuffer*& pImageBuffer)
{
    FUNC_START;
    //
    MBOOL ret = MTRUE;
    if( pImageBuffer == NULL )
    {
        return ret;
    }
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    //
    allocator->free(pImageBuffer);
    pImageBuffer = NULL;
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
RecordBufferSnapShotImp::
lockBuffer(MUINT32 type)
{
    FUNC_START;
    //
    mpImageBuffer[type]->lockBuf(
                            LOG_TAG,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_MASK);
    //mpImageBuffer[type]->syncCache(eCACHECTRL_INVALID);
    //
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
RecordBufferSnapShotImp::
unlockBuffer(MUINT32 type)
{
    FUNC_START;
    //
    mpImageBuffer[type]->unlockBuf(LOG_TAG);
    //
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
MRect
RecordBufferSnapShotImp::
calCrop(MRect src, MRect dst)
{
    MRect crop;

    // srcW/srcH < dstW/dstH
    if(src.s.w * dst.s.h < dst.s.w * src.s.h)
    {
        crop.s.w = src.s.w;
        crop.s.h = src.s.w * dst.s.h / dst.s.w;
    }
    //srcW/srcH > dstW/dstH
    else if(src.s.w * dst.s.h > dst.s.w * src.s.h)
    {
        crop.s.w = src.s.h * dst.s.w / dst.s.h;
        crop.s.h = src.s.h;
    }
    else
    {
        crop.s.w = src.s.w;
        crop.s.h = src.s.h;
    }
    //
    crop.s.w =  ROUND_TO_2X(crop.s.w);
    crop.s.h =  ROUND_TO_2X(crop.s.h);
    //
    crop.p.x = (src.s.w - crop.s.w) / 2;
    crop.p.y = (src.s.h - crop.s.h) / 2;

    return crop;
}
