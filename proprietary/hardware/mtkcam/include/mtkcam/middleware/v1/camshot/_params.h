/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MTK_CAMERA_INC_CAMSHOT_PARAMS_H_
#define _MTK_CAMERA_INC_CAMSHOT_PARAMS_H_


#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>

using namespace NSCam;
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

/**
 * @enum ECamShotNotifyMsg
 * @brief The CamShot notify message type enum
 *
 */
 enum ECamShotNotifyMsg
{
    ECamShot_NOTIFY_MSG_NONE          = 0x0000,       /*!< none notify message */
    ECamShot_NOTIFY_MSG_SOF           = 0x0001,       /*!< start of frame notify message */
    ECamShot_NOTIFY_MSG_EOF           = 0x0002,       /*!< end of frame notify  message */
    ECamShot_NOTIFY_MSG_SHOTS_END     = 0x0004,       /*!< end of shots notify  message */
    ECamShot_NOTIFY_MSG_P2DONE        = 0x0008,       /*!< end of pass2 notify  message */
    ECamShot_NOTIFY_MSG_ERROR         = 0x1000,       /*!< error notify message */
    ECamShot_NOTIFY_MSG_P1DONE        = 0x2000,       /*!< error notify message */
    ECamShot_NOTIFY_MSG_ALL           = 0x300F,       /*!< all message notify  */
};

/**
 * @enum ECamShotDataMsg
 * @brief The CamShot notify message type enum
 *
 */
enum ECamShotDataMsg
{
    ECamShot_DATA_MSG_NONE           = 0x0000,           /*!< none data  callback message */
    ECamShot_DATA_MSG_RAW            = 0x0001,           /*!< raw data callback message */
    ECamShot_DATA_MSG_YUV            = 0x0002,           /*!< yuv data callback message */
    ECamShot_DATA_MSG_POSTVIEW       = 0x0004,           /*!< postview data callback message */
    ECamShot_DATA_MSG_JPEG           = 0x0008,           /*!< jpeg data callback message */
    ECamShot_DATA_MSG_EIS            = 0x0010,           /*!< eis data callback message (ext1) */
    ECamShot_DATA_MSG_JPS            = 0x0020,           /*!< jps data callback message */
    ECamShot_DATA_MSG_DEPTHMAP       = 0x0030,           /*!< depthmap data callback message */
    ECamShot_DATA_MSG_CLEAN_MAIN     = 0x0050,           /*!< clean main data callback message */
    ECamShot_DATA_MSG_APPMETA        = 0x0040,           /*!< meta data callback message */
    ECamShot_DATA_MSG_HALMETA        = 0x0080,           /*!< meta data callback message */
    ECamShot_DATA_MSG_DNG            = 0x0100,           /*!< raw data callback message */
    ECamShot_DATA_MSG_ALL            = 0x01FF,           /*!< all data callback message */
};

/**
 * @enum ECamShotImgBufType
 * @brief The CamShot image buffer type enum
 *
 */
enum ECamShotImgBufType {
    ECamShot_BUF_TYPE_NONE           = 0x0000,        /*!< buffer type for nothing  */
    ECamShot_BUF_TYPE_RAW            = 0x0001,        /*!< buffer type for raw data */
    ECamShot_BUF_TYPE_YUV            = 0x0002,        /*!< buffer type for yuv data */
    ECamShot_BUF_TYPE_POSTVIEW       = 0x0004,        /*!< buffer type for postview data */
    ECamShot_BUF_TYPE_JPEG           = 0x0008,        /*!< buffer type for jpeg data */
    ECamShot_BUF_TYPE_THUMBNAIL      = 0x0010,        /*!< buffer type for thumnail data */
    ECamShot_BUF_TYPE_ALL            = 0x001F,        /*!< buffer type all */
};

/**
 * @enum ECamShotCmd
 * @brief The CamShot command type enum
 *
 */
enum ECamShotCmd
{
    ECamShot_CMD_SET_CSHOT_SPEED     = 0x0000,              /*!< set continuous shot speed */
    ECamShot_CMD_SET_CAPTURE_STYLE   = 0x0004,              /*!< set capture style */
    ECamShot_CMD_SET_NRTYPE          = 0x0008,              /*!< set NR Type */
    ECamShot_CMD_SET_VHDR_PROFILE    = 0x0010,              /*!< set vhdr mode for cct */
    ECamShot_CMD_SET_RWB_PROC_TYPE   = 0x0020,              /*!< set GPU process enable for rwb sensor */
    ECamShot_CMD_SET_PRV_META        = 0x0040,              /*!< set preview metadata for normal CShot */
    ECamShot_CMD_ALL                 = 0x007F,              /*!< all command */
};

/**
 * @enum ECamShotCmdStyle
 * @brief The value of ECamShot_CMD_SET_CAPTURE_STYLE
 *
 */
enum ECamShotCmdStyle
{
    ECamShot_CMD_STYLE_NORMAL    = 0x0000,            /*!< normal path */
    ECamShot_CMD_STYLE_VSS       = 0x0001,            /*!< vss path */
};

/**
 * @enum ECamShotCmdStyle
 * @brief The value of ECamShot_CMD_SET_CAPTURE_STYLE
 *
 */
enum ECamNRType
{
    ECamShot_NRTYPE_NONE         = 0x0000,            /*!< no additional NR */
    ECamShot_NRTYPE_HWNR         = 0x0001,            /*!< additional HW NR */
    ECamShot_NRTYPE_SWNR         = 0x0002,            /*!< additional SW NR */
};

/**
 * @enum ECamShotCmdStyle
 * @brief The value of ECamShot_CMD_SET_CAPTURE_STYLE
 *
 */
enum ECamRWBProcType
{
    ECamShot_RWB_NONE         = 0x0000,
    ECamShot_RWB_MDP          = 0x0001,            /*!< MDP direct link process for rwb sensor  */
    ECamShot_RWB_GPU          = 0x0002,            /*!< gpu process for rwb sensor */
};

/**
 * @enum ECamRawType
 * @brief The value of Raw Type
 *
 */
enum ECamRawType
{
    ECamShot_RAWTYPE_PURE           = 0x0000,   /*!< pure raw */
    ECamShot_RAWTYPE_PREPROC        = 0x0001,   /*!< preprocess raw */
};

/**
 * @struct SensorParam
 * @brief This structure is for the sensor parameter
 *
 */
struct SensorParam
{
public:    //// fields.
    /**
      * @var u4OpenID
      * The open ID of the sensor
      */
    MUINT32  u4OpenID;
    /**
      * @var u4Scenario
      * The sensor scenario that get from sensorHal
      */
    MUINT32  u4Scenario;
    /**
      * @var u4Bitdepth
      * The bit depth of the raw data. 8, 10 , 12
      */
    MUINT32  u4Bitdepth;
    /**
      * @var fgBypassDelay
      * Bypass the sensor delay, used for skip frame
      */
    MBOOL    fgBypassDelay;       // by pass the delay frame count
    /**
      * @var fgBypassScenaio
      * Bypass to set the sensor scenario, this can save the sensor init time
      */
    MBOOL    fgBypassScenaio;     // by pass to set the sensor scenaior
    /**
      * @var u4RawType
      * @brief The value of ECamRawType
      */
    ECamRawType u4RawType;
    /**
      * @var u4VHDRMode
      * The sensor VHDR mode
      * SENSOR_VHDR_MODE_NONE  = 0x0,
      * SENSOR_VHDR_MODE_IVHDR = 0x01,
      * SENSOR_VHDR_MODE_MVHDR = 0x02,
      * SENSOR_VHDR_MODE_ZVHDR = 0x09
      */
    MUINT32 u4VHDRMode;

public:    //// constructors.
    SensorParam(
        MUINT32     const _u4OpenID        = 0,
        MUINT32     const _u4Scenario      = 0,
        MUINT32     const _u4Bitdepth      = 0,
        MBOOL       const _fgBypassDelay   = 0,
        MBOOL       const _fgBypassScenaio = 0,
        ECamRawType const _u4RawType       = ECamShot_RAWTYPE_PURE,
        MUINT32     const _u4VHDRMode      = 0
    )
        : u4OpenID        ( _u4OpenID        )
        , u4Scenario      ( _u4Scenario      )
        , u4Bitdepth      ( _u4Bitdepth      )
        , fgBypassDelay   ( _fgBypassDelay   )
        , fgBypassScenaio ( _fgBypassScenaio )
        , u4RawType       ( _u4RawType       )
        , u4VHDRMode      ( _u4VHDRMode      )
    {
    }
};


/**
 * @struct ShotParam
 * @brief This structure is for the shot parameter
 *
 */
struct ShotParam
{
public:    //// extension parameter structures
    struct EXTPARAM_BGSERV
    {
        int64_t     instanceId;
        int32_t     vendorManagerId;
        void*       impShot;
        EXTPARAM_BGSERV()
            : instanceId(0)
            , vendorManagerId(0)
            , impShot(nullptr)
        {}
    };

public:    //// fields.
    /**
      * @var ePictureFmt
      * The picture the image format
      */
    EImageFormat      ePictureFmt;
    /**
      * @var u4PictureWidth
      * The width of the picture image
      */
    MUINT32           u4PictureWidth;
    /**
      * @var u4PictureHeight
      * The height of the picture image
      */
    MUINT32           u4PictureHeight;
    /**
      * @var u4PictureTransform
      * The rotation/flip operation of the picture image
      */
    MUINT32           u4PictureTransform;
    /**
      * @var ePostViewFmt
      * The image format of the postview image
      */
    EImageFormat      ePostViewFmt;
    /**
      * @var u4PostViewWidth
      * The width of the postview image
      */
    MUINT32           u4PostViewWidth;
    /**
      * @var u4PostViewHeight
      * The height of the postview image
      */
    MUINT32           u4PostViewHeight;
    /**
      * @var u4PostViewTransform
      * The rotation/flip operation of the postview image
      */
    MUINT32           u4PostViewTransform;
    /**
      * @var u4ZoomRatio
      * The zoom ratio of the shot operation, the value is x100
      */
    MUINT32           u4ZoomRatio;

    /**
     * @var appSetting
     * Metadata converted from parameters defined by android
     */
    IMetadata         appSetting;

    /**
     * @var halSetting
     * Metadata converted from parameters defined by android
     */
    IMetadata         halSetting;

    /**
      * @var bOpenLCS
      * P1 Need to generate LCS statistic data to enqueue LCE.
      */
    MBOOL             bOpenLCS;

    /**
      * @var bEnable4cellsensorRRZOcaptureflow
      * change from IMGO to RRZO port for 4cellsensor/flashoff/800isothreshold captureflow.
      */
    MBOOL             bEnableRrzoCapture;

   /**
      * @var bNeedP1DoneCb
      * need to callback p1done to start preview
      */
    MBOOL             bNeedP1DoneCb;

    /**
       * @var bNeedP2DoneCb
       * need to calback p2done
       */
    MBOOL             bNeedP2DoneCb;

    /**
       * @var pExtParamBGserv
       * the extended field for BackgroundService
       */
    EXTPARAM_BGSERV   extParamBGserv;


public:    //// constructors.
    ShotParam(
        EImageFormat const _ePictureFmt   = eImgFmt_YUY2,
        MUINT32 const _u4PictureWidth     = 0,
        MUINT32 const _u4PictureHeight    = 0,
        MUINT32 const _u4PictureTransform = 0,
        EImageFormat const _ePostViewFmt  = eImgFmt_YUY2,
        MUINT32 const _u4PostViewWidth    = 0,
        MUINT32 const _u4PostViewHeight   = 0,
        MUINT32 const _u4PostViewTransform= 0,
        MUINT32 const _u4ZoomRatio        = 100,
        IMetadata const _appSetting       = IMetadata(),
        IMetadata const _halSetting       = IMetadata(),
        MBOOL const _bOpenLCS             = MFALSE,
        MBOOL const _bEnableRrzoCapture   = MFALSE,
        MBOOL const _bNeedP1DoneCb        = MFALSE,
        MBOOL const _bNeedP2DoneCb        = MFALSE
    )
        : ePictureFmt        ( _ePictureFmt        )
        , u4PictureWidth     ( _u4PictureWidth     )
        , u4PictureHeight    ( _u4PictureHeight    )
        , u4PictureTransform ( _u4PictureTransform )
        , ePostViewFmt       ( _ePostViewFmt       )
        , u4PostViewWidth    ( _u4PostViewWidth    )
        , u4PostViewHeight   ( _u4PostViewHeight   )
        , u4PostViewTransform( _u4PostViewTransform)
        , u4ZoomRatio        ( _u4ZoomRatio        )
        , appSetting         ( _appSetting         )
        , halSetting         ( _halSetting         )
        , bOpenLCS           ( _bOpenLCS           )
        , bEnableRrzoCapture ( _bEnableRrzoCapture )
        , bNeedP1DoneCb      ( _bNeedP1DoneCb      )
        , bNeedP2DoneCb      ( _bNeedP2DoneCb      )
    {
    }
};


/**
 * @struct PreviewMetadata
 * @brief This structure is for the preview metadata
 *
 */
struct PreviewMetadata
{
    // preview metadata converted from parameters defined by android
    NSCam::IMetadata                mPreviewAppSetting;

    // preview metadata converted from parameters defined by mtk
    NSCam::IMetadata                mPreviewHalSetting;
};


/**
 * @struct JpsParam
 * @brief This structure is for the jps parameter
 *
 */
struct JpsParam
{
public: //// fields.
    /**
      * @var u4JpsWidth
      * The width of the jps image
      */
    MUINT32 u4JpsWidth;
    /**
      * @var u4JpsHeight
      * The height of the jps image
      */
    MUINT32 u4JpsHeight;

public:
    JpsParam (
        MUINT32 const _u4JpsWidth = 0,
        MUINT32 const _u4JpsHeight = 0
    )
        : u4JpsWidth(_u4JpsWidth)
        , u4JpsHeight(_u4JpsHeight)
    {
    }
};


/**
 * @struct ThumbnailParam
 * @brief This structure is for the thumbnail parameter
 *
 */
struct ThumbnailParam
{
public: //// fields.
    /**
      * @var u4ThumbWidth
      * The width of the thumbnail image
      */
    MUINT32 u4ThumbWidth;
    /**
      * @var u4ThumbHeight
      * The height of the thumbnail image
      */
    MUINT32 u4ThumbHeight;
    /**
      * @var u4ThumbQuality
      * The jpeg quality factor of the thumbnail image
      */
    MUINT32 u4ThumbQuality;
    /**
      * @var fgThumbIsSOI
      * If enable to add start of image header in thumbnail image
      */
    MBOOL   fgThumbIsSOI;

public:
    ThumbnailParam (
        MUINT32 const _u4ThumbWidth = 0,
        MUINT32 const _u4ThumbHeight = 0,
        MUINT32 const _u4ThumbQuality = 100,
        MBOOL   const _fgThumbIsSOI = MTRUE
    )
        : u4ThumbWidth(_u4ThumbWidth)
        , u4ThumbHeight(_u4ThumbHeight)
        , u4ThumbQuality(_u4ThumbQuality)
        , fgThumbIsSOI(_fgThumbIsSOI)
    {
    }
};

/**
 * @struct JpegParam
 * @brief This structure is for the jpeg parameter
 *
 */
struct JpegParam : public ThumbnailParam, public JpsParam
{
public:    //// fields.
    /**
      * @var u4Quality
      * The jpeg quality factor of the jpeg image
      */
    MUINT32 u4Quality;
    /**
      * @var fgIsSOI
      * If enable to add start of image header in jpeg image
      */
    MBOOL   fgIsSOI;

public:    //// constructors.
    JpegParam(
        MUINT32 const _u4Quality = 100,
        MBOOL   const _fgIsSOI = MFALSE
    )
        : ThumbnailParam()
        , u4Quality(_u4Quality)
        , fgIsSOI(_fgIsSOI)
    {
    }
    //
    JpegParam(
        ThumbnailParam _rParam,
        MUINT32 const _u4Quality = 100,
        MBOOL   const _fgIsSOI = MFALSE
    )
        : ThumbnailParam(_rParam.u4ThumbWidth,_rParam.u4ThumbHeight, _rParam.u4ThumbQuality, _rParam.fgThumbIsSOI)
        , u4Quality(_u4Quality)
        , fgIsSOI(_fgIsSOI)
    {
    }
    //
    JpegParam(
        ThumbnailParam _rParam,
        JpsParam _rJpsParam,
        MUINT32 const _u4Quality = 100,
        MBOOL   const _fgIsSOI = MFALSE
    )
        : ThumbnailParam(_rParam.u4ThumbWidth,_rParam.u4ThumbHeight, _rParam.u4ThumbQuality, _rParam.fgThumbIsSOI)
        , JpsParam(_rJpsParam.u4JpsWidth, _rJpsParam.u4JpsHeight)
        , u4Quality(_u4Quality)
        , fgIsSOI(_fgIsSOI)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot
#endif  //  _MTK_CAMERA_INC_CAMSHOT_PARAMS_H_

