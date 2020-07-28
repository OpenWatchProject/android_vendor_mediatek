#include <limits.h>
#include <gtest/gtest.h>
#include <string.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <gf_hal.h>
#include "../../inc/stereo_dp_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "GF_UT"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

#define GF_UT_CASE_PATH         "/data/nativetest/VSDoF_HAL_Test/GF_UT/"
#define GF_UT_CASE_IN_PATH      GF_UT_CASE_PATH"in"
#define GF_UT_CASE_OUT_PATH     GF_UT_CASE_PATH"out"
#define GF_UT_CASE_HAL_OUT_PATH GF_UT_CASE_OUT_PATH"/hal"

//=============================================================================
//  Test
//=============================================================================
TEST(HAL_TEST, GFHAL_TEST)
{
    FILE *fp = NULL;
    const MSize IMAGE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_MY_S);
    const MSize DEPTHMAP_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMH);
    const MSize DMBG_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMBG);
    int index = 0;
    char fileName[256];
    struct stat st;
    ::memset(&st, 0, sizeof(struct stat));
    size_t readSize;
    size_t expectSize;

    GF_HAL *gf = GF_HAL::createInstance();
    GF_HAL_IN_DATA gfHalInput;

    gfHalInput.isAFTriggered = true;
    gfHalInput.ptAF = MPoint(132, 76);
    gfHalInput.dofLevel = 16;

    sp<IImageBuffer> downScaledImg;
    if(!StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YV12, IMAGE_SIZE, !IS_ALLOC_GB, downScaledImg)) {
        MY_LOGE("Cannot alloc downScaledImg");
        return;
    }
    gfHalInput.downScaledImg = downScaledImg.get();
    gfHalInput.depthMap = new MUINT8[DEPTHMAP_SIZE.w * DEPTHMAP_SIZE.h];

    GF_HAL_OUT_DATA gfHalOutput;
    gfHalOutput.dmbg = new MUINT8[DMBG_SIZE.w * DMBG_SIZE.h];

    MUINT8 *goldenDMBG = new MUINT8[DMBG_SIZE.w * DMBG_SIZE.h];

    do {
        //Read DBuf_xxxxx.raw
        sprintf(fileName, "%s/DBuf_%05d.raw", GF_UT_CASE_IN_PATH, index);
        MY_LOGD("Read %s...", fileName);
        if(stat(fileName, &st) == -1) {
            MY_LOGD("No more test cases, end test");
            break;
        }
        fp = fopen(fileName, "rb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
            break;
        }
        expectSize = sizeof(MUINT8)*IMAGE_SIZE.w*IMAGE_SIZE.h;
        readSize = fread(gfHalInput.depthMap, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }

        //Read m_pDown_xxxxx.yv12
        sprintf(fileName, "%s/m_pDown_%05d.yv12", GF_UT_CASE_IN_PATH, index);
        MY_LOGD("Read %s...", fileName);
        if(stat(fileName, &st) == -1) {
            MY_LOGE("error: %s", ::strerror(errno));
            break;
        }
        fp = fopen(fileName, "rb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
            break;
        }

        expectSize = sizeof(MUINT8)*IMAGE_SIZE.w*IMAGE_SIZE.h;
        readSize  = fread((void *)gfHalInput.downScaledImg->getBufVA(0), 1, expectSize,    fp);
        readSize += fread((void *)gfHalInput.downScaledImg->getBufVA(1), 1, expectSize>>2, fp);
        readSize += fread((void *)gfHalInput.downScaledImg->getBufVA(2), 1, expectSize>>2, fp);
        fclose(fp);
        if(readSize != expectSize*1.5f) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }

        //Read golden
        //Read Bmap_GF_xxxxx.raw
        sprintf(fileName, "%s/Bmap_GF_%05d.raw", GF_UT_CASE_OUT_PATH, index);
        MY_LOGD("Read %s...", fileName);
        if(stat(fileName, &st) == -1) {
            MY_LOGE("error: %s", ::strerror(errno));
            break;
        }
        fp = fopen(fileName, "rb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
            break;
        }
        expectSize = sizeof(MUINT8)*DMBG_SIZE.w*DMBG_SIZE.h;
        readSize = fread(goldenDMBG, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }
        //
        gf->GFHALRun(gfHalInput, gfHalOutput);
        //Write result to buffer
        if(stat(GF_UT_CASE_HAL_OUT_PATH, &st) == -1) {
            mkdir(GF_UT_CASE_HAL_OUT_PATH, 0755);
        }
        sprintf(fileName, "%s/Bmap_GF_HAL_%05d.raw", GF_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            fwrite(gfHalOutput.dmbg, 1, sizeof(MUINT8)*DMBG_SIZE.w*DMBG_SIZE.h, fp);
            fflush(fp);
            fclose(fp);
        }

        EXPECT_TRUE(memcmp(gfHalOutput.dmbg, goldenDMBG, sizeof(MUINT8)*DMBG_SIZE.w*DMBG_SIZE.h) == 0);

        index++;
    } while(1);
}
