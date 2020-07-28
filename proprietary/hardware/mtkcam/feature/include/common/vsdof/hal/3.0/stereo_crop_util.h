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
#ifndef STEREO_CROP_UTIL_H_
#define STEREO_CROP_UTIL_H_

#include <mtkcam/feature/stereo/hal/StereoArea.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <camera_custom_stereo.h>

using namespace std;
using namespace NSCam;
using namespace StereoHAL;
using namespace NSCam::v1::Stereo;

namespace StereoHAL {

class CropUtil
{
public:
    static MRect &cropRectByImageRatio(MRect &rect, ENUM_STEREO_RATIO IMAGE_RATIO)
    {
        int m, n;
        imageRatioMToN(IMAGE_RATIO, m, n);
        //To make sure size will be even and fit aspect ratio
        n <<= 1;
        m <<= 1;

        //vertical center cropping
        int baseSize = std::min(rect.s.w/m, rect.s.h/n);
        int w = baseSize * m;
        int h = baseSize * n;
        rect.p.x += (rect.s.w - w)/2;
        rect.p.y += (rect.s.h - h)/2;
        rect.s.w = w;
        rect.s.h = h;

        return rect;
    }

    static MRect &cropRectByFOV(MRect &rect, const float CROP_RATIO, ENUM_STEREO_RATIO IMAGE_RATIO __attribute__((unused)))
    {
        if(CROP_RATIO >= 1.0f ||
           CROP_RATIO < 0.0f)
        {
            return rect;
        }

        const ENUM_ROTATION ROTATION = StereoSettingProvider::getModuleRotation();
        const bool IS_MAIN_ON_LEFT = (0 == StereoSettingProvider::getSensorRelativePosition());

        //1. Crop by FOV
        CUST_FOV_CROP_T cropSetting = StereoSettingProvider::getFOVCropSetting();
        MSize croppedSize(rect.s.w*CROP_RATIO, rect.s.h*CROP_RATIO);
        // 0 degree
        if( eRotate_0 == ROTATION )
        {
            const int TOTAL_CROP_SPACE = rect.s.w - croppedSize.w;
            const float CROP_LEFT = (IS_MAIN_ON_LEFT) ? (1-cropSetting.FOV_CROP_RATIO) : cropSetting.FOV_CROP_RATIO;
            rect.p.x += TOTAL_CROP_SPACE * CROP_LEFT;
            rect.p.y += (rect.s.h - croppedSize.h)/2;
        }
        // 180
        else if( eRotate_180 == ROTATION )
        {
            //Crop by FOV
            const int TOTAL_CROP_SPACE = rect.s.w - croppedSize.w;
            const float CROP_LEFT = (IS_MAIN_ON_LEFT) ? cropSetting.FOV_CROP_RATIO : (1-cropSetting.FOV_CROP_RATIO);
            rect.p.x += TOTAL_CROP_SPACE * CROP_LEFT;
            rect.p.y += (rect.s.h - croppedSize.h)/2;
        }
        // 90
        else if( eRotate_90 == ROTATION )
        {
            const int TOTAL_CROP_SPACE = rect.s.h - croppedSize.h;
            const float CROP_TOP = (IS_MAIN_ON_LEFT) ? cropSetting.FOV_CROP_RATIO : (1-cropSetting.FOV_CROP_RATIO);
            rect.p.y += TOTAL_CROP_SPACE * CROP_TOP;
            rect.p.x += (rect.s.w - croppedSize.w)/2;
        }
        // 270
        else
        {
            const int TOTAL_CROP_SPACE = rect.s.h - croppedSize.h;
            const float CROP_TOP = (IS_MAIN_ON_LEFT) ? (1-cropSetting.FOV_CROP_RATIO) : cropSetting.FOV_CROP_RATIO;
            rect.p.y += TOTAL_CROP_SPACE * CROP_TOP;
            rect.p.x += (rect.s.w - croppedSize.w)/2;
        }
        rect.s = croppedSize;

        return rect;
    }

    static StereoArea &cropStereoAreaByImageRatio(StereoArea &area, ENUM_STEREO_RATIO IMAGE_RATIO)
    {
        MRect rect;
        rect.p = area.startPt;
        rect.s = area.contentSize();
        cropRectByImageRatio(rect, IMAGE_RATIO);

        area.startPt = rect.p;
        area.padding = area.size - rect.s;

        return area;
    }

    static StereoArea &cropStereoAreaByFOV(StereoArea &area, const float CROP_RATIO, ENUM_STEREO_RATIO IMAGE_RATIO)
    {
        MRect rect;
        rect.p = area.startPt;
        rect.s = area.contentSize();
        cropRectByFOV(rect, CROP_RATIO, IMAGE_RATIO);

        area.startPt = rect.p;
        area.padding = area.size - rect.s;

        return area;
    }

    static StereoArea &cropStereoAreaByCaptureSize(StereoArea &area)
    {
        // Calculate current ratio and capture ratio
        MUINT32 precision = 100000;
        MSize curSize = area.contentSize();
        MSize capSize = StereoSizeProvider::getInstance()->captureImageSize();
        MUINT32 curRatio = precision*curSize.w/curSize.h;
        MUINT32 capRatio = precision*capSize.w/capSize.h;

        MSize tarSize;
        if (curRatio != capRatio) {
            // Change target size by capture ratio
            tarSize.w = (curRatio > capRatio) ? capRatio*curSize.h/precision : curSize.w;
            tarSize.h = (curRatio > capRatio) ? curSize.h : curSize.w*precision/capRatio;
            // Rounding to 2 align
            tarSize.w = (tarSize.w+1) & (~1);
            tarSize.h = (tarSize.h+1) & (~1);
            // Apply to output area
            area.padding = area.size - tarSize;
            area.startPt.x = area.padding.w>>1;
            area.startPt.y = area.padding.h>>1;
        }
        return area;
    }
};

};  //namespace StereoHAL
#endif