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

#ifndef _MTK_CAMERA_CAMADAPTER_INC_SCENARIO_SHOT_ISHOT_H_
#define _MTK_CAMERA_CAMADAPTER_INC_SCENARIO_SHOT_ISHOT_H_
//
#include <utils/RefBase.h>
#include <utils/String8.h>

#include <mtkcam/def/common.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>

namespace android {
namespace NSShot {
/******************************************************************************
 *
 ******************************************************************************/
class ImpShot;

enum EIShotDataMsg
{
    EIShot_DATA_MSG_NONE           = 0x0000,           /*!< none data  callback message */
    EIShot_DATA_MSG_RAW            = 0x0001,           /*!< raw data callback message */
    EIShot_DATA_MSG_YUV            = 0x0002,           /*!< yuv data callback message */
    EIShot_DATA_MSG_POSTVIEW       = 0x0004,           /*!< postview data callback message */
    EIShot_DATA_MSG_JPEG           = 0x0008,           /*!< jpeg data callback message */
    EIShot_DATA_MSG_EIS            = 0x0010,           /*!< eis data callback message (ext1) */
    EIShot_DATA_MSG_JPS            = 0x0020,           /*!< jps data callback message */
    EIShot_DATA_MSG_ALL            = 0x003F,           /*!< all data callback message */
};

/******************************************************************************
 *  Interface of Shot Callback.
 ******************************************************************************/
class IShotCallback : public virtual RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~IShotCallback() {}

public:     ////                    Operations.

    //
    //  Callback of Error (CAMERA_MSG_ERROR)
    //
    //  Arguments:
    //      ext1
    //          [I] extend argument 1.
    //
    //      ext2
    //          [I] extend argument 2.
    //
    virtual bool                    onCB_Error(
                                        int32_t ext1 = 0,
                                        int32_t ext2 = 0
                                    )   = 0;

    //
    //  Callback of Shutter (CAMERA_MSG_SHUTTER)
    //
    //      Invoking this callback may play a shutter sound.
    //
    //  Arguments:
    //      bPlayShutterSound
    //          [I] Play a shutter sound if ture; otherwise play no sound.
    //
    //      u4CallbackIndex
    //          [I] Callback index. 0 by default.
    //              If more than one shutter callback must be invoked during
    //              captures, for example burst shot & ev shot, this value is
    //              the callback index; and 0 indicates the first one callback.
    //
    virtual bool                    onCB_Shutter(
                                        bool const bPlayShutterSound = true,
                                        uint32_t const u4CallbackIndex = 0,
                                        uint32_t const  u4Shotmode = NSCam::eShotMode_Undefined
                                    )   = 0;

    //
    //  Callback of Postview for Client (CAMERA_MSG_POSTVIEW_FRAME)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Postview timestamp
    //
    //      u4PostviewSize
    //          [I] Postview buffer size in bytes.
    //
    //      puPostviewBuf
    //          [I] Postview buffer with its size = u4PostviewSize
    //
    virtual bool                    onCB_PostviewClient(
                                        int64_t const   i8Timestamp,
                                        uint32_t const  u4PostviewSize,
                                        uint8_t const*  puPostviewBuf,
                                        uint32_t const  u4CbId = NSCam::MTK_CAMERA_CALLBACK_CLIENT_ID_SINGLE
                                    )   = 0;

    //
    //  Callback of Postview for Client (CAMERA_MSG_POSTVIEW_FRAME)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Postview timestamp
    //
    //      pPostviewBuf
    //          [I] Postview IImageBuffer
    //
    virtual bool                    onCB_PostviewClient(
                                        int64_t const   i8Timestamp,
                                        NSCam::IImageBuffer const *pPostviewBuf,
                                        uint32_t const  u4BufOffset4Ts = 0,
                                        uint32_t const  u4BgServ = 0,
                                        uint32_t const  u4CbId = NSCam::MTK_CAMERA_CALLBACK_CLIENT_ID_SINGLE
                                    )   = 0;

    //
    //
    //  Callback of Raw Image (MTK_CAMERA_MSG_EXT_DATA_RAW16)
    //
    //  Arguments:
    //      pRawImgBuf
    //          [I] Raw16 IImageBuffer
    //
    //
    virtual bool                    onCB_Raw16Image(
                                        NSCam::IImageBuffer const *pRawImgBuf __attribute__((unused))
                                    )   {return MTRUE;};

    //
    //  Callback of Raw Image (MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONEY)
    //
    //  Arguments:
    //      pDngMeta
    //            [I] Raw16 Metadata
    //
    //
    virtual bool                    onCB_DNGMetaData(
                                     MUINTPTR const pDngMeta __attribute__((unused))
                                 )   {return MTRUE;};

    //
    //  Callback of Raw Image (MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONEY)
    //
    //  Arguments:
    //      pDngMeta
    //            [I] Raw16 Metadata
    //
    //
    virtual bool                    onCB_3rdPartyMetaData(
                                     MUINTPTR const pDngMeta __attribute__((unused))
                                 )   {return MTRUE;};

    //
    //  Callback of Raw Image (CAMERA_MSG_RAW_IMAGE/CAMERA_MSG_RAW_IMAGE_NOTIFY)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Raw image timestamp
    //
    //      u4RawImgSize
    //          [I] Raw image buffer size in bytes.
    //
    //      puRawImgBuf
    //          [I] Raw image buffer with its size = u4RawImgSize
    //
    virtual bool                    onCB_RawImage(
                                        int64_t const   i8Timestamp,
                                        uint32_t const  u4RawImgSize,
                                        uint8_t const*  puRawImgBuf
                                    )   = 0;

    //
    //  Callback of Compressed Image (CAMERA_MSG_COMPRESSED_IMAGE)
    //
    //      [Compressed Image] = [Header] + [Bitstream],
    //      where
    //          Header may be jpeg exif (including thumbnail)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Compressed image timestamp
    //
    //      u4BitstreamSize
    //          [I] Bitstream buffer size in bytes.
    //
    //      puBitstreamBuf
    //          [I] Bitstream buffer with its size = u4BitstreamSize
    //
    //      u4HeaderSize
    //          [I] Header size in bytes; header may be jpeg exif.
    //
    //      puHeaderBuf
    //          [I] Header buffer with its size = u4HeaderSize
    //
    //      u4CallbackIndex
    //          [I] Callback index. 0 by default.
    //              If more than one compressed callback must be invoked during
    //              captures, for example burst shot & ev shot, this value is
    //              the callback index; and 0 indicates the first one callback.
    //
    //      fgIsFinalImage
    //          [I] booliean value to indicate whether it is the final image.
    //              true if this is the final image callback; otherwise false.
    //              For single captures, this value must be true.
    //
    virtual bool                    onCB_CompressedImage(
                                        int64_t const   i8Timestamp,
                                        uint32_t const  u4BitstreamSize,
                                        uint8_t const*  puBitstreamBuf,
                                        uint32_t const  u4HeaderSize,
                                        uint8_t const*  puHeaderBuf,
                                        uint32_t const  u4CallbackIndex = 0,
                                        bool            fgIsFinalImage = true,
                                        uint32_t const  msgType = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                        uint32_t const  u4Shotmode = NSCam::eShotMode_Undefined
                                    )   = 0;

    //
    //  Callback of Packed Compressed Image (CAMERA_MSG_COMPRESSED_IMAGE)
    //
    //      [Compressed Image] = [Header] + [Bitstream],
    //      where
    //          Header may be jpeg exif (including thumbnail)
    //
    //  Arguments:
    //      i8Timestamp
    //          [I] Compressed image timestamp
    //
    //      u4BitstreamSize
    //          [I] Compressed Image buffer size in bytes.
    //
    //      puBitstreamBuf
    //          [I] Compressed Image buffer with its size = u4BitstreamSize
    //
    //      u4CallbackIndex
    //          [I] Callback index. 0 by default.
    //              If more than one compressed callback must be invoked during
    //              captures, for example burst shot & ev shot, this value is
    //              the callback index; and 0 indicates the first one callback.
    //
    //      fgIsFinalImage
    //          [I] booliean value to indicate whether it is the final image.
    //              true if this is the final image callback; otherwise false.
    //              For single captures, this value must be true.
    //
    //      u4BgServ
    //          [I] This method will callback message CAMERA_MSG_COMPRESSED_IMAGE
    //              via standard framework, or MMSDK. If this value is 1, this message
    //              will be sent via MMSDK. (Default is 0, using standard way).
    //
    virtual bool                    onCB_CompressedImage_packed(
                                        int64_t const   i8Timestamp __attribute__((unused)),
                                        uint32_t const  u4BitstreamSize __attribute__((unused)),
                                        uint8_t const*  puBitstreamBuf __attribute__((unused)),
                                        uint32_t const  u4CallbackIndex __attribute__((unused)) = 0,
                                        bool            fgIsFinalImage __attribute__((unused)) = true,
                                        uint32_t const  msgType __attribute__((unused)) = MTK_CAMERA_MSG_EXT_DATA_COMPRESSED_IMAGE,
                                        uint32_t const  u4Shotmode __attribute__((unused)) = NSCam::eShotMode_Undefined,
                                        uint32_t const  u4BgServ __attribute__((unused)) = 0
                                    ) { return false; }

    //
    //  Callback of P2done (MTK_CAMERA_MSG_EXT_NOTIFY_P2DONE)
    //
    //  Arguments:
    //      bNofityCaptureDone
    //          [I] To send MTK_CAMERA_MSG_EXT_NOTIFY_CAPTURE_DONE or not after
    //              sent MTK_CAMERA_MSG_EXT_NOTIFY_P2DONE. (default is NOT).
    //
    virtual bool                    onCB_P2done(
                                        int32_t bNofityCaptureDone __attribute__((unused)) = 0
                                    ){return false;};

    //
    //  Callback of P1done (MTK_CAMERA_MSG_EXT_NOTIFY_P1DONE)
    //
    virtual bool                     onCB_P1done(){return false;};

};


/******************************************************************************
 *
 ******************************************************************************/
enum ECommand
{
    //  This command is to set eng-mode parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to EngParam (i.e. ShotParam const*)
    //      arg2
    //          [I] sizeof(EngParam)
    eCmd_setEngParam,

    //  This command is to set shot-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ShotParam (i.e. ShotParam const*)
    //      arg2
    //          [I] sizeof(ShotParam)
    eCmd_setShotParam,

    //  This command is to set jpeg-related parameters.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to JpegParam (i.e. JpegParam const*)
    //      arg2
    //          [I] sizeof(JpegParam)
    eCmd_setJpegParam,

    //  This command is to set preview buffer handler
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ICamBufHandler
    //      arg2
    //          [I] sizeof(ICamBufHandler)
    eCmd_setPrvBufHdl,

    //  This command is to set image buffer provider manager
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to ImgBufProvidersManager
    //      arg2
    //          [I] sizeof(ImgBufProvidersManager)
    eCmd_setImgBufProvidersMgr,

    //  This command is to set CapBufMgr.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to CapBufMgr
    //      arg2
    //          [I] sizeof(CapBufMgr)
    eCmd_setCapBufMgr,

    //  This command is to set preview metadata.
    //
    //  Arguments:
    //      arg1
    //          [I] Pointer to PreviewMetadata
    //      arg2
    //          [I] sizeof(PreviewMetadata)
    eCmd_setPreviewMetadata,

    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    eCmd_reset,

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    eCmd_capture,

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    eCmd_cancel,

    //  This command is to perform release shot.
    //
    //  Arguments:
    //          N/A
    eCmd_release,
    //  This command is to perform set continuous shot speed.
    //
    //  Arguments:
    //      arg1
    //          [I] fps
    eCmd_setCShotSpeed,

    //  This command is to perform set pipeline mode.
    //
    //  Arguments:
    //      arg2
    //          [I] pipeline mode defined in EPipelineMode
    eCmd_setPipelineMode,
    //  This command is to set dof level
    //
    //  Arguments:
    //      arg2
    //          [I] dof level
    eCmd_setDofLevel,
    //  This command is to set af region for stereo capture
    //
    //  Arguments:
    //      arg1
    //          [I] ptr to af region array
    eCmd_setAFRegion,
    //  The command is to query if the current shot istance,
    //
    //  Arguments:
    //      arg1
    //          [O] an address of MBOOL, for the result
    //      arg2
    //          [I] the transform
    //      arg3
    //          [I] jpeg size, bit[0:15]: width, bit[16:31]: height, w/o rotation
    //
    eCmd_getIsNeedNewOne,
};


/******************************************************************************
 *
 ******************************************************************************/
struct ShotParam
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Picture.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The image format for captured pictures.
    int                             miPictureFormat;

    // The dimensions for captured pictures in pixels (width x height).
    int32_t                         mi4PictureWidth;
    int32_t                         mi4PictureHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Postview Image.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The image format for postview display.
    int                             miPostviewDisplayFormat;

    // The image format for postview client.
    int                             miPostviewClientFormat;

    // The dimensions for postview in pixels (width x height).
    int32_t                         mi4PostviewWidth;
    int32_t                         mi4PostviewHeight;

    // The image format for captured thumbnail.
    int                             miPostviewThumbFmt;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Shot File Name.
    //      This is an auxiliary path/file name to save extra files if needed.
    //      ms8ShotFileName.string() returns a pointer to a C-style
    //      null-terminated string (const char*)
    String8                         ms8ShotFileName;

    // The zoom ratio is in 1/100 increments. Ex: 320 refers to a zoom of 3.2x.
    uint32_t                        mu4ZoomRatio;

    // 1024 refers to 1.0x sensor gain.
    uint32_t                        muSensorGain;

    // AP set ISO. Ex:100/200/400/800/...
    uint32_t                        muRealIso;

    // shutter time in ms
    uint32_t                        muSensorSpeed;

    // sensor mode: sensor uasge scenario
    //              SENSOR_SCENARIO_ID_NORMAL_PREVIEW,
    //              SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
    //              SENSOR_SCENARIO_ID_NORMAL_VIDEO, etc
    uint32_t                        muSensorMode;

    // Shot count in total.
    //      0: request to take no picture.
    //      1: request to take only 1 picture.
    //      N: request to take N pictures.
    uint32_t                        mu4ShotCount;

    // The transform: includes rotation and flip
    // The rotation angle in degrees relative to the orientation of the camera.
    //
    // For example, suppose the natural orientation of the device is portrait.
    // The device is rotated 270 degrees clockwise, so the device orientation is
    // 270. Suppose a back-facing camera sensor is mounted in landscape and the
    // top side of the camera sensor is aligned with the right edge of the
    // display in natural orientation. So the camera orientation is 90. The
    // rotation should be set to 0 (270 + 90).
    //
    // Flip: horizontally/vertically
    // reference value: mtkcam/ImageFormat.h
    uint32_t                        mu4Transform;

    // The multi-frame blending mode.
    ///     This parameter only takes effect in ISmartShot.
    ///     "0": as same as ISingleShot
    ///     "1": capture N (default is 4) frames for noise reduction.
    ///     "2": capture N (default is 4) frames for capture image stabilization.
    uint32_t                        mu4MultiFrameBlending;

    // Stereo Shot File Name.
    //      This is a file name to keep stereo shot info if needed.
    //      ms8StereoFileName.string() returns a pointer to a C-style
    //      null-terminated string (const char*)
    String8                         ms8StereoFileName;

    // metadata converted from parameters defined by android
    NSCam::IMetadata                mAppSetting;

    // metadata converted from parameters defined by mtk
    NSCam::IMetadata                mHalSetting;

    // flag to tell if it's necessary to enable LTM module
    MBOOL                           mbEnableLtm;

    // flag to tell if the shot will capture with flash led on
    MBOOL                           mbFlashOn;

    // flag to tell if the shot had precapture flow or not
    MBOOL                           mbHadPrecapture;

    // paramsMgr for query current latest setting
    wp<IParamsManager>              mpParamsMgr;
    // flag to tell if it's necessary to enable rrzo for jpeg capture flow
    // when flash off and 4cellsensor and isothreshold condition rule
    MBOOL                           mbEnableRrzoCapture;

    // flag to tell if it's manual capture
    MBOOL                           mbEnableManualCapture;

    // currentEV value
    int                             miCurrentEV;

    // need to callback p1done to start preview
    MBOOL                           mbNeedP1DoneCb;

    // need to callback p2done
    MBOOL                           mbNeedP2DoneCb;

    // is using BackgroundService, updated by DefaultAdapter before taking picture.
    MBOOL                           mbIsUsingBackgroundService;

    // indicate MFB which is configured by DefaultAdapter, default is -1 (not specified yet).
    MINT32                          mi4MfbMode;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    ShotParam(
                                        int const               iPictureFormat          = NSCam::eImgFmt_UNKNOWN,
                                        int32_t const           i4PictureWidth          = 2560,
                                        int32_t const           i4PictureHeight         = 1920,
                                        //
                                        int const               iPostviewDisplayFormat  = NSCam::eImgFmt_UNKNOWN,
                                        int const               iPostviewClientFormat   = NSCam::eImgFmt_UNKNOWN,
                                        int32_t const           i4PostviewWidth         = 640,
                                        int32_t const           i4PostviewHeight        = 480,
                                        int const               iPostviewThumbFmt       = NSCam::eImgFmt_UNKNOWN,
                                        //
                                        String8 const&          rs8ShotFileName         = String8::empty(),
                                        uint32_t const          u4ZoomRatio             = 100,
                                        uint32_t const          uSensorGain             = 0,
                                        uint32_t const          uRealIso                = 0,
                                        uint32_t const          uSensorSpeed            = 0,
                                        uint32_t                uSensorMode             = NSCam::SENSOR_SCENARIO_ID_UNNAMED_START,
                                        uint32_t const          u4ShotCount             = 1,
                                        uint32_t const          u4Transform             = 0,
                                        //
                                        uint32_t const          u4MultiFrameBlending    = 0,
                                        //
                                        String8 const&          rs8StereoFileName       = String8::empty(),
                                        //
                                        MBOOL const&            bEnableLtm              = MFALSE,
                                        //
                                        MBOOL const&            bFlashOn                = MFALSE,
                                        MBOOL const&            bHadPrecapture          = MFALSE,
                                        //
                                        wp<IParamsManager>      pParamsMgr              = nullptr,
                                        //
                                        MBOOL const&            bEnableRrzoCapture      = MFALSE,
                                        //
                                        MBOOL const&            bEnableManualCapture    = MFALSE,
                                        //
                                        int const               iCurrentEV              = 0,
                                        //
                                        MBOOL const&            bNeedP1DoneCb          = MFALSE,
                                        MBOOL const&            bNeedP2DoneCb          = MFALSE
                                    )
                                        : miPictureFormat(iPictureFormat)
                                        , mi4PictureWidth(i4PictureWidth)
                                        , mi4PictureHeight(i4PictureHeight)
                                        //
                                        , miPostviewDisplayFormat(iPostviewDisplayFormat)
                                        , miPostviewClientFormat(iPostviewClientFormat)
                                        , mi4PostviewWidth(i4PostviewWidth)
                                        , mi4PostviewHeight(i4PostviewHeight)
                                        , miPostviewThumbFmt(iPostviewThumbFmt)
                                        //
                                        , ms8ShotFileName(rs8ShotFileName)
                                        , mu4ZoomRatio(u4ZoomRatio)
                                        , muSensorGain(uSensorGain)
                                        , muRealIso(uRealIso)
                                        , muSensorSpeed(uSensorSpeed)
                                        , muSensorMode(uSensorMode)
                                        , mu4ShotCount(u4ShotCount)
                                        , mu4Transform(u4Transform)
                                        //
                                        , mu4MultiFrameBlending(u4MultiFrameBlending)
                                        //
                                        , ms8StereoFileName(rs8StereoFileName)
                                        //
                                        , mbEnableLtm(bEnableLtm)
                                        //
                                        , mbFlashOn(bFlashOn)
                                        , mbHadPrecapture(bHadPrecapture)
                                        //
                                        , mpParamsMgr(pParamsMgr)
                                        //
                                        , mbEnableRrzoCapture(bEnableRrzoCapture)
                                        //
                                        , mbEnableManualCapture(bEnableManualCapture)
                                        //
                                        , miCurrentEV(iCurrentEV)
                                        //
                                        , mbNeedP1DoneCb(bNeedP1DoneCb)
                                        , mbNeedP2DoneCb(bNeedP2DoneCb)
                                        //
                                        , mbIsUsingBackgroundService(MFALSE)
                                        //
                                        , mi4MfbMode(-1)
                                    {
                                    }
};


/******************************************************************************
 *
 ******************************************************************************/
struct JpegParam
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Jpeg.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Jpeg quality of captured picture. The range is 1 to 100, with 100 being
    // the best.
    uint32_t                        mu4JpegQuality;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Jpeg Thumb.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The quality of the EXIF thumbnail in Jpeg picture. The range is 1 to 100,
    // with 100 being the best.
    uint32_t                        mu4JpegThumbQuality;

    // The width (in pixels) of EXIF thumbnail in Jpeg picture.
    int32_t                         mi4JpegThumbWidth;

    // The height (in pixels) of EXIF thumbnail in Jpeg picture.
    int32_t                         mi4JpegThumbHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  GPS.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // GPS latitude coordinate.
    // Example value: "25.032146" or "-33.462809".
    String8                         ms8GpsLatitude;

    // GPS longitude coordinate.
    // Example value: "121.564448" or "-70.660286".
    String8                         ms8GpsLongitude;

    // GPS altitude.
    // header.
    // Example value: "21.0" or "-5".
    String8                         ms8GpsAltitude;

    // GPS timestamp (UTC in seconds since January 1, 1970).
    // Example value: "1251192757".
    String8                         ms8GpsTimestamp;

    // GPS Processing Method
    // Example value: "GPS" or "NETWORK".
    String8                         ms8GpsMethod;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Stereo JPS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The width (in pixels) of EXIF thumbnail in Jps picture.
    int32_t                         mi4JpsWidth;

    // The height (in pixels) of EXIF thumbnail in Jps picture.
    int32_t                         mi4JpsHeight;

    // (Non-Jpeg miss) save path
    String8                         ms8FilePath;

    // (Non-Jpeg miss) save name
    String8                         ms8FileName;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    JpegParam(
                                        uint32_t const          u4JpegQuality       = 100,
                                        uint32_t const          u4JpegThumbQuality  = 100,
                                        int32_t const           i4JpegThumbWidth    = 160,
                                        int32_t const           i4JpegThumbHeight   = 120,
                                        String8 const&          s8GpsLatitude       = String8::empty(),
                                        String8 const&          s8GpsLongitude      = String8::empty(),
                                        String8 const&          s8GpsAltitude       = String8::empty(),
                                        String8 const&          s8GpsTimestamp      = String8::empty(),
                                        String8 const&          s8GpsMethod         = String8::empty(),
                                        int32_t const           i4JpsWidth          = 3840,
                                        int32_t const           i4JpsHeight         = 1080,
                                        String8 const&          s8FilePath          = String8::empty(),
                                        String8 const&          s8FileName          = String8::empty()
                                    )
                                        : mu4JpegQuality(u4JpegQuality)
                                        //
                                        , mu4JpegThumbQuality(u4JpegThumbQuality)
                                        , mi4JpegThumbWidth(i4JpegThumbWidth)
                                        , mi4JpegThumbHeight(i4JpegThumbHeight)
                                        //
                                        , ms8GpsLatitude(s8GpsLatitude)
                                        , ms8GpsLongitude(s8GpsLongitude)
                                        , ms8GpsAltitude(s8GpsAltitude)
                                        , ms8GpsTimestamp(s8GpsTimestamp)
                                        , ms8GpsMethod(s8GpsMethod)
                                        //
                                        , mi4JpsWidth(i4JpsWidth)
                                        , mi4JpsHeight(i4JpsHeight)
                                        //
                                        , ms8FilePath(s8FilePath)
                                        , ms8FileName(s8FileName)
                                    {
                                    }
};


/******************************************************************************
 *
 ******************************************************************************/
struct PreviewMetadata
{
    // preview metadata converted from parameters defined by android
    NSCam::IMetadata                mPreviewAppSetting;

    // preview metadata converted from parameters defined by mtk
    NSCam::IMetadata                mPreviewHalSetting;
};

/******************************************************************************
 *
 ******************************************************************************/
struct NRParam
{
    MINT32            mi4NRmode;
    MINT64            mPluginUser;
    MBOOL             mbisMfll;
    NSCam::IMetadata* mpHalMeta;
    NSCam::MSize      mFullRawSize;
    //
    NRParam()
        : mi4NRmode(-1)
        , mPluginUser(-1)
        , mbisMfll(MFALSE)
        , mpHalMeta(nullptr)
    {}
};

/******************************************************************************
 *  Interface of Shot Class.
 *******************************************************************************/
class IShot : public RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual char const*             getShotName() const;
    virtual uint32_t                getShotMode() const;
    virtual NSCam::EPipelineMode    getPipelineMode() const;
    virtual int32_t                 getOpenId() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
                                    IShot(sp<ImpShot> pImpShot);
    virtual                         ~IShot();

    virtual bool                    setCallback(sp<IShotCallback> pShotCallback);

public:     ////                    Operations.
    virtual bool                    sendCommand(
                                        ECommand const  cmd,
                                        MUINTPTR const  arg1 = 0,
                                        uint32_t const  arg2 = 0,
                                        uint32_t const  arg3 = 0
                                    );

    virtual bool   isDataMsgEnabled(MINT32 const i4MsgTypes);

    virtual void   enableDataMsg(MINT32 const i4MsgTypes);

    virtual void   disableDataMsg(MINT32 const i4MsgTypes);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    sp<ImpShot>                     mpImpShot;

};


struct StereoShotParam
{
public:
    MBOOL mbDngMode = MFALSE;
    //
    MINT32 miDOFLevel = -1;
    //
    ShotParam mShotParam;
    //
    JpegParam mJpegParam;

    bool operator==(const StereoShotParam& rhs) const {
      bool equalily = true;
      equalily = equalily && (this->mbDngMode == rhs.mbDngMode);
      equalily = equalily && (this->miDOFLevel == rhs.miDOFLevel);
      equalily = equalily && (this->mShotParam.mi4PictureWidth == rhs.mShotParam.mi4PictureWidth);
      equalily = equalily && (this->mShotParam.mi4PictureHeight == rhs.mShotParam.mi4PictureHeight);
      equalily = equalily && (this->mShotParam.mu4Transform == rhs.mShotParam.mu4Transform);
      equalily = equalily && (this->mJpegParam.mi4JpegThumbWidth == rhs.mJpegParam.mi4JpegThumbWidth);
      equalily = equalily && (this->mJpegParam.mi4JpegThumbHeight == rhs.mJpegParam.mi4JpegThumbHeight);
      return equalily;
    }
};


/******************************************************************************
 *
 ******************************************************************************/
}; // namespace NSShot
}; // namespace android
#endif  //_MTK_CAMERA_CAMADAPTER_INC_SCENARIO_SHOT_ISHOT_H_

