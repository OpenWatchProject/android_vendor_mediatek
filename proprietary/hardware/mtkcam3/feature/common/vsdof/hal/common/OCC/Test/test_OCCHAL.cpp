#define LOG_TAG "OCC_UT"

#include <limits.h>
#include <gtest/gtest.h>
#include <string.h>
#include <mtkcam3/feature/stereo/hal/stereo_common.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <occ_hal.h>
#include "../../inc/stereo_dp_util.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

#define OCC_UT_CASE_PATH            "/data/nativetest/VSDoF_HAL_Test/OCC_UT/"
#define OCC_UT_CASE_IN_PATH         OCC_UT_CASE_PATH"in"
#define OCC_UT_CASE_OUT_PATH        OCC_UT_CASE_PATH"out"
#define OCC_UT_CASE_HAL_PATH        OCC_UT_CASE_PATH"hal/"
#define OCC_UT_CASE_HAL_IN_PATH     OCC_UT_CASE_HAL_PATH"in"
#define OCC_UT_CASE_HAL_OUT_PATH    OCC_UT_CASE_HAL_PATH"out"
//=============================================================================
//  Test
//=============================================================================
TEST(HAL_TEST, OCCHAL_TEST)
{
    FILE *fp = NULL;
    const MSize IMAGE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_SV_Y);
    const MSize CROPPED_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_MY_S);
    int index = 0;
    char fileName[256];
    struct stat st;
    ::memset(&st, 0, sizeof(struct stat));
    size_t readSize;
    size_t expectSize;

    OCC_HAL *occ = OCC_HAL::createInstance();
    OCC_HAL_PARAMS occHalParam;
    occHalParam.disparityLeftToRight = new MUINT16[IMAGE_SIZE.w*IMAGE_SIZE.h];
    occHalParam.disparityRightToLEft = new MUINT16[IMAGE_SIZE.w*IMAGE_SIZE.h];
    occHalParam.ldcMain1 = new MUINT8[IMAGE_SIZE.w*IMAGE_SIZE.h];

    sp<IImageBuffer> imageMain1;
    sp<IImageBuffer> imageMain2;
    if(!StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YV12, IMAGE_SIZE, !IS_ALLOC_GB, imageMain1)) {
        MY_LOGE("Cannot alloc image 1");
        return;
    }
    occHalParam.imageMain1 = imageMain1.get();
    if(!StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YV12, IMAGE_SIZE, !IS_ALLOC_GB, imageMain2)) {
        MY_LOGE("Cannot alloc image 2");
        return;
    }
    occHalParam.imageMain2 = imageMain2.get();

    OCC_HAL_OUTPUT occHalOutput;
    MSize outputSize(CROPPED_SIZE.w, CROPPED_SIZE.h);
    sp<IImageBuffer> dsImage;
    if(!StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YV12, outputSize, !IS_ALLOC_GB, dsImage)) {
        MY_LOGE("Cannot alloc image ds");
        return;
    }
    occHalOutput.downScaledImg = dsImage.get();
    sp<IImageBuffer> depthMap;
    if(!StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, outputSize, !IS_ALLOC_GB, depthMap)) {
        MY_LOGE("Cannot alloc image depthMap");
        return;
    }
    occHalOutput.depthMap  = new MUINT8[outputSize.w * outputSize.h];

    MUINT8 *goldenImage    = new MUINT8[(int)(CROPPED_SIZE.w*CROPPED_SIZE.h*1.5f)];
    MUINT8 *goldenDepthMap = new MUINT8[CROPPED_SIZE.w*CROPPED_SIZE.h];

    //Read LDC
    sprintf(fileName, "%s/XVEC_N3D.raw", OCC_UT_CASE_IN_PATH);
    MY_LOGD("Read %s...\n", fileName);
    if(stat(fileName, &st) == -1) {
        MY_LOGE("error: %s", ::strerror(errno));
        EXPECT_TRUE(false);
        return;
    }
    fp = fopen(fileName, "rb");
    if(NULL == fp) {
        MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        EXPECT_TRUE(false);
        return;
    }
    expectSize = sizeof(MUINT8)*IMAGE_SIZE.w*IMAGE_SIZE.h;
    readSize = fread(occHalParam.ldcMain1, 1, expectSize, fp);
    fclose(fp);
    if(readSize != expectSize) {
        if(errno) {
            MY_LOGE("error: %s", ::strerror(errno));
        } else {
            MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
        }
        EXPECT_TRUE(false);
        return;
    }

    do {
        //Read DV_HW_L_xxxxx.raw
        sprintf(fileName, "%s/DV_HW_L_%05d.raw", OCC_UT_CASE_IN_PATH, index);
        if(stat(fileName, &st) == -1) {
            MY_LOGD("No more test cases, end test");
            break;
        }
        MY_LOGD("Read %s...", fileName);
        fp = fopen(fileName, "rb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
            break;
        }
        expectSize = sizeof(MUINT16)*IMAGE_SIZE.w*IMAGE_SIZE.h;
        readSize = fread(occHalParam.disparityLeftToRight, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read DV_HW_R_xxxxx.raw
        sprintf(fileName, "%s/DV_HW_R_%05d.raw", OCC_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*IMAGE_SIZE.w*IMAGE_SIZE.h;
        readSize = fread(occHalParam.disparityRightToLEft, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read m_pDown_L_xxxxx.yv12
        sprintf(fileName, "%s/m_pDown_L_%05d.yv12", OCC_UT_CASE_IN_PATH, index);
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
        if(0 == StereoSettingProvider::getSensorRelativePosition()) {
            readSize  = fread((void *)occHalParam.imageMain1->getBufVA(0), 1, expectSize,    fp);
            readSize += fread((void *)occHalParam.imageMain1->getBufVA(1), 1, expectSize>>2, fp);
            readSize += fread((void *)occHalParam.imageMain1->getBufVA(2), 1, expectSize>>2, fp);
        } else {
            readSize  = fread((void *)occHalParam.imageMain2->getBufVA(0), 1, expectSize,    fp);
            readSize += fread((void *)occHalParam.imageMain2->getBufVA(1), 1, expectSize>>2, fp);
            readSize += fread((void *)occHalParam.imageMain2->getBufVA(2), 1, expectSize>>2, fp);
        }
        fclose(fp);
        if(readSize != expectSize*1.5f) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }

        //Read m_pDown_R_xxxxx.yv12
        sprintf(fileName, "%s/m_pDown_R_%05d.yv12", OCC_UT_CASE_IN_PATH, index);
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
        if(0 == StereoSettingProvider::getSensorRelativePosition()) {
            readSize  = fread((void *)occHalParam.imageMain2->getBufVA(0), 1, expectSize,    fp);
            readSize += fread((void *)occHalParam.imageMain2->getBufVA(1), 1, expectSize>>2, fp);
            readSize += fread((void *)occHalParam.imageMain2->getBufVA(2), 1, expectSize>>2, fp);
        } else {
            readSize  = fread((void *)occHalParam.imageMain1->getBufVA(0), 1, expectSize,    fp);
            readSize += fread((void *)occHalParam.imageMain1->getBufVA(1), 1, expectSize>>2, fp);
            readSize += fread((void *)occHalParam.imageMain1->getBufVA(2), 1, expectSize>>2, fp);
        }
        fclose(fp);
        if(readSize != expectSize*1.5f) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }

        //Read goldens
        //Read DS_xxxxx.yv12
        sprintf(fileName, "%s/DS_%05d.yv12", OCC_UT_CASE_OUT_PATH, index);
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
        expectSize = sizeof(MUINT8)*CROPPED_SIZE.w*CROPPED_SIZE.h*1.5f;
        readSize = fread(goldenImage, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }

        //Read WMI_DS8_pre_xxxxx.raw
        sprintf(fileName, "%s/WMI_DS8_pre_%05d.raw", OCC_UT_CASE_OUT_PATH, index);
        MY_LOGD("Read %s...", fileName);
        if(stat(fileName, &st) == -1) {
            MY_LOGE("error: %s", ::strerror(errno));
            break;
        }
        fp = fopen(fileName, "rb");
        if(NULL == fp) {
            MY_LOGE("Cannot open3 %s(err: %s)", fileName, ::strerror(errno));
            break;
        }
        expectSize = sizeof(MUINT8)*CROPPED_SIZE.w*CROPPED_SIZE.h;
        readSize = fread(goldenDepthMap, 1, expectSize, fp);
        fclose(fp);
        expectSize = sizeof(MUINT8)*CROPPED_SIZE.w*CROPPED_SIZE.h;
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }
        // Dump input
        //Write result to buffer
        if(stat(OCC_UT_CASE_HAL_PATH, &st) == -1) {
            mkdir(OCC_UT_CASE_HAL_PATH, 0755);
        }

        if(stat(OCC_UT_CASE_HAL_IN_PATH, &st) == -1) {
            mkdir(OCC_UT_CASE_HAL_IN_PATH, 0755);
        }

        sprintf(fileName, "%s/imageMain1_%05d.yv12", OCC_UT_CASE_HAL_IN_PATH, index);
        MY_LOGD("Write %s...", fileName);
//        occHalParam.imageMain1->saveToFile(fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            expectSize = sizeof(MUINT8)*IMAGE_SIZE.w*IMAGE_SIZE.h;
            fwrite((void *)occHalParam.imageMain1->getBufVA(0), 1, expectSize,    fp);
            fwrite((void *)occHalParam.imageMain1->getBufVA(1), 1, expectSize>>2, fp);
            fwrite((void *)occHalParam.imageMain1->getBufVA(2), 1, expectSize>>2, fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/imageMain2_%05d.yv12", OCC_UT_CASE_HAL_IN_PATH, index);
        MY_LOGD("Write %s...", fileName);
//        occHalParam.imageMain2->saveToFile(fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            expectSize = sizeof(MUINT8)*IMAGE_SIZE.w*IMAGE_SIZE.h;
            fwrite((void *)occHalParam.imageMain2->getBufVA(0), 1, expectSize,    fp);
            fwrite((void *)occHalParam.imageMain2->getBufVA(1), 1, expectSize>>2, fp);
            fwrite((void *)occHalParam.imageMain2->getBufVA(2), 1, expectSize>>2, fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/disparityLeftToRight_%05d.raw", OCC_UT_CASE_HAL_IN_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            expectSize = sizeof(MUINT16)*IMAGE_SIZE.w*IMAGE_SIZE.h;
            fwrite((void *)occHalParam.disparityLeftToRight, 1, expectSize,    fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/disparityRightToLEft_%05d.raw", OCC_UT_CASE_HAL_IN_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            expectSize = sizeof(MUINT16)*IMAGE_SIZE.w*IMAGE_SIZE.h;
            fwrite((void *)occHalParam.disparityRightToLEft, 1, expectSize,    fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/ldcMain1_%05d.raw", OCC_UT_CASE_HAL_IN_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            expectSize = sizeof(MUINT8)*IMAGE_SIZE.w*IMAGE_SIZE.h;
            fwrite((void *)occHalParam.ldcMain1, 1, expectSize,    fp);
            fflush(fp);
            fclose(fp);
        }
        //
        occ->OCCHALRun(occHalParam, occHalOutput);
        //
        expectSize = sizeof(MUINT8)*CROPPED_SIZE.w*CROPPED_SIZE.h;
        MUINT8 *goldenPt = goldenImage;
        EXPECT_TRUE(memcmp((void *)occHalOutput.downScaledImg->getBufVA(0), goldenPt, expectSize) == 0);
        goldenPt += expectSize;
        EXPECT_TRUE(memcmp((void *)occHalOutput.downScaledImg->getBufVA(1), goldenPt, expectSize>>2) == 0);
        goldenPt += expectSize>>2;
        EXPECT_TRUE(memcmp((void *)occHalOutput.downScaledImg->getBufVA(2), goldenPt, expectSize>>2) == 0);
        EXPECT_TRUE(memcmp(occHalOutput.depthMap, goldenDepthMap, sizeof(MUINT8)*CROPPED_SIZE.w*CROPPED_SIZE.h) == 0);

        //Write result to buffer
        if(stat(OCC_UT_CASE_HAL_OUT_PATH, &st) == -1) {
            mkdir(OCC_UT_CASE_HAL_OUT_PATH, 0755);
        }

        sprintf(fileName, "%s/DS32_M_OCC_HAL_%05d.raw", OCC_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            expectSize = sizeof(MUINT8)*CROPPED_SIZE.w*CROPPED_SIZE.h;
            fwrite((void *)occHalOutput.downScaledImg->getBufVA(0), 1, expectSize,    fp);
            fwrite((void *)occHalOutput.downScaledImg->getBufVA(1), 1, expectSize>>2, fp);
            fwrite((void *)occHalOutput.downScaledImg->getBufVA(2), 1, expectSize>>2, fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/WMI_DS8_pre_OCC_HAL_%05d.raw", OCC_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            fwrite(occHalOutput.depthMap, 1, sizeof(MUINT8)*CROPPED_SIZE.w*CROPPED_SIZE.h, fp);
            fflush(fp);
            fclose(fp);
        }

        index++;
    } while(1);
}
