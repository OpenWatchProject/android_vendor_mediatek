#include "DpWriteBin.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpChannel.h"
#include "DpDataType.h"
#include "DpTimer.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include "gtest/gtest.h"
#include "DpPlatform.h"

#include "DpIspStream.h"
#include "DpIspStreamParam.h"
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

class IspStream: public ::testing::TestWithParam<DpIspStream_Param>{};
INSTANTIATE_TEST_CASE_P(IspParam, IspStream, testing::Values(
//                                            srcWidth, srcHeight, srcColorFormat,            srcProfile,        srcField,                                                                                                                           srcCrop, srcDoFlush,                rotate,          flip,    indexEnable,                 dstWidth,                dstHeight,                                                          dstColorFormat,                                                                                    dstProfile,                                                              dstField,                                                                                                                             dstROI,    dstDoFlush, dstWaitBuffur
#ifdef RDMA0_RSZ1_SRAM_SHARING
DpIspStream_Param{DpIspStream::ISP_ZSD_STREAM,    4096,      2160,          eYUYV,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0, 4096, 2160}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}},          0, { 270,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  0, 0, 0}, { 1440,    0,    0,   0}, { 2560,    0,    0,   0}, {          eYUYV, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0, 1440, 2560}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { 0, 0, 0, 0}, { 1, 0, 0, 0}},
DpIspStream_Param{DpIspStream::ISP_ZSD_STREAM,     640,       480,     eYUV_422_I,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0,  640,  480}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}},          0, {   0,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  0, 0, 0}, {  960,    0,    0,   0}, {  540,    0,    0,   0}, {  DP_COLOR_YUY2, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,  960,  540}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { 0, 0, 0, 0}, { 1, 0, 0, 0}}
#else
DpIspStream_Param{DpIspStream::ISP_ZSD_STREAM,    4096,      2160,          eYUYV,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0, 4096, 2160}, DpRect{   0,    0, 4096, 2160}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}},          0, { 270,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  1, 0, 0}, { 1440, 3840,    0,   0}, { 2560, 2160,    0,   0}, {          eYUYV,            eYUYV, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0, 1440, 2560}, DpRect{   0,    0, 3840, 2160}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { 0, 0, 0, 0}, { 1, 1, 0, 0}},
DpIspStream_Param{DpIspStream::ISP_ZSD_STREAM,     640,       480,     eYUV_422_I,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0,  640,  480}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}},          0, {   0,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  0, 0, 0}, {  960,    0,    0,   0}, {  540,    0,    0,   0}, {  DP_COLOR_YUY2, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,  960,  540}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { 0, 0, 0, 0}, { 1, 0, 0, 0}}
#endif //RDMA0_RSZ1_SRAM_SHARING
));


TEST_P(IspStream, IspStreamNormalTest)
{
    DpIspStream_Param ispStreamParam = GetParam();
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    void                        *pSource;
    void                        *pTarget[ISP_MAX_OUTPUT_PORT_NUM];
    void                        *pBuffer[3];
    void                        *pTargetBuffer[ISP_MAX_OUTPUT_PORT_NUM][3];
    uint32_t                    size[3];
    DpTPipe                     *pTPipe;
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
    ion_user_handle_t           ion_allocInHandle;
    ion_user_handle_t           ion_allocOutHandle[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                     ionInputFD;
    int32_t                     ionOutputFD[ISP_MAX_OUTPUT_PORT_NUM];

    pTPipe = new DpTPipe();

    memset(&ISPInfo,0,sizeof(ISPInfo));

    pStream = new DpIspStream(ispStreamParam.type);

    int ion_handle = ion_open();

    ion_alloc_mm(ion_handle, (ispStreamParam.srcWidth * ispStreamParam.srcHeight * 6), 0x200, 0, &ion_allocInHandle);

    ion_share(ion_handle, ion_allocInHandle, &ionInputFD);

    pSource = ion_mmap(ion_handle, 0, (ispStreamParam.srcWidth * ispStreamParam.srcHeight * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionInputFD, 0);

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(ispStreamParam.indexEnable[index])
        {
            ion_alloc_mm(ion_handle, (ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index] * 6), 0x200, 0, &ion_allocOutHandle[index]);

            ion_share(ion_handle, ion_allocOutHandle[index], &ionOutputFD[index]);

            pTarget[index] = ion_mmap(ion_handle, 0, (ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index] * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionOutputFD[index], 0);
        }
    }

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = ((uint8_t*)pBuffer[0] + (ispStreamParam.srcWidth * ispStreamParam.srcHeight * 4));
    pBuffer[2] = ((uint8_t*)pBuffer[1] + (ispStreamParam.srcWidth * ispStreamParam.srcHeight));

    // Setup buffer size
    size[0] = ispStreamParam.srcWidth * ispStreamParam.srcHeight * 4;
    size[1] = ispStreamParam.srcWidth * ispStreamParam.srcHeight;
    size[2] = ispStreamParam.srcWidth * ispStreamParam.srcHeight;

    status = pStream->queueSrcBuffer(ionInputFD, size, DP_COLOR_GET_PLANE_COUNT(ispStreamParam.srcColorFormat));
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->setSrcConfig(ispStreamParam.srcWidth,
                                   ispStreamParam.srcHeight,
                                   DP_COLOR_GET_MIN_Y_PITCH(ispStreamParam.srcColorFormat, ispStreamParam.srcWidth),
                                   DP_COLOR_GET_MIN_UV_PITCH(ispStreamParam.srcColorFormat, ispStreamParam.srcWidth),
                                   ispStreamParam.srcColorFormat,
                                   ispStreamParam.srcProfile,
                                   ispStreamParam.srcField,
                                   &ispStreamParam.srcCrop[0],
                                   ispStreamParam.srcDoFlush);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(ispStreamParam.indexEnable[index])
        {
            printf("crop rotate flip Index: %d\n", index);

            status = pStream->setSrcCrop(index,
                                         ispStreamParam.srcCrop[index].x,
                                         ispStreamParam.srcCrop[index].sub_x,
                                         ispStreamParam.srcCrop[index].y,
                                         ispStreamParam.srcCrop[index].sub_y,
                                         ispStreamParam.srcCrop[index].w,
                                         ispStreamParam.srcCrop[index].h);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            status = pStream->setRotation(index, ispStreamParam.rotate[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            status = pStream->setFlipStatus(index, ispStreamParam.flip[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            printf("queueDstBuffer and setDstConfig Index: %d\n", index);

            // Setup buffer address
            pTargetBuffer[index][0] = pTarget[index];
            pTargetBuffer[index][1] = ((uint8_t*)pTargetBuffer[index][0] + (ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index] * 4));
            pTargetBuffer[index][2] = ((uint8_t*)pTargetBuffer[index][1] + (ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index]));

            // Setup buffer size
            size[0] = ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index] * 4;
            size[1] = ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index];
            size[2] = ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index];

            status = pStream->queueDstBuffer(index, ionOutputFD[index], size, DP_COLOR_GET_PLANE_COUNT(ispStreamParam.dstColorFormat[index]));
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

            status = pStream->setDstConfig(index,
                                           ispStreamParam.dstWidth[index],
                                           ispStreamParam.dstHeight[index],
                                           DP_COLOR_GET_MIN_Y_PITCH(ispStreamParam.dstColorFormat[index], ispStreamParam.dstWidth[index]),
                                           DP_COLOR_GET_MIN_UV_PITCH(ispStreamParam.dstColorFormat[index], ispStreamParam.dstWidth[index]),
                                           ispStreamParam.dstColorFormat[index],
                                           ispStreamParam.dstProfile[index],
                                           ispStreamParam.dstField[index],
                                           &ispStreamParam.dstROI[index],
                                           ispStreamParam.dstDoFlush[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
        }
    }

    if(ispStreamParam.type != DpIspStream::ISP_ZSD_STREAM)
    {
        status = pTPipe->getHardCodeTPipeInfo(&ISPInfo);
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
        status = pStream->setParameter(ISPInfo);
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
    }

    status = pStream->startStream();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(ispStreamParam.indexEnable[index])
        {
            printf("dequeueDstBuffer Index: %d\n", index);
            status = pStream->dequeueDstBuffer(index, pTargetBuffer[index], ispStreamParam.dstWaitBuffur[index]);
            EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
        }
    }

    status = pStream->dequeueSrcBuffer();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->stopStream();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    if(NULL != pSource)
    {
        ion_munmap(ion_handle, pSource, (ispStreamParam.srcWidth * ispStreamParam.srcHeight * 6));
        ion_share_close(ion_handle, ionInputFD);
        ion_free(ion_handle, ion_allocInHandle);
        pSource = NULL;
    }

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(ispStreamParam.indexEnable[index])
        {
            if(NULL != pTarget[index])
            {
                ion_munmap(ion_handle, pTarget[index], (ispStreamParam.dstWidth[index] * ispStreamParam.dstHeight[index] * 6));
                ion_share_close(ion_handle, ionOutputFD[index]);
                ion_free(ion_handle, ion_allocOutHandle[index]);
                pTarget[index] = NULL;
            }
        }
    }

    ion_close(ion_handle);

    delete pStream;
}

//Add other test case here
//TEST(IspStream, IspStreamCase0)
//{
//
//}
