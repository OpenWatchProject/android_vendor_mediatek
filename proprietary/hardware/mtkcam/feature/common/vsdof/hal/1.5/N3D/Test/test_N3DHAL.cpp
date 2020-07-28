#include <limits.h>
#include <gtest/gtest.h>
#include <string.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <n3d_hal.h>
#include "../../inc/stereo_dp_util.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

#define N3D_UT_CASE_PATH            "/data/nativetest/VSDoF_HAL_Test/N3D_UT/"
#define N3D_UT_CASE_IN_PATH         N3D_UT_CASE_PATH"in"
#define N3D_UT_CASE_OUT_PATH        N3D_UT_CASE_PATH"out"
#define N3D_UT_CASE_HAL_PATH        N3D_UT_CASE_PATH"hal/"
#define N3D_UT_CASE_HAL_OUT_PATH    N3D_UT_CASE_HAL_PATH"out"

using namespace StereoHAL;
//=============================================================================
//  Test
//=============================================================================
TEST(HAL_TEST, N3DHAL_PREVIEW_TEST)
{
    FILE *fp = NULL;
    int index = 0;
    char fileName[256];
    struct stat st;
    ::memset(&st, 0, sizeof(struct stat));
    size_t readSize;
    size_t expectSize;

    Pass2SizeInfo p2a2Info;
    StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_2, eSTEREO_SCENARIO_PREVIEW, p2a2Info);
    Pass2SizeInfo p2ap2Info;
    StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_PREVIEW, p2ap2Info);
    Pass2SizeInfo p2a3Info;
    StereoSizeProvider::getInstance()->getPass2SizeInfo(PASS2A_3, eSTEREO_SCENARIO_PREVIEW, p2a3Info);
    const MSize OUTPUT_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y);

    N3D_HAL *n3d = N3D_HAL::createInstance();
    N3D_HAL_INIT_PARAM n3dInitParam;
    n3dInitParam.eScenario = eSTEREO_SCENARIO_RECORD;
    n3dInitParam.fefmRound = 2;
    n3d->N3DHALInit(n3dInitParam);

    //Depthnode will create two N3D HAL instances, here simulate the sitsuation
    N3D_HAL *n3d2 = N3D_HAL::createInstance();
    N3D_HAL_INIT_PARAM n3dInitParam2;
    n3dInitParam2.eScenario = eSTEREO_SCENARIO_CAPTURE;
    n3dInitParam2.fefmRound = 2;
    n3d2->N3DHALInit(n3dInitParam2);

    N3D_HAL_PARAM n3dHalParam;
    const MUINT32 INPUT_IMG_SIZE = p2a2Info.areaWDMA.size.w * p2a2Info.areaWDMA.size.h*1.5f;    //YV12
    n3dHalParam.rectifyImgMain1 = new MUINT8[INPUT_IMG_SIZE];
    n3dHalParam.rectifyImgMain2 = new MUINT8[INPUT_IMG_SIZE];
    const MUINT32 CC_SIZE = p2a3Info.areaIMG2O.size.w * p2a3Info.areaIMG2O.size.h;
    n3dHalParam.ccImage[0] = new MUINT8[CC_SIZE];
    n3dHalParam.ccImage[1] = new MUINT8[CC_SIZE];
    n3dHalParam.magicNumber = 123;
    const MUINT32 FE_BLOCK_SIZE = StereoSettingProvider::fefmBlockSize(1);
    const MUINT32 FE_SIZE = 40*p2a2Info.areaFEO.size.w*p2a2Info.areaFEO.size.h/(FE_BLOCK_SIZE*FE_BLOCK_SIZE)/sizeof(MUINT16);
    const MUINT32 FM_SIZE = FE_SIZE/20;
    n3dHalParam.hwfefmData.geoDataMain1[0] = new MUINT16[FE_SIZE];
    n3dHalParam.hwfefmData.geoDataMain2[0] = new MUINT16[FE_SIZE];
    n3dHalParam.hwfefmData.geoDataLeftToRight[0] = new MUINT16[FM_SIZE];
    n3dHalParam.hwfefmData.geoDataRightToLeft[0] = new MUINT16[FM_SIZE];
    n3dHalParam.hwfefmData.geoDataMain1[1] = new MUINT16[FE_SIZE];
    n3dHalParam.hwfefmData.geoDataMain2[1] = new MUINT16[FE_SIZE];
    n3dHalParam.hwfefmData.geoDataLeftToRight[1] = new MUINT16[FM_SIZE];
    n3dHalParam.hwfefmData.geoDataRightToLeft[1] = new MUINT16[FM_SIZE];

    N3D_HAL_OUTPUT n3dHalOut;
    const MUINT32 OUTPUT_IMG_SIZE = OUTPUT_SIZE.w*OUTPUT_SIZE.h*1.5f;
    n3dHalOut.rectifyImgMain1 = new MUINT8[OUTPUT_IMG_SIZE];
    n3dHalOut.rectifyImgMain2 = new MUINT8[OUTPUT_IMG_SIZE];
    n3dHalOut.maskMain1 = new MUINT8[OUTPUT_SIZE.w*OUTPUT_SIZE.h];
    n3dHalOut.maskMain2 = new MUINT8[OUTPUT_SIZE.w*OUTPUT_SIZE.h];
    n3dHalOut.ldcMain1 = new MUINT8[OUTPUT_SIZE.w*OUTPUT_SIZE.h];

    N3D_HAL_OUTPUT n3dHalOutGolden;
    n3dHalOutGolden.rectifyImgMain1 = new MUINT8[OUTPUT_IMG_SIZE];
    n3dHalOutGolden.rectifyImgMain2 = new MUINT8[OUTPUT_IMG_SIZE];
    n3dHalOutGolden.maskMain1 = new MUINT8[OUTPUT_SIZE.w*OUTPUT_SIZE.h];
    n3dHalOutGolden.maskMain2 = new MUINT8[OUTPUT_SIZE.w*OUTPUT_SIZE.h];
    n3dHalOutGolden.ldcMain1 = new MUINT8[OUTPUT_SIZE.w*OUTPUT_SIZE.h];

    MY_LOGD("Run test");
    do {
        //=====================
        //  Read UT Inputs
        //=====================
        //Read fe_left0_x.bin
        sprintf(fileName, "%s/fe_left0_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FE_SIZE;
        readSize = ( 0 == StereoSettingProvider::getSensorRelativePosition() ) ?
                    fread(n3dHalParam.hwfefmData.geoDataMain1[0], 1, expectSize, fp) :
                    fread(n3dHalParam.hwfefmData.geoDataMain2[0], 1, expectSize, fp) ;
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read fe_left1_x.bin
        sprintf(fileName, "%s/fe_left1_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FE_SIZE;
        readSize = ( 0 == StereoSettingProvider::getSensorRelativePosition() ) ?
                    fread(n3dHalParam.hwfefmData.geoDataMain1[1], 1, expectSize, fp) :
                    fread(n3dHalParam.hwfefmData.geoDataMain2[1], 1, expectSize, fp) ;
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read fe_right0_x.bin
        sprintf(fileName, "%s/fe_right0_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FE_SIZE;
        readSize = ( 0 == StereoSettingProvider::getSensorRelativePosition() ) ?
                    fread(n3dHalParam.hwfefmData.geoDataMain2[0], 1, expectSize, fp) :
                    fread(n3dHalParam.hwfefmData.geoDataMain1[0], 1, expectSize, fp) ;
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read fe_right1_x.bin
        sprintf(fileName, "%s/fe_right1_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FE_SIZE;
        readSize = ( 0 == StereoSettingProvider::getSensorRelativePosition() ) ?
                    fread(n3dHalParam.hwfefmData.geoDataMain2[1], 1, expectSize, fp) :
                    fread(n3dHalParam.hwfefmData.geoDataMain1[1], 1, expectSize, fp) ;

        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read fm_ltor0_x.bin
        sprintf(fileName, "%s/fm_ltor0_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FM_SIZE;
        readSize = fread(n3dHalParam.hwfefmData.geoDataLeftToRight[0], 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read fm_ltor1_x.bin
        sprintf(fileName, "%s/fm_ltor1_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FM_SIZE;
        readSize = fread(n3dHalParam.hwfefmData.geoDataLeftToRight[1], 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read fm_rtol0_x.bin
        sprintf(fileName, "%s/fm_rtol0_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FM_SIZE;
        readSize = fread(n3dHalParam.hwfefmData.geoDataRightToLeft[0], 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read fm_rtol1_x.bin
        sprintf(fileName, "%s/fm_rtol1_%d.bin", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = sizeof(MUINT16)*FM_SIZE;
        readSize = fread(n3dHalParam.hwfefmData.geoDataRightToLeft[1], 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read pho_main_x.y
        sprintf(fileName, "%s/pho_main_%d.y", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = CC_SIZE;
        readSize = fread(n3dHalParam.ccImage[0], 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read pho_auxi_x.y
        sprintf(fileName, "%s/pho_auxi_%d.y", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = CC_SIZE;
        readSize = fread(n3dHalParam.ccImage[1], 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read Warp_main_src_x.yuv
        sprintf(fileName, "%s/Warp_main_src_%d.yuv", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = INPUT_IMG_SIZE;
        readSize = fread(n3dHalParam.rectifyImgMain1, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read Warp_auxi_src_x.yuv
        sprintf(fileName, "%s/Warp_auxi_src_%d.yuv", N3D_UT_CASE_IN_PATH, index);
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
        expectSize = INPUT_IMG_SIZE;
        readSize = fread(n3dHalParam.rectifyImgMain2, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read goldens
        //Read STEREO_IMG_2_272x144_1.ac.crop.k2.auxi.yuv
        sprintf(fileName, "%s/STEREO_IMG_2_272x144_%d.ac.crop.k2.auxi.yuv", N3D_UT_CASE_OUT_PATH, index+1);
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
        expectSize = OUTPUT_IMG_SIZE;
        readSize = fread(n3dHalOutGolden.rectifyImgMain2, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }

        //Read STEREO_IMG_2_272x144_1.ac.crop.k2.main.yuv
        sprintf(fileName, "%s/STEREO_IMG_2_272x144_%d.ac.crop.k2.main.yuv", N3D_UT_CASE_OUT_PATH, index+1);
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
        expectSize = OUTPUT_IMG_SIZE;
        readSize = fread(n3dHalOutGolden.rectifyImgMain1, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }

            break;
        }

        //Read STEREO_IMG_2_1920x1080_1.ldc.rgb
        sprintf(fileName, "%s/STEREO_IMG_2_1920x1080_%d.ldc.rgb", N3D_UT_CASE_OUT_PATH, index+1);
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
        expectSize = OUTPUT_SIZE.w * OUTPUT_SIZE.h;
        readSize = fread(n3dHalOutGolden.ldcMain1, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read STEREO_IMG_2_1920x1080_1.maskA.k2.rgb
        sprintf(fileName, "%s/STEREO_IMG_2_1920x1080_%d.maskA.k2.rgb", N3D_UT_CASE_OUT_PATH, index+1);
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
        expectSize = OUTPUT_SIZE.w * OUTPUT_SIZE.h;
        readSize = fread(n3dHalOutGolden.maskMain2, 1, expectSize, fp);
        fclose(fp);
        if(readSize != expectSize) {
            if(errno) {
                MY_LOGE("error: %s", ::strerror(errno));
            } else {
                MY_LOGE("File size does not match(%zd expected, %zd read)", expectSize, readSize);
            }
            break;
        }

        //Read STEREO_IMG_2_1920x1080_1.maskM.k2.rgb
        sprintf(fileName, "%s/STEREO_IMG_2_1920x1080_%d.maskM.k2.rgb", N3D_UT_CASE_OUT_PATH, index+1);
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
        expectSize = OUTPUT_SIZE.w * OUTPUT_SIZE.h;
        readSize = fread(n3dHalOutGolden.maskMain1, 1, expectSize, fp);
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
        n3d->N3DHALRun(n3dHalParam, n3dHalOut);
        EXPECT_TRUE(memcmp((void *)n3dHalOut.rectifyImgMain1, n3dHalOutGolden.rectifyImgMain1, OUTPUT_IMG_SIZE) == 0);
        EXPECT_TRUE(memcmp((void *)n3dHalOut.rectifyImgMain2, n3dHalOutGolden.rectifyImgMain2, OUTPUT_IMG_SIZE) == 0);
        EXPECT_TRUE(memcmp((void *)n3dHalOut.maskMain1, n3dHalOutGolden.maskMain1, sizeof(MUINT8)*OUTPUT_SIZE.w * OUTPUT_SIZE.h) == 0);
        EXPECT_TRUE(memcmp((void *)n3dHalOut.maskMain2, n3dHalOutGolden.maskMain2, sizeof(MUINT8)*OUTPUT_SIZE.w * OUTPUT_SIZE.h) == 0);
        EXPECT_TRUE(memcmp((void *)n3dHalOut.ldcMain1, n3dHalOutGolden.ldcMain1, sizeof(MUINT8)*OUTPUT_SIZE.w * OUTPUT_SIZE.h) == 0);

        //Write result to buffer
        if(stat(N3D_UT_CASE_HAL_PATH, &st) == -1) {
            mkdir(N3D_UT_CASE_HAL_PATH, 0755);
        }

        if(stat(N3D_UT_CASE_HAL_OUT_PATH, &st) == -1) {
            mkdir(N3D_UT_CASE_HAL_OUT_PATH, 0755);
        }
        sprintf(fileName, "%s/result_main1_%d.yuv", N3D_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            fwrite(n3dHalOut.rectifyImgMain1, 1, OUTPUT_IMG_SIZE, fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/result_main2_%d.yuv", N3D_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            fwrite(n3dHalOut.rectifyImgMain2, 1, OUTPUT_IMG_SIZE, fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/mask1_%d.y", N3D_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            fwrite(n3dHalOut.maskMain1, 1, sizeof(MUINT8)*OUTPUT_SIZE.w * OUTPUT_SIZE.h, fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/mask2_%d.y", N3D_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            fwrite(n3dHalOut.maskMain2, 1, sizeof(MUINT8)*OUTPUT_SIZE.w * OUTPUT_SIZE.h, fp);
            fflush(fp);
            fclose(fp);
        }

        sprintf(fileName, "%s/ldc_%d.y", N3D_UT_CASE_HAL_OUT_PATH, index);
        MY_LOGD("Write %s...", fileName);
        fp = fopen(fileName, "wb");
        if(NULL == fp) {
            MY_LOGE("Cannot open %s(err: %s)", fileName, ::strerror(errno));
        } else {
            fwrite(n3dHalOut.ldcMain1, 1, sizeof(MUINT8)*OUTPUT_SIZE.w * OUTPUT_SIZE.h, fp);
            fflush(fp);
            fclose(fp);
        }

        index++;
    } while(1);

    delete n3d;
    delete n3d2;
}
