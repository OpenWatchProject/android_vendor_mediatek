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

#define LOG_TAG "MtkCam/EngSelector"
//
#include "MyUtils.h"
//
#include <utils/RWLock.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include "EngSelector.h"
//
#include <mtkcam/drv/IHalSensor.h>


using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::Utils;

using namespace NSCam::v3::Utils;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

#define MAX_VIDEO_DUMPCNT  200
static uint32_t g_u4RawDumpCnt = 0;

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

/******************************************************************************
 *
 ******************************************************************************/
template<typename T>
inline MBOOL QUERY_ENTRY_SINGLE(const IMetadata& metadata, MINT32 entry_tag, T& item)
{
    IMetadata::IEntry entry = metadata.entryFor(entry_tag);
    if (entry.tag() != IMetadata::IEntry::BAD_TAG)
    {
        item = entry.itemAt(0, NSCam::Type2Type< T >());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *dumpDebugInfo
 ******************************************************************************/
static
inline MBOOL
_dumpDebugInfo(const char* filename, const IMetadata& metaExif)
{
    FILE* fid = fopen(filename, "wb");
    if (fid)
    {
        IMetadata::Memory p3ADbg;
        if (QUERY_ENTRY_SINGLE<IMetadata::Memory>(metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, p3ADbg))
        {
            //MY_LOG("[%s] %s, 3A(%p, %d)", __FUNCTION__, filename, p3ADbg->array(), p3ADbg->size());
            MUINT8 hdr[6] = {0, 0, 0xFF, 0xE6, 0, 0};
            MUINT16 size = (MUINT16)(p3ADbg.size()+2);
            hdr[4] = (size >> 8); // big endian
            hdr[5] = size & 0xFF;
            fwrite(hdr, 6, 1, fid);
            fwrite(p3ADbg.array(), p3ADbg.size(), 1, fid);
        }
        IMetadata::Memory pIspDbg;
        if (QUERY_ENTRY_SINGLE<IMetadata::Memory>(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, pIspDbg))
        {
            //MY_LOG("[%s] %s, ISP(%p, %d)", __FUNCTION__, filename, pIspDbg->array(), pIspDbg->size());
            MUINT8 hdr[4] = {0xFF, 0xE7, 0, 0};
            MUINT16 size = (MUINT16)(pIspDbg.size()+2);
            hdr[2] = (size >> 8);
            hdr[3] = size & 0xFF;
            fwrite(hdr, 4, 1, fid);
            fwrite(pIspDbg.array(), pIspDbg.size(), 1, fid);
        }
        IMetadata::Memory pShdDbg;
        if (QUERY_ENTRY_SINGLE<IMetadata::Memory>(metaExif, MTK_3A_EXIF_DBGINFO_SDINFO_DATA, pShdDbg))
        {
            //MY_LOG("[%s] %s, SHAD(%p, %d)", __FUNCTION__, filename, pShdDbg->array(), pShdDbg->size());
            MUINT8 hdr[4] = {0xFF, 0xE8, 0, 0};
            MUINT16 size = (MUINT16)(pShdDbg.size()+2);
            hdr[2] = (size >> 8);
            hdr[3] = size & 0xFF;
            fwrite(hdr, 4, 1, fid);
            fwrite(pShdDbg.array(), pShdDbg.size(), 1, fid);
        }
        fclose(fid);
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
EngSelector::
EngSelector()
{
}

/******************************************************************************
 *
 ******************************************************************************/
EngSelector::
~EngSelector()
{
    // start of Video Raw Dump
    int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    if (camera_mode != 0)
    {
        sp<IRawDumpCmdQueThread> pRawDumpCmdQueThread = mpRawDumpCmdQueThread;
        if ( pRawDumpCmdQueThread  != 0 ) {
            MY_LOGD(
                "RawDumpCmdQ Thread: (tid, getStrongCount)=(%d, %d)",
                pRawDumpCmdQueThread->getTid(), pRawDumpCmdQueThread->getStrongCount()
            );
            pRawDumpCmdQueThread->requestExit();
        }
    }
    // end of Video Raw Dump
}

/******************************************************************************
 *
 ******************************************************************************/
void
EngSelector::
setConfig(
    sp<IParamsManagerV3>  pParamsManagerV3,
    MINT32 const          i4OpenId,
    sp<INotifyCallback>   pCamMsgCbInfo
)
{
    mpParamsManagerV3 = pParamsManagerV3;
    mOpenId = i4OpenId;
    mpCamMsgCbInfo = pCamMsgCbInfo;
    //eng mode video raw dump
    {
        int camera_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        int tg_out_mode = 0; // NORNAL
        MY_LOGD("camera mode = %d", camera_mode);
        if (camera_mode == 1)
        {
            tg_out_mode = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_PREVIEW_DUMP_RESOLUTION);
            MY_LOGD("tg_out_mode = %d", tg_out_mode);
        }

        if (camera_mode != 0)
        {

            MY_LOGD("create RawDumpThread instance");

            status_t status = NO_ERROR;

            NSCam::IHalSensorList *pSensorHalList = NULL;
            NSCam::SensorStaticInfo sensorStaticInfo;

            pSensorHalList = MAKE_HalSensorList();
            //
            if( pSensorHalList != NULL )
            {
                    MUINT32 sensorDev = pSensorHalList->querySensorDevIdx(mOpenId);
                    pSensorHalList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

                    MUINT32 rawSensorBit = 0;
                    switch (sensorStaticInfo.rawSensorBit)
                    {
                        case RAW_SENSOR_8BIT:
                             rawSensorBit = 8;
                             break;
                        case RAW_SENSOR_10BIT:
                             rawSensorBit = 10;
                             break;
                        case RAW_SENSOR_12BIT:
                             rawSensorBit = 12;
                             break;
                        case RAW_SENSOR_14BIT:
                             rawSensorBit = 14;
                             break;
                    }
                    //
                    mpRawDumpCmdQueThread = IRawDumpCmdQueThread::createInstance(sensorStaticInfo.sensorFormatOrder, rawSensorBit, mpParamsManagerV3->getParamsMgr());
                    if  ( mpRawDumpCmdQueThread  == 0 || OK != (status = mpRawDumpCmdQueThread->run("RawDumpCmdQueThread")) )
                    {
                        MY_LOGE(
                           "Fail to run mpRawDumpCmdQueThread  - mpRawDumpCmdQueThread .get(%p), status[%s(%d)]",
                           mpRawDumpCmdQueThread.get(), ::strerror(-status), -status
                        );
                        g_u4RawDumpCnt = 0;
                       // goto lbExit;
                    }

                    MY_LOGD("mpRawDumpCmdQueThread::setCallbacks is called");
                    mpRawDumpCmdQueThread->setCallbacks(mpCamMsgCbInfo);
            }
            else
            {
                MY_LOGE("pSensorHalList is NULL, videorawdump cant work");
            }
        }
    }
    char rRawCntValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.eng.video.dumpcnt", rRawCntValue, "0");
    mu4MaxRawDumpCnt = (atoi(rRawCntValue) == 0) ? MAX_VIDEO_DUMPCNT : atoi(rRawCntValue);
    MY_LOGD("mu4MaxRawDumpCnt:%d", mu4MaxRawDumpCnt);
    //eng mode video raw dump
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
EngSelector::
selectResult(
    MINT32                             rRequestNo,
    Vector<MetaItemSet>                rvResult,
    Vector<BufferItemSet>              rvBuffers,
    MBOOL                              errorResult
)
{
    MY_LOGD("selectResult rNo(%d)",rRequestNo);
    //
    if( errorResult )
    {
        MY_LOGW("don't reserved errorResult(1) requestNo(%d)", rRequestNo);
        //
        returnBuffersToPool(rvBuffers);
        return UNKNOWN_ERROR;
    }
    //
    { // start of Video Raw Dump

        static bool mEnableRawDump = false;
        //(4) enable RawDump
        {
            String8 const s = mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_RAW_DUMP_FLAG);
#undef TRUE
#undef FALSE
            bool RawDumpFlag = ( ! s.isEmpty() && s == CameraParameters::TRUE ) ? 1 : 0;

            if ( mEnableRawDump != RawDumpFlag )
            {
                MY_LOGD("RawDump flag changed to %d ", RawDumpFlag);
                mEnableRawDump = RawDumpFlag;
                 g_u4RawDumpCnt = 0;
            }
        }

        if (mpRawDumpCmdQueThread != 0 && (g_u4RawDumpCnt < mu4MaxRawDumpCnt) && mEnableRawDump == true )
        {
            MY_LOGD("u4RawDumpCnt, Max = (%d, %d)", g_u4RawDumpCnt, mu4MaxRawDumpCnt); // debug
            //
            //dump debug info
            for(int i=0 ; i < (int)rvResult.size() ; i++)
            {
                if(rvResult[i].id == NSCam::eSTREAMID_META_HAL_DYNAMIC_P2)
                {
                    IMetadata exifMeta;
                    tryGetMetadata<IMetadata>(&rvResult[i].meta, MTK_3A_EXIF_METADATA, exifMeta);
                    //
                    {
                        // write buffer[0-#] into disc
                        String8 s8RawFilePath = mpParamsManagerV3->getParamsMgr()->getStr(MtkCameraParameters::KEY_RAW_PATH);
                        String8 ms8RawFileExt(s8RawFilePath.getPathExtension()); // => .raw
                        s8RawFilePath = s8RawFilePath.getBasePath(); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0
                        //
                        char mpszSuffix[256] = {0};
                        sprintf(mpszSuffix, "__%d.bin", rRequestNo); /* info from EngShot::onCmd_capture */
                        s8RawFilePath.append(mpszSuffix);
                        //
                        _dumpDebugInfo(s8RawFilePath.string(), exifMeta);
                    }
                }
            }
            //
            int rawoutport = mpParamsManagerV3->getParamsMgr()->getInt(MtkCameraParameters::KEY_ENG_RAW_OUTPUT_PORT);
            //
            for(size_t i = 0; i < rvBuffers.size(); i++)
            {
                if(rawoutport == 0)
                {
                    if(rvBuffers[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
                    {
                        sp<IImageBuffer> buf = rvBuffers[i].heap->createImageBuffer();
                        //
                        mpRawDumpCmdQueThread->postCommand(buf.get(), rRequestNo);
                        g_u4RawDumpCnt++;
                    }
                }
                else if(rawoutport == 1)
                {
                    if(rvBuffers[i].id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
                    {
                        sp<IImageBuffer> buf = rvBuffers[i].heap->createImageBuffer();
                        //
                        mpRawDumpCmdQueThread->postCommand(buf.get(), rRequestNo);
                        g_u4RawDumpCnt++;
                    }
                }
            }

        }
        else if (g_u4RawDumpCnt == mu4MaxRawDumpCnt)
        {
            MY_LOGD("Stop");
            mpParamsManagerV3->getParamsMgr()->set(MtkCameraParameters::KEY_RAW_DUMP_FLAG, CameraParameters::FALSE);
            mpRawDumpCmdQueThread->postCommand(NULL, 0);
            mEnableRawDump=false;
            g_u4RawDumpCnt=0;
        }
    } // end of Video Raw Dump
    //
    returnBuffersToPool(rvBuffers);
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngSelector::
returnBuffer(
    BufferItemSet&    rBuffer
)
{
    return returnBufferToPool(rBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
EngSelector::
flush()
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);

    if ( ! mResultSetMap.empty() ) {
        for ( size_t i = 0; i < mResultSetMap.size(); ++i ) {
            returnBuffersToPool(mResultSetMap.editItemAt(i).bufferSet);
        }
    }

    mResultSetMap.clear();
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
EngSelector::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;
    flush();
    FUNC_END;
}

