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
#ifndef _FEFM_SETTING_PROVIDER_H_
#define _FEFM_SETTING_PROVIDER_H_

#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <FEFM_Trigger.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <utils/Mutex.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IHal3A.h>

namespace StereoHAL {

class FEFMSettingProvider
{
public:
    static FEFMSettingProvider *getInstance();
    static void destroyInstance();

    void init(int main1Index, int main2Index);

    bool needToRunFEFM(int main1MagicNumber, int main2MagicNumber, bool isCapture=false);

protected:
    FEFMSettingProvider();
    virtual ~FEFMSettingProvider();

private:
    static android::Mutex __Lock; //static member is mutable, so we no need to add mutable
    static FEFMSettingProvider *__instance;
    static MTKFEFMTrigger *__triggerInstance;

    int                     __sensorIndex[2] = {-1, -1};
    NS3Av3::DAF_TBL_STRUCT* __pAFTable[2]    = {NULL, NULL};
    bool                    __isAF[2]        = {false, false};
};

};  //namespace StereoHAL
#endif