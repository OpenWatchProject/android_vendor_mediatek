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
#ifndef FOV_CROP_UTIL_H_
#define FOV_CROP_UTIL_H_

#include <camera_custom_stereo.h>   //For DEFAULT_STEREO_SETTING
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include "stereo_setting_def.h"

using namespace std;
using namespace NSCam;
using namespace StereoHAL;

/**
 * \brief Get "keeped degrees" for a ratio to a degree
 * \details E.g. 60 degrees, ratio 0.95, return 57.488
 *
 * \param degree Degree before crop
 * \param ratio Ratio to cropped degree
 *
 * \return Degrees keeped from input degree
 */
inline float cropRatioToKeepDegree(float degree, float ratio)
{
    if(ratio < 0.0f ||
       ratio > 1.0f)
    {
        return 0.0f;
    }

    return radiansToDegree(2.0f*atan(ratio*tan(degreeToRadians(degree)/2.0f)));
}

/**
 * \brief Get "cropped degrees" for a ratio to a degree
 * \details E.g. 60 degrees, ratio 0.95, return 2.51198239
 *
 * \param degree Degree before crop
 * \param ratio Ratio to cropped degree
 *
 * \return Degrees to crop from input degree
 */
inline float cropRatioToDegree(float degree, float ratio)
{
    if(ratio < 0.0f ||
       ratio > 1.0f)
    {
        return 0.0f;
    }

    return degree - cropRatioToKeepDegree(degree, ratio);
}

class FOVCropUtil
{
public:
    static bool updateFOVCropRatios(StereoSensorConbinationSetting_T &sensorCombinatioSetting)
    {
        bool result = true;

        if(sensorCombinatioSetting.disableCrop) {
            return true;
        }

        ENUM_ROTATION ROTATION = eRotate_0;
        const bool IS_BACK_CAM = (0 == sensorCombinatioSetting.sensorSettings[0]->staticInfo.facingDirection);
        if(1 == sensorCombinatioSetting.moduleType ||
           3 == sensorCombinatioSetting.moduleType)
        {
            ROTATION = (IS_BACK_CAM) ? eRotate_90 : eRotate_270;
        }

        bool IS_MAIN_ON_LEFT = true;
        if(IS_BACK_CAM) {
            if(3 == sensorCombinatioSetting.moduleType ||
               4 == sensorCombinatioSetting.moduleType)
            {
                IS_MAIN_ON_LEFT = false;
            }
        } else {
            if(3 == sensorCombinatioSetting.moduleType) {
                IS_MAIN_ON_LEFT = false;
            }
        }

        const float MODULE_VARIATION = sensorCombinatioSetting.moduleVariation;

        bool cropMain1 = false;
        StereoSensorSetting_T *nonCropSensor = sensorCombinatioSetting.sensorSettings[0];
        StereoSensorSetting_T *croppedSensor = sensorCombinatioSetting.sensorSettings[1];
        float INNER_FOV;
        float OUTER_FOV;
        if(ROTATION & 0x2) {
            //Consider inf side FOV coverage
            cropMain1 = (sensorCombinatioSetting.sensorSettings[0]->fovVRuntime+2.0f*MODULE_VARIATION >=
                         sensorCombinatioSetting.sensorSettings[1]->fovVRuntime);

            //Consider near side FOV coverage
            if(!cropMain1)
            {
                //working range should be >= baseline(tan(Main2_FOV)-tan(Main1_FOV+module_variation))
                float main1Factor = tan(degreeToRadians(sensorCombinatioSetting.sensorSettings[0]->fovVRuntime/2.0f+MODULE_VARIATION));
                float main2Factor = tan(degreeToRadians(sensorCombinatioSetting.sensorSettings[1]->fovVRuntime/2.0f));
                cropMain1 = (sensorCombinatioSetting.workingRange - sensorCombinatioSetting.baseline/(main2Factor-main1Factor) < 0);
            }

            if(cropMain1)
            {
                cropMain1 = true;
                nonCropSensor = sensorCombinatioSetting.sensorSettings[1];
                croppedSensor = sensorCombinatioSetting.sensorSettings[0];
            }

            INNER_FOV = std::min(nonCropSensor->fovVRuntime, croppedSensor->fovVRuntime);
            OUTER_FOV = std::max(nonCropSensor->fovVRuntime, croppedSensor->fovVRuntime);
        } else {
            //Consider inf side FOV coverage
            cropMain1 = (sensorCombinatioSetting.sensorSettings[0]->fovHRuntime+2.0f*MODULE_VARIATION >=
                         sensorCombinatioSetting.sensorSettings[1]->fovHRuntime);

            //Consider near side FOV coverage
            if(!cropMain1)
            {
                //working range should be >= baseline(tan(Main2_FOV)-tan(Main1_FOV+module_variation))
                float main1Factor = tan(degreeToRadians(sensorCombinatioSetting.sensorSettings[0]->fovHRuntime/2.0f+MODULE_VARIATION));
                float main2Factor = tan(degreeToRadians(sensorCombinatioSetting.sensorSettings[1]->fovHRuntime/2.0f));
                cropMain1 = (sensorCombinatioSetting.workingRange - sensorCombinatioSetting.baseline/(main2Factor-main1Factor) < 0);
            }

            if(cropMain1)
            {
                nonCropSensor = sensorCombinatioSetting.sensorSettings[1];
                croppedSensor = sensorCombinatioSetting.sensorSettings[0];
            }

            INNER_FOV = std::min(nonCropSensor->fovHRuntime, croppedSensor->fovHRuntime);
            OUTER_FOV = std::max(nonCropSensor->fovHRuntime, croppedSensor->fovHRuntime);
        }

        const float HALF_INNER_FOV = INNER_FOV/2.0f;
        const float HALF_OUTER_FOV = OUTER_FOV/2.0f;
        const float BASELINE_RATIO = sensorCombinatioSetting.baseline / sensorCombinatioSetting.workingRange;

        //Calculate inf side
        float INF_RATIO  = 1.0f;
        float NEAR_RATIO = 1.0f;
        if(cropMain1) {
            INF_RATIO = degreeRatio(INNER_FOV - 2.0f*MODULE_VARIATION, OUTER_FOV);
            NEAR_RATIO = (tan(degreeToRadians(HALF_INNER_FOV - MODULE_VARIATION)) - BASELINE_RATIO)
                         / tan(degreeToRadians(HALF_OUTER_FOV));
        } else {
            INF_RATIO = degreeRatio(INNER_FOV + 2.0f*MODULE_VARIATION, OUTER_FOV);
            NEAR_RATIO = (tan(degreeToRadians(HALF_INNER_FOV + MODULE_VARIATION)) + BASELINE_RATIO)
                         / tan(degreeToRadians(HALF_OUTER_FOV));
        }

        if(INF_RATIO > 1.0f) {
            INF_RATIO = 1.0f;
        }

        if(NEAR_RATIO > 1.0f) {
            NEAR_RATIO = 1.0f;
        }

        if(sensorCombinatioSetting.isCenterCrop)
        {
            croppedSensor->cropSetting.keepRatio =
                (cropMain1) ? std::min(INF_RATIO, NEAR_RATIO)
                            : std::max(INF_RATIO, NEAR_RATIO);
            croppedSensor->cropSetting.cropRatio = 0.5f;
        }
        else
        {
            croppedSensor->cropSetting.keepRatio = (INF_RATIO + NEAR_RATIO)/2.0f;
            //Crop direction is from cropped sensor to non-cropped sensor
            //If crop main1, main1 inf side is toward main2
            //If crop main2, main2 near side is toward main1,
            //however CropUtil is refer to main1 direction, so we should apply inf side ratio
            croppedSensor->cropSetting.cropRatio = (1.0f-INF_RATIO)/(1.0f-NEAR_RATIO + 1.0f-INF_RATIO);
        }

        croppedSensor->cropSetting.cropDegreeH = cropRatioToDegree(croppedSensor->fovHRuntime, croppedSensor->cropSetting.keepRatio);
        croppedSensor->cropSetting.cropDegreeV = cropRatioToDegree(croppedSensor->fovVRuntime, croppedSensor->cropSetting.keepRatio);

        return result;
    }
};

#endif