#include "TestDefine.h"
#include "DpBufferPool.h"
#include "DpChannel.h"
#include "DpStream.h"
#include "DpReadBMP.h"


/*
 * Composer Test #0: 1 x 1 output
 */
void boundaryTestCase0(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAutoBufferPool  inBuf;
    DpAutoBufferPool  outBuf;
    DpChannel         channel;
    DpStream          stream;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           width;
    int32_t           height;
    int32_t           pitch;
    int32_t           chanID;
    int32_t           index;

    DPLOGI("Create source buffer\n");
    status = inBuf.createBuffer(eRGB888,
                                640,
                                960,
                                640 * 3,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGBX8888,
                                 1,
                                 1,
                                 4,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eRGB888,
                                   640,
                                   960,
                                   640 * 3,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGBX8888,
                                   1,
                                   1,
                                   4,
                                   270,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   outBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port failed");

    DPLOGI("Add stream channel\n");
    status = stream.addChannel(channel,
                               &chanID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add stream channel failed");

    DPLOGI("Start stream\n");
    status = stream.startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start stream failed");

    DPLOGI("Dequeue source buffer\n");
    // Dequeue the buffer
    status = inBuf.dequeueBuffer(&buffID,
                                 &format,
                                 &base[0],
                                 &width,
                                 &height,
                                 &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dqueue source buffer failed");

    // Decode image
    switch(format)
    {
        case eRGB888:
            for (index = 0; index < (640 * 480 * 3); index += 3)
            {
                (((uint8_t*)base[0])[index + 0]) = 0xFF;
                (((uint8_t*)base[0])[index + 1]) = 0x00;
                (((uint8_t*)base[0])[index + 2]) = 0x00;
            }
            break;
        default:
            REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect input buffer format");
            break;
    }

    DPLOGI("Queue source buffer\n");
    // Queue the buffer
    status = inBuf.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy target buffer\n");
    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}

DEFINE_TEST_CASE("BoundaryTest0", BoundaryTest0, boundaryTestCase0)



/*
 * Composer Test #0: 3 x 3 output
 */
void boundaryTestCase1(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAutoBufferPool  inBuf;
    DpAutoBufferPool  outBuf;
    DpChannel         channel;
    DpStream          stream;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           width;
    int32_t           height;
    int32_t           pitch;
    int32_t           chanID;
    int32_t           index;

    DPLOGI("Create source buffer\n");
    status = inBuf.createBuffer(eRGB888,
                                640,
                                960,
                                640 * 3,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGBX8888,
                                 3,
                                 3,
                                 12,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eRGB888,
                                   640,
                                   960,
                                   640 * 3,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGBX8888,
                                   3,
                                   3,
                                   12,
                                   270,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   outBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port failed");

    DPLOGI("Add stream channel\n");
    status = stream.addChannel(channel,
                               &chanID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add stream channel failed");

    DPLOGI("Start stream\n");
    status = stream.startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start stream failed");

    DPLOGI("Dequeue source buffer\n");
    // Dequeue the buffer
    status = inBuf.dequeueBuffer(&buffID,
                                 &format,
                                 &base[0],
                                 &width,
                                 &height,
                                 &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dqueue source buffer failed");

    // Decode image
    switch(format)
    {
        case eRGB888:
            for (index = 0; index < (640 * 480 * 3); index += 3)
            {
                (((uint8_t*)base[0])[index + 0]) = 0xFF;
                (((uint8_t*)base[0])[index + 1]) = 0x00;
                (((uint8_t*)base[0])[index + 2]) = 0x00;
            }
            break;
        default:
            REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect input buffer format");
            break;
    }

    DPLOGI("Queue source buffer\n");
    // Queue the buffer
    status = inBuf.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy target buffer\n");
    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}

DEFINE_TEST_CASE("BoundaryTest1", BoundaryTest1, boundaryTestCase1)
