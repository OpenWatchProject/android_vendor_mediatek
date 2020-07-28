#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <IPipe.h>
#include <ICamIOPipe.h>
#include "isp_drv_cam.h"//for cfg the test pattern of seninf only
#include <tuning_mgr.h>

#include <cam_capibility.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
//#include "Cam_Notify.h"
#include <Cam_Notify_datatype.h>
#include <utils/Thread.h>
#include <semaphore.h>

#undef LOG_TAG
#define LOG_TAG "Test_ImageIo_Cam"

using namespace NSImageio;
using namespace NSIspio;


typedef enum{
    E_5M,
    E_8M,
    E_16M
}E_TGSZIE;

int TG_SIZE(E_TGSZIE size,MUINT32 *pSize_w, MUINT32 *pSize_h)
{
    switch(size){
        case E_16M:
            *pSize_w = 6400;
            *pSize_h = 4800;
            break;
        case E_8M:
            *pSize_w = 3200;
            *pSize_h = 2400;
            break;
        case E_5M:
            *pSize_w = 2592;
            *pSize_h = 1944;
            break;
        default:
            printf("#####################################\n");
            printf("error:unsupported size:0x%x\n",size);
            printf("#####################################\n");
            return 1;
            break;
    }
    return 0;
}

IspDrvImp* g_pDrvCam[2] = {NULL};


//pix_mode 1: 1pix, 2: 2pix, 4:4pix

int Seninf_Pattern(MUINT32 cnt,MBOOL bYUV,MUINT32 pix_mode)
{
    MINT32 ret = 0;

    g_pDrvCam[0] = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
    g_pDrvCam[0]->init("test pattern");
    g_pDrvCam[0]->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only

    if(cnt>1){
        g_pDrvCam[1] = (IspDrvImp*)IspDrvImp::createInstance(CAM_B);
        g_pDrvCam[1]->init("test pattern");
        g_pDrvCam[1]->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only
    }
    printf("#####################################\n");
    printf("SENINF start in TestPattern Mode !!!![is YUV = %d]\n",bYUV);
    printf("pattern size :%d x %d with %d enabled pattern!!!!\n",0x1f00,0xfa0,cnt);
    printf("pix_mode: %d\n", pix_mode);
    printf("#####################################\n");


    for(MUINT32 i=0;i< cnt;i++){
        printf("%d pattern open\n",i+1);
        printf("#####################################\n");

        static MUINT32 mPowerOnTM_2[][2] = {
            0x15040000, 0x00000C00, //SENINF_TOP_CTRL
            0x15040200, 0x00001001,
            0x15040d00, 0x96DF1080,
            0x15040d04, 0x8000007F,
            0x15040d0c, 0x00000000,
            0x15040d2c, 0x000E2000,
            0x15040d3c, 0x00000000,
            0x15040d38, 0x00000000,
            0x15040608, 0x000404C1, // TG1_TM_CTL
                                    // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
            0x1504060c, 0x1F001F00, // TG1_TM_SIZE
            0x15040610, 0x00000000,  // TG1_TM_CLK
            0x15040614, 0x1,        // always setting timestamp dividor to 1 for test only
        };



        for(MUINT32 j=0;j<(sizeof(mPowerOnTM_2)/(sizeof(MUINT32)*2)); j++){
            g_pDrvCam[i]->writeReg(mPowerOnTM_2[j][0]- 0x15040000 ,mPowerOnTM_2[j][1]);
        }

        // yuv format
        if(1 == bYUV)
        {
            g_pDrvCam[i]->writeReg(0x15040608 - 0x15040000 , g_pDrvCam[i]->readReg(0x15040608 - 0x15040000 ) | 0x4);
        }

        // pixel mode
        switch(pix_mode)
        {
                case 1:
                    g_pDrvCam[i]->writeReg(0x15040D00 - 0x15040000 , g_pDrvCam[i]->readReg(0x15040D00 - 0x15040000 ) & ~0x100);
                    g_pDrvCam[i]->writeReg(0x15040D3C - 0x15040000 , g_pDrvCam[i]->readReg(0x15040D3C - 0x15040000 ) & ~0x10);
                    break;
                case 2:
                    g_pDrvCam[i]->writeReg(0x15040D00 - 0x15040000 , g_pDrvCam[i]->readReg(0x15040D00 - 0x15040000 ) | 0x100);
                    g_pDrvCam[i]->writeReg(0x15040D3C - 0x15040000 , g_pDrvCam[i]->readReg(0x15040D3C - 0x15040000 ) & 0x10);
                    break;
                case 4:
                    g_pDrvCam[i]->writeReg(0x15040D00 - 0x15040000 , g_pDrvCam[i]->readReg(0x15040D00 - 0x15040000 ) & ~0x100);
                    g_pDrvCam[i]->writeReg(0x15040D3C - 0x15040000 , g_pDrvCam[i]->readReg(0x15040D3C - 0x15040000 ) | 0x10);
                    break;
                default:
                    printf("#####################################\n");
                    printf("error:unsupported pix_mode:0x%d\n",pix_mode);
                    printf("#####################################\n");
        }
        g_pDrvCam[0]->writeReg(0x15040204 - 0x15040000 , g_pDrvCam[0]->readReg(0x15040204 - 0x15040000 ) | 0x02);
    }
    g_pDrvCam[0]->setRWMode(ISP_DRV_RW_MMAP);
    if(g_pDrvCam[1])
        g_pDrvCam[1]->setRWMode(ISP_DRV_RW_MMAP);



    return ret;
}

int Seninf_Pattern_close(void)
{
    printf("############################\n");
    printf("Seninf_Pattern_close\n");
    printf("############################\n");
    g_pDrvCam[0]->uninit("test pattern");
    g_pDrvCam[0]->destroyInstance();
    g_pDrvCam[0] = NULL;
    if(g_pDrvCam[1]){
        g_pDrvCam[1]->uninit("test pattern");
        g_pDrvCam[1]->destroyInstance();
        g_pDrvCam[1] = NULL;
    }
    return 0;
}

class TEST_TUNING
{
public:
    TEST_TUNING(MUINT32 senSorId){sensorIdx = senSorId;tuningMgr = NULL;}
    ~TEST_TUNING(void){}
    int AF_START(void);
    int AF_CFG(MUINT32 in_h,MUINT32 in_v,MUINT32 mag);
    int AF_STOP(void);

public:

private:
    MUINT32     sensorIdx;
    TuningMgr*  tuningMgr;
    char* userName = "af_cfg";
};

class TuningNotifyImp_EIS : public P1_TUNING_NOTIFY {
    public:
        TuningNotifyImp_EIS() {}
        ~TuningNotifyImp_EIS() {};
        virtual const char* TuningName() { return "UT_EIS"; }
        virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

};

class TuningNotifyImp_SGG2 : public P1_TUNING_NOTIFY {
    public:
        TuningNotifyImp_SGG2() {}
        ~TuningNotifyImp_SGG2() {};
        virtual const char* TuningName() { return "UT_SGG2"; }
        virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

};


class TEST_CAMIO
{
public:
    TEST_CAMIO(MUINT32 senidx)
        :ring_cnt(0)
        ,bypass_imgo(MTRUE)
        ,bypass_rrzo(MTRUE)
        ,bypass_af(MTRUE)
        ,bypass_eiso(MTRUE)
        ,bypass_thread11(MTRUE)
        ,bypass_thread1(MTRUE)
        ,bypass_tg(MFALSE)
        ,pCamIo(NULL)
        ,bEnque_before_start(MFALSE)
        ,rrz_out_size_shift(1)
        ,bthread_start(MTRUE)
        ,Surce(TG_A)
        ,Source_Size(E_5M)
        ,TG_PixMode(ePixMode_1)
        ,buf_imgo(NULL)
        ,buf_imgo_fh(NULL)
        ,buf_rrzo(NULL)
        ,buf_rrzo_fh(NULL)
        ,buf_eiso(NULL)
        ,buf_eiso_fh(NULL)
        ,pSttIO(NULL)
    {
        SenIdx = senidx;
        pTuning = new TEST_TUNING(SenIdx);
        m_subSample = 0;

        memset(&m_Thread, 0, sizeof(pthread_t));
        memset(&m_semP1Start, 0, sizeof(sem_t));

    }

    ~TEST_CAMIO(){
        delete pTuning;
        pTuning = NULL;
    }

    int     Stop_cam(void);
    int     Case_0(void);
    int     Case_1(void);

private:
    int enque(void);
    int deque(void);
public:
    MBOOL       bEnque_before_start;
    MBOOL       bypass_imgo;
    MBOOL       bypass_rrzo;
    MBOOL       bypass_af;
    MBOOL       bypass_eiso;
    MBOOL       bypass_thread11;
    MBOOL       bypass_thread1;

    E_INPUT     Surce;
    E_TGSZIE    Source_Size;
    E_CamPixelMode    TG_PixMode;
    MUINT32     rrz_out_size_shift;
    MUINT32     m_subSample;

    pthread_t   m_Thread;
    MBOOL       bthread_start;
    sem_t       m_semP1Start;

private:
    #define BUF_DEPTH (2)

    IMEM_BUF_INFO* buf_imgo;
    IMEM_BUF_INFO* buf_imgo_fh;
    IMEM_BUF_INFO* buf_rrzo;
    IMEM_BUF_INFO* buf_rrzo_fh;
    IMEM_BUF_INFO buf_afo[BUF_DEPTH];
    IMEM_BUF_INFO buf_afo_fh[BUF_DEPTH];
    IMEM_BUF_INFO* buf_eiso;
    IMEM_BUF_INFO* buf_eiso_fh;

    MUINT32     ring_cnt;

    PortInfo    imgo;
    STImgCrop   imgo_crop;


    PortInfo    rrzo;
    STImgCrop   rrzo_crop;
    STImgResize rrzo_resize;


    PortInfo    afo;


    PortInfo    tgi;
    MBOOL       bypass_tg;

    ICamIOPipe *pCamIo;
    ICamIOPipe *pSttIO;

    TEST_TUNING *pTuning;    //dual sensor

    MUINT32     SenIdx;


    TuningNotifyImp_SGG2    m_sgg2;
    TuningNotifyImp_EIS     m_eis;
};



void TuningNotifyImp_EIS::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
#define EIS_WIN_H   4
#define EIS_WIN_V   8
#define EIS_OFST_X  17
#define EIS_OFST_Y  17
#define MAX_STEP_H  16
#define MAX_STEP_V  8
#define EIS_XSIZE   0xFF
#define EIS_YSIZE   0x0
    LMV_INPUT_INFO  lmv_in = *(LMV_INPUT_INFO*)pIn;
    LMV_CFG lmv_cfg;
    MUINT32 win_h = 4, win_v = 8;
    MUINT32 ofst_x=0, ofst_y=0;
    MUINT32 FL_ofst_x = 0,FL_ofst_y=0;
    MUINT32 in_size_h, in_size_v;
    MUINT32 pix_mode = 0, bFG = MFALSE, eis_pipe_mode = 0;
    MUINT32 step_h = 16,step_v = 8, win_size_h, win_size_v, op_h=1, op_v=1;
    MUINT32 first_frm_flag  = MTRUE;

    printf("EIS info: 1stFrm(%d) RMXOut(%d,%d) HBinOut(%d,%d) TG(%d,%d) Fmt(x%x) PixMod(%d)",\
            first_frm_flag, lmv_in.sRMXOut.w, lmv_in.sRMXOut.h, lmv_in.sHBINOut.w, lmv_in.sHBINOut.h,\
            lmv_in.sTGOut.w, lmv_in.sTGOut.h, lmv_in.bYUVFmt, lmv_in.pixMode);

    //max window number
    win_h = (win_h > EIS_WIN_H)?(EIS_WIN_H):(win_h);
    win_v = (win_v > EIS_WIN_V)?(EIS_WIN_V):(win_v);

    //ofst,ofst have min constraint
    ofst_x = (ofst_x < EIS_OFST_X)?(EIS_OFST_X):(ofst_x);
    ofst_y = (ofst_y < EIS_OFST_Y)?(EIS_OFST_Y):(ofst_y);

    //floating ofset, current frame ofst from previous frame, normally set 0.

    pix_mode    = lmv_in.pixMode; //TBD
    in_size_h   = lmv_in.sHBINOut.w;
    in_size_v   = lmv_in.sHBINOut.h;

    step_h = (step_h > MAX_STEP_H)?(MAX_STEP_H):(step_h);
    step_v = (step_v > MAX_STEP_V)?(MAX_STEP_V):(step_v);

    win_size_h = ((in_size_h/op_h) - (step_h*16) - ofst_x)/win_h;
    while(win_size_h < ((step_h + 1) * 16 + 2)){
        if( op_h> 1)
            op_h = op_h>>1;
        else{
            if(step_h > 1)
                step_h--;
            else{
                if(win_h > 1)
                    win_h -=1;
                else{
                    printf("#######################\n");
                    printf("#######################\n");
                    printf("EIS H WINDOW SIZE FAIL\n");
                    printf("#######################\n");
                    printf("#######################\n");
                    break;
                }
            }
        }
        win_size_h = ((((in_size_h - ofst_x)/op_h)/16) - step_h) / win_h ;
    }

    win_size_v = ((in_size_v/op_v) - (step_v*16) - ofst_y)/win_v;
    while(win_size_v < ((step_v + 1) * 16 + 2)){
        if(op_v > 1)
            op_v = op_v>>1;
        else{
            if(step_v > 1)
                step_v--;
            else{
                if(win_v > 1)
                    win_v -=1;
                else{
                    printf("#######################\n");
                    printf("#######################\n");
                    printf("EIS V WINDOW SIZE FAIL\n");
                    printf("#######################\n");
                    printf("#######################\n");
                    break;
                }
            }
        }
        win_size_v = ((((in_size_v - ofst_y)/op_v)/16) - step_v) / win_v ;
    }

    lmv_cfg.bypassLMV = MFALSE;
    lmv_cfg.enLMV     = MTRUE;
    /* IN ISP4.0 ALWAYS USE GSE to replace SUBG_EN */
    /* 0x3550 TBD */ lmv_cfg.cfg_lmv_prep_me_ctrl1 = ((win_v<<28)|(win_h<<25)|(step_v<<21)|(0<<16/*knee1 RSTVAL*/)
                                |(0<<12/*knee2 RSTVAL*/)|(step_h<<8)|(0/*SUBG_EN*/<<5)|(op_v<<3)|(op_h));
    /* 0x3554 */ lmv_cfg.cfg_lmv_prep_me_ctrl2 = (0x0000506C/*RSTVAL*/);
    /* 0x3558 TBD */ lmv_cfg.cfg_lmv_lmv_th = 0/*RSTVAL*/;
    /* 0x355C */ lmv_cfg.cfg_lmv_fl_offset = (((FL_ofst_x&0xFFF)<<16) | (FL_ofst_y&0xFFF));
    /* 0x3560 */ lmv_cfg.cfg_lmv_mb_offset = (((ofst_x&0xFFF)<<16) | (ofst_y&0xFFF));
    /* 0x3564 */ lmv_cfg.cfg_lmv_mb_interval = (((win_size_h&0xFFF)<<16)| (win_size_v&0xFFF));
    /* 0x3568 */ lmv_cfg.cfg_lmv_gmv    = 0; /* RO, GMV */
    /* 0x356C TBD */ lmv_cfg.cfg_lmv_err_ctrl = 0x000F0000/*RSTVAL*/;
    /* 0x3570 TBD*/ lmv_cfg.cfg_lmv_image_ctrl = ((eis_pipe_mode/*RSTVAL*/<<31)|((in_size_h&0x3FFF)<<16)
                                |(in_size_v&0x3FFF));

    *(LMV_CFG*)pOut = lmv_cfg;
}

void TuningNotifyImp_SGG2::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    LMV_SGG_CFG sgg2_cfg;
    (void)pIn;

    sgg2_cfg.bSGG2_Bypass   = 1; //TBD
    sgg2_cfg.bSGG2_EN       = 1; //TBD
    sgg2_cfg.PGN  = 0x10/*RSTVAL*/;
    sgg2_cfg.GMRC_1 = 0x10080402/*RSTVAL*/;
    sgg2_cfg.GMRC_2 = 0x00804020/*RSTVAL*/;

    //TBD CAM_UNI_TOP_MOD_EN |= SGG2_A_EN ???

    *(LMV_SGG_CFG*)pOut = sgg2_cfg;
}

int TEST_CAMIO::enque(void)
{
    int ret=0;
    PortID portID;
    QBufInfo rQBufInfo;
    list<MUINT32> Input_L_mag;

    /**
        perframe control squence:
            Camiopipe:
                1. EPIPECmd_SET_IMGO if imgo_en
                2. enque imgo if imgo_en
                3. EPIPECmd_SET_RRZO if rrzo_en
                4. enque rrzo if rrzo_en
                5. EPIPECmd_SET_P1_UPDATE
            STTiopipe:
                1. enque only. [no dependency with Camiopipe]
    */

    if(this->bypass_imgo == MFALSE){
        //cfg dmao
        list<STImgCrop> Input_L;
        list<MUINT32> Input2_L;
        MUINT32 rawType;

        for(MUINT32 i=0;i<(this->m_subSample+1);i++){
            Input_L.push_back(this->imgo_crop);
            if(this->ring_cnt&0x1)
                rawType = 0;
            else
                rawType = 1;
            Input2_L.push_back(rawType);

            //enque
            portID.index = EPortIndex_IMGO;

            rQBufInfo.vBufInfo.resize((this->m_subSample+1));

            rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st]  = (MUINT32)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st]    = (MUINTPTR)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st]    = (MUINTPTR)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st]    = (MUINT32)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]      = (MUINTPTR)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st]      = (MUINTPTR)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;


            printf("##############################\n");
            printf("enque_imgo: pa:%p_%p,mag:0x%x,crop:%d_%d_%d_%d,rawtype:%d\n",\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                this->imgo_crop.x,this->imgo_crop.y,\
                this->imgo_crop.w,this->imgo_crop.h,\
                rawType);
            printf("##############################\n");
        }
        pCamIo->sendCommand(EPIPECmd_SET_IMGO,(MINTPTR)&Input_L,(MINTPTR)&Input2_L,0);

        if(pCamIo->enqueOutBuf(portID,rQBufInfo) == MFALSE)
            ret =1;
    }

    if(this->bypass_rrzo == MFALSE){
        //cfg rrz
        list<STImgCrop> Input_L;
        list<STImgResize> Input2_L;
        for(MUINT32 i=0;i<(this->m_subSample+1);i++){
            Input_L.push_back(this->rrzo_crop);
            if(this->ring_cnt & 0x1){
                STImgResize rrz_out_size;
                rrz_out_size.tar_w = this->rrzo_resize.tar_w - 128;
                rrz_out_size.tar_h = this->rrzo_resize.tar_h - 128;
                Input2_L.push_back(rrz_out_size);
            }
            else
                Input2_L.push_back(this->rrzo_resize);


            //enque
            portID.index = EPortIndex_RRZO;

            rQBufInfo.vBufInfo.resize((this->m_subSample+1));

            rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st]  = (MUINT32)this->buf_rrzo[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st]    = (MUINTPTR)this->buf_rrzo[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st]    = (MUINTPTR)this->buf_rrzo[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st]    = (MUINT32)this->buf_rrzo_fh[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]      = (MUINTPTR)this->buf_rrzo_fh[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st]      = (MUINTPTR)this->buf_rrzo_fh[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;



            printf("##############################\n");
            printf("enque_rrzo: pa:%px_%p,mag:0x%x,crop:%d_%d_%d_%d,out:%d_%d\n",\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).m_num,\
                this->rrzo_crop.x,this->rrzo_crop.y,\
                this->rrzo_crop.w,this->rrzo_crop.h,\
                this->rrzo_resize.tar_w,this->rrzo_resize.tar_h);
            printf("##############################\n");
        }
        pCamIo->sendCommand(EPIPECmd_SET_RRZ,(MINTPTR)&Input_L,(MINTPTR)&Input2_L,0);

        if(pCamIo->enqueOutBuf(portID,rQBufInfo) == MFALSE)
            ret = 1;
    }

    if(this->bypass_eiso == MFALSE){
        for(MUINT32 i=0;i<(this->m_subSample+1);i++){
            //enque
            portID.index = EPortIndex_EISO;

            rQBufInfo.vBufInfo.resize((this->m_subSample+1));

            rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st]  = (MUINT32)this->buf_eiso[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st]    = (MUINTPTR)this->buf_eiso[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st]    = (MUINTPTR)this->buf_eiso[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st]    = (MUINT32)this->buf_eiso_fh[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]      = (MUINTPTR)this->buf_eiso_fh[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st]      = (MUINTPTR)this->buf_eiso_fh[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;


            printf("##############################\n");
            printf("enque_eiso: pa:%p_%p,mag:0x%x\n",\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num);
            printf("##############################\n");
        }

        if(pCamIo->enqueOutBuf(portID,rQBufInfo) == MFALSE)
            ret =1;
    }

    for(MUINT32 i=0;i<(this->m_subSample+1);i++){
        Input_L_mag.push_back(this->ring_cnt*(this->m_subSample+1) + i);
        //
        if(this->bypass_af == MFALSE){
            this->pTuning->AF_CFG(this->tgi.u4ImgWidth,this->tgi.u4ImgHeight,(this->ring_cnt*(this->m_subSample+1) + i));
        }
    }
    pCamIo->sendCommand(EPIPECmd_SET_P1_UPDATE,0,(MINTPTR)&Input_L_mag,this->SenIdx);//test pattern, so ,sensor id dan't care


    //
    if(this->bypass_af == MFALSE){
        portID.index = EPortIndex_AFO;

        rQBufInfo.vBufInfo.resize(1);

        rQBufInfo.vBufInfo[0].u4BufSize[ePlane_1st]  = (MUINT32)this->buf_afo[this->ring_cnt].size;
        rQBufInfo.vBufInfo[0].u4BufVA[ePlane_1st]    = (MUINTPTR)this->buf_afo[this->ring_cnt].virtAddr;
        rQBufInfo.vBufInfo[0].u4BufPA[ePlane_1st]    = (MUINTPTR)this->buf_afo[this->ring_cnt].phyAddr;
        rQBufInfo.vBufInfo[0].Frame_Header.u4BufSize[ePlane_1st]    = (MUINT32)this->buf_afo_fh[this->ring_cnt].size;
        rQBufInfo.vBufInfo[0].Frame_Header.u4BufVA[ePlane_1st]      = (MUINTPTR)this->buf_afo_fh[this->ring_cnt].virtAddr;
        rQBufInfo.vBufInfo[0].Frame_Header.u4BufPA[ePlane_1st]      = (MUINTPTR)this->buf_afo_fh[this->ring_cnt].phyAddr;


        if(pSttIO->enqueOutBuf(portID,rQBufInfo) == MFALSE)
            ret =1;


        printf("##############################\n");
        printf("enque_afo: pa:%p_%p,crop:%d_%d_%d_%d\n",\
            (MUINT8*)rQBufInfo.vBufInfo.at(0).u4BufPA[ePlane_1st],\
            (MUINT8*)rQBufInfo.vBufInfo.at(0).Frame_Header.u4BufPA,\
            this->imgo_crop.x,this->imgo_crop.y,\
            this->imgo_crop.w,this->imgo_crop.h);
        printf("##############################\n");

    }

    if( (this->bypass_thread11 == MFALSE) && (this->m_subSample!= 0) ){
        ISPIO_REG_CFG reg1,reg2,reg3;
        list<ISPIO_REG_CFG> input;
        static MUINT32 i=0;
        if(this->ring_cnt&0x1){
            reg1.Addr = 0x0c84;
            reg1.Data = 0x1234;
            reg2.Addr = 0x0c88;
            reg2.Data = 0x5678;
            reg3.Addr = 0x0ca8;
            reg3.Data = (++i);
            input.push_back(reg1);
            input.push_back(reg2);
            input.push_back(reg3);

            pCamIo->sendCommand(EPIPECmd_HIGHSPEED_AE,this->m_subSample,(MINTPTR)&input,1);
        }
    }

    if(this->bypass_thread1 == MFALSE){
        if(this->ring_cnt&0x1)
            pCamIo->sendCommand(EPIPECmd_AE_SMOOTH,0xff,0,0);
        else
            pCamIo->sendCommand(EPIPECmd_AE_SMOOTH,0xfe,0,0);
    }

    this->ring_cnt = (this->ring_cnt + 1)%BUF_DEPTH;


    //magic number & tuning
    //pCamIo->sendCommand(EPIPECmd_SET_P1_UPDATE,MINTPTR arg1,MINTPTR arg2,MINTPTR arg3);
    return ret;
}

int TEST_CAMIO::deque(void)
{
    int ret=0;
    PortID portID;
    QTimeStampBufInfo rQBufInfo;

    if(this->bypass_imgo == MFALSE){
        portID.index = EPortIndex_IMGO;

        if(pCamIo->dequeOutBuf(portID,rQBufInfo) == MFALSE)
            ret = 1;

        for(UINT32 i=0;i<rQBufInfo.vBufInfo.size();i++){
            printf("##############################\n");
            printf("deque_imgo_%d: pa:%px_%p,mag:0x%x,time:0x%lx,tg size:%dx%d,crop:%d_%d_%d_%d\n",i,\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_us,\
                rQBufInfo.vBufInfo.at(i).img_w, rQBufInfo.vBufInfo.at(i).img_h,\
                rQBufInfo.vBufInfo.at(i).crop_win.p.x, rQBufInfo.vBufInfo.at(i).crop_win.p.y,\
                rQBufInfo.vBufInfo.at(i).crop_win.s.w, rQBufInfo.vBufInfo.at(i).crop_win.s.h);
            printf("##############################\n");
        }
    }

    if(this->bypass_rrzo == MFALSE){
        portID.index = EPortIndex_RRZO;

        if(pCamIo->dequeOutBuf(portID,rQBufInfo) == MFALSE)
            ret = 1;

        for(UINT32 i=0;i<rQBufInfo.vBufInfo.size();i++){
            printf("##############################\n");
            printf("deque_rrzo_%d: pa:%p_%p,mag:0x%x,time:0x%x,tg size:%dx%d,crop:%d_%d_%d_%d,out:%d_%d\n",i,\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_us,\
                rQBufInfo.vBufInfo.at(i).img_w, rQBufInfo.vBufInfo.at(i).img_h,\
                rQBufInfo.vBufInfo.at(i).crop_win.p.x, rQBufInfo.vBufInfo.at(i).crop_win.p.y,\
                rQBufInfo.vBufInfo.at(i).crop_win.s.w, rQBufInfo.vBufInfo.at(i).crop_win.s.h,\
                rQBufInfo.vBufInfo.at(i).DstSize.w,rQBufInfo.vBufInfo.at(i).DstSize.h);
            printf("##############################\n");
        }
    }

    if(this->bypass_eiso == MFALSE){
        portID.index = EPortIndex_EISO;

        if(pCamIo->dequeOutBuf(portID,rQBufInfo) == MFALSE)
            ret = 1;

        for(UINT32 i=0;i<rQBufInfo.vBufInfo.size();i++){
            printf("##############################\n");
            printf("deque_eiso_%d: pa:%p_%p,mag:0x%x,time:0x%lx\n",i,\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_us);
            printf("##############################\n");
        }
    }

    //
    if(this->bypass_af == MFALSE){

        //enque
        portID.index = EPortIndex_AFO;


        if(pSttIO->dequeOutBuf(portID,rQBufInfo) == MFALSE)
            ret = 1;

        for(UINT32 i=0;i<rQBufInfo.vBufInfo.size();i++){
            printf("##############################\n");
            printf("deque_afo_%d: pa:%p_%p,mag:0x%x,time:0x%lx,tg size:%dx%d\n",i,\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                (MUINT8*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_us,\
                rQBufInfo.vBufInfo.at(i).img_w, rQBufInfo.vBufInfo.at(i).img_h);
            printf("##############################\n");
        }

    }
    return ret;
}

int TEST_CAMIO::Stop_cam(void)
{
    MINT32 ret = 0;

    for(UINT32 i=0;i<2;i++){
        if(1){//(g_pDrvCam[i]){
            if(i==0)
                g_pDrvCam[i] = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
            else
                g_pDrvCam[i] = (IspDrvImp*)IspDrvImp::createInstance(CAM_B);
            g_pDrvCam[i]->init("stop_cam");

            g_pDrvCam[i]->writeReg(0x0500,0x0);
            g_pDrvCam[i]->writeReg(0x0504,0x0);
            g_pDrvCam[i]->writeReg(0x0004,0x0);
            g_pDrvCam[i]->writeReg(0x0008,0x0);

            g_pDrvCam[i]->writeReg(0x0040,0x1);
            while(g_pDrvCam[i]->readReg(0x0040) != 0x2){
                printf("reseting...\n");
            }
            g_pDrvCam[i]->writeReg(0x0040,0x4);
            g_pDrvCam[i]->writeReg(0x0040,0x0);

            g_pDrvCam[i]->uninit("stop_cam");
            g_pDrvCam[i]->destroyInstance();
            g_pDrvCam[i] = NULL;
        }
    }
    return ret;
}



int TEST_CAMIO::Case_0(void)
{

    int ret = 0, cget = 0;

    MUINT32 tg_size_w,tg_size_h;
    MUINT32 rrz_size_w, rrz_size_h;
    vector<PortInfo const*> vCamIOInPorts;
    vector<PortInfo const*> vCamIOOutPorts;
    vector<PortInfo const*> dummy;
    vector<PortInfo const*> vSTTOutPorts;
    IMemDrv* mpImemDrv=NULL;
    capibility CamInfo;
    tCAM_rst rst;
    MUINTPTR org_va_imgo,org_pa_imgo;
    MUINTPTR org_va_rrzo,org_pa_rrzo;
    MUINTPTR org_va_afo,org_pa_afo;
    MUINTPTR org_va_eiso,org_pa_eiso;
    const char Name[32] = {"case_0"};
    CAMIO_Func func;
    func.Raw = 0;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    TG_SIZE(this->Source_Size,&tg_size_w,&tg_size_h);
    //
    rrz_size_w = tg_size_w>>this->rrz_out_size_shift;
    rrz_size_h = tg_size_h>>this->rrz_out_size_shift;


    for(MUINT32 i=0;i<(BUF_DEPTH* (this->m_subSample+1));i++){
        //
        if(this->bypass_imgo == MFALSE){
            if(this->buf_imgo == NULL){
                this->buf_imgo = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_imgo_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
            }
            this->buf_imgo[i].useNoncache = MTRUE;
            this->buf_imgo_fh[i].useNoncache = MTRUE;

            this->buf_imgo[i].size = tg_size_w*tg_size_h*10 / 8;//10bit case
            mpImemDrv->allocVirtBuf(&this->buf_imgo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo[i]);

            //
            this->buf_imgo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_imgo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo_fh[i]);
            //keep org address for release buf
            org_va_imgo = this->buf_imgo_fh[i].virtAddr;
            org_pa_imgo = this->buf_imgo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_imgo_fh[i].virtAddr = ((this->buf_imgo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_imgo_fh[i].phyAddr = ((this->buf_imgo_fh[i].phyAddr + 15) / 16 * 16);
        }
        if(this->bypass_rrzo == MFALSE){
            if(this->buf_rrzo == NULL){
                this->buf_rrzo = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_rrzo_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
            }
            this->buf_rrzo[i].useNoncache = MTRUE;
            this->buf_rrzo_fh[i].useNoncache = MTRUE;

            this->buf_rrzo[i].size = rrz_size_w*rrz_size_h*10 / 8 * 3 / 2;//10bit  FG case
            mpImemDrv->allocVirtBuf(&this->buf_rrzo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_rrzo[i]);

            //
            this->buf_rrzo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_rrzo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_rrzo_fh[i]);
            //keep org address for release buf
            org_va_rrzo = this->buf_rrzo_fh[i].virtAddr;
            org_pa_rrzo = this->buf_rrzo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_rrzo_fh[i].virtAddr = ((this->buf_rrzo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_rrzo_fh[i].phyAddr = ((this->buf_rrzo_fh[i].phyAddr + 15) / 16 * 16);
        }

        if(this->bypass_eiso == MFALSE){
            if(this->buf_eiso == NULL){
                this->buf_eiso = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_eiso_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
            }
            this->buf_eiso[i].useNoncache = MTRUE;
            this->buf_eiso_fh[i].useNoncache = MTRUE;

            this->buf_eiso[i].size = 0x100;//10bit case
            mpImemDrv->allocVirtBuf(&this->buf_eiso[i]);
            mpImemDrv->mapPhyAddr(&this->buf_eiso[i]);

            //
            this->buf_eiso_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_eiso_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_eiso_fh[i]);
            //keep org address for release buf
            org_va_eiso = this->buf_eiso_fh[i].virtAddr;
            org_pa_eiso = this->buf_eiso_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_eiso_fh[i].virtAddr = ((this->buf_eiso_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_eiso_fh[i].phyAddr = ((this->buf_eiso_fh[i].phyAddr + 15) / 16 * 16);
        }
    }

    //stt mem allocate
    for(MUINT32 i=0;i<BUF_DEPTH;i++){
        //
        if(this->bypass_af == MFALSE){
            this->buf_afo[i].useNoncache = MTRUE;
            this->buf_afo_fh[i].useNoncache = MTRUE;

            this->buf_afo[i].size = 64*16*64;//af size, just a meaningless value bigger than the window size in af_cfg()
            mpImemDrv->allocVirtBuf(&this->buf_afo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_afo[i]);

            //
            this->buf_afo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_afo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_afo_fh[i]);
            //keep org address for release buf
            org_va_afo = this->buf_afo_fh[i].virtAddr;
            org_pa_afo = this->buf_afo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_afo_fh[i].virtAddr = ((this->buf_afo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_afo_fh[i].phyAddr = ((this->buf_afo_fh[i].phyAddr + 15) / 16 * 16);
        }

    }

    if((pCamIo = ICamIOPipe::createInstance((MINT8 const*)Name,this->Surce,ICamIOPipe::CAMIO)) == NULL){
        ret = 1;
        goto EXIT;
    }

    if(pCamIo->init() == MFALSE){
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }

    //stt pipe
    if(this->bypass_af == MFALSE){
        this->pTuning->AF_START();
        if((pSttIO = ICamIOPipe::createInstance((MINT8 const*)Name,this->Surce,ICamIOPipe::STTIO)) == NULL){
            ret = 1;
            goto EXIT;
        }

        if(pSttIO->init() == MFALSE){
            pSttIO->destroyInstance();
            ret = 1;
            goto EXIT;
        }

    }


    //
    if(this->Surce == TG_A){
        this->tgi.index       = EPortIndex_TG1I;
    }else{
        this->tgi.index       = EPortIndex_TG2I;
    }
    this->tgi.ePxlMode    = this->TG_PixMode;
    this->tgi.eImgFmt     = NSCam::eImgFmt_BAYER10;
    this->tgi.eRawPxlID   = ERawPxlID_Gb;
    this->tgi.u4ImgWidth  = tg_size_w;
    this->tgi.u4ImgHeight = tg_size_h;
    this->tgi.type        = EPortType_Sensor;
    this->tgi.inout       = EPortDirection_In;
    this->tgi.tgFps       = 1;
    this->tgi.crop1.x        = 0;
    this->tgi.crop1.y        = 0;
    this->tgi.crop1.floatX   = 0;
    this->tgi.crop1.floatY   = 0;
    this->tgi.crop1.w        = tg_size_w;
    this->tgi.crop1.h        = tg_size_h;
    this->tgi.tTimeClk    = 60;  //take 6mhz for example.
    vCamIOInPorts.push_back(&tgi);

    if(this->bypass_imgo == MFALSE){
    //
    this->imgo.index          = EPortIndex_IMGO;
    this->imgo.eImgFmt        = NSCam::eImgFmt_BAYER10;
    this->imgo.u4PureRaw      = MFALSE;
    this->imgo.u4PureRawPak   = MTRUE;
    this->imgo.u4ImgWidth     = tg_size_w;
    this->imgo.u4ImgHeight    = tg_size_h;
    this->imgo.crop1.x        = 0;
    this->imgo.crop1.y        = 0;
    this->imgo.crop1.floatX   = 0;
    this->imgo.crop1.floatY   = 0;
    this->imgo.crop1.w        = tg_size_w;
    this->imgo.crop1.h        = tg_size_h;
    this->imgo.u4Stride[ePlane_1st] = this->buf_imgo[0].size / tg_size_h;

    //prepare for enque if needed
    this->imgo_crop.x = this->imgo_crop.y = this->imgo_crop.floatX = this->imgo_crop.floatY = 0;
    this->imgo_crop.w = tg_size_w;
    this->imgo_crop.h = tg_size_h;

    if (CamInfo.GetCapibility(
            this->imgo.index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->imgo.eImgFmt, this->imgo.u4ImgWidth, ePixMode_NONE),
            rst, E_CAM_UNKNOWNN) == MFALSE) {
        printf("##############################\n");
        printf("error:fmt(0x%x) | dma(0x%x) err\n",this->imgo.eImgFmt,this->imgo.index);
        printf("##############################\n");
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }
    else {
        if(rst.stride_byte[0] != this->imgo.u4Stride[ePlane_1st]){
            printf("cur dma(%d) stride:0x%x, recommanded:0x%x",this->imgo.index,this->imgo.u4Stride[ePlane_1st],rst.stride_byte[0]);
        }
    }
    this->imgo.u4Stride[ePlane_2nd] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
    this->imgo.u4Stride[ePlane_3rd] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
    this->imgo.type   = EPortType_Memory;
    this->imgo.inout  = EPortDirection_Out;
    vCamIOOutPorts.push_back(&this->imgo);
    }

    if(this->bypass_rrzo == MFALSE){
    //
    this->rrzo.index          = EPortIndex_RRZO;
    this->rrzo.eImgFmt        = NSCam::eImgFmt_FG_BAYER10;
    this->rrzo.u4PureRaw      = MFALSE;
    this->rrzo.u4PureRawPak   = MFALSE;
    this->rrzo.u4ImgWidth     = rrz_size_w;
    this->rrzo.u4ImgHeight    = rrz_size_h;
    this->rrzo.crop1.x        = 0;
    this->rrzo.crop1.y        = 0;
    this->rrzo.crop1.floatX   = 0;
    this->rrzo.crop1.floatY   = 0;
    this->rrzo.crop1.w        = tg_size_w;
    this->rrzo.crop1.h        = tg_size_h;
    this->rrzo.u4Stride[ePlane_1st] = this->buf_rrzo[0].size / rrz_size_h;
    //prepare for enque if needed
    this->rrzo_crop.x = this->rrzo_crop.y = this->rrzo_crop.floatX = this->rrzo_crop.floatY = 0;
    this->rrzo_crop.w = tg_size_w;
    this->rrzo_crop.h = tg_size_h;

    this->rrzo_resize.tar_w = rrz_size_w;
    this->rrzo_resize.tar_h = rrz_size_h;

    if (CamInfo.GetCapibility(
            this->rrzo.index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->rrzo.eImgFmt, this->rrzo.u4ImgWidth, ePixMode_NONE),
            rst, E_CAM_UNKNOWNN) == MFALSE) {
        printf("##############################\n");
        printf("error:fmt(0x%x) | dma(0x%x) err\n",this->rrzo.eImgFmt,this->rrzo.index);
        printf("##############################\n");
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }
    else {
        if(rst.stride_byte[0] != this->rrzo.u4Stride[ePlane_1st]){
            printf("cur dma(%d) stride:0x%x, recommanded:0x%x",this->rrzo.index,this->rrzo.u4Stride[ePlane_1st],rst.stride_byte[0]);
        }
    }
    this->rrzo.u4Stride[ePlane_2nd] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
    this->rrzo.u4Stride[ePlane_3rd] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
    this->rrzo.type   = EPortType_Memory;
    this->rrzo.inout  = EPortDirection_Out;
    vCamIOOutPorts.push_back(&this->rrzo);
    }
    func.Bits.SUBSAMPLE = this->m_subSample;
    if(pCamIo->configPipe(vCamIOInPorts, vCamIOOutPorts, &func) == MFALSE){
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }


    if(this->bypass_eiso == MFALSE){
        if(pCamIo->sendCommand(EPIPECmd_ALLOC_UNI,0,0,0) == MTRUE){
            pCamIo->sendCommand(EPIPECmd_SET_EIS_CBFP,(MINTPTR)&this->m_eis,0,0);
            pCamIo->sendCommand(EPIPECmd_SET_SGG2_CBFP,(MINTPTR)&this->m_sgg2,0,0);
        }
        else{
            this->bypass_eiso = MTRUE;
            for(MUINT32 i=0;i<(BUF_DEPTH* (this->m_subSample + 1) );i++){
                //
                mpImemDrv->freeVirtBuf(&this->buf_eiso[i]);
                //
                this->buf_eiso_fh[i].phyAddr = org_pa_eiso;
                this->buf_eiso_fh[i].virtAddr = org_va_eiso;
                mpImemDrv->freeVirtBuf(&this->buf_eiso_fh[i]);

            }
            free(this->buf_eiso);
            free(buf_eiso_fh);
        }
    }

    if(this->bypass_af == MFALSE){
        //
        list<MUINTPTR> ImgPA_L;
        list<MUINTPTR> FHPA_L;
        this->afo.index = EPortIndex_AFO;
        vSTTOutPorts.push_back(&this->afo);
        func.Bits.SUBSAMPLE = this->m_subSample;
        if(pSttIO->configPipe(dummy, vSTTOutPorts, &func) == MFALSE){
            pSttIO->uninit();
            pSttIO->destroyInstance();
            ret = 1;
            goto EXIT;
        }

        //cfg img/frame header PA
        for(MUINT32 i=0;i<BUF_DEPTH;i++){
            ImgPA_L.push_back(this->buf_afo[i].phyAddr);
            FHPA_L.push_back(this->buf_afo_fh[i].phyAddr);
        }

        if(pSttIO->sendCommand(EPIPECmd_SET_STT_BA,(MINTPTR)this->afo.index,(MINTPTR)&ImgPA_L,(MINTPTR)&FHPA_L) == MFALSE){
            pSttIO->uninit();
            pSttIO->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }




    if(this->bEnque_before_start == MTRUE){
        //
        if(this->enque()){
            printf("##############################\n");
            printf("error:enque fail\n");
            printf("##############################\n");
            pCamIo->uninit();
            pCamIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }
    //
    if(this->bypass_af == MFALSE){
        if(pSttIO->start() == MFALSE){
            ret =1 ;
            //
            pSttIO->uninit();
            pSttIO->destroyInstance();
            //
            pCamIo->uninit();
            pCamIo->destroyInstance();
            goto EXIT;
        }
    }
    //
    if(pCamIo->start() == MFALSE){
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }

    //
    if(this->bEnque_before_start == MTRUE){
        if(this->deque()){
            printf("##############################\n");
            printf("error:deque fail\n");
            printf("##############################\n");
            pCamIo->stop();
            pCamIo->uninit();
            pCamIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }
    //
    for(MUINT32 i=0;i<5;i++){
        Irq_t irq;
        irq.Type = Irq_t::_CLEAR_WAIT;
        irq.StatusType = Irq_t::_SIGNAL_INT_;
        irq.Status = Irq_t::_SOF_;
        irq.UserKey = 0;
        irq.Timeout = 1000000;

        printf("##############################\n");
        printf("wait sof_%d\n",i);
        printf("##############################\n");
        if(this->bypass_thread1 == MFALSE){
            printf("##############################\n");
            printf("check OB value:0x%x\n",pCamIo->ReadReg(0x5f0));
            printf("##############################\n");
        }
        if(pCamIo->irq(&irq) == MFALSE){
            printf("##############################\n");
            printf("error:wait sof fail\n");
            printf("##############################\n");
            pCamIo->stop();
            pCamIo->uninit();
            pCamIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
        if(this->enque()){
            printf("##############################\n");
            printf("error:enque fail\n");
            printf("##############################\n");
            pCamIo->stop();
            pCamIo->uninit();
            pCamIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
        if(this->deque()){
            printf("##############################\n");
            printf("error:deque fail\n");
            printf("##############################\n");
            pCamIo->stop();
            pCamIo->uninit();
            pCamIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }

        printf("###################\n");
        printf("###################\n");
        printf("###################\n");
        printf("###################\n");
        printf("###################\n");
        printf("###################\n");
        printf("###################\n");
        cget = getchar();
    }

    if(this->bypass_af == MFALSE){

        pSttIO->stop();
        pSttIO->uninit();
        pSttIO->destroyInstance();
    }

    pCamIo->stop();
    pCamIo->uninit();
    pCamIo->destroyInstance();


EXIT:
    if(this->bypass_af == MFALSE)
        this->pTuning->AF_STOP();
    //
    for(MUINT32 i=0;i<(BUF_DEPTH * (this->m_subSample + 1) );i++){
        if(this->bypass_rrzo == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_rrzo[i]);
            //
            this->buf_rrzo_fh[i].phyAddr = org_pa_rrzo;
            this->buf_rrzo_fh[i].virtAddr = org_va_rrzo;
            mpImemDrv->freeVirtBuf(&this->buf_rrzo_fh[i]);

        }
        if(this->bypass_imgo == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_imgo[i]);
            //
            this->buf_imgo_fh[i].phyAddr = org_pa_imgo;
            this->buf_imgo_fh[i].virtAddr = org_va_imgo;
            mpImemDrv->freeVirtBuf(&this->buf_imgo_fh[i]);
        }


        if(this->bypass_eiso == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_eiso[i]);
            //
            this->buf_eiso_fh[i].phyAddr = org_pa_eiso;
            this->buf_eiso_fh[i].virtAddr = org_va_eiso;
            mpImemDrv->freeVirtBuf(&this->buf_eiso_fh[i]);
        }
    }
    if(this->bypass_rrzo == MFALSE){
        free(buf_rrzo);
        free(buf_rrzo_fh);
    }
    if(this->bypass_imgo == MFALSE){
        free(buf_imgo);
        free(buf_imgo_fh);
    }
    if(this->bypass_eiso == MFALSE){
        free(buf_eiso);
        free(buf_eiso_fh);
    }

    //stt
    for(MUINT32 i=0;i<BUF_DEPTH;i++){
        if(this->bypass_af == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_afo[i]);
            //
            this->buf_afo_fh[i].phyAddr = org_pa_afo;
            this->buf_afo_fh[i].virtAddr = org_va_afo;
            mpImemDrv->freeVirtBuf(&this->buf_afo_fh[i]);

        }
    }

    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //
    return ret;
}

static void*    RunningThread(void *arg)
{
    int ret = 0;
    TEST_CAMIO *_this = reinterpret_cast<TEST_CAMIO*>(arg);
    ret = _this->Case_0();

    //
    if(ret)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);
}

int TEST_CAMIO::Case_1(void)//inifinite loop
{

    int ret = 0;

    MUINT32 tg_size_w,tg_size_h;
    MUINT32 rrz_size_w, rrz_size_h;
    vector<PortInfo const*> vCamIOInPorts;
    vector<PortInfo const*> vCamIOOutPorts;
    vector<PortInfo const*> dummy;
    vector<PortInfo const*> vSTTOutPorts;
    IMemDrv* mpImemDrv=NULL;
    capibility CamInfo;
    tCAM_rst rst;
    MUINTPTR org_va_imgo,org_pa_imgo;
    MUINTPTR org_va_rrzo,org_pa_rrzo;
    MUINTPTR org_va_afo,org_pa_afo;
    MUINTPTR org_va_eiso,org_pa_eiso;
    const char Name[32] = {"case_1"};
    CAMIO_Func func;
    func.Raw = 0;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    TG_SIZE(this->Source_Size,&tg_size_w,&tg_size_h);
    //
    rrz_size_w = tg_size_w>>this->rrz_out_size_shift;
    rrz_size_h = tg_size_h>>this->rrz_out_size_shift;


    for(MUINT32 i=0;i<(BUF_DEPTH* (this->m_subSample+1));i++){
        //
        if(this->bypass_imgo == MFALSE){
            if(this->buf_imgo == NULL){
                this->buf_imgo = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_imgo_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
            }
            this->buf_imgo[i].useNoncache = MTRUE;
            this->buf_imgo_fh[i].useNoncache = MTRUE;

            this->buf_imgo[i].size = tg_size_w*tg_size_h*10 / 8;//10bit case
            mpImemDrv->allocVirtBuf(&this->buf_imgo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo[i]);

            //
            this->buf_imgo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_imgo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo_fh[i]);
            //keep org address for release buf
            org_va_imgo = this->buf_imgo_fh[i].virtAddr;
            org_pa_imgo = this->buf_imgo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_imgo_fh[i].virtAddr = ((this->buf_imgo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_imgo_fh[i].phyAddr = ((this->buf_imgo_fh[i].phyAddr + 15) / 16 * 16);
        }
        if(this->bypass_rrzo == MFALSE){
            if(this->buf_rrzo == NULL){
                this->buf_rrzo = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_rrzo_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
            }
            this->buf_rrzo[i].useNoncache = MTRUE;
            this->buf_rrzo_fh[i].useNoncache = MTRUE;

            this->buf_rrzo[i].size = rrz_size_w*rrz_size_h*10 / 8 * 3 / 2;//10bit  FG case
            mpImemDrv->allocVirtBuf(&this->buf_rrzo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_rrzo[i]);

            //
            this->buf_rrzo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_rrzo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_rrzo_fh[i]);
            //keep org address for release buf
            org_va_rrzo = this->buf_rrzo_fh[i].virtAddr;
            org_pa_rrzo = this->buf_rrzo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_rrzo_fh[i].virtAddr = ((this->buf_rrzo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_rrzo_fh[i].phyAddr = ((this->buf_rrzo_fh[i].phyAddr + 15) / 16 * 16);
        }

        if(this->bypass_eiso == MFALSE){
            if(this->buf_eiso == NULL){
                this->buf_eiso = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_eiso_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
            }
            this->buf_eiso[i].useNoncache = MTRUE;
            this->buf_eiso_fh[i].useNoncache = MTRUE;

            this->buf_eiso[i].size = 0x100;//10bit case
            mpImemDrv->allocVirtBuf(&this->buf_eiso[i]);
            mpImemDrv->mapPhyAddr(&this->buf_eiso[i]);

            //
            this->buf_eiso_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_eiso_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_eiso_fh[i]);
            //keep org address for release buf
            org_va_eiso = this->buf_eiso_fh[i].virtAddr;
            org_pa_eiso = this->buf_eiso_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_eiso_fh[i].virtAddr = ((this->buf_eiso_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_eiso_fh[i].phyAddr = ((this->buf_eiso_fh[i].phyAddr + 15) / 16 * 16);
        }
    }

    //stt mem allocate
    for(MUINT32 i=0;i<BUF_DEPTH;i++){
        //
        if(this->bypass_af == MFALSE){
            this->buf_afo[i].useNoncache = MTRUE;
            this->buf_afo_fh[i].useNoncache = MTRUE;

            this->buf_afo[i].size = 64*16*64;//af size, just a meaningless value bigger than the window size in af_cfg()
            mpImemDrv->allocVirtBuf(&this->buf_afo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_afo[i]);

            //
            this->buf_afo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            mpImemDrv->allocVirtBuf(&this->buf_afo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_afo_fh[i]);
            //keep org address for release buf
            org_va_afo = this->buf_afo_fh[i].virtAddr;
            org_pa_afo = this->buf_afo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_afo_fh[i].virtAddr = ((this->buf_afo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_afo_fh[i].phyAddr = ((this->buf_afo_fh[i].phyAddr + 15) / 16 * 16);
        }

    }

    if((pCamIo = ICamIOPipe::createInstance((MINT8 const*)Name,this->Surce,ICamIOPipe::CAMIO)) == NULL){
        printf("pCamIo==NULL");
        ret = 1;
        goto EXIT;
    }

    if(pCamIo->init() == MFALSE){
        pCamIo->destroyInstance();
        printf("pCamIo init fail");
        ret = 1;
        goto EXIT;
    }

    //stt pipe
    if(this->bypass_af == MFALSE){
        this->pTuning->AF_START();
        if((pSttIO = ICamIOPipe::createInstance((MINT8 const*)Name,this->Surce,ICamIOPipe::STTIO)) == NULL){
            ret = 1;
            goto EXIT;
        }

        if(pSttIO->init() == MFALSE){
            pSttIO->destroyInstance();
            ret = 1;
            goto EXIT;
        }

    }


    //
    if(this->Surce == TG_A)
        this->tgi.index       = EPortIndex_TG1I;
    else
        this->tgi.index       = EPortIndex_TG2I;

    this->tgi.ePxlMode    = this->TG_PixMode;
    this->tgi.eImgFmt     = NSCam::eImgFmt_BAYER10;
    this->tgi.eRawPxlID   = ERawPxlID_Gb;
    this->tgi.u4ImgWidth  = tg_size_w;
    this->tgi.u4ImgHeight = tg_size_h;
    this->tgi.type        = EPortType_Sensor;
    this->tgi.inout       = EPortDirection_In;
    this->tgi.tgFps       = 1;
    this->tgi.crop1.x        = 0;
    this->tgi.crop1.y        = 0;
    this->tgi.crop1.floatX   = 0;
    this->tgi.crop1.floatY   = 0;
    this->tgi.crop1.w        = tg_size_w;
    this->tgi.crop1.h        = tg_size_h;
    this->tgi.tTimeClk    = 60;  //take 6mhz for example.
    vCamIOInPorts.push_back(&tgi);

    if(this->bypass_imgo == MFALSE){
    //
    this->imgo.index          = EPortIndex_IMGO;
    this->imgo.eImgFmt        = NSCam::eImgFmt_BAYER10;
    this->imgo.u4PureRaw      = MFALSE;
    this->imgo.u4PureRawPak   = MTRUE;
    this->imgo.u4ImgWidth     = tg_size_w;
    this->imgo.u4ImgHeight    = tg_size_h;
    this->imgo.crop1.x        = 0;
    this->imgo.crop1.y        = 0;
    this->imgo.crop1.floatX   = 0;
    this->imgo.crop1.floatY   = 0;
    this->imgo.crop1.w        = tg_size_w;
    this->imgo.crop1.h        = tg_size_h;
    this->imgo.u4Stride[ePlane_1st] = this->buf_imgo[0].size / tg_size_h;

    //prepare for enque if needed
    this->imgo_crop.x = this->imgo_crop.y = this->imgo_crop.floatX = this->imgo_crop.floatY = 0;
    this->imgo_crop.w = tg_size_w;
    this->imgo_crop.h = tg_size_h;

    if (CamInfo.GetCapibility(
            this->imgo.index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->imgo.eImgFmt, this->imgo.u4ImgWidth, ePixMode_NONE),
            rst, E_CAM_UNKNOWNN) == MFALSE) {
        printf("##############################\n");
        printf("error:fmt(0x%x) | dma(0x%x) err\n",this->imgo.eImgFmt,this->imgo.index);
        printf("##############################\n");
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }
    else {
        if(rst.stride_byte[0] != this->imgo.u4Stride[ePlane_1st]){
            printf("cur dma(%d) stride:0x%x, recommanded:0x%x",this->imgo.index,this->imgo.u4Stride[ePlane_1st],rst.stride_byte[0]);
        }
    }
    this->imgo.u4Stride[ePlane_2nd] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
    this->imgo.u4Stride[ePlane_3rd] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
    this->imgo.type   = EPortType_Memory;
    this->imgo.inout  = EPortDirection_Out;
    vCamIOOutPorts.push_back(&this->imgo);
    }

    if(this->bypass_rrzo == MFALSE){
    //
    this->rrzo.index          = EPortIndex_RRZO;
    this->rrzo.eImgFmt        = NSCam::eImgFmt_FG_BAYER10;
    this->rrzo.u4PureRaw      = MFALSE;
    this->rrzo.u4PureRawPak   = MFALSE;
    this->rrzo.u4ImgWidth     = rrz_size_w;
    this->rrzo.u4ImgHeight    = rrz_size_h;
    this->rrzo.crop1.x        = 0;
    this->rrzo.crop1.y        = 0;
    this->rrzo.crop1.floatX   = 0;
    this->rrzo.crop1.floatY   = 0;
    this->rrzo.crop1.w        = tg_size_w;
    this->rrzo.crop1.h        = tg_size_h;
    this->rrzo.u4Stride[ePlane_1st] = this->buf_rrzo[0].size / rrz_size_h;
    //prepare for enque if needed
    this->rrzo_crop.x = this->rrzo_crop.y = this->rrzo_crop.floatX = this->rrzo_crop.floatY = 0;
    this->rrzo_crop.w = tg_size_w;
    this->rrzo_crop.h = tg_size_h;

    this->rrzo_resize.tar_w = rrz_size_w;
    this->rrzo_resize.tar_h = rrz_size_h;

    if (CamInfo.GetCapibility(
            this->rrzo.index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->rrzo.eImgFmt, this->rrzo.u4ImgWidth, ePixMode_NONE),
            rst, E_CAM_UNKNOWNN) == MFALSE) {
        printf("##############################\n");
        printf("error:fmt(0x%x) | dma(0x%x) err\n",this->rrzo.eImgFmt,this->rrzo.index);
        printf("##############################\n");
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }
    else {
        if(rst.stride_byte[0] != this->rrzo.u4Stride[ePlane_1st]){
            printf("cur dma(%d) stride:0x%x, recommanded:0x%x",this->rrzo.index,this->rrzo.u4Stride[ePlane_1st],rst.stride_byte[0]);
        }
    }
    this->rrzo.u4Stride[ePlane_2nd] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
    this->rrzo.u4Stride[ePlane_3rd] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
    this->rrzo.type   = EPortType_Memory;
    this->rrzo.inout  = EPortDirection_Out;
    vCamIOOutPorts.push_back(&this->rrzo);
    }

    func.Bits.SUBSAMPLE = this->m_subSample;
    if(pCamIo->configPipe(vCamIOInPorts, vCamIOOutPorts, &func) == MFALSE){
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }


    if(this->bypass_eiso == MFALSE){
        if(pCamIo->sendCommand(EPIPECmd_ALLOC_UNI,0,0,0) == MTRUE){
            pCamIo->sendCommand(EPIPECmd_SET_EIS_CBFP,(MINTPTR)&this->m_eis,0,0);
            pCamIo->sendCommand(EPIPECmd_SET_SGG2_CBFP,(MINTPTR)&this->m_sgg2,0,0);
        }
        else{
            this->bypass_eiso = MTRUE;
            for(MUINT32 i=0;i<(BUF_DEPTH* (this->m_subSample + 1) );i++){
                //
                mpImemDrv->freeVirtBuf(&this->buf_eiso[i]);
                //
                this->buf_eiso_fh[i].phyAddr = org_pa_eiso;
                this->buf_eiso_fh[i].virtAddr = org_va_eiso;
                mpImemDrv->freeVirtBuf(&this->buf_eiso_fh[i]);

            }
            free(this->buf_eiso);
            free(buf_eiso_fh);
        }
    }

    if(this->bypass_af == MFALSE){
        //
        list<MUINTPTR> ImgPA_L;
        list<MUINTPTR> FHPA_L;
        this->afo.index = EPortIndex_AFO;
        vSTTOutPorts.push_back(&this->afo);
        func.Bits.SUBSAMPLE = this->m_subSample;
        if(pSttIO->configPipe(dummy, vSTTOutPorts, &func) == MFALSE){
            pSttIO->uninit();
            pSttIO->destroyInstance();
            ret = 1;
            goto EXIT;
        }

        //cfg img/frame header PA
        for(MUINT32 i=0;i<BUF_DEPTH;i++){
            ImgPA_L.push_back(this->buf_afo[i].phyAddr);
            FHPA_L.push_back(this->buf_afo_fh[i].phyAddr);
        }

        if(pSttIO->sendCommand(EPIPECmd_SET_STT_BA,(MINTPTR)this->afo.index,(MINTPTR)&ImgPA_L,(MINTPTR)&FHPA_L) == MFALSE){
            pSttIO->uninit();
            pSttIO->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }




    if(this->bEnque_before_start == MTRUE){
        //
        if(this->enque()){
            printf("##############################\n");
            printf("error:enque fail\n");
            printf("##############################\n");
            pCamIo->uninit();
            pCamIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }
    //
    if(this->bypass_af == MFALSE){
        if(pSttIO->start() == MFALSE){
            ret =1 ;
            //
            pSttIO->uninit();
            pSttIO->destroyInstance();
            //
            pCamIo->uninit();
            pCamIo->destroyInstance();
            goto EXIT;
        }
    }
    //
    if(pCamIo->start() == MFALSE){
        pCamIo->uninit();
        pCamIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }

    //
    if(this->bEnque_before_start == MTRUE){
        if(this->deque()){
            printf("##############################\n");
            printf("error:deque fail\n");
            printf("##############################\n");
            pCamIo->stop();
            pCamIo->uninit();
            pCamIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }
    //
    if(this->bthread_start == MTRUE){
        MUINT32 i=0;
        ::sem_post(&this->m_semP1Start);
        while(this->bthread_start == MTRUE){
            Irq_t irq;
            irq.Type = Irq_t::_CLEAR_WAIT;
            irq.StatusType = Irq_t::_SIGNAL_INT_;
            irq.Status = Irq_t::_SOF_;
            irq.UserKey = 0;
            irq.Timeout = 1000000;

            printf("##############################\n");
            printf("wait sof_%d\n",i);
            printf("##############################\n");
            if(pCamIo->irq(&irq) == MFALSE){
                printf("##############################\n");
                printf("error:wait sof fail\n");
                printf("##############################\n");
                pCamIo->stop();
                pCamIo->uninit();
                pCamIo->destroyInstance();
                ret = 1;
                goto EXIT;
            }
            if(this->enque()){
                printf("##############################\n");
                printf("error:enque fail\n");
                printf("##############################\n");
                pCamIo->stop();
                pCamIo->uninit();
                pCamIo->destroyInstance();
                ret = 1;
                goto EXIT;
            }
            if(this->deque()){
                printf("##############################\n");
                printf("error:deque fail\n");
                printf("##############################\n");
                pCamIo->stop();
                pCamIo->uninit();
                pCamIo->destroyInstance();
                ret = 1;
                goto EXIT;
            }
            i++;
        }
    }

    if(this->bypass_af == MFALSE){

        pSttIO->stop();
        pSttIO->uninit();
        pSttIO->destroyInstance();
    }

    pCamIo->stop();
    pCamIo->uninit();
    pCamIo->destroyInstance();


EXIT:
    ::sem_post(&this->m_semP1Start);
    //
    if(this->bypass_af == MFALSE)
        this->pTuning->AF_STOP();
    //
    for(MUINT32 i=0;i<(BUF_DEPTH * (this->m_subSample + 1) );i++){
        if(this->bypass_rrzo == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_rrzo[i]);
            //
            this->buf_rrzo_fh[i].phyAddr = org_pa_rrzo;
            this->buf_rrzo_fh[i].virtAddr = org_va_rrzo;
            mpImemDrv->freeVirtBuf(&this->buf_rrzo_fh[i]);

        }
        if(this->bypass_imgo == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_imgo[i]);
            //
            this->buf_imgo_fh[i].phyAddr = org_pa_imgo;
            this->buf_imgo_fh[i].virtAddr = org_va_imgo;
            mpImemDrv->freeVirtBuf(&this->buf_imgo_fh[i]);
        }


        if(this->bypass_eiso == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_eiso[i]);
            //
            this->buf_eiso_fh[i].phyAddr = org_pa_eiso;
            this->buf_eiso_fh[i].virtAddr = org_va_eiso;
            mpImemDrv->freeVirtBuf(&this->buf_eiso_fh[i]);
        }
    }
    if(this->bypass_rrzo == MFALSE){
        free(buf_rrzo);
        free(buf_rrzo_fh);
    }
    if(this->bypass_imgo == MFALSE){
        free(buf_imgo);
        free(buf_imgo_fh);
    }
    if(this->bypass_eiso == MFALSE){
        free(buf_eiso);
        free(buf_eiso_fh);
    }

    //stt
    for(MUINT32 i=0;i<BUF_DEPTH;i++){
        if(this->bypass_af == MFALSE){
            //
            mpImemDrv->freeVirtBuf(&this->buf_afo[i]);
            //
            this->buf_afo_fh[i].phyAddr = org_pa_afo;
            this->buf_afo_fh[i].virtAddr = org_va_afo;
            mpImemDrv->freeVirtBuf(&this->buf_afo_fh[i]);

        }
    }

    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //
    return ret;
}

static void*    RunningInfiniteThread(void *arg)
{
    int ret = 0;
    TEST_CAMIO *_this = reinterpret_cast<TEST_CAMIO*>(arg);
    printf("floria\n");
    ret = _this->Case_1();

    //
    if(ret)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);
}


int TEST_TUNING::AF_START(void)
{
    this->tuningMgr = (TuningMgr*)TuningMgr::getInstance(this->sensorIdx);
    this->tuningMgr->init(userName, 1);

    return 0;
}

int TEST_TUNING::AF_CFG(MUINT32 in_h,MUINT32 in_v,MUINT32 mag)
{
#if 1
    MUINT32 tmp=0;
#define AF_WIN_NUM_X  32
#define AF_WIN_NUM_Y  32
    MUINT32 h_size,win_h_size;
    MUINT32 v_size,win_v_size;
    MUINT32 xsize,ysize;
    MUINT32 af_v_avg_lvl,af_v_gonly;
    MUINT32 start_x=0,start_y=0;
    MUINT32 sensorIndex = 0;


    this->tuningMgr->dequeBuffer((MINT32*)&mag);

    this->tuningMgr->updateEngine(eTuningMgrFunc_AF, MTRUE, 0);
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AF_CON, 0, 0);

    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AF_SIZE, in_h, 0);//case for no frontal binning


    //
    af_v_avg_lvl = 0;
    af_v_gonly = 0;

    //AF image wd
    h_size  = in_h;
    v_size = in_v;

    //ofset
    start_x += ((start_x & 0x1)?(0x1):(0));
    start_y += ((start_y & 0x1)?(0x1):(0));
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AF_VLD, (start_y<<16 | start_x), 0);

    //window num
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AF_BLK_1, ((AF_WIN_NUM_Y<<16)|AF_WIN_NUM_X), 0);

    //
    win_h_size = (h_size-start_x) / AF_WIN_NUM_X;
    if(win_h_size > 254)
        win_h_size = 254;
    else{//min constraint
        if((af_v_avg_lvl == 3) && (af_v_gonly == 1)){
            win_h_size = (win_h_size < 32)? (32):(win_h_size);
        }
        else if((af_v_avg_lvl == 3) && (af_v_gonly == 0)){
            win_h_size = (win_h_size < 16)? (16):(win_h_size);
        }
        else if((af_v_avg_lvl == 2) && (af_v_gonly == 1)){
            win_h_size = (win_h_size < 16)? (16):(win_h_size);
        }
        else{
            win_h_size = (win_h_size < 8)? (8):(win_h_size);
        }
    }
    if(af_v_gonly == 1)
        win_h_size = win_h_size/4 * 4;
    else
        win_h_size = win_h_size/2 * 2;

    win_v_size = (v_size-start_y) / AF_WIN_NUM_Y;
    if(win_v_size > 255)
        win_v_size = 255;
    else
        win_v_size = (win_v_size < 1)? (1):(win_v_size);

    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AF_BLK_0, ((win_v_size<<16)|win_h_size), 0);


    //xsize/ysize
    xsize = AF_WIN_NUM_X*16;
    ysize = AF_WIN_NUM_Y;
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AFO_XSIZE, xsize, 0);
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AFO_YSIZE, ysize, 0);
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_AFO_STRIDE, xsize, 0);

    this->tuningMgr->updateEngine(eTuningMgrFunc_SGG1, MTRUE, 0);
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_SGG1_PGN, 0x200, 0);
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_SGG1_GMRC_1, 0xffffffff, 0);
    TUNING_MGR_WRITE_REG_CAM(this->tuningMgr, CAM_SGG1_GMRC_2, 0xffffffff, 0);

    this->tuningMgr->enqueBuffer();
#undef AF_WIN_NUM_X
#undef AF_WIN_NUM_Y
#endif
    return 0;
}

int TEST_TUNING::AF_STOP(void)
{
#if 1

    this->tuningMgr->uninit(userName);
#endif
    return 0;
}



int Test_ImageIo_Cam(int argc, char** argv)
{
    int rst = 0, cget=0;
    char s[16],st[16];
    MUINT32 path, choice;
    TEST_CAMIO* pCamio;
    TEST_CAMIO* pCamio_2;
    void* thread_rst = NULL;
    (void)argc;(void)argv;
    cget = getchar();
    printf("##############################\n");
    printf("case 100:stop cam[for segmentation fault only]\n");
    printf("case 0: single[8m + imgo + rrzo]\n");
    printf("case 1: same as case0, but enque before start\n");
    printf("case 2: single[8m + imgo + rrzo + afo]\n");
    printf("case 3: single[8m + imgo + rrzo + eiso + ae smoothing]\n");
    printf("case 4: dual sensor,no N3D[8m+imgo+rrzo+afo,5m+imgo+rrzo+afo]\n");
    printf("case 21: Twin Mgr: seninf+tg+dmxi: 2, twin: 0, dmxo: 2, dbn: 0, bin: 1, bmxo: 0. rmxo: 0\n");
    printf("case 5: single with subsample=2[8m + imgo + rrzo + eiso + afo]\n");
    printf("case 6: single with subsample=2[based on case5, add thread11 test]\n");
    printf("case 7: twin with subsample=1[imgo/rrzo]\n");
    printf("case 8: twin with subsample=1[imgo/rrzo/eiso]\n");
    printf("case 9: twin with subsample=1[imgo/rrzo/eiso/afo]\n");
    printf("case 10: twin with subsample=1[imgo/rrzo/eiso/afo/ae smoothing]\n");
    printf("case 11: twin with subsample=2[imgo/rrzo]\n");
    printf("case 12: twin with subsample=2[imgo/rrzo/eiso]\n");
    printf("case 13: twin with subsample=2[imgo/rrzo/eiso/afo]\n");
    printf("case 14: twin with subsample=2[imgo/rrzo/eiso/thread11]\n");
    printf("case (Removed) 22: Twin Mgr: seninf+tg+dmxi: 4, twin: 0, dmxo: 4, dbn: 1, bin: 1, bmxo: 0. rmxo: 0\n");
    printf("case 23: Twin Mgr: seninf+tg+dmxi: 1, twin: 1, dmxo: 1, dbn: 0, bin: 0, bmxo: 0. rmxo: 0\n");
    printf("case 24: Twin Mgr: seninf+tg+dmxi: 2, twin: 1, dmxo: 1, dbn: 0, bin: 0, bmxo: 2. rmxo: 2\n");
    printf("case 25: Twin Mgr: seninf+tg+dmxi: 4, twin: 1, dmxo: 2, dbn: 0, bin: 1, bmxo: 2. rmxo: 2\n");
    printf("case 26: Twin Mgr: rrzo out test, seninf+tg+dmxi: 2, twin: 0, dmxo: 2, dbn: 0, bin: 1, bmxo: 0. rmxo: 0\n");
    printf("case 27: twin[imgo + rrzo + afo], seninf+tg+dmxi: 1, twin: 1, dmxo: 1, dbn: 0, bin: 0, bmxo: 0. rmxo: 0\n");
    printf("case 30: [I2C test]Start. single with subsample=2[8m + imgo + rrzo]\n");
    printf("select path\n");
    printf("##############################\n");


    scanf("%s", s);
    path = atoi((const char*)s);
    printf("select path=0x%x\n",path);


    switch(path){
        case 0:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MFALSE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 1:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->Source_Size = E_8M;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 2:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MFALSE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 3:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MTRUE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread1 = MFALSE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 4:
            Seninf_Pattern(2,MFALSE,1);
            pCamio_2 = new TEST_CAMIO(1);
            pCamio_2->Surce = TG_B;
            pCamio_2->TG_PixMode = ePixMode_1;
            pCamio_2->Source_Size = E_5M;
            pCamio_2->bypass_imgo = MFALSE;
            pCamio_2->bypass_rrzo = MFALSE;
            pCamio_2->bypass_af= MFALSE;
            pCamio_2->bypass_eiso = MFALSE;
            pCamio_2->bEnque_before_start = MTRUE;

            pthread_create(&pCamio_2->m_Thread, NULL, RunningThread, pCamio_2);

            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MFALSE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();

            pthread_join(pCamio_2->m_Thread, &thread_rst);
            delete pCamio_2;
            Seninf_Pattern_close();

            break;
        case 5:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MFALSE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 1;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 6:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MFALSE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread11 = MFALSE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 1;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;

        case 7:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MTRUE;
            pCamio->bypass_eiso = MTRUE;
            pCamio->bypass_thread11 = MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 0;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 8:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MTRUE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread11 = MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 0;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 9:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MFALSE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread11 = MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 0;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 10:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MFALSE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread11 = MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->bypass_thread1 = MFALSE;
            pCamio->m_subSample = 0;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 11:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MTRUE;
            pCamio->bypass_eiso = MTRUE;
            pCamio->bypass_thread11 = MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 1;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 12:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MTRUE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread11 = MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 1;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 13:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MFALSE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread11 = MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 1;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 14:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0;
            pCamio->bypass_af= MTRUE;
            pCamio->bypass_eiso = MFALSE;
            pCamio->bypass_thread11 = MFALSE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 1;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 30:
            Seninf_Pattern(1,MFALSE,1);
            //
            if(g_pDrvCam[0]==NULL){
                printf("g_pDrvCam==NULL\n");
                break;
            }
            g_pDrvCam[0]->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only
            g_pDrvCam[0]->writeReg(0x0100, g_pDrvCam[0]->readReg(0x0100) | 0x8);//1A040100, I2C2_EN
            printf("[I2C]N3D_CTL=0x%08x\n",g_pDrvCam[0]->readReg(0x0100));
            g_pDrvCam[0]->setRWMode(ISP_DRV_RW_MMAP);
            //
            pCamio = new TEST_CAMIO(0);
            if(pCamio == NULL)
                printf("new pCamio_I2C fail\n");
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            pCamio->m_subSample = 2;
            //
            ::sem_init(&pCamio->m_semP1Start, 0, 0);
            pthread_create(&pCamio->m_Thread, NULL, RunningInfiniteThread, pCamio);
            //wait for p1 loop start
            ::sem_wait(&pCamio->m_semP1Start);
            //0x1A004500, CAM_A_TG_SEN_MODE, I2C_CQ_EN
            g_pDrvCam[0]->writeReg(0x0500, g_pDrvCam[0]->readReg(0x0500) | 0x00080000);
            printf("[I2C]CAMA_TG_SEN_MODE=0x%08x\n",g_pDrvCam[0]->readReg(0x0500));
            //
            choice = 1;
            while(choice != 0) {
                cget = getchar();
                printf("##############################\n");
                printf("1: trigger I2C\n");
                printf("0: leave test case\n");
                printf("##############################\n");
                scanf("%s", st);
                choice = atoi((const char*)st);
                printf("select path=%d\n",choice);

                if(choice == 0) {
                    printf("### Leave I2C test case ###\n");
                    break;
                }

                printf("### triger I2C ###\n");
                //add i2c cmd here


                //0x1A004560, CAM_A_TG_I2C_CQ_TRIG, bit0
                g_pDrvCam[0]->writeReg(0x0560, 0x1);
            }
            //
            printf("##############################\n");
            printf("stop thread\n");
            printf("##############################\n");
            pCamio->bthread_start = MFALSE;
            pthread_join(pCamio->m_Thread, &thread_rst);
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 21:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
         case 22:
            Seninf_Pattern(1,MFALSE,4);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_4;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 2; // rrz out size is 1/4 tg size
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 23:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M; // Modify line buffer limitation of twin mgr to smaller than 8M for avoiding ion mem not enough issue
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 1; // rrz out size is 1/2 tg size
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 24:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M; // Modify line buffer limitation of twin mgr to smaller than 8M for avoiding ion mem not enough issue
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0; // rrz out size is 1/2 tg size
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 25:
            Seninf_Pattern(1,MFALSE,4);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_4;
            pCamio->Source_Size = E_5M; // Modify line buffer limitation of twin mgr to smaller than 8M for avoiding ion mem not enough issue
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 1; // rrz out size is 1/2 tg size
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 26:
            Seninf_Pattern(1,MFALSE,2);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_2;
            pCamio->Source_Size = E_8M;
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 0; // rrz out size is 1 tg size, rrzo out test
            pCamio->bypass_af= MTRUE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 27:
            Seninf_Pattern(1,MFALSE,1);
            pCamio = new TEST_CAMIO(0);
            pCamio->Surce = TG_A;
            pCamio->TG_PixMode = ePixMode_1;
            pCamio->Source_Size = E_8M; // Modify line buffer limitation of twin mgr to smaller than 8M for avoiding ion mem not enough issue
            pCamio->bypass_imgo = MFALSE;
            pCamio->bypass_rrzo = MFALSE;
            pCamio->rrz_out_size_shift = 1; // rrz out size is 1/2 tg size
            pCamio->bypass_af= MFALSE;
            pCamio->bEnque_before_start = MTRUE;
            rst = pCamio->Case_0();
            delete pCamio;
            Seninf_Pattern_close();
            break;
        case 100:
            pCamio = new TEST_CAMIO(0);
            rst = pCamio->Stop_cam();
            delete pCamio;
            break;
        default:
            break;
    }
    return rst;
}

