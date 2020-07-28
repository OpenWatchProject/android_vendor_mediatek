/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "test-hdrpoc"

#include <gtest/gtest.h>

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include <log/log.h>

#include <mtkcam/BuiltinTypes.h>
#include <mtkcam/feature/hdr/2.0/HDRProc2.h>
#include <mtkcam/feature/hdr/2.0/utils/ImageBufferUtils.h>

#include <camera_custom_AEPlinetable.h>

// camera3test_fixtures.h, hardware.h, camera3.h add for start camera3 preview
#include "camera3test_fixtures.h"
#include <hardware/hardware.h>
#include <hardware/camera3.h>

#define RUN_IN_CAM3_LOAD 1

using namespace android;
using namespace camera3;
using namespace tests;

// ---------------------------------------------------------------------------

namespace NSCam {

// ---------------------------------------------------------------------------

class HDRProcTest : public ::testing::Test
{
protected:
    HDRProcTest() {}

    virtual void SetUp()
    {
        const ::testing::TestInfo* const testInfo =
            ::testing::UnitTest::GetInstance()->current_test_info();
        ALOGD("begin test: %s.%s", testInfo->test_case_name(), testInfo->name());

        mHDRProcDone = MFALSE;

        mpCamera3Device = new android::camera3::tests::Camera3Device;
    }

    virtual void TearDown()
    {
        const ::testing::TestInfo* const testInfo =
            ::testing::UnitTest::GetInstance()->current_test_info();
        ALOGD("end test: %s.%s", testInfo->test_case_name(), testInfo->name());
    }

    uint32_t loadFileToBuf(char const*const fname, MUINT8*  buf, MUINT32 size);
    static MBOOL HDRProcCompleteCallback(MVOID* user,MBOOL ret);

protected:
    MBOOL    mHDRProcDone;

    android::camera3::tests::Camera3Device *mpCamera3Device;
};

// ---------------------------------------------------------------------------

}; // namespace NSCam

// ---------------------------------------------------------------------------

uint32_t HDRProcTest::loadFileToBuf(
        char const * const fname, MUINT8*  buf, MUINT32 size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    ALOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        ALOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    ALOGD("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                buf + readCnt,
                size - readCnt);
        if (nr < 0) {
            ALOGE("failed to read from file [%s]: %s",
                    fname, strerror(errno));
            break;
        }
        if (nr == 0) {
            ALOGE("can't read from file [%s]", fname);
            break;
        }
        readCnt += nr;
        cnt++;
    }
    ALOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}

MBOOL HDRProcTest::HDRProcCompleteCallback(MVOID* user,MBOOL ret)
{
    HDRProcTest *self = reinterpret_cast<HDRProcTest *>(user);
    if (NULL == self)
	{
        return MFALSE;
    }

    ALOGD("HDRProcCompleteCallback ret(%d)", ret);
    self->mHDRProcDone = ret;

    return MTRUE;
}

// ---------------------------------------------------------------------------

TEST_F(HDRProcTest, HDRProc)
{
    ALOGD("HDRProc +");

    // open ID specify which amera device is used
    MINT32 openID = 0;
    char szSrcFileName[6][100];

    HDRProc2& hdrProc(HDRProc2::getInstance());

    ASSERT_EQ(MTRUE, hdrProc.init(openID));

    // set callback
    hdrProc.setCompleteCallback(HDRProcCompleteCallback, this);

    // AE info should set before prepare, or we don't know how many HDR frame
    // will be processed.
    hdrProc.setParam(HDRProcParam_Set_AOEMode, 0, 0);
    hdrProc.setParam(HDRProcParam_Set_MaxSensorAnalogGain, 16384, 0);
    hdrProc.setParam(HDRProcParam_Set_MaxAEExpTimeInUS, 500000, 0);
    hdrProc.setParam(HDRProcParam_Set_MinAEExpTimeInUS, 500, 0);
    hdrProc.setParam(HDRProcParam_Set_ShutterLineTime, 13139, 0);
    hdrProc.setParam(HDRProcParam_Set_MaxAESensorGain, 131072, 0);
    hdrProc.setParam(HDRProcParam_Set_MinAESensorGain, 1195, 0);
    hdrProc.setParam(HDRProcParam_Set_ExpTimeInUS0EV, 29996, 0);
    hdrProc.setParam(HDRProcParam_Set_SensorGain0EV, 3712, 0);
    hdrProc.setParam(HDRProcParam_Set_FlareOffset0EV, 0, 0);
    hdrProc.setParam(HDRProcParam_Set_GainBase0EV, 0, 0);
    hdrProc.setParam(HDRProcParam_Set_LE_LowAvg, 0, 0);
    hdrProc.setParam(HDRProcParam_Set_SEDeltaEVx100, 0, 0);
    hdrProc.setParam(HDRProcParam_Set_DetectFace, 0, 0);

    MUINT32 u4Histogram[128] = {
        0, 382, 773, 361, 404, 347, 256, 164, 148, 212, 270, 390, 392, 504, 561,
        491, 370, 336, 279, 254, 234, 197, 177, 104, 140, 115, 96, 91, 74, 41,
        44, 36, 63, 38, 63, 42, 26, 29, 33, 19, 26, 24, 14, 20, 25, 8, 26, 16,
        33, 28, 22, 19, 19, 20, 17, 16, 14, 16, 13, 13, 18, 22, 24, 19, 13, 16,
        16, 9, 18, 18, 9, 16, 22, 14, 21, 19, 25, 16, 17, 31, 16, 29, 60, 75, 82,
        62, 56, 36, 33, 31, 43, 49, 37, 27, 25, 22, 27, 32, 51, 138, 201, 210, 139, 11, 0
    };

    hdrProc.setParam(
            HDRProcParam_Set_Histogram, reinterpret_cast<MUINTPTR>(u4Histogram), 0);

    MUINT32 u4FlareHistogram[128] = {
        0, 0, 4, 147, 349,
        407, 268, 196, 134, 209, 212, 156, 142, 133, 115, 89, 76, 47, 80, 81, 92,
        98, 131, 133, 154, 189, 179, 175, 188, 249, 251, 247, 280, 244, 245, 164,
        173, 159, 155, 149, 0, 0, 3, 190, 386, 445, 276, 157, 157, 227, 215, 165,
        140, 144, 83, 78, 61, 99, 91, 115, 133, 147, 185, 204, 196, 194, 216,
        301, 299, 275, 269, 206, 197, 182, 168, 168, 149, 138, 133, 125, 0, 0,
        67, 343, 465, 356, 209, 166, 200, 246, 199, 145, 135, 95, 88, 94, 85,
        111, 131, 178, 203, 230, 248, 251, 250, 321, 324, 231, 258, 249, 190,
        173, 153, 162, 167, 130, 131, 105, 136, 105, 0, 0, 0, 0, 0, 0, 0, 0
    };
    hdrProc.setParam(
            HDRProcParam_Set_FlareHistogram, reinterpret_cast<MUINTPTR>(u4FlareHistogram), 0);

    MINT32 aeTableCurrentIndex = 99;
    strAETable aePlineTable;
    strEvPline aeCurrentPlineTable;
    strEvPline plineTable = {
       {{96, 1216, 1048, 0, 0, 0},
        {106, 1136, 1080, 0, 0, 0},
        {106, 1216, 1064, 0, 0, 0},
        {117, 1216, 1064, 0, 0, 0},
        {128, 1216, 1032, 0, 0, 0},
        {138, 1136, 1088, 0, 0, 0},
        {138, 1216, 1080, 0, 0, 0},
        {159, 1136, 1088, 0, 0, 0},
        {170, 1136, 1096, 0, 0, 0},
        {180, 1216, 1040, 0, 0, 0},
        {191, 1216, 1056, 0, 0, 0},
        {202, 1216, 1040, 0, 0, 0},
        {223, 1136, 1088, 0, 0, 0},
        {233, 1216, 1040, 0, 0, 0},
        {255, 1136, 1088, 0, 0, 0},
        {265, 1216, 1048, 0, 0, 0},
        {286, 1216, 1040, 0, 0, 0},
        {318, 1136, 1080, 0, 0, 0},
        {339, 1136, 1080, 0, 0, 0},
        {360, 1136, 1096, 0, 0, 0},
        {382, 1216, 1024, 0, 0, 0},
        {413, 1136, 1096, 0, 0, 0},
        {445, 1136, 1088, 0, 0, 0},
        {477, 1136, 1088, 0, 0, 0},
        {509, 1136, 1096, 0, 0, 0},
        {540, 1216, 1024, 0, 0, 0},
        {583, 1136, 1096, 0, 0, 0},
        {625, 1136, 1096, 0, 0, 0},
        {678, 1136, 1080, 0, 0, 0},
        {720, 1136, 1096, 0, 0, 0},
        {773, 1136, 1088, 0, 0, 0},
        {826, 1136, 1096, 0, 0, 0},
        {890, 1136, 1088, 0, 0, 0},
        {953, 1136, 1088, 0, 0, 0},
        {1027, 1136, 1080, 0, 0, 0},
        {1101, 1136, 1088, 0, 0, 0},
        {1175, 1136, 1088, 0, 0, 0},
        {1260, 1136, 1088, 0, 0, 0},
        {1345, 1136, 1088, 0, 0, 0},
        {1451, 1136, 1080, 0, 0, 0},
        {1546, 1136, 1088, 0, 0, 0},
        {1662, 1136, 1088, 0, 0, 0},
        {1779, 1136, 1088, 0, 0, 0},
        {1916, 1136, 1088, 0, 0, 0},
        {2054, 1136, 1080, 0, 0, 0},
        {2213, 1136, 1080, 0, 0, 0},
        {2361, 1136, 1080, 0, 0, 0},
        {2530, 1136, 1080, 0, 0, 0},
        {2699, 1136, 1088, 0, 0, 0},
        {2911, 1136, 1080, 0, 0, 0},
        {3112, 1136, 1080, 0, 0, 0},
        {3334, 1136, 1080, 0, 0, 0},
        {3599, 1136, 1080, 0, 0, 0},
        {3853, 1136, 1080, 0, 0, 0},
        {4118, 1136, 1080, 0, 0, 0},
        {4414, 1136, 1080, 0, 0, 0},
        {4721, 1136, 1080, 0, 0, 0},
        {5060, 1136, 1080, 0, 0, 0},
        {5451, 1136, 1080, 0, 0, 0},
        {5832, 1136, 1080, 0, 0, 0},
        {6245, 1136, 1080, 0, 0, 0},
        {6690, 1136, 1080, 0, 0, 0},
        {7166, 1136, 1080, 0, 0, 0},
        {7674, 1136, 1080, 0, 0, 0},
        {8224, 1136, 1080, 0, 0, 0},
        {8806, 1136, 1080, 0, 0, 0},
        {9441, 1136, 1080, 0, 0, 0},
        {10003, 1136, 1096, 0, 0, 0},
        {10003, 1216, 1104, 0, 0, 0},
        {10003, 1328, 1072, 0, 0, 0},
        {10003, 1424, 1080, 0, 0, 0},
        {10003, 1536, 1064, 0, 0, 0},
        {10003, 1632, 1080, 0, 0, 0},
        {10003, 1840, 1024, 0, 0, 0},
        {10003, 1936, 1040, 0, 0, 0},
        {10003, 2048, 1056, 0, 0, 0},
        {10003, 2240, 1032, 0, 0, 0},
        {20005, 1136, 1096, 0, 0, 0},
        {20005, 1216, 1096, 0, 0, 0},
        {20005, 1328, 1072, 0, 0, 0},
        {20005, 1424, 1072, 0, 0, 0},
        {20005, 1536, 1072, 0, 0, 0},
        {20005, 1632, 1072, 0, 0, 0},
        {29996, 1216, 1032, 0, 0, 0},
        {29996, 1216, 1104, 0, 0, 0},
        {29996, 1328, 1080, 0, 0, 0},
        {29996, 1424, 1080, 0, 0, 0},
        {29996, 1536, 1080, 0, 0, 0},
        {29996, 1728, 1024, 0, 0, 0},
        {29996, 1840, 1040, 0, 0, 0},
        {29996, 1936, 1056, 0, 0, 0},
        {29996, 2048, 1064, 0, 0, 0},
        {29996, 2240, 1048, 0, 0, 0},
        {29996, 2448, 1024, 0, 0, 0},
        {29996, 2560, 1048, 0, 0, 0},
        {29996, 2752, 1040, 0, 0, 0},
        {29996, 2960, 1048, 0, 0, 0},
        {29996, 3200, 1032, 0, 0, 0},
        {29996, 3456, 1032, 0, 0, 0},
        {29996, 3712, 1024, 0, 0, 0},
        {29996, 3968, 1032, 0, 0, 0},
        {29996, 4224, 1032, 0, 0, 0},
        {29996, 4480, 1040, 0, 0, 0},
        {29996, 4864, 1032, 0, 0, 0},
        {29996, 5248, 1024, 0, 0, 0},
        {29996, 5632, 1024, 0, 0, 0},
        {29996, 6016, 1032, 0, 0, 0},
        {29996, 6384, 1040, 0, 0, 0},
        {29996, 6896, 1032, 0, 0, 0},
        {29996, 7472, 1024, 0, 0, 0},
        {29996, 7936, 1032, 0, 0, 0},
        {29996, 8448, 1040, 0, 0, 0},
        {29996, 9040, 1040, 0, 0, 0},
        {29996, 9696, 1048, 0, 0, 0},
        {29996, 10480, 1032, 0, 0, 0},
        {29996, 10912, 1064, 0, 0, 0},
        {29996, 11904, 1048, 0, 0, 0},
        {29996, 12480, 1072, 0, 0, 0},
        {29996, 13792, 1040, 0, 0, 0},
        {29996, 14560, 1048, 0, 0, 0},
        {29996, 15408, 1064, 0, 0, 0},
        {29996, 16384, 1072, 0, 0, 0},
        {29996, 16384, 1144, 0, 0, 0},
        {39997, 14560, 1040, 0, 0, 0},
        {39997, 15408, 1048, 0, 0, 0},
        {39997, 16384, 1064, 0, 0, 0},
        {49999, 14560, 1032, 0, 0, 0},
        {49999, 15408, 1032, 0, 0, 0},
        {49999, 16384, 1048, 0, 0, 0},
        {60002, 14560, 1056, 0, 0, 0},
        {60002, 15408, 1064, 0, 0, 0},
        {60002, 16384, 1072, 0, 0, 0},
        {60002, 16384, 1152, 0, 0, 0},
        {60002, 16384, 1240, 0, 0, 0},
        {60002, 16384, 1328, 0, 0, 0},
        {60002, 16384, 1424, 0, 0, 0},
        {60002, 16384, 1520, 0, 0, 0},
        {60002, 16384, 1632, 0, 0, 0},
        {60002, 16384, 1752, 0, 0, 0},
        {60002, 16384, 1872, 0, 0, 0},
        {60002, 16384, 2008, 0, 0, 0},
        {60002, 16384, 2152, 0, 0, 0},
        {60002, 16384, 2312, 0, 0, 0},
        {60002, 16384, 2472, 0, 0, 0},
        {60002, 16384, 2648, 0, 0, 0},
        {60002, 16384, 2840, 0, 0, 0},
        {60002, 16384, 3048, 0, 0, 0}}
    };

    memset(&aePlineTable, 0, sizeof(strAETable));
    memset(&aeCurrentPlineTable, 0, sizeof(strEvPline));
    memcpy(aeCurrentPlineTable.sPlineTable, &plineTable, sizeof(strEvPline));

    ALOGD("current pline table(%u, %u, %u, %u, %u, %u)",
        aeCurrentPlineTable.sPlineTable[0].u4Eposuretime,
        aeCurrentPlineTable.sPlineTable[0].u4AfeGain,
        aeCurrentPlineTable.sPlineTable[0].u4IspGain,
        aeCurrentPlineTable.sPlineTable[0].uIris,
        aeCurrentPlineTable.sPlineTable[0].uSensorMode,
        aeCurrentPlineTable.sPlineTable[0].uFlag);

    aePlineTable.u4TotalIndex = 147;
    aePlineTable.pCurrentTable = &aeCurrentPlineTable;

    aePlineTable.i4StrobeTrigerBV = 20;
    aePlineTable.i4MaxBV = 106;
    aePlineTable.i4MinBV = -40;
    aePlineTable.i4EffectiveMaxBV = 90;
    aePlineTable.i4EffectiveMinBV = -30;

    hdrProc.setParam(HDRProcParam_Set_PLineAETable,
            (MUINTPTR)(&aePlineTable), 99);

    // get HDR capture information from HDR proc
    MINT32 HDRFrameNum = 0;
    android::Vector < MUINT32 > vu4Eposuretime;
    android::Vector < MUINT32 > vu4SensorGain;
    android::Vector < MUINT32 > vu4FlareOffset;

    hdrProc.getHDRCapInfo(HDRFrameNum, vu4Eposuretime,vu4SensorGain,vu4FlareOffset);

    ALOGD("HDR input frames(%d)", HDRFrameNum);

    //  camera 3 preview start
#if RUN_IN_CAM3_LOAD
    if (mpCamera3Device == NULL)
    {
        ALOGE("mpCamera3Device is NULL");
    }
    else
    {
        ALOGD("mpCamera3Device SetUp");
        mpCamera3Device->SetUp();

        ALOGD("mpCamera3Device getNumOfCams:%d",mpCamera3Device->getNumOfCams());
        int id = 0;
        for (id = 0; id < mpCamera3Device->getNumOfCams(); id++)
        {
            if (!mpCamera3Device->isHal3Supported(id))
                break;
        }

        ALOGD("mpCamera3Device id:%d",id);
        mpCamera3Device->openCamera(0);
        //Camera init with callback
        ALOGD("mpCamera3Device init");
        mpCamera3Device->init();

        for (int i = CAMERA3_TEMPLATE_PREVIEW; i < CAMERA3_TEMPLATE_COUNT; i++)
        {
            const camera_metadata_t *request = /*mpCamera3Device->getCam3Device()*/mpCamera3Device->mDevice->ops->construct_default_request_settings(mpCamera3Device->mDevice, i);
            EXPECT_TRUE(request != NULL);
            EXPECT_LT((size_t)0, get_camera_metadata_entry_count(request));
            EXPECT_LT((size_t)0, get_camera_metadata_data_count(request));

            ALOGD("Template type %d:",i);
            dump_indented_camera_metadata(request, 0, 2, 4);
        }
    }
#endif
    // camera 3 preview end

    MUINT32 pic_w = 2560;
    MUINT32 pic_h = 1440;
    MUINT32 sensor_w = pic_w;
    MUINT32 sensor_h = pic_h;
    String8 pic_format = String8("jpeg");
    MUINT32 th_w = 160;
    MUINT32 th_h = 128;
    MUINT32 th_q = 90;
    MUINT32 rot = 0;
    MRect   cropRegion(pic_w, pic_h);
    //String8 sensor_capture_width = String8("4192");
    //String8 sensor_capture_height = String8("3104");


    MSize jpegSize(pic_w, pic_h);
    MSize thumbnailSize(th_w, th_h);
    hdrProc.setJpegParam(jpegSize, thumbnailSize);

    MSize pictureSize(pic_w, pic_h);
    MSize postviewSize(800, 600);
    hdrProc.setShotParam(pictureSize, postviewSize, cropRegion, rot);

    hdrProc.setParam(HDRProcParam_Set_sensor_size, sensor_w, sensor_h);

    hdrProc.setParam(HDRProcParam_Set_sensor_type, 1/*NSCam::SENSOR_TYPE_RAW*/, 0);

    // should input 3A information and get cap info before prepare.
    hdrProc.prepare();


    MUINT32 uSrcMainFormat = 0;
    MUINT32 uSrcMainWidth = 0;
    MUINT32 uSrcMainHeight = 0;

    MUINT32 uSrcSmallFormat = 0;
    MUINT32 uSrcSmallWidth = 0;
    MUINT32 uSrcSmallHeight = 0;
    MUINT32 empty = 0;

    hdrProc.getParam(HDRProcParam_Get_src_main_format, uSrcMainFormat, empty);
    hdrProc.getParam(HDRProcParam_Get_src_main_size, uSrcMainWidth, uSrcMainHeight);
    hdrProc.getParam(HDRProcParam_Get_src_small_format, uSrcSmallFormat, empty);
    hdrProc.getParam(HDRProcParam_Get_src_small_size, uSrcSmallWidth, uSrcSmallHeight);

    MUINT32 u4SurfaceIndex[6];
    EImageFormat InputImageFormat[6];
    MUINT32 InputImageWidth[6] ;
    MUINT32 InputImageHeight[6] ;

    for (MINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        u4SurfaceIndex[i]     =  i;
        if((i%2) == 0){
            InputImageFormat[i] = (EImageFormat)uSrcMainFormat;
            InputImageWidth[i]  =  uSrcMainWidth;
            InputImageHeight[i] =  uSrcMainHeight;
        } else {
            InputImageFormat[i] = (EImageFormat)uSrcSmallFormat;
            InputImageWidth[i]  =  uSrcSmallWidth;
            InputImageHeight[i] =  uSrcSmallHeight;
        }
        ALOGD("Surface[%d] Format[%d] size (%dX%d)",u4SurfaceIndex[i],InputImageFormat[i],InputImageWidth[i],InputImageHeight[i]);
    }

    sp<IImageBuffer> SrcImgBuffer[6];
    EImageFormat DstimageFormat = eImgFmt_YUY2;

    sp<IImageBuffer> DstImgBuffer;
    MUINT32 DstimageWidth = pic_w;
    MUINT32 DstimageHeight = pic_h;

    sp<IImageBuffer> DstImg_TB_Buffer;
    EImageFormat Dstimage_TB_Format = eImgFmt_Y800;
    MUINT32 Dstimage_TB_Width = 160;
    MUINT32 Dstimage_TB_Height = 128;

    ALOGD("allocate source buffer");

    for (MINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        ALOGD("allocBuffer [%d] , InputImageWidth[%d], InputImageHeight[%d], InputImageFormat[%d]",i, InputImageWidth[i], InputImageHeight[i], InputImageFormat[i]);

        ImageBufferUtils::getInstance().allocBuffer(SrcImgBuffer[i], InputImageWidth[i], InputImageHeight[i], InputImageFormat[i]);
        ASSERT_NE((void*)NULL, SrcImgBuffer[i].get());
        if((i%2) == 0){
            sprintf(szSrcFileName[i], "/data/input/mpSourceImgBuf[%d]_%dx%d.i420",i,InputImageWidth[i],InputImageHeight[i]);
        } else {
            sprintf(szSrcFileName[i], "/data/input/mpSourceImgBuf[%d]_%dx%d.y",i,InputImageWidth[i],InputImageHeight[i]);
        }

    }

    // fill src buffer
    for (MINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        if((i%2) == 0){
            loadFileToBuf(szSrcFileName[i],(MUINT8*) SrcImgBuffer[i]->getBufVA(0),SrcImgBuffer[i]->getBufSizeInBytes(0)+SrcImgBuffer[i]->getBufSizeInBytes(1)+SrcImgBuffer[i]->getBufSizeInBytes(2));
        }else{
            loadFileToBuf(szSrcFileName[i],(MUINT8*) SrcImgBuffer[i]->getBufVA(0),SrcImgBuffer[i]->getBufSizeInBytes(0));
        }
    }

    ALOGD("allocate dst buffer");
    ImageBufferUtils::getInstance().allocBuffer(DstImgBuffer, DstimageWidth, DstimageHeight, DstimageFormat);

    ALOGD("allocate dst tb buffer");
    ImageBufferUtils::getInstance().allocBuffer(DstImg_TB_Buffer, Dstimage_TB_Width, Dstimage_TB_Height, Dstimage_TB_Format);

    ASSERT_NE((void*)NULL, DstImgBuffer.get());
    ASSERT_NE((void*)NULL, DstImg_TB_Buffer.get());
    //
    // TODO: should set output format in parameter

#if RUN_IN_CAM3_LOAD
    if (mpCamera3Device)
    {
        ALOGD("closeCamera");
        mpCamera3Device->closeCamera();
        //ALOGD("TearDown");
        //mpCamera3Device->TearDown();
    }
#endif

    ALOGD("mpHdrProc start");
    ASSERT_EQ(MTRUE, hdrProc.start());

    ALOGD("mpHdrProc addOutputFrame (VA)addr:%p", DstImgBuffer->getBufVA(0));
    ASSERT_EQ(MTRUE, hdrProc.addOutputFrame(HDR_OUTPUT_JPEG_YUV ,DstImgBuffer));
    ALOGD("mpHdrProc addOutputFrame (VA)addr:%p", DstImg_TB_Buffer->getBufVA(0));
    ASSERT_EQ(MTRUE, hdrProc.addOutputFrame(HDR_OUTPUT_JPEG_THUMBNAIL_YUV ,DstImg_TB_Buffer));

#if 1  // TEST

    ALOGD("mpHdrProc addInputFrame");

    for (MINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        ASSERT_EQ(MTRUE, hdrProc.addInputFrame(i,SrcImgBuffer[i]));
    }
#endif

    while(MFALSE == mHDRProcDone)
    {
        sleep(1);
        ALOGD("wait for HDR process done");
    }
    ALOGD("!!! HDR process done");

    char szResultFileName[100];

    ::sprintf(szResultFileName, "/sdcard/0000_10_DstImgBuffer_%dx%d.yuy2",DstimageWidth, DstimageHeight);
    DstImgBuffer->saveToFile(szResultFileName);
    ::sprintf(szResultFileName, "/sdcard/0000_10_DstImg_TB_Buffer_%dx%d.y", Dstimage_TB_Width, Dstimage_TB_Height);
    DstImg_TB_Buffer->saveToFile(szResultFileName);

    hdrProc.release();
    hdrProc.uninit();

#if 1 //TODO, should free
    ALOGD("deallocBuffer DstImgBuffer");
    for (MINT32 i = 0; i < HDRFrameNum; i++)
    {
        ImageBufferUtils::getInstance().deallocBuffer(SrcImgBuffer[i]);
    }
    ALOGD("deallocBuffer DstImgBuffer");
    ImageBufferUtils::getInstance().deallocBuffer(DstImgBuffer);
    ALOGD("deallocBuffer DstImg_TB_Buffer");
    ImageBufferUtils::getInstance().deallocBuffer(DstImg_TB_Buffer);
#endif

    ALOGD("HDRProc -");
} // TEST_F(HDRProcTest, HDRProc)
