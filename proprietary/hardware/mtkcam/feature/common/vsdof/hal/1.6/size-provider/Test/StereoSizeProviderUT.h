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
#ifndef _STEREO_SIZE_PROVIDER_UT_H_
#define _STEREO_SIZE_PROVIDER_UT_H_

#include <limits.h>
#include <gtest/gtest.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

#define PRINT_SIZE  1

inline void print(const char *tag, MSize size)
{
#if PRINT_SIZE
    printf("%s: %dx%d\n", tag, size.w, size.h);
#endif
}

inline void print(const char *tag, MRect rect)
{
#if PRINT_SIZE
    printf("%s: (%d, %d), %dx%d\n", tag, rect.p.x, rect.p.y, rect.s.w, rect.s.h);
#endif
}

inline void print(const char *tag, StereoArea area)
{
#if PRINT_SIZE
    printf("%s: Size %dx%d, Padding %dx%d, StartPt (%d, %d), ContentSize %dx%d\n", tag,
           area.size.w, area.size.h, area.padding.w, area.padding.h,
           area.startPt.x, area.startPt.y, area.contentSize().w, area.contentSize().h);
#endif
}

template<class T>
inline bool isEqual(T value, T expect)
{
    if(value != expect) {
        print("[Value ]", value);
        print("[Expect]", expect);

        return false;
    }

    return true;
}

#define MYEXPECT_EQ(val1, val2) EXPECT_TRUE(isEqual(val1, val2))

#define FRONT_STEREO_SENSOR_COUNT 4
class StereoSizeProviderUTBase: public ::testing::Test
{
public:
    StereoSizeProviderUTBase()
        : SENSOR_COUNT(MAKE_HalSensorList()->queryNumberOfSensors())
    {
        sizeProvider = StereoSizeProvider::getInstance();
    }

    virtual ~StereoSizeProviderUTBase() {}

protected:
    virtual void SetUp() {
        if(ignoreTest())
        {
            return;
        }

        FOV_RATIO  = StereoSettingProvider::getFOVCropRatio();
        IS_FOV_CROP = (StereoSettingProvider::isBMVSDoF() && FOV_RATIO < 1.0f && FOV_RATIO > 0.0f);

        sizeProvider->getPass1Size( eSTEREO_SENSOR_MAIN1,
                                    eImgFmt_FG_BAYER10,
                                    EPortIndex_RRZO,
                                    scenario,
                                    //below are outputs
                                    tgCropRect[0],
                                    szRRZO[0],
                                    junkStride);

        sizeProvider->getPass1Size( eSTEREO_SENSOR_MAIN2,
                                    eImgFmt_FG_BAYER10,
                                    EPortIndex_RRZO,
                                    scenario,
                                    //below are outputs
                                    tgCropRect[1],
                                    szRRZO[1],
                                    junkStride);

        if(eSTEREO_SCENARIO_CAPTURE == getScenario()) {
            sizeProvider->getPass1Size( eSTEREO_SENSOR_MAIN1,
                                        eImgFmt_BAYER10,
                                        EPortIndex_IMGO,
                                        scenario,
                                        //below are outputs
                                        tgCropRect[2],
                                        szMainIMGO,
                                        junkStride);
        } else {
            sizeProvider->getPass1ActiveArrayCrop(eSTEREO_SENSOR_MAIN1, activityArray[0]);
            sizeProvider->getPass1ActiveArrayCrop(eSTEREO_SENSOR_MAIN2, activityArray[1]);
        }

        //=============================================================================
        //  PASS 2
        //=============================================================================
        sizeProvider->captureImageSize() = MSize(3072, 2304);
        sizeProvider->getPass2SizeInfo(PASS2A,          scenario,   pass2SizeInfo[PASS2A]);
        sizeProvider->getPass2SizeInfo(PASS2A_P,        scenario,   pass2SizeInfo[PASS2A_P]);
        sizeProvider->getPass2SizeInfo(PASS2A_2,        scenario,   pass2SizeInfo[PASS2A_2]);
        sizeProvider->getPass2SizeInfo(PASS2A_P_2,      scenario,   pass2SizeInfo[PASS2A_P_2]);
        sizeProvider->getPass2SizeInfo(PASS2A_3,        scenario,   pass2SizeInfo[PASS2A_3]);
        sizeProvider->getPass2SizeInfo(PASS2A_P_3,      scenario,   pass2SizeInfo[PASS2A_P_3]);
        sizeProvider->getPass2SizeInfo(PASS2A_CROP,     scenario,   pass2SizeInfo[PASS2A_CROP]);
        sizeProvider->getPass2SizeInfo(PASS2A_B_CROP,   scenario,   pass2SizeInfo[PASS2A_B_CROP]);
    }

    // virtual void TearDown() {}

    virtual bool ignoreTest() = 0;

    virtual ENUM_STEREO_SCENARIO        getScenario()      = 0;
    virtual ENUM_STEREO_RATIO           getImageRatio()    = 0;
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile() = 0;
    virtual bool                        isDeNoise()        = 0;

    virtual void init()
    {
        scenario                               = getScenario();
        if( isDeNoise() ) {
            StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_DENOISE);
        } else {
            StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_VSDOF);
        }
        StereoSettingProvider::setImageRatio(getImageRatio());
        StereoSettingProvider::setStereoProfile(getStereoProfile());
        //Module rotation is defined in camera_custom_stereo.cpp of each project in custom folder
    }

protected:
    //Init
    const int SENSOR_COUNT;
    StereoSizeProvider *sizeProvider;
    ENUM_STEREO_SCENARIO scenario;

    //Pass 1
    MUINT32 junkStride;
    MRect tgCropRect[3];
    MSize szRRZO[2];
    MRect activityArray[2];
    MSize szMainIMGO;

    //Pass 2
    Pass2SizeInfo pass2SizeInfo[PASS2_ROUND_COUNT];

    //FOV cropping
    float FOV_RATIO  = 1.0f;
    bool IS_FOV_CROP = false;
};

#endif