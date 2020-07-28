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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_3DNR_HAL_BASE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_3DNR_HAL_BASE_H_

#include <mtkcam/def/common.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>

//#define NR3D_WORKING_BUFF_WIDTH         (2304 * 6 / 5)  // (896)
//#define NR3D_WORKING_BUFF_HEIGHT        (2304 * 6 / 5)  // (1344 * 6 / 5)   // (512)
#define NR3D_WORKING_BUFF_WIDTH         2752     //(1920 * 6 / 5)  , this is for EIS limitation
#define NR3D_WORKING_BUFF_HEIGHT        2752     //(1080 * 6 / 5)  , this is for EIS limitation

using namespace NSCam;
using namespace android;

namespace NS3Av3 {

    class IHal3A;

};

namespace NSCam {

    class IImageBuffer;

};
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc { //for NR3D* &pNr3dParam

typedef enum
{
    NR3D_STATE_STOP         = 0x00,     // NR3D, IMG3O, VIPI all disabled.
    NR3D_STATE_PREPARING    = 0x01,     // IMG3O enabled, to output current frame for next frame use. NR3D, VIPI disabled. (When NR3D is disable, IMG30 output original image without any process.)
    NR3D_STATE_WORKING      = 0x02,     // NR3D, IMG3O, VIPI all enabled.
} NR3D_STATE_ENUM;

typedef enum
{
    NR3D_ERROR_NONE                = 0x00,     // No error.
    NR3D_ERROR_INVALID_GMV         = 0x01,     // GMV is invalid due to drop frame or something.
    NR3D_ERROR_GMV_TOO_LARGE       = 0x02,     // GMV X or Y is larger than a certain value.
    NR3D_ERROR_DROP_FRAME          = 0x04,     // Drop frame.
    NR3D_ERROR_FRAME_SIZE_CHANGED  = 0x08,     // Current frame size is not the same as previous frame. E.g. during DZ.
    NR3D_ERROR_FORCE_SKIP          = 0x10,     // Force skip by user, probably by adb command.
    NR3D_ERROR_UNDER_ISO_THRESHOLD = 0x11,     // must > iso threshold to turn on 3dnrs
    NR3D_ERROR_NOT_SUPPORT         = 0x12,     // not supported function
    NR3D_ERROR_INVALID_PARAM       = 0x14,     // invalid parameter
    NR3D_ERROR_INPUT_SRC_CHANGE    = 0x18,     //  input src change
} NR3D_ERROR_ENUM;

/******************************************************************************
 *
 * @struct NR3DParam
 * @brief parameter for NR3D HW setting
 * @details
 *
 ******************************************************************************/
struct NR3DParam
{
public:
    MUINT32 ctrl_onEn;
    MUINT32 onOff_onOfStX;
    MUINT32 onOff_onOfStY;
    MUINT32 onSiz_onWd;
    MUINT32 onSiz_onHt;
    MUINT32 vipi_offst;     //in byte
    MUINT32 vipi_readW;     //in pixel
    MUINT32 vipi_readH;     //in pixel

   NR3DParam()
       : ctrl_onEn(0x0)
       , onOff_onOfStX(0x0)
       , onOff_onOfStY(0x0)
       , onSiz_onWd(0x0)
       , onSiz_onHt(0x0)
       , vipi_offst(0x0)
       , vipi_readW(0x0)
       , vipi_readH(0x0)
   {
   }
};

struct NR3DRSCInfo
{
    MINTPTR pMV;
    MINTPTR pBV; // size is rssoSize
    MSize   rrzoSize;
    MSize   rssoSize;
    MUINT32 staGMV; // gmv value of RSC
    MBOOL   isValid;

    NR3DRSCInfo()
        : pMV(NULL)
        , pBV(NULL)
        , staGMV(0)
        , isValid(MFALSE)
    {
    }
};

struct NR3DGMVInfo
{
    MUINT32 x_int;
    MUINT32 y_int;
    MINT32 gmvX;
    MINT32 gmvY;
    MINT32 confX;
    MINT32 confY;

    NR3DGMVInfo()
        : x_int(0x0)
        , y_int(0x0)
        , gmvX(0x0)
        , gmvY(0x0)
        , confX(0x0)
        , confY(0x0)
    {
    }
};

struct NR3DHALParam
{
    public:
        // 3a tuning buffer
        void *pTuningData;
        NS3Av3::IHal3A* p3A;

        // frame generic
        MUINT32 frameNo;
        MINT32 iso;
        MINT32 isoThreshold;

        // imgi related
        MBOOL isCRZUsed;
        MBOOL isIMGO;
        MBOOL isBinning; // TODO: future use

        // lmv related info
        NR3DGMVInfo GMVInfo;
        NR3DRSCInfo RSCInfo;

        // vipi related
        IImageBuffer *pIMGBufferVIPI;

       // output related, ex: img3o
        MRect dst_resizer_rect;

        NR3D::GyroData gyroData;

        NR3DHALParam()
            : pTuningData(NULL)
            , p3A(NULL)
            , frameNo(0)
            , iso(0)
            , isoThreshold(0)
            // imgi related
            , isCRZUsed(MFALSE)
            , isIMGO(MFALSE)
            , isBinning(MFALSE)
            // vipi related
            , pIMGBufferVIPI(NULL)
            // output related, ex: img3o
            , dst_resizer_rect(0,0)
            , gyroData()
        {
        }

};

class hal3dnrBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static  hal3dnrBase*          createInstance();//deprecated
    virtual void                  destroyInstance() = 0;//deprecated
    static  hal3dnrBase*          createInstance(char const *userName, const MUINT32 sensorIdx);
    virtual void                  destroyInstance(char const *userName, const MUINT32 sensorIdx) = 0;
    virtual                       ~hal3dnrBase() {};

    virtual MBOOL                 init(MINT32 force3DNR = 0) = 0;
    virtual MBOOL                 uninit() = 0;
    virtual MBOOL                 prepare(MUINT32 frameNo, MINT32 iso) = 0;
    virtual MVOID                 setCMVMode(MBOOL useCMV);
    virtual MBOOL                 setGMV(MUINT32 frameNo, MINT32 gmvX, MINT32 gmvY, MINT32 cmvX_Int, MINT32 cmvY_Int) = 0;
    virtual MBOOL                 checkIMG3OSize(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH) = 0;
    virtual MBOOL                 setVipiParams(MBOOL isVIPIIn, MUINT32 vipiW, MUINT32 vipiH, MINT imgFormat, size_t stride) = 0;
    virtual MBOOL                 get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3DParam* &pNr3dParam) = 0;//deprecated
    virtual MBOOL                 get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3DParam &nr3dParam) = 0;
    virtual MBOOL                 checkStateMachine(NR3D_STATE_ENUM status) = 0;

    // warp several APIs into one simple API
    virtual MBOOL                 do3dnrFlow(
                                    void *pTuningData,
                                    MBOOL useCMV,
                                    MRect const &dst_resizer_rect,
                                    NR3DGMVInfo const &GMVInfo,
                                    NSCam::IImageBuffer *pIMGBufferVIPI,
                                    MINT32 iso,
                                    MUINT32 requestNo,
                                    NS3Av3::IHal3A* p3A) = 0;

    // warp several APIs into one simple API
    virtual MBOOL                do3dnrFlow_v2(const NR3DHALParam& nr3dHalParam) = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};
}
}
}


#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_3DNR_HAL_BASE_H_
