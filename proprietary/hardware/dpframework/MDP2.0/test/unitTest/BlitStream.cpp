#include "TestDefine.h"
#include "DpBlitStream.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpChannel.h"
#include "DpDataType.h"

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

volatile uint32_t gBlitCounter = 0;

static void* blitStreamCaseTH(void* pInput)
{

    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pSource;
    DpRect        tmpRect;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pYTarget;
    void          *pUTarget;
    void          *pVTarget;


    pSource = malloc(640 * 480 * 3);

    status = utilReadBMP("/system/pat/640x480.bmp",
        (uint8_t*)pSource,
        (uint8_t*)NULL,
        (uint8_t*)NULL,
        eRGB888,
        640,
        480,
        640 * 3);

    pYTarget = malloc(64 * 48);

    pUTarget = malloc((64 * 48) >> 2);

    pVTarget = malloc((64 * 48) >> 2);
    pStream = new DpBlitStream();


    while(1)
    {


        // Setup buffer address
        pBuffer[0] = pSource;
        pBuffer[1] = NULL;
        pBuffer[2] = NULL;

        // Setup buffer size
        size[0] = 640 * 480 * 3;
        size[1] = 0;
        size[2] = 0;
        pStream->setSrcBuffer(pBuffer, size, 1);

        pStream->setRotate(0);

        tmpRect.x = 0;
        tmpRect.y = 0;
        tmpRect.w = 640;
        tmpRect.h = 480;
        pStream->setSrcConfig(640, 480, eRGB888, eInterlace_None, &tmpRect);


        // Setup buffer address
        pBuffer[0] = pYTarget;
        pBuffer[1] = pUTarget;
        pBuffer[2] = pVTarget;

        // Setup buffer size
        size[0] = 64 * 48;
        size[1] = (64 * 48) >> 2;
        size[2] = (64 * 48) >> 2;

        pStream->setDstBuffer(pBuffer, size, 3);

        pStream->setDstConfig(64, 48, eI420);

        //pStream->setTdshp(2);

        pStream->invalidate();


        gBlitCounter++;
        sleep(20);

    }

    delete pStream;


    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    return  NULL;

}


void blitStreamCaseRSZ2PixelsLimit(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pYSource;
    void          *pUVSource;
    DpRect        tmpRect;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pYTarget;
    void          *pUTarget;
    void          *pVTarget;
    void           *pYSource_512Align;
    void           *pUVSource_512Align;
    DpPqParam      pqparam;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pYSource = malloc(30720 + 512);
    pUVSource = malloc(15360 + 512);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/sdcard/pat/192x128.bmp",
                         (uint8_t*)pYSource,
                         (uint8_t*)pUVSource,
                         (uint8_t*)NULL,
                         DP_COLOR_420_BLKP_10_V,
                         192,
                         128,
                         7680);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pYSource_512Align  = (void*)(((unsigned long)pYSource + 511) & -512L);
    pUVSource_512Align = (void*)(((unsigned long)pUVSource + 511) & -512L);

    pBuffer[0] = pYSource_512Align;
    pBuffer[1] = pUVSource_512Align;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = (192 * 128 * 5) >> 2;
    size[1] = (192 * 128 * 5) >> 3;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 2);

	pStream->setRotate(90);

    pqparam.enable = 1;
    pqparam.scenario = MEDIA_VIDEO;
    pStream->setPQParameter(pqparam);

	tmpRect.x = 0;
	tmpRect.y = 0;
	tmpRect.w = 176;
	tmpRect.h = 96;
    pStream->setSrcConfig(192, 128, DP_COLOR_420_BLKP_10_H, eInterlace_None, &tmpRect);

    pYTarget = malloc(1398 * 2556 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = (7372800);
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(pBuffer, size, 1);

	tmpRect.x = 10;
	tmpRect.y = 4;
	tmpRect.w = 1398;
	tmpRect.h = 2556;

    pStream->setDstConfig(1398, 2556, 2880,0,DP_COLOR_YUYV,DP_PROFILE_BT601,eInterlace_None, &tmpRect);

    pStream->invalidate();

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pYSource)
    {
        free(pYSource);
        pYSource = NULL;
    }

    if (NULL != pUVSource)
    {
        free(pUVSource);
        pUVSource = NULL;
    }


    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamRSZ2PixelsLimit", BlitStreamRSZ2PixelsLimit, blitStreamCaseRSZ2PixelsLimit);


void blitStreamCaseRSZAlgoOverflow(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pSource;
	DpRect         tmpRect;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(57671680);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/sdcard/pat/5632x2560.bmp",
                         (uint8_t*)pSource,
                         NULL,
                         NULL,
                         DP_COLOR_RGBA8888,
                         5632,
                         2560,
                         5632 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 57671680;
    size[1] = 0;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 1);

	pStream->setRotate(270);
	tmpRect.x = 2095;
	tmpRect.y = 561;
	tmpRect.w = 1440;
	tmpRect.h = 1438;
    pStream->setSrcConfig(5632, 2560, DP_COLOR_RGBA8888, eInterlace_None, &tmpRect);

    pYTarget = malloc(14745600);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 14745600;
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(pBuffer, size, 1);

	tmpRect.x = 0;
	tmpRect.y = 560;
	tmpRect.w = 1440;
	tmpRect.h = 1440;

    pStream->setDstConfig(1440, 1440, 5760,0,DP_COLOR_RGBA8888,DP_PROFILE_BT601,eInterlace_None, &tmpRect);

    pStream->invalidate();

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamRSZAlgoOverflow", BlitStreamRSZAlgoOverflow, blitStreamCaseRSZAlgoOverflow)


void blitStreamCaseRGB888(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pSource;
    DpRect        tmpRect;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pYTarget;
    void          *pUTarget;
    void          *pVTarget;


    //blitStreamCaseTH(0);



    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         eRGB888,
                         640,
                         480,
                         640 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 * 3;
    size[1] = 0;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 1);

	pStream->setRotate(0);

    //pStream->setTdshp(0x30003);

	tmpRect.x = 0;
	tmpRect.y = 0;
	tmpRect.w = 640;
	tmpRect.h = 480;
    pStream->setSrcConfig(640, 480, eRGB888, eInterlace_None, &tmpRect);

    pYTarget = malloc(64 * 48);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 64 * 48;
    size[1] = (64 * 48) >> 2;
    size[2] = (64 * 48) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(64, 48, eI420);

    pStream->invalidate();

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamRGB888", BlitStreamRGB888, blitStreamCaseRGB888);


void blitStreamCaseRGB888Crop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pSource;
    DpRect         tmpRect;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pSource,
                         NULL,
                         NULL,
                         eRGB888,
                         640,
                         480,
                         640 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 * 3;
    size[1] = 0;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 1);

	pStream->setRotate(0);

	tmpRect.x = 160;
	tmpRect.y = 120;
	tmpRect.w = 320;
	tmpRect.h = 240;
    pStream->setSrcConfig(640, 480, eRGB888, eInterlace_None, &tmpRect);

    pYTarget = malloc(64 * 48);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 64 * 48;
    size[1] = (64 * 48) >> 2;
    size[2] = (64 * 48) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(64, 48, eI420);

    pStream->invalidate();

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamRGB888Crop", BlitStreamRGB888Crop, blitStreamCaseRGB888Crop);


void blitStreamCaseRGB888Crop180(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pSource;
	DpRect         tmpRect;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pSource,
                         NULL,
                         NULL,
                         eRGB888,
                         640,
                         480,
                         640 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 * 3;
    size[1] = (640 * 480) >> 2;
    size[2] = (640 * 480) >> 2;
    pStream->setSrcBuffer(pBuffer, size, 1);

	pStream->setRotate(180);

	tmpRect.x = 161;
	tmpRect.y = 121;
	tmpRect.w = 321;
	tmpRect.h = 241;
    pStream->setSrcConfig(640, 480, eRGB888, eInterlace_None, &tmpRect);

    pYTarget = malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 640 * 480 * 3;
    size[1] = (640 * 480) >> 2;
    size[2] = (640 * 480) >> 2;
    pStream->setDstBuffer(pBuffer, size, 1);

    pStream->setDstConfig(640, 480, eRGB888);

    pStream->invalidate();

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    //REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    //REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamRGB888Crop180", BlitStreamRGB888Crop180, blitStreamCaseRGB888Crop180);


void blitStreamCaseRGB565(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pSource;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pSource,
                         NULL,
                         NULL,
                         eRGB565,
                         640,
                         480,
                         640 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 * 2;
    size[1] = (640 * 480) >> 2;
    size[2] = (640 * 480) >> 2;
    pStream->setSrcBuffer(pBuffer, size, 1);
    pStream->setSrcConfig(640, 480, eRGB565);



	pStream->setRotate(0);

    pYTarget = malloc(64 * 48);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 64 * 48;
    size[1] = (64 * 48) >> 2;
    size[2] = (64 * 48) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(64, 48, eI420);

    pStream->invalidate();

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamRGB565", BlitStreamRGB565, blitStreamCaseRGB565);


void blitStreamCaseYUYV(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pSource;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pTarget;


    ion_user_handle_t ion_allocInHandle;
    ion_user_handle_t ion_allocOutHandle;

    int32_t         ionInputFD;
    int32_t         ionOutputFD;


    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    int ion_handle = ion_open();

    ion_alloc_mm(ion_handle, (1920 * 1088 * 2), 0x200, 0, &ion_allocInHandle);

    ion_share(ion_handle, ion_allocInHandle, &ionInputFD);

    pSource = ion_mmap(ion_handle, 0, (1920 * 1088 * 2), PROT_READ | PROT_WRITE , MAP_SHARED, ionInputFD, 0);

    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

#if 1
    status = utilReadBMP("/system/pat/1920x1088.bmp",
                         (uint8_t*)pSource,
                         ((uint8_t*)pSource)+(1920*1088),
                         NULL,
                         DP_COLOR_420_BLKP,
                         1920,
                         1088,
                         1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");
#endif
    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = ((uint8_t*)pSource+(1920*1088));
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 1920 * 1088;
    size[1] = (1920 * 1088) >> 1;
    size[2] = 0;
    pStream->setSrcBuffer(ionInputFD, size, 2);

    DpRect tmpRect;
    memset(&tmpRect,0,sizeof(tmpRect));

    tmpRect.x = 1;
    tmpRect.y = 1;
    tmpRect.w = 1904;
    tmpRect.h = 1080;

    pStream->setSrcConfig(1920, 1088, 1920*32,1920*16,DP_COLOR_420_BLKP,DP_PROFILE_BT601,eInterlace_None,&tmpRect);
    //pStream->setSrcConfig(1920, 1088, 1920*32,1920*16,DP_COLOR_420_BLKP);


    ion_alloc_mm(ion_handle, (740*304*2), 0x40, 0, &ion_allocOutHandle);

    ion_share(ion_handle, ion_allocOutHandle, &ionOutputFD);

    pTarget = ion_mmap(ion_handle, 0, (740*304*2), PROT_READ | PROT_WRITE, MAP_SHARED, ionOutputFD, 0);
    REPORTER_ASSERT_MESSAGE(pReporter, (pTarget != NULL), "Can't allocate Y target buffer");

    DPLOGI("pTARGET: %p\n", pTarget);
    // Setup buffer address
    pBuffer[0] = pTarget;
    pBuffer[1] = 0;
    pBuffer[2] = 0;

    // Setup buffer size
    size[0] = 740 * 304 * 2;
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(ionOutputFD, size, 1);

    pStream->setDstConfig(740, 304, eYUYV);

    pStream->invalidate();

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);


    if (NULL != pTarget)
    {
        ion_munmap(ion_handle, pTarget, (740*304*2));
        ion_share_close(ion_handle, ionOutputFD);
        ion_free(ion_handle, ion_allocOutHandle);
        pTarget = NULL;
    }

    if (NULL != pSource)
    {
        ion_munmap(ion_handle, pSource, (1920*1088*2));
        ion_share_close(ion_handle, ionInputFD);
        ion_free(ion_handle, ion_allocInHandle);
        pSource = NULL;
    }


    ion_close(ion_handle);


    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamYUYV", BlitStreamYUYV, blitStreamCaseYUYV);


void blitStreamCaseYUYV90(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pSource;
    void           *pSource_1;

    void           *pTarget;

    void           *pBuffer[3];
    uint32_t       size[3];

    ion_user_handle_t ion_allocInHandle;
    ion_user_handle_t ion_allocOutHandle;

    int32_t         ionInputFD;
    int32_t         ionOutputFD;



    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    int ion_handle = ion_open();

    ion_alloc_mm(ion_handle, (1280*960*2), 0x200, 0, &ion_allocInHandle);

    ion_share(ion_handle, ion_allocInHandle, &ionInputFD);

    pSource = ion_mmap(ion_handle, 0, (1280*960*2), PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, ionInputFD, 0);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    pSource_1 =  ((uint8_t*)pSource+(1280*960));

    DPLOGD("pSource: %p, pSource_1: %p\n", pSource, pSource_1);

    status = utilReadBMP( "/system/pat/1280x960.bmp",
                          (uint8_t*)pSource,
                          (uint8_t*)pSource_1,
                          NULL,
                          DP_COLOR_420_BLKP,
                          1280,
                          960,
                          1280);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer size
    size[0] = 1280*960;
    size[1] = (1280*960) >> 1;
    size[2] = 0;

    pStream->setSrcBuffer(ionInputFD, size, 2);
	pStream->setRotate(270);
	pStream->setFlip(1);

    DpRect tmpRect;
    tmpRect.x = 0;
    tmpRect.y = 0;
    tmpRect.w = 1280;
    tmpRect.h = 960;

    pStream->setSrcConfig(1280, 960, DP_COLOR_420_BLKP,eInterlace_None,&tmpRect);

    ion_alloc_mm(ion_handle, (1024 * 768 * 2) , 0x200, 0, &ion_allocOutHandle);

    ion_share(ion_handle, ion_allocOutHandle, &ionOutputFD);

    pTarget = ion_mmap(ion_handle, 0, (1024 * 768 * 2), PROT_READ | PROT_WRITE, MAP_SHARED, ionOutputFD, 0);
    REPORTER_ASSERT_MESSAGE(pReporter, (pTarget != NULL), "Can't allocate Y target buffer");

    // Setup buffer address
    pBuffer[0] = pTarget;
    pBuffer[1] = 0;
    pBuffer[2] = 0;

    // Setup buffer size
    size[0] = (1024 * 768 * 2);
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(ionOutputFD, size, 1);

    pStream->setDstConfig(768, 1024, eYUYV);
    pStream->invalidate();

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    if (NULL != pTarget)
    {
        ion_munmap(ion_handle, pTarget, (1024 * 768 * 2));
        ion_share_close(ion_handle, ionOutputFD);
        ion_free(ion_handle, ion_allocOutHandle);
        pTarget = NULL;
    }

    if (NULL != pSource)
    {
        ion_munmap(ion_handle, pSource, (1280*960*2));
        ion_share_close(ion_handle, ionInputFD);
        ion_free(ion_handle, ion_allocInHandle);
        pSource = NULL;
    }


    ion_close(ion_handle);
    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamYUYV90", BlitStreamYUYV90, blitStreamCaseYUYV90);


void blitStreamCaseNV12BP(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pYSource;
    void           *pUVSource;
    void           *pYSource_512Align;
    void           *pUVSource_512Align;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pYSource = malloc(640 * 480 + 512);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYSource != NULL), "Can't allocate Y source buffer");

    pUVSource = malloc(((640 * 480) >> 1) + 512);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUVSource != NULL), "Can't allocate UV source buffer");

    // Setup buffer address
    pYSource_512Align  = (void*)(((unsigned long)pYSource + 511) & -512L);
    pUVSource_512Align = (void*)(((unsigned long)pUVSource + 511) & -512L);

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pYSource_512Align,
                         (uint8_t*)pUVSource_512Align,
                         NULL,
                         eNV12_BP,
                         640,
                         480,
                         640 * 32);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pBuffer[0] = pYSource_512Align;
    pBuffer[1] = pUVSource_512Align;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 ;
    size[1] = (640 * 480) >> 1;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 2);

	pStream->setRotate(0);

	pStream->setSrcConfig(640, 480, eNV12_BP);

    pYTarget = malloc(640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((640 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((640 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 640 * 480;
    size[1] = (640 * 480) >> 2;
    size[2] = (640 * 480) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(640, 480, eI420);

    pStream->invalidate();

	// Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pUVSource)
    {
        free(pUVSource);
        pUVSource = NULL;
    }

    if (NULL != pYSource)
    {
        free(pYSource);
        pYSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamNV12BP", BlitStreamNV12BP, blitStreamCaseNV12BP);


void blitStreamCaseNV12BPCrop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pYSource;
    void           *pUVSource;
    DpRect         tmpRect;
    void           *pYSource_512Align;
    void           *pUVSource_512Align;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pYSource = malloc(640 * 480 + 512 );
    REPORTER_ASSERT_MESSAGE(pReporter, (pYSource != NULL), "Can't allocate Y source buffer");

    pUVSource = malloc(((640 * 480) >> 1) + 512);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUVSource != NULL), "Can't allocate UV source buffer");

    pYSource_512Align  = (void*)(((unsigned long)pYSource + 511) & -512L);
    pUVSource_512Align = (void*)(((unsigned long)pUVSource + 511) & -512L);

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pYSource_512Align,
                         (uint8_t*)pUVSource_512Align,
                         NULL,
                         eNV12_BP,
                         640,
                         480,
                         640 * 32);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pYSource_512Align;
    pBuffer[1] = pUVSource_512Align;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 ;
    size[1] = (640 * 480) >> 1;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 2);

	pStream->setRotate(0);

	tmpRect.x = 160;
	tmpRect.y = 0;
	tmpRect.h = 320;
	tmpRect.w = 320;
    pStream->setSrcConfig(640, 480, eNV12_BP, eInterlace_None, &tmpRect);

    pYTarget = malloc(64 * 48);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 64 * 48;
    size[1] = (64 * 48) >> 2;
    size[2] = (64 * 48) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(64, 48, eI420);

    pStream->invalidate();

	// Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pUVSource)
    {
        free(pUVSource);
        pUVSource = NULL;
    }

    if (NULL != pYSource)
    {
        free(pYSource);
        pYSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamNV12BPCrop", BlitStreamNV12BPCrop, blitStreamCaseNV12BPCrop);


void blitStreamCaseNV12BPCrop270(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pYSource;
    void           *pUVSource;
    DpRect         tmpRect;
    void           *pYSource_512Align;
    void           *pUVSource_512Align;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pYSource = malloc(640 * 480 + 512 );
    REPORTER_ASSERT_MESSAGE(pReporter, (pYSource != NULL), "Can't allocate Y source buffer");

    pUVSource = malloc(((640 * 480) >> 1) + 512);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUVSource != NULL), "Can't allocate UV source buffer");

    pYSource_512Align  = (void*)(((unsigned long)pYSource + 511)  & -512L);
    pUVSource_512Align = (void*)(((unsigned long)pUVSource + 511) & -512L);

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pYSource_512Align,
                         (uint8_t*)pUVSource_512Align,
                         NULL,
                         eNV12_BP,
                         640,
                         480,
                         640 * 32);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pYSource_512Align;
    pBuffer[1] = pUVSource_512Align;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 ;
    size[1] = (640 * 480) >> 1;
    size[2] = (640 * 480) >> 2;
    pStream->setSrcBuffer(pBuffer, size, 2);

	pStream->setRotate(270);

	tmpRect.x = 160;
	tmpRect.y = 120;
	tmpRect.w = 320;
	tmpRect.h = 240;

    pStream->setSrcConfig(640, 480, eNV12_BP, eInterlace_None, &tmpRect);

    pYTarget = malloc(64 * 48);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 64 * 48;
    size[1] = (64 * 48) >> 2;
    size[2] = (64 * 48) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(48, 64, eI420);

    pStream->invalidate();

	// Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pUVSource)
    {
        free(pUVSource);
        pUVSource = NULL;
    }

    if (NULL != pYSource)
    {
        free(pYSource);
        pYSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamNV12BPCrop270", BlitStreamNV12BPCrop270, blitStreamCaseNV12BPCrop270);


void blitStreamCaseNV12Crop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pYSource;
    void           *pUVSource;
    DpRect         tmpRect;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pYSource = malloc(640 * 480 );
    REPORTER_ASSERT_MESSAGE(pReporter, (pYSource != NULL), "Can't allocate Y source buffer");

    pUVSource = malloc((640 * 480) >> 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUVSource != NULL), "Can't allocate UV source buffer");

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pYSource,
                         (uint8_t*)pUVSource,
                         NULL,
                         eNV12,
                         640,
                         480,
                         640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pYSource;
    pBuffer[1] = pUVSource;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 ;
    size[1] = (640 * 480) >> 1;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 2);

	pStream->setRotate(270);

	tmpRect.x = 160;
	tmpRect.y = 120;
	tmpRect.w = 320;
	tmpRect.h = 240;
    pStream->setSrcConfig(640, 480, eNV12, eInterlace_None, &tmpRect);

    pYTarget = malloc(64 * 48);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((64 * 48) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 64 * 48;
    size[1] = (64 * 48) >> 2;
    size[2] = (64 * 48) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(48, 64, eI420);

    pStream->invalidate();

	// Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pUVSource)
    {
        free(pUVSource);
        pUVSource = NULL;
    }

    if (NULL != pYSource)
    {
        free(pYSource);
        pYSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamNV12Crop", BlitStreamNV12Crop, blitStreamCaseNV12Crop);


void blitStreamCaseI420Crop270(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream   *pStream;
    void           *pYSource;
    void           *pUSource;
    void           *pVSource;
    DpRect         tmpRect;
    void           *pBuffer[3];
    uint32_t       size[3];
    void           *pYTarget;
    void           *pUTarget;
    void           *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pYSource = malloc(640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYSource != NULL), "Can't allocate Y source buffer");

    pUSource = malloc((640 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUSource != NULL), "Can't allocate U source buffer");

    pVSource = malloc((640 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVSource != NULL), "Can't allocate V source buffer");

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pYSource,
                         (uint8_t*)pUSource,
                         (uint8_t*)pVSource,
                         eI420,
                         640,
                         480,
                         640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pYSource;
    pBuffer[1] = pUSource;
    pBuffer[2] = pVSource;

    // Setup buffer size
    size[0] = 640 * 480;
    size[1] = (640 * 480) >> 2;
    size[2] = (640 * 480) >> 2;
    pStream->setSrcBuffer(pBuffer, size, 3);

	pStream->setRotate(270);

	tmpRect.x = 0;
	tmpRect.y = 0;
	tmpRect.w = 320;
	tmpRect.h = 240;
    pStream->setSrcConfig(640, 480, eI420, eInterlace_None, &tmpRect);

    pYTarget = malloc(640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((640 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((640 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 640 * 480;
    size[1] = (640 * 480) >> 2;
    size[2] = (640 * 480) >> 2;
    pStream->setDstBuffer(pBuffer, size, 3);

    pStream->setDstConfig(480, 640, eI420);

    pStream->invalidate();

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pVSource)
    {
        free(pVSource);
        pVSource = NULL;
    }

    if (NULL != pUSource)
    {
        free(pUSource);
        pUSource = NULL;
    }

    if (NULL != pYSource)
    {
        free(pYSource);
        pYSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamI420Crop270", BlitStreamI420Crop270, blitStreamCaseI420Crop270);


void blitStreamCaseXRGB8888Crop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pSource;
    DpRect        tmpRect;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(199 * 199 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/system/pat/199x199.bmp",
                (uint8_t*)pSource,
                (uint8_t*)NULL,
                (uint8_t*)NULL,
                eXRGB8888,
                199,
                199,
                199 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 199 * 199 * 4;
    size[1] = 0;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 1);

	pStream->setRotate(0);

    tmpRect.x = 0;
	tmpRect.y = 0;
	tmpRect.w = 199;
	tmpRect.h = 199;
    pStream->setSrcConfig(199, 199, eXRGB8888, eInterlace_None, &tmpRect);

    pTarget = malloc(205 * 205 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (pTarget != NULL), "Can't allocate target buffer");

    // Setup buffer address
    pBuffer[0] = pTarget;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 205 * 205 * 4;
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(pBuffer, size, 1);

    pStream->setDstConfig(205, 205, eRGBX8888);

    pStream->invalidate();

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    if (NULL != pTarget)
    {
        free(pTarget);
        pTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamXRGB8888Crop", BlitStreamXRGB8888Crop, blitStreamCaseXRGB8888Crop);

void blitStreamCaseNV12BP_UFO(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pSource;
    void          *pSource1;
    DpRect        tmpRect;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pYTarget;
    void          *pUTarget;
    void          *pVTarget;
    FILE          *pFile;
    uint32_t      filesize = 0;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    //pSource = malloc(640 * 480);
    //REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    //pSource1 = malloc(640 * 240);
    //REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

#if 1
    uint32_t u4PIC_SIZE_Y,u4PIC_SIZE,u4PIC_WIDTH,u4PIC_HEIGHT,u4UFO_LEN_SIZE_Y,u4UFO_LEN_SIZE_C,u4PIC_SIZE_Y_BS,u4PIC_SIZE_BS,u4PIC_SIZE_REF;
    u4PIC_WIDTH = 720;
    u4PIC_HEIGHT = 480;
    u4PIC_SIZE_Y = ((u4PIC_WIDTH * u4PIC_HEIGHT + 511) >> 9) << 9;
    u4PIC_SIZE = ((u4PIC_SIZE_Y + (u4PIC_SIZE_Y >> 1) + 511) >> 9) << 9;

    u4UFO_LEN_SIZE_Y = ((((u4PIC_SIZE_Y + 255) >> 8) + 63 + (16*8)) >> 6) << 6;
    u4UFO_LEN_SIZE_C = (((u4UFO_LEN_SIZE_Y >> 1) + 15 + (16*8)) >> 4) << 4;

    u4PIC_SIZE_Y_BS = (((u4PIC_SIZE_Y + 4095) >> 12) << 12);;
    u4PIC_SIZE_BS = ((u4PIC_SIZE_Y_BS + (u4PIC_SIZE_Y >> 1) + 511) >> 9) << 9;
    u4PIC_SIZE_REF = (((u4PIC_SIZE_BS + (u4UFO_LEN_SIZE_Y << 1)) + 4095) >> 12) << 12;

    pSource = malloc(u4PIC_SIZE_REF);

    pFile = fopen("/system/pat/4K_NV12BP_Y.out", "rb");
    filesize = u4PIC_SIZE_Y;
    fread (pSource,1,filesize,pFile);
    fclose(pFile);

    pSource1 = (uint8_t*)pSource + u4PIC_SIZE_Y_BS;
    pFile = fopen("/system/pat/4K_NV12BP_C.out", "rb");
    filesize = u4PIC_SIZE_Y >> 1;
    fread (pSource1,1,filesize,pFile);
    fclose(pFile);

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = pSource1;
    pBuffer[2] = NULL;

    pSource1 = (uint8_t*)pSource + u4PIC_SIZE_BS;
    pFile = fopen("/system/pat/4K_NV12BP_LEN_Y.out", "rb");
    filesize = u4UFO_LEN_SIZE_Y;
    fread (pSource1,1,filesize,pFile);
    fclose(pFile);

    pSource1 = (uint8_t*)pSource1 + u4UFO_LEN_SIZE_Y;
    pFile = fopen("/system/pat/4K_NV12BP_LEN_C.out", "rb");
    filesize = u4UFO_LEN_SIZE_C - 128;
    fread (pSource1,1,filesize,pFile);
    fclose(pFile);


#endif

#if 0
    status = utilReadBMP("/system/pat/640x480.bmp",
        (uint8_t*)pSource,
        (uint8_t*)pSource1,
        (uint8_t*)NULL,
        DP_COLOR_420_BLKP,
        3840,
        2176,
        3840);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");
#endif

    // Setup buffer size
    size[0] = u4PIC_SIZE_Y_BS;
    size[1] = (u4PIC_SIZE_REF - u4PIC_SIZE_Y_BS);
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 2);

    pStream->setRotate(0);

    //pStream->setTdshp(0x30003);

    tmpRect.x = 0;
    tmpRect.y = 0;
    tmpRect.w = u4PIC_WIDTH;
    tmpRect.h = u4PIC_HEIGHT;
    pStream->setSrcConfig(u4PIC_WIDTH, u4PIC_HEIGHT, DP_COLOR_420_BLKP_UFO, eInterlace_None, &tmpRect);

#define DispSizeW 1280
#define DispSizeH 720


    pYTarget = malloc(DispSizeW  * DispSizeH * 2 );
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    pUTarget = malloc((DispSizeW * DispSizeH) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    pVTarget = malloc((DispSizeW * DispSizeH) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = DispSizeW * DispSizeH * 2;
    size[1] = 0;// (256 * 160) >> 2;
    size[2] = 0;// (256 * 160) >> 2;
    pStream->setDstBuffer(pBuffer, size, 1);

    pStream->setDstConfig(DispSizeW, DispSizeH, eYUYV);

    pStream->invalidate();

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamNV12BP_UFO", BlitStreamNV12BP_UFO, blitStreamCaseNV12BP_UFO);


void blitStreamCaseTDSHP(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pSource;
    DpRect        tmpRect;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pYTarget;
    void          *pUTarget;
    void          *pVTarget;


    //blitStreamCaseTH(0);



    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         eRGB888,
                         640,
                         480,
                         640 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 * 3;
    size[1] = 0;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 1);

	pStream->setRotate(0);

    pStream->setTdshp(0x00001);

	tmpRect.x = 0;
	tmpRect.y = 0;
	tmpRect.w = 640;
	tmpRect.h = 480;
    pStream->setSrcConfig(640, 480, eRGB888, eInterlace_None, &tmpRect);

    pYTarget = malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYTarget != NULL), "Can't allocate Y target buffer");

    //pUTarget = malloc((640 * 480) >> 2);
    //REPORTER_ASSERT_MESSAGE(pReporter, (pUTarget != NULL), "Can't allocate U target buffer");

    //pVTarget = malloc((640 * 480) >> 2);
    //REPORTER_ASSERT_MESSAGE(pReporter, (pVTarget != NULL), "Can't allocate V target buffer");

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = 0;//pUTarget;
    pBuffer[2] = 0;//pVTarget;

    // Setup buffer size
    size[0] = 640 * 480 * 3;
    size[1] = 0;//(640 * 480);
    size[2] = 0;//(640 * 480);
    pStream->setDstBuffer(pBuffer, size, 1);

    pStream->setDstConfig(640, 480, eRGB888);

    pStream->invalidate();

    // Verify Y output buffer
    //REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[0], size[0]);

    // Verify U output buffer
    //REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[1], size[1]);

    // Verify V output buffer
    //REPORTER_VERIFY_BIT_TRUE(pReporter, pBuffer[2], size[2]);

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pUTarget)
    {
        free(pUTarget);
        pUTarget = NULL;
    }

    if (NULL != pYTarget)
    {
        free(pYTarget);
        pYTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamTDSHP", BlitStreamTDSHP, blitStreamCaseTDSHP);

#if CONFIG_FOR_SEC_VIDEO_PATH
//for SVP
#include <tlc_sec_mem.h>

void blitStreamCaseSVP(TestReporter *pReporter)
{
    DpBlitStream   *pStream;
    void           *pBuffer[3];
    uint32_t       size[3];
    UREE_SECUREMEM_HANDLE secSrcHd, secDstHd;
    pStream = new DpBlitStream();

    // config Src buffer
    size[0] = 640 * 480 * 3;
    size[1] = 0;
    size[2] = 0;

    int ret = UREE_AllocSecurememTBL(&secSrcHd, 2048, size[0]+size[1]+size[2]);
    if (ret != 0)
    {
        printf("UREE_AllocSecurememTBL failed %d =>  size %d\n", ret, size[0]+size[1]+size[2]);
        return;
    }

    pBuffer[0] = (void*)(unsigned long)(secSrcHd);
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    pStream->setSrcBuffer(pBuffer, size, 1);
    pStream->setSrcConfig(640,
                          480,
                          DP_COLOR_GET_MIN_Y_PITCH(DP_COLOR_RGB888, 640),
                          DP_COLOR_GET_MIN_UV_PITCH(DP_COLOR_RGB888, 640),
                          DP_COLOR_RGB888,
                          DP_PROFILE_BT601,
                          eInterlace_None,
                          0,
                          DP_SECURE,
                          true);

// config Dst buffer
    size[0] = 640 * 480;
    size[1] = (640 * 480) >> 2;
    size[2] = (640 * 480) >> 2;

    ret = UREE_AllocSecurememTBL(&secDstHd, 2048, size[0]+size[1]+size[2]);
    if (ret != 0)
    {
        printf("UREE_AllocSecurememTBL failed %d =>  size %d \n", ret, size[0]+size[1]+size[2]);
        return;
    }

    pBuffer[0] = (void*)(unsigned long)(secDstHd);
    pBuffer[1] = (void*)(unsigned long)(secDstHd);
    pBuffer[2] = (void*)(unsigned long)(secDstHd);

    pStream->setDstBuffer(pBuffer, size, 3);
    pStream->setDstConfig(640,
                          480,
                          DP_COLOR_GET_MIN_Y_PITCH(DP_COLOR_YV12, 640),
                          DP_COLOR_GET_MIN_UV_PITCH(DP_COLOR_YV12, 640),
                          DP_COLOR_YV12,
                          DP_PROFILE_BT601,
                          eInterlace_None,
                          NULL,
                          DP_SECURE,
                          true);

    pStream->invalidate();

    uint32_t count;
    UREE_UnreferenceSecurememTBL(secSrcHd, &count);
    UREE_UnreferenceSecurememTBL(secDstHd, &count);

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamSVP", BlitStreamSVP, blitStreamCaseSVP);
#endif


void blitStreamCaseARGB8888(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pSource;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pYTarget;
    void          *pUTarget;
    void          *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(640 * 480 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    pYTarget = malloc(640 * 480 * 4);
    pUTarget = NULL;
    pVTarget = NULL;

    status = utilReadBMP("/system/pat/640x480.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         eARGB8888,
                         640,
                         480,
                         640 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 * 4;
    size[1] = 0;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 1);

    pStream->setRotate(180);

    pStream->setSrcConfig(640, 480, eARGB8888);

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 640 * 480 * 4;
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(pBuffer, size, 1);

    pStream->setDstConfig(640, 480, eARGB8888);

    pStream->invalidate();

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 640 * 480 * 4;
    size[1] = 0;
    size[2] = 0;
    pStream->setSrcBuffer(pBuffer, size, 1);

    pStream->setRotate(90);

    pStream->setSrcConfig(640, 480, eARGB8888);

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 640 * 480 * 4;
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(pBuffer, size, 1);

    pStream->setDstConfig(480, 640, eARGB8888);

    pStream->invalidate();

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamARGB8888", BlitStreamARGB8888, blitStreamCaseARGB8888)

void blitStreamCaseARGB8888Crop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpBlitStream  *pStream;
    void          *pSource;
    void          *pBuffer[3];
    uint32_t      size[3];
    void          *pYTarget;
    void          *pUTarget;
    void          *pVTarget;

    pStream = new DpBlitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "blitStream object should not be equal");

    pSource = malloc(1920 * 1088 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source buffer");

    pYTarget = malloc(1920 * 1088 * 4);
    pUTarget = NULL;
    pVTarget = NULL;

    status = utilReadBMP("/system/pat/1920x1080.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         eARGB8888,
                         1920,
                         1080,
                         1920 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Setup buffer address
    pBuffer[0] = pSource;
    pBuffer[1] = NULL;
    pBuffer[2] = NULL;

    // Setup buffer size
    size[0] = 1920 * 1088 * 4;
    size[1] = 0;
    size[2] = 0;

    DpRect src_roi(0, 0, 1920, 1080);

    pStream->setSrcBuffer(pBuffer, size, 1);

    pStream->setSrcConfig(1920, 1088, eARGB8888, eInterlace_None,  &src_roi);

    // Setup buffer address
    pBuffer[0] = pYTarget;
    pBuffer[1] = pUTarget;
    pBuffer[2] = pVTarget;

    // Setup buffer size
    size[0] = 1920 * 1088 * 4;
    size[1] = 0;
    size[2] = 0;
    pStream->setDstBuffer(pBuffer, size, 1);

    pStream->setRotate(0);

    DpRect dst_roi0(0, 4, 1920, 1080);

    pStream->setDstConfig(1920, 1080, eARGB8888, eInterlace_None, &dst_roi0);

    pStream->invalidate();

    memset(pBuffer[0], 0, size[0]);

    pStream->setRotate(90);

    DpRect dst_roi1(4, 0, 1080, 1920);

    pStream->setDstConfig(1080, 1920, 1088*4, 0, eARGB8888, DP_PROFILE_BT601, eInterlace_None, &dst_roi1);

    pStream->invalidate();

    if (NULL != pVTarget)
    {
        free(pVTarget);
        pVTarget = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("BlitStreamARGB8888Crop", BlitStreamARGB8888Crop, blitStreamCaseARGB8888Crop)


