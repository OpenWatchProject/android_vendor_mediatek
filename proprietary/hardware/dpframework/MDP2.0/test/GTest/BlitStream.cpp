#include "DpWriteBin.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpChannel.h"
#include "DpDataType.h"
#include "DpTimer.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include "gtest/gtest.h"

#include "DpBlitStream.h"
#include "DpBlitStreamParam.h"

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

class BlitStream: public ::testing::TestWithParam<DpBlitStream_Param>{};

INSTANTIATE_TEST_CASE_P(BlitParam, BlitStream, testing::Values(
//                 srcWidth, srcHeight, srcColorFormat,            srcProfile,        srcField,                        srcCrop,       srcSecure, srcDoFlush, rotate, flip, dstWidth, dstHeight, dstColorFormat,            dstProfile,        dstField,                         dstROI,      dstSecure, dstDoFlush
DpBlitStream_Param{     640,       480,        eRGB888,      DP_PROFILE_BT601, eInterlace_None, DpRect{   0,    0,  640,  480},  DP_SECURE_NONE,          0,      0,    0,       64,        48,          eI420,      DP_PROFILE_BT601, eInterlace_None, DpRect{   0,    0,   64,   48}, DP_SECURE_NONE,          0},
DpBlitStream_Param{     640,       480,        eRGB888,      DP_PROFILE_BT601, eInterlace_None, DpRect{ 160,  120,  320,  240},  DP_SECURE_NONE,          0,      0,    0,       64,        48,          eI420,      DP_PROFILE_BT601, eInterlace_None, DpRect{   0,    0,   64,   48}, DP_SECURE_NONE,          0},
DpBlitStream_Param{     640,       480,        eRGB888,      DP_PROFILE_BT601, eInterlace_None, DpRect{ 161,  121,  321,  241},  DP_SECURE_NONE,          0,    180,    0,      640,       480,        eRGB888,      DP_PROFILE_BT601, eInterlace_None, DpRect{   0,    0,  640,  480}, DP_SECURE_NONE,          0},
DpBlitStream_Param{     640,       480,        eRGB565,      DP_PROFILE_BT601, eInterlace_None, DpRect{   0,    0,  640,  480},  DP_SECURE_NONE,          0,      0,    0,       64,        48,          eI420,      DP_PROFILE_BT601, eInterlace_None, DpRect{   0,    0,   64,   48}, DP_SECURE_NONE,          0}
));

TEST_P(BlitStream, BlitStreamNormalTest)
{
    DpBlitStream_Param blitStreamParam = GetParam();
    DP_STATUS_ENUM    status;
    DpBlitStream      *pStream;
    void              *pSource;
    DpRect            tmpRect;
    void              *pBuffer[3];
    uint32_t          size[3];
    void              *pTarget;
    ion_user_handle_t ion_allocInHandle;
    ion_user_handle_t ion_allocOutHandle;
    int32_t           ionInputFD;
    int32_t           ionOutputFD;


    pStream = new DpBlitStream();

    int ion_handle = ion_open();

    ion_alloc_mm(ion_handle, (blitStreamParam.srcWidth * blitStreamParam.srcHeight * 6), 0x200, 0, &ion_allocInHandle);

    ion_share(ion_handle, ion_allocInHandle, &ionInputFD);

    pSource = ion_mmap(ion_handle, 0, (blitStreamParam.srcWidth * blitStreamParam.srcHeight * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionInputFD, 0);

    pBuffer[0] = pSource;
    pBuffer[1] = ((uint8_t*)pBuffer[0] + (blitStreamParam.srcWidth * blitStreamParam.srcHeight * 4));
    pBuffer[2] = ((uint8_t*)pBuffer[1] + (blitStreamParam.srcWidth * blitStreamParam.srcHeight));


    // Setup buffer size
    size[0] = blitStreamParam.srcWidth * blitStreamParam.srcHeight * 4;
    size[1] = blitStreamParam.srcWidth * blitStreamParam.srcHeight;
    size[2] = blitStreamParam.srcWidth * blitStreamParam.srcHeight;

    status = pStream->setSrcBuffer(ionInputFD, size, DP_COLOR_GET_PLANE_COUNT(blitStreamParam.srcColorFormat));
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->setRotate(blitStreamParam.rotate);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->setSrcConfig(blitStreamParam.srcWidth,
                                   blitStreamParam.srcHeight,
                                   DP_COLOR_GET_MIN_Y_PITCH(blitStreamParam.srcColorFormat, blitStreamParam.srcWidth),
                                   DP_COLOR_GET_MIN_UV_PITCH(blitStreamParam.srcColorFormat, blitStreamParam.srcWidth),
                                   blitStreamParam.srcColorFormat,
                                   blitStreamParam.srcProfile,
                                   blitStreamParam.srcField,
                                   &blitStreamParam.srcCrop,
                                   blitStreamParam.srcSecure,
                                   blitStreamParam.srcDoFlush);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    ion_alloc_mm(ion_handle, (blitStreamParam.dstWidth * blitStreamParam.dstHeight * 6), 0x200, 0, &ion_allocOutHandle);

    ion_share(ion_handle, ion_allocOutHandle, &ionOutputFD);

    pTarget = ion_mmap(ion_handle, 0, (blitStreamParam.dstWidth * blitStreamParam.dstHeight * 6), PROT_READ | PROT_WRITE , MAP_SHARED, ionOutputFD, 0);

    // Setup buffer address
    pBuffer[0] = pTarget;
    pBuffer[1] = ((uint8_t*)pBuffer[0] + (blitStreamParam.dstWidth * blitStreamParam.dstHeight * 4));
    pBuffer[2] = ((uint8_t*)pBuffer[1] + (blitStreamParam.dstWidth * blitStreamParam.dstHeight));

    // Setup buffer size
    size[0] = blitStreamParam.dstWidth * blitStreamParam.dstHeight * 4;
    size[1] = blitStreamParam.dstWidth * blitStreamParam.dstHeight;
    size[2] = blitStreamParam.dstWidth * blitStreamParam.dstHeight;

    status = pStream->setDstBuffer(ionOutputFD, size, DP_COLOR_GET_PLANE_COUNT(blitStreamParam.dstColorFormat));
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->setDstConfig(blitStreamParam.dstWidth,
                                   blitStreamParam.dstHeight,
                                   DP_COLOR_GET_MIN_Y_PITCH(blitStreamParam.dstColorFormat, blitStreamParam.dstWidth),
                                   DP_COLOR_GET_MIN_UV_PITCH(blitStreamParam.dstColorFormat, blitStreamParam.dstWidth),
                                   blitStreamParam.dstColorFormat,
                                   blitStreamParam.dstProfile,
                                   blitStreamParam.dstField,
                                   &blitStreamParam.dstROI,
                                   blitStreamParam.dstSecure,
                                   blitStreamParam.dstDoFlush);
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    status = pStream->invalidate();
    EXPECT_EQ(DP_STATUS_RETURN_SUCCESS, status);

    if (NULL != pTarget)
    {
        ion_munmap(ion_handle, pTarget, (blitStreamParam.dstWidth * blitStreamParam.dstHeight * 6));
        ion_share_close(ion_handle, ionOutputFD);
        ion_free(ion_handle, ion_allocOutHandle);
        pTarget = NULL;
    }

    if (NULL != pSource)
    {
        ion_munmap(ion_handle, pSource, (blitStreamParam.srcWidth * blitStreamParam.srcHeight * 6));
        ion_share_close(ion_handle, ionInputFD);
        ion_free(ion_handle, ion_allocInHandle);
        pSource = NULL;
    }

    ion_close(ion_handle);

    delete pStream;
}

//Add other test case here
//TEST(BlitStream, BlitStreamCase0)
//{
//
//}
