#include "TestDefine.h"
#include "DpIspStream.h"
#include "DpWriteBMP.h"
#include "DpWriteBin.h"
#include "DpReadBMP.h"
#include "DpWriteBin.h"
#include <tpipe_config.h>


#include "imgi.h"
#include "isp_reg_setting_value.h"
#include "DpEngineType.h"
#include "DpMemory.h"
#include <fcntl.h>
#include <pthread.h>
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifdef WIN32
#include"windows.h"
#define usleep(n) Sleep(n/1000)
#endif


/*
 * ISP stream Test #0: 1 in 2 out test case
 */
void IspStreamCase0(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    uint8_t                     *pOutput0;
    void                        *outBuf0[3];
    uint32_t                    outSize[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
    memset(&ISPInfo,0,sizeof(ISPInfo));

    pStream = new DpIspStream(DpIspStream::ISP_IC_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(3264 * 2448 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    // Copy source image
    //memcpy(pSource, bayer12_3264x2448, sizeof(bayer12_3264x2448));

    status = pStream->queueSrcBuffer(pSource,
                                     3264 * 2448 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_BAYER10,
                                   3264,
                                   2448,
                                   3264 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(3264 * 2448 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer0\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    outBuf0[0] = pOutput0;
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 3264 * 2448 * 2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(3,
                                     outBuf0,
                                     outSize,
                                     1);

    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(3,
                                   3264,
                                   2448,
                                   3264*2,
                                   0,
                                   DP_COLOR_YUYV);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");
#if 0
    pOutput0 = (uint8_t*)malloc(2560 * 1600 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    outBuf0[0] = pOutput0;
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 2560 * 1600 * 2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(1,
                                     outBuf0,
                                     outSize,
                                     1);

    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(1,
                                   2560,
                                   1600,
                                   5120,
                                   0,
                                   DP_COLOR_YUYV);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");
#endif
    DpJPEGEnc_Config jpegEnc_cfg;
    jpegEnc_cfg.fileDesc = 7;
    jpegEnc_cfg.size = 3000;
    jpegEnc_cfg.fQuality = 70;
    jpegEnc_cfg.soi_en = 0;
    status = pStream->setPortType(3,2);

#if 0
    /* Got each function's frame information here */
    ISPInfo.sw.log_en = 1 ;
    ISPInfo.sw.src_width = 3264 ;
    ISPInfo.sw.src_height = 2448 ;
    ISPInfo.sw.tpipe_width = 768 ;
    ISPInfo.sw.tpipe_height = 8192 ;
    ISPInfo.sw.tpipe_irq_mode = 0 ;
    ISPInfo.top.scenario = 6 ;
    ISPInfo.top.mode = 0 ;
    ISPInfo.top.pixel_id = 0 ;
    ISPInfo.top.cam_in_fmt = 0 ;
    ISPInfo.top.ctl_extension_en = 0 ;
    ISPInfo.top.fg_mode = 0 ;
    ISPInfo.top.ufdi_fmt = 0 ;
    ISPInfo.top.vipi_fmt = 2 ;
    ISPInfo.top.img3o_fmt = 2 ;
    ISPInfo.top.imgi_en = 1 ;
    ISPInfo.top.g2c_en = 1 ;
    ISPInfo.top.c42_en = 1 ;
    ISPInfo.top.nbc_en = 1 ;
    ISPInfo.top.sl2c_en = 1 ;
    ISPInfo.top.seee_en = 1 ;
    ISPInfo.top.crz_en = 1 ;
    ISPInfo.top.img2o_en = 1 ;
    ISPInfo.top.crsp_en = 0 ;
    ISPInfo.top.img3o_en = 1 ;
    ISPInfo.top.c24b_en = 0 ; //Holmes turn off MDP_CROP
    ISPInfo.top.mdp_crop_en = 0 ; //Holmes turn off MDP_CROP
    ISPInfo.top.imgi_v_flip_en = 0 ;
    ISPInfo.top.ufd_sel = 0 ;
    ISPInfo.top.ccl_sel = 0 ;
    ISPInfo.top.ccl_sel_en = 1 ;
    ISPInfo.top.g2g_sel = 1 ;
    ISPInfo.top.g2g_sel_en = 1 ;
    ISPInfo.top.mix1_sel = 0 ;
    ISPInfo.top.crz_sel = 0 ;
    ISPInfo.top.nr3d_sel = 0 ;
    ISPInfo.top.fe_sel = 1 ;
    ISPInfo.top.mdp_sel = 0 ;
    ISPInfo.top.pca_sel = 0 ;
    ISPInfo.top.interlace_mode = 0 ;
    ISPInfo.imgi.imgi_stride = 4080 ;
    ISPInfo.g2c.g2c_shade_en = 1 ;
    ISPInfo.nbc.anr_eny = 1 ;
    ISPInfo.nbc.anr_enc = 1 ;
    ISPInfo.nbc.anr_iir_mode = 0 ;
    ISPInfo.nbc.anr_scale_mode = 2 ;
    ISPInfo.sl2c.sl2c_hrz_comp = 0 ;
    ISPInfo.seee.se_edge = 1 ;
    ISPInfo.cdrz.cdrz_input_crop_width = 2612 ;
    ISPInfo.cdrz.cdrz_input_crop_height = 1959 ;
    ISPInfo.cdrz.cdrz_output_width = 3264 ;
    ISPInfo.cdrz.cdrz_output_height = 2448 ;
    ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 326 ;
    ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0 ;
    ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 244 ;
    ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0 ;
    ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0 ;
    ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0 ;
    ISPInfo.cdrz.cdrz_horizontal_coeff_step = 26220 ;
    ISPInfo.cdrz.cdrz_vertical_coeff_step = 26220 ;
    ISPInfo.img2o.img2o_stride = 0 ;
    ISPInfo.img2o.img2o_xoffset = 0 ;
    ISPInfo.img2o.img2o_yoffset = 0 ;
    ISPInfo.img2o.img2o_xsize = 6527 ;
    ISPInfo.img2o.img2o_ysize = 2447 ;
    ISPInfo.img3o.img3o_stride = 0 ;
    ISPInfo.img3o.img3o_xoffset = 0 ;
    ISPInfo.img3o.img3o_yoffset = 0 ;
    ISPInfo.img3o.img3o_xsize = 6527 ;
    ISPInfo.img3o.img3o_ysize = 2447 ;

#endif

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->stopStream();

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(3,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream0", IspStream0, IspStreamCase0);


/*
 * ISP stream Test #1: 1 in 2 out test case
 */

DpIspStream *pStream;
volatile uint32_t frameCounter = 0;
pthread_mutex_t frCounterMutex;
pthread_cond_t  frameCond;
volatile uint32_t dequeueCtr = 0;

static void* dequeueThread(void* pInput)
{
    DP_STATUS_ENUM              status;
    void                        *outBuf0[3];
    dequeueCtr = 0;

    while(1)
    {

        if(frameCounter > 0)
        {
            //printf("dequeueThread: %d , %d\n",dequeueCtr,frameCounter);

            status = pStream->dequeueSrcBuffer();

            status = pStream->dequeueDstBuffer(0,
                &outBuf0[0],
                true);

            #if 0
            {
                char     name[256];
                sprintf(name, "./out/output0_0_%d.bmp", dequeueCtr);
                utilWriteBMP(name,
                    outBuf0[0],
                    outBuf0[1],
                    outBuf0[2],
                    DP_COLOR_YUYV,
                    160,
                    240,
                    320,
                    0);

            }
            #endif

            status = pStream->dequeueDstBuffer(1,
                &outBuf0[0],
                true);

#if 0
            {
                char     name[256];
                sprintf(name, "./out/output1_0_%d.bmp", dequeueCtr);
                utilWriteBMP(name,
                    outBuf0[0],
                    outBuf0[1],
                    outBuf0[2],
                    DP_COLOR_YUYV,
                    320,
                    240,
                    640,
                    0);

            }
#endif
            status = pStream->dequeueFrameEnd();

            pthread_mutex_lock(&frCounterMutex);
            dequeueCtr++;
            frameCounter--;
            pthread_mutex_unlock(&frCounterMutex);

			DPLOGD("MDP: DeQueueThread: %d\n",dequeueCtr);

        }else
        {
            pthread_mutex_lock(&frCounterMutex);
            pthread_cond_wait(&frameCond, &frCounterMutex);
            pthread_mutex_unlock(&frCounterMutex);
        }
    };
    return NULL;
}

volatile uint32_t VSS_end = false;
volatile uint32_t runCounter = 0;
pthread_mutex_t VSSMutex;

static void* IspStreamCaseVSS(void* pInput)
{

    while(1)
    {

        DP_STATUS_ENUM              status;
        DpIspStream                 *pStream;
        uint8_t                     *pSource;
        uint8_t                     *pOutput0;
        void                        *outBuf0[3];
        uint32_t                    outSize[3];
        ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
        memset(&ISPInfo,0,sizeof(ISPInfo));

        pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
        pSource = (uint8_t*)malloc(640 * 480 * 2);

        DPLOGI("IspStreamCaseVSS  start: %X\n",pStream);


        // Copy source image
        status = utilReadBMP("/system/pat/640x480.bmp",
            (uint8_t*)pSource,
            (uint8_t*)NULL,
            (uint8_t*)NULL,
            eYUV_422_I,
            640,
            480,
            640);


        status = pStream->queueSrcBuffer(pSource,
            640 * 480 * 2);

        status = pStream->setSrcConfig(eYUV_422_I,
            640,
            480,
            640 * 2);

        pOutput0 = (uint8_t*)malloc(960 * 540 * 2);

        DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

        outBuf0[0] = pOutput0;
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outSize[0] = 960 * 540 *2;
        outSize[1] = 0;
        outSize[2] = 0;
        status = pStream->queueDstBuffer(0,
            outBuf0,
            outSize,
            1);


        status = pStream->setDstConfig(0,
            540,
            960,
            1080,
            0,
            DP_COLOR_YUY2);

        status = pStream->setRotation(0,90);

        DpJPEGEnc_Config jpegEnc_cfg;
        jpegEnc_cfg.fileDesc = 7;
        jpegEnc_cfg.size = 3000;
        jpegEnc_cfg.fQuality = 95;
        jpegEnc_cfg.soi_en = 1;
        //status = pStream->setPortType(0,1,&jpegEnc_cfg);

        status = pStream->setParameter(ISPInfo);

        status = pStream->startStream();

        while(1)
        {
            //struct timespec _intervalTmp;
            //_intervalTmp.tv_sec = 1;
            status = pStream->dequeueDstBuffer(0, &outBuf0[0], false);
            if(DP_STATUS_RETURN_SUCCESS == status)
            {
                DPLOGD("dequeueDstBuffer Pass : %d\n",status);
                break;
            }

            DPLOGD("dequeueDstBuffer failed : %d\n",status);
            usleep(500000);
        }

        DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

        status = pStream->dequeueSrcBuffer();

        status = pStream->stopStream();

        if (NULL != pOutput0)
        {
            delete pOutput0;
            pOutput0 = NULL;
        }

        if (NULL != pSource)
        {
            delete pSource;
            pSource = NULL;
        }

        if (NULL != pStream)
        {
            delete pStream;
        }

        usleep(100000);


        if(VSS_end)
        {
            pthread_cond_broadcast(&frameCond);

            return NULL;
        }

    }
    return NULL;
}


void IspStreamCase1(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    void                        *outBuf0[3];
    uint32_t                    outSize[3];
    uint32_t                    outBufPa[3];

    uint8_t                     *pSource;
    uint8_t                     *pSource1;

    uint32_t                    pSourcePA;
    uint32_t                    pSourcePA1;

    uint8_t                    *pOutput0;
    uint32_t                    pOutput0PA;

    uint8_t                    *pOutput0s;
    uint32_t                    pOutput0sPA;

    uint8_t                    *pOutput1;
    uint32_t                    pOutput1PA;

    uint8_t                    *pOutput1s;
    uint32_t                    pOutput1sPA;


    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
    memset(&ISPInfo,0,sizeof(ISPInfo));

    pStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    DpMemory* pISPSourceMem = DpMemory::Factory(DP_MEMORY_ION, -1, (4096*2160*2), false);
    pSource = (uint8_t*) pISPSourceMem->mapSWAddress();
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");
    pSourcePA = pISPSourceMem->mapHWAddress(tRDMA0,0);

    DpMemory* pISPSourceMem1 = DpMemory::Factory(DP_MEMORY_ION, -1, (3264*2448*2), false);
    pSource1 = (uint8_t*) pISPSourceMem1->mapSWAddress();
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");
    pSourcePA1 = pISPSourceMem1->mapHWAddress(tRDMA0,0);


    //Alocate destination
    DpMemory* pISPTargetMem = DpMemory::Factory(DP_MEMORY_ION, -1, (2560*1440*2), false);
    pOutput0 = (uint8_t*)pISPTargetMem->mapSWAddress();
    pOutput0PA = pISPTargetMem->mapHWAddress(tWROT0,0);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    //Alocate destination
    DpMemory* pISPTargetMem1 = DpMemory::Factory(DP_MEMORY_ION, -1, (3840*2160*2), false);
    pOutput1 = (uint8_t*)pISPTargetMem1->mapSWAddress();
    pOutput1PA = pISPTargetMem1->mapHWAddress(tWDMA,0);

    //Alocate destination
    DpMemory* pISPTargetMem_1 = DpMemory::Factory(DP_MEMORY_ION, -1, (2560*1440*2), false);
    pOutput0s = (uint8_t*)pISPTargetMem_1->mapSWAddress();
    pOutput0sPA = pISPTargetMem_1->mapHWAddress(tWROT0,0);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    //Alocate destination
    DpMemory* pISPTargetMem1_1 = DpMemory::Factory(DP_MEMORY_ION, -1,(3840*2160*2) , false);
    pOutput1s = (uint8_t*)pISPTargetMem1_1->mapSWAddress();
    pOutput1sPA = pISPTargetMem1_1->mapHWAddress(tWDMA,0);


    // Copy source image
    status = utilReadBMP("/system/pat/4096x2160.bmp",
                            (uint8_t*)pSource,
                            (uint8_t*)NULL,
                            (uint8_t*)NULL,
                            DP_COLOR_YUYV,
                            4096,
                            2160,
                            8192);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Copy source image
    status = utilReadBMP("/system/pat/3264x2448.bmp",
                            (uint8_t*)pSource1,
                            (uint8_t*)NULL,
                            (uint8_t*)NULL,
                            DP_COLOR_YUYV,
                            3264,
                            2448,
                            3264*2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    pthread_mutex_init(&frCounterMutex, NULL);
    pthread_mutex_init(&VSSMutex, NULL);
    pthread_cond_init(&frameCond, NULL);

    while(runCounter < 200)
    {

        status = pStream->queueSrcBuffer(pSource,pSourcePA, 4096 * 2160 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

        status = pStream->setSrcConfig(DP_COLOR_YUYV, 4096, 2160, 4096 * 2, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

        //Target 0
        outBuf0[0] = pOutput0;
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outBufPa[0] = pOutput0PA;
        outBufPa[1] = 0;
        outBufPa[2] = 0;

        outSize[0] = 2560 * 1440 * 2;
        outSize[1] = 0;
        outSize[2] = 0;

        status = pStream->queueDstBuffer(0, outBuf0,outBufPa,outSize, DP_COLOR_GET_PLANE_COUNT(DP_COLOR_YUYV));
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

        status = pStream->setDstConfig(0,
                                       1440,
                                       2560,
                                       2880,
                                       0,
                                       DP_COLOR_YUYV, DP_PROFILE_BT601, eInterlace_None, NULL, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

        status = pStream->setRotation(0,270);

        REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer1\n");

        //Target 1
        outBuf0[0] = pOutput1;
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outBufPa[0] = pOutput1PA;
        outBufPa[1] = 0;
        outBufPa[2] = 0;

        outSize[0] = 3840 * 2160 * 2;
        outSize[1] = 0;
        outSize[2] = 0;
        status = pStream->queueDstBuffer(1, outBuf0,outBufPa,outSize, DP_COLOR_GET_PLANE_COUNT(DP_COLOR_YUYV));
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

        status = pStream->setDstConfig(1,
                                       3840,
                                       2160,
                                       7680,
                                       0,
                                       DP_COLOR_YUYV, DP_PROFILE_BT601, eInterlace_None, NULL, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

        status = pStream->setParameter(ISPInfo);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

        status = pStream->startStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");


        status = pStream->stopStream();
        //First frame end
        pthread_mutex_lock(&frCounterMutex);
        frameCounter++;
        pthread_mutex_unlock(&frCounterMutex);



        status = pStream->queueSrcBuffer(pSource1, pSourcePA1, 3264 * 2448 * 2);

        status = pStream->setSrcConfig(DP_COLOR_YUYV, 3264, 2448, 3264 * 2, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");


        outBuf0[0] = pOutput0s;
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outBufPa[0] = pOutput0sPA;
        outBufPa[1] = 0;
        outBufPa[2] = 0;

        outSize[0] = 2560 * 1440 * 2;
        outSize[1] = 0;
        outSize[2] = 0;

        status = pStream->queueDstBuffer(0, outBuf0,outBufPa,outSize, DP_COLOR_GET_PLANE_COUNT(DP_COLOR_YUYV));
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");


        status = pStream->setDstConfig(0,
                                       1440,
                                       2560,
                                       2880,
                                       0,
                                       DP_COLOR_YUYV, DP_PROFILE_BT601, eInterlace_None, NULL, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");
        status = pStream->setRotation(0,270);


        //Target 1
        outBuf0[0] = pOutput1s;
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outBufPa[0] = pOutput1sPA;
        outBufPa[1] = 0;
        outBufPa[2] = 0;

        outSize[0] = 3840 * 2160 * 2;
        outSize[1] = 0;
        outSize[2] = 0;
        status = pStream->queueDstBuffer(1, outBuf0,outBufPa,outSize, DP_COLOR_GET_PLANE_COUNT(DP_COLOR_YUYV));
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

        status = pStream->setDstConfig(1,
                                        3840,
                                        2160,
                                        7680,
                                        0,
                                        DP_COLOR_YUYV, DP_PROFILE_BT601, eInterlace_None, NULL, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");


        status = pStream->startStream();

        if(1 == runCounter)
        {
            pthread_t id;
            pthread_create(&id,NULL,dequeueThread,NULL);
        }

        status = pStream->stopStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");
        //2nd frame done
        pthread_mutex_lock(&frCounterMutex);
        frameCounter++;
        pthread_mutex_unlock(&frCounterMutex);

        pthread_cond_signal(&frameCond);

        if(0xFFFFFFFF == runCounter)
        {
            pthread_t id;
            pthread_create(&id,NULL,IspStreamCaseVSS,NULL);
        }

        runCounter++;

    }

    DPLOGI("MDP: ============================Main thread is done============================\n");

#if 1
    while(dequeueCtr < 400)
    {
        if(dequeueCtr > 399)
            break;
    }
    VSS_end = true;

#if 0
    pthread_mutex_lock(&VSSMutex);
    pthread_cond_wait(&frameCond,&VSSMutex);
    pthread_mutex_unlock(&VSSMutex);
#endif

#endif

    if (NULL != pISPSourceMem)
    {
        delete pISPSourceMem;
        pISPSourceMem = NULL;
    }

    if (NULL != pISPSourceMem1)
    {
        delete pISPSourceMem1;
        pISPSourceMem1 = NULL;
    }

    if (NULL != pISPTargetMem)
    {
        delete pISPTargetMem;
        pISPTargetMem = NULL;
    }

    if (NULL != pISPTargetMem1)
    {
        delete pISPTargetMem1;
        pISPTargetMem1= NULL;
    }

    if (NULL != pISPTargetMem_1)
    {
        delete pISPTargetMem_1;
        pISPTargetMem_1 = NULL;
    }

    if (NULL != pISPTargetMem1_1)
    {
        delete pISPTargetMem1_1;
        pISPTargetMem1_1 = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream1", IspStream1, IspStreamCase1);



/*
 * ISP stream Test #2: 1 in 2 out test case
 */
void IspStreamCase2(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    uint8_t                     *pOutput0;//,*pOutput1;
    void                        *outBuf0[3];
    uint32_t                    outSize[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
    memset(&ISPInfo,0,sizeof(ISPInfo));

    pStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    // Copy source image
    status = utilReadBMP("/system/pat/640x480.bmp",
        (uint8_t*)pSource,
        (uint8_t*)NULL,
        (uint8_t*)NULL,
        eYUV_422_I,
        640,
        480,
        640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");


    status = pStream->queueSrcBuffer(pSource,
        640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(eYUV_422_I,
                                    640,
                                    480,
                                    640 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(960 * 540 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer0\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    outBuf0[0] = pOutput0;
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 960 * 540 *2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(0,
                                     outBuf0,
                                     outSize,
                                     1);

    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
        960,
        540,
        1920,
        0,
        DP_COLOR_YUY2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    //status = pStream->setRotation(0,90);

#if 0
    pOutput1 = (uint8_t*)malloc(320 * 240 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer1: %p\n", pOutput1);

    outBuf0[0] = pOutput1;
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 320 * 240 * 2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(1,
        outBuf0,
        outSize,
        1);

    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

    status = pStream->setDstConfig(1,
        320,
        240,
        640,
        0,
        DP_COLOR_YUYV);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");
#endif

    DpJPEGEnc_Config jpegEnc_cfg;
    jpegEnc_cfg.fileDesc = 7;
    jpegEnc_cfg.size = 3000;
    jpegEnc_cfg.fQuality = 95;
    jpegEnc_cfg.soi_en = 1;
    //status = pStream->setPortType(0,1,&jpegEnc_cfg);

#if 0
    /* Got each function's frame information here */
    ISPInfo.sw.log_en = 1 ;
    ISPInfo.sw.src_width = 3264 ;
    ISPInfo.sw.src_height = 2448 ;
    ISPInfo.sw.tpipe_width = 768 ;
    ISPInfo.sw.tpipe_height = 8192 ;
    ISPInfo.sw.tpipe_irq_mode = 0 ;
    ISPInfo.top.scenario = 6 ;
    ISPInfo.top.mode = 0 ;
    ISPInfo.top.pixel_id = 0 ;
    ISPInfo.top.cam_in_fmt = 0 ;
    ISPInfo.top.ctl_extension_en = 0 ;
    ISPInfo.top.fg_mode = 0 ;
    ISPInfo.top.ufdi_fmt = 0 ;
    ISPInfo.top.vipi_fmt = 2 ;
    ISPInfo.top.img3o_fmt = 2 ;
    ISPInfo.top.imgi_en = 1 ;
    ISPInfo.top.g2c_en = 1 ;
    ISPInfo.top.c42_en = 1 ;
    ISPInfo.top.nbc_en = 1 ;
    ISPInfo.top.sl2c_en = 1 ;
    ISPInfo.top.seee_en = 1 ;
    ISPInfo.top.crz_en = 1 ;
    ISPInfo.top.img2o_en = 1 ;
    ISPInfo.top.crsp_en = 0 ;
    ISPInfo.top.img3o_en = 1 ;
    ISPInfo.top.c24b_en = 1 ;
    ISPInfo.top.mdp_crop_en = 1 ;
    ISPInfo.top.imgi_v_flip_en = 0 ;
    ISPInfo.top.ufd_sel = 0 ;
    ISPInfo.top.ccl_sel = 0 ;
    ISPInfo.top.ccl_sel_en = 1 ;
    ISPInfo.top.g2g_sel = 1 ;
    ISPInfo.top.g2g_sel_en = 1 ;
    ISPInfo.top.mix1_sel = 0 ;
    ISPInfo.top.crz_sel = 0 ;
    ISPInfo.top.nr3d_sel = 0 ;
    ISPInfo.top.fe_sel = 1 ;
    ISPInfo.top.mdp_sel = 0 ;
    ISPInfo.top.pca_sel = 0 ;
    ISPInfo.top.interlace_mode = 0 ;
    ISPInfo.imgi.imgi_stride = 4080 ;
    ISPInfo.g2c.g2c_shade_en = 1 ;
    ISPInfo.nbc.anr_eny = 1 ;
    ISPInfo.nbc.anr_enc = 1 ;
    ISPInfo.nbc.anr_iir_mode = 0 ;
    ISPInfo.nbc.anr_scale_mode = 2 ;
    ISPInfo.sl2c.sl2c_hrz_comp = 0 ;
    ISPInfo.seee.se_edge = 1 ;
    ISPInfo.cdrz.cdrz_input_crop_width = 2612 ;
    ISPInfo.cdrz.cdrz_input_crop_height = 1959 ;
    ISPInfo.cdrz.cdrz_output_width = 3264 ;
    ISPInfo.cdrz.cdrz_output_height = 2448 ;
    ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 326 ;
    ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0 ;
    ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 244 ;
    ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0 ;
    ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0 ;
    ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0 ;
    ISPInfo.cdrz.cdrz_horizontal_coeff_step = 26220 ;
    ISPInfo.cdrz.cdrz_vertical_coeff_step = 26220 ;
    ISPInfo.img2o.img2o_stride = 0 ;
    ISPInfo.img2o.img2o_xoffset = 0 ;
    ISPInfo.img2o.img2o_yoffset = 0 ;
    ISPInfo.img2o.img2o_xsize = 6527 ;
    ISPInfo.img2o.img2o_ysize = 2447 ;
    ISPInfo.img3o.img3o_stride = 0 ;
    ISPInfo.img3o.img3o_xoffset = 0 ;
    ISPInfo.img3o.img3o_yoffset = 0 ;
    ISPInfo.img3o.img3o_xsize = 6527 ;
    ISPInfo.img3o.img3o_ysize = 2447 ;

#endif

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");


    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    while(1)
    {
        status = pStream->dequeueDstBuffer(0, &outBuf0[0], true);
        if(DP_STATUS_RETURN_SUCCESS == status)
        {
            DPLOGD("dequeueDstBuffer Pass : %d\n",status);
            break;
        }

        DPLOGD("dequeueDstBuffer failed : %d\n",status);
        usleep(100);
    }
    //REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);
    uint32_t jpegEnc_filesize;
    pStream->getJPEGFilesize(&jpegEnc_filesize);
    DPLOGI("JPEG File size: %d\n", jpegEnc_filesize);
    status = pStream->dequeueSrcBuffer();

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream2", IspStream2, IspStreamCase2);



/*
 * ISP stream Test #3: 1 in 2 out with rotation
 */
void IspStreamCase3(TestReporter *pReporter)
{
        DP_STATUS_ENUM              status;
        uint8_t                     *pSource;
        uint8_t                     *pSource1;
        uint8_t                     *pOutput0,*pOutput1;
        uint8_t                     *pOutput0_1,*pOutput1_1;
        void                        *outBuf0[3];
        uint32_t                    outSize[3];

        runCounter = 0;
        frameCounter = 0;

        ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
        memset(&ISPInfo,0,sizeof(ISPInfo));

        pStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
        REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

        pSource = (uint8_t*)malloc(640 * 480 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");
        pSource1 = (uint8_t*)malloc(320 * 240 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");


        pOutput0 = (uint8_t*)malloc(1280 * 720 * 2);
        pOutput0_1 = (uint8_t*)malloc(1280 * 720 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer0\n");
        DPLOGI("Malloc target image buffer0: %p\n", pOutput0);
        DPLOGI("Malloc target image buffer1: %p\n", pOutput0_1);

        pOutput1 = (uint8_t*)malloc(960 * 540 * 2);
        pOutput1_1 = (uint8_t*)malloc(960 * 540 * 2);

        DPLOGI("Malloc target image buffer1: %p\n", pOutput1);
        DPLOGI("Malloc target image buffer1: %p\n", pOutput1_1);

        // Copy source image
        status = utilReadBMP("/system/pat/640x480.bmp",
            (uint8_t*)pSource,
            (uint8_t*)NULL,
            (uint8_t*)NULL,
            eYUV_422_I,
            640,
            480,
            1280);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

        // Copy source image
        status = utilReadBMP("/system/pat/320x240.bmp",
            (uint8_t*)pSource1,
            (uint8_t*)NULL,
            (uint8_t*)NULL,
            eYUV_422_I,
            320,
            240,
            640);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

        pthread_mutex_init(&frCounterMutex, NULL);
        pthread_cond_init(&frameCond, NULL);

        while(runCounter<200000)
        {



            status = pStream->queueSrcBuffer(pSource,
                640 * 480 * 2);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

            status = pStream->setSrcConfig(eYUV_422_I,
                640,
                480,
                640 * 2);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");



            outBuf0[0] = pOutput0;
            outBuf0[1] = pOutput0 + (1280*720);
            outBuf0[2] = pOutput0 + (1280*720*3);

            outSize[0] = 1280 * 720 * 2 ;
            outSize[1] = 1280 * 720;
            outSize[2] = 1280 * 720;
            status = pStream->queueDstBuffer(0,
                                             outBuf0,
                                             outSize,
                                             1);

            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

            status = pStream->setDstConfig(0,
                                           720,
                                           1280,
                                           1440,
                                           0,
                                           DP_COLOR_YUYV);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

            status = pStream->setRotation(0,270);

            REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer1\n");


            outBuf0[0] = pOutput1;
            outBuf0[1] = 0;
            outBuf0[2] = 0;

            outSize[0] = 960 * 540 * 2;
            outSize[1] = 0;
            outSize[2] = 0;
            status = pStream->queueDstBuffer(1,
                outBuf0,
                outSize,
                1);

            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

            status = pStream->setDstConfig(1,
                960,
                540,
                1920,
                0,
                DP_COLOR_YUYV);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

            DpJPEGEnc_Config jpegEnc_cfg;
            jpegEnc_cfg.fileDesc = 7;
            jpegEnc_cfg.size = 3000;
            jpegEnc_cfg.fQuality = 95;
            jpegEnc_cfg.soi_en = 1;
            //status = pStream->setPortType(0,1,&jpegEnc_cfg);

    #if 0
            /* Got each function's frame information here */
            ISPInfo.sw.log_en = 1 ;
            ISPInfo.sw.src_width = 3264 ;
            ISPInfo.sw.src_height = 2448 ;
            ISPInfo.sw.tpipe_width = 768 ;
            ISPInfo.sw.tpipe_height = 8192 ;
            ISPInfo.sw.tpipe_irq_mode = 0 ;
            ISPInfo.top.scenario = 6 ;
            ISPInfo.top.mode = 0 ;
            ISPInfo.top.pixel_id = 0 ;
            ISPInfo.top.cam_in_fmt = 0 ;
            ISPInfo.top.ctl_extension_en = 0 ;
            ISPInfo.top.fg_mode = 0 ;
            ISPInfo.top.ufdi_fmt = 0 ;
            ISPInfo.top.vipi_fmt = 2 ;
            ISPInfo.top.img3o_fmt = 2 ;
            ISPInfo.top.imgi_en = 1 ;
            ISPInfo.top.g2c_en = 1 ;
            ISPInfo.top.c42_en = 1 ;
            ISPInfo.top.nbc_en = 1 ;
            ISPInfo.top.sl2c_en = 1 ;
            ISPInfo.top.seee_en = 1 ;
            ISPInfo.top.crz_en = 1 ;
            ISPInfo.top.img2o_en = 1 ;
            ISPInfo.top.crsp_en = 0 ;
            ISPInfo.top.img3o_en = 1 ;
            ISPInfo.top.c24b_en = 1 ;
            ISPInfo.top.mdp_crop_en = 1 ;
            ISPInfo.top.imgi_v_flip_en = 0 ;
            ISPInfo.top.ufd_sel = 0 ;
            ISPInfo.top.ccl_sel = 0 ;
            ISPInfo.top.ccl_sel_en = 1 ;
            ISPInfo.top.g2g_sel = 1 ;
            ISPInfo.top.g2g_sel_en = 1 ;
            ISPInfo.top.mix1_sel = 0 ;
            ISPInfo.top.crz_sel = 0 ;
            ISPInfo.top.nr3d_sel = 0 ;
            ISPInfo.top.fe_sel = 1 ;
            ISPInfo.top.mdp_sel = 0 ;
            ISPInfo.top.pca_sel = 0 ;
            ISPInfo.top.interlace_mode = 0 ;
            ISPInfo.imgi.imgi_stride = 4080 ;
            ISPInfo.g2c.g2c_shade_en = 1 ;
            ISPInfo.nbc.anr_eny = 1 ;
            ISPInfo.nbc.anr_enc = 1 ;
            ISPInfo.nbc.anr_iir_mode = 0 ;
            ISPInfo.nbc.anr_scale_mode = 2 ;
            ISPInfo.sl2c.sl2c_hrz_comp = 0 ;
            ISPInfo.seee.se_edge = 1 ;
            ISPInfo.cdrz.cdrz_input_crop_width = 2612 ;
            ISPInfo.cdrz.cdrz_input_crop_height = 1959 ;
            ISPInfo.cdrz.cdrz_output_width = 3264 ;
            ISPInfo.cdrz.cdrz_output_height = 2448 ;
            ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 326 ;
            ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0 ;
            ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 244 ;
            ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0 ;
            ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0 ;
            ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0 ;
            ISPInfo.cdrz.cdrz_horizontal_coeff_step = 26220 ;
            ISPInfo.cdrz.cdrz_vertical_coeff_step = 26220 ;
            ISPInfo.img2o.img2o_stride = 0 ;
            ISPInfo.img2o.img2o_xoffset = 0 ;
            ISPInfo.img2o.img2o_yoffset = 0 ;
            ISPInfo.img2o.img2o_xsize = 6527 ;
            ISPInfo.img2o.img2o_ysize = 2447 ;
            ISPInfo.img3o.img3o_stride = 0 ;
            ISPInfo.img3o.img3o_xoffset = 0 ;
            ISPInfo.img3o.img3o_yoffset = 0 ;
            ISPInfo.img3o.img3o_xsize = 6527 ;
            ISPInfo.img3o.img3o_ysize = 2447 ;

    #endif

            status = pStream->setParameter(ISPInfo);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

            status = pStream->startStream();
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");


            status = pStream->stopStream();
            //First frame end
            pthread_mutex_lock(&frCounterMutex);
            frameCounter++;
            pthread_mutex_unlock(&frCounterMutex);



            status = pStream->queueSrcBuffer(pSource1,
                320 * 240 * 2);

            status = pStream->setSrcConfig(eYUV_422_I,
                320,
                240,
                320 * 2);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");


            outBuf0[0] = pOutput0_1;
            outBuf0[1] = 0;
            outBuf0[2] = 0;

            outSize[0] = 1280 * 720 * 2;
            outSize[1] = 0;
            outSize[2] = 0;

            status = pStream->queueDstBuffer(0,
                outBuf0,
                outSize,
                1);

            status = pStream->setDstConfig(0,
                                           720,
                                           1280,
                                           1440,
                                           0,
                                           DP_COLOR_YUYV);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

            //status = pStream->setPortType(0,1,&jpegEnc_cfg);

            status = pStream->setRotation(0,270);


            outBuf0[0] = pOutput1_1;
            outBuf0[1] = 0;
            outBuf0[2] = 0;

            outSize[0] = 960 * 540 * 2;
            outSize[1] = 0;
            outSize[2] = 0;

            status = pStream->queueDstBuffer(1,
                outBuf0,
                outSize,
                1);

            status = pStream->setDstConfig(1,
                960,
                540,
                1920,
                0,
                DP_COLOR_YUYV);
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");


            status = pStream->startStream();

            if(0xFFFFFFFF == runCounter)
            {
                pthread_t id;
                pthread_create(&id,NULL,dequeueThread,NULL);
            }

            status = pStream->stopStream();


#if 1
            status = pStream->dequeueSrcBuffer();
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

            status = pStream->dequeueDstBuffer(0,
                                               &outBuf0[0],
                                               true);



            status = pStream->dequeueDstBuffer(1,
                &outBuf0[0],
                true);

            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");
            DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);


            status = pStream->dequeueFrameEnd();

            status = pStream->dequeueSrcBuffer();
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

            status = pStream->dequeueDstBuffer(0,
                                               &outBuf0[0],
                                               true);

            status = pStream->dequeueDstBuffer(1,
                &outBuf0[0],
                true);


            status = pStream->dequeueFrameEnd();
#endif
            REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");
            //2nd frame done
            pthread_mutex_lock(&frCounterMutex);
            frameCounter++;
            pthread_mutex_unlock(&frCounterMutex);

            pthread_cond_signal(&frameCond);

            runCounter++;

        }

        DPLOGI("============================Main thread is done============================\n");

#if 0
        while(dequeueCtr < 4)
        {
            if(dequeueCtr > 3)
                break;
        }
#endif

        if (NULL != pOutput0)
        {
            delete pOutput0;
            pOutput0 = NULL;
        }

        if (NULL != pSource)
        {
            delete pSource;
            pSource = NULL;
        }

        if (NULL != pStream)
        {
            delete pStream;
        }
}


DEFINE_TEST_CASE("IspStream3", IspStream3, IspStreamCase3);



/*
 * ISP stream Test #4: 1 in 2 out test case
 */

#define MULTIPLE_INSTANCE_NUM (20)

DpIspStream*                pStreamArray[MULTIPLE_INSTANCE_NUM] = {0};
uint8_t                     *pOutput0[MULTIPLE_INSTANCE_NUM] = {0};
uint8_t                     *pOutput1[MULTIPLE_INSTANCE_NUM] = {0};
uint8_t                     *pOutput0_1[MULTIPLE_INSTANCE_NUM] = {0};
uint8_t                     *pOutput1_1[MULTIPLE_INSTANCE_NUM] = {0};
uint8_t                     *pSource = NULL;
uint8_t                     *pSource1 = NULL;

DP_STATUS_ENUM              status;


static void* IspStreamQueue1(void* pInput)
{

    void                        *outBuf0[3];
    uint32_t                    outSize[3];
    uint32_t index;

    index = (unsigned long)pInput;
    DPLOGI("=========================== ISP IspStreamQueue1 ID: %d ============================\n",index);

    pOutput0[index] = (uint8_t*)malloc(960 * 540 * 2);
    pOutput0_1[index] = (uint8_t*)malloc(1280 * 720 * 2);
    DPLOGI("Malloc target image buffer0: %p\n", pOutput0[index]);
    DPLOGI("Malloc target image buffer0_1: %p\n", pOutput0_1[index]);

    pStreamArray[index] = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);

    status = pStreamArray[index]->queueSrcBuffer(pSource, 640 * 480 * 2);
    status = pStreamArray[index]->setSrcConfig(eYUV_422_I, 640, 480, 640 * 2);

    outBuf0[0] = pOutput0[index];
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 960 * 540 * 2 ;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStreamArray[index]->queueDstBuffer(0, outBuf0, outSize, 1);
    status = pStreamArray[index]->setDstConfig(0, 540, 960, 1080, 0, DP_COLOR_YUYV);
    status = pStreamArray[index]->setRotation(0,270);

    outBuf0[0] = pOutput0_1[index];
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 1280 * 720 * 2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStreamArray[index]->queueDstBuffer(1, outBuf0, outSize, 1);
    status = pStreamArray[index]->setDstConfig(1, 1280, 720, 2560, 0, DP_COLOR_YUYV);

    DPLOGI("IspStreamQueue1[%d] is frameCtr before: %d\n",index,frameCounter);
    pthread_mutex_lock(&frCounterMutex);
    frameCounter++;
    pthread_mutex_unlock(&frCounterMutex);
    DPLOGI("IspStreamQueue1[%d] is frameCtr after: %d\n",index,frameCounter);
    pthread_cond_signal(&frameCond);

    return 0;

}

static void* IspStreamStart1(void* pInput)
{
    uint32_t index;
    void                        *outBuf0[3];
    uint32_t                    outSize[3];


    index = (unsigned long)pInput;
    status = pStreamArray[index]->startStream();
    status = pStreamArray[index]->stopStream();

    DPLOGI("=========================== ISP IspStreamStart1 ID: %d ============================\n",index);
    pthread_mutex_lock(&frCounterMutex);
    frameCounter++;
    pthread_mutex_unlock(&frCounterMutex);
    DPLOGI("=========================== IspStreamStart1 is frameCtr: %d ============================\n",frameCounter);
    pthread_cond_signal(&frameCond);
    return 0;

}

static void* IspStreamQueue2(void* pInput)
{
    uint32_t index;
    void                        *outBuf0[3];
    uint32_t                    outSize[3];


    index = (unsigned long)pInput;
    pOutput1[index] = (uint8_t*)malloc(240 * 160 * 2);
    pOutput1_1[index] = (uint8_t*)malloc(160 * 128 * 2);

    DPLOGI("Malloc target image buffer1: %p\n", pOutput1[index]);
    DPLOGI("Malloc target image buffer1_1: %p\n", pOutput1_1[index]);

    //First frame end
    status = pStreamArray[index]->queueSrcBuffer(pSource1,320 * 240 * 2);

    status = pStreamArray[index]->setSrcConfig(eYUV_422_I,320, 240, 320 * 2);

    outBuf0[0] = pOutput1[index];
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 240 * 160 * 2;
    outSize[1] = 0;
    outSize[2] = 0;

    status = pStreamArray[index]->queueDstBuffer(0, outBuf0, outSize, 1);
    status = pStreamArray[index]->setDstConfig(0, 160, 240, 320, 0, DP_COLOR_YUYV);
    status = pStreamArray[index]->setRotation(0,270);

    outBuf0[0] = pOutput1_1[index];
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 160 * 128 * 2;
    outSize[1] = 0;
    outSize[2] = 0;

    status = pStreamArray[index]->queueDstBuffer(1, outBuf0, outSize, 1);
    status = pStreamArray[index]->setDstConfig(1, 160, 128, 320, 0, DP_COLOR_YUYV);

    DPLOGI("=========================== ISP IspStreamQueue2, ID: %d ============================\n",index);
    pthread_mutex_lock(&frCounterMutex);
    frameCounter++;
    pthread_mutex_unlock(&frCounterMutex);
    DPLOGI("=========================== IspStreamQueue2 is frameCtr: %d ============================\n",frameCounter);
    pthread_cond_signal(&frameCond);
    return 0;
}

static void* IspStreamStart2(void* pInput)
{
    uint32_t index;
    void                        *outBuf0[3];
    uint32_t                    outSize[3];

    index = (unsigned long)pInput;

    status = pStreamArray[index]->startStream();
    status = pStreamArray[index]->stopStream();

    status = pStreamArray[index]->dequeueSrcBuffer();
    status = pStreamArray[index]->dequeueDstBuffer(0, &outBuf0[0], true);

#if 1
    {
        char     name[256];
        sprintf(name, "/system/out/output%d_0.bmp", index);
        utilWriteBMP(name, outBuf0[0],
            outBuf0[1],
            outBuf0[2],
            DP_COLOR_YUYV,
            540,
            960,
            1080,
            0);

    }
#endif

    status = pStreamArray[index]->dequeueDstBuffer(1, &outBuf0[0], true);

#if 1
    {
        char     name[256];
        sprintf(name, "/system/out/output%d_0_1.bmp", index);
        utilWriteBMP(name,
            outBuf0[0],
            outBuf0[1],
            outBuf0[2],
            DP_COLOR_YUYV,
            1280,
            720,
            2560,
            0);

    }
#endif
    status = pStreamArray[index]->dequeueFrameEnd();

    status = pStreamArray[index]->dequeueSrcBuffer();
    status = pStreamArray[index]->dequeueDstBuffer(0, &outBuf0[0], true);

#if 1
    {
        char     name[256];
        sprintf(name, "/system/out/output%d_1.bmp", index);
        utilWriteBMP(name, outBuf0[0],
            outBuf0[1],
            outBuf0[2],
            DP_COLOR_YUYV,
            160,
            240,
            320,
            0);

    }
#endif

    status = pStreamArray[index]->dequeueDstBuffer(1, &outBuf0[0], true);
#if 1
    {
        char     name[256];
        sprintf(name, "/system/out/output%d_1_1.bmp", index);
        utilWriteBMP(name, outBuf0[0],
            outBuf0[1],
            outBuf0[2],
            DP_COLOR_YUYV,
            160,
            128,
            320,
            0);

    }
#endif
    status = pStreamArray[index]->dequeueFrameEnd();



    if (NULL != pOutput0[index])
    {
        delete (pOutput0[index]);
        pOutput0[index] = NULL;
    }

    if (NULL != pOutput0_1[index])
    {
        delete (pOutput0_1[index]);
        pOutput0_1[index] = NULL;
    }

    if (NULL != pOutput1[index])
    {
        delete (pOutput1[index]);
        pOutput1[index] = NULL;
    }

    if (NULL != pOutput1_1[index])
    {
        delete (pOutput1_1[index]);
        pOutput1_1[index] = NULL;
    }

    if(NULL != pStreamArray[index])
    {
        delete pStreamArray[index];
        pStreamArray[index] = NULL;
    }

    DPLOGI("============================ISP thread %d is done============================\n",index);
    pthread_mutex_lock(&frCounterMutex);
    frameCounter++;
    pthread_mutex_unlock(&frCounterMutex);
    DPLOGI("===========================frameCounter is %d============================\n",frameCounter);
    pthread_cond_signal(&frameCond);

    return 0;

}


void IspStreamCase4(TestReporter *pReporter)
{
    pthread_t                   id;
    unsigned long               index;
    DP_STATUS_ENUM              status;

    runCounter = 0;
    frameCounter = 0;

    pthread_mutex_init(&frCounterMutex, NULL);
    pthread_cond_init(&frameCond, NULL);


    pSource = (uint8_t*)malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");
    pSource1 = (uint8_t*)malloc(320 * 240 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    // Copy source image
    status = utilReadBMP("/system/pat/640x480.bmp",
        (uint8_t*)pSource,
        (uint8_t*)NULL,
        (uint8_t*)NULL,
        eYUV_422_I,
        640,
        480,
        1280);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    // Copy source image
    status = utilReadBMP("/system/pat/320x240.bmp",
        (uint8_t*)pSource1,
        (uint8_t*)NULL,
        (uint8_t*)NULL,
        eYUV_422_I,
        320,
        240,
        640);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read input image source failed");

    for(index = 0;index<MULTIPLE_INSTANCE_NUM;index++)
    {
        if(0 != pthread_create(&id,NULL,IspStreamQueue1,(void*)index))
        {
            DPLOGE("IspStreamQueue1 ID: %d is failed\n",index);
        }
    }

    while(frameCounter < MULTIPLE_INSTANCE_NUM)
    {
        struct timespec outtime;
        outtime.tv_nsec = 500000000;
        outtime.tv_sec  = 0;

        pthread_mutex_lock(&frCounterMutex);
        pthread_cond_timedwait(&frameCond, &frCounterMutex,&outtime);
        pthread_mutex_unlock(&frCounterMutex);
    }
    frameCounter = 0;

    for(index = 0;index<MULTIPLE_INSTANCE_NUM;index++)
    {
        if(0 != pthread_create(&id,NULL,IspStreamStart1,(void*)index))
        {
            DPLOGE("IspStreamStart1 ID: %d is failed\n",index);
        }
    }

    while(frameCounter < MULTIPLE_INSTANCE_NUM)
    {
        pthread_mutex_lock(&frCounterMutex);
        pthread_cond_wait(&frameCond, &frCounterMutex);
        pthread_mutex_unlock(&frCounterMutex);
    }
    frameCounter = 0;

    for(index = 0;index<MULTIPLE_INSTANCE_NUM;index++)
    {
        if(0 != pthread_create(&id,NULL,IspStreamQueue2,(void*)index))
        {
            DPLOGE("IspStreamQueue2 ID: %d is failed\n",index);
        }
    }

    while(frameCounter < MULTIPLE_INSTANCE_NUM)
    {
        pthread_mutex_lock(&frCounterMutex);
        pthread_cond_wait(&frameCond, &frCounterMutex);
        pthread_mutex_unlock(&frCounterMutex);
    }
    frameCounter = 0;

    for(index = 0;index<MULTIPLE_INSTANCE_NUM;index++)
    {
        if(0 != pthread_create(&id,NULL,IspStreamStart2,(void*)index))
        {
            DPLOGE("IspStreamStart2 ID: %d is failed\n",index);
        }
    }

    while(frameCounter < MULTIPLE_INSTANCE_NUM)
    {
        pthread_mutex_lock(&frCounterMutex);
        pthread_cond_wait(&frameCond, &frCounterMutex);
        pthread_mutex_unlock(&frCounterMutex);
    }

#if 0

    for(index = 0;index < MULTIPLE_INSTANCE_NUM ;index++)
    {
        pOutput0[index] = (uint8_t*)malloc(960 * 540 * 2);
        pOutput0_1[index] = (uint8_t*)malloc(1280 * 720 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0[index] != NULL), "Can't allocate target image buffer0\n");
        DPLOGI("Malloc target image buffer0: %p\n", pOutput0[index]);
        DPLOGI("Malloc target image buffer0_1: %p\n", pOutput0_1[index]);

        pStreamArray[index] = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
        REPORTER_ASSERT_MESSAGE(pReporter, (pStreamArray[index] != NULL), "Can't allocate DpIspStream object\n");

        status = pStreamArray[index]->queueSrcBuffer(pSource, 640 * 480 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

        status = pStreamArray[index]->setSrcConfig(eYUV_422_I, 640, 480, 640 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

        outBuf0[0] = pOutput0[index];
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outSize[0] = 960 * 540 * 2 ;
        outSize[1] = 0;
        outSize[2] = 0;
        status = pStreamArray[index]->queueDstBuffer(0, outBuf0, outSize, 1);

        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

        status = pStreamArray[index]->setDstConfig(0, 540, 960, 1080, 0, DP_COLOR_YUYV);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

        status = pStreamArray[index]->setRotation(0,270);

        REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer1\n");

        outBuf0[0] = pOutput0_1[index];
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outSize[0] = 1280 * 720 * 2;
        outSize[1] = 0;
        outSize[2] = 0;
        status = pStreamArray[index]->queueDstBuffer(1, outBuf0, outSize, 1);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

        status = pStreamArray[index]->setDstConfig(1, 1280, 720, 2560, 0, DP_COLOR_YUYV);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    }

    for(index = 0;index < MULTIPLE_INSTANCE_NUM ;index++)
    {
        status = pStreamArray[index]->startStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");
        status = pStreamArray[index]->stopStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop ISP stream processing failed\n");
    }

    for(index = 0;index < MULTIPLE_INSTANCE_NUM ;index++)
    {
        pOutput1[index] = (uint8_t*)malloc(240 * 160 * 2);
        pOutput1_1[index] = (uint8_t*)malloc(160 * 128 * 2);

        DPLOGI("Malloc target image buffer1: %p\n", pOutput1[index]);
        DPLOGI("Malloc target image buffer1_1: %p\n", pOutput1_1[index]);

        //First frame end
        status = pStreamArray[index]->queueSrcBuffer(pSource1,320 * 240 * 2);

        status = pStreamArray[index]->setSrcConfig(eYUV_422_I,320, 240, 320 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

        outBuf0[0] = pOutput1[index];
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outSize[0] = 240 * 160 * 2;
        outSize[1] = 0;
        outSize[2] = 0;

        status = pStreamArray[index]->queueDstBuffer(0, outBuf0, outSize, 1);

        status = pStreamArray[index]->setDstConfig(0, 160, 240, 320, 0, DP_COLOR_YUYV);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

        status = pStreamArray[index]->setRotation(0,270);

        outBuf0[0] = pOutput1_1[index];
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outSize[0] = 160 * 128 * 2;
        outSize[1] = 0;
        outSize[2] = 0;

        status = pStreamArray[index]->queueDstBuffer(1, outBuf0, outSize, 1);

        status = pStreamArray[index]->setDstConfig(1, 160, 128, 320, 0, DP_COLOR_YUYV);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");
    }

    for(index = 0;index < MULTIPLE_INSTANCE_NUM ;index++)
    {
        status = pStreamArray[index]->startStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");
        status = pStreamArray[index]->stopStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Stop ISP stream processing failed\n");
    }

    for(index = 0;index < MULTIPLE_INSTANCE_NUM ;index++)
    {
        status = pStreamArray[index]->dequeueSrcBuffer();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

        status = pStreamArray[index]->dequeueDstBuffer(0, &outBuf0[0], true);
#if 1
        {
            char     name[256];
            sprintf(name, "/system/out/output%d_0.bmp", index);
            utilWriteBMP(name, outBuf0[0],
                               outBuf0[1],
                               outBuf0[2],
                               DP_COLOR_YUYV,
                               540,
                               960,
                               1080,
                               0);

        }
#endif

        status = pStreamArray[index]->dequeueDstBuffer(1, &outBuf0[0], true);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");
#if 1
        {
            char     name[256];
            sprintf(name, "/system/out/output%d_0_1.bmp", index);
            utilWriteBMP(name,
                            outBuf0[0],
                            outBuf0[1],
                            outBuf0[2],
                            DP_COLOR_YUYV,
                            1280,
                            720,
                            2560,
                            0);

        }
#endif


        status = pStreamArray[index]->dequeueFrameEnd();

        status = pStreamArray[index]->dequeueSrcBuffer();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

        status = pStreamArray[index]->dequeueDstBuffer(0, &outBuf0[0], true);

#if 1
        {
            char     name[256];
            sprintf(name, "/system/out/output%d_1.bmp", index);
            utilWriteBMP(name, outBuf0[0],
                                outBuf0[1],
                                outBuf0[2],
                                DP_COLOR_YUYV,
                                160,
                                240,
                                320,
                                0);

        }
#endif

        status = pStreamArray[index]->dequeueDstBuffer(1, &outBuf0[0], true);
#if 1
        {
            char     name[256];
            sprintf(name, "/system/out/output%d_1_1.bmp", index);
            utilWriteBMP(name, outBuf0[0],
                            outBuf0[1],
                            outBuf0[2],
                            DP_COLOR_YUYV,
                            160,
                            128,
                            320,
                            0);

        }
#endif

        status = pStreamArray[index]->dequeueFrameEnd();


    }


    DPLOGI("============================Main thread is done============================\n");

    for(index = 0;index < MULTIPLE_INSTANCE_NUM; index++)
    {
        if (NULL != pOutput0[index])
        {
            delete (pOutput0[index]);
            pOutput0[index] = NULL;
        }

        if (NULL != pOutput0_1[index])
        {
            delete (pOutput0_1[index]);
            pOutput0_1[index] = NULL;
        }

        if (NULL != pOutput1[index])
        {
            delete (pOutput1[index]);
            pOutput1[index] = NULL;
        }

        if (NULL != pOutput1_1[index])
        {
            delete (pOutput1_1[index]);
            pOutput1_1[index] = NULL;
        }

    }

#endif

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    if (NULL != pSource1)
    {
        free(pSource);
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream4", IspStream4, IspStreamCase4);



/*
 * ISP stream Test #5: 1 in 3 out test case
 */
void IspStreamCase5(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pBuffer = 0;
    uint8_t                     *pSource0 = 0;
    uint8_t                     *pSource1 = 0;
    uint8_t                     *pOutput0 = 0;
    uint8_t                     *pOutput1 = 0;
    uint8_t                     *pOutput2 = 0;

    uint32_t                    outSize[3];
    void                        *outBuf0[3];
    void                        *outBuf1[3];

#define input_width (1920)
#define input_height (1088)
#define input_size (1920*1088)

#define input_buffer_size (input_size + 128)
#define input_align(ptr) (uint8_t*)((((unsigned long)(ptr) + 127) >> 7) << 7)

#define output_width (2560)
#define output_height (1440)
#define output_size (2560*1440)
#define output_format (DP_COLOR_YUYV)


    pStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pBuffer = (uint8_t*)malloc(input_buffer_size * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pBuffer != NULL), "Can't allocate source image buffer\n");

    pSource0 = input_align(pBuffer);
    pSource1 = input_align(pBuffer + input_buffer_size);

    // Copy source image
    status = utilReadBMP("/system/pat/1920x1088.bmp",
        pSource0,
        pSource1,
        (uint8_t*)NULL,
        DP_COLOR_420_BLKP,
        input_width,
        input_height,
        input_width);

    outBuf0[0] = pSource0;
    outBuf0[1] = pSource1;
    outBuf0[2] = 0;

    outSize[0] = input_size;
    outSize[1] = input_size;
    outSize[2] = 0;

#if 1
    status = utilWriteBMP("/system/out/input1.bmp",
        (uint8_t*)outBuf0[0],
        (uint8_t*)outBuf0[1],
        (uint8_t*)outBuf0[2],
        DP_COLOR_420_BLKP,
        input_width,
        input_height,
        input_width,input_width >> 1);
    status = utilWriteBin("/system/out/input1.bin",
        (uint8_t*)outBuf0[0],
        (uint8_t*)outBuf0[1],
        (uint8_t*)outBuf0[2],
        DP_COLOR_420_BLKP,
        input_width,
        input_height,
        input_width,input_width >> 1);
#endif

    status = pStream->queueSrcBuffer(outBuf0, outSize, 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(input_width,
                                   input_height,
                                   input_width*32,input_width*16,DP_COLOR_420_BLKP);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    /*
     * Target port0
     */
#if 1
    pOutput0 = (uint8_t*)malloc(output_size * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);
    outBuf0[0] = pOutput0;
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = output_size * 2 ;
    outSize[1] = 0;
    outSize[2] = 0;

    status = pStream->queueDstBuffer(0,
                                     outBuf0,outSize,
                                     DP_COLOR_GET_PLANE_COUNT(output_format));
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0, output_height,
                                      output_width,
                                      output_height * 2,
                                      0,
                                      output_format);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    pStream->setRotation(0,90);
    pStream->setSharpness(0,0x30003);
#endif

    /*
     * Target port1
     */
#if 0
    pOutput1 = (uint8_t*)malloc(1920 * 1080 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image image buffer0: %p\n", pOutput1);
    outBuf0[0] = pOutput1;
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 1920 * 1080 * 2;
    outSize[1] = 0;
    outSize[2] = 0;

    status = pStream->queueDstBuffer(1, outBuf0,
                                     outSize,
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

    status = pStream->setDstConfig(1,
                                   DP_COLOR_YUYV,
                                   1920,
                                   1080,
                                   3840);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");
    pStream->setSharpness(1,0x30003);
#endif

#if 0
    /*
     * Target port2
     */
    pOutput2 = (uint8_t*)malloc(1920*540*3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput2 != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image image buffer1: %p\n", pOutput2);
    outBuf0[0] = pOutput2;
    outBuf0[1] = pOutput2 + (1920*1080);
    outBuf0[2] = (uint8_t)outBuf0[1]+(960*540);

    outSize[0] = 640*480*2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(2,
                                     outBuf0,outSize,
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer2 failed\n");

    status = pStream->setDstConfig(2,
                                   DP_COLOR_YV12,
                                   640,
                                   480,
                                   1280);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure output port3 failed\n");
#endif
    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

#if 1
    status = pStream->dequeueDstBuffer(0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);
#endif
#if 0
    status = pStream->dequeueDstBuffer(1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);
#endif
//    status = pStream->dequeueDstBuffer(2,
//                                       &outBuf2[0],
//                                       true);
//    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer2 failed\n");

//   DPLOGI("Port2 target buffer: 0x%08x\n", outBuf2[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    /*
     * Dump output port0
     */
#if 1
    status = utilWriteBMP("/system/out/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          output_format,
                          output_height,
                          output_width,
                          output_height*2,0);
    status = utilWriteBin("/system/out/output1.bin",
                            (uint8_t*)outBuf0[0],
                            (uint8_t*)outBuf0[1],
                            (uint8_t*)outBuf0[2],
                            output_format,
                            output_height,
                            output_width,
                            output_height*2,0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");
#endif
    /*
     * Dump target port1
     */
#if 0
    status = utilWriteBMP(".//out//output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_YUYV,
                          1920,
                          1080,
                          3840,0);
    status = utilWriteBin(".//out//output2.bin",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_YUYV,
                          1920,
                          1080,
                          3840,0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");
#endif
#if 0
    /*
     * Dump target port2
     */
    status = utilWriteBMP(".//out//output3.bmp",
                          (uint8_t*)outBuf2[0],
                          (uint8_t*)outBuf2[1],
                          (uint8_t*)outBuf2[2],
                          DP_COLOR_YUYV,
                          640,
                          480,
                          1280,0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");
#endif

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pOutput1)
    {
        delete pOutput1;
        pOutput1 = NULL;
    }

    if (NULL != pOutput2)
    {
        delete pOutput2;
        pOutput2 = NULL;
    }

    if (NULL != pBuffer)
    {
        delete pBuffer;
        pBuffer = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream5", IspStream5, IspStreamCase5);


/*
 * ISP stream Test #6: 1 in 3 out test case
 */
#define CQ_APB                  0x00	//jump to virtual memory
#define CQ_IDLE      			0x3A	//back to idle
#define CQ_TRIGGER_POLLING      0x3B	//trigger isp, wait isp finish
#define CQ_TRIGGER_IDLE         0x3F	//trigger isp, back to idle
#define REG_RANGE_SIZE			0x4000
#define CAM_ISP_BIN_START_ADDR	0x15004000
#define CAM_BASE				0x15000000
#define SET_CQ_APB_ADDR(_x_)    ((_x_)&(0xFFFF)) 			/* bit0 -bit15 */
#define SET_CQ_CMD_COUNT(_x_)   (((_x_)<<16)&(0x3FF0000)) 	/* bit16 -bit25 */
#define SET_CQ_COMMAND(_x_)     (((_x_)<<26)&(0xFC000000)) 	/* bit26 -bit31 */


void setting_cq_path_writesetting(uint32_t *cqtablePA,uint32_t regvalueArray)	//return cq table base addr(physical addr)
{
    int ret=0;
    //construct cq table virtual register map
    //construct cq table descriptor
    uint32_t curAddr=CAM_ISP_BIN_START_ADDR;
    int cnt=0;
    for(int i=0;i<REG_RANGE_SIZE/sizeof(uint32_t);i++)
    {

        if((curAddr==0x15004000)/*start*/||(curAddr==0x1500408c)/*reset*/||
            (curAddr==0x150040a0)/*CQ1 baseAddr*/||(curAddr==0x150040a4)/*CQ2 baseAddr*/||(curAddr==0x150040a8)/*CQ3 baseAddr*/||
            (curAddr==0x1500410c)/*CQ1,2,3 enable*/|| (curAddr==0x15007204)/*tdri baseAddr*/||
            (((uint32_t)curAddr>(uint32_t)0x15004400) && ((uint32_t)curAddr<(uint32_t)0x15004500))/*TG*/   ||
            (((uint32_t)curAddr>(uint32_t)0x15006400) && ((uint32_t)curAddr<(uint32_t)0x15006500))/*TG2*/ ||
            (curAddr==0x15004004)/*p1_en*/ || (curAddr==0x15004008)/*p1_en_dma*/|| (curAddr==0x15004010)/*p1_d_en*/||
            (curAddr==0x15004014)/*p1_d_en_dma*/|| (curAddr==0x15004020)/*p1_cq_en*/ /*|| (curAddr==0x15004024)*//*scenario*/||
            (curAddr==0x15004028)/*p1_fmt_sel*/ || (curAddr==0x1500402c)/*p1_d_fmt_sel*/ || (curAddr==0x15004034)/*cam_sel_p1*/ ||
            (curAddr==0x15004038)/*cam_sel_p1_d*/ || (curAddr==0x15004040)/*global_sel*/ || (curAddr==0x15004048)/*p1_int_en*/ ||
            (curAddr==0x1500404c)/*p1_int_st*/ ||(curAddr==0x15004050)/*p1_dma_int_en*/ ||(curAddr==0x15004054)/*p1_dma_int_st*/ ||
            (curAddr==0x15004058)/*p1_d_int_en*/ ||(curAddr==0x1500405c)/*p1_d_int_st*/ || (curAddr==0x15004060)/*p1_d_dma_int_en*/ ||
            (curAddr==0x15004064)/*p1_d_dma_int_st*/ ||
            (((uint32_t)curAddr>=(uint32_t)0x15004070) && ((uint32_t)curAddr<=(uint32_t)0x15004078))/*statusx,2x,3x*/  ||
            (((uint32_t)curAddr>=(uint32_t)0x15007300) && ((uint32_t)curAddr<(uint32_t)0x15007320))/*imgo related*/  ||
            (((uint32_t)curAddr>=(uint32_t)0x150040a0) && ((uint32_t)curAddr<=(uint32_t)0x150040d8))/*cq0 related*/   ||
            (((uint32_t)curAddr>=(uint32_t)0x150040dc) && ((uint32_t)curAddr<=(uint32_t)0x150040e8))/*pass1 related*/ ||
            (((uint32_t)curAddr>=(uint32_t)0x150040f0) && ((uint32_t)curAddr<=(uint32_t)0x150040fc))/*pass1 related*/  ||
            (((uint32_t)curAddr>=(uint32_t)0x150044f0) && ((uint32_t)curAddr<=(uint32_t)0x15004894))/*pass1 engine*/ ||
            (((uint32_t)curAddr>=(uint32_t)0x15004dc0) && ((uint32_t)curAddr<=(uint32_t)0x15004e18))/*pass1 engine*/  ||
            (((uint32_t)curAddr>=(uint32_t)0x15006500) && ((uint32_t)curAddr<=(uint32_t)0x15006e28))/*pass1 engine*/ ||
            (curAddr==0x15007230) || (curAddr==0x15007440))

        {	//we do not get this
            curAddr=curAddr+0x4;
            continue;
        }
        *((uint32_t*)cqtablePA+(2*cnt)) = SET_CQ_APB_ADDR(curAddr-CAM_BASE) | SET_CQ_CMD_COUNT(0x00) | SET_CQ_COMMAND(CQ_APB);
        *((uint32_t*)cqtablePA+(2*cnt+1)) = (regvalueArray+i*sizeof(uint32_t));	//address of each register value
        curAddr=curAddr+0x4;
        cnt++;
        //printf("===test=== cqformat(0x%8x) (0x%8x)\n",cqdescriptorArray[2*i],cqdescriptorArray[2*i+1]);
    }
    //printf("===test===\n");
    *((uint32_t*)cqtablePA+(2*cnt))=SET_CQ_APB_ADDR(0x0000) | SET_CQ_CMD_COUNT(0x00) | SET_CQ_COMMAND(CQ_TRIGGER_IDLE);
    //printf("===test=== (0x%08x),value(0x%08x)\n",(MUINT32*)cqtablePA+(2*cnt),*((MUINT32*)cqtablePA+(2*cnt)));

}


void IspStreamCase6(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                *pStream;
    uint8_t                    *pSource;
    uint32_t                    pSourcePA;

    uint8_t                    *pOutput0;
    uint32_t                    pOutput0PA;

    uint32_t                    outSize[3];
    uint32_t                    outBufPa[3];
    void                       *outBuf0[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
    uint32_t                    TestCounter = 0;

    DpMemory* pISPBufMem = DpMemory::Factory(DP_MEMORY_ION, -1, 2*sizeof(isp_reg_setting_value_array), false);
    DpMemory* pISPBufMemVal = DpMemory::Factory(DP_MEMORY_ION, -1, sizeof(isp_reg_setting_value_array)+0x100, false);

    uint32_t* ISPRegBufVA = (uint32_t*)pISPBufMem->mapSWAddress();
    uint32_t ISPRegBufMVA = pISPBufMem->mapHWAddress(0,0);
    uint32_t* ISPRegBufValVA = (uint32_t*)pISPBufMemVal->mapSWAddress();
    uint32_t ISPRegBufValMVA = pISPBufMemVal->mapHWAddress(0,0);


    memset(&ISPInfo,0,sizeof(ISPInfo));


    pStream = new DpIspStream(DpIspStream::ISP_VR_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    DpMemory* pISPSourceMem = DpMemory::Factory(DP_MEMORY_ION, -1, (3264 * 2448 * 2), false);
    pSource = (uint8_t*) pISPSourceMem->mapSWAddress();
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    pSourcePA = pISPSourceMem->mapHWAddress(0,0);

    // Copy source image
    memcpy(pSource, g_imgi_array, sizeof(g_imgi_array));
    memcpy(ISPRegBufValVA,&isp_reg_setting_value_array[0],sizeof(isp_reg_setting_value_array));

    //Alocate destination
    DpMemory* pISPTargetMem = DpMemory::Factory(DP_MEMORY_ION, -1, (640 * 480 * 2), false);
    pOutput0 = (uint8_t*)pISPTargetMem->mapSWAddress();
    pOutput0PA = pISPTargetMem->mapHWAddress(13,0);

    //Alocate destination
    DpMemory* pTDRIMem = DpMemory::Factory(DP_MEMORY_ION, -1, (800*1024), false);
    ISPInfo.drvinfo.tpipeTableVa = (uint32_t*)pTDRIMem->mapSWAddress();
    ISPInfo.drvinfo.tpipeTablePa = pTDRIMem->mapHWAddress(0,0);


    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    while(TestCounter < 1)
    {

        status = pStream->queueSrcBuffer(pSource, pSourcePA, 3264 * 2448 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

        status = pStream->setSrcConfig(DP_COLOR_BAYER10, 3264, 2448, 4080, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

        DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

        outBuf0[0] = pOutput0;
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outBufPa[0] = pOutput0PA;
        outBufPa[1] = 0;
        outBufPa[2] = 0;

        outSize[0] = 640 * 480 * 2;
        outSize[1] = 0;
        outSize[2] = 0;

        status = pStream->queueDstBuffer(0,
                                         outBuf0,outBufPa,outSize,
                                         DP_COLOR_GET_PLANE_COUNT(DP_COLOR_YUYV));

        status = pStream->setDstConfig(0, DP_COLOR_YUYV, 640, 480, (640*2));
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

        //status = pStream->setPortType(0,2);

        status = pStream->setRotation(0,90);
#if 0        
        // tpipe_id = 0
        ISPInfo.sw.log_en = 1 ;
        ISPInfo.sw.src_width = 3264 ;
        ISPInfo.sw.src_height = 2448;
        ISPInfo.sw.tpipe_width = 768 ;
        ISPInfo.sw.tpipe_height = 2448;
        ISPInfo.sw.tpipe_irq_mode = 2;
        ISPInfo.top.scenario = 6 ;
        ISPInfo.top.mode = 0 ;
        ISPInfo.top.pixel_id = 0 ;
        ISPInfo.top.cam_in_fmt = 1;
        ISPInfo.top.ctl_extension_en = 0 ;
        ISPInfo.top.fg_mode = 0 ;
        ISPInfo.top.ufdi_fmt = 1 ;
        ISPInfo.top.vipi_fmt = 1 ;
        ISPInfo.top.img3o_fmt = 1 ;
        ISPInfo.top.imgi_en = 1 ;
        ISPInfo.top.ufdi_en = 0 ;
        ISPInfo.top.unp_en = 1 ;
        ISPInfo.top.ufd_en = 0 ;
        ISPInfo.top.bnr_en = 0 ;
        ISPInfo.top.lsci_en = 0 ;
        ISPInfo.top.lsc_en = 0 ;
        ISPInfo.top.sl2_en = 0 ;
        ISPInfo.top.cfa_en = 1 ;
        ISPInfo.top.c24_en = 0 ;
        ISPInfo.top.vipi_en = 0 ;
        ISPInfo.top.vip2i_en = 0 ;
        ISPInfo.top.vip3i_en = 0 ;
        ISPInfo.top.mfb_en = 0 ;
        ISPInfo.top.mfbo_en = 0 ;
        ISPInfo.top.g2c_en = 1 ;
        ISPInfo.top.c42_en = 1 ;
        ISPInfo.top.sl2b_en = 0 ;
        ISPInfo.top.nbc_en = 0 ;
        ISPInfo.top.mix1_en = 0 ;
        ISPInfo.top.mix2_en = 0 ;
        ISPInfo.top.pca_en = 0 ;
        ISPInfo.top.sl2c_en = 0 ;
        ISPInfo.top.seee_en = 0 ;
        ISPInfo.top.lcei_en = 0 ;
        ISPInfo.top.lce_en = 0 ;
        ISPInfo.top.mix3_en = 0 ;
        ISPInfo.top.crz_en = 0 ;
        ISPInfo.top.img2o_en = 0;
        ISPInfo.top.srz1_en = 0 ;
        ISPInfo.top.fe_en = 0 ;
        ISPInfo.top.feo_en = 0 ;
        ISPInfo.top.c02_en = 0 ;
        ISPInfo.top.nr3d_en = 0 ;
        ISPInfo.top.crsp_en = 0 ;
        ISPInfo.top.img3o_en = 0 ;
        ISPInfo.top.img3bo_en = 0 ;
        ISPInfo.top.img3co_en = 0 ;
        ISPInfo.top.c24b_en = 1 ;
        ISPInfo.top.mdp_crop_en = 1 ;
        ISPInfo.top.srz2_en = 0 ;
        ISPInfo.top.imgi_v_flip_en = 0 ;
        ISPInfo.top.lcei_v_flip_en = 0 ;
        ISPInfo.top.ufdi_v_flip_en = 0 ;
        ISPInfo.top.ufd_sel = 0 ;
        ISPInfo.top.ccl_sel = 0 ;
        ISPInfo.top.ccl_sel_en = 1 ;
        ISPInfo.top.g2g_sel = 1 ;
        ISPInfo.top.g2g_sel_en = 1 ;
        ISPInfo.top.c24_sel = 0 ;
        ISPInfo.top.srz1_sel = 1 ;
        ISPInfo.top.mix1_sel = 0 ;
        ISPInfo.top.crz_sel = 2 ;
        ISPInfo.top.nr3d_sel = 1 ;
        ISPInfo.top.fe_sel = 1 ;
        ISPInfo.top.mdp_sel = 0 ;
        ISPInfo.top.pca_sel = 0 ;
        ISPInfo.top.interlace_mode = 0 ;
        ISPInfo.imgi.imgi_stride = 4080 ;
        ISPInfo.ufdi.ufdi_stride = 3264 ;
        ISPInfo.bnr.bpc_en = 0 ;
        ISPInfo.bnr.bpc_tbl_en = 0 ;
        ISPInfo.lsci.lsci_stride = 640 ;
        ISPInfo.lsc.sdblk_xnum = 0 ;
        ISPInfo.lsc.sdblk_ynum = 0 ;
        ISPInfo.lsc.sdblk_width = 0 ;
        ISPInfo.lsc.sdblk_height = 0 ;
        ISPInfo.lsc.sdblk_last_width = 0 ;
        ISPInfo.lsc.sdblk_last_height = 0 ;
        ISPInfo.sl2.sl2_hrz_comp = 14111 ;
        ISPInfo.cfa.bayer_bypass = 0 ;
        ISPInfo.cfa.dm_fg_mode = 0 ;
        ISPInfo.vipi.vipi_xsize = -1 ;
        ISPInfo.vipi.vipi_ysize = -1 ;
        ISPInfo.vipi.vipi_stride = 0 ;
        ISPInfo.vip2i.vip2i_xsize = -1 ;
        ISPInfo.vip2i.vip2i_ysize = -1 ;
        ISPInfo.vip2i.vip2i_stride = 0 ;
        ISPInfo.vip3i.vip3i_xsize = -1 ;
        ISPInfo.vip3i.vip3i_ysize = -1 ;
        ISPInfo.vip3i.vip3i_stride = 0 ;
        ISPInfo.mfb.bld_mode = 0 ;
        ISPInfo.mfb.bld_deblock_en = 0 ;
        ISPInfo.mfb.bld_brz_en = 0 ;
        ISPInfo.mfbo.mfbo_stride = 0 ;
        ISPInfo.mfbo.mfbo_xoffset = 0 ;
        ISPInfo.mfbo.mfbo_yoffset = 0 ;
        ISPInfo.mfbo.mfbo_xsize = 0 ;
        ISPInfo.mfbo.mfbo_ysize = 0 ;
        ISPInfo.g2c.g2c_shade_en = 0 ;
        ISPInfo.sl2b.sl2b_hrz_comp = 6877;
        ISPInfo.nbc.anr_eny = 0 ;
        ISPInfo.nbc.anr_enc = 1 ;
        ISPInfo.nbc.anr_iir_mode = 0 ;
        ISPInfo.nbc.anr_scale_mode = 0;
        ISPInfo.sl2c.sl2c_hrz_comp = 15428 ;
        ISPInfo.seee.se_edge = 0 ;
        ISPInfo.lcei.lcei_stride =0 ;
        ISPInfo.lce.lce_slm_width = 0 ;
        ISPInfo.lce.lce_slm_height = 0 ;
        ISPInfo.lce.lce_bc_mag_kubnx = 0 ;
        ISPInfo.lce.lce_bc_mag_kubny = 0 ;
        ISPInfo.cdrz.cdrz_input_crop_width = 3264 ;
        ISPInfo.cdrz.cdrz_input_crop_height = 2448 ;
        ISPInfo.cdrz.cdrz_output_width = 3264 ;
        ISPInfo.cdrz.cdrz_output_height = 2448;
        ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0 ;
        ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0 ;
        ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0 ;
        ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0 ;
        ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
        ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0 ;
        ISPInfo.cdrz.cdrz_horizontal_coeff_step = 32768;
        ISPInfo.cdrz.cdrz_vertical_coeff_step = 32768 ;
        ISPInfo.img2o.img2o_stride = 6528;
        ISPInfo.img2o.img2o_xoffset = 0 ;
        ISPInfo.img2o.img2o_yoffset = 0 ;
        ISPInfo.img2o.img2o_xsize = 6527 ;
        ISPInfo.img2o.img2o_ysize = 2447 ;
        ISPInfo.srz1.srz_input_crop_width = 3264 ;
        ISPInfo.srz1.srz_input_crop_height = 2448 ;
        ISPInfo.srz1.srz_output_width = 3264 ;
        ISPInfo.srz1.srz_output_height = 2448 ;
        ISPInfo.srz1.srz_luma_horizontal_integer_offset = 0 ;
        ISPInfo.srz1.srz_luma_horizontal_subpixel_offset = 0 ;
        ISPInfo.srz1.srz_luma_vertical_integer_offset = 0 ;
        ISPInfo.srz1.srz_luma_vertical_subpixel_offset = 0 ;
        ISPInfo.srz1.srz_horizontal_coeff_step = 32768 ;
        ISPInfo.srz1.srz_vertical_coeff_step = 32768 ;
        ISPInfo.srz2.srz_input_crop_width = 271 ;
        ISPInfo.srz2.srz_input_crop_height = 5556;
        ISPInfo.srz2.srz_output_width = 271 ;
        ISPInfo.srz2.srz_output_height = 5556 ;
        ISPInfo.srz2.srz_luma_horizontal_integer_offset = 0 ;
        ISPInfo.srz2.srz_luma_horizontal_subpixel_offset = 0 ;
        ISPInfo.srz2.srz_luma_vertical_integer_offset = 0;
        ISPInfo.srz2.srz_luma_vertical_subpixel_offset = 0 ;
        ISPInfo.srz2.srz_horizontal_coeff_step = 32768;
        ISPInfo.srz2.srz_vertical_coeff_step = 32768;
        ISPInfo.fe.fe_mode = 0 ;
        ISPInfo.feo.feo_stride = 1 ;
        ISPInfo.nr3d.nr3d_on_en = 1 ;
        ISPInfo.nr3d.nr3d_on_xoffset = 54 ;
        ISPInfo.nr3d.nr3d_on_yoffset = 0 ;
        ISPInfo.nr3d.nr3d_on_width = 3210 ;
        ISPInfo.nr3d.nr3d_on_height = 2352 ;
        ISPInfo.crsp.crsp_ystep = 4 ;
        ISPInfo.crsp.crsp_xoffset = 0 ;
        ISPInfo.crsp.crsp_yoffset = 1 ;
        ISPInfo.img3o.img3o_stride = 3266 ;
        ISPInfo.img3o.img3o_xoffset = 0 ;
        ISPInfo.img3o.img3o_yoffset = 0      ;
        ISPInfo.img3o.img3o_xsize = 3263     ;
        ISPInfo.img3o.img3o_ysize = 2447     ;
        ISPInfo.img3bo.img3bo_stride = 3265  ;
        ISPInfo.img3bo.img3bo_xsize = 3263   ;
        ISPInfo.img3bo.img3bo_ysize = 1223   ;
        ISPInfo.img3co.img3co_stride = 10001 ;
        ISPInfo.img3co.img3co_xsize = 9999   ;
        ISPInfo.img3co.img3co_ysize = 5130   ;
#endif

        setting_cq_path_writesetting(ISPRegBufVA, ISPRegBufValMVA);

        ISPInfo.pass2.Pass2CmdqNum = TPIPE_PASS2_CMDQ_1;
        ISPInfo.drvinfo.p2Cq = ISP_TPIPE_P2_CQ1;
        ISPInfo.drvinfo.DesCqPa = ISPRegBufMVA;

        status = pStream->setParameter(ISPInfo);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

        status = pStream->startStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

        status = pStream->stopStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

        status = pStream->dequeueSrcBuffer();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");


        status = pStream->dequeueDstBuffer(0,&outBuf0[0]);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

        DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

        pStream->dequeueFrameEnd();

        TestCounter++;

    }
#if 1
    status = utilWriteBMP("/system/out/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_YUYV,
                          640,
                          480,
                          1280,0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");
#endif

}

DEFINE_TEST_CASE("IspStream6", IspStream6, IspStreamCase6);



/*
 * ISP stream Test #7: 1 in 3 out test case
 */
void IspStreamCase7(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                *pStream;
    uint8_t                    *pSource;
    uint32_t                    pSourcePA;

    uint8_t                    *pOutput0;
    uint32_t                    pOutput0PA;

    uint8_t                    *pOutput1;
    uint32_t                    pOutput1PA;

    uint32_t                    outSize[3];
    uint32_t                    outBufPa[3];
    void                       *outBuf0[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;
    uint32_t                    TestCounter = 0;

    DpMemory* pISPBufMem = DpMemory::Factory(DP_MEMORY_ION, -1, 2*sizeof(isp_reg_setting_value_array), false);
    DpMemory* pISPBufMemVal = DpMemory::Factory(DP_MEMORY_ION, -1, sizeof(isp_reg_setting_value_array)+0x100, false);

    uint32_t* ISPRegBufVA = (uint32_t*)pISPBufMem->mapSWAddress();
    uint32_t ISPRegBufMVA = pISPBufMem->mapHWAddress(0,0);
    uint32_t* ISPRegBufValVA = (uint32_t*)pISPBufMemVal->mapSWAddress();
    uint32_t ISPRegBufValMVA = pISPBufMemVal->mapHWAddress(0,0);


    memset(&ISPInfo,0,sizeof(ISPInfo));


    pStream = new DpIspStream(DpIspStream::ISP_VR_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    DpMemory* pISPSourceMem = DpMemory::Factory(DP_MEMORY_ION, -1, (3264 * 2448 * 2), false);
    pSource = (uint8_t*) pISPSourceMem->mapSWAddress();
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    pSourcePA = pISPSourceMem->mapHWAddress(0,0);

    // Copy source image
    memcpy(pSource, g_imgi_array, sizeof(g_imgi_array));
    memcpy(ISPRegBufValVA,&isp_reg_setting_value_array[0],sizeof(isp_reg_setting_value_array));

    //Alocate destination
    DpMemory* pISPTargetMem = DpMemory::Factory(DP_MEMORY_ION, -1, (2560 * 1440 * 2), false);
    pOutput0 = (uint8_t*)pISPTargetMem->mapSWAddress();
    pOutput0PA = pISPTargetMem->mapHWAddress(13,0);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    //Alocate destination
    DpMemory* pISPTargetMem1 = DpMemory::Factory(DP_MEMORY_ION, -1, (1920*1080*2), false);
    pOutput1 = (uint8_t*)pISPTargetMem1->mapSWAddress();
    pOutput1PA = pISPTargetMem1->mapHWAddress(13,0);


    //Alocate destination
    DpMemory* pTDRIMem = DpMemory::Factory(DP_MEMORY_ION, -1, (800*1024), false);
    ISPInfo.drvinfo.tpipeTableVa = (uint32_t*)pTDRIMem->mapSWAddress();
    ISPInfo.drvinfo.tpipeTablePa = pTDRIMem->mapHWAddress(0,0);



    while(TestCounter < 0xFFFFFFFF)
    {

        status = pStream->queueSrcBuffer(pSource, pSourcePA, 3264 * 2448 * 2);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

        status = pStream->setSrcConfig(DP_COLOR_BAYER10, 3264, 2448, 4080, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

        DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

        //Target 0
        outBuf0[0] = pOutput0;
        outBuf0[1] = 0;
        outBuf0[2] = 0;

        outBufPa[0] = pOutput0PA;
        outBufPa[1] = 0;
        outBufPa[2] = 0;

        outSize[0] = 2560 * 1440 * 2;
        outSize[1] = 0;
        outSize[2] = 0;

        status = pStream->queueDstBuffer(0, outBuf0,outBufPa,outSize,
                                         DP_COLOR_GET_PLANE_COUNT(DP_COLOR_YUYV));

        status = pStream->setDstConfig(0, DP_COLOR_YUYV, 1440, 2560, (1440*2),false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");


        status = pStream->setRotation(0,90);

        //Target 1
        outBuf0[0] = pOutput1;
        outBuf0[1] = pOutput1+1920*1080;
        outBuf0[2] = pOutput1+((1920*1080)>>2)*5;

        outBufPa[0] = pOutput1PA;
        outBufPa[1] = pOutput1PA+(1920*1080);
        outBufPa[2] = pOutput1PA+((1920*1080)>>2)*5;

        outSize[0] = 1920*1080;
        outSize[1] = (1920*1080 >> 2);
        outSize[2] = (1920*1080 >> 2);

        status = pStream->queueDstBuffer(1, outBuf0,outBufPa,outSize,
                                         DP_COLOR_GET_PLANE_COUNT(DP_COLOR_I420));

        status = pStream->setDstConfig(1, 1920, 1080, 1920, 960, DP_COLOR_I420, DP_PROFILE_BT601, eInterlace_None, NULL, false);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

#if 0
        // tpipe_id = 0
        ISPInfo.sw.log_en = 1 ;
        ISPInfo.sw.src_width = 3264 ;
        ISPInfo.sw.src_height = 2448;
        ISPInfo.sw.tpipe_width = 768 ;
        ISPInfo.sw.tpipe_height = 2448;
        ISPInfo.sw.tpipe_irq_mode = 2;
        ISPInfo.top.scenario = 6 ;
        ISPInfo.top.mode = 0 ;
        ISPInfo.top.pixel_id = 0 ;
        ISPInfo.top.cam_in_fmt = 1;
        ISPInfo.top.ctl_extension_en = 0 ;
        ISPInfo.top.fg_mode = 0 ;
        ISPInfo.top.ufdi_fmt = 1 ;
        ISPInfo.top.vipi_fmt = 1 ;
        ISPInfo.top.img3o_fmt = 1 ;
        ISPInfo.top.imgi_en = 1 ;
        ISPInfo.top.ufdi_en = 0 ;
        ISPInfo.top.unp_en = 1 ;
        ISPInfo.top.ufd_en = 0 ;
        ISPInfo.top.bnr_en = 0 ;
        ISPInfo.top.lsci_en = 0 ;
        ISPInfo.top.lsc_en = 0 ;
        ISPInfo.top.sl2_en = 0 ;
        ISPInfo.top.cfa_en = 1 ;
        ISPInfo.top.c24_en = 0 ;
        ISPInfo.top.vipi_en = 0 ;
        ISPInfo.top.vip2i_en = 0 ;
        ISPInfo.top.vip3i_en = 0 ;
        ISPInfo.top.mfb_en = 0 ;
        ISPInfo.top.mfbo_en = 0 ;
        ISPInfo.top.g2c_en = 1 ;
        ISPInfo.top.c42_en = 1 ;
        ISPInfo.top.sl2b_en = 0 ;
        ISPInfo.top.nbc_en = 0 ;
        ISPInfo.top.mix1_en = 0 ;
        ISPInfo.top.mix2_en = 0 ;
        ISPInfo.top.pca_en = 0 ;
        ISPInfo.top.sl2c_en = 0 ;
        ISPInfo.top.seee_en = 0 ;
        ISPInfo.top.lcei_en = 0 ;
        ISPInfo.top.lce_en = 0 ;
        ISPInfo.top.mix3_en = 0 ;
        ISPInfo.top.crz_en = 0 ;
        ISPInfo.top.img2o_en = 0;
        ISPInfo.top.srz1_en = 0 ;
        ISPInfo.top.fe_en = 0 ;
        ISPInfo.top.feo_en = 0 ;
        ISPInfo.top.c02_en = 0 ;
        ISPInfo.top.nr3d_en = 0 ;
        ISPInfo.top.crsp_en = 0 ;
        ISPInfo.top.img3o_en = 0 ;
        ISPInfo.top.img3bo_en = 0 ;
        ISPInfo.top.img3co_en = 0 ;
        ISPInfo.top.c24b_en = 1 ;
        ISPInfo.top.mdp_crop_en = 1 ;
        ISPInfo.top.srz2_en = 0 ;
        ISPInfo.top.imgi_v_flip_en = 0 ;
        ISPInfo.top.lcei_v_flip_en = 0 ;
        ISPInfo.top.ufdi_v_flip_en = 0 ;
        ISPInfo.top.ufd_sel = 0 ;
        ISPInfo.top.ccl_sel = 0 ;
        ISPInfo.top.ccl_sel_en = 1 ;
        ISPInfo.top.g2g_sel = 1 ;
        ISPInfo.top.g2g_sel_en = 1 ;
        ISPInfo.top.c24_sel = 0 ;
        ISPInfo.top.srz1_sel = 1 ;
        ISPInfo.top.mix1_sel = 0 ;
        ISPInfo.top.crz_sel = 2 ;
        ISPInfo.top.nr3d_sel = 1 ;
        ISPInfo.top.fe_sel = 1 ;
        ISPInfo.top.mdp_sel = 0 ;
        ISPInfo.top.pca_sel = 0 ;
        ISPInfo.top.interlace_mode = 0 ;
        ISPInfo.imgi.imgi_stride = 4080 ;
        ISPInfo.ufdi.ufdi_stride = 3264 ;
        ISPInfo.bnr.bpc_en = 0 ;
        ISPInfo.bnr.bpc_tbl_en = 0 ;
        ISPInfo.lsci.lsci_stride = 640 ;
        ISPInfo.lsc.sdblk_xnum = 0 ;
        ISPInfo.lsc.sdblk_ynum = 0 ;
        ISPInfo.lsc.sdblk_width = 0 ;
        ISPInfo.lsc.sdblk_height = 0 ;
        ISPInfo.lsc.sdblk_last_width = 0 ;
        ISPInfo.lsc.sdblk_last_height = 0 ;
        ISPInfo.sl2.sl2_hrz_comp = 14111 ;
        ISPInfo.cfa.bayer_bypass = 0 ;
        ISPInfo.cfa.dm_fg_mode = 0 ;
        ISPInfo.vipi.vipi_xsize = -1 ;
        ISPInfo.vipi.vipi_ysize = -1 ;
        ISPInfo.vipi.vipi_stride = 0 ;
        ISPInfo.vip2i.vip2i_xsize = -1 ;
        ISPInfo.vip2i.vip2i_ysize = -1 ;
        ISPInfo.vip2i.vip2i_stride = 0 ;
        ISPInfo.vip3i.vip3i_xsize = -1 ;
        ISPInfo.vip3i.vip3i_ysize = -1 ;
        ISPInfo.vip3i.vip3i_stride = 0 ;
        ISPInfo.mfb.bld_mode = 0 ;
        ISPInfo.mfb.bld_deblock_en = 0 ;
        ISPInfo.mfb.bld_brz_en = 0 ;
        ISPInfo.mfbo.mfbo_stride = 0 ;
        ISPInfo.mfbo.mfbo_xoffset = 0 ;
        ISPInfo.mfbo.mfbo_yoffset = 0 ;
        ISPInfo.mfbo.mfbo_xsize = 0 ;
        ISPInfo.mfbo.mfbo_ysize = 0 ;
        ISPInfo.g2c.g2c_shade_en = 0 ;
        ISPInfo.sl2b.sl2b_hrz_comp = 6877;
        ISPInfo.nbc.anr_eny = 0 ;
        ISPInfo.nbc.anr_enc = 1 ;
        ISPInfo.nbc.anr_iir_mode = 0 ;
        ISPInfo.nbc.anr_scale_mode = 0;
        ISPInfo.sl2c.sl2c_hrz_comp = 15428 ;
        ISPInfo.seee.se_edge = 0 ;
        ISPInfo.lcei.lcei_stride =0 ;
        ISPInfo.lce.lce_slm_width = 0 ;
        ISPInfo.lce.lce_slm_height = 0 ;
        ISPInfo.lce.lce_bc_mag_kubnx = 0 ;
        ISPInfo.lce.lce_bc_mag_kubny = 0 ;
        ISPInfo.cdrz.cdrz_input_crop_width = 3264 ;
        ISPInfo.cdrz.cdrz_input_crop_height = 2448 ;
        ISPInfo.cdrz.cdrz_output_width = 3264 ;
        ISPInfo.cdrz.cdrz_output_height = 2448;
        ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0 ;
        ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0 ;
        ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0 ;
        ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0 ;
        ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
        ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0 ;
        ISPInfo.cdrz.cdrz_horizontal_coeff_step = 32768;
        ISPInfo.cdrz.cdrz_vertical_coeff_step = 32768 ;
        ISPInfo.img2o.img2o_stride = 6528;
        ISPInfo.img2o.img2o_xoffset = 0 ;
        ISPInfo.img2o.img2o_yoffset = 0 ;
        ISPInfo.img2o.img2o_xsize = 6527 ;
        ISPInfo.img2o.img2o_ysize = 2447 ;
        ISPInfo.srz1.srz_input_crop_width = 3264 ;
        ISPInfo.srz1.srz_input_crop_height = 2448 ;
        ISPInfo.srz1.srz_output_width = 3264 ;
        ISPInfo.srz1.srz_output_height = 2448 ;
        ISPInfo.srz1.srz_luma_horizontal_integer_offset = 0 ;
        ISPInfo.srz1.srz_luma_horizontal_subpixel_offset = 0 ;
        ISPInfo.srz1.srz_luma_vertical_integer_offset = 0 ;
        ISPInfo.srz1.srz_luma_vertical_subpixel_offset = 0 ;
        ISPInfo.srz1.srz_horizontal_coeff_step = 32768 ;
        ISPInfo.srz1.srz_vertical_coeff_step = 32768 ;
        ISPInfo.srz2.srz_input_crop_width = 271 ;
        ISPInfo.srz2.srz_input_crop_height = 5556;
        ISPInfo.srz2.srz_output_width = 271 ;
        ISPInfo.srz2.srz_output_height = 5556 ;
        ISPInfo.srz2.srz_luma_horizontal_integer_offset = 0 ;
        ISPInfo.srz2.srz_luma_horizontal_subpixel_offset = 0 ;
        ISPInfo.srz2.srz_luma_vertical_integer_offset = 0;
        ISPInfo.srz2.srz_luma_vertical_subpixel_offset = 0 ;
        ISPInfo.srz2.srz_horizontal_coeff_step = 32768;
        ISPInfo.srz2.srz_vertical_coeff_step = 32768;
        ISPInfo.fe.fe_mode = 0 ;
        ISPInfo.feo.feo_stride = 1 ;
        ISPInfo.nr3d.nr3d_on_en = 1 ;
        ISPInfo.nr3d.nr3d_on_xoffset = 54 ;
        ISPInfo.nr3d.nr3d_on_yoffset = 0 ;
        ISPInfo.nr3d.nr3d_on_width = 3210 ;
        ISPInfo.nr3d.nr3d_on_height = 2352 ;
        ISPInfo.crsp.crsp_ystep = 4 ;
        ISPInfo.crsp.crsp_xoffset = 0 ;
        ISPInfo.crsp.crsp_yoffset = 1 ;
        ISPInfo.img3o.img3o_stride = 3266 ;
        ISPInfo.img3o.img3o_xoffset = 0 ;
        ISPInfo.img3o.img3o_yoffset = 0      ;
        ISPInfo.img3o.img3o_xsize = 3263     ;
        ISPInfo.img3o.img3o_ysize = 2447     ;
        ISPInfo.img3bo.img3bo_stride = 3265  ;
        ISPInfo.img3bo.img3bo_xsize = 3263   ;
        ISPInfo.img3bo.img3bo_ysize = 1223   ;
        ISPInfo.img3co.img3co_stride = 10001 ;
        ISPInfo.img3co.img3co_xsize = 9999   ;
        ISPInfo.img3co.img3co_ysize = 5130   ;
#endif

        setting_cq_path_writesetting(ISPRegBufVA, ISPRegBufValMVA);

        ISPInfo.pass2.Pass2CmdqNum = TPIPE_PASS2_CMDQ_1;
        ISPInfo.drvinfo.p2Cq = ISP_TPIPE_P2_CQ1;
        ISPInfo.drvinfo.DesCqPa = ISPRegBufMVA;

        status = pStream->setParameter(ISPInfo);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

        status = pStream->startStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

        status = pStream->stopStream();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

        status = pStream->dequeueSrcBuffer();
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");


        status = pStream->dequeueDstBuffer(0,&outBuf0[0]);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

        status = pStream->dequeueDstBuffer(1,&outBuf0[0]);
        REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

        DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

        pStream->dequeueFrameEnd();

        TestCounter++;

    }
#if 1
    status = utilWriteBMP("/system/out/output1.bmp",
        (uint8_t*)outBuf0[0],
        (uint8_t*)outBuf0[1],
        (uint8_t*)outBuf0[2],
        DP_COLOR_I420,
        1920,
        1080,
        1920,960);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");
#endif

}

DEFINE_TEST_CASE("IspStream7", IspStream7, IspStreamCase7);

#if 0
/*
 * ISP stream Test #8: 1 in 3 out test case
 */
void IspStreamCase8(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    void                        *inBuf1[3];
    uint8_t                     *pOutput0;
    uint8_t                     *pOutput1;
    uint8_t                     *pOutput2;
    int32_t                     bufID0;
    int32_t                     bufID1;
    int32_t                     bufID2;
    void                        *outBuf0[3];
    void                        *outBuf1[3];
    void                        *outBuf2[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = utilReadBMP("/system/pat/1296x972.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         DP_COLOR_UYVY,
                         1296,
                         972,
                         2592);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed\n");

    status = pStream->queueSrcBuffer(pSource,
                                        1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_UYVY,
                                   1296,
                                   972,
                                   2592);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    status = pStream->queueDstBuffer(0,
                                        pOutput0,
                                        640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   DP_COLOR_RGB565,
                                   480,
                                   640,
                                   960);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    status = pStream->setRotation(0,
                                  90);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure port0 rotation failed\n");

    pOutput1 = (uint8_t*)malloc(640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image image buffer1: %p\n", pOutput1);

    status = pStream->queueDstBuffer(1,
                                        pOutput1,
                                        640 * 480 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

    status = pStream->setDstConfig(1,
                                   DP_COLOR_RGB888,
                                   640,
                                   480,
                                   1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    pOutput2 = (uint8_t*)malloc(128 * 128 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput2 != NULL), "Can't allocate output image buffer3\n");

    DPLOGI("Malloc target image image buffer2: %p\n", pOutput2);

    status = pStream->queueDstBuffer(2,
                                        pOutput2,
                                        128 * 128 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer2 failed\n");

    status = pStream->setDstConfig(2,
                                   DP_COLOR_YUYV,
                                   128,
                                   128,
                                   256);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure output port3 failed\n");

    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 0;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 0;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 0;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 1;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 128;
    //ISPInfo.cdrz.cdrz_input_crop_height = 128;
    //ISPInfo.cdrz.cdrz_output_width = 128;
    //ISPInfo.cdrz.cdrz_output_height = 128;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    //ISPInfo.top.mdp_sel = 1; // MDP path sel

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer(&bufID0,
                                       inBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->queueSrcBuffer(bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &bufID0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &bufID1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->dequeueDstBuffer(2,
                                       &bufID2,
                                       &outBuf2[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer2 failed\n");

    DPLOGI("Port2 target buffer: 0x%08x", outBuf2[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_RGB565,
                          480,
                          640,
                          960);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");


    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_RGB888,
                          640,
                          480,
                          1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");

    status = utilWriteBMP("/data/output3.bmp",
                          (uint8_t*)outBuf2[0],
                          (uint8_t*)outBuf2[1],
                          (uint8_t*)outBuf2[2],
                          DP_COLOR_YUYV,
                          128,
                          128,
                          256);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer2 failed\n");

    status = pStream->queueDstBuffer(0,
                                     bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer0 failed\n");

    status = pStream->queueDstBuffer(1,
                                     bufID1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer1 failed\n");

    status = pStream->queueDstBuffer(2,
                                     bufID2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer2 failed\n");

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pOutput1)
    {
        delete pOutput1;
        pOutput1 = NULL;
    }

    if (NULL != pOutput2)
    {
        delete pOutput2;
        pOutput2 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream8", IspStream8, IspStreamCase8);



/*
 * ISP stream Test #8: 1 in 1 out test case
 */
void IspStreamCase9(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    void                        *inBuf1[3];
    uint8_t                     *pOutput0;
    int32_t                     bufID0;
    void                        *outBuf0[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(1274 * 948 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");


    status = utilReadBMP("/system/pat/1274x948.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         DP_COLOR_UYVY,
                         1274,
                         948,
                         2548);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed\n");

    status = pStream->queueSrcBuffer(pSource,
                                        1274 * 948 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_UYVY,
                                   1274,
                                   948,
                                   2548);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(1274 * 948 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    status = pStream->queueDstBuffer(0,
                                        pOutput0,
                                        1274 * 948 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   DP_COLOR_YUYV,
                                   1274,
                                   948,
                                   2548);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 0;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 0;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 0;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 1;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 128;
    //ISPInfo.cdrz.cdrz_input_crop_height = 128;
    //ISPInfo.cdrz.cdrz_output_width = 128;
    //ISPInfo.cdrz.cdrz_output_height = 128;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    //ISPInfo.top.mdp_sel = 1; // MDP path sel

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer(&bufID0,
                                       inBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->queueSrcBuffer(bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &bufID0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");


    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_YUYV,
                          1274,
                          948,
                          2548);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");

    status = pStream->queueDstBuffer(0,
                                     bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer0 failed\n");

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream9", IspStream9, IspStreamCase9);



/*
 * ISP stream Test #10: 1 in 2 out test case
 */
void IspStreamCase10(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    void                        *inBuf1[3];
    uint8_t                     *pOutput0;
    uint8_t                     *pOutput1;
    int32_t                     bufID0;
    int32_t                     bufID1;
    void                        *outBuf0[3];
    void                        *outBuf1[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(64 * 48 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");


    status = utilReadBMP("/system/pat/64x48.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         DP_COLOR_UYVY,
                         64,
                         48,
                         128);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed\n");


    utilWriteBin("/data/64x48.bin",
                 (uint8_t*)pSource,
                 (uint8_t*)NULL,
                 (uint8_t*)NULL,
                  DP_COLOR_UYVY,
                  64,
                  48,
                  128);

    status = pStream->queueSrcBuffer(pSource,
                                        64 * 48 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_UYVY,
                                   64,
                                   48,
                                   128);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(64 * 48 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    status = pStream->queueDstBuffer(0,
                                        pOutput0,
                                        64 * 48 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   DP_COLOR_RGB565,
                                   64,
                                   48,
                                   128);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    pOutput1 = (uint8_t*)malloc(64 * 48 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image image buffer1: %p\n", pOutput1);

    status = pStream->queueDstBuffer(1,
                                        pOutput1,
                                        64 * 48 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(1,
                                   DP_COLOR_YUYV,
                                   64,
                                   48,
                                   128);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 0;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 0;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 0;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 1;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 128;
    //ISPInfo.cdrz.cdrz_input_crop_height = 128;
    //ISPInfo.cdrz.cdrz_output_width = 128;
    //ISPInfo.cdrz.cdrz_output_height = 128;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    //ISPInfo.top.mdp_sel = 1; // MDP path sel

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer(&bufID0,
                                       inBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->queueSrcBuffer(bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &bufID0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &bufID1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_RGB565,
                          64,
                          48,
                          128);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");


    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_YUYV,
                          64,
                          48,
                          128);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");

    status = pStream->queueDstBuffer(0,
                                     bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer0 failed\n");

    status = pStream->queueDstBuffer(1,
                                     bufID1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer1 failed\n");

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pOutput1)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream10", IspStream10, IspStreamCase10);


/*
 * ISP stream Test #8: 1 in 2 out test case
 */
void IspStreamCase11(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    uint8_t                     *pOutput0;
    uint8_t                     *pOutput1;
    void                        *outBuf0[3];
    void                        *outBuf1[3];
    uint32_t                    outSize[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = utilReadBMP("/system/pat/1296x972.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         DP_COLOR_UYVY,
                         1296,
                         972,
                         1296 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed\n");

    status = pStream->queueSrcBuffer(pSource,
                                     1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_UYVY,
                                   1296,
                                   972,
                                   1296 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    outBuf0[0] = pOutput0;
    outBuf0[1] = 0;
    outBuf0[2] = 0;

    outSize[0] = 640 * 480 * 2;
    outSize[1] = 0;
    outSize[2] = 0;

    status = pStream->queueDstBuffer(0,
                                     &outBuf0[0],
                                     &outSize[0],
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   DP_COLOR_RGB565,
                                   640,
                                   480,
                                   640 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    pOutput1 = (uint8_t*)malloc(240 * 320 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image buffer1: %p\n", pOutput1);

    outBuf1[0] = pOutput1;
    outBuf1[1] = 0;
    outBuf1[2] = 0;

    outSize[0] = 240 * 320 * 2;
    outSize[1] = 0;
    outSize[2] = 0;

    status = pStream->queueDstBuffer(1,
                                     &outBuf1[0],
                                     &outSize[0],
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(1,
                                   DP_COLOR_YUYV,
                                   240,
                                   320,
                                   240 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");
#if 0
    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 0;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 0;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 0;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 1;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 128;
    //ISPInfo.cdrz.cdrz_input_crop_height = 128;
    //ISPInfo.cdrz.cdrz_output_width = 128;
    //ISPInfo.cdrz.cdrz_output_height = 128;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    //ISPInfo.top.mdp_sel = 1; // MDP path sel
#endif
    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

#if 0
    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_RGB565,
                          640,
                          480,
                          640 * 2);
    //REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");

    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_YUYV,
                          240,
                          320,
                          240 * 2);
    //REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");
#endif // 0

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pOutput1)
    {
        delete pOutput1;
        pOutput1 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream11", IspStream11, IspStreamCase11);

/*
 * ISP stream Test #8: 1 in 2 out test case
 */
void IspStreamCase12(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    void                        *inBuf1[3];
    uint8_t                     *pOutput0;
    uint8_t                     *pOutput1;
    int32_t                     bufID0;
    int32_t                     bufID1;
    void                        *outBuf0[3];
    void                        *outBuf1[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = utilReadBMP("/system/pat/1296x972.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         DP_COLOR_UYVY,
                         1296,
                         972,
                         1296 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed\n");

    status = pStream->queueSrcBuffer(pSource,
                                        1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_UYVY,
                                   1296,
                                   972,
                                   1296 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    status = pStream->queueDstBuffer(0,
                                        pOutput0,
                                        640 * 480 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   DP_COLOR_RGB565,
                                   640,
                                   480,
                                   640 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    pOutput1 = (uint8_t*)malloc(240 * 320 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image buffer1: %p\n", pOutput1);

    status = pStream->queueDstBuffer(1,
                                        pOutput1,
                                        240 * 320 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(1,
                                   DP_COLOR_YUYV,
                                   240,
                                   320,
                                   240 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 0;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 0;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 0;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 1;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 128;
    //ISPInfo.cdrz.cdrz_input_crop_height = 128;
    //ISPInfo.cdrz.cdrz_output_width = 128;
    //ISPInfo.cdrz.cdrz_output_height = 128;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    //ISPInfo.top.mdp_sel = 1; // MDP path sel

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer(&bufID0,
                                       inBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->queueSrcBuffer(bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &bufID0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &bufID1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_RGB565,
                          640,
                          480,
                          640 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");

    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_YUYV,
                          240,
                          320,
                          240 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");

    status = pStream->queueDstBuffer(0,
                                     bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer0 failed\n");

    status = pStream->queueDstBuffer(1,
                                     bufID1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer1 failed\n");

    status = pStream->startStream();

    status = pStream->dequeueSrcBuffer(&bufID0,
                                       inBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->queueSrcBuffer(bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &bufID0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &bufID1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_RGB565,
                          640,
                          480,
                          640 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");

    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_YUYV,
                          240,
                          320,
                          240 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");

    status = pStream->queueDstBuffer(0,
                                     bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer0 failed\n");

    status = pStream->queueDstBuffer(1,
                                     bufID1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer1 failed\n");

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pOutput1)
    {
        delete pOutput1;
        pOutput1 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream12", IspStream12, IspStreamCase12);



/*
 * ISP stream Test #8: 1 in 2 out test case
 */
void IspStreamCase13(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    void                        *inBuf1[3];
    uint8_t                     *pYOut0;
    uint8_t                     *pUVOut0;
    uint8_t                     *pYOut1;
    uint8_t                     *pUVOut1;
    int32_t                     bufID0;
    int32_t                     bufID1;
    void                        *outBuf0[3];
    uint32_t                    outSize0[3];
    void                        *outBuf1[3];
    uint32_t                    outSize1[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = utilReadBMP("/system/pat/1296x972.bmp",
                         (uint8_t*)pSource,
                         (uint8_t*)NULL,
                         (uint8_t*)NULL,
                         DP_COLOR_UYVY,
                         1296,
                         972,
                         1296 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Read source image failed\n");

    status = pStream->queueSrcBuffer(pSource,
                                        1296 * 972 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_UYVY,
                                   1296,
                                   972,
                                   1296 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    DPLOGI("Allocate Y target buffer\n");
    pYOut0 = (uint8_t*)malloc(sizeof(uint8_t) * 320 * 240);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOut0 != NULL), "Allocate Y target buffer failed");

    DPLOGI("Allocate UV tearget buffer\n");
    pUVOut0 = (uint8_t*)malloc(sizeof(uint8_t) * (320 * 240 >> 1));
    REPORTER_ASSERT_MESSAGE(pReporter, (pUVOut0 != NULL), "Allocate U target buffer failed");

    outBuf0[0] = pYOut0;
    outBuf0[1] = pUVOut0;
    outBuf0[2] = 0;

    outSize0[0] = 320 * 240 ;
    outSize0[1] = (320 * 240 >> 1);
    outSize0[2] = 0;

    status = pStream->queueDstBuffer(0,
                                        &outBuf0[0],
                                        &outSize0[0],
                                        2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   eNV12,
                                   320,
                                   240,
                                   320);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    DPLOGI("Allocate Y target buffer\n");
    pYOut1 = (uint8_t*)malloc(sizeof(uint8_t) * 240 * 320);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOut1 != NULL), "Allocate Y target buffer failed");

    DPLOGI("Allocate UV tearget buffer\n");
    pUVOut1 = (uint8_t*)malloc(sizeof(uint8_t) * (240 * 320 >> 1));
    REPORTER_ASSERT_MESSAGE(pReporter, (pUVOut1 != NULL), "Allocate U target buffer failed");

    outBuf1[0] = pYOut1;
    outBuf1[1] = pUVOut1;
    outBuf1[2] = 0;

    outSize1[0] = 240 * 320;
    outSize1[1] = (240 * 320 >> 1);
    outSize1[2] = 0;

    status = pStream->queueDstBuffer(1,
                                        &outBuf1[0],
                                        &outSize1[0],
                                        2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(1,
                                   eNV21,
                                   240,
                                   320,
                                   240);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port0 failed\n");

    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 0;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 0;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 0;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 1;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 128;
    //ISPInfo.cdrz.cdrz_input_crop_height = 128;
    //ISPInfo.cdrz.cdrz_output_width = 128;
    //ISPInfo.cdrz.cdrz_output_height = 128;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    //ISPInfo.top.mdp_sel = 1; // MDP path sel

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer(&bufID0,
                                       inBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->queueSrcBuffer(bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &bufID0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &bufID1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          eNV12,
                          320,
                          240,
                          320);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");

    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          eNV21,
                          240,
                          320,
                          240);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");

    status = pStream->queueDstBuffer(0,
                                     bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer0 failed\n");

    status = pStream->queueDstBuffer(1,
                                     bufID1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer1 failed\n");

    if (NULL != pUVOut0)
    {
        delete pUVOut0;
        pUVOut0 = NULL;
    }

    if (NULL != pYOut0)
    {
        delete pYOut0;
        pYOut0 = NULL;
    }

    if (NULL != pUVOut1)
    {
        delete pUVOut1;
        pUVOut1 = NULL;
    }

    if (NULL != pYOut1)
    {
        delete pYOut1;
        pYOut1 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream13", IspStream13, IspStreamCase13);


/*
 * ISP stream Test #0: 1 in 2 out test case
 */
void IspStreamCase14(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    void                        *inBuf1[3];
    uint8_t                     *pOutput0;
    uint8_t                     *pOutput1;
    int32_t                     bufID0;
    int32_t                     bufID1;
    void                        *outBuf0[3];
    void                        *outBuf1[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(3264 * 2448 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    // Copy source image
    memcpy(pSource, bayer12_3264x2448, sizeof(bayer12_3264x2448));

    status = pStream->queueSrcBuffer(pSource,
                                        3264 * 2448 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_BAYER12,
                                   3264,
                                   2448,
                                   6528);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput0 = (uint8_t*)malloc(1920 * 1080 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput0 != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pOutput0);

    status = pStream->queueDstBuffer(0,
                                        pOutput0,
                                        1920 * 1080 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   DP_COLOR_YUYV,
                                   1920,
                                   1080,
                                   1920 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    pOutput1 = (uint8_t*)malloc(1920 * 1080 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput1 != NULL), "Can't allocate target image buffer2\n");
    DPLOGI("Malloc target image image buffer1: %p\n", pOutput1);

    status = pStream->queueDstBuffer(1,
                                        pOutput1,
                                        1920 * 1080 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

    status = pStream->setDstConfig(1,
                                   DP_COLOR_RGB888,
                                   1920,
                                   1080,
                                   1920 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port2 failed\n");

    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 3;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 1;

    // BNR settings
    ISPInfo.top.bnr_en = 1;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 1;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 1;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 1;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 1;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 1;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_input_crop_height = 0x990;
    //ISPInfo.cdrz.cdrz_output_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_output_height = 0x990;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 1;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 1;
    ISPInfo.img2o.img2o_mux = 1;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    ISPInfo.top.mdp_sel = 0; // MDP path sel

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer(&bufID0,
                                       inBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->queueSrcBuffer(bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &bufID0,
                                       outBuf0,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &bufID1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_YUYV,
                          1920,
                          1080,
                          1920 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");


    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_RGB888,
                          1920,
                          1080,
                          1920 * 3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");

    status = pStream->queueDstBuffer(0,
                                     bufID0);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer0 failed\n");

    status = pStream->queueDstBuffer(1,
                                     bufID1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Queue target image buffer1 failed\n");

    if (NULL != pOutput0)
    {
        delete pOutput0;
        pOutput0 = NULL;
    }

    if (NULL != pOutput1)
    {
        delete pOutput1;
        pOutput1 = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream14", IspStream14, IspStreamCase14);
#endif // 0

#if 0

/*
 * ISP stream Test #0: 1 in 2 out test case
 */
void IspStreamCase15(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    uint8_t                     *pYOutput;
    uint8_t                     *pUOutput;
    uint8_t                     *pVOutput;
    void                        *outBuf0[3];
    uint32_t                    MVABuf[3];
    void                        *outBuf1[3];
    uint32_t                    outSize[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(3200 * 2400 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = pStream->queueSrcBuffer(pSource,
                                     3200 * 2400 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_BAYER10,
                                   3200,
                                   2400,
                                   6400);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pYOutput = (uint8_t*)malloc(3600 * 2160 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate target image buffer1\n");

    DPLOGI("Malloc target image buffer0: %p\n", pYOutput);

    outBuf1[0] = pYOutput;
    outBuf1[1] = 0;
    outBuf1[2] = 0;

    outSize[0] = 3600 * 2160 * 2;
    outSize[1] = 0;
    outSize[2] = 0;

    status = pStream->queueDstBuffer(0,
                                     &outBuf1[0],
                                     &outSize[0],
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   DP_COLOR_YUYV,
                                   3600,
                                   2160,
                                   3600 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    pYOutput = (uint8_t*)malloc(840 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image image buffer1: %p\n", pYOutput);

    pUOutput = (uint8_t*)malloc((840 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUOutput != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image image buffer1: %p\n", pUOutput);

    pVOutput = (uint8_t*)malloc((840 * 480) >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOutput != NULL), "Can't allocate target image buffer2\n");

    DPLOGI("Malloc target image image buffer1: %p\n", pVOutput);

    outBuf1[0] = pYOutput;
    outBuf1[1] = pUOutput;
    outBuf1[2] = pVOutput;

    MVABuf[0] = 0x01000000;
    MVABuf[1] = 0x02000000;
    MVABuf[2] = 0x03000000;

    outSize[0] = 800 * 480;
    outSize[1] = (800 * 480) >> 2;
    outSize[2] = (800 * 480) >> 2;
    status = pStream->queueDstBuffer(1,
                                     &outBuf1[0],
                                     &MVABuf[0],
                                     &outSize[0],
                                     3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

    status = pStream->setDstConfig(1,
                                   DP_COLOR_YV12,
                                   840,
                                   480,
                                   840);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port2 failed\n");

#if 0
    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 3;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 1;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 1;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 0;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_input_crop_height = 0x990;
    //ISPInfo.cdrz.cdrz_output_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_output_height = 0x990;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 1;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 1;
    ISPInfo.img2o.img2o_mux = 1;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    ISPInfo.top.mdp_sel = 0; // MDP path sel

#endif
    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    pStream->setSrcCrop(0, 0, 240, 0, 3200, 1920);

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       outBuf0,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       outBuf1,
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    if (NULL != outBuf0[0])
    {
        free(outBuf0[0]);
        outBuf0[0] = NULL;
    }

    if (NULL != outBuf0[1])
    {
        free(outBuf0[1]);
        outBuf0[1] = NULL;
    }

    if (NULL != outBuf0[2])
    {
        free(outBuf0[2]);
        outBuf0[2] = NULL;
    }

    if (NULL != outBuf1[0])
    {
        free(outBuf1[0]);
        outBuf1[0] = NULL;
    }

    if (NULL != outBuf1[1])
    {
        free(outBuf1[1]);
        outBuf1[1] = NULL;
    }

    if (NULL != outBuf1[2])
    {
        free(outBuf1[2]);
        outBuf1[2] = NULL;
    }

    if (NULL != pSource)
    {
        free(pSource);
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream15", IspStream15, IspStreamCase15);


/*
 * ISP stream Test #16: 1 in 2 out test case
 */

void IspStreamCase16(TestReporter *pReporter)
{
    DP_STATUS_ENUM              status;
    DpIspStream                 *pStream;
    uint8_t                     *pSource;
    uint8_t                     *pYOutput;
    uint8_t                     *pUOutput;
    uint8_t                     *pVOutput;
    void                        *outBuf0[3];
    void                        *outBuf1[3];
    uint32_t                    outSize[3];
    ISP_TPIPE_CONFIG_STRUCT     ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(4000 * 1800 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = pStream->queueSrcBuffer(pSource,
                                     4000 * 1800 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_BAYER10,
                                   3200,
                                   1800,
                                   4000);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pYOutput = (uint8_t*)malloc(1280 * 720);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate Y target image buffer1\n");

    pUOutput = (uint8_t*)malloc(1280 * 720 >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUOutput != NULL), "Can't allocate U target image buffer1\n");

    pVOutput = (uint8_t*)malloc(1280 * 720 >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOutput != NULL), "Can't allocate V target image buffer1\n");

    outBuf0[0] = pYOutput;
    outBuf0[1] = pUOutput;
    outBuf0[2] = pVOutput;

    outSize[0] = 1280 * 720;
    outSize[1] = 1280 * 720 >> 2;
    outSize[2] = 1280 * 720 >> 2;
    status = pStream->queueDstBuffer(0,
                                     outBuf0,
                                     outSize,
                                     3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   1280,
                                   720,
                                   1280,
                                   640,
                                   DP_COLOR_YV12);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    pYOutput = (uint8_t*)malloc(1280 * 720);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate Y target image buffer1\n");

    pUOutput = (uint8_t*)malloc(1280 * 720 >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUOutput != NULL), "Can't allocate U target image buffer1\n");

    pVOutput = (uint8_t*)malloc(1280 * 720 >> 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOutput != NULL), "Can't allocate V target image buffer1\n");

    outBuf1[0] = pYOutput;
    outBuf1[1] = pUOutput;
    outBuf1[2] = pVOutput;

    outSize[0] = 1280 * 720;
    outSize[1] = 1280 * 720 >> 2;
    outSize[2] = 1280 * 720 >> 2;

    status = pStream->queueDstBuffer(1,
                                     outBuf1,
                                     outSize,
                                     3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer1 failed\n");

    status = pStream->setDstConfig(1,
                                   1280,
                                   720,
                                   1280,
                                   640,
                                   DP_COLOR_YV12);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port2 failed\n");

    status = pStream->setSrcCrop(456,
                                 0,
                                 6,
                                 0,
                                 2666,
                                 1500);
#if 0
    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 3;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 1;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 1;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 0;

    // C42 settings
    ISPInfo.top.c42_en = 1;

    // NBC settings
    ISPInfo.top.nbc_en = 1;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 1;

    // SEEE settings
    ISPInfo.top.seee_en = 1;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_input_crop_height = 0x990;
    //ISPInfo.cdrz.cdrz_output_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_output_height = 0x990;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 1;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 1;
    ISPInfo.img2o.img2o_mux = 1;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    ISPInfo.top.mdp_sel = 0; // MDP path sel
#endif

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &outBuf0[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", outBuf0[0]);

    status = pStream->dequeueDstBuffer(1,
                                       &outBuf1[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer1 failed\n");

    DPLOGI("Port1 target buffer: 0x%08x\n", outBuf1[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

#if 0
    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_YUYV,
                          3264,
                          2448,
                          6528);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");


    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_RGB888,
                          640,
                          480,
                          1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");
#endif // 0

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream16", IspStream16, IspStreamCase16);


/*
 * ISP stream Test #16: 1 in 2 out test case
 */
void IspStreamCase17(TestReporter *pReporter)
{
    DP_STATUS_ENUM           status;
    DpIspStream              *pStream;
    uint8_t                  *pSource;
    uint8_t                  *pOutput;
    void                     *pOutBuf[3];
     uint32_t                outSize[3];
    ISP_TPIPE_CONFIG_STRUCT  ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(58 * 34);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = pStream->queueSrcBuffer(pSource,
                                     58 * 34 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_GREY,
                                   58,
                                   34,
                                   58 * 2);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pOutput = (uint8_t*)malloc(1794 * 1078);
    REPORTER_ASSERT_MESSAGE(pReporter, (pOutput != NULL), "Can't allocate Y target image buffer1\n");

    pOutBuf[0] = pOutput;
    pOutBuf[1] = 0;
    pOutBuf[2] = 0;

    outSize[0] = 1794 * 1078;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(0,
                                     pOutBuf,
                                     outSize,
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   1794,
                                   1078,
                                   1794,
                                   0,
                                   DP_COLOR_GREY);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    status = pStream->setSrcCrop(0,
                                 0,
                                 0,
                                 0,
                                 58,
                                 34);
#if 0
    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 3;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 0;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 0;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 0;

    // C42 settings
    ISPInfo.top.c42_en = 0;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 0;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_input_crop_height = 0x990;
    //ISPInfo.cdrz.cdrz_output_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_output_height = 0x990;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    ISPInfo.top.mdp_sel = 0; // MDP path sel
#endif

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &pOutBuf[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", pOutBuf[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

#if 0
    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_YUYV,
                          3264,
                          2448,
                          6528);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");


    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_RGB888,
                          640,
                          480,
                          1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");
#endif // 0

    if (NULL != pOutput)
    {
        delete pOutput;
        pOutput = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream17", IspStream17, IspStreamCase17);



/*
 * ISP stream Test #18: 1 in 2 out test case
 */
void IspStreamCase18(TestReporter *pReporter)
{
    DP_STATUS_ENUM           status;
    DpIspStream              *pStream;
    uint8_t                  *pSource;
    uint8_t                  *pYOutput;
    uint8_t                  *pUOutput;
    uint8_t                  *pVOutput;
    void                     *pOutBuf[3];
     uint32_t                outSize[3];
    ISP_TPIPE_CONFIG_STRUCT  ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(2560 * 1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = pStream->queueSrcBuffer(pSource,
                                     3200 * 1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_BAYER10,
                                   2560,
                                   1920,
                                   3200);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pYOutput = (uint8_t*)malloc(800 * 600);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate Y target image buffer1\n");

    pUOutput = (uint8_t*)malloc(400 * 300);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUOutput != NULL), "Can't allocate Y target image buffer1\n");

    pVOutput = (uint8_t*)malloc(400 * 300);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOutput != NULL), "Can't allocate Y target image buffer1\n");

    pOutBuf[0] = pYOutput;
    pOutBuf[1] = pUOutput;
    pOutBuf[2] = pVOutput;

    outSize[0] = 800 * 600;
    outSize[1] = 400 * 300;
    outSize[2] = 400 * 300;
    status = pStream->queueDstBuffer(0,
                                     pOutBuf,
                                     outSize,
                                     3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   800,
                                   600,
                                   800,
                                   400,
                                   DP_COLOR_YV12);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    pYOutput = (uint8_t*)malloc(1920 * 2560);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate Y target image buffer1\n");

    pOutBuf[0] = pYOutput;
    pOutBuf[1] = 0;
    pOutBuf[2] = 0;

    outSize[0] = 1920 * 2560 * 2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(1,
                                     pOutBuf,
                                     outSize,
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(1,
                                   1920,
                                   2560,
                                   3840,
                                   0,
                                   DP_COLOR_YUYV);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    status = pStream->setRotation(1, 90);

    status = pStream->setSrcCrop(0,
                                 0,
                                 0,
                                 0,
                                 2560,
                                 1920);
#if 0
    /* Got each function's frame information here */
    ISPInfo.top.pixel_id   = 3;
    ISPInfo.top.cam_in_fmt = 2;
    ISPInfo.top.mode       = 0;
    ISPInfo.top.scenario   = 6;
    ISPInfo.top.debug_sel  = 0;

    // IMGI settings
    //ISPInfo.top.imgi_en      = 1;
    //ISPInfo.imgi.imgi_stride = 6528;

    // UNP settings
    ISPInfo.top.unp_en = 1;

    // BNR settings
    ISPInfo.top.bnr_en = 0;
    ISPInfo.bnr.bpc_en = 0;

    // LSC settings
    ISPInfo.top.lsc_en = 0;
    ISPInfo.lsc.sdblk_xnum = 0xa;
    ISPInfo.lsc.sdblk_ynum = 0xc;
    ISPInfo.lsc.sdblk_width = 0x5;
    ISPInfo.lsc.sdblk_height = 0x4;
    ISPInfo.lsc.sdblk_last_width = 0xe;
    ISPInfo.lsc.sdblk_last_height = 0x10;

    // SL2 settings
    ISPInfo.top.sl2_en = 0;
    ISPInfo.sl2.sl2_hrz_comp = 0x3a3f;

    // LSCI settings
    ISPInfo.top.lsci_en = 0;
    ISPInfo.lsci.lsci_stride = 0x2C0;

    // CFA settings
    ISPInfo.top.cfa_en = 1;
    ISPInfo.cfa.bayer_bypass = 0;

    // C24 settings
    ISPInfo.top.c24_en = 0;

    // C42 settings
    ISPInfo.top.c42_en = 0;

    // NBC settings
    ISPInfo.top.nbc_en = 0;
    ISPInfo.nbc.anr_eny = 0;
    ISPInfo.nbc.anr_enc = 0;
    ISPInfo.nbc.anr_iir_mode = 0;
    ISPInfo.nbc.anr_scale_mode = 0;

    // SEEE settings
    ISPInfo.top.seee_en = 0;
    ISPInfo.seee.se_edge = 0x2;

    // CDRZ settings
    ISPInfo.top.cdrz_en = 0;
    //ISPInfo.cdrz.cdrz_input_crop_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_input_crop_height = 0x990;
    //ISPInfo.cdrz.cdrz_output_width = 0xCC0;
    //ISPInfo.cdrz.cdrz_output_height = 0x990;
    //ISPInfo.cdrz.cdrz_luma_horizontal_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_horizontal_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_integer_offset = 0;
    //ISPInfo.cdrz.cdrz_luma_vertical_subpixel_offset = 0;
    //ISPInfo.cdrz.cdrz_horizontal_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_vertical_luma_algorithm = 0;
    //ISPInfo.cdrz.cdrz_horizontal_coeff_step = 0;
    //ISPInfo.cdrz.cdrz_vertical_coeff_step = 0;

    // IMGO settings
    //ISPInfo.top.imgo_en = 0;
    //ISPInfo.imgo.imgo_stride = 6528;
    //ISPInfo.imgo.imgo_crop_en = 0;
    //ISPInfo.imgo.imgo_xoffset = 0;
    //ISPInfo.imgo.imgo_yoffset = 0;
    //ISPInfo.imgo.imgo_xsize = 6527;
    //ISPInfo.imgo.imgo_ysize = 2447;
    ISPInfo.imgo.imgo_mux_en = 0;
    ISPInfo.imgo.imgo_mux = 0;

    // IMG2O settings
    //ISPInfo.top.img2o_en = 1;
    //ISPInfo.img2o.img2o_stride = 6528;
    //ISPInfo.img2o.img2o_crop_en = 0;
    //ISPInfo.img2o.img2o_xoffset = 0;
    //ISPInfo.img2o.img2o_yoffset = 0;
    //ISPInfo.img2o.img2o_xsize = 6527;
    //ISPInfo.img2o.img2o_ysize = 2447;
    ISPInfo.img2o.img2o_mux_en = 0;
    ISPInfo.img2o.img2o_mux = 0;

    // RSP settings
    //ISPInfo.top.rsp_en = 1;

    // MDP_CROP settings
    //ISPInfo.top.mdp_crop_en = 1;

    ISPInfo.top.mdp_sel = 0; // MDP path sel
#endif
    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &pOutBuf[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", pOutBuf[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

#if 0
    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_YUYV,
                          3264,
                          2448,
                          6528);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");


    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_RGB888,
                          640,
                          480,
                          1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");
#endif // 0

    if (NULL != pYOutput)
    {
        delete pYOutput;
        pYOutput = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream18", IspStream18, IspStreamCase18);



/*
 * ISP stream Test #19: 1 in 1 out test case
 */
void IspStreamCase19(TestReporter *pReporter)
{
    DP_STATUS_ENUM status;
    DpIspStream    *pStream;
    uint8_t        *pYSource;
    uint8_t        *pUSource;
    uint8_t        *pVSource;
    void           *pInBuf[3];
    uint32_t       inSize[3];
    uint8_t        *pYOutput;
    uint8_t        *pUOutput;
    uint8_t        *pVOutput;
    void           *pOutBuf[3];
     uint32_t      outSize[3];

    pStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pYSource = (uint8_t*)malloc(1280 * 768);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYSource != NULL), "Can't allocate Y source image buffer\n");

    pUSource = (uint8_t*)malloc(1280 * 768 >> 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUSource != NULL), "Can't allocate U source image buffer\n");

    pVSource = (uint8_t*)malloc(1280 * 768 >> 1);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVSource != NULL), "Can't allocate V source image buffer\n");

    utilReadBMP("/data/1280x768.bmp",
                pYSource,
                pUSource,
                pVSource,
                DP_COLOR_YV16,
                1280,
                768,
                1280);

    pInBuf[0] = pYSource;
    pInBuf[1] = pUSource;
    pInBuf[2] = pVSource;

    inSize[0] = 1280 * 768;
    inSize[1] = (1280 * 768 >> 1);
    inSize[2] = (1280 * 768 >> 1);

    status = pStream->queueSrcBuffer(pInBuf,
                                     inSize,
                                     3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(1280,
                                   768,
                                   1280,
                                   640,
                                   DP_COLOR_YV16);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pYOutput = (uint8_t*)malloc(800 * 480);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate Y target image buffer1\n");

    pUOutput = (uint8_t*)malloc(400 * 240);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUOutput != NULL), "Can't allocate Y target image buffer1\n");

    pVOutput = (uint8_t*)malloc(400 * 240);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOutput != NULL), "Can't allocate Y target image buffer1\n");

    pOutBuf[0] = pYOutput;
    pOutBuf[1] = pUOutput;
    pOutBuf[2] = pVOutput;

    outSize[0] = 800 * 480;
    outSize[1] = 400 * 240;
    outSize[2] = 400 * 240;
    status = pStream->queueDstBuffer(0,
                                     pOutBuf,
                                     outSize,
                                     3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   800,
                                   480,
                                   800,
                                   400,
                                   DP_COLOR_I420);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    status = pStream->setSrcCrop(0,
                                 0,
                                 0,
                                 0,
                                 1280,
                                 768);

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &pOutBuf[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGD("Port0 Y target buffer: 0x%08x\n", pOutBuf[0]);
    DPLOGD("Port0 U target buffer: 0x%08x\n", pOutBuf[1]);
    DPLOGD("Port0 V target buffer: 0x%08x\n", pOutBuf[2]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

    status = utilWriteBMP("/data/output.bmp",
                          pOutBuf[0],
                          pOutBuf[1],
                          pOutBuf[2],
                          DP_COLOR_I420,
                          800,
                          480,
                          800,
                          240);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");

    if (NULL != pYOutput)
    {
        delete pYOutput;
        pYOutput = NULL;
    }

    if (NULL != pUOutput)
    {
        delete pUOutput;
        pUOutput = NULL;
    }

    if (NULL != pVOutput)
    {
        delete pVOutput;
        pVOutput = NULL;
    }

    if (NULL != pYSource)
    {
        delete pYSource;
        pYSource = NULL;
    }

    if (NULL != pUSource)
    {
        delete pUSource;
        pUSource = NULL;
    }

    if (NULL != pVSource)
    {
        delete pVSource;
        pVSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream19", IspStream19, IspStreamCase19);
#endif


#if 0
/*
 * ISP stream Test #18: 1 in 2 out test case
 */
void IspStreamCase19(TestReporter *pReporter)
{
    DP_STATUS_ENUM           status;
    DpIspStream              *pStream;
    uint8_t                  *pSource;
    uint8_t                  *pYOutput;
    uint8_t                  *pUOutput;
    uint8_t                  *pVOutput;
    void                     *pOutBuf[3];
     uint32_t                outSize[3];
    ISP_TPIPE_CONFIG_STRUCT  ISPInfo;

    pStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
    REPORTER_ASSERT_MESSAGE(pReporter, (pStream != NULL), "Can't allocate DpIspStream object\n");

    pSource = (uint8_t*)malloc(2000 * 1200);
    REPORTER_ASSERT_MESSAGE(pReporter, (pSource != NULL), "Can't allocate source image buffer\n");

    status = pStream->queueSrcBuffer(pSource,
                                     2000 * 1200);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register source buffer failed\n");

    status = pStream->setSrcConfig(DP_COLOR_BAYER10,
                                   1600,
                                   1200,
                                   2000);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure source port failed\n");

    pYOutput = (uint8_t*)malloc(800 * 600);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate Y target image buffer1\n");

    pUOutput = (uint8_t*)malloc(400 * 300);
    REPORTER_ASSERT_MESSAGE(pReporter, (pUOutput != NULL), "Can't allocate Y target image buffer1\n");

    pVOutput = (uint8_t*)malloc(400 * 300);
    REPORTER_ASSERT_MESSAGE(pReporter, (pVOutput != NULL), "Can't allocate Y target image buffer1\n");

    pOutBuf[0] = pYOutput;
    pOutBuf[1] = pUOutput;
    pOutBuf[2] = pVOutput;

    outSize[0] = 800 * 600;
    outSize[1] = 400 * 300;
    outSize[2] = 400 * 300;
    status = pStream->queueDstBuffer(0,
                                     pOutBuf,
                                     outSize,
                                     3);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(0,
                                   800,
                                   600,
                                   800,
                                   400,
                                   DP_COLOR_YV12);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    pYOutput = (uint8_t*)malloc(2400 * 1600);
    REPORTER_ASSERT_MESSAGE(pReporter, (pYOutput != NULL), "Can't allocate Y target image buffer1\n");

    pOutBuf[0] = pYOutput;
    pOutBuf[1] = 0;
    pOutBuf[2] = 0;

    outSize[0] = 2400 * 1600 * 2;
    outSize[1] = 0;
    outSize[2] = 0;
    status = pStream->queueDstBuffer(1,
                                     pOutBuf,
                                     outSize,
                                     1);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Register target image buffer0 failed\n");

    status = pStream->setDstConfig(1,
                                   1200,
                                   1600,
                                   2400,
                                   0,
                                   DP_COLOR_YUYV);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Configure target port1 failed\n");

    status = pStream->setRotation(1, 90);

    status = pStream->setSrcCrop(0,
                                 0,
                                 0,
                                 0,
                                 1600,
                                 1200);

    status = pStream->setParameter(ISPInfo);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP extra parameter failed\n");

    status = pStream->startStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Start ISP stream processing failed\n");

    status = pStream->dequeueSrcBuffer();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue source image buffer failed\n");

    status = pStream->dequeueDstBuffer(0,
                                       &pOutBuf[0],
                                       true);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dequeue target image buffer0 failed\n");

    DPLOGI("Port0 target buffer: 0x%08x\n", pOutBuf[0]);

    status = pStream->stopStream();
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "stop ISP stream failed\n");

#if 0
    status = utilWriteBMP("/data/output1.bmp",
                          (uint8_t*)outBuf0[0],
                          (uint8_t*)outBuf0[1],
                          (uint8_t*)outBuf0[2],
                          DP_COLOR_YUYV,
                          3264,
                          2448,
                          6528);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer0 failed\n");


    status = utilWriteBMP("/data/output2.bmp",
                          (uint8_t*)outBuf1[0],
                          (uint8_t*)outBuf1[1],
                          (uint8_t*)outBuf1[2],
                          DP_COLOR_RGB888,
                          640,
                          480,
                          1920);
    REPORTER_ASSERT_MESSAGE(pReporter, (DP_STATUS_RETURN_SUCCESS == status), "Dump target image buffer1 failed\n");
#endif // 0

    if (NULL != pYOutput)
    {
        delete pYOutput;
        pYOutput = NULL;
    }

    if (NULL != pSource)
    {
        delete pSource;
        pSource = NULL;
    }

    if (NULL != pStream)
    {
        delete pStream;
    }
}

DEFINE_TEST_CASE("IspStream19", IspStream19, IspStreamCase19);
#endif // 0
