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
#ifndef PASS2A_P_2_SIZE_PROVIDER_H_
#define PASS2A_P_2_SIZE_PROVIDER_H_

#include "pass2_size_provider_base.h"
#include "pass2A_2_size_provider.h"

class Pass2A_P_2_SizeProvider: public Pass2SizeProviderBase<Pass2A_P_2_SizeProvider>
{
public:
    friend class Pass2SizeProviderBase<Pass2A_P_2_SizeProvider>;

    virtual StereoArea getWDMAArea( ENUM_STEREO_SCENARIO eScenario ) const {
        StereoArea area = Pass2A_2_SizeProvider::instance()->getWDMAArea(eScenario);
        if(StereoSettingProvider::isActiveStereo()) {
            return area;
        } else {
            area.removePadding();   //Capture buffer has padding due to 32-align
        }
        //For quality enhancement
        const float WARPING_IMAGE_RATIO = 1.0f; //For better quality
        const float FOV_RATIO           = StereoSettingProvider::getStereoCameraFOVRatio();
        const float IMAGE_RATIO         = WARPING_IMAGE_RATIO * FOV_RATIO;
        area.enlargeWith2AlignedRounding(IMAGE_RATIO);

        //Work around for GPU: input size must >= output size
        MSize outputSize = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, eScenario);
        if(area.size.w < outputSize.w ||
           area.size.h < outputSize.h)
        {
            MSize contentSize = area.contentSize();
            area.size.w = std::max(outputSize.w, contentSize.w);
            area.size.h = std::max(outputSize.h, contentSize.h);
            area.padding = area.size - contentSize;
        }

        // if(eSTEREO_SCENARIO_CAPTURE == eScenario)
        {
            area.apply32Align(true, false);   //For GPU
        }

        return area;
    }

    virtual StereoArea getIMG2OArea( ENUM_STEREO_SCENARIO eScenario ) const {
        StereoArea area = Pass2A_2_SizeProvider::instance()->getIMG2OArea(eScenario);

        const float FOV_RATIO = StereoSettingProvider::getStereoCameraFOVRatio();
        if(FOV_RATIO > 1.0f) {
            area *= FOV_RATIO;
        }

        return area.apply2Align();
    }

    virtual StereoArea getFEOInputArea( ENUM_STEREO_SCENARIO eScenario ) const {
        MSize oriFEOSize = Pass2A_2_SizeProvider::instance()->getFEOInputArea(eScenario);
        StereoArea areaFE(Pass2A_P_SizeProvider::instance()->getWROTArea(eScenario));
        areaFE.padding = areaFE.size - oriFEOSize;
        areaFE.startPt = MPoint(areaFE.padding.w>>1, areaFE.padding.h>>1);
        return areaFE;
    }
protected:
    Pass2A_P_2_SizeProvider() {}
    virtual ~Pass2A_P_2_SizeProvider() {}
private:
};

#endif