#include "TestDefine.h"
#include "DpBufferPool.h"
#include "DpChannel.h"
#include "DpStream.h"
#include "DpReadBMP.h"

/*
 * Composer Test #0: Test resize with crop
 */
void sourceCropCase0(TestReporter *pReporter)
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

    DPLOGI("Create source buffer\n");

    // 1st source and target
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 320,
                                 240,
                                 640,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eI420,
                                   640,
                                   480,
                                   640,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Set source crop\n");
    status = channel.setSourceCrop(150,
                                   150,
                                   340,
                                   180);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source crop failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   320,
                                   240,
                                   640,
                                   0,
                                   false,
                                   false,
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source buffer failed");

    // Decode image
    switch(format)
    {
        case eI420:
            DPLOGI("Read bitmap source\n");
            status = utilReadBMP((char*)"./pat/640x480.bmp",
                                 (uint8_t*)base[0],
                                 (uint8_t*)base[1],
                                 (uint8_t*)base[2],
                                 format,
                                 width,
                                 height,
                                 pitch);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed");
            break;
        default:
            REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect source buffer format");
            break;
    }

    DPLOGI("Queue source buffer\n");
    // Queue the buffer
    status = inBuf.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed");

    DPLOGI("Acquire target buffer\n");
	status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire buffer failed");

    // Verify target buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer\n");
    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release buffer failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");  

    DPLOGI("Destroy target buffer\n");
    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");  
}

DEFINE_TEST_CASE("SourceCrop0", SourceCrop0, sourceCropCase0)


void sourceCropCase1(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAutoBufferPool  inBuf;
    DpAutoBufferPool  outBuf1;
    DpAutoBufferPool  outBuf2;
    DpChannel         channel;
    DpStream          stream;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           width;
    int32_t           height;
    int32_t           pitch;
    int32_t           chanID;

    DPLOGI("Create source buffer\n");
    // 1st source and target
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer0\n");
    status = outBuf1.createBuffer(eRGB565,
                                  480,
                                  800,
                                  960,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer0 failed");

    DPLOGI("Create target buffer1\n");
    status = outBuf2.createBuffer(eRGB565,
                                  320,
                                  240,
                                  640,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer1 failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eI420,
                                   640,
                                   480,
                                   640,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Set source crop\n");
    status = channel.setSourceCrop(40,
                                   40,
                                   560,
                                   400);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source crop failed");

    DPLOGI("Add target port0\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   480,
                                   800,
                                   960,
                                   90,
                                   true,
                                   false,  // No TDSHP
                                   outBuf1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port0 failed");

    DPLOGI("Add target port1\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   320,
                                   240,
                                   640,
                                   0,
                                   false,
                                   false,  // No TDSHP
                                   outBuf2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port1 failed");

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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source buffer failed");

    // Decode image
    switch(format)
    {
        case eI420:
            DPLOGI("Read bitmap source\n");
            status = utilReadBMP((char*)"./pat/640x480.bmp",
                                 (uint8_t*)base[0],
                                 (uint8_t*)base[1],
                                 (uint8_t*)base[2],
                                 format,
                                 width,
                                 height,
                                 pitch);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed");
            break;
        default:
            REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect source buffer format");
            break;
    }

    DPLOGI("Queue source buffer\n");
    // Queue the buffer
    status = inBuf.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed");

    DPLOGI("Acquire target buffer0\n");
	status = outBuf1.acquireBuffer(&buffID,
                                   &format,
                                   &base[0],
                                   &width,
                                   &height,
                                   &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer0 failed");

    // Verify target buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer0\n");
    status = outBuf1.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer0 failed");

    DPLOGI("Acquire target buffer1\n");
    status = outBuf2.acquireBuffer(&buffID,
                                   &format,
                                   &base[0],
                                   &width,
                                   &height,
                                   &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer1 failed");

    // Verify target buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer1\n");
    status = outBuf2.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer1 failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");  

    DPLOGI("Destroy target buffer0\n");
    status = outBuf1.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer0 failed");
    
    DPLOGI("Destroy target buffer1\n");
    status = outBuf2.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer1 failed");
}

DEFINE_TEST_CASE("SourceCrop1", SourceCrop1, sourceCropCase1)


void sourceCropCase2(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAutoBufferPool  inBuf;
    DpAutoBufferPool  outBuf1;
    DpAutoBufferPool  outBuf2;
    DpChannel         channel;
    DpStream          stream;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           width;
    int32_t           height;
    int32_t           pitch;
    int32_t           chanID;

    DPLOGI("Create source buffer\n");

    // 1st source and target
    status = inBuf.createBuffer(eI444,
                                3264,
                                2448,
                                3264,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer0\n");
    status = outBuf1.createBuffer(eYUYV,
                                  3264,
                                  2448,
                                  6528,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer0 failed");

    DPLOGI("Create target buffer1\n");
    status = outBuf2.createBuffer(eRGB888,
                                  640,
                                  480,
                                  1920,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer1 failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eI444,
                                   3264,
                                   2448,
                                   3264,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Set source crop\n");
    status = channel.setSourceCrop(512,
                                   512,
                                   2240,
                                   1424);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source crop failed");

    DPLOGI("Add target port0\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eYUYV,
                                   3264,
                                   2448,
                                   6528,
                                   0,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   outBuf1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port0 failed");

    DPLOGI("Add target port1\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB888,
                                   640,
                                   480,
                                   1920,
                                   0,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   outBuf2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port1 failed");

    DPLOGI("Add stream channel\n");
    status = stream.addChannel(channel,
                               &chanID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add channel failed");

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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source buffer failed");

    // Decode image
    switch(format)
    {
        case eI444:
            DPLOGI("Read bitmap source\n");
            status = utilReadBMP((char*)"./pat/3264x2448.bmp",
                                 (uint8_t*)base[0],
                                 (uint8_t*)base[1],
                                 (uint8_t*)base[2],
                                 format,
                                 width,
                                 height,
                                 pitch);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed");
            break;
        default:
            REPORTER_ASSERT_MESSAGE(pReporter, 0, "Incorrect source buffer format");
            break;
    }

    DPLOGI("Queue source buffer\n");
    // Queue the buffer
    status = inBuf.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed");

    DPLOGI("Acquire target buffer0\n");
    status = outBuf1.acquireBuffer(&buffID,
                                   &format,
                                   &base[0],
                                   &width,
                                   &height,
                                   &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer0 failed");

    // Verify target buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer0\n");
    status = outBuf1.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer0 failed");

    DPLOGI("Acquire target buffer1\n");
    status = outBuf2.acquireBuffer(&buffID,
                                   &format,
                                   &base[0],
                                   &width,
                                   &height,
                                   &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer1 failed");

    // Verify target buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer1\n");
    status = outBuf2.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer1 failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");  

    DPLOGI("Destroy target buffer0\n");
    status = outBuf1.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer0 failed");
    
    DPLOGI("Destroy target buffer1\n");
    status = outBuf2.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer1 failed");
}

DEFINE_TEST_CASE("SourceCrop2", SourceCrop2, sourceCropCase2)
