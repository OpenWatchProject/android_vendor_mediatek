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
#ifndef PASS2_SIZE_PROVIDER_BASE_H_
#define PASS2_SIZE_PROVIDER_BASE_H_

#include <utils/threads.h>
#include <cutils/atomic.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/pass2_size_data.h>

using android::Mutex;
using namespace StereoHAL;

template <class T>
class Pass2SizeProviderBase
{
private:
    static Mutex       mLock;
public:
    static T *instance() {
        Mutex::Autolock lock(mLock);

        static T *_instance = NULL;
        if(NULL == _instance) {
            _instance = new T();
        }

        return _instance;
    }

    virtual Pass2SizeInfo sizeInfo( ENUM_STEREO_SCENARIO eScenario ) const {
        return Pass2SizeInfo( getWDMAArea(eScenario),       //WDMA
                              getWROTArea(eScenario),       //WROT
                              getIMG2OArea(eScenario),      //IMG2O
                              getIMG3OArea(eScenario),      //IMG3O
                              getFEOInputArea(eScenario)    //FEO Input
                            );
    }

    virtual StereoArea getWDMAArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const { return STEREO_AREA_ZERO; }
    virtual StereoArea getWROTArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const { return STEREO_AREA_ZERO; }
    virtual StereoArea getIMG2OArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const { return STEREO_AREA_ZERO; }
    virtual StereoArea getIMG3OArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const { return MSIZE_ZERO; }
    virtual StereoArea getFEOInputArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const { return STEREO_AREA_ZERO; }
protected:
    Pass2SizeProviderBase() {}

    virtual ~Pass2SizeProviderBase() {}
};

template <class T>
Mutex Pass2SizeProviderBase<T>::mLock;

#endif