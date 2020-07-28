#include "TestDefine.h"
#include "DpBufferPool.h"
#include "DpChannel.h"
#include "DpStream.h"
#include "DpReadBMP.h"

/*
 * Composer Test #0: single output test
 */
void composerTestCase0(TestReporter *pReporter)
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
                                 64,
                                 48,
                                 128,
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
                                  64,
                                  48,
                                  128,
                                  0,     // No rotation,
                                  false,
                                  false, // No TDSHP
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

    DPLOGI("Acquire target buffer");
    status = outBuf.acquireBuffer(&buffID,
                                  &format,
                                  &base[0],
                                  &width,
                                  &height,
                                  &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");
   
    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer");
    status = outBuf.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy target buffer");
    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}


DEFINE_TEST_CASE("ComposerTest0", ComposerTest0, composerTestCase0)


/*
 * Composer Test #1: multiple output test
 */
void composerTestCase1(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAutoBufferPool  inBuf;
    DpAutoBufferPool  outBuf0;
    DpAutoBufferPool  outBuf1;
    DpAutoBufferPool  outBuf2;
    DpAutoBufferPool  outBuf3;
    DpAutoBufferPool  outBuf4;
    DpAutoBufferPool  outBuf5;
    DpAutoBufferPool  outBuf6;
    DpAutoBufferPool  outBuf7;
    DpAutoBufferPool  outBuf8;
    DpAutoBufferPool  outBuf9;
    DpAutoBufferPool  outBuf10;
    DpAutoBufferPool  outBuf11;
    DpAutoBufferPool  outBuf12;
    DpAutoBufferPool  outBuf13;
    DpChannel         channel;
    DpStream          stream;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           width;
    int32_t           height;
    int32_t           pitch;
    int32_t           chanID;
	int32_t			  loopIndex;

	DpAutoBufferPool* outBufArray[14]  = { 
                                             &outBuf0, &outBuf1, &outBuf2, &outBuf3,  &outBuf4,  &outBuf5,  &outBuf6,
										     &outBuf7, &outBuf8, &outBuf9, &outBuf10, &outBuf11, &outBuf12, &outBuf13
										 };

    DPLOGI("Create source buffer\n");
    status = inBuf.createBuffer(eI420,
                                1920,
                                1080,
                                1920,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eI420,
                                   1920,
                                   1080,
                                   1920,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");


    DPLOGI("Create target buffer0\n");
    status = outBuf0.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer0 failed");

    DPLOGI("Add target port0\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,     // No rotation
                                   false, // No flip
                                   false,
                                   outBuf0);   
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port0 failed");

    DPLOGI("Create target buffer1\n");
    status = outBuf1.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer1 failed");

    DPLOGI("Add target port1\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port1 failed");

    DPLOGI("Create target buffer2\n");
    status = outBuf2.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer2 failed");

    DPLOGI("Add target port2\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port2 failed");

    DPLOGI("Create target buffer3\n");
    status = outBuf3.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer3 failed");

    DPLOGI("Add target port3\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port3 failed");

    DPLOGI("Create target buffer4\n");
    status = outBuf4.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer4 failed");

    DPLOGI("Add target port4\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf4);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port4 failed");

    DPLOGI("Create target buffer5\n");
    status = outBuf5.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer5 failed");

    DPLOGI("Add target port5\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf5);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port5 failed");

    DPLOGI("Create target buffer6\n");
    status = outBuf6.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer5 failed");

    DPLOGI("Add target port6\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf6);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port6 failed");

    DPLOGI("Create target buffer7\n");
    status = outBuf7.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer7 failed");

    DPLOGI("Add target port7\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf7);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port7 failed");

    DPLOGI("Create target buffer8\n");
    status = outBuf8.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer8 failed");

    DPLOGI("Add target port8\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf8);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port8 failed");

    DPLOGI("Create target buffer9\n");
    status = outBuf9.createBuffer(eRGB565,
                                  640,
                                  480,
                                  1280,
                                  1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer9 failed");

    DPLOGI("Add target port9\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf9);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port9 failed");

    DPLOGI("Create target buffer10\n");
    status = outBuf10.createBuffer(eRGB565,
                                   640,
                                   480,
                                   1280,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer10 failed");

    DPLOGI("Add target port10\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf10);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port10 failed");

    DPLOGI("Create target buffer11\n");
    status = outBuf11.createBuffer(eRGB565,
                                   640,
                                   480,
                                   1280,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer11 failed");

    DPLOGI("Add target port11\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf11);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port11 failed");

    DPLOGI("Create target buffer12\n");
    status = outBuf12.createBuffer(eRGB565,
                                   640,
                                   480,
                                   1280,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer12 failed");

    DPLOGI("Add target port12\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf12);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port12 failed");

    DPLOGI("Create target buffer13\n");
    status = outBuf13.createBuffer(eRGB565,
                                   640,
                                   480,
                                   1280,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer13 failed");

    DPLOGI("Add target port13\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   640,
                                   480,
                                   1280,
                                   0,      // No rotation
                                   false,  // No flip
                                   false,
                                   outBuf13);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port13 failed");

    DPLOGI("Add stream channel\n");
    // Add the channel
    status = stream.addChannel(channel, &chanID);
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
            status = utilReadBMP((char*)"./pat/1920x1080.bmp",
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
    status = inBuf.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

	for(loopIndex = 0; loopIndex < 14; loopIndex++)
	{
    	status = outBufArray[loopIndex]->acquireBuffer(&buffID,
    	                                               &format,
    	                                               &base[0],
    	                                               &width,
    	                                               &height,
    	                                               &pitch);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer failed");

        // Verify output buffer
        REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

        status = outBufArray[loopIndex]->releaseBuffer(buffID);
	    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer failed");
    
        status = outBufArray[loopIndex]->destroyBuffer();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");
    }

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}

DEFINE_TEST_CASE("ComposerTest1", ComposerTest1, composerTestCase1)


/*
 * Composer Test #2: scale down < 1/64 test
 */
void composerTestCase2(TestReporter *pReporter)
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
                                1920,
                                1080,
                                1920,
                                1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer\n");
    status = outBuf.createBuffer(eRGB565,
                                 28,
                                 28,
                                 56,
                                 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eI420,
                                   1920,
                                   1080,
                                   1920,
                                   inBuf);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB565,
                                   28,
                                   28,
                                   56,
                                   0,      // No rotation
                                   false,  // No flip
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
            status = utilReadBMP((char*)"./pat/1920x1080.bmp",
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
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

    DPLOGI("Acquire target buffer\n");
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

    DPLOGI("Destroy target buffer\n");
    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    DPLOGI("Destroy source buffer\n");
    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}

DEFINE_TEST_CASE("ComposerTest2", ComposerTest2, composerTestCase2)


/*
 * Composer Test #3: TDSHP path test
 */
void composerTestCase3(TestReporter *pReporter)
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
    status = outBuf.createBuffer(eRGB888,
                                 640,
                                 480,
                                 1920,
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
                                   eRGB888,
                                   640,
                                   480,
                                   1920,
                                   0,  // No rotation,
                                   false,
                                   true,
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
    stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

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

    status = outBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer failed");

    status = inBuf.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}


DEFINE_TEST_CASE("ComposerTest3", ComposerTest3, composerTestCase3)


/*
 * Composer Test #4: 12MP 1 in 2 out test
 */
void composerTestCase4(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAutoBufferPool  srcPool;
    DpAutoBufferPool  dstPool0;
    DpAutoBufferPool  dstPool1;
    DpChannel         channel;
    DpStream          stream;
    int32_t           firstID;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           width;
    int32_t           height;
    int32_t           pitch;

    DPLOGI("Create source buffer\n");
    status = srcPool.createBuffer(eUYVY,
                                   3264,
                                   2448,
                                   6528,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer0\n");
    status = dstPool0.createBuffer(eUYVY,
                                    3264,
                                    2448,
                                    6528,
                                    1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer0 failed");

    DPLOGI("Create target buffer1\n");
    status = dstPool1.createBuffer(eRGB888,
                                   640,
                                   480,
                                   640 * 3,
                                   1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer1 failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eUYVY,
                                   3264,
                                   2448,
                                   6528,
                                   srcPool);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port0\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eUYVY,
                                   3264,
                                   2448,
                                   6528,
                                   0,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   dstPool0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port0 failed");

    DPLOGI("Add target port1\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB888,
                                   640,
                                   480,
                                   640 * 3,
                                   0,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   dstPool1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port1 failed");

    DPLOGI("Add stream channel\n");
    status = stream.addChannel(channel,
                               &firstID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add stream channel failed");

    DPLOGI("Start stream\n");
    status = stream.startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start stream failed");

    DPLOGI("Dequeue source buffer\n");
    // Dequeue the buffer
    status = srcPool.dequeueBuffer(&buffID,
                                   &format,
                                   &base[0],
                                   &width,
                                   &height,
                                   &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source buffer failed");

    // Decode image
    switch(format)
    {
        case eUYVY:
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
    status = srcPool.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");    

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

    DPLOGI("Acquire target buffer\n");
    status = dstPool0.acquireBuffer(&buffID,
                                    &format,
                                    &base[0],
                                    &width,
                                    &height,
                                    &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer0 failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer\n");
    status = dstPool0.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer0 failed");

    DPLOGI("Acquire target buffer\n");
    status = dstPool1.acquireBuffer(&buffID,
                                    &format,
                                    &base[0],
                                    &width,
                                    &height,
                                    &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer1 failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer\n");
    dstPool1.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer1 failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy target buffer0\n");
    status = dstPool0.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer0 failed");

    DPLOGI("Destroy target buffer1\n");
    status = dstPool1.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer1 failed");

    DPLOGI("Destroy source buffer\n");
    status = srcPool.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}

DEFINE_TEST_CASE("ComposerTest4", ComposerTest4, composerTestCase4)


/*
 * Composer Test #5: queue and dequeue test
 */
void composerTestCase5(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAutoBufferPool  srcPool;
    DpAutoBufferPool  dstPool0;
    DpAutoBufferPool  dstPool1;
    DpChannel         channel;
    DpStream          stream;
    int32_t           firstID;
    int32_t           buffID;
    DpColorFormat     format;
    uint32_t          base[3];
    int32_t           width;
    int32_t           height;
    int32_t           pitch;

    DPLOGI("Create source buffer\n");
    status = srcPool.createBuffer(eUYVY,
                                   3264,
                                   2448,
                                   6528,
                                   2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create source buffer failed");

    DPLOGI("Create target buffer0\n");
    status = dstPool0.createBuffer(eUYVY,
                                    3264,
                                    2448,
                                    6528,
                                    2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer0 failed");

    DPLOGI("Create target buffer1\n");
    status = dstPool1.createBuffer(eRGB888,
                                   640,
                                   480,
                                   640 * 3,
                                   2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Create target buffer1 failed");

    DPLOGI("Set source port\n");
    status = channel.setSourcePort(DpChannel::eMEMORY_PORT,
                                   eUYVY,
                                   3264,
                                   2448,
                                   6528,
                                   srcPool);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Set source port failed");

    DPLOGI("Add target port0\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eUYVY,
                                   3264,
                                   2448,
                                   6528,
                                   0,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   dstPool0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port0 failed");

    DPLOGI("Add target port1\n");
    status = channel.addTargetPort(DpChannel::eMEMORY_PORT,
                                   eRGB888,
                                   640,
                                   480,
                                   640 * 3,
                                   0,
                                   false,  // No flip
                                   false,  // No TDSHP
                                   dstPool1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add target port1 failed");

    DPLOGI("Add stream channel\n");
    status = stream.addChannel(channel,
                               &firstID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Add stream channel failed");

    DPLOGI("Start stream\n");
    status = stream.startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start stream failed");

    DPLOGI("Dequeue source buffer\n");
    // Dequeue the buffer
    status = srcPool.dequeueBuffer(&buffID,
                                   &format,
                                   &base[0],
                                   &width,
                                   &height,
                                   &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source buffer failed");

    // Decode image
    switch(format)
    {
        case eUYVY:
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
    status = srcPool.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Wait stream idle failed");

    DPLOGI("Acquire target buffer0\n");
    status = dstPool0.acquireBuffer(&buffID,
                                    &format,
                                    &base[0],
                                    &width,
                                    &height,
                                    &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer0 failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    status = dstPool0.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer0 failed");

    DPLOGI("Acquire target buffer1\n");
    status = dstPool1.acquireBuffer(&buffID,
                                    &format,
                                    &base[0],
                                    &width,
                                    &height,
                                    &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire target buffer1 failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer1\n");
    status = dstPool1.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release target buffer1 failed");    


    DPLOGI("Dequeue source buffer1\n");
    status = srcPool.dequeueBuffer(&buffID,
                                   &format,
                                   &base[0],
                                   &width,
                                   &height,
                                   &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source buffer failed");  

    // Decode image
    switch(format)
    {
        case eUYVY:
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
    status = srcPool.queueBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source buffer failed");

    DPLOGI("Wait stream idle\n");
    status = stream.waitStream();

    DPLOGI("Acquire target buffer0\n");
    status = dstPool0.acquireBuffer(&buffID,
                                    &format,
                                    &base[0],
                                    &width,
                                    &height,
                                    &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire tearget buffer0 failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer0\n");
    status = dstPool0.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release tearget buffer0 failed");

    DPLOGI("Acquire target buffer1\n");
    dstPool1.acquireBuffer(&buffID,
                           &format,
                           &base[0],
                           &width,
                           &height,
                           &pitch);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Acquire tearget buffer1 failed");

    // Verify output buffer
    REPORTER_VERIFY_BIT_TRUE(pReporter, base[0], pitch * height);

    DPLOGI("Release target buffer1\n");
    dstPool1.releaseBuffer(buffID);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Release tearget buffer1 failed");

    DPLOGI("Stop stream\n");
    status = stream.stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop stream failed");

    DPLOGI("Destroy target buffer0\n");
    status = dstPool0.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer0 failed");

    DPLOGI("Destroy target buffer1\n");
    status = dstPool1.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy target buffer1 failed");

    DPLOGI("Destroy source buffer\n");
    status = srcPool.destroyBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Destroy source buffer failed");
}

DEFINE_TEST_CASE("ComposerTest5", ComposerTest5, composerTestCase5)
