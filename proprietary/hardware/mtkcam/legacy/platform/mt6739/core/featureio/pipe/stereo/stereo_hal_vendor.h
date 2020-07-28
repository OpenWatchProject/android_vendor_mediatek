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
#ifndef _STEREO_HAL_VENDOR_H_
#define _STEREO_HAL_VENDOR_H_

#include "common.h"
#include <mtkcam/iopipe/SImager/IImageTransform.h>
#include "stereo_hal_base.h"
//	#include "MTKStereoKernel.h"
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <stdlib.h>
#include <vector>
#include <semaphore.h>

#include "camera_custom_nvram.h"
#include "mtkcam/featureio/IHal3A.h"
#include <mtkcam/featureio/fd_hal_base.h>
#include <mtkcam/drv/IHalSensor.h>
#include "stereo_setting_provider.h"    //For IMAGE_RESOLUTION_INFO_STRUCT
using android::Mutex;
using namespace NS3A;
using namespace NSCam::NSIoPipe::NSSImager;

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define MAX_FD_FACE_SUPPORT         15      // hardcore define here

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class StereoHalVendor: public StereoHalBase
{
protected:
    StereoHalVendor();
    virtual ~StereoHalVendor();

public:
    ////////////////////////////////////////////////////////////////////
    //  1. StereoHalVendor is singleton
    //  2. When caller use createInstance(), it will do "getInstance() + init()".
    //  3. init() function should allow to use multiple times
    ///////////////////////////////////////////////////////////////////
    static StereoHalVendor* createInstance();
    static StereoHalVendor* getInstance();

    //=== stereo_hal_base ===
    virtual void destroyInstance();
    virtual bool init();
    virtual bool uninit();

    virtual void setParameters(sp<IParamsManager> spParamsMgr);

    //Interface for middleware
    virtual MSize getMainSize(MSize const imgSize) const;
    virtual MSize getAlgoInputSize(MUINT32 const idx) const;
    virtual MSize getFEImgSize() const  {   return mFEImgSize; };
    virtual MINT32 getAlgoInputTransform() const ;

    virtual bool STEREOGetRrzInfo(RRZ_DATA_STEREO_T &OutData) const;
    virtual bool STEREOGetInfo(HW_DATA_STEREO_T &OutData) const;
    virtual bool STEREOInit(INIT_DATA_STEREO_IN_T InData, INIT_DATA_STEREO_OUT_T &OutData);
    virtual bool STEREOSetParams(SET_DATA_STEREO_T RunData); //For GPU buffer should in one thread, if not can meger with STEREORun
    virtual bool STEREORun(OUT_DATA_STEREO_T &OutData, MBOOL bEnableAlgo = MTRUE);
    virtual bool STEREOGetDebugInfo(DBG_DATA_STEREO_T &DbgData);
    virtual bool STEREODestroy(void);
    //=== stereo_hal_base ===

private:
    void _setMainImgSize(MSize const& imgSize)     	{ mMainSize = imgSize; };
    void _setAlgoImgSize(MSize const& imgSize)     	{ mAlgoSize = imgSize; };
    MSize _getAlgoImgSize(MUINT32 const idx) const	{ return (idx == 0)? mAlgoSize: mAlgoAppendSize; };
    MSize _getMainImgSize() const            		{ return mMainSize; };

    void _setDaf_info(DAF_VEC_STRUCT dac_vec)     	{ Mutex::Autolock lock(mLock_daf_vec); mdaf_vec = dac_vec; };
    DAF_VEC_STRUCT _getDaf_info()                    { Mutex::Autolock lock(mLock_daf_vec); return mdaf_vec; };

    void _encodeDebugInfo();
    void _dumpBuffer(MBOOL isPreview);
    //
    void _DoFakeAlgo(MBOOL isPreview);


private:
    volatile MINT32         mUsers;
    mutable Mutex           mLock;
    //
    MSize                   mAlgoSize;
    MSize                   mAlgoAppendSize;
    MSize                   mMainSize;
    MSize                   mFEImgSize;

    MUINT                   m_nMain1SensorIdx;
    MUINT                   m_nMain2SensorIdx;

    // for 3rd party algo preview
    MSize                   mAlgoSize_main1_prv_rrz;
    MSize                   mAlgoSize_main2_prv_rrz;
    IImageBuffer*           mAlgoSrcBuf_main1_prv_rrz;
    IImageBuffer*           mAlgoSrcBuf_main2_prv_rrz;
    IImageBuffer*           mAlgoDstBuf_prv;

    // for 3rd party algo capture
    MSize                   mAlgoSize_cap_fullsize;
    MSize                   mAlgoSize_main1_cap_rrz;
    MSize                   mAlgoSize_main2_cap_rrz;
    IImageBuffer*           mAlgoSrcBuf_main1_cap_fullsize;
    IImageBuffer*           mAlgoSrcBuf_main1_cap_rrz;
    IImageBuffer*           mAlgoSrcBuf_main2_cap_rrz;
    IImageBuffer*           mAlgoDstBuf_cap;
    IImageBuffer*           mDepthmapDstBuf_cap;

    //
    IImageTransform*        mpImgTransform;

    //
    MINT32                  m_nCapOrientation;

    // to get FD info
    halFDBase*              mpHalFD;
    MtkCameraFaceMetadata   mFD_info;
    MtkCameraFace           mfaces[MAX_FD_FACE_SUPPORT];
    MtkFaceInfo             mposInfo[MAX_FD_FACE_SUPPORT];

    // for AF info
    IHal3A*                 mpHal3A;
    DAF_TBL_STRUCT *        mprDafTbl;
    DAF_VEC_STRUCT          mdaf_vec;
    mutable Mutex           mLock_daf_vec;

    // to get sensor max crop size
    IHalSensor*             mpIHalSensor;
    MRect                   mMain1SensorMaxCrop;
    MRect                   mMain2SensorMaxCrop;

    // string for callback to APP
    char                   *m_debugInfo;

    // Parameter manager
    sp<IParamsManager>      mParamsMgr;

    int32_t                 mFakeAlgoTest;
    int32_t                 mDumpBuf;
    int32_t                 mEnableDebugLog;
    MUINT32                 mPrvCnt;
    MUINT32                 mCapCnt;
};

#endif  // _STEREO_HAL_VENDOR_H_

