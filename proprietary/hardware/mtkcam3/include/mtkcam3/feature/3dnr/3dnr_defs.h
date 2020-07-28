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

/**
* @file 3dnr_defs.h
*
* 3DNR Defs Header File
*/


#ifndef _3DNR_DEFS_H_
#define _3DNR_DEFS_H_

#include <mtkcam/def/BuiltinTypes.h>
#include <mtkcam/def/common.h>

namespace NSCam {
    class IImageBuffer;
};
// === FEATURE COMBINE MASK ===

//!!NOTES:
// if HAL_3DNR_INST_MAX_NUM is changed, the following functions need modification
// Hal3dnr::getInstance/destroyInstance
#define HAL_3DNR_INST_MAX_NUM 6

// !!NOTES: #3DNR_USERID has to be the same as #3DNR_FEAT_MASK_OFFSET
typedef enum ENUM_3DNR_USERID
{
    USERID_3DNR_P2CAMCONTEXT = 0,
    USERID_3DNR_P2SW,
    USERID_3DNR_VSDOF,
    USERID_3DNR_W_T,
    USERID_3DNR_TRICAM,
    USERID_LAST,
} ENUM_3DNR_USERID;

typedef enum ENUM_3DNR_FEAT_MASK_OFFSET
{
    OFFSET_3DNR_FEAT_MASK_BASIC               = 0,
    OFFSET_3DNR_FEAT_MASK_BASIC_DSDN20        = 1,
    OFFSET_3DNR_FEAT_MASK_BASIC_DSDN25        = 2,

    OFFSET_3DNR_FEAT_MASK_SMVRB               = 3,
    OFFSET_3DNR_FEAT_MASK_SMVRB_DSDN20        = 4,
    OFFSET_3DNR_FEAT_MASK_SMVRB_DSDN25        = 5,

    OFFSET_3DNR_FEAT_MASK_VSDOF               = 7,
    OFFSET_3DNR_FEAT_MASK_W_T                 = 8,
    OFFSET_3DNR_FEAT_MASK_TRICAM              = 9,
} ENUM_3DNR_FEAT_MASK_OFFSET;


#ifdef NR3D_FEAT_MASK
#error NR3D_FEAT_MASK macro redefine
#endif

#define NR3D_FEAT_MASK(name) (1 << OFFSET_3DNR_FEAT_MASK_##name)


#define MAKE_NR3D_FEAT_MASK_FUNC(name, tag)             \
    const MUINT32 NR3D_FEAT_MASK_##name = (1 << OFFSET_3DNR_FEAT_MASK_##name); \
    inline MBOOL HAS_3DNR_##name(MUINT32 feature)          \
    {                                                 \
        return !!(feature & NR3D_FEAT_MASK(name));      \
    }                                                 \
    inline MVOID ENABLE_3DNR_##name(MUINT32 &feature)      \
    {                                                 \
        feature |= NR3D_FEAT_MASK(name);                \
    }                                                 \
    inline MVOID DISABLE_3DNR_##name(MUINT32 &feature)     \
    {                                                 \
        feature &= ~NR3D_FEAT_MASK(name);               \
    }                                                 \
    inline const char* TAG_##name()                   \
    {                                                 \
        return tag;                                   \
    }

namespace NSCam{
namespace NR3D{

/*********************************************************************
* Define Value
*********************************************************************/

static const char *NR3D_PROP_3DNR_ENABLE = "vendor.debug.camera.3dnr.enable";
static const char *NR3D_PROP_DEBUG_ENABLE = "vendor.debug.camera.3dnr.debug.enable";
static const char *NR3D_PROP_LOG_LEVEL = "vendor.debug.camera.3dnr.log.level";
static const char *NR3D_PROP_DEBUG_RESET_GMV = "vendor.debug.camera.3dnr.reset_gmv";
static const char *NR3D_PROP_FIX_LATENCY = "vendor.debug.camera.3dnr.fix_latency";
static const char *NR3D_PROP_3DNR_LIB_LOG = "vendor.debug.3dnr.lib.log";
static const char *NR3D_PROP_ZOOM = "vendor.debug.3dnr.zoom";
static const char *NR3D_PROP_FORCEISOLIMIT = "vendor.debug.camera.3dnr.forceisolimit";
static const char *NR3D_PROP_FORCEGMV_X = "vendor.debug.camera.3dnr.forcegmv.x";
static const char *NR3D_PROP_FORCEGMV_Y = "vendor.debug.camera.3dnr.forcegmv.y";
static const char *NR3D_PROP_INPUTCHG_ON = "vendor.debug.camera.3dnr.inputchg.on";
static const char *NR3D_PROP_3DNR_P2A_ENABLE = "vendor.debug.camera.3dnr.p2a.enable";
static const char *NR3D_PROP_3DNR_DSDN_ENABLE = "vendor.debug.camera.3dnr.dsdn.enable";
static const char *NR3D_PROP_SL2E_ENABLE = "vendor.debug.3dnr.sl2e.enable";
static const char *NR3D_PROP_ISO_THRESHOLD = "vendor.debug.3dnr.iso.threshold";

#define NR3D_LMV_MAX_GMV_DEFAULT 32

MAKE_NR3D_FEAT_MASK_FUNC(BASIC, "BASIC");
MAKE_NR3D_FEAT_MASK_FUNC(BASIC_DSDN20, "BASIC_DSDN20");
MAKE_NR3D_FEAT_MASK_FUNC(BASIC_DSDN25, "BASIC_DSDN25");
MAKE_NR3D_FEAT_MASK_FUNC(SMVRB, "SMVRB");
MAKE_NR3D_FEAT_MASK_FUNC(SMVRB_DSDN20, "SMVRB_DSDN20");
MAKE_NR3D_FEAT_MASK_FUNC(SMVRB_DSDN25, "SMVRB_DSDN25");
MAKE_NR3D_FEAT_MASK_FUNC(VSDOF,  "VSDOF");
MAKE_NR3D_FEAT_MASK_FUNC(W_T,    "W_T");
MAKE_NR3D_FEAT_MASK_FUNC(TRICAM, "TRICAM");

/*********************************************************************
* ENUM
*********************************************************************/

/**
*@brief Return enum of 3DNR Modr for P2 flow.
*/
typedef enum E3DNR_MODE_MASK
{
    E3DNR_MODE_MASK_HAL_FORCE_SUPPORT     = 1 << 0,  // hal force support 3DNR
    E3DNR_MODE_MASK_UI_SUPPORT            = 1 << 1,  // feature option on/off
    E3DNR_MODE_MASK_RSC_EN                = 1 << 5,  // enable RSC support for 3DNR
    E3DNR_MODE_MASK_SL2E_EN               = 1 << 6,  // enable SL2E
    E3DNR_MODE_MASK_DSDN_EN               = 1 << 7,  // enable DSDN
    E3DNR_MODE_MASK_SMVR_EN               = 1 << 8,  // enable slow motion
} E3DNR_MODE_MASK;

typedef enum
{
    NR3D_STATE_STOP         = 0x00,     // NR3D, IMG3O, VIPI all disabled.
    NR3D_STATE_PREPARING    = 0x01,     // IMG3O enabled, to output current frame for next frame use. NR3D, VIPI disabled. (When NR3D is disable, IMG30 output original image without any process.)
    NR3D_STATE_WORKING      = 0x02,     // NR3D, IMG3O, VIPI all enabled.
} NR3D_STATE_ENUM;

#define E3DNR_MODE_MASK_ENABLED(x, mask)   ((x) & (mask))

typedef enum
{
    NR3D_ERROR_NONE                = 0,     // No error.
    NR3D_ERROR_INVALID_GMV         = 1,     // GMV is invalid due to drop frame or something.
    NR3D_ERROR_GMV_TOO_LARGE       = 2,     // GMV X or Y is larger than a certain value.
    NR3D_ERROR_DROP_FRAME          = 3,     // Drop frame.
    NR3D_ERROR_FRAME_SIZE_CHANGED  = 4,     // Current frame size is not the same as previous frame. E.g. during DZ.
    NR3D_ERROR_FORCE_SKIP          = 5,     // Force skip by user, probably by adb command.
    NR3D_ERROR_UNDER_ISO_THRESHOLD = 6,     // must > iso threshold to turn on 3dnrs
    NR3D_ERROR_NOT_SUPPORT         = 7,     // not supported function
    NR3D_ERROR_NO_VIPI_INFO        = 8,     // no vipi info
    NR3D_ERROR_INPUT_SRC_CHANGE    = 9,     //  input src change
} NR3D_ERROR_ENUM;

/*********************************************************************
* Struct
*********************************************************************/

struct GyroData
{
public: // fucntion
    void print(int needPrint);

public: // member
    MBOOL isValid = MFALSE;
    MFLOAT x = 0.0f;
    MFLOAT y = 0.0f;
    MFLOAT z = 0.0f;

};


/******************************************************************************
 *
 * @struct NR3DMVInfo
 * @brief parameter for set nr3d
 * @details
 *
 ******************************************************************************/

struct NR3DMVInfo
{
public: // function
    void print(int needPrint);

public: // member
    enum Status {
        INVALID = 0,
        VALID
    };
    // 3dnr vipi: needs x_int/y_int/gmvX/gmvY
    // ISP smoothNR3D: needs gmvX/gmvY/confX/confY/maxGMV
    MINT32 status = INVALID;

    MUINT32 x_int = 0;
    MUINT32 y_int = 0;

    MINT32 gmvX = 0; // pixel = gmvX / 256
    MINT32 gmvY = 0; // pixel = gmvX / 256
    MFLOAT gmvX_f_in_pixel = 0.0;
    MFLOAT gmvY_f_in_pixel = 0.0;
    MINT32 gmvX_ds = 0; // pixel = gmvX / 256
    MINT32 gmvY_ds = 0; // pixel = gmvX / 256
    MINT32 confX = 0;
    MINT32 confY = 0;
    MINT32 maxGMV = NR3D_LMV_MAX_GMV_DEFAULT;//before 3DNR3.5
    MINT32 maxGMVX = 0;//after 3DNR3.5, ex: rsc(256, 64), lmv(32, 32)
    MINT32 maxGMVY = 0;//after 3DNR3.5, ex: rsc(256, 64), lmv(32, 32)

    MINT64 ts = 0;//for 3dnr_lib

};

/******************************************************************************
 *
 * @struct NR3DParam
 * @brief parameter for NR3D HW setting
* @details
 *
 ******************************************************************************/
struct NR3DHwParam
{
public:
    MUINT32 ctrl_onEn = 0;
    MUINT32 dramBitsPerPixel = 8;
    MUINT32 onOff_onOfStX = 0;
    MUINT32 onOff_onOfStY = 0;
    MUINT32 onSiz_onWd = 0;
    MUINT32 onSiz_onHt = 0;
    MUINT32 vipi_offst = 0;     //in byte
    MUINT32 vipi_readW = 0;     //in pixel
    MUINT32 vipi_readH = 0;     //in pixel
    MUINT32 vipi_ofst_x_in_pixel = 0;
    MUINT32 vipi_ofst_y_in_pixel = 0;
};

struct NR3DTuningInfo
{
    MBOOL canEnable3dnrOnFrame = MFALSE;
    MINT32 isoThreshold = 0;
    NR3DMVInfo mvInfo;
    MSize inputSize; // of NR3D hardware
    MRect inputCrop;
    GyroData gyroData;
    NR3DHwParam nr3dHwParam;
};

struct NR3DRSCInfo
{
public: // functions
    void print(int needPrint);
public: // members
    MBOOL   isValid = MFALSE;
    MINTPTR pMV = 0;
    MINTPTR pBV = 0; // size is rssoSize
    MSize   rrzoSize;
    MSize   rssoSize;
    MUINT32 staGMV = 0; // gmv value of RSC
};

typedef struct Hal3dnrSavedFrameInfo
{
    MBOOL               isValid = MFALSE;
    MUINT32             CmvX = 0;             // Keep track of CMV X.
    MUINT32             CmvY = 0;             // Keep track of CMV Y.

    MBOOL               isCRZUsed = MFALSE;
    MBOOL               isIMGO = MFALSE;
    MBOOL               isBinning = MFALSE;
} Hal3dnrSavedFrameInfo;


#define NR3D_LAYER_MAX_NUM 8 // 3dnrLib supports max=8 layers

struct NR3DHALParam
{
public:
    // 3a tuning buffer
    void *pTuningData = NULL;
    void *p3A = NULL; // void * -> NS3Av3::IHal3A*

    MUINT32 featMask =  NR3D_FEAT_MASK_BASIC;

    // frame generic
    MINT32 uniqueKey = 0;  // tuning hint
    MINT32 requestNo = 0;  // tuningHint: ref pipeline requestNo
    MINT32 frameNo = 0;   // tuningHint: ref pipeline frameNo

    MBOOL  needChkIso = MTRUE;
    MINT32 iso = 0;
    MINT32 isoThreshold = 0;

    // imgi related
    MBOOL isCRZUsed = MFALSE;
    MBOOL isIMGO = MFALSE;
    MBOOL isBinning = MFALSE; // TODO: future use

    // for 3dnr later run
    MRect p2aSrcCrop;
    MRect srcCrop;

    // mv related info
    NR3DRSCInfo rscInfo;
    mutable NR3DMVInfo  gmvInfo;

    // vipi related
//    IImageBuffer *pVipiImgBuf = NULL;
    struct VipiInfo
    {
        MBOOL isValid = MFALSE;
        int32_t imgFmt = 0;
        int32_t bufStridesInBytes_p0 = 0;
        MSize imgSize;

        VipiInfo() { }
        VipiInfo(int32_t valid, int32_t fmt, int32_t stridesInBytes_p0, MSize size)
        {
            isValid = valid;
            imgFmt = fmt;
            bufStridesInBytes_p0 = stridesInBytes_p0;
            imgSize = size;
        }
    } vipiInfo;

   // output related, ex: img3o / ufbco
//    MRect dst_resizer_rect;
    MRect dstRect;

    NR3D::GyroData gyroData;

    //
    NR3DHALParam():
          srcCrop(0, 0)
        , dstRect(0,0)
        , gyroData()
    {
    }

};

struct NR3DHALResult
{
public:
    NR3DHwParam nr3dHwParam;

    MBOOL isGMVInfoUpdated = MFALSE;
    NR3DMVInfo gmvInfo;  // 3dnrlib-calculated mv info result

    Hal3dnrSavedFrameInfo prevSavedFrameInfo;
    Hal3dnrSavedFrameInfo currSavedFrameInfo;

//    MUINT32 errStatus = NR3D_ERROR_NONE;
    NR3D_STATE_ENUM state = NR3D_STATE_PREPARING;

    inline void resetNr3dHwParam()
    {
        nr3dHwParam.ctrl_onEn = 0;
    }
    inline void resetGMVInfo()
    {
        isGMVInfoUpdated = MFALSE;
    }

    void reset()
    {
        resetNr3dHwParam();
        resetGMVInfo();

        state = NR3D_STATE_PREPARING;
    }
};



typedef enum
{
    NR3D_PATH_NOT_DEF           = 0x00,     // invalid path
    NR3D_PATH_RRZO              = 0x01,     // rrzo path
    NR3D_PATH_RRZO_CRZ          = 0x02,     // rrzo + EIS1.2 apply CMV crop
    NR3D_PATH_IMGO              = 0x03,     // ZSD preview IMGO path
} NR3D_PATH_ENUM;

struct NR3DAlignParam
{
public:
    MUINT32 onOff_onOfStX;
    MUINT32 onOff_onOfStY;
    MUINT32 onSiz_onWd;
    MUINT32 onSiz_onHt;
    MUINT32 u4VipiOffset_X;
    MUINT32 u4VipiOffset_Y;
    MUINT32 vipi_readW;     //in pixel
    MUINT32 vipi_readH;     //in pixel

    NR3DAlignParam()
        : onOff_onOfStX(0x0)
        , onOff_onOfStY(0x0)
        , onSiz_onWd(0x0)
        , onSiz_onHt(0x0)
        , u4VipiOffset_X(0x0)
        , u4VipiOffset_Y(0x0)
        , vipi_readW(0x0)
        , vipi_readH(0x0)
     {
     }
  };



}; // namespace NR3D
}; // namespace NSCam
#endif // _3DNR_DEFS_H_

