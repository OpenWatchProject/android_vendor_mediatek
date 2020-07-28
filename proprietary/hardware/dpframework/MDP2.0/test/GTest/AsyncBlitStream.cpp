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

#include "DpAsyncBlitStream.h"
#include "DpAsyncBlitStreamParam.h"
#include "DpSync.h"

//#include <tpipe_config.h>
#include <pthread.h>
#include <sync.h>
#include <sw_sync.h>
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

#define JOB_MAX_NUM 2

class AsyncBlitStream: public ::testing::TestWithParam<DpAsyncBlitStream_Param>{};
INSTANTIATE_TEST_CASE_P(AsyncBlitParam, AsyncBlitStream, testing::Values(
//                      srcWidth, srcHeight, srcColorFormat,            srcProfile,        srcField,                                                                                                                           srcCrop,      srcSecure, srcDoFlush,                rotate,          flip,    indexEnable,                 dstWidth,                dstHeight,                                                          dstColorFormat,                                                                                    dstProfile,                                                              dstField,                                                                                                                             dstROI,                                                         dstSecure,    dstDoFlush
#ifdef RDMA0_RSZ1_SRAM_SHARING
DpAsyncBlitStream_Param{     640,       480,    eYUV_422_3P,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0,  640,  480}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, DP_SECURE_NONE,          0, {   0,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  0, 0, 0}, {  320,    0,    0,   0}, {  240,    0,    0,   0}, {          eI420, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,  320,  240}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE}, { 0, 0, 0, 0}},
DpAsyncBlitStream_Param{     640,       480,    eYUV_422_3P,      DP_PROFILE_BT601, eInterlace_None, { DpRect{ 320,  120,  320,  240}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, DP_SECURE_NONE,          0, {   0,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  0, 0, 0}, {   64,    0,    0,   0}, {   48,    0,    0,   0}, {          eI420, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,   64,   48}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE}, { 0, 0, 0, 0}},
DpAsyncBlitStream_Param{     640,       480,    eYUV_422_3P,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0,  640,  480}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, DP_SECURE_NONE,          0, {  90,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  0, 0, 0}, {   48,    0,    0,   0}, {   64,    0,    0,   0}, {          eI420, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,   48,   64}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE}, { 0, 0, 0, 0}}
#else
DpAsyncBlitStream_Param{     640,       480,    eYUV_422_3P,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0,  640,  480}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, DP_SECURE_NONE,          0, {   0,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  0, 0, 0}, {  320,    0,    0,   0}, {  240,    0,    0,   0}, {          eI420, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,  320,  240}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE}, { 0, 0, 0, 0}},
DpAsyncBlitStream_Param{     640,       480,    eYUV_422_3P,      DP_PROFILE_BT601, eInterlace_None, { DpRect{ 320,  120,  320,  240}, DpRect{ 160,  120,  320,  240}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, DP_SECURE_NONE,          0, {   0,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  1, 0, 0}, {   64,  320,    0,   0}, {   48,  240,    0,   0}, {          eI420,            eI420, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,   64,   48}, DpRect{   0,    0,  320,  240}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE}, { 0, 0, 0, 0}},
DpAsyncBlitStream_Param{     640,       480,    eYUV_422_3P,      DP_PROFILE_BT601, eInterlace_None, { DpRect{   0,    0,  640,  480}, DpRect{   0,    0,  640,  480}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, DP_SECURE_NONE,          0, {  90,   0,   0,   0}, { 0, 0, 0, 0}, { 1,  1, 0, 0}, {   48,  320,    0,   0}, {   64,  240,    0,   0}, {          eI420,            eI420, DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN}, {      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601,      DP_PROFILE_BT601}, { eInterlace_None, eInterlace_None, eInterlace_None, eInterlace_None},  { DpRect{   0,    0,   48,   64}, DpRect{   0,    0,  320,  240}, DpRect{   0,    0,    0,    0}, DpRect{   0,    0,    0,    0}}, { DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE, DP_SECURE_NONE}, { 0, 0, 0, 0}}
#endif //RDMA0_RSZ1_SRAM_SHARING
));

void* consumerThread(void* para)
{
    int32_t* fenceFD = (int32_t*)para;

    // Wake up
    for (int jobIndex = 0 ; jobIndex < JOB_MAX_NUM ; jobIndex++)
    {
        sync_wait(*(fenceFD + jobIndex), -1);
        DPLOGD("%d frame is waited\n", jobIndex);
        close(*(fenceFD + jobIndex));
    }
    return NULL;
}


TEST_P(AsyncBlitStream, AsyncBlitStreamNormalTest)
{
    DpAsyncBlitStream_Param asyncBlitStreamParam = GetParam();
    DP_STATUS_ENUM              status;
    DpAsyncBlitStream           *pStream;
    uint32_t                    job[JOB_MAX_NUM];
    int32_t                     fence[JOB_MAX_NUM];
    pthread_t                   tid;
    void                        *pSource;
    void                        *pBuffer[3];
    void                        *pTarget[ISP_MAX_OUTPUT_PORT_NUM];
    void                        *pTargetBuffer[ISP_MAX_OUTPUT_PORT_NUM][3];
    uint32_t                    size[3];
    ion_user_handle_t ion_allocInHandle;
    ion_user_handle_t ion_allocOutHandle[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t         ionInputFD;
    int32_t         ionOutputFD[ISP_MAX_OUTPUT_PORT_NUM];

    pStream = new DpAsyncBlitStream();

    int ion_handle = ion_open();

    ion_alloc_mm(ion_handle, (asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight * 6), 0x200, 0, &ion_allocInHandle);

    ion_share(ion_handle, ion_allocInHandle, &ionInputFD);

    pSource = ion_mmap(ion_handle, 0, (asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionInputFD, 0);

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(asyncBlitStreamParam.indexEnable[index])
        {
            ion_alloc_mm(ion_handle, (asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index] * 6), 0x200, 0, &ion_allocOutHandle[index]);

            ion_share(ion_handle, ion_allocOutHandle[index], &ionOutputFD[index]);

            pTarget[index] = ion_mmap(ion_handle, 0, (asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index] * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionOutputFD[index], 0);
        }
    }

    for(int jobIndex = 0 ; jobIndex < JOB_MAX_NUM ; jobIndex++)
    {
        pStream->createJob(job[jobIndex], fence[jobIndex]);
    }

    for(int jobIndex = 0 ; jobIndex < JOB_MAX_NUM ; jobIndex++)
    {
        pStream->setConfigBegin(job[jobIndex]);

        // Setup buffer address
        pBuffer[0] = pSource;
        pBuffer[1] = ((uint8_t*)pBuffer[0] + (asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight * 4));
        pBuffer[2] = ((uint8_t*)pBuffer[1] + (asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight));

        // Setup buffer size
        size[0] = asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight * 4;
        size[1] = asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight;
        size[2] = asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight;

        status = pStream->setSrcBuffer(ionInputFD, size, DP_COLOR_GET_PLANE_COUNT(asyncBlitStreamParam.srcColorFormat));
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

        status = pStream->setSrcConfig(asyncBlitStreamParam.srcWidth,
                                       asyncBlitStreamParam.srcHeight,
                                       DP_COLOR_GET_MIN_Y_PITCH(asyncBlitStreamParam.srcColorFormat, asyncBlitStreamParam.srcWidth),
                                       DP_COLOR_GET_MIN_UV_PITCH(asyncBlitStreamParam.srcColorFormat, asyncBlitStreamParam.srcWidth),
                                       asyncBlitStreamParam.srcColorFormat,
                                       asyncBlitStreamParam.srcProfile,
                                       asyncBlitStreamParam.srcField,
                                       asyncBlitStreamParam.srcSecure,
                                       asyncBlitStreamParam.srcDoFlush);
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

        for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
        {
            if(asyncBlitStreamParam.indexEnable[index])
            {
                printf("job Index: %d, crop rotate flip Index: %d\n", jobIndex, index);

                status = pStream->setSrcCrop(index, asyncBlitStreamParam.srcCrop[index]);
                EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

                status = pStream->setRotate(index, asyncBlitStreamParam.rotate[index]);
                EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

                status = pStream->setFlip(index, asyncBlitStreamParam.flip[index]);
                EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

                printf("job Index: %d, setDstBuffer and setDstConfig Index: %d\n", jobIndex, index);

                // Setup buffer address
                pTargetBuffer[index][0] = pTarget[index];
                pTargetBuffer[index][1] = ((uint8_t*)pTargetBuffer[index][0] + (asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index] * 4));
                pTargetBuffer[index][2] = ((uint8_t*)pTargetBuffer[index][1] + (asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index]));

                // Setup buffer size
                size[0] = asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index] * 4;
                size[1] = asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index];
                size[2] = asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index];

                status = pStream->setDstBuffer(index, ionOutputFD[index], size, DP_COLOR_GET_PLANE_COUNT(asyncBlitStreamParam.dstColorFormat[index]));
                EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

                status = pStream->setDstConfig(index,
                                               asyncBlitStreamParam.dstWidth[index],
                                               asyncBlitStreamParam.dstHeight[index],
                                               DP_COLOR_GET_MIN_Y_PITCH(asyncBlitStreamParam.dstColorFormat[index], asyncBlitStreamParam.dstWidth[index]),
                                               DP_COLOR_GET_MIN_UV_PITCH(asyncBlitStreamParam.dstColorFormat[index], asyncBlitStreamParam.dstWidth[index]),
                                               asyncBlitStreamParam.dstColorFormat[index],
                                               asyncBlitStreamParam.dstProfile[index],
                                               asyncBlitStreamParam.dstField[index],
                                               &asyncBlitStreamParam.dstROI[index],
                                               asyncBlitStreamParam.dstSecure[index],
                                               asyncBlitStreamParam.dstDoFlush[index]);
                EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
            }
        }
        status = pStream->setConfigEnd();
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
    }

    pthread_create(&tid, NULL, consumerThread, &fence);

    for(int jobIndex = 0 ; jobIndex < JOB_MAX_NUM ; jobIndex++)
    {
        printf("job Index: %d, invalidate\n", jobIndex);
        status = pStream->invalidate();
        EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);
    }

    pthread_join(tid, NULL);

    if(NULL != pSource)
    {
        ion_munmap(ion_handle, pSource, (asyncBlitStreamParam.srcWidth * asyncBlitStreamParam.srcHeight * 6));
        ion_share_close(ion_handle, ionInputFD);
        ion_free(ion_handle, ion_allocInHandle);
        pSource = NULL;

    }

    for(int index = 0 ; index < ISP_MAX_OUTPUT_PORT_NUM ; index++)
    {
        if(asyncBlitStreamParam.indexEnable[index])
        {
            if(NULL != pTarget[index])
            {
                ion_munmap(ion_handle, pTarget[index], (asyncBlitStreamParam.dstWidth[index] * asyncBlitStreamParam.dstHeight[index] * 6));
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
//TEST(AsyncBlitStream, AsyncBlitStreamCase0)
//{
//
//}
