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
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>

using namespace NSCam;
using namespace android;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;

// #define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
// #define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
// #define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
// #define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

// #define FUNC_START MY_LOGD("[%s] +", __FUNCTION__)
// #define FUNC_END   MY_LOGD("[%s] -", __FUNCTION__)

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

class StereoTuningProviderUTBase: public ::testing::Test
{
public:
    StereoTuningProviderUTBase() {}

    virtual ~StereoTuningProviderUTBase() {}

protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {}

    virtual ENUM_STEREO_RATIO           getImageRatio()        = 0;
    virtual MUINT32                     getLogicalDeviceID()   = 0;
    virtual MINT32                      getStereoFeatureMode() = 0;

    virtual void init()
    {
        StereoSettingProvider::setLogicalDeviceID(getLogicalDeviceID());
        StereoSettingProvider::setStereoFeatureMode(getStereoFeatureMode());
        StereoSettingProvider::setImageRatio(getImageRatio());
    }

protected:
    const char *SCENARIO_NAMES[TOTAL_STEREO_SCENARIO] = { "Unknown", "Preview", "Record", "Capture" };
};

#endif