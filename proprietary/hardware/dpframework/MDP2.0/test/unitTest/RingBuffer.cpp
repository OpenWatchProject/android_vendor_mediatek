#include "TestDefine.h"
#include "DpChannel.h"
#include "DpStream.h"
#include "DpReadBMP.h"

void ringBufferCase0(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpRingBufferPool  inBuf;
    DpAutoBufferPool  outBuf;
    DpChannel         channel;
    DpStream          stream;
    uint8_t           *pImage;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           XStart;
    int32_t           YStart;
    int32_t           width;
    int32_t           height;
    int32_t           pitch;
    int32_t           chanID;
    int32_t           index;
    uint8_t           *pSource;
    uint8_t           *pTarget;

    // 1st input and output
    status = inBuf.createBuffer(16,
                                eRGB888,
                                640,
                                480,
                                1920,
                                256,
                                200);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    status = outBuf.createBuffer(eRGB565,
                                 256,
                                 200,
                                 512,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer failed");

    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                  eRGB888,
                                  640,
                                  480,
                                  1920,
                                  inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   256,
                                   200,
                                   512,
                                   0,
                                   false,
                                   false,
                                   outBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port failed");

    status = stream.addChannel(channel,
                               &chanID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add stream channel failed");

    pImage = (uint8_t*)malloc(1920 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (NULL != pImage), "Malloc source buffer failed");

    status = utilReadBMP((char*)"./pat/640x480.bmp",
                         pImage,
                         NULL,
                         NULL,
                         eRGB888,
                         640,
                         480,
                         1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed");

    status = stream.startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start stream failed");

    do
    {
        // Dequeue the buffer
        status = inBuf.dequeueBuffer(&buffID,
                                     &format,
                                     &base[0],
                                     &XStart,
                                     &YStart,
                                     &width,
                                     &height,
                                     &pitch);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_BUFFER_DONE == status), "Dequeue source buffer failed");
            break;
        }

        // Decode image
        for (index = 0; index < height; index++)
        {
            pSource = pImage + (YStart + index) * 1920 + XStart * 3;

            switch(format)
            {
                case eRGB888:
                    pTarget = (uint8_t*)(base[0] + index * pitch);

                    // Copy to destination buffer
                    memcpy(pTarget, pSource, width * 3);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        // Queue the buffer
        status = inBuf.queueBuffer(buffID);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");
    }  while(1);

    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");

    // Verify target buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");

    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    if (NULL != pImage)
    {
        free(pImage);
        pImage = NULL;
    }
}

DEFINE_TEST_CASE("RingBuffer0", RingBuffer0, ringBufferCase0)