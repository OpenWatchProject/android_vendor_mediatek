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
#ifndef STEREO_TUNING_PROVIDER_KERNEL_H_
#define STEREO_TUNING_PROVIDER_KERNEL_H_

#include <utils/threads.h>
#include <cutils/atomic.h>
#include <utils/Mutex.h>
#include <map>

#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>

#include "stereo_tuning_base.h"

#include <mtkcam3/feature/stereo/hal/FastLogger.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace android;
using android::Mutex;

typedef std::map<ENUM_STEREO_TUNING, StereoTuningBase *> TUNING_MAP_T;

class StereoTuningProviderKernel
{
public:
    static StereoTuningProviderKernel *getInstance();
    static void destroyInstance();

    void init();

    bool getTuningParams(ENUM_STEREO_TUNING tuningID, TuningQuery_T &param);

protected:
    StereoTuningProviderKernel();
    virtual ~StereoTuningProviderKernel();

private:
    void __saveTuningToFile(const KeepOrderJSON &loadedJson);

private:
    static Mutex    __instanceLock;
    static StereoTuningProviderKernel *__instance;

    TUNING_MAP_T    __tuningHolderMap;
    FastLogger      __logger;
    const bool      __IS_TUNING_ENABLED = (1 == checkStereoProperty(PROPERTY_ENABLE_TUNING));
    const bool      __IS_EXPORT_ENABLED = (1 == checkStereoProperty(PROPERTY_EXPORT_TUNING));
};

#endif