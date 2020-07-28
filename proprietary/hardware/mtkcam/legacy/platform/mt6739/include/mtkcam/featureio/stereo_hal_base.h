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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _STEREO_HAL_BASE_H_
#define _STEREO_HAL_BASE_H_

#include <mtkcam/common.h>
#include <vector>
using namespace NSCam;

#include <mtkcam/v1/IParamsManager.h>
using namespace android;

#include "stereo_setting_provider.h"    //For STEREO_RATIO_E, STEREO_PARAMS_T

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum
{
    STEREO_SCENARIO_UNKNOWN     =   0x0000,
    STEREO_SCENARIO_PREVIEW     =   0x0001,     // DepthAF/Image Refocus preview
    STEREO_SCENARIO_RECORD      =   0x0002,     // DepthAF/Image Refocus record
    STEREO_SCENARIO_CAPTURE     =   0x0004,     // Image Refocus
    STEREO_SCENARIO_EIS         =   0x0008,     // EIS
    STEREO_SCENARIO_DISTANCE    =   0x0010,     // Distance measurement
}
STEREO_SCENARIO_ENUM;

struct INIT_DATA_STEREO_IN_T
{
    STEREO_SCENARIO_ENUM    eScenario;                 // only support 2 cases: preview/capture
    MSize                   main_image_size;           // JPEG size for image refocus
    MSize                   algo_image_size;           // preview size / JPS(SBS) size
    MSize                   algo_image_size_main2;     // main2 rrzo size
    MINT32                  main1_sensor_index;
    MINT32                  main2_sensor_index;
    MINT32                  orientation;               // image orientation? 0=>0 degree; 4=>90; 3=>180; 7=>270 (no use here)
    MINT32                  main1_sensor_scenario;     // use it to get main1 sensor crop info
    MINT32                  main2_sensor_scenario;     // use it to get main2 sensor crop info
    STEREO_RATIO_E          eImageRatio;               // image ratio (4:3 or 16:9)

public:     ////    Operations.
    INIT_DATA_STEREO_IN_T()
        : eScenario(STEREO_SCENARIO_UNKNOWN)
        , main_image_size()
        , algo_image_size()
        , main1_sensor_index(-1)
        , main2_sensor_index(-1)
        , orientation(0)
        , main1_sensor_scenario()
        , main2_sensor_scenario()
        , eImageRatio(eRatio_Default) {}

};


struct INIT_DATA_STEREO_OUT_T
{
    MSize   algoin_size;

public:     ////    Operations.
    INIT_DATA_STEREO_OUT_T()
        : algoin_size() {}
};


struct RRZ_DATA_STEREO_T
{
    MRect   rrz_crop_main1;
    MSize   rrz_size_main1;
    MRect   rrz_crop_main2;
    MSize   rrz_size_main2;

public:     ////    Operations.
    RRZ_DATA_STEREO_T()
        : rrz_crop_main1()
        , rrz_size_main1()
        , rrz_crop_main2()
        , rrz_size_main2() {}
};

struct FD_DATA_STEREO_T
{
    MUINT32 left;
    MUINT32 top;
    MUINT32 right;
    MUINT32 bottom;
    MUINT32 rotation;   //0~11

    FD_DATA_STEREO_T()
        : left(-9999)
        , top(-9999)
        , right(-9999)
        , bottom(-9999)
        , rotation(-1) {}
};

struct AF_WIN
{
    MUINT16 af_dac_index ;
    MUINT8  af_valid ;
    MFLOAT  af_confidence ;
    MUINT16 af_win_start_x ;
    MUINT16 af_win_start_y ;
    MUINT16 af_win_end_x ;
    MUINT16 af_win_end_y ;

public:     ////    Operations.
    AF_WIN()  { ::memset(this, 0, sizeof(AF_WIN)); }
};

struct SET_DATA_STEREO_T
{
    STEREO_SCENARIO_ENUM    eScenario;                 // only support 2 cases: preview/capture
    void* mSrcGraphicBuffer;
    void* mSrcEGLImage;
    void* mDstGraphicBuffer;
    void* mDstEGLImage;
    void* u4RgbaAddr_main1;	    // main1 rgba image address
    void* u4RgbaAddr_main2;	    // main2 rgba image address
    void* u4FEBufAddr_main1;    // main1 image feature points data array
    void* u4FEBufAddr_main2;    // main2 image feature points data array

    void* YV12BufAddr_main1;    // main2 YV12 image address
    void* YV12BufAddr_main2;    // main2 YV12 image address

    uintptr_t mSrcGraphicBufferVA;
    uintptr_t mDstGraphicBufferVA;
    MUINT32 mMagicNum;

///////// for 3rd party used/////////////
//  preview case:
//      (1) AlgoSrcBuf_main1_prv_rrz : main1 re-sized yuv
//      (2) AlgoSrcBuf_main2_prv_rrz : main2 re-sized yuv
//      (3) AlgoDstBuf_prv : output bokeh yuv for preview
//
//  capture case:
//      (1) AlgoSrcBuf_main1_cap_fullsize : main1 full sized yuv
//      (2) AlgoSrcBuf_main1_cap_rrz : main1 re-sized yuv
//      (3) AlgoSrcBuf_main2_cap_rrz : main2 re-sized yuv
//      (4) AlgoDstBuf_cap : output bokeh yuv for capture
//      (5) DepthmapDstBuf_cap : output depthmap raw for capture

        // preview buffer (IImageBuffer*)
        void* AlgoSrcBuf_main1_prv_rrz;
        void* AlgoSrcBuf_main2_prv_rrz;
        void* AlgoDstBuf_prv;

        // capture buffer (IImageBuffer*)
        void* AlgoSrcBuf_main1_cap_fullsize;
        void* AlgoSrcBuf_main1_cap_rrz;
        void* AlgoSrcBuf_main2_cap_rrz;

        void* AlgoDstBuf_cap;
        void* DepthmapDstBuf_cap;               // eImgFmt_Y8
/////////////////////////////////////////

public:     ////    Operations.
    SET_DATA_STEREO_T()  { ::memset(this, 0, sizeof(SET_DATA_STEREO_T)); }
};


struct OUT_DATA_STEREO_T
{
    STEREO_SCENARIO_ENUM    eScenario;                 // only support 2 cases: preview/capture

    MRect   algo_main1;     // Image Capture
    MRect   algo_main2;     // Image Capture
    MRect   main_crop;      // Image Refocus

public:     ////    Operations.
    OUT_DATA_STEREO_T()
        : algo_main1()
        , algo_main2()
        , main_crop()
        , eScenario(STEREO_SCENARIO_PREVIEW)
        {}
};


struct DBG_DATA_STEREO_T
{
    MUINT32     dbgDataSize;
    MUINT8*     dbgDataAddr;

public:     ////    Operations.
    DBG_DATA_STEREO_T()
        : dbgDataSize(0)
        , dbgDataAddr(NULL) {}
};


struct HW_DATA_STEREO_T
{
    MUINT32 rgba_image_width; // Image Capture
    MUINT32 rgba_image_height;
    MUINT32 rgba_image_stride;

    MUINT32 hwfe_block_size; // HWFE & Algorithm Image
    MUINT32 fefm_image_width;
    MUINT32 fefm_imgae_height;

public:     ////    Operations.
    HW_DATA_STEREO_T()  { ::memset(this, 0, sizeof(HW_DATA_STEREO_T)); }
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class StereoHalBase
{
public:
    static StereoHalBase* createInstance();
    virtual void destroyInstance() = 0;
    virtual bool init() = 0;
    virtual bool uninit() = 0;
    virtual ~StereoHalBase() {};

    virtual void setParameters(sp<IParamsManager> spParamsMgr) = 0;
    virtual MSize getMainSize(MSize const imgSize) const = 0;
    virtual MSize getAlgoInputSize(MUINT32 const idx) const = 0;
//	    virtual MSize getRrzSize(MUINT32 const idx) const = 0;
    virtual MSize getFEImgSize() const = 0;
    virtual MINT32 getAlgoInputTransform() const =0;
    virtual bool STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const = 0;
    virtual bool STEREOGetInfo(HW_DATA_STEREO_T &OutData) const = 0;
    virtual bool STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData) = 0;
    virtual bool STEREOSetParams(SET_DATA_STEREO_T RunData) = 0;
    virtual bool STEREORun(OUT_DATA_STEREO_T &OutData,MBOOL EnableAlgo = MTRUE) = 0;
    virtual bool STEREOGetDebugInfo(DBG_DATA_STEREO_T &DbgData) = 0;
    virtual bool STEREODestroy(void) = 0;

protected:

private:

};

#endif  // _STEREO_HAL_BASE_H_

