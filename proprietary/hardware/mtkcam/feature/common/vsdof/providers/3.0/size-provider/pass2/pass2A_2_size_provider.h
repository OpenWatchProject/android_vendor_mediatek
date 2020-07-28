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
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

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
        if(StereoSettingProvider::isActiveStereo()) {
            return StereoSize::getStereoArea1x(StereoSizeConfig(STEREO_AREA_WO_PADDING, STEREO_AREA_W_ROTATE));
        }

        StereoArea area;
        switch(eScenario) {
            case eSTEREO_SCENARIO_PREVIEW:
            case eSTEREO_SCENARIO_RECORD:
                area = StereoSize::getStereoArea1x(StereoSizeConfig(STEREO_AREA_WO_PADDING, STEREO_AREA_W_ROTATE));
                if(StereoSettingProvider::getStereoBaseline() <= DOUBLE_WIDTH_THRESHOLD) {
                    area.applyDoubleWidth();
                }

                // if(eSTEREO_SCENARIO_RECORD == eScenario) {
                //     //1. At lease EIS_RAIO enlarged (N3D requirement)
                //     //2. Must keep original ratio (N3D requirement)
                //     //3. Width must 16-align (Hardware requirement)
                //     int gcd = StereoGCD(area.size.w, area.size.h);
                //     int baseWidth  = area.size.w / gcd;
                //     int baseHeight = area.size.h / gcd;

                //     area.enlargeAndKeepOriginalRatio(EIS_RATIO);
                //     if(area.size.w % 16 != 0)
                //     {
                //         //min 16-align factor
                //         int factor = (area.size.w+15)/16;
                //         //meet image ratio, which means
                //         //(factor+n) * 16 / baseWidth must be integer
                //         int ratioFactor = baseWidth / StereoGCD(16, baseWidth);
                //         factor = ((factor+ratioFactor-1)/ratioFactor) * ratioFactor;
                //         area.size.w = factor * 16;
                //         area.size.h = area.size.w * baseHeight / baseWidth;
                //     }
                // }

                break;
            case eSTEREO_SCENARIO_CAPTURE:
                {
                    area = StereoSize::getStereoArea2x(StereoSizeConfig(STEREO_AREA_WO_PADDING, STEREO_AREA_W_ROTATE));
                    area.apply32Align(true, false);    //For GPU
                }
                break;
            default:
                break;
        }

        area.apply32Align(true, false);    //For GPU
        return area;
    }

    virtual StereoArea getIMG2OArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const {
        return (Pass2A_SizeProvider::instance()->getWROTArea(eScenario)*0.5f);
    }

    virtual StereoArea getFEOInputArea( ENUM_STEREO_SCENARIO eScenario ) const {
        return Pass2A_SizeProvider::instance()->getWROTArea(eScenario);
    }
protected:
    Pass2A_2_SizeProvider() {}
    virtual ~Pass2A_2_SizeProvider() {}
private:

};

#endif