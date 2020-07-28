#include "DpWriteBin.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpChannel.h"
#include "DpDataType.h"
#include "DpTimer.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include "gtest/gtest.h"

#include "DpVEncStream.h"
#include "DpVEncStreamParam.h"
#include "DpTPipeParam.h"

#include <tpipe_config.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <ion.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>

#ifdef WIN32
#include"windows.h"
#define sleep(n) Sleep(n)
#endif

class VEncStream: public ::testing::TestWithParam<DpVEncStream_Param>{};
INSTANTIATE_TEST_CASE_P(VEncParam, VEncStream, testing::Values(
//                                              srcWidth, srcHeight, srcColorFormat,            srcProfile,        srcField,                                                                                                                           srcCrop, srcDoFlush,                rotate,          flip, VEncIndex,    indexEnable,                 dstWidth,                dstHeight,                                                          dstColorFormat,                                                                                    dstProfile,                                                              dstField,                                                                                                                             dstROI,    dstDoFlush, dstWaitBuffur
DpVEncStream_Param{DpVEncStream::ISP_ZSD_STREAM,    1920,      1088,  DP_COLOR_YUY2,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0, 1920, 1088}, DpRect{   0,    0, 1920, 1088}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}},          0, {   0,   0,   0,   0}, { 0, 0, 0, 0},         0, { 1,  1, 0, 0}, { 1920, 1280,    0,   0}, { 1088,  720,    0,   0}, {  DP_COLOR_I420,    DP_COLOR_YUYV, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0, 1920, 1088}, DpRect{   0,    0, 1280,  720}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { 0, 0, 0, 0}, { 1, 1, 0, 0}},
DpVEncStream_Param{DpVEncStream::ISP_ZSD_STREAM,     640,       480,     eYUV_422_I,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0,  640,  480}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}},          0, {   0,   0,   0,   0}, { 0, 0, 0, 0},         0, { 1,  0, 0, 0}, {  960,    0,    0,   0}, {  540,    0,    0,   0}, {  DP_COLOR_YUY2, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,  960,  540}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { 0, 0, 0, 0}, { 1, 0, 0, 0}}
));

TEST_P(VEncStream, VEncStreamNormalTest)
{
    DpVEncStream_Param vencStreamParam = GetParam();
    DP_STATUS_ENUM              status;
    DpVEncStream                *pStream;
    uint8_t                     *pSource;
    uint32_t                    pSourcePA;
    void                        *pBuffer[3];
    uint32_t                    pBufferPA[3];
    DpMemory                   *pISPSourceMem;
    uint8_t                     *pTarget[ISP_MAX_OUTPUT_PORT_NUM];
    uint32_t                    pTargetPA[ISP_MAX_OUTPUT_PORT_NUM];
    DpMemory                    *pISPTargetMem[ISP_MAX_OUTPUT_PORT_NUM];
    void                        *pTargetBuffer[ISP_MAX_OUTPUT_PORT_NUM][3];
    uint32_t                    pTargetBufferPA[ISP_MAX_OUTPUT_PORT_NUM][3];
    uint32_t                    size[3];
    DpTPipe                     *pTPipe;
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pTPipe = new DpTPipe();

    memset(&ISPInfo,0,sizeof(ISPInfo));

    pStream = new DpVEncStream(vencStreamParam.type);

    pISPSourceMem = DpMemory::Factory(DP_MEMORY_ION, -1, (vencStreamParam.srcWidth * vencStreamParam.srcHeight * 6), false);
    pSource = (uint8_t*) pISPSourceMem->mapSWAddress();
    pSourcePA = pISPSourceMem->mapHWAddress(tRDMA0,0);

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(vencStreamParam.indexEnable[index])
        {
            pISPTargetMem[index] = DpMemory::Factory(DP_MEMORY_ION, -1, (vencStreamParam.dstWidth[index] * vencStreamParam.dstHeight[index] * 6), false);
            pTarget[index] = (uint8_t*)pISPTargetMem[index]->mapSWAddress();
            pTargetPA[index] = pISPTargetMem[index]->mapHWAddress(tWROT0,0);
        }
    }

    uint32_t maxDstWidth = 0;
    uint32_t maxDstHeight = 0;
    uint32_t maxWidhtXHeight = 0;
    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(vencStreamParam.indexEnable[index])
        {
            if(maxWidhtXHeight < vencStreamParam.dstWidth[index] * vencStreamParam.dstHeight[index])
            {
                maxWidhtXHeight = vencStreamParam.dstWidth[index] * vencStreamParam.dstHeight[index];
                maxDstWidth = vencStreamParam.dstWidth[index];
                maxDstHeight = vencStreamParam.dstHeight[index];
            }
        }
    }

    status = pStream->startVideoRecord(maxDstWidth, maxDstHeight);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    // Setup buffer size
    size[0] = vencStreamParam.srcWidth * vencStreamParam.srcHeight * 4;
    size[1] = vencStreamParam.srcWidth * vencStreamParam.srcHeight;
    size[2] = vencStreamParam.srcWidth * vencStreamParam.srcHeight;

    pBuffer[0] = pSource;
    pBuffer[1] = pSource + size[0];
    pBuffer[2] = pSource + size[0] + size[1];

    pBufferPA[0] = pSourcePA;
    pBufferPA[1] = pSourcePA + size[0];
    pBufferPA[2] = pSourcePA + size[0] + size[1];

    status = pStream->queueSrcBuffer(pBuffer, pBufferPA, size, DP_COLOR_GET_PLANE_COUNT(vencStreamParam.srcColorFormat));
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->setSrcConfig(vencStreamParam.srcWidth,
                                   vencStreamParam.srcHeight,
                                   DP_COLOR_GET_MIN_Y_PITCH(vencStreamParam.srcColorFormat, vencStreamParam.srcWidth),
                                   DP_COLOR_GET_MIN_UV_PITCH(vencStreamParam.srcColorFormat, vencStreamParam.srcWidth),
                                   vencStreamParam.srcColorFormat,
                                   vencStreamParam.srcProfile,
                                   vencStreamParam.srcField,
                                   &vencStreamParam.srcCrop[0],
                                   vencStreamParam.srcDoFlush);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(vencStreamParam.indexEnable[index])
        {
            printf("crop rotate flip Index: %d\n", index);

            status = pStream->setSrcCrop(index,
                                         vencStreamParam.srcCrop[index].x,
                                         vencStreamParam.srcCrop[index].sub_x,
                                         vencStreamParam.srcCrop[index].y,
                                         vencStreamParam.srcCrop[index].sub_y,
                                         vencStreamParam.srcCrop[index].w,
                                         vencStreamParam.srcCrop[index].h);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            status = pStream->setRotation(index, vencStreamParam.rotate[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            status = pStream->setFlipStatus(index, vencStreamParam.flip[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            printf("queueDstBuffer and setDstConfig Index: %d\n", index);

            // Setup buffer size
            size[0] = vencStreamParam.dstWidth[index] * vencStreamParam.dstHeight[index] * 4;
            size[1] = vencStreamParam.dstWidth[index] * vencStreamParam.dstHeight[index];
            size[2] = vencStreamParam.dstWidth[index] * vencStreamParam.dstHeight[index];

            // Setup buffer address
            pTargetBuffer[index][0] = pTarget[index];
            pTargetBuffer[index][1] = pTarget[index] + size[0];
            pTargetBuffer[index][2] = pTarget[index] + size[0] + size[1];

            pTargetBufferPA[index][0] = pTargetPA[index];
            pTargetBufferPA[index][0] = pTargetPA[index] + size[0];
            pTargetBufferPA[index][0] = pTargetPA[index] + size[0] + size[1];

            status = pStream->queueDstBuffer(index, pTargetBuffer[index], pTargetBufferPA[index], size, DP_COLOR_GET_PLANE_COUNT(vencStreamParam.dstColorFormat[index]));
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            status = pStream->setDstConfig(index,
                                           vencStreamParam.dstWidth[index],
                                           vencStreamParam.dstHeight[index],
                                           DP_COLOR_GET_MIN_Y_PITCH(vencStreamParam.dstColorFormat[index], vencStreamParam.dstWidth[index]),
                                           DP_COLOR_GET_MIN_UV_PITCH(vencStreamParam.dstColorFormat[index], vencStreamParam.dstWidth[index]),
                                           vencStreamParam.dstColorFormat[index],
                                           vencStreamParam.dstProfile[index],
                                           vencStreamParam.dstField[index],
                                           &vencStreamParam.dstROI[index],
                                           vencStreamParam.dstDoFlush[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
        }
    }

    if(vencStreamParam.type != DpVEncStream::ISP_ZSD_STREAM)
    {
        status = pTPipe->getHardCodeTPipeInfo(&ISPInfo);
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
        status = pStream->setParameter(ISPInfo);
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
    }

    DpVEnc_Config VEnc_cfg;
    status = pStream->setPortType(vencStreamParam.VEncIndex, PORT_VENC, &VEnc_cfg);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->startStream();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(vencStreamParam.indexEnable[index])
        {
            printf("dequeueDstBuffer Index: %d\n", index);
            status = pStream->dequeueDstBuffer(index, pTargetBuffer[index], vencStreamParam.dstWaitBuffur[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
        }
    }

    status = pStream->dequeueSrcBuffer();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->stopStream();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->stopVideoRecord();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    if(NULL != pISPSourceMem)
    {
        delete pISPSourceMem;
        pISPSourceMem = NULL;
    }

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(vencStreamParam.indexEnable[index])
        {
            if(NULL != pISPTargetMem[index])
            {
                delete pISPTargetMem[index];
                pISPTargetMem[index] = NULL;
            }
        }
    }

    delete pStream;
}

//Add other test case here
//TEST(VEncStream, VEncStreamCase0)
//{
//
//}
