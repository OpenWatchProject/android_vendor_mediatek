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
#ifndef AIDEPTH_HAL_UT_H_
#define AIDEPTH_HAL_UT_H_

#define LOG_TAG "AIDEPTH_HAL_UT"

#include <limits.h>
#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>

#include <aidepth_hal.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <MTKAIDepth.h>
#pragma GCC diagnostic pop
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

#include "../../inc/stereo_dp_util.h"

#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>
#include <mtkcam/utils/std/ULog.h>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;

#define UT_CASE_PATH            "/sdcard/AIDEPTH_HAL_UT"
#define UT_CASE_IN_FOLDER       UT_CASE_PATH"/in"
#define UT_CASE_GOLDEN_FOLDER   UT_CASE_PATH"/golden"
#define UT_CASE_OUT_FOLDER      UT_CASE_PATH"/out"

#define MY_LOGD(fmt, arg...)    if(LOG_ENABLED) { printf("[D][%s]" fmt"\n", __func__, ##arg); }
#define MY_LOGI(fmt, arg...)    if(LOG_ENABLED) { printf("[I][%s]" fmt"\n", __func__, ##arg); }
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

#define FUNC_START MY_LOGD("[%s] +", __FUNCTION__)
#define FUNC_END   MY_LOGD("[%s] -", __FUNCTION__)

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

class AIDepthHALUTBase: public ::testing::Test
{
public:
    AIDepthHALUTBase() {}
    virtual ~AIDepthHALUTBase() {}

protected:
    virtual const char *getUTCaseName() { return "AIDEPTH_UT"; }

    virtual void SetUp() {
        if(!_isReadyToUT) {
            return;
        }

        MY_LOGD("[Create AIDEPTH_HAL...]");
        _pAIDepthHAL = AIDEPTH_HAL::createInstance();
        MY_LOGD("AIDepthHAL %p", _pAIDepthHAL);

        loadParam();
        loadImages();
    }

    virtual void TearDown() {
        if(!_isReadyToUT) {
            return;
        }

        if(_pAIDepthHAL) {
            delete _pAIDepthHAL;
            _pAIDepthHAL = NULL;
        }

        if(_imgMain1.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _imgMain1);
        }

        if(_imgMain2.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _imgMain2);
        }

        if(_depthmap.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _depthmap);
        }

        if(_depthmapGolden.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _depthmapGolden);
        }
    }

    virtual void loadParam()
    {
        //Load Init Info
        json document;
        std::ifstream fin(UT_CASE_IN_FOLDER"/AIDEPTH_INIT_INFO.json");
        bool parseSuccess = false;
        if(fin) {
            try
            {
                document = json::parse(fin, nullptr, false);
                parseSuccess = true;
            }
            catch (json::parse_error& e)
            {
                CAM_ULOGW(NSCam::Utils::ULog::MOD_VSDOF_HAL, "Parse error: %s", e.what());
            }
        }

        if(!parseSuccess) {
            CAM_ULOGE(NSCam::Utils::ULog::MOD_VSDOF_HAL, "Fail to parse AIDEPTH_INIT_INFO.json");
            return;
        }

        _input.mainCamPos              = document["TuningInfo"]["flipFlag"].get<int>();
        _imgSize.w                     = document["TuningInfo"]["imgWidth"].get<int>();
        _imgSize.h                     = document["TuningInfo"]["imgHeight"].get<int>();
        _finalOutputSize.w             = document["TuningInfo"]["imgFinalWidth"].get<int>();
        _finalOutputSize.h             = document["TuningInfo"]["imgFinalHeight"].get<int>();

        _tuningInfo.imgWidth          = _imgSize.w;
        _tuningInfo.imgHeight         = _imgSize.h;
        _tuningInfo.imgFinalWidth     = _finalOutputSize.w;
        _tuningInfo.convergenceOffset = document["TuningInfo"]["convergenceOffset"].get<int>();
        _tuningInfo.dispGain          = document["TuningInfo"]["dispGain"].get<int>();

        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _imgSize, !IS_ALLOC_GB, _imgMain1);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _imgSize, !IS_ALLOC_GB, _imgMain2);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _finalOutputSize, !IS_ALLOC_GB, _depthmap);

        _input.imageMain1 = _imgMain1.get();
        _input.imageMain2 = _imgMain2.get();
        _output.depthMap  = _depthmap.get();

        fin.close();
        fin.clear();

        //Load Params
        fin.open(UT_CASE_IN_FOLDER"/AIDEPTH_PROC_INFO.json");
        parseSuccess = false;
        if(fin) {
            try
            {
                document = json::parse(fin, nullptr, false);
                parseSuccess = true;
            }
            catch (json::parse_error& e)
            {
                CAM_ULOGW(NSCam::Utils::ULog::MOD_VSDOF_HAL, "Parse error: %s", e.what());
            }
        }

        if(!parseSuccess) {
            CAM_ULOGE(NSCam::Utils::ULog::MOD_VSDOF_HAL, "Fail to parse AIDEPTH_PROC_INFO.json");
            return;
        }

        _input.iso          = document["ParamInfo"]["ISOValue"].get<int>();
        _input.exposureTime = document["ParamInfo"]["ExposureTime"].get<int>();
        _input.dumpHint     = &_dumphint;

        fin.close();
        fin.clear();
    }

    virtual void init()
    {
        StereoSettingProvider::setLogicalDeviceID(3);
        StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_VSDOF);
        StereoSettingProvider::setImageRatio(eRatio_16_9);
    }

    void loadImages()
    {
        char filePath[256];
        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_RECT_IN1_%dx%d.yuv", _tuningInfo.imgWidth, _tuningInfo.imgHeight);
        readImage(filePath, _imgMain1.get());
        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_RECT_IN2_%dx%d.yuv", _tuningInfo.imgWidth, _tuningInfo.imgHeight);
        readImage(filePath, _imgMain2.get());

        // _depthmap
        if(!StereoDpUtil::allocImageBuffer(getUTCaseName(), eImgFmt_Y8, _finalOutputSize, !IS_ALLOC_GB, _depthmap)) {
            MY_LOGE("Cannot alloc depthmap");
            return;
        }
        // _depthmapGolden
    }

    virtual bool readImage(char *path, IImageBuffer *image)
    {
        struct stat st;
        ::memset(&st, 0, sizeof(struct stat));

        if(stat(path, &st) == -1) {
            MY_LOGD("%s does not exist", path);
            return false;
        }

        FILE *fp = fopen(path, "r");
        if(fp) {
            for(size_t p = 0; p < image->getPlaneCount(); ++p) {
                fread((void *)image->getBufVA(p), 1, image->getBufSizeInBytes(p), fp);
            }

            fclose(fp);
            fp = NULL;
        } else {
            MY_LOGE("Cannot open %s, err: %s", path, strerror(errno));
            return false;
        }

        return true;
    }

    virtual void readBuffer(char *path, void *&buffer)
    {
        struct stat st;
        ::memset(&st, 0, sizeof(struct stat));

        if(stat(path, &st) == -1) {
            MY_LOGE("%s does not exist", path);
            return;
        }

        FILE *fp = fopen(path, "r");
        if(fp) {
            if(NULL == buffer &&
               st.st_size > 0)
            {
                buffer = new MUINT8[st.st_size];
            }

            fread(buffer, 1, st.st_size, fp);
            fclose(fp);
            fp = NULL;
        } else {
            MY_LOGE("Cannot open %s, err: %s", path, strerror(errno));
        }
    }

    virtual bool isResultEmpty()
    {
        return false;
    }

    virtual bool isBitTrue()
    {
        return true;
    }

protected:
    //AIDEPTH parameters
    AIDEPTH_HAL *_pAIDepthHAL = NULL;
    AIDEPTH_HAL_PARAMS        _input;
    AIDEPTH_HAL_OUTPUT        _output;
    TuningUtils::FILE_DUMP_NAMING_HINT _dumphint;

    //data
    MSize                     _imgSize;
    MSize                     _finalOutputSize;
    AIDepthTuningInfo         _tuningInfo;
    AIDepthInitInfo           _initInfo;
    AIDepthParam              _paramInfo;

    sp<IImageBuffer> _imgMain1;
    sp<IImageBuffer> _imgMain2;
    sp<IImageBuffer> _depthmap;
    sp<IImageBuffer> _depthmapGolden;

    bool _isReadyToUT = true;
    bool LOG_ENABLED = StereoSettingProvider::isLogEnabled("vendor.STEREO.log.hal.aidepth.ut");
};

#endif