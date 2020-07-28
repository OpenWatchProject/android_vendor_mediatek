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
#ifndef _OCC_HAL_IMP_H_
#define _OCC_HAL_IMP_H_

#include <occ_hal.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <libocc/MTKOcc.h>
#pragma GCC diagnostic pop
#include <mtkcam/feature/stereo/hal/FastLogger.h>

using namespace std;
using namespace StereoHAL;

class OCC_HAL_IMP : public OCC_HAL
{
public:
    OCC_HAL_IMP();
    virtual ~OCC_HAL_IMP();

    virtual bool OCCHALRun(OCC_HAL_PARAMS &occHalParam, OCC_HAL_OUTPUT &occHalOutput);
protected:

private:
    void _setOCCParams(OCC_HAL_PARAMS &occHalParam);
    void _runOCC(OCC_HAL_OUTPUT &occHalOutput);
    //
    void _dumpInitData();
    void _dumpSetProcData();
    void _dumpOCCResult();
    void _dumpOCCBufferInfo(OccBufferInfo *buf, int index);
    //
    void _splitDisparityMapAndDump(const short *dispL, const short *dispR);
private:
    enum {
        OCC_PRV = 0,
        OCC_CAP,
        OCC_COUNT
    };

    const char *OCC_NAME[OCC_COUNT] = {
        "Preview",
        "Capture"
    };

    const ENUM_STEREO_SCENARIO SCENARIOS[OCC_COUNT] =
    {
        eSTEREO_SCENARIO_PREVIEW,
        eSTEREO_SCENARIO_CAPTURE
    };

    MTKOcc          *__pOCC[OCC_COUNT];

#ifdef OCC_CUSTOM_PARAM
    vector<OCCTuningParam> __tuningParams[OCC_COUNT];
#endif
    OccInitInfo     __initInfo[OCC_COUNT];
    OccProcInfo     __procInfo[OCC_COUNT];
    OccResultInfo   __resultInfo[OCC_COUNT];
    int             __occIndex = OCC_PRV;

    OccProcInfo     &__curProcInfo   = __procInfo[__occIndex];
    OccResultInfo   &__curResultInfo = __resultInfo[__occIndex];
    const char      *__curOccName    = OCC_NAME[__occIndex];

    const bool      DUMP_BUFFER;
    const MINT32    DUMP_START;
    const MINT32    DUMP_END;
    const bool      LOG_ENABLED;
    MINT32          m_requestNumber;
    MINT32          __timestamp;
    bool            m_isDump;

    FastLogger      __fastLogger;
};

#endif