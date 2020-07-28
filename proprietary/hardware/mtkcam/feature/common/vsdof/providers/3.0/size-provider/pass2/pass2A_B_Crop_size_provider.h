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
#ifndef PASS2A_B_CROP_SIZE_PROVIDER_H_
#define PASS2A_B_CROP_SIZE_PROVIDER_H_

#include "pass2_size_provider_base.h"
#include <stereo_crop_util.h>

class Pass2A_B_Crop_SizeProvider: public Pass2SizeProviderBase<Pass2A_B_Crop_SizeProvider>
{
public:
    friend class Pass2SizeProviderBase<Pass2A_B_Crop_SizeProvider>;

    virtual StereoArea getWDMAArea( ENUM_STEREO_SCENARIO eScenario ) const {
        return Pass2A_Crop_SizeProvider::instance()->getWDMAArea(eScenario);
    }

    virtual StereoArea getWROTArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const  {
        //This is the size before module rotation, so we don't apply rotatedByModule
        return getWDMAArea(eScenario);
    }

    virtual StereoArea getIMG2OArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const  {
        //This is the size before module rotation, so we don't apply rotatedByModule
        MUINT32 junkStride;
        MSize   szMain1Output;
        MRect   tgCropRect;

        StereoSizeProvider::getInstance()->getPass1Size( eSTEREO_SENSOR_MAIN1,
                                                         (eSTEREO_SCENARIO_CAPTURE == eScenario) ? eImgFmt_BAYER10 : eImgFmt_FG_BAYER10,
                                                         (eSTEREO_SCENARIO_CAPTURE == eScenario) ? EPortIndex_IMGO : EPortIndex_RRZO,
                                                         eScenario,
                                                         //below are outputs
                                                         tgCropRect,
                                                         szMain1Output,
                                                         junkStride);

        StereoArea area(szMain1Output);
        //Only capture(IMGO) shuold be cropped by P2, preview/record(RRZO) is pre-cropped by P1
        if( eSTEREO_SCENARIO_CAPTURE == eScenario )
        {
            ENUM_STEREO_RATIO imageRatio = StereoSettingProvider::imageRatio();
            CropUtil::cropStereoAreaByFOV(CropUtil::cropStereoAreaByImageRatio(area, imageRatio),
                                          StereoSettingProvider::getMain1FOVCropRatio(),
                                          imageRatio);
        }

        return area;
    }

    virtual StereoArea getIMG3OArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const {
        //This is the size before module rotation, so we don't apply rotatedByModule
        return getWDMAArea(eScenario);
    }

    virtual StereoArea getFEOInputArea( ENUM_STEREO_SCENARIO eScenario __attribute__((unused))) const {
        //This is the size before module rotation, so we don't apply rotatedByModule
        return getWDMAArea(eScenario);
    }

protected:
    Pass2A_B_Crop_SizeProvider() {}
    virtual ~Pass2A_B_Crop_SizeProvider() {}
private:

};

#endif