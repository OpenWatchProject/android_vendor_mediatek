#if 0
extern "C" {
#include "frame_0/rsc_test_case_00_frame_0.h"
#include "frame_1/rsc_test_case_00_frame_1.h"
}
#include <common/include/common.h>
#include <drv/imem_drv.h>
#include <IImageBuffer.h>
#include <iopipe/PostProc/IRscStream.h>

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSRsc;
#else
#include "MultiEnque_rsc_test_case_00_frame_0_rsc_setting_00.h"
#endif
MBOOL g_b_multi_enque_rsc_tc00_request0_RSCCallback;
MBOOL g_b_multi_enque_rsc_tc00_request1_RSCCallback;

int g_bMultipleBuffer = 0;
int g_RscCount = 0;

MVOID RSC_multi_enque_rsc_tc00_request1_RSCCallback(RSCParams& rParams)
{
    printf("--- [request 1 callback func]\n");
    g_b_multi_enque_rsc_tc00_request1_RSCCallback = MTRUE;
}

MVOID RSC_multi_enque_rsc_tc00_request0_RSCCallback(RSCParams& rParams)
{
    printf("--- [request 0 callback func]\n");
    if (g_bMultipleBuffer == 1)
    {
        if (g_RscCount%2==0)
        {
            printf("--- [RSC frame(0/1) in request 0 callback func]\n");            
        }
        else
        {
            printf("--- [RSC frame(1/1) in request 0 callback func]\n");        
        }

        
    }
    else
    {
        printf("--- [RSC frame(0/0) in request 0callback func]\n");
    }
    g_b_multi_enque_rsc_tc00_request0_RSCCallback = MTRUE;
    g_RscCount++; 
}


bool multi_enque_rsc_tc00_frames_RSC_Config()
{
    int ret = 0;

    NSCam::NSIoPipe::NSRsc::IRscStream* pStream;
    pStream= NSCam::NSIoPipe::NSRsc::IRscStream::createInstance("tc00_multi_enque");
    pStream->init();
    printf("--- [test_rsc_default...RscStream init done\n");

    IMemDrv* mpImemDrv = NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMAI memory asignment
    */

    /*static Golden Data*/
    char * golden_mvo_frame  = tc00_frame_0_rsc_mvo_frame_00_00;
    char * golden_bvo_frame  = tc00_frame_0_rsc_bvo_frame_00_00;
    char * golden_mvo_frame1 = tc00_frame_1_rsc_mvo_frame_01_00;
    char * golden_bvo_frame1 = tc00_frame_1_rsc_bvo_frame_01_00;

    MUINT32 mvo_size_bytes   = tc00_frame_0_rsc_mvo_frame_00_00_size;
    MUINT32 bvo_size_bytes   = tc00_frame_0_rsc_bvo_frame_00_00_size;
    MUINT32 mvo1_size_bytes  = tc00_frame_1_rsc_mvo_frame_01_00_size;
    MUINT32 bvo1_size_bytes  = tc00_frame_1_rsc_bvo_frame_01_00_size;

    /*static DMAI memories for frame 0/1 */
    char * rsc_apli_c_frame;
    char * rsc_apli_p_frame;
    char * rsc_mvi_frame;
    char * rsc_imgi_c_frame;
    char * rsc_imgi_p_frame;

    char * rsc_apli_c_frame1;
    char * rsc_apli_p_frame1;
    char * rsc_mvi_frame1;
    char * rsc_imgi_c_frame1;
    char * rsc_imgi_p_frame1;

    rsc_apli_c_frame = tc00_frame_0_rsc_apli_c_frame_00;
    rsc_apli_p_frame = tc00_frame_0_rsc_apli_p_frame_00;
    rsc_mvi_frame    = tc00_frame_0_rsc_mvi_frame_00;
    rsc_imgi_c_frame = tc00_frame_0_rsc_imgi_c_frame_00;
    rsc_imgi_p_frame = tc00_frame_0_rsc_imgi_p_frame_00;

    rsc_apli_c_frame = tc00_frame_1_rsc_apli_c_frame_01;
    rsc_apli_p_frame = tc00_frame_1_rsc_apli_p_frame_01;
    rsc_mvi_frame    = tc00_frame_1_rsc_mvi_frame_01;
    rsc_imgi_c_frame = tc00_frame_1_rsc_imgi_c_frame_01;
    rsc_imgi_p_frame = tc00_frame_1_rsc_imgi_p_frame_01;

    MUINT32 rsc_apli_c_frame_size = tc00_frame_0_rsc_apli_c_frame_00_size;
    MUINT32 rsc_apli_p_frame_size = tc00_frame_0_rsc_apli_p_frame_00_size;
    MUINT32 rsc_mvi_frame_size    = tc00_frame_0_rsc_mvi_frame_00_size;
    MUINT32 rsc_imgi_c_frame_size = tc00_frame_0_rsc_imgi_c_frame_00_size;
    MUINT32 rsc_imgi_p_frame_size = tc00_frame_0_rsc_imgi_p_frame_00_size;

    MUINT32 rsc_apli_c_frame1_size = tc00_frame_1_rsc_apli_c_frame_01_size;
    MUINT32 rsc_apli_p_frame1_size = tc00_frame_1_rsc_apli_p_frame_01_size;
    MUINT32 rsc_mvi_frame1_size    = tc00_frame_1_rsc_mvi_frame_01_size;
    MUINT32 rsc_imgi_c_frame1_size = tc00_frame_1_rsc_imgi_c_frame_01_size;
    MUINT32 rsc_imgi_p_frame1_size = tc00_frame_1_rsc_imgi_p_frame_01_size;

    /*static DMAI memory allocation and copy*/
    printf("--- rsc  input  allocate init\n");
    /*frame0*/
    IMEM_BUF_INFO buf_rsc_apli_c_frame;
    buf_rsc_apli_c_frame.size = rsc_apli_c_frame_size;
    printf("buf_rsc_apli_c_frame.size:%d",buf_rsc_apli_c_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_apli_c_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_apli_c_frame);
    memcpy( (MUINT8*)(buf_rsc_apli_c_frame.virtAddr), (MUINT8*)(rsc_apli_c_frame), buf_rsc_apli_c_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_apli_c_frame);
    
    IMEM_BUF_INFO buf_rsc_apli_p_frame;
    buf_rsc_apli_p_frame.size = rsc_apli_p_frame_size;
    printf("buf_rsc_apli_p_frame.size:%d",buf_rsc_apli_p_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_apli_p_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_apli_p_frame);
    memcpy( (MUINT8*)(buf_rsc_apli_p_frame.virtAddr), (MUINT8*)(rsc_apli_p_frame), buf_rsc_apli_p_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_apli_p_frame);

    IMEM_BUF_INFO buf_rsc_mvi_frame;
    buf_rsc_mvi_frame.size = rsc_mvi_frame_size;
    printf("buf_rsc_mvi_frame.size:%d",buf_rsc_mvi_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_mvi_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_mvi_frame);
    memcpy( (MUINT8*)(buf_rsc_mvi_frame.virtAddr), (MUINT8*)(rsc_mvi_frame), buf_rsc_mvi_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_mvi_frame);

    IMEM_BUF_INFO buf_rsc_imgi_p_frame;
    buf_rsc_imgi_p_frame.size = rsc_imgi_p_frame_size;
    printf("buf_rsc_imgi_p_frame.size:%d",buf_rsc_imgi_p_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_imgi_p_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_imgi_p_frame);
    memcpy( (MUINT8*)(buf_rsc_imgi_p_frame.virtAddr), (MUINT8*)(rsc_imgi_p_frame), buf_rsc_imgi_p_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_imgi_p_frame);

    IMEM_BUF_INFO buf_rsc_imgi_c_frame;
    buf_rsc_imgi_c_frame.size = rsc_imgi_c_frame_size;
    printf("buf_rsc_imgi_c_frame.size:%d",buf_rsc_imgi_c_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_imgi_c_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_imgi_c_frame);
    memcpy( (MUINT8*)(buf_rsc_imgi_c_frame.virtAddr), (MUINT8*)(rsc_imgi_c_frame), buf_rsc_imgi_c_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_imgi_c_frame);
    /*frame1*/
    IMEM_BUF_INFO buf_rsc_apli_c_frame1;
    buf_rsc_apli_c_frame1.size = rsc_apli_c_frame1_size;
    printf("buf_rsc_apli_c_frame1.size:%d",buf_rsc_apli_c_frame1.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_apli_c_frame1);
    mpImemDrv->mapPhyAddr(&buf_rsc_apli_c_frame1);
    memcpy( (MUINT8*)(buf_rsc_apli_c_frame1.virtAddr), (MUINT8*)(rsc_apli_c_frame), buf_rsc_apli_c_frame1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_apli_c_frame1);
    
    IMEM_BUF_INFO buf_rsc_apli_p_frame1;
    buf_rsc_apli_p_frame1.size = rsc_apli_p_frame1_size;
    printf("buf_rsc_apli_p_frame1.size:%d",buf_rsc_apli_p_frame1.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_apli_p_frame1);
    mpImemDrv->mapPhyAddr(&buf_rsc_apli_p_frame1);
    memcpy( (MUINT8*)(buf_rsc_apli_p_frame1.virtAddr), (MUINT8*)(rsc_apli_p_frame), buf_rsc_apli_p_frame1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_apli_p_frame1);

    IMEM_BUF_INFO buf_rsc_mvi_frame1;
    buf_rsc_mvi_frame1.size = rsc_mvi_frame1_size;
    printf("buf_rsc_mvi_frame1.size:%d",buf_rsc_mvi_frame1.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_mvi_frame1);
    mpImemDrv->mapPhyAddr(&buf_rsc_mvi_frame1);
    memcpy( (MUINT8*)(buf_rsc_mvi_frame1.virtAddr), (MUINT8*)(rsc_mvi_frame), buf_rsc_mvi_frame1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_mvi_frame1);

    IMEM_BUF_INFO buf_rsc_imgi_p_frame1;
    buf_rsc_imgi_p_frame1.size = rsc_imgi_p_frame1_size;
    printf("buf_rsc_imgi_p_frame1.size:%d",buf_rsc_imgi_p_frame1.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_imgi_p_frame1);
    mpImemDrv->mapPhyAddr(&buf_rsc_imgi_p_frame1);
    memcpy( (MUINT8*)(buf_rsc_imgi_p_frame1.virtAddr), (MUINT8*)(rsc_imgi_p_frame), buf_rsc_imgi_p_frame1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_imgi_p_frame1);

    IMEM_BUF_INFO buf_rsc_imgi_c_frame1;
    buf_rsc_imgi_c_frame1.size = rsc_imgi_c_frame1_size;
    printf("buf_rsc_imgi_c_frame1.size:%d",buf_rsc_imgi_c_frame1.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_imgi_c_frame1);
    mpImemDrv->mapPhyAddr(&buf_rsc_imgi_c_frame1);
    memcpy( (MUINT8*)(buf_rsc_imgi_c_frame1.virtAddr), (MUINT8*)(rsc_imgi_c_frame), buf_rsc_imgi_c_frame1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_imgi_c_frame1);

    printf("--- rsc DMI allocation done\n");

    /*
    *  4. DMAO memory allocation
    */
    /*frame0*/
    IMEM_BUF_INFO buf_rsc_mvo_frame;    
    buf_rsc_mvo_frame.size = mvo_size_bytes;
    mpImemDrv->allocVirtBuf(&buf_rsc_mvo_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_mvo_frame);
    memset((MUINT8*)buf_rsc_mvo_frame.virtAddr, 0xffffffff, buf_rsc_mvo_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_mvo_frame);

    IMEM_BUF_INFO buf_rsc_bvo_frame;    
    buf_rsc_bvo_frame.size = bvo_size_bytes;
    mpImemDrv->allocVirtBuf(&buf_rsc_bvo_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_bvo_frame);
    memset((MUINT8*)buf_rsc_bvo_frame.virtAddr, 0xffffffff, buf_rsc_bvo_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_bvo_frame);
    /*frame1*/
    IMEM_BUF_INFO buf_rsc_mvo_frame1;    
    buf_rsc_mvo_frame1.size = mvo1_size_bytes;
    mpImemDrv->allocVirtBuf(&buf_rsc_mvo_frame1);
    mpImemDrv->mapPhyAddr(&buf_rsc_mvo_frame1);
    memset((MUINT8*)buf_rsc_mvo_frame1.virtAddr, 0xffffffff, buf_rsc_mvo_frame1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_mvo_frame1);

    IMEM_BUF_INFO buf_rsc_bvo_frame1;    
    buf_rsc_bvo_frame1.size = bvo1_size_bytes;
    mpImemDrv->allocVirtBuf(&buf_rsc_bvo_frame1);
    mpImemDrv->mapPhyAddr(&buf_rsc_bvo_frame1);
    memset((MUINT8*)buf_rsc_bvo_frame1.virtAddr, 0xffffffff, buf_rsc_bvo_frame1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_bvo_frame1);

    /*
    * default request
    */
    RSCParams rRscParams;
    RSCConfig rscconfig;
    rRscParams.mpfnCallback = RSC_multi_enque_rsc_tc00_request0_RSCCallback;

    rscconfig.Rsc_Apli_c.u4BufVA = buf_rsc_apli_c_frame.virtAddr;
    rscconfig.Rsc_Apli_p.u4BufVA = buf_rsc_apli_p_frame.virtAddr;
    rscconfig.Rsc_mvi.u4BufVA    = buf_rsc_mvi_frame.virtAddr;
    rscconfig.Rsc_Imgi_c.u4BufVA = buf_rsc_imgi_c_frame.virtAddr;
    rscconfig.Rsc_Imgi_p.u4BufVA = buf_rsc_imgi_p_frame.virtAddr;
    rscconfig.Rsc_mvo.u4BufVA    = buf_rsc_mvo_frame.virtAddr;
    rscconfig.Rsc_bvo.u4BufVA    = buf_rsc_bvo_frame.virtAddr;

    rscconfig.Rsc_Apli_c.u4BufPA = buf_rsc_apli_c_frame.phyAddr;
    rscconfig.Rsc_Apli_p.u4BufPA = buf_rsc_apli_p_frame.phyAddr;
    rscconfig.Rsc_mvi.u4BufPA    = buf_rsc_mvi_frame.phyAddr;
    rscconfig.Rsc_Imgi_c.u4BufPA = buf_rsc_imgi_c_frame.phyAddr;
    rscconfig.Rsc_Imgi_p.u4BufPA = buf_rsc_imgi_p_frame.phyAddr;
    rscconfig.Rsc_mvo.u4BufPA    = buf_rsc_mvo_frame.phyAddr;
    rscconfig.Rsc_bvo.u4BufPA    = buf_rsc_bvo_frame.phyAddr;

    rscconfig.Rsc_Imgi_c.u4Stride = 0x140;
    rscconfig.Rsc_Imgi_p.u4Stride = 0x150;
    rscconfig.Rsc_mvi.u4Stride    = 0x150;
    rscconfig.Rsc_mvo.u4Stride    = 0x200;
    rscconfig.Rsc_bvo.u4Stride    = 0x160;


    rscconfig.Rsc_Ctrl_Init_MV_Waddr     = 0x14;
    rscconfig.Rsc_Ctrl_Init_MV_Flush_cnt = 0x3;
    rscconfig.Rsc_Ctrl_Trig_Num          = 0x1;
    rscconfig.Rsc_Ctrl_Imgi_c_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig.Rsc_Ctrl_Imgi_p_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig.Rsc_Ctrl_Gmv_mode          = 0x1;
    rscconfig.Rsc_Ctrl_First_Me          = 0x1;
    rscconfig.Rsc_Ctrl_Skip_Pre_Mv       = 0x1;
    rscconfig.Rsc_Ctrl_Start_Scan_Order  = 0x1;

    rscconfig.Rsc_Size_Height  = 0x01FF;
    rscconfig.Rsc_Size_Width   = 0x0120;

    rscconfig.Rsc_Horz_Sr   = 0x0008;
    rscconfig.Rsc_Vert_Sr   = 0x0020;

    rRscParams.mRSCConfigVec.push_back(rscconfig);

    /*
    *   Another Request
    */ 
    RSCParams rRscParams1;
    RSCConfig rscconfig1;
    rRscParams1.mpfnCallback = RSC_multi_enque_rsc_tc00_request1_RSCCallback;

    rscconfig1.Rsc_Apli_c.u4BufVA = buf_rsc_apli_c_frame1.virtAddr;
    rscconfig1.Rsc_Apli_p.u4BufVA = buf_rsc_apli_p_frame1.virtAddr;
    rscconfig1.Rsc_mvi.u4BufVA    = buf_rsc_mvi_frame1.virtAddr;
    rscconfig1.Rsc_Imgi_c.u4BufVA = buf_rsc_imgi_c_frame1.virtAddr;
    rscconfig1.Rsc_Imgi_p.u4BufVA = buf_rsc_imgi_p_frame1.virtAddr;
    rscconfig1.Rsc_mvo.u4BufVA    = buf_rsc_mvo_frame1.virtAddr;
    rscconfig1.Rsc_bvo.u4BufVA    = buf_rsc_bvo_frame1.virtAddr;

    rscconfig1.Rsc_Apli_c.u4BufPA = buf_rsc_apli_c_frame1.phyAddr;
    rscconfig1.Rsc_Apli_p.u4BufPA = buf_rsc_apli_p_frame1.phyAddr;
    rscconfig1.Rsc_mvi.u4BufPA    = buf_rsc_mvi_frame1.phyAddr;
    rscconfig1.Rsc_Imgi_c.u4BufPA = buf_rsc_imgi_c_frame1.phyAddr;
    rscconfig1.Rsc_Imgi_p.u4BufPA = buf_rsc_imgi_p_frame1.phyAddr;
    rscconfig1.Rsc_mvo.u4BufPA    = buf_rsc_mvo_frame1.phyAddr;
    rscconfig1.Rsc_bvo.u4BufPA    = buf_rsc_bvo_frame1.phyAddr;

    rscconfig1.Rsc_Imgi_c.u4Stride = 0x1C0;
    rscconfig1.Rsc_Imgi_p.u4Stride = 0x150;
    rscconfig1.Rsc_mvi.u4Stride    = 0x240;
    rscconfig1.Rsc_mvo.u4Stride    = 0x1e0;
    rscconfig1.Rsc_bvo.u4Stride    = 0x180;


    rscconfig1.Rsc_Ctrl_Init_MV_Waddr     = 0x14;
    rscconfig1.Rsc_Ctrl_Init_MV_Flush_cnt = 0x3;
    rscconfig1.Rsc_Ctrl_Trig_Num          = 0x1;
    rscconfig1.Rsc_Ctrl_Imgi_c_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig1.Rsc_Ctrl_Imgi_p_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig1.Rsc_Ctrl_Gmv_mode          = 0x1;
    rscconfig1.Rsc_Ctrl_First_Me          = 0x1;
    rscconfig1.Rsc_Ctrl_Skip_Pre_Mv       = 0x0;
    rscconfig1.Rsc_Ctrl_Start_Scan_Order  = 0x1;

    rscconfig1.Rsc_Size_Height  = 0x01FF;
    rscconfig1.Rsc_Size_Width   = 0x0120;

    rscconfig1.Rsc_Horz_Sr   = 0x0008;
    rscconfig1.Rsc_Vert_Sr   = 0x0020;

    if (g_bMultipleBuffer == 0)
    {        
        rRscParams1.mRSCConfigVec.push_back(rscconfig1);
    }
    else if (g_bMultipleBuffer == 1)
    {
        rRscParams.mRSCConfigVec.push_back(rscconfig1);
    }
    
    g_b_multi_enque_rsc_tc00_request0_RSCCallback = MFALSE;
    g_b_multi_enque_rsc_tc00_request1_RSCCallback = MFALSE;

    /*
    *   enque the request(s)
    */
    ret=pStream->RSCenque(rRscParams);
    if(!ret)
    {
        printf("---multi_enque_rsc_tc00_frame_0 ERRRRRRRRR [.rsc enque fail\n]");
    }
    else
    {
        printf("---multi_enque_rsc_tc00_frame_0 [.rsc enque done\n]");
    }
    if (g_bMultipleBuffer == 0)
    {
        ret=pStream->RSCenque(rRscParams1);
        if(!ret)
        {
            printf("---multi_enque_rsc_tc00_frame_1 ERRRRRRRRR [..frame1 rsc enque fail\n]");
        }
        else
        {
            printf("---multi_enque_rsc_tc00_frame_1 [.frame1 rsc enque done\n]");
        }

    }

    /*
    *  Compare rsc_mvo/bvo_frame_0
    */
    do{
        usleep(100000);
        if (MTRUE == g_b_multi_enque_rsc_tc00_request0_RSCCallback)
        {
            break;
        }
    }while(1);


#if 0
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_mvo_frame);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_bvo_frame);

    int_data_dma_0 = 0;
    int_data_dma_1 = 0;
    rsc_curr_width = (REG_RSC_RSC_SIZE_0 & 0xfff);
    rsc_curr_height = ((REG_RSC_RSC_SIZE_0 >> 16) & 0xfff);
    int_data_dma_2 = rsc_curr_width-1;
    int_data_dma_3 = rsc_curr_height-1;

    err_cnt_dma = comp_roi_mem_with_file((char*)golden_mvo_frame, 
                          0, 
                          rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4BufVA,
                          rsc_curr_width,
                          rsc_curr_height,
                          rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4Stride,
                          int_data_dma_0,
                          int_data_dma_1,
                          int_data_dma_2,
                          int_data_dma_3,
                          0,
                          0,
                          0
                          ); 
    if (err_cnt_dma)
    {
        //Error
        printf("rsc RSC DPO Frame0 bit true fail: errcnt (%d)!!!\n", err_cnt_dma);
        do
        {
            usleep(200000);
            printf("rsc RSC DPO Frame0 error:(%d)!!!\n", err_cnt_dma);
        }while(1);
    }
    else
    {
        //Pass
        printf("rsc RSC DPO Frame0 bit true pass!!!\n");
    }
#endif

    if (g_bMultipleBuffer == 0)
    {
        do{
            usleep(100000);
            if (MTRUE == g_b_multi_enque_rsc_tc00_request1_RSCCallback)
            {
                break;
            }
        }while(1);
    }


#if 0
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_mvo_frame1);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_bvo_frame1);

    int_data_dma_0 = 0;
    int_data_dma_1 = 0;
    rsc_curr_width = (REG_RSC_RSC_SIZE_0 & 0xfff);
    rsc_curr_height = ((REG_RSC_RSC_SIZE_0 >> 16) & 0xfff);
    int_data_dma_2 = rsc_curr_width-1;
    int_data_dma_3 = rsc_curr_height-1;

    err_cnt_dma = comp_roi_mem_with_file((char*)frame1_golden_rsc_wmf_dpo_frame_0, 
                          0, 
                          frame1_rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4BufVA,
                          rsc_curr_width,
                          rsc_curr_height,
                          frame1_rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4Stride,
                          int_data_dma_0,
                          int_data_dma_1,
                          int_data_dma_2,
                          int_data_dma_3,
                          0,
                          0,
                          0
                          ); 
    if (err_cnt_dma)
    {
        //Error
        printf("rsc frame1 RSC DPO Frame0 bit true fail: errcnt (%d)!!!\n", err_cnt_dma);
        do
        {
            usleep(200000);
            printf("rsc frame1 RSC DPO Frame0 error:(%d)!!!\n", err_cnt_dma);
        }while(1);
    }
    else
    {
        //Pass
        printf("rsc frame1 RSC DPO Frame0 bit true pass!!!\n");
    }
#endif

    mpImemDrv->freeVirtBuf(&buf_rsc_apli_c_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_apli_p_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_mvi_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_imgi_c_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_imgi_p_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_mvo_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_bvo_frame);

    mpImemDrv->freeVirtBuf(&buf_rsc_apli_c_frame1);
    mpImemDrv->freeVirtBuf(&buf_rsc_apli_p_frame1);
    mpImemDrv->freeVirtBuf(&buf_rsc_mvi_frame1);
    mpImemDrv->freeVirtBuf(&buf_rsc_imgi_c_frame1);
    mpImemDrv->freeVirtBuf(&buf_rsc_imgi_p_frame1);
    mpImemDrv->freeVirtBuf(&buf_rsc_mvo_frame1);
    mpImemDrv->freeVirtBuf(&buf_rsc_bvo_frame1);
   
    pStream->uninit();   
    printf("--- [RscStream uninit done\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done\n");


    return true;
}
