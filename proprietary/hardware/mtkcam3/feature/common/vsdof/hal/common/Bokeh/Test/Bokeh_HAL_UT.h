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
#ifndef BOKEH_HAL_UT_H_
#define BOKEH_HAL_UT_H_

#include <limits.h>
#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>

#include <bokeh_hal.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

#include "../../inc/stereo_dp_util.h"

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;

#define UT_CASE_PATH            "/sdcard/Bokeh_HAL_UT"
#define UT_CASE_IN_FOLDER       "in"
#define UT_CASE_GOLDEN_FOLDER   "golden"
#define UT_CASE_OUT_FOLDER      "out"

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

class BokehHALUTBase: public ::testing::Test
{
public:
    BokehHALUTBase() {}
    virtual ~BokehHALUTBase() {}

protected:
    virtual void SetUp() {
        if(!_isReadyToUT) {
            return;
        }

        MY_LOGD("[Create BOKEH_HAL...]");
        _pBokehHAL = BOKEH_HAL::createInstance(&_init);
        MY_LOGD("BWDNHAL %p", _pBokehHAL);

        loadImages();
        _input.cleanImage      = _cleanImage.get();
        _input.depthMap        = _depthMap.get();
        _input.nocMap          = _nocMap.get();
        _input.occMap          = _occMap.get();
        _input.extraData       = getExtraData();

        _output.bokehImage     = _bokehImage.get();
        _output.packedDepthmap = _packedBuffer.get();
    }

    virtual void TearDown() {
        if(!_isReadyToUT) {
            return;
        }

        char imagePath[256];
        MSize imageSize = getImageSize();
        sprintf(imagePath, UT_CASE_PATH"/%s/%s/BokehImage_%dx%d.yuv", getUTCaseName(), UT_CASE_OUT_FOLDER, imageSize.w, imageSize.h);
        MY_LOGD("Save output to %s", imagePath);
        _output.bokehImage->saveToFile(imagePath);

        const MSize DEPTH_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMW, eSTEREO_SCENARIO_CAPTURE);
        sprintf(imagePath, UT_CASE_PATH"/%s/%s/PackedDepthBuffer_%dx%d.yuv", getUTCaseName(), UT_CASE_OUT_FOLDER, DEPTH_SIZE.w, DEPTH_SIZE.h*4);
        MY_LOGD("Save packed buffer to %s", imagePath);
        _output.packedDepthmap->saveToFile(imagePath);

        _pBokehHAL->destroyInstance();

        if(_cleanImage.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _cleanImage);
        }

        if(_depthMap.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _depthMap);
        }

        if(_nocMap.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _nocMap);
        }

        if(_occMap.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _occMap);
        }

        if(_bokehImage.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _bokehImage);
        }

        if(_packedBuffer.get()) {
            StereoDpUtil::freeImageBuffer(getUTCaseName(), _packedBuffer);
        }

        if(_extraData) {
            delete [] _extraData;
            _extraData = NULL;
        }

        if(_golden) {
            delete [] _golden;
            _golden = NULL;
        }
    }

    virtual ENUM_STEREO_RATIO           getImageRatio()         = 0;
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()      = 0;
    virtual int                         getCaptureOrientation() = 0;
    virtual AF_WIN_COORDINATE_STRUCT    getFocusROI()           = 0;
    virtual int                         getDOFLevel()           = 0;
    virtual const char *                getUTCaseName()         = 0;

    virtual bool                        isReadyToUT()           { return isUTCaseExist(); }

    virtual char *getExtraData()
    {
        if(NULL == _extraData) {
            _extraData = new char[StereoSettingProvider::getExtraDataBufferSizeInBytes()];
        }

        AF_WIN_COORDINATE_STRUCT focus_roi = getFocusROI();
        sprintf(_extraData,
                "{\"capture_orientation\":{\"orientation\":%d},\"focus_roi\":{\"top\":%d,\"left\":%d,\"right\":%d,\"bottom\":%d},\"dof_level\":%d}",
                getCaptureOrientation(),
                focus_roi.af_win_start_y,
                focus_roi.af_win_start_x,
                focus_roi.af_win_end_x,
                focus_roi.af_win_end_y,
                getDOFLevel());
        MY_LOGD("Extra data:\n%s", _extraData);
        return _extraData;
    }

    virtual MSize getImageSize()
    {
        Pass2SizeInfo pass2SizeInfo;
        StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
        return pass2SizeInfo.areaWDMA.size;
    }

    virtual bool isUTCaseExist()
    {
        struct stat st;
        ::memset(&st, 0, sizeof(struct stat));

        char utPath[256];
        sprintf(utPath, "%s/%s/%s", UT_CASE_PATH, getUTCaseName(), UT_CASE_IN_FOLDER);
        if(stat(utPath, &st) == -1) {
            MY_LOGD("%s does not exist", utPath);
            return false;
        }

        sprintf(utPath, "%s/%s/%s", UT_CASE_PATH, getUTCaseName(), UT_CASE_GOLDEN_FOLDER);
        if(stat(utPath, &st) == -1) {
            MY_LOGD("%s does not exist", utPath);
            return false;
        }

        sprintf(utPath, "%s/%s/%s", UT_CASE_PATH, getUTCaseName(), UT_CASE_OUT_FOLDER);
        if(stat(utPath, &st) == -1) {
            MY_LOGD("%s does not exist", utPath);
            return false;
        }

        return true;
    }

    virtual void init()
    {
        _isReadyToUT = isReadyToUT();

        if(_isReadyToUT) {
            StereoSettingProvider::setImageRatio(getImageRatio());
            StereoSettingProvider::setStereoProfile(getStereoProfile());
            StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_VSDOF);
            StereoSettingProvider::setStereoModuleType(BAYER_AND_MONO);
        }
    }

    void loadImages()
    {
        MY_LOGD("[Load clean image]");
        char imagePath[256];
        MSize imageSize = getImageSize();
        // Clean image
        if(NULL == _cleanImage.get()) {
            if(!StereoDpUtil::allocImageBuffer(getUTCaseName(), eImgFmt_YV12, imageSize, !IS_ALLOC_GB, _cleanImage)) {
                MY_LOGE("Cannot alloc clean image");
                return;
            }
        }

        sprintf(imagePath, UT_CASE_PATH"/%s/%s/CleanImage_%dx%d.yuv", getUTCaseName(), UT_CASE_IN_FOLDER, imageSize.w, imageSize.h);
        MY_LOGD("Read %s...\n", imagePath);
        _cleanImage->unlockBuf( getUTCaseName() ) ;
        if(!_cleanImage->loadFromFile(imagePath)) {
            MY_LOGE("Fail to load %s", imagePath);
        }
        _cleanImage->lockBuf( getUTCaseName(), eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );

        //Load buffers
        const MSize DEPTH_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMW, eSTEREO_SCENARIO_CAPTURE);
        size_t depthBufferSize = DEPTH_SIZE.w*DEPTH_SIZE.h;

        if(NULL == _depthMap.get()) {
            if(!StereoDpUtil::allocImageBuffer(getUTCaseName(), eImgFmt_Y8, DEPTH_SIZE, !IS_ALLOC_GB, _depthMap)) {
                MY_LOGE("Cannot alloc depthmap");
                return;
            }
        }

        if(NULL == _nocMap.get()) {
            if(!StereoDpUtil::allocImageBuffer(getUTCaseName(), eImgFmt_Y8, DEPTH_SIZE, !IS_ALLOC_GB, _nocMap)) {
                MY_LOGE("Cannot alloc noc map");
                return;
            }
        }

        if(NULL == _occMap.get()) {
            if(!StereoDpUtil::allocImageBuffer(getUTCaseName(), eImgFmt_Y8, DEPTH_SIZE, !IS_ALLOC_GB, _occMap)) {
                MY_LOGE("Cannot alloc occ map");
                return;
            }
        }

        const char *BUFFER_NAME[3] =
        {
            "WMI", "NOC", "OCC"
        };

        IImageBuffer *imageBuffer[3] =
        {
            _depthMap.get(),
            _nocMap.get(),
            _occMap.get()
        };

        for(int i = 0; i < 3; i++) {
            MY_LOGD("[Load %s]", BUFFER_NAME[i]);
            sprintf(imagePath, UT_CASE_PATH"/%s/%s/%s_%dx%d.y", getUTCaseName(), UT_CASE_IN_FOLDER, BUFFER_NAME[i], DEPTH_SIZE.w, DEPTH_SIZE.h);
            MY_LOGD("Read %s...\n", imagePath);

            imageBuffer[i]->unlockBuf( getUTCaseName() ) ;
            if(!imageBuffer[i]->loadFromFile(imagePath)) {
                MY_LOGE("Fail to load %s", imagePath);
            }
            imageBuffer[i]->lockBuf( getUTCaseName(), eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );
        }

        //Output
        //bokeh image
        if(NULL == _bokehImage.get()) {
            if(!StereoDpUtil::allocImageBuffer(getUTCaseName(), eImgFmt_YV12, imageSize, !IS_ALLOC_GB, _bokehImage)) {
                MY_LOGE("Cannot alloc bokeh image");
                return;
            }
        }

        if(NULL == _packedBuffer.get()) {
            if(!StereoDpUtil::allocImageBuffer(getUTCaseName(), eImgFmt_Y8, DEPTH_SIZE*4, !IS_ALLOC_GB, _packedBuffer)) {
                MY_LOGE("Cannot alloc packed buffer");
                return;
            }
        }

        //Golden
        size_t bufferSize = imageSize.w * imageSize.h * 1.5f;
        if(NULL == _golden) {
            _golden = new MUINT8[bufferSize];
        }
        sprintf(imagePath, UT_CASE_PATH"/%s/%s/BokehImage_%dx%d.yuv", getUTCaseName(), UT_CASE_GOLDEN_FOLDER, imageSize.w, imageSize.h);
        MY_LOGD("Read %s...\n", imagePath);
        FILE *fp = fopen(imagePath, "rb");
        if(fp) {
            size_t readSize = fread(_golden, 1, bufferSize, fp);
            if(bufferSize != readSize) {
                MY_LOGE("Read golden failed, read %zu byte, expect %zu", readSize, bufferSize);
            }
        } else {
            MY_LOGE("Cannot read golden from %s, %s", imagePath, strerror(errno));
        }

        MY_LOGD("Load image done");
    }

    virtual bool isResultEmpty()
    {
        MSize imgSize = _output.bokehImage->getImgSize();
        size_t bufferSize = imgSize.w * imgSize.h;
        MUINT8 *emptyBuffer = new MUINT8[bufferSize];
        ::memset(emptyBuffer, 0, bufferSize);
        int result = ::memcmp((void *)_output.bokehImage->getBufVA(0), emptyBuffer, bufferSize);
        result |= ::memcmp((void *)_output.bokehImage->getBufVA(1), emptyBuffer, bufferSize>>2);
        result |= ::memcmp((void *)_output.bokehImage->getBufVA(2), emptyBuffer, bufferSize>>2);
        delete [] emptyBuffer;

        return (result == 0);
    }

    virtual bool isBitTrue()
    {
        MSize imgSize = _output.bokehImage->getImgSize();
        size_t bufferSize = imgSize.w * imgSize.h;

        MUINT8 *golden = _golden;

        int result = ::memcmp((void *)_output.bokehImage->getBufVA(0), golden, bufferSize);
        golden += bufferSize;
        result |= ::memcmp((void *)_output.bokehImage->getBufVA(1), golden, bufferSize>>2);
        golden += bufferSize>>2;
        result |= ::memcmp((void *)_output.bokehImage->getBufVA(2), golden, bufferSize>>2);

        return (result == 0);
    }

protected:
    //BWDN parameters
    BOKEH_HAL *_pBokehHAL = NULL;
    BOKEH_HAL_INIT_PARAMS   _init;
    BOKEH_HAL_PARAMS        _input;
    BOKEH_HAL_OUTPUT        _output;

    MUINT8* _golden = NULL;
    sp<IImageBuffer> _cleanImage;
    sp<IImageBuffer> _depthMap;
    sp<IImageBuffer> _nocMap;
    sp<IImageBuffer> _occMap;

    sp<IImageBuffer> _bokehImage;
    sp<IImageBuffer> _packedBuffer;
    char *_extraData = NULL;

    bool _isReadyToUT;
    bool LOG_ENABLED = StereoSettingProvider::isLogEnabled("vendor.STEREO.log.hal.bokeh.ut");
};

#endif