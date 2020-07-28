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
#ifndef PASS2A_2_SIZE_PROVIDER_H_
#define PASS2A_2_SIZE_PROVIDER_H_

#include "pass2_size_provider_base.h"
#include "pass2A_size_provider.h"
#include <mtkcam3/feature/eis/eis_ext.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>

#define EIS_FACTOR 120

#ifdef EIS_FHD_FACTOR
const float EIS_RATIO = EIS_FHD_FACTOR/100.0f;
#else
const float EIS_RATIO = EIS_FACTOR/100.0f;
#endif

class Pass2A_2_SizeProvider: public Pass2SizeProviderBase<Pass2A_2_SizeProvider>
{
public:
    friend class Pass2SizeProviderBase<Pass2A_2_SizeProvider>;

    virtual StereoArea getWDMAArea( ENUM_STEREO_SCENARIO eScenario ) const {
        return StereoSizeProvider::getInstance()->getBufferSize(E_RECT_IN_M, eScenario);
    }

    virtual StereoArea getIMG2OArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const {
        return (Pass2A_SizeProvider::instance()->getWROTArea(eScenario)*0.5f);
    }

    virtual StereoArea getFEOInputArea( ENUM_STEREO_SCENARIO eScenario ) const {
        return Pass2A_SizeProvider::instance()->getWROTArea(eScenario).apply4Align();
    }
protected:
    Pass2A_2_SizeProvider() {}
    virtual ~Pass2A_2_SizeProvider() {}
private:

};

#endif