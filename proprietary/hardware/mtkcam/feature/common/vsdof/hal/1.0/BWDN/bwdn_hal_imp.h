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
#ifndef _BWDN_HAL_IMP_H_
#define _BWDN_HAL_IMP_H_

#include <bwdn_hal.h>
#include <libbwdn/MTKBWDN.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <vector>
#include <thread>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/StereoArea.h>
#include <mtkcam/feature/stereo/hal/FastLogger.h>

using namespace StereoHAL;
using namespace std;
using namespace android;
using namespace NSCam;

#define HAL_PIXEL_FORMAT_F16 0xDFC9CEA5 // MM requesting format FLOAT16
#define HAL_PIXEL_FORMAT_Y32 0xDFC9CEA6 // MM requesting format INT32
#define HAL_PIXEL_FORMAT_F32 0xDFC9CEA7 // MM requesting format FLOAT32

#define OB_OFFSET_SIZE      (4)
#define PRE_GAIN_SIZE       (3)
#define WARPING_MATRIX_SIZE (9)
#define DPADDING_SIZE       (2)
#define ALGO_PADDING        (32)

const size_t FLOAT_SIZE = sizeof(float);
const size_t BAYER_INPUT_SIZE_PER_PLANE = GAIN_IMAGE_SIZE*GAIN_IMAGE_SIZE*FLOAT_SIZE;

class BWDN_HAL_IMP : public BWDN_HAL
{
public:
    BWDN_HAL_IMP(BWDN_HAL_INIT *initParams = NULL);
    virtual ~BWDN_HAL_IMP();
    virtual void destroyInstance();

    virtual bool BWDNHALRun(BWDN_HAL_PARAMS &in, BWDN_HAL_OUTPUT &out);
protected:

private:
    void    __setParams(BWDN_HAL_PARAMS &in);
    void    __setImages(BWDN_HAL_PARAMS &in, BWDN_HAL_OUTPUT &out);
    void    __extractBayerGain(float* outGain, float* BayerGain, int plane, int gainWidth, int gainHeight);
    void    __extractMonoGain(float* outGain, float* MonoGain, int gainWidth, int gainHeight);
    inline void __simRotate90position(int inX, int inY, int Width, int& oriX, int& oriY);
    void    __fixShadingGain(float* padGain, float* inGain, int inW, int inH, int isRotate = 0);

    bool    __initBWDN(BWDN_HAL_INIT *initParams = NULL);
    bool    __initWorkingBuffer();
    void    __uninitWorkingBuffer();
    void    __initInputImageTemplate();
    void    __resetPerfService();

    void    __logInitData(bool initResult);
    void    __logParams(bool setParamsResult);
    void    __logImageDetails(const char *prefix, BWDNImage *image);
    void    __logImages();

    char    *__getDumpFolderName(int folderNumber, char path[]);
    void    __mkdir();  //make output dir for debug
    void    __dumpInputImage(BWDN_HAL_PARAMS &in);
    void    __dumpOutputImage(BWDN_HAL_OUTPUT &out);
    void    __dumpImage(char *fileName, void *data, size_t size);

    void    __waitInitThread();

    void    __lockAHardwareBuffer(void *pBuffer, void *&pVA){
                AHardwareBuffer_lock(
                    (AHardwareBuffer*)pBuffer,
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN|AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN,
                    -1, NULL, (void**)(&pVA)
                );
            }

    void    __unlockAHardwareBuffer(void *pBuffer){
                int fence = -1;
                AHardwareBuffer_unlock((AHardwareBuffer*)pBuffer, &fence);
            }

private:
    MTKBWDN                 *__pDrv;

    BWDNWorkingBufferInfo   __workingBufferInfo;
    BWDNInitParamsIF        __initParams;
    BWDNParamsIF            __runtimeParams;
    BWDNImageInfo           __runtimeImages;

    //Buffer holders
    int                 __initGBCount = 0;
    AHardwareBuffer*   __initGBs[25];
    AHardwareBuffer*   __bayerGain[PRE_GAIN_SIZE];
    AHardwareBuffer*   __monoGain;

    BWDNImage   __monoProcessedRawImage;
    BWDNImage   __bayerProcessedRawImage, __bayerWImage;
    BWDNImage   __depthImage;
    BWDNImage   __bayerGainImage, __monoGainImage;
    BWDNImage   __outputImage;

    const bool  DUMP_BUFFER;
    const bool  LOG_ENABLED;
    MINT32      __requestNumber;

    thread      __initThread;

    const bool IS_ROTATE = (eRotate_90  == StereoSettingProvider::getModuleRotation() ||
                            eRotate_270 == StereoSettingProvider::getModuleRotation());

    const StereoArea GAIN_AREA;

    FastLogger              __fastLogger;
};

#define BWDN_HAL_DEBUG

#ifdef BWDN_HAL_DEBUG    // Enable debug log.

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)

#endif  // BWDN_HAL_DEBUG

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNC_START MY_LOGD_IF(LOG_ENABLED, "+")
#define FUNC_END   MY_LOGD_IF(LOG_ENABLED, "-")

#define SINGLE_LINE_LOG 0

#endif