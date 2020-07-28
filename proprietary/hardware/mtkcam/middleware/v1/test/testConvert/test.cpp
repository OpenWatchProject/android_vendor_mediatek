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
#define LOG_TAG "CONVERT_TEST"

#include <hardware/camera3.h> // for template

#include <v1/converter/ConvertParam.h>
#include <v1/converter/RequestSettingBuilder.h>
//#include <v1/converter/InFlightRequest.h>
#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <Hal3/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/drv/IHalSensor.h> // for searchSensor
#include <mtkcam/utils/metadata/IMetadataTagSet.h>
#include <mtkcam/utils/metadata/IMetadataConverter.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/metadata/mtk_metadata_types.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>

using namespace android;
using namespace NSCam;
using namespace std;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define TEST(cond, result)          do { if ( (cond) == (result) ) { printf("Pass\n"); } else { printf("Failed\n"); } }while(0)
#define FUNCTION_IN     MY_LOGD_IF(1, "+");

/******************************************************************************
 *
 ******************************************************************************/
 namespace {
    IHalSensor* mpSensorHalObj;
    static int gSensorId = 0;
 };
 /******************************************************************************
 *
 ******************************************************************************/
void prepareSensor()
{
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();
    mpSensorHalObj = pHalSensorList->createSensor("tester", gSensorId);
    MUINT32    sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOn("tester", 1, &sensorArray[0]);
}

/******************************************************************************
 *
 ******************************************************************************/
void closeSensor()
{
    MUINT32    sensorArray[1] = {(MUINT32)gSensorId};
    mpSensorHalObj->powerOff("tester", 1, &sensorArray[0]);
    mpSensorHalObj->destroyInstance("tester");
    mpSensorHalObj = NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
int main(int /*argc*/, char** /*argv*/)
{
    printf(" searchSensors\n");
    //MAKE_HalSensorList()->searchSensors();
    prepareSensor();
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(gSensorId);
    NSMetadataProviderManager::add(gSensorId, pMetadataProvider.get());

    MtkCameraParameters parameters;

    printf("prepare parameters\n");
    {
        MSize thumbSize(128, 106);
        int jpegQ = 90;
        int rot = 180;
        int fps_range_m = 5000;
        int fps_range_l = 15000;
        int zoom = 6;
        parameters.setPreviewSize(1980, 1080);
        parameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, thumbSize.w);
        parameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, thumbSize.h);
        parameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, jpegQ);
        parameters.set(CameraParameters::KEY_JPEG_QUALITY, jpegQ);
        parameters.set(CameraParameters::KEY_ROTATION, rot);
        parameters.set(CameraParameters::KEY_ZOOM, zoom);
        String8 str = String8::format("%d,%d", fps_range_m, fps_range_l);
        parameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, str.string());

        const char *gps_a = "25.032146";
        parameters.set(CameraParameters::KEY_GPS_LATITUDE, gps_a);
        const char *gps_b = "121.564448";
        parameters.set(CameraParameters::KEY_GPS_LONGITUDE, gps_b);
        const char *gps_c = "21.0";
        parameters.set(CameraParameters::KEY_GPS_ALTITUDE, gps_c);
        const char *gps_d = "1251192757";
        parameters.set(CameraParameters::KEY_GPS_TIMESTAMP, gps_d);
        const char *gps_e = "GPS";
        parameters.set(CameraParameters::KEY_GPS_PROCESSING_METHOD, gps_e);

        const char *ratio = "100,114,132,151,174,200,229,263,303,348,400";
        parameters.set(CameraParameters::KEY_ZOOM_RATIOS, ratio);
        printf("%s\n", ratio);
        const char *effect = "none";
        parameters.set(CameraParameters::KEY_EFFECT, effect);

        const char *anti = "auto";
        parameters.set(CameraParameters::KEY_ANTIBANDING, anti);

        const char *sceneM = "beach";
        parameters.set(CameraParameters::KEY_SCENE_MODE, sceneM);

        const char *flashM = "torch";
        parameters.set(CameraParameters::KEY_FLASH_MODE, flashM);

        const char *awbM = "shade";
        parameters.set(CameraParameters::KEY_WHITE_BALANCE, awbM);

        const char *focusM = "fixed";
        parameters.set(CameraParameters::KEY_FOCUS_MODE, focusM);

        int exposure = 0;
        parameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, exposure);

        const char *bool_1 = "true";
        parameters.set(CameraParameters::KEY_AUTO_EXPOSURE_LOCK, bool_1);

        const char *bool_2 = "true";
        parameters.set(CameraParameters::KEY_AUTO_WHITEBALANCE_LOCK, bool_2);

        const char *bool_3 = "true";
        parameters.set(CameraParameters::KEY_VIDEO_STABILIZATION, bool_3);
    }


    //Param.set(parameters);

    printf("createDefaultRequest\n");
    IMetadata metadata1;
    sp<RequestSettingBuilder> RequestBuilder = RequestSettingBuilder::createInstance(gSensorId);

    static int requestTemplate = CAMERA3_TEMPLATE_PREVIEW;//CAMERA3_TEMPLATE_STILL_CAPTURE;
    MERROR err = RequestBuilder->createDefaultRequest(requestTemplate, metadata1);
    if(err != OK){
        printf("can't not create default request!\n");
    }

    printf("dump old meta to log\n");
    IMetadataTagSet const &mtagInfo = IDefaultMetadataTagSet::singleton()->getTagSet();
    sp<IMetadataConverter> mMetaDataConverter = IMetadataConverter::createInstance(mtagInfo);
    mMetaDataConverter->dumpAll(metadata1);

    printf("convert parameter\n");
    // put default metadata and parameter setting in
    // get metadata out
    ConvertParam Param(gSensorId);
    //printf("updateRequest\n");
    //Param.updateRequest(&metadata1);
    //Param.updateRequestJpeg(&metadata1);
    Param.convert(parameters, &metadata1);
    /*{
        IMetadata::IEntry &entry1 = metadata1.editEntryFor(MTK_JPEG_THUMBNAIL_SIZE);
        MSize size = entry1.itemAt(0, Type2Type< MSize >());
        printf("w=%d, h=%d\n", size.w, size.h);

        IMetadata::IEntry &entry2 = metadata1.editEntryFor(MTK_JPEG_THUMBNAIL_QUALITY);
        int quality = entry2.itemAt(0, Type2Type< MUINT8 >());
        printf("quality = %d\n", quality);

        IMetadata::IEntry &entry3 = metadata1.editEntryFor(MTK_CONTROL_AE_TARGET_FPS_RANGE);
        printf("min = %d max = %d\n",entry3.itemAt(0, Type2Type< MINT32 >()), entry3.itemAt(1, Type2Type< MINT32 >()));

        IMetadata::IEntry &entry4 = metadata1.editEntryFor(MTK_CONTROL_AE_EXPOSURE_COMPENSATION);
        printf("exposure = %d",entry4.itemAt(0, Type2Type< MINT32 >()));

        //IMetadata::IEntry &entry5 = metadata1.editEntryFor(MTK_FLASH_MODE);
        //printf("flash mode = %d",entry5.itemAt(0, Type2Type< MUINT8 >()));

    }*/
    printf("dump after meta\n");
    //mMetaDataConverter->dump(metadata1);
    mMetaDataConverter->dumpAll(metadata1);

    printf("========================test run2========================\n");
    IMetadata metadata2;
    {
    //int requestId = 10;
    //RequestBuilder.triggerPrecaptureMetering(1);
    //RequestBuilder.triggerAutofocus(2);
    //RequestBuilder.triggerCancelAutofocus(3);
    //RequestBuilder.triggerTriggerZoom(4);
    printf("setRequest\n");
    RequestBuilder->setStreamingRequest(metadata1);

    printf("getRequest\n");
    int frameNo = 1;
    RequestBuilder->getRequest(frameNo, metadata2);
    mMetaDataConverter->dumpAll(metadata2);
  #if 0
    //check request id
    IMetadata::IEntry &entry = metadata2.editEntryFor(MTK_REQUEST_ID);
    printf("request id = %d/%d\n", requestId, entry.itemAt(0, Type2Type< MINT32 >()));

    android::sp<InFlightRequest> inFlightR;
    inFlightR = RequestBuilder.getInFlightRequest();

    InFlightRequest::FlightRequest trigger = inFlightR->deque(frameNo);
    printf("precaptureTriggerId = %d afTriggerId = %d zoomIndex = %d\n"
                            , trigger.precaptureTriggerId, trigger.afTriggerId
                            , trigger.zoomIndex
                            );
#endif
    }
 #if 0
    printf("========================test run3========================\n");
    {
    int requestId = 11;
    IMetadata metadata3;

    RequestBuilder.triggerPrecaptureMetering(4);
    RequestBuilder.triggerAutofocus(5);
    RequestBuilder.triggerCancelAutofocus(6);
    printf("capture\n");
    RequestBuilder.capture(metadata1, requestId);

    printf("getCaptureRequest\n");
    int frameNo = RequestBuilder.getCaptureRequest(metadata3);

    mMetaDataConverter->dumpAll(metadata3);

    //check request id
    IMetadata::IEntry &entry = metadata3.editEntryFor(MTK_REQUEST_ID);
    printf("request id = %d/%d\n", requestId, entry.itemAt(0, Type2Type< MINT32 >()));
/*
    android::sp<InFlightRequest> inFlightR;
    inFlightR = RequestBuilder.getInFlightRequest();

    InFlightRequest::FlightRequest trigger = inFlightR->deque(frameNo);
    printf("precaptureTriggerId = %d afTriggerId = %d zoomIndex = %d\n"
                            , trigger.precaptureTriggerId, trigger.afTriggerId
                            , trigger.zoomIndex
                            );
*/
    }
    closeSensor();
    printf("end test\n");
    return 0;
#endif
}
