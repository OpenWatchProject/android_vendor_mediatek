#include "TestDefine.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpChannel.h"
#include "DpStream.h"

void autoBufferCase0(TestReporter *pReporter)
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

    DPLOGI("Start autoBufferCase0\n");

    DPLOGI("Create input buffer\n");

    // 1st input and output
    status = inBuf.createBuffer(eYV12,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create input buffer failed");

    DPLOGI("Create output buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 64,
                                 48,
                                 128,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create output buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eYV12,
                                   640,
                                   480,
                                   640,
                                   320,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   64,
                                   48,
                                   128,
                                   0,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   outBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port failed");

    DPLOGI("Add stream channel\n");
    stream.addChannel(channel,
                      &chanID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add stream channel failed");

    DPLOGI("Start stream\n");
    status = stream.startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start stream failed");

    DPLOGI("Dequeue buffer\n");
    // Dequeue the buffer
    status = inBuf.dequeueBuffer(&buffID,
                                 &format,
                                 &base[0],
                                 &width,
                                 &height,
                                 &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue buffer failed");

    // Decode image
    switch(format)
    {
        case eYV12:
            DPLOGI("Read bitmap source\n");
            status = utilReadBMP((char*)"./pat/640x480.bmp",
                                 (uint8_t*)base[0],
                                 (uint8_t*)base[1],
                                 (uint8_t*)base[2],
                                 format,
                                 width,
                                 height,
                                 pitch);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image failed");
            break;
        default:
            REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect input buffer format");
            break;
    }

    DPLOGI("Queue buffer\n");
    // Queue the buffer
    status = inBuf.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");
}

DEFINE_TEST_CASE("AutoBuffer0", AutoBuffer0, autoBufferCase0)
