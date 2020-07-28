#include "DpWriteBin.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpChannel.h"
#include "DpDataType.h"
#include "DpTimer.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include "gtest/gtest.h"

#include "DpFragStream.h"
#include "DpFragStreamParam.h"

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


class FragStream: public ::testing::TestWithParam<DpFragStream_Param>{};

INSTANTIATE_TEST_CASE_P(FragParam, FragStream, testing::Values(
//                 srcWidth, srcHeight, srcColorFormat, MCUXSize, MCUYSize,                        srcCrop, rotate, flip, dstWidth, dstHeight, dstColorFormat,                         dstROI, waitBuf
DpFragStream_Param{     640,       480,          eI420,       16,       16, DpRect{   0,    0,  640,  480},      0,    0,      320,       240,        eRGB888, DpRect{   0,    0,  320,  240},   false},
DpFragStream_Param{     640,       480,          eI422,       16,        8, DpRect{ 161,  121,  339,  279},      0,    0,      640,       480,      eRGBX8888, DpRect{   0,    0,  640,  480},   false},
DpFragStream_Param{     640,       480,          eI444,       16,        8, DpRect{ 131,  158,  327,  241},      0,    0,      640,       480,          eYV12, DpRect{   0,    0,  640,  480},   false}
));



TEST_P(FragStream, FragStreamNormalTest)
{
    DpFragStream_Param fragStreamParam = GetParam();
    DP_STATUS_ENUM status;
    DpFragStream   *pStream;
    DpRect         tmpRect;
    void           *pSource;
    void           *pTarget;
    int32_t        bufID;
    DpColorFormat  format;
    void           *pBuffer[3];
    uint32_t       size[3];
    int32_t        mcuXStart;
    int32_t        mcuYStart;
    int32_t        width;
    int32_t        height;
    int32_t        pitch;
    int32_t        index;
    uint8_t        *pSourceFrag;
    uint8_t        *pTargetFrag;
    uint32_t       mcuYCount;
    ion_user_handle_t ion_allocInHandle;
    ion_user_handle_t ion_allocOutHandle;
    int32_t           ionInputFD;
    int32_t           ionOutputFD;

    pStream = new DpFragStream();

    status = pStream->setSrcConfig(fragStreamParam.srcColorFormat,
                                   fragStreamParam.srcWidth,
                                   fragStreamParam.srcHeight,
                                   fragStreamParam.MCUXSize,
                                   fragStreamParam.MCUYSize,
                                   DP_COLOR_GET_MIN_Y_PITCH(fragStreamParam.srcColorFormat, fragStreamParam.srcWidth),
                                   &fragStreamParam.srcCrop);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    int ion_handle = ion_open();

    ion_alloc_mm(ion_handle, (fragStreamParam.dstWidth * fragStreamParam.dstHeight * 6), 0x200, 0, &ion_allocOutHandle);

    ion_share(ion_handle, ion_allocOutHandle, &ionOutputFD);

    pTarget = ion_mmap(ion_handle, 0, (fragStreamParam.dstWidth * fragStreamParam.dstHeight * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionOutputFD, 0);

    // Setup buffer address
    pBuffer[0] = pTarget;
    pBuffer[1] = ((uint8_t*)pBuffer[0] + (fragStreamParam.dstWidth * fragStreamParam.dstHeight * 4));
    pBuffer[2] = ((uint8_t*)pBuffer[1] + (fragStreamParam.dstWidth * fragStreamParam.dstHeight));


    // Setup buffer size
    size[0] = fragStreamParam.dstWidth * fragStreamParam.dstHeight * 4;
    size[1] = fragStreamParam.dstWidth * fragStreamParam.dstHeight;
    size[2] = fragStreamParam.dstWidth * fragStreamParam.dstHeight;

    status = pStream->setDstBuffer(ionOutputFD,
                                   size,
                                   DP_COLOR_GET_PLANE_COUNT(fragStreamParam.dstColorFormat));
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->setDstConfig(fragStreamParam.dstColorFormat,
                                   fragStreamParam.dstWidth,
                                   fragStreamParam.dstHeight,
                                   DP_COLOR_GET_MIN_Y_PITCH(fragStreamParam.dstColorFormat, fragStreamParam.dstWidth),
                                   DP_COLOR_GET_MIN_UV_PITCH(fragStreamParam.dstColorFormat, fragStreamParam.dstWidth));
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->startFrag(&mcuYCount,
                                false);

    ion_alloc_mm(ion_handle, (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 6), 0x200, 0, &ion_allocInHandle);

    ion_share(ion_handle, ion_allocInHandle, &ionInputFD);

    pSource = ion_mmap(ion_handle, 0, (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionInputFD, 0);

    do
    {
        status = pStream->dequeueFrag(&bufID,
                                      &format,
                                      pBuffer,
                                      &mcuXStart,
                                      &mcuYStart,
                                      &width,
                                      &height,
                                      &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            break;
        }

        // Decode image
        for (index = 0; index < height; index++)
        {
            switch(format)
            {
                case eI420:
                    pSourceFrag = (uint8_t*)pSource + (mcuYStart * fragStreamParam.MCUYSize + index) * fragStreamParam.dstWidth + mcuXStart * fragStreamParam.MCUXSize;
                    pTargetFrag = (uint8_t*)pBuffer[0] + index * pitch;
                    memcpy(pTargetFrag, pSourceFrag, width);

                    pSourceFrag = ((uint8_t*)pSource + (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 4)) + (mcuYStart * fragStreamParam.MCUYSize + index) * (fragStreamParam.dstWidth>>2) + (mcuXStart * fragStreamParam.MCUXSize>>2);
                    pTargetFrag = (uint8_t*)pBuffer[1] + index * (pitch >> 2);
                    memcpy(pTargetFrag, pSourceFrag, (width >> 2));

                    pSourceFrag = ((uint8_t*)pSource + (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 5)) + (mcuYStart * fragStreamParam.MCUYSize + index) * (fragStreamParam.dstWidth>>2) + (mcuXStart * fragStreamParam.MCUXSize>>2);
                    pTargetFrag = (uint8_t*)pBuffer[2] + index * (pitch >> 2);
                    memcpy(pTargetFrag, pSourceFrag, (width >> 2));
                    break;

                case eI422:
                    pSourceFrag = (uint8_t*)pSource + (mcuYStart * fragStreamParam.MCUYSize + index) * fragStreamParam.dstWidth + mcuXStart * fragStreamParam.MCUXSize;
                    pTargetFrag = (uint8_t*)pBuffer[0] + index * pitch;
                    memcpy(pTargetFrag, pSourceFrag, width);

                    pSourceFrag = ((uint8_t*)pSource + (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 4)) + (mcuYStart * fragStreamParam.MCUYSize + index) * (fragStreamParam.dstWidth>>1) + (mcuXStart * fragStreamParam.MCUXSize >> 1);
                    pTargetFrag = (uint8_t*)pBuffer[1] + index * (pitch >> 1);
                    memcpy(pTargetFrag, pSourceFrag, (width >> 1));

                    pSourceFrag = ((uint8_t*)pSource + (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 5)) + (mcuYStart * fragStreamParam.MCUYSize + index) * (fragStreamParam.dstWidth>>1) + (mcuXStart * fragStreamParam.MCUXSize >> 1);
                    pTargetFrag = (uint8_t*)pBuffer[2] + index * (pitch >> 1);
                    memcpy(pTargetFrag, pSourceFrag, (width >> 1));
                    break;

                case eI444:
                    pSourceFrag = (uint8_t*)pSource + (mcuYStart * fragStreamParam.MCUYSize + index) * fragStreamParam.dstWidth + mcuXStart * fragStreamParam.MCUXSize;
                    pTargetFrag = (uint8_t*)pBuffer[0] + index * pitch;
                    memcpy(pTargetFrag, pSourceFrag, width);

                    pSourceFrag = ((uint8_t*)pSource + (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 4)) + (mcuYStart * fragStreamParam.MCUYSize + index) * fragStreamParam.dstWidth + mcuXStart * fragStreamParam.MCUXSize;
                    pTargetFrag = (uint8_t*)pBuffer[1] + index * pitch;
                    memcpy(pTargetFrag, pSourceFrag, width);

                    pSourceFrag = ((uint8_t*)pSource + (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 5)) + (mcuYStart * fragStreamParam.MCUYSize + index) * fragStreamParam.dstWidth + mcuXStart * fragStreamParam.MCUXSize;
                    pTargetFrag = (uint8_t*)pBuffer[2] + index * pitch;
                    memcpy(pTargetFrag, pSourceFrag, width);
                    break;
                default:
                    ADD_FAILURE();
                    break;
            }
        }
        status = pStream->queueFrag(bufID);
    } while(1);

    status = pStream->stopFrag();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    if (NULL != pTarget)
    {
        ion_munmap(ion_handle, pTarget, (fragStreamParam.dstWidth * fragStreamParam.dstHeight * 6));
        ion_share_close(ion_handle, ionOutputFD);
        ion_free(ion_handle, ion_allocOutHandle);
        pTarget = NULL;
    }

    if (NULL != pSource)
    {
        ion_munmap(ion_handle, pSource, (fragStreamParam.srcWidth * fragStreamParam.srcHeight * 6));
        ion_share_close(ion_handle, ionInputFD);
        ion_free(ion_handle, ion_allocInHandle);
        pSource = NULL;
    }

    ion_close(ion_handle);

    delete pStream;
}

//Add other test case here
//TEST(FragStream, FragStreamCase0)
//{
//
//}
