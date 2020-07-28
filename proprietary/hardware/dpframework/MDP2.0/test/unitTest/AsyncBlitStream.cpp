#include "TestDefine.h"
#include "DpAsyncBlitStream.h"
#include "DpReadBMP.h"
#include "DpWriteBMP.h"
#include "DpWriteBin.h"
#include "DpSync.h"

#include <pthread.h>
#include <sync.h>
#include <sw_sync.h>

#ifdef WIN32
#include"windows.h"
#define sleep(n) Sleep(n)
#endif

// Minimum Y pitch that is acceptable by HW
#define DP_COLOR_GET_MIN_Y_PITCH(color, width)                                                                  \
    (((DP_COLOR_BITS_PER_PIXEL(color) * width) + 4) >> 3)

// Minimum UV pitch that is acceptable by HW
#define DP_COLOR_GET_MIN_UV_PITCH(color, width)                                                                 \
    ((1 == DP_COLOR_GET_PLANE_COUNT(color))? 0:                                                                 \
     (((0 == DP_COLOR_IS_UV_COPLANE(color)) || (DP_COLOR_420_BLKP == color) || (DP_COLOR_420_BLKP_UFO == color) || (DP_COLOR_420_BLKI == color))?   \
     (DP_COLOR_GET_MIN_Y_PITCH(color, width)  >> DP_COLOR_GET_H_SUBSAMPLE(color)):                              \
     ((DP_COLOR_GET_MIN_Y_PITCH(color, width) >> DP_COLOR_GET_H_SUBSAMPLE(color)) * 2)))

class DpTestImageBuffer
{
public:
    DpTestImageBuffer() : m_colorFormat(DP_COLOR_UNKNOWN), m_width(0), m_height(0), m_aligned(64)
    {
        memset(m_pBuffer, 0, sizeof(m_pBuffer));
        memset(m_pAlignedBuffer, 0, sizeof(m_pAlignedBuffer));
        memset(m_size, 0, sizeof(m_size));
    }
    ~DpTestImageBuffer()
    {
        freeBuffer(0);
        freeBuffer(1);
        freeBuffer(2);
    }

    DpTestImageBuffer& setColorFormat(DpColorFormat format){ m_colorFormat = format; return *this; }
    DpTestImageBuffer& setWidth(int32_t width){ m_width = width; return *this; }
    DpTestImageBuffer& setHeight(int32_t height){ m_height = height; return *this; }
    DpTestImageBuffer& setAligned(int32_t aligned){ m_aligned= aligned; return *this; }
    DpTestImageBuffer& allocBuffer()
    {
        int ySize = 0;
        int uSize = 0;
        int vSize = 0;

        if (DP_COLOR_GET_BLOCK_MODE(m_colorFormat))
        {
            if (1 == plane())
            {
                ySize = (width() * height() * DP_COLOR_BITS_PER_PIXEL(m_colorFormat) >> 9);
            }
            else if (2 == plane())
            {
                ySize = (width() * height() * DP_COLOR_BITS_PER_PIXEL(m_colorFormat) >> 8);
                uSize = (width() * height() * DP_COLOR_BITS_PER_PIXEL(m_colorFormat) >> 8) >> 1;
            }
        }
        else
        {
            switch(plane())
            {
                case 3:
                    vSize = (uvPitch() * height()) >> DP_COLOR_GET_V_SUBSAMPLE(m_colorFormat);
                case 2:
                    uSize = (uvPitch() * height()) >> DP_COLOR_GET_V_SUBSAMPLE(m_colorFormat);
                case 1:
                    ySize = yPitch() * height();
            }
        }

        return allocY(ySize).allocU(uSize).allocV(vSize);
    }

    DpTestImageBuffer& allocY(uint32_t size){ return alloc(0, size); }
    DpTestImageBuffer& allocU(uint32_t size){ return alloc(1, size); }
    DpTestImageBuffer& allocV(uint32_t size){ return alloc(2, size); }

    DpColorFormat colorFormat() {return m_colorFormat;}
    int32_t width() {return m_width;}
    int32_t height() {return m_height;}
    int32_t yPitch() {return DP_COLOR_GET_MIN_Y_PITCH(m_colorFormat, m_width);}
    int32_t uvPitch() {return DP_COLOR_GET_MIN_UV_PITCH(m_colorFormat, m_width);}
    int32_t plane() {return DP_COLOR_GET_PLANE_COUNT(m_colorFormat);}

    void* y() {return m_pAlignedBuffer[0];}
    void* u() {return m_pAlignedBuffer[1];}
    void* v() {return m_pAlignedBuffer[2];}

    uint32_t ySize() {return m_size[0];}
    uint32_t uSize() {return m_size[1];}
    uint32_t vSize() {return m_size[2];}

    void** yuv() { return (void**)m_pAlignedBuffer; }
    uint32_t* yuvSize() { return (uint32_t*)m_size; }

private:
    DpTestImageBuffer& alloc(int index, uint32_t size)
    {
        if (size > 0)
        {
            m_pBuffer[index] = malloc(size + m_aligned);
            m_pAlignedBuffer[index] = (void*)((unsigned long)m_pBuffer[index] + m_aligned- ((unsigned long)m_pBuffer[index]%m_aligned));
            m_size[index] = size;
        }

        return *this;
    }

    DpTestImageBuffer& freeBuffer(int index)
    {

        if (m_pBuffer[index])
        {
            free(m_pBuffer[index]);
            m_size[index] = 0;
        }

        return *this;
    }

    DpColorFormat   m_colorFormat;
    int32_t         m_width;
    int32_t         m_height;
    int32_t         m_aligned;

    void            *m_pBuffer[3];
    void            *m_pAlignedBuffer[3];
    uint32_t        m_size[3];
};

void* consumerThread(void* para)
{
    int32_t* fenceFD = (int32_t*)para;

    // Wake up
    sync_wait(*fenceFD, -1);
    DPLOGE("First frame is waited\n");
    close(*fenceFD);

    sync_wait(*(fenceFD+1), -1);
    DPLOGE("2nd frame is waited\n");
    close(*(fenceFD+1));

    return NULL;
}

void asyncBlitStreamCaseRGB888(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAsyncBlitStream *pStream;
    uint32_t          job[2];
    int32_t           fence[2];
    pthread_t         tid;
    uint32_t          indexLoop = 0;

    DpTestImageBuffer inBuffer;
    DpTestImageBuffer outBuffer;

    inBuffer.setColorFormat(eYUV_422_3P).setWidth(640).setHeight(480).allocBuffer();
    outBuffer.setColorFormat(eI420).setWidth(320).setHeight(240).allocBuffer();

    pStream = new DpAsyncBlitStream();

    status = utilReadBMP("/system/pat/640x480.bmp",
                        (uint8_t*)inBuffer.y(),
                        (uint8_t*)inBuffer.u(),
                        (uint8_t*)inBuffer.v(),
                        inBuffer.colorFormat(),
                        inBuffer.width(),
                        inBuffer.height(),
                        inBuffer.yPitch());
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pStream->createJob(job[0], fence[0]);

    pStream->createJob(job[1], fence[1]);

    pStream->setConfigBegin(job[0]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer.yuv(), outBuffer.yuvSize(), outBuffer.plane());

        pStream->setDstConfig(0, outBuffer.width(), outBuffer.height(), outBuffer.yPitch(), outBuffer.uvPitch(),
            outBuffer.colorFormat(), DP_PROFILE_JPEG);
    }

    pStream->setConfigEnd();

    pStream->setConfigBegin(job[1]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer.yuv(), outBuffer.yuvSize(), outBuffer.plane());

        pStream->setDstConfig(0, outBuffer.width(), outBuffer.height(), outBuffer.yPitch(), outBuffer.uvPitch(),
            outBuffer.colorFormat(), DP_PROFILE_JPEG);
    }

    pStream->setConfigEnd();

    pthread_create(&tid, NULL, consumerThread, fence);

    pStream->invalidate();
    indexLoop++;

    pStream->invalidate();
    indexLoop++;

    pthread_join(tid, NULL);

    {
        char     name[256];
        sprintf(name, "/system/out/%s_output0_1.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer.y(),
            outBuffer.u(),
            outBuffer.v(),
            outBuffer.colorFormat(),
            outBuffer.width(),
            outBuffer.height(),
            outBuffer.yPitch(),
            outBuffer.uvPitch());
    }

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStreamRGB888", AsyncBlitStreamRGB888, asyncBlitStreamCaseRGB888)

void asyncBlitStreamCase1In2Out(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAsyncBlitStream *pStream;
    uint32_t          job[2];
    int32_t           fence[2];
    pthread_t         tid;
    uint32_t          indexLoop = 0;

    DpTestImageBuffer inBuffer;
    DpTestImageBuffer outBuffer0;
    DpTestImageBuffer outBuffer1;

    inBuffer.setColorFormat(eYUV_422_3P).setWidth(640).setHeight(480).allocBuffer();
    outBuffer0.setColorFormat(eI420).setWidth(64).setHeight(48).allocBuffer();
    outBuffer1.setColorFormat(eI420).setWidth(320).setHeight(240).allocBuffer();

    pStream = new DpAsyncBlitStream();

    status = utilReadBMP("/system/pat/640x480.bmp",
                        (uint8_t*)inBuffer.y(),
                        (uint8_t*)inBuffer.u(),
                        (uint8_t*)inBuffer.v(),
                        inBuffer.colorFormat(),
                        inBuffer.width(),
                        inBuffer.height(),
                        inBuffer.yPitch());
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pStream->createJob(job[0], fence[0]);

    pStream->createJob(job[1], fence[1]);

    pStream->setConfigBegin(job[0]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG);

    }

    pStream->setConfigEnd();

    pStream->setConfigBegin(job[1]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG);

    }

    pStream->setConfigEnd();

    pthread_create(&tid, NULL, consumerThread, fence);

    pStream->invalidate();
    indexLoop++;

    pStream->invalidate();
    indexLoop++;

    pthread_join(tid, NULL);

    {
        char     name[256];
        sprintf(name, "/system/out/%s_output0_0.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer0.y(),
            outBuffer0.u(),
            outBuffer0.v(),
            outBuffer0.colorFormat(),
            outBuffer0.width(),
            outBuffer0.height(),
            outBuffer0.yPitch(),
            outBuffer0.uvPitch());

        sprintf(name, "/system/out/%s_output0_1.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer1.y(),
            outBuffer1.u(),
            outBuffer1.v(),
            outBuffer1.colorFormat(),
            outBuffer1.width(),
            outBuffer1.height(),
            outBuffer1.yPitch(),
            outBuffer1.uvPitch());
    }

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStream1In2Out", AsyncBlitStream1In2Out, asyncBlitStreamCase1In2Out)

void asyncBlitStreamCase1In2OutCrop(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAsyncBlitStream *pStream;
    uint32_t          job[2];
    int32_t           fence[2];
    pthread_t         tid;
    uint32_t          indexLoop = 0;
    DpRect            roi0(320, 120, 320, 240);
    DpRect            roi1(160, 120, 320, 240);

    DpTestImageBuffer inBuffer;
    DpTestImageBuffer outBuffer0;
    DpTestImageBuffer outBuffer1;

    inBuffer.setColorFormat(eYUV_422_3P).setWidth(640).setHeight(480).allocBuffer();
    outBuffer0.setColorFormat(eI420).setWidth(64).setHeight(48).allocBuffer();
    outBuffer1.setColorFormat(eI420).setWidth(320).setHeight(240).allocBuffer();

    pStream = new DpAsyncBlitStream();

    status = utilReadBMP("/system/pat/640x480.bmp",
                        (uint8_t*)inBuffer.y(),
                        (uint8_t*)inBuffer.u(),
                        (uint8_t*)inBuffer.v(),
                        inBuffer.colorFormat(),
                        inBuffer.width(),
                        inBuffer.height(),
                        inBuffer.yPitch());
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pStream->createJob(job[0], fence[0]);

    pStream->createJob(job[1], fence[1]);

    pStream->setConfigBegin(job[0]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG);

        pStream->setSrcCrop(0, roi0);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG);

        pStream->setSrcCrop(1, roi1);
    }

    pStream->setConfigEnd();

    pStream->setConfigBegin(job[1]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG);

        pStream->setSrcCrop(0, roi0);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG);

        pStream->setSrcCrop(1, roi1);
    }

    pStream->setConfigEnd();

    pthread_create(&tid, NULL, consumerThread, fence);

    pStream->invalidate();
    indexLoop++;

    pStream->invalidate();
    indexLoop++;


    pthread_join(tid, NULL);

    {
        char     name[256];
        sprintf(name, "/system/out/%s_output0_0.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer0.y(),
            outBuffer0.u(),
            outBuffer0.v(),
            outBuffer0.colorFormat(),
            outBuffer0.width(),
            outBuffer0.height(),
            outBuffer0.yPitch(),
            outBuffer0.uvPitch());

        sprintf(name, "/system/out/%s_output0_1.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer1.y(),
            outBuffer1.u(),
            outBuffer1.v(),
            outBuffer1.colorFormat(),
            outBuffer1.width(),
            outBuffer1.height(),
            outBuffer1.yPitch(),
            outBuffer1.uvPitch());
    }

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStream1In2OutCrop", AsyncBlitStream1In2OutCrop, asyncBlitStreamCase1In2OutCrop)

void asyncBlitStreamCase1In2OutRotate(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAsyncBlitStream *pStream;
    uint32_t          job[2];
    int32_t           fence[2];
    pthread_t         tid;
    uint32_t          indexLoop = 0;

    DpTestImageBuffer inBuffer;
    DpTestImageBuffer outBuffer0;
    DpTestImageBuffer outBuffer1;

    inBuffer.setColorFormat(eYUV_422_3P).setWidth(640).setHeight(480).allocBuffer();
    outBuffer0.setColorFormat(eI420).setWidth(64).setHeight(48).allocBuffer();
    outBuffer1.setColorFormat(eI420).setWidth(320).setHeight(240).allocBuffer();

    pStream = new DpAsyncBlitStream();

    status = utilReadBMP("/system/pat/640x480.bmp",
                        (uint8_t*)inBuffer.y(),
                        (uint8_t*)inBuffer.u(),
                        (uint8_t*)inBuffer.v(),
                        inBuffer.colorFormat(),
                        inBuffer.width(),
                        inBuffer.height(),
                        inBuffer.yPitch());
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pStream->createJob(job[0], fence[0]);

    pStream->createJob(job[1], fence[1]);

    pStream->setConfigBegin(job[0]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setRotate(0, 90);

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG);

        pStream->setRotate(1, 180);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG);
    }

    pStream->setConfigEnd();

    pStream->setConfigBegin(job[1]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setRotate(0, 90);

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG);

        pStream->setRotate(1, 180);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG);
    }

    pStream->setConfigEnd();

    pthread_create(&tid, NULL, consumerThread, fence);

    pStream->invalidate();
    indexLoop++;

    pStream->invalidate();
    indexLoop++;

    pthread_join(tid, NULL);

    {
        char     name[256];
        sprintf(name, "/system/out/%s_output0_0.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer0.y(),
            outBuffer0.u(),
            outBuffer0.v(),
            outBuffer0.colorFormat(),
            outBuffer0.width(),
            outBuffer0.height(),
            outBuffer0.yPitch(),
            outBuffer0.uvPitch());

        sprintf(name, "/system/out/%s_output0_1.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer1.y(),
            outBuffer1.u(),
            outBuffer1.v(),
            outBuffer1.colorFormat(),
            outBuffer1.width(),
            outBuffer1.height(),
            outBuffer1.yPitch(),
            outBuffer1.uvPitch());
    }

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStream1In2OutRotate", AsyncBlitStream1In2OutRotate, asyncBlitStreamCase1In2OutRotate)

void asyncBlitStreamCase1In2OutCropRotate(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAsyncBlitStream *pStream;
    uint32_t          job[2];
    int32_t           fence[2];
    pthread_t         tid;
    uint32_t          indexLoop = 0;
    DpRect            roi0(320, 120, 320, 240);
    DpRect            roi1(160, 120, 320, 240);

    DpTestImageBuffer inBuffer;
    DpTestImageBuffer outBuffer0;
    DpTestImageBuffer outBuffer1;

    inBuffer.setColorFormat(eYUV_422_3P).setWidth(640).setHeight(480).allocBuffer();
    outBuffer0.setColorFormat(eI420).setWidth(64).setHeight(48).allocBuffer();
    outBuffer1.setColorFormat(eI420).setWidth(320).setHeight(240).allocBuffer();

    pStream = new DpAsyncBlitStream();

    status = utilReadBMP("/system/pat/640x480.bmp",
                        (uint8_t*)inBuffer.y(),
                        (uint8_t*)inBuffer.u(),
                        (uint8_t*)inBuffer.v(),
                        inBuffer.colorFormat(),
                        inBuffer.width(),
                        inBuffer.height(),
                        inBuffer.yPitch());
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pStream->createJob(job[0], fence[0]);

    pStream->createJob(job[1], fence[1]);

    pStream->setConfigBegin(job[0]);

    {
            pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

            pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

            pStream->setRotate(0, 90);

            pStream->setSrcCrop(0, roi0);

            pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

            pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                              outBuffer0.yPitch(), outBuffer0.uvPitch(),
                              outBuffer0.colorFormat(), DP_PROFILE_JPEG);

            pStream->setRotate(1, 180);

            pStream->setSrcCrop(1, roi1);

            pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

            pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                              outBuffer1.yPitch(), outBuffer1.uvPitch(),
                              outBuffer1.colorFormat(), DP_PROFILE_JPEG);
    }

    pStream->setConfigEnd();

    pStream->setConfigBegin(job[1]);

    {
            pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

            pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

            pStream->setRotate(0, 90);

            pStream->setSrcCrop(0, roi0);

            pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

            pStream->setDstConfig(0, outBuffer0.width(), outBuffer0.height(),
                              outBuffer0.yPitch(), outBuffer0.uvPitch(),
                              outBuffer0.colorFormat(), DP_PROFILE_JPEG);

            pStream->setRotate(1, 180);

            pStream->setSrcCrop(1, roi1);

            pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

            pStream->setDstConfig(1, outBuffer1.width(), outBuffer1.height(),
                              outBuffer1.yPitch(), outBuffer1.uvPitch(),
                              outBuffer1.colorFormat(), DP_PROFILE_JPEG);
    }

    pStream->setConfigEnd();

    pthread_create(&tid, NULL, consumerThread, fence);

    pStream->invalidate();
    indexLoop++;

    pStream->invalidate();
    indexLoop++;

    pthread_join(tid, NULL);

    {
        char     name[256];
        sprintf(name, "/system/out/%s_output0_0.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer0.y(),
            outBuffer0.u(),
            outBuffer0.v(),
            outBuffer0.colorFormat(),
            outBuffer0.width(),
            outBuffer0.height(),
            outBuffer0.yPitch(),
            outBuffer0.uvPitch());

        sprintf(name, "/system/out/%s_output0_1.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer1.y(),
            outBuffer1.u(),
            outBuffer1.v(),
            outBuffer1.colorFormat(),
            outBuffer1.width(),
            outBuffer1.height(),
            outBuffer1.yPitch(),
            outBuffer1.uvPitch());
    }

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStream1In2OutCropRotate", AsyncBlitStream1In2OutCropRotate, asyncBlitStreamCase1In2OutCropRotate)

void asyncBlitStreamCase1In2OutTargetROI(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAsyncBlitStream *pStream;
    uint32_t          job[2];
    int32_t           fence[2];
    pthread_t         tid;
    uint32_t          indexLoop = 0;

    DpTestImageBuffer inBuffer;
    DpTestImageBuffer outBuffer0;
    DpTestImageBuffer outBuffer1;

    inBuffer.setColorFormat(eYUV_422_3P).setWidth(640).setHeight(480).allocBuffer();
    outBuffer0.setColorFormat(eI420).setWidth(1280).setHeight(720).allocBuffer();
    outBuffer1.setColorFormat(eI420).setWidth(1920).setHeight(1080).allocBuffer();

    //align center
    DpRect targetROI0;
    DpRect targetROI1;

    targetROI0.x = (outBuffer0.width() - inBuffer.width())/2;
    targetROI0.y = (outBuffer0.height() - inBuffer.height())/2;
    targetROI0.w = inBuffer.width();
    targetROI0.h = inBuffer.height();

    targetROI1.x = (outBuffer1.width() - inBuffer.width())/2;
    targetROI1.y = (outBuffer1.height() - inBuffer.height())/2;
    targetROI1.w = inBuffer.width();
    targetROI1.h = inBuffer.height();

    pStream = new DpAsyncBlitStream();

    status = utilReadBMP("/system/pat/640x480.bmp",
                        (uint8_t*)inBuffer.y(),
                        (uint8_t*)inBuffer.u(),
                        (uint8_t*)inBuffer.v(),
                        inBuffer.colorFormat(),
                        inBuffer.width(),
                        inBuffer.height(),
                        inBuffer.yPitch());
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pStream->createJob(job[0], fence[0]);

    pStream->createJob(job[1], fence[1]);

    pStream->setConfigBegin(job[0]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, targetROI0.w, targetROI0.h,
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG, eInterlace_None, &targetROI0);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, targetROI1.w, targetROI1.h,
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG, eInterlace_None, &targetROI1);
    }

    pStream->setConfigEnd();

    pStream->setConfigBegin(job[1]);

    {
        pStream->setSrcBuffer(inBuffer.yuv(), inBuffer.yuvSize(), inBuffer.plane());

        pStream->setSrcConfig(inBuffer.width(), inBuffer.height(), inBuffer.colorFormat());

        pStream->setDstBuffer(0, outBuffer0.yuv(), outBuffer0.yuvSize(), outBuffer0.plane());

        pStream->setDstConfig(0, targetROI0.w, targetROI0.h,
                          outBuffer0.yPitch(), outBuffer0.uvPitch(),
                          outBuffer0.colorFormat(), DP_PROFILE_JPEG, eInterlace_None, &targetROI0);

        pStream->setDstBuffer(1, outBuffer1.yuv(), outBuffer1.yuvSize(), outBuffer1.plane());

        pStream->setDstConfig(1, targetROI1.w, targetROI1.h,
                          outBuffer1.yPitch(), outBuffer1.uvPitch(),
                          outBuffer1.colorFormat(), DP_PROFILE_JPEG, eInterlace_None, &targetROI1);
    }

    pStream->setConfigEnd();

    pthread_create(&tid, NULL, consumerThread, fence);

    pStream->invalidate();
    indexLoop++;

    pStream->invalidate();
    indexLoop++;

    pthread_join(tid, NULL);

    {
        char     name[256];
        sprintf(name, "/system/out/%s_output0_0.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer0.y(),
            outBuffer0.u(),
            outBuffer0.v(),
            outBuffer0.colorFormat(),
            outBuffer0.width(),
            outBuffer0.height(),
            outBuffer0.yPitch(),
            outBuffer0.uvPitch());

        sprintf(name, "/system/out/%s_output0_1.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer1.y(),
            outBuffer1.u(),
            outBuffer1.v(),
            outBuffer1.colorFormat(),
            outBuffer1.width(),
            outBuffer1.height(),
            outBuffer1.yPitch(),
            outBuffer1.uvPitch());
    }

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStream1In2OutTargetROI", AsyncBlitStream1In2OutTargetROI, asyncBlitStreamCase1In2OutTargetROI)


struct FenceThreadContext
{
    pthread_t           tid;
    int32_t             timelineFd;
    DpTestImageBuffer   *buffer0;
    DpTestImageBuffer   *buffer1;
};

void* srcThread(void* para)
{
    DPLOGD("Src Thread: start of src thread\n");

    FenceThreadContext& ctx = *(FenceThreadContext*)para;

    DP_STATUS_ENUM    status;

    {
        status = utilReadBMP("/system/pat/640x480.bmp",
                            (uint8_t*)ctx.buffer0->y(),
                            (uint8_t*)ctx.buffer0->u(),
                            (uint8_t*)ctx.buffer0->v(),
                            ctx.buffer0->colorFormat(),
                            ctx.buffer0->width(),
                            ctx.buffer0->height(),
                            ctx.buffer0->yPitch());


        if(DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGD("Src Thread: Read input image source failed\n");
        }

        DPLOGD("Src Thread: task done\n");

        sw_sync_timeline_inc(ctx.timelineFd, 1);

        DPLOGD("Src Thread: increase timeline %d by 1\n", ctx.timelineFd);
    }

    {
        status = utilReadBMP("/system/pat/640x480.bmp",
                            (uint8_t*)ctx.buffer1->y(),
                            (uint8_t*)ctx.buffer1->u(),
                            (uint8_t*)ctx.buffer1->v(),
                            ctx.buffer1->colorFormat(),
                            ctx.buffer1->width(),
                            ctx.buffer1->height(),
                            ctx.buffer1->yPitch());


        if(DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGD("Src Thread: Read input image source failed\n");
        }

        DPLOGD("Src Thread: task done\n");

        sw_sync_timeline_inc(ctx.timelineFd, 1);

        DPLOGD("Src Thread: increase timeline %d by 1\n", ctx.timelineFd);
    }

    DPLOGD("Src Thread: end of src thread\n");

    return NULL;
}

void* dstThread(void* para)
{
    DPLOGD("Dst Thread: start of dst thread\n");

    FenceThreadContext& ctx = *(FenceThreadContext*)para;

    {
        //set top half background to white
        memset(ctx.buffer0->y(), 255, ctx.buffer0->ySize()/2);
        memset(ctx.buffer0->u(), 255, ctx.buffer0->uSize()/2);
        memset(ctx.buffer0->v(), 255, ctx.buffer0->vSize()/2);

        DPLOGD("Dst Thread: task done\n");

        sw_sync_timeline_inc(ctx.timelineFd, 1);

        DPLOGD("Dst Thread: increase timeline %d by 1\n", ctx.timelineFd);
    }

    {
        //set bottom half background to gray
        memset(ctx.buffer0->y()+ctx.buffer0->ySize()/2, 128, ctx.buffer0->ySize()/2);
        memset(ctx.buffer0->u()+ctx.buffer0->uSize()/2, 128, ctx.buffer0->uSize()/2);
        memset(ctx.buffer0->v()+ctx.buffer0->vSize()/2, 128, ctx.buffer0->vSize()/2);

        DPLOGD("Dst Thread: task done\n");

        sw_sync_timeline_inc(ctx.timelineFd, 1);

        DPLOGD("Dst Thread: increase timeline %d by 1\n", ctx.timelineFd);
    }


    DPLOGD("Dst Thread: end of dst thread\n");

    return NULL;
}

void asyncBlitStreamCaseFence(TestReporter *pReporter)
{
    DP_STATUS_ENUM    status;
    DpAsyncBlitStream *pStream;
    uint32_t          job[2];
    int32_t           fence[2];
    pthread_t         tid;
    uint32_t          indexLoop = 0;

    DpTestImageBuffer inBuffer0, inBuffer1;
    DpTestImageBuffer outBuffer;
    DpRect            targetROI;
    FenceThreadContext srcCtx, dstCtx;
    int32_t srcFenceFd[2];
    int32_t dstFenceFd[2];

    inBuffer0.setColorFormat(eYUV_422_3P).setWidth(640).setHeight(480).allocBuffer();
    inBuffer1.setColorFormat(eGREY).setWidth(640).setHeight(480).allocBuffer();
    outBuffer.setColorFormat(eBGR888).setWidth(1280).setHeight(960).allocBuffer();

    srcCtx.buffer0 = &inBuffer0;
    srcCtx.buffer1 = &inBuffer1;
    srcCtx.timelineFd = sw_sync_timeline_create();

    dstCtx.buffer0 = &outBuffer;
    dstCtx.buffer1 = NULL;
    dstCtx.timelineFd = sw_sync_timeline_create();

    pStream = new DpAsyncBlitStream();

    srcFenceFd[0] = sw_sync_fence_create(srcCtx.timelineFd, "SrcFence0", 1);
    srcFenceFd[1] = sw_sync_fence_create(srcCtx.timelineFd, "SrcFence1", 2);

    dstFenceFd[0] = sw_sync_fence_create(dstCtx.timelineFd, "DstFence0", 1);
    dstFenceFd[1] = sw_sync_fence_create(dstCtx.timelineFd, "DstFence1", 2);

    /*
    srcFenceFd[0] = -1;
    srcFenceFd[1] = -1;
    dstFenceFd[0] = -1;
    dstFenceFd[1] = -1;
    //*/

    pStream->createJob(job[0], fence[0]);

    pStream->createJob(job[1], fence[1]);

    //align top left
    targetROI.x = 0;
    targetROI.y = 0;
    targetROI.w = inBuffer0.width();
    targetROI.h = inBuffer0.height();

    pStream->setConfigBegin(job[0]);

    {
        pStream->setSrcBuffer(inBuffer0.yuv(), inBuffer0.yuvSize(), inBuffer0.plane(), srcFenceFd[0]);

        pStream->setSrcConfig(inBuffer0.width(), inBuffer0.height(), inBuffer0.colorFormat());

        pStream->setDstBuffer(0, outBuffer.yuv(), outBuffer.yuvSize(), outBuffer.plane(), dstFenceFd[0]);

        pStream->setDstConfig(0, targetROI.w, targetROI.h, outBuffer.yPitch(), outBuffer.uvPitch(),
            outBuffer.colorFormat(), DP_PROFILE_JPEG, eInterlace_None, &targetROI);
    }

    pStream->setConfigEnd();

    //align bottom right
    targetROI.x = inBuffer0.width();
    targetROI.y = inBuffer0.height();
    targetROI.w = inBuffer0.width();
    targetROI.h = inBuffer0.height();

    pStream->setConfigBegin(job[1]);

    {
        pStream->setSrcBuffer(inBuffer1.yuv(), inBuffer1.yuvSize(), inBuffer1.plane(), srcFenceFd[1]);

        pStream->setSrcConfig(inBuffer1.width(), inBuffer1.height(), inBuffer1.colorFormat());

        pStream->setDstBuffer(0, outBuffer.yuv(), outBuffer.yuvSize(), outBuffer.plane(), dstFenceFd[1]);

        pStream->setDstConfig(0, targetROI.w, targetROI.h, outBuffer.yPitch(), outBuffer.uvPitch(),
            outBuffer.colorFormat(), DP_PROFILE_JPEG, eInterlace_None, &targetROI);
    }

    pStream->setConfigEnd();

    pthread_create(&srcCtx.tid, NULL, srcThread, &srcCtx);
    pthread_create(&dstCtx.tid, NULL, dstThread, &dstCtx);
    pthread_create(&tid, NULL, consumerThread, fence);

    pStream->invalidate();
    indexLoop++;

    pStream->invalidate();
    indexLoop++;

    pthread_join(tid, NULL);
    pthread_join(srcCtx.tid, NULL);
    pthread_join(dstCtx.tid, NULL);

    {
        char     name[256];
        sprintf(name, "/system/out/%s_output0_0.bmp", pReporter->getTestName());
        utilWriteBMP(name, outBuffer.y(),
            outBuffer.u(),
            outBuffer.v(),
            outBuffer.colorFormat(),
            outBuffer.width(),
            outBuffer.height(),
            outBuffer.yPitch(),
            outBuffer.uvPitch());
    }

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStreamFence", AsyncBlitStreamFence, asyncBlitStreamCaseFence)


#if CONFIG_FOR_SEC_VIDEO_PATH
//for SVP
#include <tlc_sec_mem.h>

void asyncBlitStreamCaseSVP(TestReporter *pReporter)
{
    DpAsyncBlitStream   *pStream;
    void                *pBuffer[3];
    uint32_t            size[3];
    uint32_t            job;
    int32_t             fence;
    UREE_SECUREMEM_HANDLE secSrcHd, secDstHd;
    pStream = new DpAsyncBlitStream();

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

    pStream->createJob(job, fence);

    pStream->setConfigBegin(job);

    pStream->setSrcBuffer(pBuffer, size, 1);
    pStream->setSrcConfig(640,
                          480,
                          DP_COLOR_GET_MIN_Y_PITCH(DP_COLOR_RGB888, 640),
                          DP_COLOR_GET_MIN_UV_PITCH(DP_COLOR_RGB888, 640),
                          DP_COLOR_RGB888,
                          DP_PROFILE_BT601,
                          eInterlace_None,
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

    pStream->setDstBuffer(0, pBuffer, size, 3);
    pStream->setDstConfig(0,
                          640,
                          480,
                          DP_COLOR_GET_MIN_Y_PITCH(DP_COLOR_YV12, 640),
                          DP_COLOR_GET_MIN_UV_PITCH(DP_COLOR_YV12, 640),
                          DP_COLOR_YV12,
                          DP_PROFILE_BT601,
                          eInterlace_None,
                          NULL,
                          DP_SECURE,
                          true);

    pStream->setConfigEnd();

    pStream->invalidate();

    sync_wait(fence, -1);
    close(fence);

    uint32_t count;
    UREE_UnreferenceSecurememTBL(secSrcHd, &count);
    UREE_UnreferenceSecurememTBL(secDstHd, &count);

    delete pStream;
}

DEFINE_TEST_CASE("AsyncBlitStreamSVP", AsyncBlitStreamSVP, asyncBlitStreamCaseSVP);
#endif

