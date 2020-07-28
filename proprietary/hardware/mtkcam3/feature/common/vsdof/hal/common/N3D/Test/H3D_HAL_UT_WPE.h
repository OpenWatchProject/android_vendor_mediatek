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
#ifndef N3D_HAL_UT_WPE_H_
#define N3D_HAL_UT_WPE_H_

#include <limits.h>
#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>

#include <n3d_hal.h>
#include "../n3d_hal_kernel.h"
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <utils/String8.h>

#include "../../inc/stereo_dp_util.h"

#include <mtkcam/utils/LogicalCam/LogicalCamJSONUtil.h>

#include <fstream>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;

#define UT_CASE_PATH            "/sdcard/N3D_HAL_UT"
#define UT_CASE_IN_FOLDER       UT_CASE_PATH"/in"
#define UT_CASE_GOLDEN_FOLDER   UT_CASE_PATH"/golden"
#define UT_CASE_OUT_FOLDER      UT_CASE_PATH"/out"

// #define MY_LOGD(fmt, arg...)    if(LOG_ENABLED) { printf("[D][%s]" fmt"\n", __func__, ##arg); }
// #define MY_LOGI(fmt, arg...)    if(LOG_ENABLED) { printf("[I][%s]" fmt"\n", __func__, ##arg); }
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

#define RING_BUFFER_SIZE (3)

class N3DHALUTWPEBase: public ::testing::Test
{
public:
    N3DHALUTWPEBase() {}
    virtual ~N3DHALUTWPEBase() {}

protected:
    virtual void SetUp() {
        // StereoSettingProvider::setLogicalDeviceID(3);
        // StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_VSDOF);
        // StereoSettingProvider::setPreviewSize(MSize(1440,1080));

        loadInitInfo(); //Will set image ratio here after reading init config
        FILE *fp = NULL;
        char filePath[256];
        // sprintf(filePath, UT_CASE_IN_FOLDER"/LensINFO");
        // readLDC(filePath, _LDCTable);

        sprintf(filePath, UT_CASE_IN_FOLDER"/N3D_NVRAM_IN");
        void *nvramData = &(_nvram.StereoNvramData.StereoData);
        readBuffer(filePath, nvramData);

        _pN3DKernel = new N3D_HAL_KERNEL(_initParam);

        MUINT32 offset = _pN3DKernel->getCalibrationOffsetInNVRAM();
        MY_LOGD("Calibration offset %u", offset);

        _pN3DKernel->setDumpConfig(true);
        StereoArea maskArea(_n3dInitInfo.iio_main.out_w, _n3dInitInfo.iio_main.out_h);
        if(_n3dInitInfo.system_cfg & (1<<7)) {
            maskArea.padding = MSize(_n3dInitInfo.iio_main.out_w - _n3dInitInfo.iio_main.src_h,
                                     _n3dInitInfo.iio_main.out_h - _n3dInitInfo.iio_main.src_w);
        } else {
            maskArea.padding = MSize(_n3dInitInfo.iio_main.out_w - _n3dInitInfo.iio_main.src_w,
                                     _n3dInitInfo.iio_main.out_h - _n3dInitInfo.iio_main.src_h);
        }

        if(maskArea.padding.w < 0) {
            maskArea.padding.w = 0;
        }
        if(maskArea.padding.h < 0) {
            maskArea.padding.h = 0;
        }
        maskArea.startPt.x = maskArea.padding.w/2;
        maskArea.startPt.y = maskArea.padding.h/2;

        StereoImageMargin maskMargin;
        // StereoTuningProvider::getN3DMaskMargin(eSTEREO_SCENARIO_PREVIEW, maskMargin);
        maskMargin.applyToStereoArea(maskArea);

        _pN3DKernel->initMain1Mask(maskArea);
        _pN3DKernel->initN3D(_n3dInitInfo, &_nvram, &(_LDCTable[0]));
        _pN3DKernel->dumpN3DInitConfig();

        loadProcInfo();
        _pN3DKernel->dumpN3DRuntimeConfig();
    }

    virtual void TearDown() {
        //Release graphic buffers
        for(int j = 0; j < 3; ++j) {
            for(auto &buf : _oututWarpMapMain2[j]) {
                StereoDpUtil::freeImageBuffer(LOG_TAG, buf);
            }
        }

        StereoDpUtil::freeImageBuffer(LOG_TAG, _inputImageMain1);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _outputImageMain1);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _outputMaskMain1);

        StereoDpUtil::freeImageBuffer(LOG_TAG, _feoMain1[0]);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _feoMain1[1]);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _feoMain2[0]);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _feoMain2[1]);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _fmoLR[0]);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _fmoLR[1]);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _fmoRL[0]);
        StereoDpUtil::freeImageBuffer(LOG_TAG, _fmoRL[1]);

        if(_afInfoMain) {
            delete _afInfoMain;
            _afInfoMain = NULL;
        }

        if(_afInfoAuxi) {
            delete _afInfoAuxi;
            _afInfoAuxi = NULL;
        }

        delete _pN3DKernel;
        _pN3DKernel = NULL;
    }

    virtual void init()
    {

    }

    virtual void loadInitInfo()
    {
        json document;
        std::ifstream fin(UT_CASE_IN_FOLDER"/N3D_INIT_INFO.json");
        bool parseSuccess = false;
        if(fin) {
            try
            {
                document = json::parse(fin, nullptr, false);
                parseSuccess = true;
            }
            catch (json::parse_error& e)
            {
                ALOGW("Parse error: %s", e.what());
            }
        }

        if(!parseSuccess) {
            ALOGE("Fail to parse N3D_INIT_INFO.json");
            return;
        }

        _n3dInitInfo.scenario = static_cast<STEREO_KERNEL_SCENARIO_ENUM>(document["scenario"].get<int>());

        _n3dInitInfo.iio_main.inp_w = document["iio_main"]["inp_w"].get<int>();
        _n3dInitInfo.iio_main.inp_h = document["iio_main"]["inp_h"].get<int>();
        _n3dInitInfo.iio_main.out_w = document["iio_main"]["out_w"].get<int>();
        _n3dInitInfo.iio_main.out_h = document["iio_main"]["out_h"].get<int>();
        _n3dInitInfo.iio_main.src_w = document["iio_main"]["src_w"].get<int>();
        _n3dInitInfo.iio_main.src_h = document["iio_main"]["src_h"].get<int>();
        _n3dInitInfo.iio_main.wpe_w = document["iio_main"]["wpe_w"].get<int>();
        _n3dInitInfo.iio_main.wpe_h = document["iio_main"]["wpe_h"].get<int>();

        _n3dInitInfo.iio_auxi.inp_w = document["iio_auxi"]["inp_w"].get<int>();
        _n3dInitInfo.iio_auxi.inp_h = document["iio_auxi"]["inp_h"].get<int>();
        _n3dInitInfo.iio_auxi.out_w = document["iio_auxi"]["out_w"].get<int>();
        _n3dInitInfo.iio_auxi.out_h = document["iio_auxi"]["out_h"].get<int>();
        _n3dInitInfo.iio_auxi.src_w = document["iio_auxi"]["src_w"].get<int>();
        _n3dInitInfo.iio_auxi.src_h = document["iio_auxi"]["src_h"].get<int>();
        _n3dInitInfo.iio_auxi.wpe_w = document["iio_auxi"]["wpe_w"].get<int>();
        _n3dInitInfo.iio_auxi.wpe_h = document["iio_auxi"]["wpe_h"].get<int>();

        _n3dInitInfo.geo_info = document["geo_info"].get<int>();

        json& geoImgValues = document["geo_img"];
        for(int i = 0; i < geoImgValues.size(); i++) {
            const json &geoImgValue = geoImgValues[i];

            _n3dInitInfo.geo_img[i].size                = geoImgValue["size"].get<int>();

            _n3dInitInfo.geo_img[i].img_main.width      = geoImgValue["img_main"]["width"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.height     = geoImgValue["img_main"]["height"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.depth      = geoImgValue["img_main"]["depth"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.stride     = geoImgValue["img_main"]["stride"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.format     = geoImgValue["img_main"]["format"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.act_width  = geoImgValue["img_main"]["act_width"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.act_height = geoImgValue["img_main"]["act_height"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.offset_x   = geoImgValue["img_main"]["offset_x"].get<int>();
            _n3dInitInfo.geo_img[i].img_main.offset_y   = geoImgValue["img_main"]["offset_y"].get<int>();

            _n3dInitInfo.geo_img[i].img_auxi.width      = geoImgValue["img_auxi"]["width"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.height     = geoImgValue["img_auxi"]["height"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.depth      = geoImgValue["img_auxi"]["depth"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.stride     = geoImgValue["img_auxi"]["stride"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.format     = geoImgValue["img_auxi"]["format"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.act_width  = geoImgValue["img_auxi"]["act_width"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.act_height = geoImgValue["img_auxi"]["act_height"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.offset_x   = geoImgValue["img_auxi"]["offset_x"].get<int>();
            _n3dInitInfo.geo_img[i].img_auxi.offset_y   = geoImgValue["img_auxi"]["offset_y"].get<int>();
        }

        _n3dInitInfo.pho_img.width      = document["pho_img"]["width"].get<int>();
        _n3dInitInfo.pho_img.height     = document["pho_img"]["height"].get<int>();
        _n3dInitInfo.pho_img.depth      = document["pho_img"]["depth"].get<int>();
        _n3dInitInfo.pho_img.stride     = document["pho_img"]["stride"].get<int>();
        _n3dInitInfo.pho_img.format     = document["pho_img"]["format"].get<int>();
        _n3dInitInfo.pho_img.act_width  = document["pho_img"]["act_width"].get<int>();
        _n3dInitInfo.pho_img.act_height = document["pho_img"]["act_height"].get<int>();
        _n3dInitInfo.pho_img.offset_x   = document["pho_img"]["offset_x"].get<int>();
        _n3dInitInfo.pho_img.offset_y   = document["pho_img"]["offset_y"].get<int>();

        _n3dInitInfo.fov_main[0] = document["fov_main.h"].get<float>();
        _n3dInitInfo.fov_main[1] = document["fov_main.v"].get<float>();
        _n3dInitInfo.fov_auxi[0] = document["fov_auxi.h"].get<float>();
        _n3dInitInfo.fov_auxi[1] = document["fov_auxi.v"].get<float>();

        _n3dInitInfo.baseline = document["baseline"].get<float>();

        _n3dInitInfo.system_cfg = document["system_cfg"].get<int>();

        _n3dInitInfo.af_init_main.dac_mcr  = document["af_init_main"]["dac_mcr"].get<int>();
        _n3dInitInfo.af_init_main.dac_inf  = document["af_init_main"]["dac_inf"].get<int>();
        _n3dInitInfo.af_init_main.dac_str  = document["af_init_main"]["dac_str"].get<int>();
        _n3dInitInfo.af_init_main.dist_mcr = document["af_init_main"]["dist_mcr"].get<int>();
        _n3dInitInfo.af_init_main.dist_inf = document["af_init_main"]["dist_inf"].get<int>();

        _n3dInitInfo.af_init_auxi.dac_mcr  = document["af_init_auxi"]["dac_mcr"].get<int>();
        _n3dInitInfo.af_init_auxi.dac_inf  = document["af_init_auxi"]["dac_inf"].get<int>();
        _n3dInitInfo.af_init_auxi.dac_str  = document["af_init_auxi"]["dac_str"].get<int>();
        _n3dInitInfo.af_init_auxi.dist_mcr = document["af_init_auxi"]["dist_mcr"].get<int>();
        _n3dInitInfo.af_init_auxi.dist_inf = document["af_init_auxi"]["dist_inf"].get<int>();

        _n3dInitInfo.flow_main.pixel_array_width  = document["flow_main"]["pixel_array_width"].get<int>();
        _n3dInitInfo.flow_main.pixel_array_height = document["flow_main"]["pixel_array_height"].get<int>();
        _n3dInitInfo.flow_main.sensor_offset_x0   = document["flow_main"]["sensor_offset_x0"].get<int>();
        _n3dInitInfo.flow_main.sensor_offset_y0   = document["flow_main"]["sensor_offset_y0"].get<int>();
        _n3dInitInfo.flow_main.sensor_size_w0     = document["flow_main"]["sensor_size_w0"].get<int>();
        _n3dInitInfo.flow_main.sensor_size_h0     = document["flow_main"]["sensor_size_h0"].get<int>();
        _n3dInitInfo.flow_main.sensor_scale_w     = document["flow_main"]["sensor_scale_w"].get<int>();
        _n3dInitInfo.flow_main.sensor_scale_h     = document["flow_main"]["sensor_scale_h"].get<int>();
        _n3dInitInfo.flow_main.sensor_offset_x1   = document["flow_main"]["sensor_offset_x1"].get<int>();
        _n3dInitInfo.flow_main.sensor_offset_y1   = document["flow_main"]["sensor_offset_y1"].get<int>();
        _n3dInitInfo.flow_main.sensor_size_w1     = document["flow_main"]["sensor_size_w1"].get<int>();
        _n3dInitInfo.flow_main.sensor_size_h1     = document["flow_main"]["sensor_size_h1"].get<int>();
        _n3dInitInfo.flow_main.tg_offset_x        = document["flow_main"]["tg_offset_x"].get<int>();
        _n3dInitInfo.flow_main.tg_offset_y        = document["flow_main"]["tg_offset_y"].get<int>();
        _n3dInitInfo.flow_main.tg_size_w          = document["flow_main"]["tg_size_w"].get<int>();
        _n3dInitInfo.flow_main.tg_size_h          = document["flow_main"]["tg_size_h"].get<int>();
        _n3dInitInfo.flow_main.rrz_offset_x       = document["flow_main"]["rrz_offset_x"].get<int>();
        _n3dInitInfo.flow_main.rrz_offset_y       = document["flow_main"]["rrz_offset_y"].get<int>();
        _n3dInitInfo.flow_main.rrz_usage_width    = document["flow_main"]["rrz_usage_width"].get<int>();
        _n3dInitInfo.flow_main.rrz_usage_height   = document["flow_main"]["rrz_usage_height"].get<int>();
        _n3dInitInfo.flow_main.rrz_out_width      = document["flow_main"]["rrz_out_width"].get<int>();
        _n3dInitInfo.flow_main.rrz_out_height     = document["flow_main"]["rrz_out_height"].get<int>();
        _n3dInitInfo.flow_main.mdp_rotate         = document["flow_main"]["mdp_rotate"].get<int>();
        _n3dInitInfo.flow_main.mdp_offset_x       = document["flow_main"]["mdp_offset_x"].get<int>();
        _n3dInitInfo.flow_main.mdp_offset_y       = document["flow_main"]["mdp_offset_y"].get<int>();
        _n3dInitInfo.flow_main.mdp_usage_width    = document["flow_main"]["mdp_usage_width"].get<int>();
        _n3dInitInfo.flow_main.mdp_usage_height   = document["flow_main"]["mdp_usage_height"].get<int>();

        _n3dInitInfo.flow_auxi.pixel_array_width  = document["flow_auxi"]["pixel_array_width"].get<int>();
        _n3dInitInfo.flow_auxi.pixel_array_height = document["flow_auxi"]["pixel_array_height"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_offset_x0   = document["flow_auxi"]["sensor_offset_x0"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_offset_y0   = document["flow_auxi"]["sensor_offset_y0"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_size_w0     = document["flow_auxi"]["sensor_size_w0"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_size_h0     = document["flow_auxi"]["sensor_size_h0"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_scale_w     = document["flow_auxi"]["sensor_scale_w"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_scale_h     = document["flow_auxi"]["sensor_scale_h"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_offset_x1   = document["flow_auxi"]["sensor_offset_x1"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_offset_y1   = document["flow_auxi"]["sensor_offset_y1"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_size_w1     = document["flow_auxi"]["sensor_size_w1"].get<int>();
        _n3dInitInfo.flow_auxi.sensor_size_h1     = document["flow_auxi"]["sensor_size_h1"].get<int>();
        _n3dInitInfo.flow_auxi.tg_offset_x        = document["flow_auxi"]["tg_offset_x"].get<int>();
        _n3dInitInfo.flow_auxi.tg_offset_y        = document["flow_auxi"]["tg_offset_y"].get<int>();
        _n3dInitInfo.flow_auxi.tg_size_w          = document["flow_auxi"]["tg_size_w"].get<int>();
        _n3dInitInfo.flow_auxi.tg_size_h          = document["flow_auxi"]["tg_size_h"].get<int>();
        _n3dInitInfo.flow_auxi.rrz_offset_x       = document["flow_auxi"]["rrz_offset_x"].get<int>();
        _n3dInitInfo.flow_auxi.rrz_offset_y       = document["flow_auxi"]["rrz_offset_y"].get<int>();
        _n3dInitInfo.flow_auxi.rrz_usage_width    = document["flow_auxi"]["rrz_usage_width"].get<int>();
        _n3dInitInfo.flow_auxi.rrz_usage_height   = document["flow_auxi"]["rrz_usage_height"].get<int>();
        _n3dInitInfo.flow_auxi.rrz_out_width      = document["flow_auxi"]["rrz_out_width"].get<int>();
        _n3dInitInfo.flow_auxi.rrz_out_height     = document["flow_auxi"]["rrz_out_height"].get<int>();
        _n3dInitInfo.flow_auxi.mdp_rotate         = document["flow_auxi"]["mdp_rotate"].get<int>();
        _n3dInitInfo.flow_auxi.mdp_offset_x       = document["flow_auxi"]["mdp_offset_x"].get<int>();
        _n3dInitInfo.flow_auxi.mdp_offset_y       = document["flow_auxi"]["mdp_offset_y"].get<int>();
        _n3dInitInfo.flow_auxi.mdp_usage_width    = document["flow_auxi"]["mdp_usage_width"].get<int>();
        _n3dInitInfo.flow_auxi.mdp_usage_height   = document["flow_auxi"]["mdp_usage_height"].get<int>();

        // float imageRatio = _n3dInitInfo.flow_main.rrz_out_width / (float)_n3dInitInfo.flow_main.rrz_out_height;
        // if(fabs(imageRatio - (16.0f/9.0f)) < 0.01f) {
        //     _imageRatio = eRatio_16_9;
        //     MY_LOGD("Set image ratio to 16:9");
        // } else if(fabs(imageRatio - (4.0f/3.0f)) < 0.01f) {
        //     _imageRatio = eRatio_4_3;
        //     MY_LOGD("Set image ratio to 4:3");
        // } else {
        //     _imageRatio = eRatio_4_3;
        //     MY_LOGW("Unkown ratio (RRZO %dx%d), I'll use 4:3", _n3dInitInfo.flow_main.rrz_out_width, _n3dInitInfo.flow_main.rrz_out_height);
        // }
        // StereoSettingProvider::setImageRatio(_imageRatio);

        //Init main1 images
        MSize inputSize  = MSize(_n3dInitInfo.iio_main.inp_w, _n3dInitInfo.iio_main.inp_h);
        MSize outputSize = MSize(_n3dInitInfo.iio_main.out_w, _n3dInitInfo.iio_main.out_h);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YV12, inputSize, !IS_ALLOC_GB, _inputImageMain1);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YV12, outputSize, !IS_ALLOC_GB, _outputImageMain1);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, outputSize, !IS_ALLOC_GB, _outputMaskMain1);

        //Init CC images
        Pass2SizeInfo pass2Info;
        ENUM_STEREO_SCENARIO scenario = (STEREO_KERNEL_SCENARIO_IMAGE_CAPTURE == _n3dInitInfo.scenario)
                                        ? eSTEREO_SCENARIO_PREVIEW : eSTEREO_SCENARIO_CAPTURE;

        //Init fefm buffers
        StereoSettingProvider::queryHWFEOBufferSize(MSize(_n3dInitInfo.geo_img[0].img_main.act_width, _n3dInitInfo.geo_img[0].img_main.act_height),
                                                    _n3dInitInfo.geo_img[0].size, _feSize);
        StereoSettingProvider::queryHWFMOBufferSize(_feSize, _fmSize);

        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _feSize, !IS_ALLOC_GB, _feoMain1[0]);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _feSize, !IS_ALLOC_GB, _feoMain1[1]);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _feSize, !IS_ALLOC_GB, _feoMain2[0]);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _feSize, !IS_ALLOC_GB, _feoMain2[1]);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _fmSize, !IS_ALLOC_GB, _fmoLR[0]);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _fmSize, !IS_ALLOC_GB, _fmoLR[1]);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _fmSize, !IS_ALLOC_GB, _fmoRL[0]);
        StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, _fmSize, !IS_ALLOC_GB, _fmoRL[1]);

        createGraphicBuffers();
    }

    virtual void createGraphicBuffers()
    {
        MSize outputSize = MSize(_n3dInitInfo.iio_auxi.wpe_w, _n3dInitInfo.iio_auxi.wpe_h);
        for(int j = 0; j < 3; ++j) {
            for(int i = 0; i < RING_BUFFER_SIZE; i++) {
                sp<IImageBuffer> buf;
                StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_RGBA8888, outputSize, IS_ALLOC_GB, buf);
                _oututWarpMapMain2[j].push_back(buf);
                ALOGD("_oututWarpMapMain2[%d][%d]: %p, HW Buffer %p", j, i, buf.get(), buf.get()->getImageBufferHeap()->getHWBuffer());
                _initParam.outputWarpMapMain2[j].push_back(buf.get());
            }
        }
    }

    virtual void loadProcInfo()
    {
        json document;
        std::ifstream fin(UT_CASE_IN_FOLDER"/N3D_PROC_INFO.json");
        bool parseSuccess = false;
        if(fin) {
            try
            {
                document = json::parse(fin, nullptr, false);
                parseSuccess = true;
            }
            catch (json::parse_error& e)
            {
                ALOGW("Parse error: %s", e.what());
            }
        }

        if(!parseSuccess) {
            ALOGE("Fail to parse N3D_PROC_INFO.json");
            return;
        }

        _afInfoMain = NULL;
        if(_n3dInitInfo.system_cfg & (1<<9)) {
            _afInfoMain = new STEREO_KERNEL_AF_INFO_STRUCT();
            _afInfoMain->dac_i = document["af_main"]["dac_i"].get<int>();
            _afInfoMain->dac_v = document["af_main"]["dac_v"].get<int>();
            _afInfoMain->dac_c = document["af_main"]["dac_c"].get<float>();

            json& dacWValues = document["af_main"]["dac_w"];
            for(int i = 0; i < dacWValues.size(); i++) {
                const json &dacWValue = dacWValues[i];
                _afInfoMain->dac_w[i] = dacWValue.get<int>();
            }
        }

        _afInfoAuxi = NULL;
        if(_n3dInitInfo.system_cfg & (1<<10)) {
            _afInfoAuxi = new STEREO_KERNEL_AF_INFO_STRUCT();
            _afInfoAuxi->dac_i = document["af_auxi"]["dac_i"].get<int>();
            _afInfoAuxi->dac_v = document["af_auxi"]["dac_v"].get<int>();
            _afInfoAuxi->dac_c = document["af_auxi"]["dac_c"].get<float>();

            json& dacWValues = document["af_auxi"]["dac_w"];
            for(int i = 0; i < dacWValues.size(); i++) {
                const json &dacWValue = dacWValues[i];
                _afInfoAuxi->dac_w[i] = dacWValues.get<int>();
            }
        }

        loadBuffers();
    }

    virtual void loadBuffers()
    {
        char filePath[256];

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_RECT_IN1__%dx%d_8_s0.yv12", _n3dInitInfo.iio_main.inp_w, _n3dInitInfo.iio_main.inp_h);
        readImage(filePath, _inputImageMain1.get());

        // sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_RECT_IN2__%dx%d_8_s0.yv12", _n3dInitInfo.iio_auxi.inp_w, _n3dInitInfo.iio_auxi.inp_h);
        // readImage(filePath, _inputImageBuffers[_ringIdx].get());

        bool hasFEFM = false;
        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FE1BO__%dx%d_8_s0.unknown", _feSize.w, _feSize.h);
        hasFEFM = readImage(filePath, _feoMain1[0].get());

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FE1CO__%dx%d_8_s0.unknown", _feSize.w, _feSize.h);
        hasFEFM = hasFEFM && readImage(filePath, _feoMain1[1].get());

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FE2BO__%dx%d_8_s0.unknown", _feSize.w, _feSize.h);
        hasFEFM = hasFEFM && readImage(filePath, _feoMain2[0].get());

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FE2CO__%dx%d_8_s0.unknown", _feSize.w, _feSize.h);
        hasFEFM = hasFEFM && readImage(filePath, _feoMain2[1].get());

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FMBO_LR__%dx%d_8_s0.unknown", _fmSize.w, _fmSize.h);
        hasFEFM = hasFEFM && readImage(filePath, _fmoLR[0].get());

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FMBO_RL__%dx%d_8_s0.unknown", _fmSize.w, _fmSize.h);
        hasFEFM = hasFEFM && readImage(filePath, _fmoLR[1].get());

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FMCO_LR__%dx%d_8_s0.unknown", _fmSize.w, _fmSize.h);
        hasFEFM = hasFEFM && readImage(filePath, _fmoRL[0].get());

        sprintf(filePath, UT_CASE_IN_FOLDER"/BID_P2A_OUT_FMCO_RL__%dx%d_8_s0.unknown", _fmSize.w, _fmSize.h);
        hasFEFM = hasFEFM && readImage(filePath, _fmoRL[1].get());

        if(hasFEFM) {
            _hwfefmData.geoDataMain1[0]       = _feoMain1[0].get();
            _hwfefmData.geoDataMain1[1]       = _feoMain1[1].get();
            _hwfefmData.geoDataMain2[0]       = _feoMain2[0].get();
            _hwfefmData.geoDataMain2[1]       = _feoMain2[1].get();
            _hwfefmData.geoDataLeftToRight[0] = _fmoLR[0].get();
            _hwfefmData.geoDataLeftToRight[1] = _fmoLR[1].get();
            _hwfefmData.geoDataRightToLeft[0] = _fmoRL[0].get();
            _hwfefmData.geoDataRightToLeft[1] = _fmoRL[1].get();
        }

        _previewOutput.warpMapMain2[0] = _oututWarpMapMain2[0][_ringIdx].get();
        _previewOutput.warpMapMain2[1] = _oututWarpMapMain2[1][_ringIdx].get();
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

    // virtual void readLDC(char *path, vector<float> &ldcTable)
    // {
    //     struct stat st;
    //     ::memset(&st, 0, sizeof(struct stat));

    //     if(stat(path, &st) == -1) {
    //         MY_LOGE("%s does not exist", path);
    //         ldcTable.clear();
    //         return;
    //     }

    //     std::ifstream fin(path);
    //     if(!fin) {
    //         MY_LOGE("Cannot open %s, err: %s", path, strerror(errno));
    //         return;
    //     }

    //     std::string line;
    //     float number = 0.0f;
    //     while(std::getline(fin, line))
    //     {
    //         char *start = (char *)line.c_str();
    //         if(start) {
    //             char *end = NULL;
    //             do {
    //                 number = ::strtof(start, &end);
    //                 if  ( start == end ) {
    //                     // MY_LOGD("No LDC data: %s", start);
    //                     break;
    //                 }
    //                 ldcTable.push_back(number);
    //                 start = end + 1;
    //             } while ( end && *end );
    //         }
    //     }

    //     fin.close();

    //     if(LOG_ENABLED) {
    //         if(ldcTable.size() > 3) {
    //             MY_LOGD("LDC table size %zu:", ldcTable.size());

    //             float *element = &ldcTable[0];
    //             for(int k = 0; k < 2; k++) {
    //                 int line = (int)*element++;
    //                 int sizePerLine = (int)*element++;
    //                 MY_LOGD(" %d %d", line, sizePerLine);
    //                 for(int i = 1; i <= line; i++) {
    //                     MY_LOGD("[%02d]% *.10f % *.10f ... % *.10f % *.10f %*d %*d",
    //                                     i,
    //                                     2, *element, 2, *(element+1),
    //                                     2, *(element+sizePerLine-2), 2, *(element+sizePerLine-1),
    //                                     4, (int)*(element+sizePerLine),
    //                                     2, (int)*(element+sizePerLine+1));
    //                     element += (sizePerLine+2);
    //                 }
    //             }
    //         } else {
    //             MY_LOGD("LDC: Empty");
    //         }
    //     }
    // }

protected:
    ENUM_STEREO_RATIO _imageRatio;

    N3D_HAL_KERNEL *_pN3DKernel = NULL;
    STEREO_KERNEL_SET_ENV_INFO_STRUCT _n3dInitInfo;
    STEREO_KERNEL_AF_INFO_STRUCT *_afInfoMain = NULL;
    STEREO_KERNEL_AF_INFO_STRUCT *_afInfoAuxi = NULL;

    N3D_HAL_INIT_PARAM_WPE     _initParam;
    N3D_HAL_PARAM_WPE          _previewParam;
    N3D_HAL_OUTPUT_WPE         _previewOutput;
    HWFEFM_DATA                _hwfefmData;

    unsigned int _ringIdx = 0;
    std::vector<sp<IImageBuffer>> _oututWarpMapMain2[3];

    sp<IImageBuffer> _inputImageMain1;
    sp<IImageBuffer> _outputImageMain1;
    sp<IImageBuffer> _outputMaskMain1;

    sp<IImageBuffer> _feoMain1[2];
    sp<IImageBuffer> _feoMain2[2];
    sp<IImageBuffer> _fmoLR[2];
    sp<IImageBuffer> _fmoRL[2];

    String8 _initPath;
    String8 _currentPath;

    MSize _feSize;
    MSize _fmSize;

    NVRAM_CAMERA_GEOMETRY_STRUCT _nvram;
    vector<float> _LDCTable;

    bool LOG_ENABLED = StereoSettingProvider::isLogEnabled("vendor.STEREO.log.hal.n3d.ut");
};

#endif