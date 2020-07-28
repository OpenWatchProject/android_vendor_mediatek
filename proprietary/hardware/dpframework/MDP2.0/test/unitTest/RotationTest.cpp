#include "TestDefine.h"
#include "DpBufferPool.h"
#include "DpChannel.h"
#include "DpStream.h"
#include "DpReadBMP.h"

/*
 * Composer Test #0: flip without rotation
 */
void rotationTestCase0(TestReporter *pReporter)
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
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 640,
                                 480,
                                 1280,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eI420,
                                   640,
                                   480,
                                   640,
                                   320,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,
                                   true,
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed");

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");

    DPLOGI("Destroy target buffer\n");
    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    DPLOGI("Stream done\n");
}

DEFINE_TEST_CASE("RotationTest0", RotationTest0, rotationTestCase0)



/*
 * Composer Test #1: rotate 90 degree without flip
 */
void rotationTestCase1(TestReporter *pReporter)
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
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 480,
                                 640,
                                 960,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create output buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eI420,
                                   640,
                                   480,
                                   640,
                                   320,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   480,
                                   640,
                                   960,
                                   90,
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source buffer failed"); 

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed"); 

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");

    DPLOGI("Destroy target buffer\n");
    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    DPLOGI("Stream done\n");
}

DEFINE_TEST_CASE("RotationTest1", RotationTest1, rotationTestCase1)



/*
 * Composer Test #2: rotate 90 degree and flip
 */
void rotationTestCase2(TestReporter *pReporter)
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
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 480,
                                 640,
                                 960,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                          eI420,
                          640,
                          480,
                          640,
                          320,
                          inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   480,
                                   640,
                                   960,
                                   90,
                                   true,
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed");

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");

    DPLOGI("Destroy target buffer\n");
    outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");
}

DEFINE_TEST_CASE("RotationTest2", RotationTest2, rotationTestCase2)



/*
 * Composer Test #3: rotate 180 degree without flip
 */
void rotationTestCase3(TestReporter *pReporter)
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
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 640,
                                 480,
                                 1280,
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

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   180,
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

    DPLOGI("Dequeue buffer\n");
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

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");

    DPLOGI("Destroy target buffer\n");
    outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");
}


DEFINE_TEST_CASE("RotationTest3", RotationTest3, rotationTestCase3)



/*
 * Composer Test #4: rotate 180 degree and flip
 */
void rotationTestCase4(TestReporter *pReporter)
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
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 640,
                                 480,
                                 1280,
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

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   180,
                                   true,
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed"); 

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed"); 

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed"); 

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed"); 

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


DEFINE_TEST_CASE("RotationTest4", RotationTest4, rotationTestCase4)


/*
 * Composer Test #5: rotate 270 degree without flip
 */
void rotationTestCase5(TestReporter *pReporter)
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
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 480,
                                 640,
                                 960,
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

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   480,
                                   640,
                                   960,
                                   270,
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed"); 

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed"); 

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed"); 

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed"); 

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

DEFINE_TEST_CASE("RotationTest5", RotationTest5, rotationTestCase5)


/*
 * Composer Test #6: rotate 270 degree and flip
 */
void rotationTestCase6(TestReporter *pReporter)
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
    // 1st input and output
    status = inBuf.createBuffer(eI420,
                                640,
                                480,
                                640,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 480,
                                 640,
                                 960,
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

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   480,
                                   640,
                                   960,
                                   270,
                                   true,
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream failed");

    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source failed");

    DPLOGI("Destroy target buffer\n");
    outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target failed");
}

DEFINE_TEST_CASE("RotationTest6", RotationTest6, rotationTestCase6)
