#include "TestDefine.h"
#include "DpFragStream.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpWriteBin.h"
#include "JPEGYInput.h"
#include "JPEGUInput.h"
#include "JPEGVInput.h"
#include "DpTimer.h"

void fragStreamCaseI420Crop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpFragStream   *pStream;
    DpRect         tmpRect;
    uint8_t        *pImageY;
    uint8_t        *pImageU;
    uint8_t        *pImageV;
    uint8_t        *pYOut;
    uint8_t        *pUOut;
    uint8_t        *pVOut;
    int32_t        bufID;
    DpColorFormat  format;
    void           *pBuf[3];
    uint32_t       size[3];
    int32_t        mcuXStart;
    int32_t        mcuYStart;
    int32_t        mcuXSize;
    int32_t        mcuYSize;
    int32_t        width;
    int32_t        height;
    int32_t        pitch;
    int32_t        index;
    uint8_t        *pSource;
    uint8_t        *pTarget;
	uint32_t 		mcuYCount;

    pStream = new DpFragStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Create fragment stream object failed");

	mcuXSize = 16;
	mcuYSize = 16;

	tmpRect.x = 0;
	tmpRect.y = 0;
	tmpRect.w = 640;
	tmpRect.h = 480;	

    DPLOGI("Set source information\n");
    status = pStream->setSrcConfig(eI420,
                                   640,
                                   480,
                                   mcuXSize,
                                   mcuYSize,
                                   640,
                                   &tmpRect);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source information failed");

    DPLOGI("Allocate Y target buffer\n");
    pYOut = (uint8_t*)malloc(sizeof(uint8_t) * 320 * 240 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOut != NULL), "Allocate Y target buffer failed");

    pBuf[0] = pYOut;
    pBuf[1] = 0;
    pBuf[2] = 0;

    size[0] = 320 * 240 * 3 ;
    size[1] = 0;
    size[2] = 0;

    DPLOGI("Set target buffer\n");
    status = pStream->setDstBuffer(pBuf,
                                   size,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set target buffer failed");

    DPLOGI("Set target information\n");
    status = pStream->setDstConfig(eRGB888,
                                   320,
                                   240,
                                   960);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Source target information failed");

    DPLOGI("Start fragment processing\n");
    status = pStream->startFrag(&mcuYCount,
                                false);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start fragment processing failed");

    DPLOGI("Allocate Y source buffer\n");
    pImageY = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageY != NULL), "Allocate Y source buffer failed");

    DPLOGI("Allocate U source buffer\n");
    pImageU = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageU != NULL), "Allocate U source buffer failed");

    DPLOGI("Allocate V source buffer\n");
    pImageV = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageV != NULL), "Allocate V source buffer failed");

    DPLOGI("Read bitmap source image\n");
    status = utilReadBMP("./pat/640x480.bmp",
                         pImageY,
                         pImageU,
                         pImageV,
                         eI420,
                         640,
                         480,
                         640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read bitmap source image failed");

    do
    {
        status = pStream->dequeueFrag(&bufID,
                                      &format,
                                      pBuf,
                                      &mcuXStart,
                                      &mcuYStart,
                                      &width,
                                      &height,
                                      &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            break;
        }

        DPLOGI("xStart: %d, yStart: %d, width %d, height %d\n", mcuXStart, mcuYStart, width, height);

        // Decode image
        for (index = 0; index < height; index++)
        {

            switch(format)
            {
                case eI420:
		            pSource = pImageY + (mcuYStart * mcuYSize + index) * 640 + mcuXStart * mcuXSize;
					pTarget = (uint8_t*)pBuf[0] + index * pitch;
                    memcpy(pTarget, pSource, width);

		            pSource = pImageU + (mcuYStart * mcuYSize + index) * 160 + (mcuXStart * mcuXSize>>2);
                    pTarget = (uint8_t*)pBuf[1] + index * (pitch >> 2);
                    memcpy(pTarget, pSource, (width >> 2));

					pSource = pImageV + (mcuYStart * mcuYSize + index) * 160 + (mcuXStart * mcuXSize>>2);
                    pTarget = (uint8_t*)pBuf[2] + index * (pitch >> 2);
                    memcpy(pTarget, pSource, (width >> 2));
                    break;
                default:
                    REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect source buffer format");
                    break;
            }
        }

        status = utilWriteBMP("./out/fragment.bmp",
                              (uint8_t*)pBuf[0],
                              (uint8_t*)pBuf[1],
                              (uint8_t*)pBuf[2],
                              format,
                              width,
                              height,
                              pitch,
                              pitch>>1);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump fragment buffer failed");

        status = pStream->queueFrag(bufID);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue fragment buffer failed");
    } while(1);

    DPLOGI("Stop fragment processing\n");
    status = pStream->stopFrag();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop fragment processing failed");

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pYOut, 320 * 240 * 3);

    if (NULL != pYOut)
    {
        free(pYOut);
        pYOut = NULL;
    }

    if (NULL != pImageY)
    {
        free(pImageY);
        pImageY = NULL;
    }

    if (NULL != pImageU)
    {
        free(pImageU);
        pImageU = NULL;
    }

    if (NULL != pImageV)
    {
        free(pImageV);
        pImageV = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("FragStreamI420Crop", FragStreamI420Crop, fragStreamCaseI420Crop)


void fragStreamCaseI422Crop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpFragStream   *pStream;
    DpRect         tmpRect;
    uint8_t        *pImageY;
    uint8_t        *pImageU;
    uint8_t        *pImageV;
    uint8_t        *pOut;
    int32_t        bufID;
    DpColorFormat  format;
    void           *pBuf[3];
    uint32_t       size[3];
    int32_t        mcuXStart;
    int32_t        mcuYStart;
    int32_t        mcuXSize;
    int32_t        mcuYSize;
    int32_t        width;
    int32_t        height;
    int32_t        pitch;
    int32_t        index;
    uint8_t        *pSource;
    uint8_t        *pTarget;
	uint32_t 		mcuYCount;

    pStream = new DpFragStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Create fragment stream object failed");

	tmpRect.x = 161;
	tmpRect.y = 121;
	tmpRect.w = 339;
	tmpRect.h = 279;

	mcuXSize = 16;
	mcuYSize = 8;

    DPLOGI("Set source information\n");
    status = pStream->setSrcConfig(eI422,
                                   640,
                                   480,
								   mcuXSize,
								   mcuYSize,
                                   640,
                                   &tmpRect);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source information failed");

    DPLOGI("Allocate target buffer\n");
    pOut = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOut != NULL), "Allocate target buffer failed");

    pBuf[0] = pOut;
    pBuf[1] = 0;
    pBuf[2] = 0;

    size[0] = 640 * 480 * 4;
    size[1] = 0;
    size[2] = 0;

    DPLOGI("Set target buffer\n");
    status = pStream->setDstBuffer(pBuf,
                                   size,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set target buffer failed"); 

    DPLOGI("Set target information\n");
    status = pStream->setDstConfig(eRGBX8888,
                                   640,
                                   480,
                                   640 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Source target information failed");


    DPLOGI("Start fragment processing\n");
    status = pStream->startFrag(&mcuYCount,
                                false);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start fragment processing");

    DPLOGI("Allocate Y source buffer\n");
    pImageY = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageY != NULL), "Allocate Y source buffer failed");

    DPLOGI("Allocate U source buffer\n");
    pImageU = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageU != NULL), "Allocate U source buffer failed");

    DPLOGI("Allocate V source buffer\n");
    pImageV = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageV != NULL), "Allocate V source buffer failed");

    DPLOGI("Read bitmap source image\n");
    status = utilReadBMP("./pat/640x480.bmp",
                         pImageY,
                         pImageU,
                         pImageV,
                         eI422,
                         640,
                         480,
                         640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read bitmap source image failed");

    do
    {
        status = pStream->dequeueFrag(&bufID,
                                      &format,
                                      pBuf,
                                      &mcuXStart,
                                      &mcuYStart,
                                      &width,
                                      &height,
                                      &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            break;
        }

        DPLOGI("xStart: %d, yStart: %d, width %d, height %d\n", mcuXStart, mcuYStart, width, height);

        // Decode image
        for (index = 0; index < height; index++)
        {

            switch(format)
            {
				case eI422:
		            pSource = pImageY + (mcuYStart * mcuYSize + index) * 640 + mcuXStart * mcuXSize;
					pTarget = (uint8_t*)pBuf[0] + index * pitch;
                    memcpy(pTarget, pSource, width);

		            pSource = pImageU + (mcuYStart * mcuYSize + index) * 320 + (mcuXStart * mcuXSize >> 1);
                    pTarget = (uint8_t*)pBuf[1] + index * (pitch >> 1);
                    memcpy(pTarget, pSource, (width >> 1));

					pSource = pImageV + (mcuYStart * mcuYSize + index) * 320 + (mcuXStart * mcuXSize >> 1);
                    pTarget = (uint8_t*)pBuf[2] + index * (pitch >> 1);
                    memcpy(pTarget, pSource, (width >> 1));
                    break;
                default:
                    REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect source buffer format");
                    break;
            }
        }

        status = utilWriteBMP("./out/fragment.bmp",
                              (uint8_t*)pBuf[0],
                              (uint8_t*)pBuf[1],
                              (uint8_t*)pBuf[2],
                              format,
                              width,
                              height,
                              pitch,
                              pitch>>1);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump fragment buffer failed");

        status = pStream->queueFrag(bufID);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue fragment buffer failed");
    } while(1);

    DPLOGI("Stop fragment processing\n");
    status = pStream->stopFrag();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop the fragment processing");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pOut, 640 * 480 * 4);

    if (NULL != pOut)
    {
        free(pOut);
        pOut = NULL;
    }

    if (NULL != pImageY)
    {
        free(pImageY);
        pImageY = NULL;
    }

    if (NULL != pImageU)
    {
        free(pImageU);
        pImageU = NULL;
    }

    if (NULL != pImageV)
    {
        free(pImageV);
        pImageV = NULL;
    }


    delete pStream;
}

DEFINE_TEST_CASE("FragStreamI422Crop", FragStreamI422Crop, fragStreamCaseI422Crop)


void fragStreamCaseI444Crop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpFragStream   *pStream;
    DpRect         tmpRect;
    uint8_t        *pImageY;
    uint8_t        *pImageU;
    uint8_t        *pImageV;
    uint8_t        *pYOut;
    uint8_t        *pUOut;
    uint8_t        *pVOut;
    int32_t        bufID;
    DpColorFormat  format;
    void           *pBuf[3];
    uint32_t       size[3];
    int32_t        mcuXStart;
    int32_t        mcuYStart;
    int32_t        mcuXSize;
    int32_t        mcuYSize;
    int32_t        width;
    int32_t        height;
    int32_t        pitch;
    int32_t        index;
    uint8_t        *pSource;
    uint8_t        *pTarget;
	uint32_t 		mcuYCount;

    pStream = new DpFragStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Create fragment stream object failed");

	tmpRect.x = 131;
	tmpRect.y = 158;
	tmpRect.w = 327;
	tmpRect.h = 241;	

    mcuXSize = 16;
	mcuYSize = 8;		

    DPLOGI("Set source information\n");
    status = pStream->setSrcConfig(eI444,
                                   640,
                                   480,
								   mcuXSize,
								   mcuYSize,
                                   640,
                                   &tmpRect);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source information failed");

    DPLOGI("Allocate Y target buffer\n");
    pYOut = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOut != NULL), "Allocate Y target buffer failed");

    DPLOGI("Allocate U target buffer\n");
    pUOut = (uint8_t*)malloc(sizeof(uint8_t) * (640 * 480 >> 2));
    REPORTER_ASSERT_MESSAGE(pReporter, (pUOut != NULL), "Allocate U target buffer failed");

    DPLOGI("Allocate V target buffer\n");
    pVOut = (uint8_t*)malloc(sizeof(uint8_t) * (640 * 480 >> 2));
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOut != NULL), "Allocate V target buffer failed");

    pBuf[0] = pYOut;
    pBuf[1] = pUOut;
    pBuf[2] = pVOut;

    size[0] = 640 * 480;
    size[1] = (640 * 480 >> 2);
    size[2] = (640 * 480 >> 2);

    DPLOGI("Set target buffer\n");
    status = pStream->setDstBuffer(pBuf,
                                   size,
                                   3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOut != NULL), "Set target buffer failed");

    DPLOGI("Set target information\n");
    status = pStream->setDstConfig(eYV12,
                                   640,
                                   480,
                                   640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set target information failed");

    DPLOGI("Start fragment processing\n");
    status = pStream->startFrag(&mcuYCount,
                                false);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start fragment processing failed");

    DPLOGI("Allocate Y source buffer\n");
    pImageY = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageY != NULL), "Allocate Y source buffer failed");

    DPLOGI("Allocate U source buffer\n");
    pImageU = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageU != NULL), "Allocate U source buffer failed");

    DPLOGI("Allocate V source buffer\n");
    pImageV = (uint8_t*)malloc(sizeof(uint8_t) * 640 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageV != NULL), "Allocate V source buffer failed");

    DPLOGI("Read bitmap source image\n");
    status = utilReadBMP("./pat/640x480.bmp",
                         pImageY,
                         pImageU,
                         pImageV,
                         eI444,
                         640,
                         480,
                         640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read bitmap source image failed");

    do
    {
        status = pStream->dequeueFrag(&bufID,
                                      &format,
                                      pBuf,
                                      &mcuXStart,
                                      &mcuYStart,
                                      &width,
                                      &height,
                                      &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            break;
        }

        DPLOGI("xStart: %d, yStart: %d, width %d, height %d\n", mcuXStart, mcuYStart, width, height);

        // Decode image
        for (index = 0; index < height; index++)
        {

            switch(format)
            {
				case eI444:
		            pSource = pImageY + (mcuYStart * mcuYSize + index) * 640 + mcuXStart * mcuXSize;
					pTarget = (uint8_t*)pBuf[0] + index * pitch;
                    memcpy(pTarget, pSource, width);

		            pSource = pImageU + (mcuYStart * mcuYSize + index) * 640 + mcuXStart * mcuXSize;
                    pTarget = (uint8_t*)pBuf[1] + index * pitch;
                    memcpy(pTarget, pSource, width);

					pSource = pImageV + (mcuYStart * mcuYSize + index) * 640 + mcuXStart * mcuXSize;
                    pTarget = (uint8_t*)pBuf[2] + index * pitch;
                    memcpy(pTarget, pSource, width);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        status = utilWriteBMP("./out/fragment.bmp",
                              (uint8_t*)pBuf[0],
                              (uint8_t*)pBuf[1],
                              (uint8_t*)pBuf[2],
                              format,
                              width,
                              height,
                              pitch,
                              pitch>>1);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump fragment buffer failed");

        status = pStream->queueFrag(bufID);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue fragment buffer failed");
    } while(1);

    DPLOGI("Stop fragment processing\n");
    status = pStream->stopFrag();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop fragment processing failed");

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pYOut, 640 * 480);

    // Verify U output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pUOut, (640 * 480) >> 2);

    // Verify V output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pVOut, (640 * 480) >> 2);

    if (NULL != pVOut)
    {
        free(pVOut);
        pVOut = NULL;
    }

    if (NULL != pUOut)
    {
        free(pUOut);
        pUOut = NULL;
    }

    if (NULL != pYOut)
    {
        free(pYOut);
        pYOut = NULL;
    }

    if (NULL != pImageY)
    {
        free(pImageY);
        pImageY = NULL;
    }

    if (NULL != pImageU)
    {
        free(pImageU);
        pImageU = NULL;
    }

    if (NULL != pImageV)
    {
        free(pImageV);
        pImageV = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("FragStreamI444Crop", FragStreamI444Crop, fragStreamCaseI444Crop)


void fragStreamCaseGreyCrop(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpRect         tmpRect;
    DpFragStream   *pStream;
    uint8_t        *pImageY;
    uint8_t        *pOut;
    int32_t        bufID;
    DpColorFormat  format;
    void           *pBuf[3];
    uint32_t       size[3];
    int32_t        mcuXStart;
    int32_t        mcuYStart;
    int32_t        mcuXSize;
    int32_t        mcuYSize;
    int32_t        width;
    int32_t        height;
    int32_t        pitch;
    int32_t        index;
    uint8_t        *pSource;
    uint8_t        *pTarget;
	uint32_t 		mcuYCount;

    pStream = new DpFragStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Create fragment stream object failed");

	tmpRect.x = 0;
	tmpRect.y = 0;
	tmpRect.w = 320;
	tmpRect.h = 240;	

	mcuXSize = 8;
    mcuYSize = 8;		

    DPLOGI("Set source information\n");
    status = pStream->setSrcConfig(eGREY,
                                   320,
                                   240,
                                   mcuXSize,
                                   mcuYSize,
                                   320,
                                   &tmpRect);

    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Source source information failed");

    DPLOGI("Allocate target buffer\n");
    pOut = (uint8_t*)malloc(sizeof(uint8_t) * 199 * 199 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOut != NULL), "Allocate target buffer failed");

    pBuf[0] = pOut;
    pBuf[1] = NULL;
    pBuf[2] = NULL;

    size[0] = 199 * 199 * 4;
    size[1] = 0;
    size[2] = 0;

    DPLOGI("Set target buffer\n");
    status = pStream->setDstBuffer(pBuf,
                                   size,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set target buffer failed");

    DPLOGI("Set target information\n");
    status = pStream->setDstConfig(eRGBX8888,
                                   199,
                                   199,
                                   199 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set target information failed");

    DPLOGI("Start fragment processing\n");
    status = pStream->startFrag(&mcuYCount,
                                false);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start fragment processing failed");


    pImageY = (uint8_t*)malloc(sizeof(uint8_t) * 320 * 240);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageY != NULL), "Allocate source image buffer failed\n");

    status = utilReadBMP("./pat/320x240.bmp",
                         pImageY,
                         NULL,
                         NULL,
                         eGREY,
                         320,
                         240,
                         320);   
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read bitmap source image failed");

    do
    {
        status = pStream->dequeueFrag(&bufID,
                                      &format,
                                      pBuf,
                                      &mcuXStart,
                                      &mcuYStart,
                                      &width,
                                      &height,
                                      &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            break;
        }

        DPLOGI("xStart: %d, yStart: %d, width %d, height %d\n", mcuXStart, mcuYStart, width, height);

        // Decode image
        for (index = 0; index < height; index++)
        {

            switch(format)
            {
                case eGREY:
                    pSource = pImageY + (mcuYStart * mcuYSize + index) * 320 * 1 + mcuXStart * mcuXSize * 1;
					pTarget = (uint8_t*)pBuf[0] + index * pitch;

                    // Copy to destination buffer
                    memcpy(pTarget, pSource, width * 1);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        status = utilWriteBMP("./out/fragment.bmp",
                              (uint8_t*)pBuf[0],
                              (uint8_t*)pBuf[1],
                              (uint8_t*)pBuf[2],
                              format,
                              width,
                              height,
                              pitch,
                              pitch>>1);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump fragment buffer failed");

        status = pStream->queueFrag(bufID);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue fragment buffer failed");
    } while(1);

    DPLOGI("Stop fragment processing\n");
    status = pStream->stopFrag();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop fragment processing failed");

    // Verify Y output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pOut, 199 * 199 * 4);

    if (NULL != pOut)
    {
        free(pOut);
        pOut = NULL;
    }

    if (NULL != pImageY)
    {
        free(pImageY);
        pImageY = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("FragStreamGreyCrop", FragStreamGreyCrop, fragStreamCaseGreyCrop)



void fragStreamJPEGCase(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpFragStream   *pStream;
    uint8_t        *pOutput;
    int32_t        bufID;
    DpColorFormat  format;
    void           *pBuf[3];
    uint32_t       size[3];
    int32_t        mcuXStart;
    int32_t        mcuYStart;
    int32_t        mcuXSize;
    int32_t        mcuYSize;
    int32_t        width;
    int32_t        height;
    int32_t        pitch;
    int32_t        index;
    uint8_t        *pSource;
    uint8_t        *pTarget;
	uint32_t 		mcuYCount;

    pStream = new DpFragStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Create fragment stream object failed");

	mcuXSize = 16;
	mcuYSize = 16;

    DPLOGI("Set source information\n");
    status = pStream->setSrcConfig(eI420,
                                   2000,
                                   1200,
                                   mcuXSize,
                                   mcuYSize,
                                   2000,
                                   (DpRect*)NULL);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source information failed");

    DPLOGI("Allocate Y target buffer\n");
    pOutput = (uint8_t*)malloc(sizeof(uint8_t) * 499 * 299 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput != NULL), "Allocate Y target buffer failed");

    pBuf[0] = pOutput;
    pBuf[1] = NULL;
    pBuf[2] = NULL;

    size[0] = 499 * 299 * 3;
    size[1] = 0;
    size[2] = 0;

    DPLOGI("Set target buffer\n");
    status = pStream->setDstBuffer(pBuf,
                                   size,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set target buffer failed");

    DPLOGI("Set target information\n");
    status = pStream->setDstConfig(eRGB888,
                                   499,
                                   299,
                                   499 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Source target information failed");

    DPLOGI("Start fragment processing\n");
    status = pStream->startFrag(&mcuYCount,
                                false);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start fragment processing failed");

    do
    {
        status = pStream->dequeueFrag(&bufID,
                                      &format,
                                      pBuf,
                                      &mcuXStart,
                                      &mcuYStart,
                                      &width,
                                      &height,
                                      &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            break;
        }

        DPLOGI("xStart: %d, yStart: %d, width %d, height %d\n", mcuXStart, mcuYStart, width, height);

        // Decode image
        for (index = 0; index < height; index++)
        {

            switch(format)
            {
                case eI420:
		            pSource = (uint8_t*)(JPEGYInput + (mcuYStart * mcuYSize + index) * 2000 + mcuXStart * mcuXSize);
					pTarget = (uint8_t*)pBuf[0] + index * pitch;
                    memcpy(pTarget, pSource, width);

		            pSource = (uint8_t*)(JPEGUInput + (mcuYStart * mcuYSize + index) * 500 + (mcuXStart * mcuXSize >> 2));
                    pTarget = (uint8_t*)pBuf[1] + index * (pitch >> 2);
                    memcpy(pTarget, pSource, (width >> 2));

					pSource = (uint8_t*)(JPEGVInput + (mcuYStart * mcuYSize + index) * 500 + (mcuXStart * mcuXSize >> 2));
                    pTarget = (uint8_t*)pBuf[2] + index * (pitch >> 2);
                    memcpy(pTarget, pSource, (width >> 2));
                    break;
                default:
                    REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect source buffer format");
                    break;
            }
        }

        status = utilWriteBMP("./out/fragment.bmp",
                              (uint8_t*)pBuf[0],
                              (uint8_t*)pBuf[1],
                              (uint8_t*)pBuf[2],
                              format,
                              width,
                              height,
                              pitch,
                              pitch>>1);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump fragment buffer failed");

        status = utilWriteBin("./out/fragment.bin",
                              (uint8_t*)pBuf[0],
                              (uint8_t*)pBuf[1],
                              (uint8_t*)pBuf[2],
                              format,
                              width,
                              height,
                              pitch,
                              pitch>>1);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump fragment buffer failed");


        status = pStream->queueFrag(bufID);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue fragment buffer failed");
    } while(1);

    DPLOGI("Stop fragment processing\n");
    status = pStream->stopFrag();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop fragment processing failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pOutput, 499 * 299 * 3);

    if (NULL != pOutput)
    {
        free(pOutput);
        pOutput = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("FragStreamJPEG", FragStreamJPEG, fragStreamJPEGCase)


void fragStreamCaseI444(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpFragStream   *pStream;
    uint8_t        *pImageY;
    uint8_t        *pImageU;
    uint8_t        *pImageV;
    uint8_t        *pOutput;
    int32_t        bufID;
    DpColorFormat  format;
    void           *pBuf[3];
    uint32_t       size[3];
    int32_t        mcuXStart;
    int32_t        mcuYStart;
    int32_t        mcuXSize;
    int32_t        mcuYSize;
    int32_t        width;
    int32_t        height;
    int32_t        pitch;
    int32_t        index;
    uint8_t        *pSource;
    uint8_t        *pTarget;
	uint32_t 		mcuYCount;

    pStream = new DpFragStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Create fragment stream object failed");

    mcuXSize = 8;
	mcuYSize = 8;

    DPLOGI("Set source information\n");
    status = pStream->setSrcConfig(eI444,
                                   3264,
                                   2448,
								   mcuXSize,
								   mcuYSize,
                                   3264,
                                   (DpRect*)NULL);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source information failed");

    DPLOGI("Allocate Y target buffer\n");
    pOutput = (uint8_t*)malloc(sizeof(uint8_t) * 3264 * 2448 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput != NULL), "Allocate Y target buffer failed");

    pBuf[0] = pOutput;
    pBuf[1] = 0;
    pBuf[2] = 0;

    size[0] = 3264 * 2448 * 4;
    size[1] = 0;
    size[2] = 0;

    DPLOGI("Set target buffer\n");
    status = pStream->setDstBuffer(pBuf,
                                   size,
                                   1);

    DPLOGI("Set target information\n");
    status = pStream->setDstConfig(eRGBA8888,
                                   3264,
                                   2448,
                                   3264 * 4);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set target information failed");

    DPLOGI("Start fragment processing\n");
    status = pStream->startFrag(&mcuYCount,
                                false);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start fragment processing failed");

    DPLOGI("Allocate Y source buffer\n");
    pImageY = (uint8_t*)malloc(sizeof(uint8_t) * 3264 * 2448);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageY != NULL), "Allocate Y source buffer failed");

    DPLOGI("Allocate U source buffer\n");
    pImageU = (uint8_t*)malloc(sizeof(uint8_t) * 3264 * 2448);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageU != NULL), "Allocate U source buffer failed");

    DPLOGI("Allocate V source buffer\n");
    pImageV = (uint8_t*)malloc(sizeof(uint8_t) * 3264 * 2448);
    REPORTER_ASSERT_MESSAGE(pReporter, (pImageV != NULL), "Allocate V source buffer failed");

    uint64_t start;
    uint64_t end;
    uint64_t total;

    //DP_TIMER_GET_CURRENT_TIME(start);

    DPLOGI("Read bitmap source image\n");
    status = utilReadBMP("./pat/3264x2448.bmp",
                         pImageY,
                         pImageU,
                         pImageV,
                         eI444,
                         3264,
                         2448,
                         3264);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read bitmap source image failed");

    //DP_TIMER_GET_CURRENT_TIME(end);

    //DP_TIMER_GET_TIME_DURATION(start, end, total);

    //printf("Read bitmap time %d\n", DP_TIMER_GET_DURATION_IN_MS(total));

    do
    {
        status = pStream->dequeueFrag(&bufID,
                                      &format,
                                      pBuf,
                                      &mcuXStart,
                                      &mcuYStart,
                                      &width,
                                      &height,
                                      &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            break;
        }

        DPLOGI("xStart: %d, yStart: %d, width %d, height %d\n", mcuXStart, mcuYStart, width, height);

        //DP_TIMER_GET_CURRENT_TIME(start);

        // Decode image
        for (index = 0; index < height; index++)
        {

            switch(format)
            {
				case eI444:
		            pSource = pImageY + (mcuYStart * mcuYSize + index) * 3264 + mcuXStart * mcuXSize;
					pTarget = (uint8_t*)pBuf[0] + index * pitch;
                    memcpy(pTarget, pSource, width);

		            pSource = pImageU + (mcuYStart * mcuYSize + index) * 3264 + mcuXStart * mcuXSize;
                    pTarget = (uint8_t*)pBuf[1] + index * pitch;
                    memcpy(pTarget, pSource, width);

					pSource = pImageV + (mcuYStart * mcuYSize + index) * 3264 + mcuXStart * mcuXSize;
                    pTarget = (uint8_t*)pBuf[2] + index * pitch;
                    memcpy(pTarget, pSource, width);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        //DP_TIMER_GET_CURRENT_TIME(end);

        //DP_TIMER_GET_TIME_DURATION(start, end, total);

        //printf("Fill ring buffer time %d\n", DP_TIMER_GET_DURATION_IN_MS(total));

        status = pStream->queueFrag(bufID);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue fragment buffer failed");
    } while(1);

    DPLOGI("Stop fragment processing\n");
    status = pStream->stopFrag();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop fragment processing failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, pOutput, 3264 * 2448 * 4);

    if (NULL != pOutput)
    {
        free(pOutput);
        pOutput = NULL;
    }

    if (NULL != pImageY)
    {
        free(pImageY);
        pImageY = NULL;
    }

    if (NULL != pImageU)
    {
        free(pImageU);
        pImageU = NULL;
    }

    if (NULL != pImageV)
    {
        free(pImageV);
        pImageV = NULL;
    }

    delete pStream;
}

DEFINE_TEST_CASE("FragStreamI444", FragStreamI444, fragStreamCaseI444)
