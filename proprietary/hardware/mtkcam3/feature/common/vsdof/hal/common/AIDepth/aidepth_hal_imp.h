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
#ifndef AIDEPTH_HAL_IMP_H_
#define AIDEPTH_HAL_IMP_H_

#include <aidepth_hal.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <MTKAIDepth.h>
#pragma GCC diagnostic pop
#include <mtkcam3/feature/stereo/hal/FastLogger.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>

using namespace android;
using namespace StereoHAL;
using namespace NSCam;
using namespace std;

#define DUMP_PATH_SIZE (128)

class AIDEPTH_HAL_IMP : public AIDEPTH_HAL
{
public:
    AIDEPTH_HAL_IMP();
    virtual ~AIDEPTH_HAL_IMP();

    virtual bool AIDepthHALRun(AIDEPTH_HAL_PARAMS &param, AIDEPTH_HAL_OUTPUT &output);
protected:

private:
    void __init();
    bool __setAIDepthParams(AIDEPTH_HAL_PARAMS &occHalParam);
    bool __runAIDepth(AIDEPTH_HAL_OUTPUT &occHalOutput);

    void __logInitData();
    void __logSetProcData();
    void __logAIDepthResult();

    void __dumpInitData();
    void __dumpSetProcData();

private:
    MTKAIDepth          *__pAIDepth;

    int                 __mainCamPos = 0;
    MSize               __imgSize;
    MSize               __outputSize;   //after resizing, real output
    sp<IImageBuffer>    __aidepthOutputImg;
    AIDepthTuningInfo   __tuningInfo;
    AIDepthInitInfo     __initInfo;
    AIDepthParam        __paramInfo;
    AIDepthImageInfo    __imgInfo;
    AIDepthResultInfo   __resultInfo;
    size_t              __workingBufferSize = 0;

    const bool          LOG_ENABLED;
    char                __dumpPrefix[DUMP_PATH_SIZE];
    TuningUtils::FILE_DUMP_NAMING_HINT *__dumpHint = NULL;

    FastLogger      __fastLogger;
};

#endif