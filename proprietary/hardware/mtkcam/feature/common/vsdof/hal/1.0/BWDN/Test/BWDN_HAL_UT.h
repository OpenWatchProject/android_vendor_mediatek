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
#ifndef BWDN_HAL_UT_H_
#define BWDN_HAL_UT_H_

#include <limits.h>
#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>

#include <bwdn_hal.h>
#include "MTKBWDN.h"
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <ui/GraphicBuffer.h>
#include <ui/gralloc_extra.h>
#include <EGL/egl.h>

#include "../../inc/stereo_dp_util.h"

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;

#define UT_CASE_PATH            "/sdcard/BWDN_HAL_UT"
#define UT_CASE_IN_FOLDER       "in"
#define UT_CASE_GOLDEN_FOLDER   "golden"
#define UT_CASE_OUT_FOLDER      "out"
#define UT_CASE_CONFIG_NAME     "config.dat"

#define MY_LOGD(fmt, arg...)    if(LOG_ENABLED) { printf("[D][%s]" fmt"\n", __func__, ##arg); }
#define MY_LOGI(fmt, arg...)    if(LOG_ENABLED) { printf("[I][%s]" fmt"\n", __func__, ##arg); }
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

#define FUNC_START MY_LOGD("[%s] +", __FUNCTION__)
#define FUNC_END   MY_LOGD("[%s] -", __FUNCTION__)

#define HAL_PIXEL_FORMAT_F16 0xDFC9CEA5 // MM requesting format FLOAT16
#define HAL_PIXEL_FORMAT_Y32 0xDFC9CEA6 // MM requesting format INT32
#define HAL_PIXEL_FORMAT_F32 0xDFC9CEA7 // MM requesting format FLOAT32

const StereoArea GAIN_AREA(GAIN_IMAGE_SIZE+BWDN_SHADING_PADDING_SIZE, GAIN_IMAGE_SIZE+BWDN_SHADING_PADDING_SIZE,
                           BWDN_SHADING_PADDING_SIZE, BWDN_SHADING_PADDING_SIZE);
const size_t GAIN_SIZE_PER_PLANE = GAIN_AREA.size.w * GAIN_AREA.size.h;

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

enum BWDNBufferID
{
    MONO_PROCESS_RAW,
    BAYER_PROCESS_RAW,
    BAYER_W,
    DEPTH,
    BAYER_GAIN,
    MONO_GAIN,
    OUTPUT,
    GOLDEN,
    TOTAL_BUFFER_COUNT
};

static const char *BUFFER_NAME[TOTAL_BUFFER_COUNT] =
{
    "MONO_PROCESS_RAW",
    "BAYER_PROCESS_RAW",
    "BAYER_W",
    "DEPTH",
    "BAYER_GAIN",
    "MONO_GAIN",
    "OUTPUT",
    "GOLDEN",
};

class BWDNHALUTBase: public ::testing::Test
{
public:
    BWDNHALUTBase() {}
    virtual ~BWDNHALUTBase() {}

protected:
    virtual void SetUp() {
        if(!_isReadyToUT) {
            return;
        }

        char configPath[256];
        sprintf(configPath, "%s/%s/%s/%s", UT_CASE_PATH, getUTCaseName(), UT_CASE_IN_FOLDER, UT_CASE_CONFIG_NAME);
        readConfig(configPath);
        loadImages();

        MY_LOGD("[Create BWDN_HAL...]");
        _pBWDNHAL = BWDN_HAL::createInstance(&_init);
        MY_LOGD("BWDNHAL %p", _pBWDNHAL);
    }

    virtual void TearDown() {
        if(!_isReadyToUT) {
            return;
        }

        char outPath[256];
        sprintf(outPath, "%s/%s/%s/S_Output__%dx%d_14_0.raw", UT_CASE_PATH, getUTCaseName(), UT_CASE_OUT_FOLDER,
                bufferInfo[OUTPUT].i4Width, bufferInfo[OUTPUT].i4Height);
        writeRawImage(outPath, _output.outBuffer, bufferInfo[OUTPUT].i4Width * bufferInfo[OUTPUT].i4Height * sizeof(short));

        _pBWDNHAL->destroyInstance();
        releaseImages();
    }

    virtual ENUM_STEREO_RATIO           getImageRatio()     = 0;
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  = 0;
    virtual const char *                getUTCaseName()     = 0;
    virtual bool                        isReadyToUT()       { return isUTCaseExist(); }

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
            StereoSettingProvider::setStereoFeatureMode(E_STEREO_FEATURE_DENOISE);
            StereoSettingProvider::setStereoModuleType(BAYER_AND_MONO);
        }
    }

    bool readImage(const char *PATH, void *buffer, size_t size)
    {
        FILE * pFile = fopen ( PATH , "rb" );
        if (pFile == NULL)
        {
            MY_LOGE("Open %s fail", PATH);
            return false;
        }

        size_t read_size = fread (buffer, 1, size, pFile);
        if (read_size != size)
        {
            MY_LOGE("Read: %zu, expect: %zu", read_size, size);
        }

        // terminate
        fclose (pFile);
        return true;
    }

    void readConfig(const char *PATH)
    {
        MY_LOGD("Read config from %s...", PATH);
        std::ifstream infile(PATH, std::ios::binary);
        std::string line;

        while (std::getline(infile, line))
        {
            std::size_t found = line.find("=");
            if(found != std::string::npos)
            {
                std::string key = line.substr(0, found);
                std::string val = line.substr(found+1);
                if(key == "INIT")
                {
                    MY_LOGD("===== INIT =====");
                    parseInitParams(infile);
                }
                else if(key == "PARAMS")
                {
                    MY_LOGD("===== PARAMS =====");
                    parseParams(infile);
                }
                else if(key == "IMG")
                {
                    MY_LOGD("===== IMAGES =====");
                    parseImgInfo(infile);
                }
            }
        }

        ::memcpy(&bufferInfo[OUTPUT], &bufferInfo[GOLDEN], sizeof(BWDNImage));

        _input.isRotate = (bufferInfo[OUTPUT].i4Width < bufferInfo[OUTPUT].i4Height);
        if(_input.isRotate) {
            int rotate90Order[4] =  {2, 0, 3, 1};
            _input.bayerOrder = rotate90Order[_input.bayerOrder];
        }
    }

    void parseInitParams(std::ifstream& infile)
    {
        std::string line;
        while (std::getline(infile, line))
        {
            if(line == "end")
                break;

            std::size_t found = line.find("=");
            if(found != std::string::npos)
            {
                std::string key = line.substr(0, found);
                std::string val = line.substr(found+1);
                MY_LOGD("%s: %s", key.c_str(), val.c_str());

                if(key == "CoreNum")
                {
                    sscanf(val.c_str(),"%d",&_init.coreNum);
                }
                else if(key == "SplitSize")
                {
                    sscanf(val.c_str(),"%d",&_init.splitSize);
                }
                else if(key == "B2W")
                {
                    sscanf(val.c_str(),"%f,%f,%f",&_init.warpingMatrix[0],&_init.warpingMatrix[1],&_init.warpingMatrix[2]);
                }
                else if(key == "ACC")
                {
                    sscanf(val.c_str(),"%d",&_init.acc);
                }
                else
                {
                    MY_LOGD("Key=%s don't process", key.c_str());
                }
            }
        }
    }

    void parseParams(std::ifstream& infile)
    {
        std::string line;
        while (std::getline(infile, line))
        {
            if(line == "end")
                break;

            std::size_t found = line.find("=");
            if(found != std::string::npos)
            {
                std::string key = line.substr(0, found);
                std::string val = line.substr(found+1);
                MY_LOGD("%s: %s", key.c_str(), val.c_str());

                if(key == "OBOffsetBayer")
                {
                    sscanf(val.c_str(),"%d,%d,%d,%d",&_input.OBOffsetBayer[0], &_input.OBOffsetBayer[1],&_input.OBOffsetBayer[2], &_input.OBOffsetBayer[3]);
                }
                else if(key == "OBOffsetMono")
                {
                    sscanf(val.c_str(),"%d,%d,%d,%d",&_input.OBOffsetMono[0], &_input.OBOffsetMono[1],&_input.OBOffsetMono[2], &_input.OBOffsetMono[3]);
                }
                else if(key == "SensorGainBayer")
                {
                    sscanf(val.c_str(),"%d",&_input.sensorGainBayer);
                }
                else if(key == "SensorGainMono")
                {
                    sscanf(val.c_str(),"%d",&_input.sensorGainMono);
                }
                else if(key == "IspGainBayer")
                {
                    sscanf(val.c_str(),"%d",&_input.ispGainBayer);
                }
                else if(key == "IspGainMono")
                {
                    sscanf(val.c_str(),"%d", &_input.ispGainMono);
                }
                else if(key == "PreGainBayer")
                {
                    sscanf(val.c_str(),"%d,%d,%d",&_input.preGainBayer[0], &_input.preGainBayer[1], &_input.preGainBayer[2]);
                }
                /*else if(key == "IsRotate")
                {
                    sscanf(val.c_str(),"%d",&_input.IsRotate);
                }*/
                else if(key == "BayerOrder")
                {
                    sscanf(val.c_str(),"%d",&_input.bayerOrder);
                }
                else if(key == "RA")
                {
                    sscanf(val.c_str(),"%d",&_input.RA);
                }
                /*else if(key == "OffsetX")
                {
                    sscanf(val.c_str(),"%d",&_input.OffsetX);
                }
                else if(key == "OffsetY")
                {
                    sscanf(val.c_str(),"%d",&_input.OffsetY);
                }*/
                else if(key == "BWSingleRange")
                {
                    sscanf(val.c_str(),"%d",&_input.BW_SingleRange);
                }
                else if(key == "BWOccRange")
                {
                    sscanf(val.c_str(),"%d",&_input.BW_OccRange);
                }
                else if(key == "BWRange")
                {
                    sscanf(val.c_str(),"%d",&_input.BW_Range);
                }
                else if(key == "BWKernel")
                {
                    sscanf(val.c_str(),"%d",&_input.BW_Kernel);
                }
                else if(key == "BKernel")
                {
                    sscanf(val.c_str(),"%d",&_input.B_Kernel);
                }
                else if(key == "BRange")
                {
                    sscanf(val.c_str(),"%d",&_input.B_Range);
                }
                else if(key == "WKernel")
                {
                    sscanf(val.c_str(),"%d",&_input.W_Kernel);
                }
                else if(key == "WRange")
                {
                    sscanf(val.c_str(),"%d",&_input.W_Range);
                }
                else if(key == "VarianceModelScale")
                {
                    sscanf(val.c_str(),"%d",&_input.VSL);
                }
                else if(key == "VarianceModelOffset")
                {
                    sscanf(val.c_str(),"%d",&_input.VOFT);
                }
                else if(key == "VarianceGain")
                {
                    sscanf(val.c_str(),"%d",&_input.VGAIN);
                }
                else if(key == "AMatrix")
                {
                    int bufferSize = 0;
                    sscanf(val.c_str(),"%d",&bufferSize);
                    infile.read((char*)&(_input.Trans[0]), bufferSize);
                }
                else if(key == "WarpPadding")
                {
                    sscanf(val.c_str(),"%d,%d",&_input.dPadding[0],&_input.dPadding[1]);
                }
                else if(key == "FastPreGF")
                {
                    sscanf(val.c_str(),"%d",&_input.FPREPROC);
                }
                else if(key == "FullSkipStep")
                {
                    sscanf(val.c_str(),"%d",&_input.FSSTEP);
                }
                else if(key == "DblkRto")
                {
                    sscanf(val.c_str(),"%d",&_input.DblkRto);
                }
                else if(key == "DblkTH")
                {
                    sscanf(val.c_str(),"%d",&_input.DblkTH);
                }
                else if(key == "Qsch")
                {
                    sscanf(val.c_str(),"%d",&_input.QSch);
                }
                else if(key == "UnPack")
                {
                    sscanf(val.c_str(),"%d",&_input.isUnPack);
                }
                else
                {
                    MY_LOGD(" Key=%s don't process", key.c_str());
                }
            }
        }
    }

    void parseImgInfo(std::ifstream& infile)
    {
        BWDNImage* pImage = NULL;
        std::string line;
        while (std::getline(infile, line))
        {
            if(line == "end")
                break;

            std::size_t found = line.find("=");
            if(found != std::string::npos)
            {
                std::string key = line.substr(0, found);
                std::string val = line.substr(found+1);
                MY_LOGD("%s: %s", key.c_str(), val.c_str());

                pImage = NULL;
                if(key == "DepthInfo")
                {
                    sscanf(val.c_str(),"%d,%d", &_input.dsH, &_input.dsV);
                }
                else if(key == "MonoProcessedRaw")
                {
                    pImage = &bufferInfo[MONO_PROCESS_RAW];
                }
                else if(key == "BayerProcessedRaw")
                {
                    pImage = &bufferInfo[BAYER_PROCESS_RAW];
                }
                else if(key == "BayerW")
                {
                    pImage = &bufferInfo[BAYER_W];
                }
                else if(key == "Depth")
                {
                    pImage = &bufferInfo[DEPTH];
                }
                else if(key == "BayerGain")
                {
                    pImage = &bufferInfo[BAYER_GAIN];
                }
                else if(key == "MonoGain")
                {
                    pImage = &bufferInfo[MONO_GAIN];
                }
                else if(key == "Output")
                {
                    pImage = &bufferInfo[GOLDEN];
                }
                else
                {
                    MY_LOGD(" Key=%s don't process", key.c_str());
                }

                if(pImage) {
                    sscanf(val.c_str(),"%d,%d,%d,%d,%d,%d,%d,%d",
                                        &pImage->eMemType,&pImage->eImgFmt,
                                        &pImage->i4Width, &pImage->i4Height,
                                        &pImage->i4Offset, &pImage->i4Pitch,
                                        &pImage->i4Size, &pImage->i4PlaneNum);
                }
            }
        }
    }

    void lockGraphicBuffer(void *pBuffer, void *&pVA)
    {
        void* prVa;
        sp<GraphicBuffer> *pGB = (sp<GraphicBuffer> *)pBuffer;
        (*pGB)->lock(GRALLOC_USAGE_SW_WRITE_OFTEN|GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&pVA));
    }

    void unlockGraphicBuffer(void *pBuffer)
    {
        sp<GraphicBuffer> *pGB = (sp<GraphicBuffer> *)pBuffer;
        (*pGB)->unlock();
    }

    void loadImages()
    {
        MY_LOGD("[Load images...]");
        const char *bufferPaths[TOTAL_BUFFER_COUNT] =
        {
            UT_CASE_PATH"/%s/%s/MonoProcessed__%dx%d_12_0.raw",          // MONO_PROCESS_RAW
            UT_CASE_PATH"/%s/%s/BayerProcessed__%dx%d_12_0.raw",         // BAYER_PROCESS_RAW
            UT_CASE_PATH"/%s/%s/BayerW__%dx%d_8.raw",                    // BAYER_W
            UT_CASE_PATH"/%s/%s/Depth__%dx%d_16_1_%dx%d_16.raw",         // DEPTH
            UT_CASE_PATH"/%s/%s/BayerShadingGain__%dx%d_32_%d.float",    // BAYER_GAIN
            UT_CASE_PATH"/%s/%s/MonoShadingGain__%dx%d_32.float",        // MONO_GAIN
            "",                                                          // OUTPUT
            UT_CASE_PATH"/%s/%s/S_Output__%dx%d_14_0.raw"                // GOLDEN
        };

        const StereoArea DEPTH_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMP_H, eSTEREO_SCENARIO_CAPTURE);
        const MSize GB_SIZE[TOTAL_BUFFER_COUNT] =
        {
            // MONO_PROCESS_RAW
            MSize(bufferInfo[MONO_PROCESS_RAW].i4Width, bufferInfo[MONO_PROCESS_RAW].i4Height),
            // BAYER_PROCESS_RAW
            MSize(bufferInfo[BAYER_PROCESS_RAW].i4Pitch, bufferInfo[BAYER_PROCESS_RAW].i4Size/bufferInfo[BAYER_PROCESS_RAW].i4Pitch),
            // BAYER_W
            MSize(bufferInfo[BAYER_W].i4Pitch, bufferInfo[BAYER_W].i4Size/bufferInfo[BAYER_W].i4Pitch),
            // DEPTH
            DEPTH_SIZE.size,
            // BAYER_GAIN, not graphic buffer
            0,
            // MONO_GAIN, not graphic buffer
            0,
            // OUTPUT
            MSize(bufferInfo[OUTPUT].i4Pitch, bufferInfo[OUTPUT].i4Size/bufferInfo[OUTPUT].i4Pitch),
            // GOLDEN
            MSize(bufferInfo[GOLDEN].i4Pitch, bufferInfo[GOLDEN].i4Size/bufferInfo[GOLDEN].i4Pitch),
        };

        const MUINT32 bufferFormat[TOTAL_BUFFER_COUNT] =
        {
            HAL_PIXEL_FORMAT_Y16,   // MONO_PROCESS_RAW
            HAL_PIXEL_FORMAT_Y8,    // BAYER_PROCESS_RAW
            HAL_PIXEL_FORMAT_Y8,    // BAYER_W
            HAL_PIXEL_FORMAT_Y8,    // DEPTH
            0,                      // BAYER_GAIN
            0,                      // MONO_GAIN
            HAL_PIXEL_FORMAT_Y8,    // OUTPUT
            HAL_PIXEL_FORMAT_Y8,    // GOLDEN
        };

        const uint32_t GB_USAGE[TOTAL_BUFFER_COUNT] =
        {
            GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_HW_TEXTURE, // MONO_PROCESS_RAW
            GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_HW_RENDER,  // BAYER_PROCESS_RAW
            GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_HW_RENDER,  // BAYER_W
            GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_HW_RENDER,  // DEPTH
            0,                                                                                                          // BAYER_GAIN
            0,                                                                                                          // MONO_GAIN
            GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_HW_RENDER,  // OUTPUT
            GraphicBuffer::USAGE_SW_WRITE_OFTEN | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_HW_RENDER,  // GOLDEN
        };

        const size_t PIXEL_SIZE[TOTAL_BUFFER_COUNT] =
        {
            sizeof(short), // MONO_PROCESS_RAW
            sizeof(short), // BAYER_PROCESS_RAW
            sizeof(short), // BAYER_W
            sizeof(char),  // DEPTH
            sizeof(float), // BAYER_GAIN
            sizeof(float), // MONO_GAIN
            sizeof(short), // OUTPUT
            sizeof(short), // GOLDEN
        };

        // prepare input image
        eglInitialize(eglGetDisplay(EGL_DEFAULT_DISPLAY), NULL, NULL);

        char path[256];
        size_t imageSize = 0;
        size_t width, height;
        void *prVa;
        for(size_t i = 0; i < TOTAL_BUFFER_COUNT; ++i) {
            imageSize = bufferInfo[i].i4Width * bufferInfo[i].i4Height * PIXEL_SIZE[i];

            if(GOLDEN == i) {
                if(_golden) {
                    delete [] _golden;
                    _golden = NULL;
                }
                _golden = new(std::nothrow) unsigned short[bufferInfo[i].i4Width * bufferInfo[i].i4Height];
                sprintf(path, bufferPaths[i], getUTCaseName(), UT_CASE_GOLDEN_FOLDER,
                                bufferInfo[i].i4Width, bufferInfo[i].i4Height);
                MY_LOGD("Read %s (%s)", BUFFER_NAME[i], path);
                readImage(path, _golden, imageSize);
                continue;
            }

            if(BAYER_GAIN == i) {
                imageSize = GAIN_AREA.size.w*GAIN_AREA.size.h * sizeof(float);
                if(__bayerGainBuffer) {
                    delete [] __bayerGainBuffer;
                    __bayerGainBuffer = NULL;
                }
                __bayerGainBuffer = new float[GAIN_IMAGE_SIZE * GAIN_IMAGE_SIZE * 4 * bufferInfo[i].i4PlaneNum];

                float gainBuffer[GAIN_AREA.size.w*GAIN_AREA.size.h * bufferInfo[i].i4PlaneNum];
                for(MINT32 j = 0; j < bufferInfo[i].i4PlaneNum; ++j) {
                    sprintf(path, bufferPaths[i], getUTCaseName(), UT_CASE_IN_FOLDER,
                            bufferInfo[i].i4Width, bufferInfo[i].i4Height, j);
                    readImage(path, gainBuffer+j*GAIN_AREA.size.w*GAIN_AREA.size.h, imageSize);
                }

                //Expand RGB to BGGR
                float *srcR = gainBuffer;
                float *srcG = gainBuffer + GAIN_AREA.size.w*GAIN_AREA.size.h;
                float *srcB = gainBuffer + GAIN_AREA.size.w*GAIN_AREA.size.h*2;
                float *dst = __bayerGainBuffer;
                for(int h = 0; h < GAIN_IMAGE_SIZE; ++h) {
                    for(int w = 0; w < GAIN_IMAGE_SIZE; ++w) {
                        *dst++ = *(srcB+w);
                        *dst++ = *(srcG+w);
                        *dst++ = *(srcG+w);
                        *dst++ = *(srcR+w);
                    }

                    srcR += GAIN_AREA.size.w;
                    srcG += GAIN_AREA.size.w;
                    srcB += GAIN_AREA.size.w;
                }

                continue;
            }

            if(MONO_GAIN == i) {
                imageSize = GAIN_AREA.size.w*GAIN_AREA.size.h * sizeof(float);
                if(__monoGainBuffer) {
                    delete [] __monoGainBuffer;
                    __monoGainBuffer = NULL;
                }
                __monoGainBuffer = new float[GAIN_IMAGE_SIZE * GAIN_IMAGE_SIZE * 4];

                float gainBuffer[GAIN_AREA.size.w*GAIN_AREA.size.h];
                sprintf(path, bufferPaths[i], getUTCaseName(), UT_CASE_IN_FOLDER,
                        bufferInfo[i].i4Width, bufferInfo[i].i4Height);
                readImage(path, gainBuffer, imageSize);

                //Expand mono gain
                float *src = gainBuffer;
                float *dst = __monoGainBuffer;
                for(int h = 0; h < GAIN_IMAGE_SIZE; ++h, src += GAIN_AREA.size.w) {
                    for(int w = 0; w < GAIN_IMAGE_SIZE; ++w, dst+=4) {
                        *dst = *(src+w);
                    }
                }

                continue;
            }

            if(DEPTH == i) {
                size_t inputOffset = (DEPTH_SIZE.startPt.x + DEPTH_SIZE.startPt.y * DEPTH_SIZE.size.w) * 2;
                imageSize = DEPTH_SIZE.size.w * DEPTH_SIZE.contentSize().h * 2; //bufferInfo[i].i4Size;
                if(0 == _input.isUnPack) {
                    imageSize *= 2;
                    inputOffset *= 2;
                }

                _graphicBuffers[i][0] = new GraphicBuffer(GB_SIZE[i].w*2, GB_SIZE[i].h,
                                                          bufferFormat[i],
                                                          GB_USAGE[i]);
                bufferInfo[i].pvPlane[0] = (void *)&(_graphicBuffers[i][0]);

                lockGraphicBuffer(bufferInfo[i].pvPlane[0], prVa);

                sprintf(path, bufferPaths[i], getUTCaseName(), UT_CASE_IN_FOLDER,
                        bufferInfo[i].i4Width, bufferInfo[i].i4Height, DEPTH_SIZE.size.w, DEPTH_SIZE.size.h);
                MY_LOGD("Read %s (%s)", BUFFER_NAME[i], path);

                MY_LOGD("Depth Offset: %zu isUnPack %d, read size %zu", inputOffset, _input.isUnPack, imageSize);
                prVa = (MUINT8*)prVa + inputOffset;

                readImage(path, prVa, imageSize);
                unlockGraphicBuffer(bufferInfo[i].pvPlane[0]);
                continue;
            }

            for(MINT32 j = 0; j < bufferInfo[i].i4PlaneNum; ++j) {
                _graphicBuffers[i][j] = new GraphicBuffer(GB_SIZE[i].w, GB_SIZE[i].h,
                                                          bufferFormat[i],
                                                          GB_USAGE[i]);
                bufferInfo[i].pvPlane[j] = (void *)&(_graphicBuffers[i][j]);

                if(OUTPUT != i) {
                    lockGraphicBuffer(bufferInfo[i].pvPlane[j], prVa);

                    if(bufferInfo[i].i4PlaneNum == 1) {
                        sprintf(path, bufferPaths[i], getUTCaseName(), UT_CASE_IN_FOLDER,
                                bufferInfo[i].i4Width, bufferInfo[i].i4Height);
                        MY_LOGD("Read %s (%s)", BUFFER_NAME[i], path);
                    } else {
                        sprintf(path, bufferPaths[i], getUTCaseName(), UT_CASE_IN_FOLDER,
                                bufferInfo[i].i4Width, bufferInfo[i].i4Height, j);
                        MY_LOGD("Read %s[%d] (%s)", BUFFER_NAME[i], j, path);
                    }

                    readImage(path, prVa, imageSize);
                    unlockGraphicBuffer(bufferInfo[i].pvPlane[j]);
                }
            }
        }

        _input.monoProcessedRaw  = (sp<GraphicBuffer>*)bufferInfo[MONO_PROCESS_RAW].pvPlane[0];
        _input.bayerProcessedRaw = (sp<GraphicBuffer>*)bufferInfo[BAYER_PROCESS_RAW].pvPlane[0];
        _input.bayerW            = (sp<GraphicBuffer>*)bufferInfo[BAYER_W].pvPlane[0];
        _input.depth             = (sp<GraphicBuffer>*)bufferInfo[DEPTH].pvPlane[0];
        _input.bayerGain         = (float*)__bayerGainBuffer;
        _input.monoGain          = (float*)__monoGainBuffer;
        _output.outBuffer        = (sp<GraphicBuffer>*)bufferInfo[OUTPUT].pvPlane[0];
    }

    void releaseImages()
    {
        for(int i = 0; i < TOTAL_BUFFER_COUNT; ++i) {
            for(int j = 0; j < 3; ++j) {
                _graphicBuffers[i][j] = nullptr;
            }
        }

        if(__bayerGainBuffer) {
            delete [] __bayerGainBuffer;
            __bayerGainBuffer = NULL;
        }

        if(__monoGainBuffer) {
            delete [] __monoGainBuffer;
            __monoGainBuffer = NULL;
        }

        if(_golden) {
            delete [] _golden;
            _golden = NULL;
        }

        eglTerminate(eglGetDisplay(EGL_DEFAULT_DISPLAY));
    }

    void writeRawImage(char* path, void* buffer, size_t size)
    {
        void* prVa;
        lockGraphicBuffer(buffer, prVa);

        MY_LOGD("Write %s...", path);
        FILE * pFile = fopen ( path , "wb" );
        size_t wroteSize = fwrite (prVa, 1, size, pFile);
        fflush(pFile);
        fclose (pFile);

        if(wroteSize == size) {
            MY_LOGD("success");
        } else {
            MY_LOGD("failed, write size %zu, expect %zu", wroteSize, size);
        }

        unlockGraphicBuffer(buffer);
    }


    virtual bool isResultEmpty(void *output)
    {
        unsigned short *emptyBuffer = new unsigned short[bufferInfo[OUTPUT].i4Size];
        ::memset(emptyBuffer, 0, bufferInfo[OUTPUT].i4Size);
        int result = memcmp(output, emptyBuffer, bufferInfo[OUTPUT].i4Width*bufferInfo[OUTPUT].i4Height*sizeof(short));
        delete [] emptyBuffer;
        return (result == 0);
    }

    virtual bool isBitTrue(void *output)
    {
        return (memcmp(output, _golden, bufferInfo[OUTPUT].i4Width*bufferInfo[OUTPUT].i4Height*sizeof(short)) == 0);
    }

protected:
    //BWDN parameters
    BWDN_HAL *_pBWDNHAL = NULL;
    BWDN_HAL_INIT   _init;
    BWDN_HAL_PARAMS _input;
    BWDN_HAL_OUTPUT _output;

    unsigned short* _golden = NULL;

    //Buffer holders
    BWDNImage bufferInfo[TOTAL_BUFFER_COUNT];
    sp<GraphicBuffer> _graphicBuffers[TOTAL_BUFFER_COUNT][3];

    float *__bayerGainBuffer;   //200x200x3
    float *__monoGainBuffer;    //200x200

    bool _isReadyToUT;
    bool LOG_ENABLED = StereoSettingProvider::isLogEnabled("vendor.STEREO.log.hal.bwdn.ut");
};

#endif