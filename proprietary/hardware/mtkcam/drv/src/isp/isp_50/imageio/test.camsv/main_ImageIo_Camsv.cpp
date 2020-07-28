#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//
#include <errno.h>
#include <fcntl.h>

#include <IPipe.h>
#include <ICamIOPipe.h>
#include "isp_drv_cam.h"//for cfg the test pattern of seninf only
#include <android/log.h>

#include <cam_capibility.h>

/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <Cam_Notify_datatype.h>
#include <utils/Thread.h>

#undef LOG_TAG
#define LOG_TAG "Test_ImageIo_Camsv"

#define  LOG__I(...)  do {  printf(__VA_ARGS__); } while(0)
#define  LOG__D(...)  do {  printf(__VA_ARGS__); } while(0)
#define  LOG__D(...)  do {  printf(__VA_ARGS__); } while(0)

using namespace NSImageio;
using namespace NSIspio;

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;
    bool ret = true;

    LOG__I("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        LOG__I("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            LOG__I("failed to write to file [%s]: %s", fname, ::strerror(errno));
            ret = false;
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return ret;
}

IspDrvImp* g_pDrvCamsv[2] = {NULL};

typedef enum{
    E_5M,
    E_8M,
    E_16M
}E_TGSZIE;

static int TG_SIZE(E_TGSZIE size,MUINT32 *pSize_w, MUINT32 *pSize_h)
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
            LOG__I("#####################################\n");
            LOG__I("error:unsupported size:0x%x\n",size);
            LOG__I("#####################################\n");
            *pSize_w = -1;
            *pSize_h = -1;
            return 1;
    }
    return 0;
}


//pix_mode 1: 1pix, 2: 2pix, 4:4pix
//#define PIXEL_MODE_4

static int Seninf_Pattern(MUINT32 cnt, MBOOL bYUV, E_CamPixelMode epixel_mode, MUINT32 _tg_pattern)
{
    MINT32 ret = 0, i = 0;

    g_pDrvCamsv[0] = (IspDrvImp*)IspDrvImp::createInstance(CAMSV_0);
    g_pDrvCamsv[0]->init("test pattern");
    g_pDrvCamsv[0]->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only

    g_pDrvCamsv[0]->writeReg(0x0010, 0); //SENINF_TOP_CAM_MUX_CTRL

    LOG__I("#####################################\n");
    LOG__I("SENINF start in TestPattern Mode !!!![is YUV = %d]\n",bYUV);
    LOG__I("pattern size :%d x %d with %d enabled pattern!!!!\n",0x1f00,0xfa0,cnt);
    LOG__I("pix_mode: %d\n", 1<<epixel_mode);
    LOG__I("#####################################\n");

    LOG__I("%d TM open\n",i+1);
    LOG__I("#####################################\n");

    static MUINT32 mPowerOnTM_2[][2] = {
    //0x200 -> 0x100,0x500
    0x0200, 0x00001001, //SENINF1_CTRL
    //0xd00 -> 0x120,0x520
    0x0D00, 0x96DF1080, //SENINF1_MUX_CTRL
    0x0D04, 0x8000007F, //SENINF1_MUX_INTEN(RST)
    0x0D0c, 0x00000000, //SENINF1_MUX_SIZE(RST)
    0x0D2c, 0x000E2000, //SENINF1_MUX_SPARE(RST)
    0x0D38, 0x00000000, //SENINF1_MUX_CROP(RST)

    //0x600 -> 0x200
    0x0608, 0x002804C1, // TG1_TM_CTL
                            // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
    0x060c, 0x0FA01F00, // TG1_TM_SIZE
    0x0610, 0x00000008, // TG1_TM_CLK
    0x0614, 0x1        // always setting timestamp dividor to 1 for test only

    #if 0
    #define DUAL_CAMSV
    #ifdef DUAL_CAMSV
    0x0D00, 0x00001001, //SENINF4_CTR: Test Model + SENINF_EN
    0x0D20, 0x96DF1080, //SENINF4_MUX_CTRL
    0x0D24, 0x8000007F, //SENINF4_MUX_INTEN(RST)
    0x0D2c, 0x00000000, //SENINF4_MUX_SIZE(RST)
    0x0D4c, 0x000E2000, //SENINF4_MUX_SPARE(RST)
    0x0D58, 0x00000000, //SENINF4_MUX_CROP(RST)
    #endif
    #endif

    };

    g_pDrvCamsv[i]->writeReg(0x0000, 0x00000C00); //SENINF_TOP_CTRL
    g_pDrvCamsv[i]->writeReg(0x0204, g_pDrvCamsv[i]->readReg(0x0204) | 0x02); //TM enable //SENINF1_CTRL_EXT

    for(MUINT32 j=0;j<(sizeof(mPowerOnTM_2)/(sizeof(MUINT32)*2)); j++){
        if( mPowerOnTM_2[j][0]==0x0608 ) {
            mPowerOnTM_2[j][1] &= 0xFFFFFF0F;
            mPowerOnTM_2[j][1] |= (_tg_pattern<<4);
        }
        g_pDrvCamsv[i]->writeReg(mPowerOnTM_2[j][0], mPowerOnTM_2[j][1]);
    }

    // yuv format
    if(1 == bYUV)
    {
        g_pDrvCamsv[i]->writeReg(0x0608, g_pDrvCamsv[i]->readReg(0x0608) | 0x4);
    }

    // pixel mode
    switch (epixel_mode) {
    case ePixMode_1:
        g_pDrvCamsv[i]->writeReg(0x0D00, g_pDrvCamsv[i]->readReg(0x0D00) & ~0x100);
        g_pDrvCamsv[i]->writeReg(0x0D3C, g_pDrvCamsv[i]->readReg(0x0D3C) & ~0x10);
        break;
    case ePixMode_2:
        g_pDrvCamsv[i]->writeReg(0x0D00, g_pDrvCamsv[i]->readReg(0x0D00) | 0x100);
        g_pDrvCamsv[i]->writeReg(0x0D3C, g_pDrvCamsv[i]->readReg(0x0D3C) & ~0x10);
        break;
    case ePixMode_4:
        g_pDrvCamsv[i]->writeReg(0x0D00, g_pDrvCamsv[i]->readReg(0x0D00) & ~0x100);
        g_pDrvCamsv[i]->writeReg(0x0D3C, g_pDrvCamsv[i]->readReg(0x0D3C) | 0x10);
        break;
    default:
        LOG__I("#####################################\n");
        LOG__I("error:unsupported pix_mode:%d\n",1<<epixel_mode);
        LOG__I("#####################################\n");
        break;
    }

    for (MUINT32 j=0;j<(sizeof(mPowerOnTM_2)/(sizeof(MUINT32)*2)); j++) {
        LOG__I("****** %s %d, Reg(0x%08x)=0x%08x\n", __FUNCTION__, __LINE__,
            mPowerOnTM_2[j][0], g_pDrvCamsv[i]->readReg(mPowerOnTM_2[j][0]));
    }

    g_pDrvCamsv[0]->setRWMode(ISP_DRV_RW_MMAP);

    return ret;
}

static int Seninf_Pattern_close(void)
{
    LOG__I("############################\n");
    LOG__I("Seninf_Pattern_close\n");
    LOG__I("############################\n");
    g_pDrvCamsv[0]->uninit("test pattern");
    g_pDrvCamsv[0]->destroyInstance();
    g_pDrvCamsv[0] = NULL;
    if(g_pDrvCamsv[1]){
        g_pDrvCamsv[1]->uninit("test pattern");
        g_pDrvCamsv[1]->destroyInstance();
        g_pDrvCamsv[1] = NULL;
    }
    return 0;
}




class TEST_CAMSVIO
{
public:
    TEST_CAMSVIO(MUINT32 senidx)
        :bEnque_before_start(MFALSE)
        ,bypass_imgo(MTRUE)
        ,Surce(TG_CAMSV_0)
        ,Source_Size(E_8M)
        ,TG_PixMode(ePixMode_1)
        ,TG_FmtSel(NSCam::eImgFmt_BAYER8)
        ,m_Thread()
        ,m_threadName(NULL)
        ,m_imgo_bin(MFALSE)
        ,ring_cnt(0)
        ,bypass_tg(MFALSE)
        ,pCamsvIo(NULL)
    {
        SenIdx = senidx;
        m_subSample = 0;
        buf_imgo = NULL;
        buf_imgo_fh = NULL;

    }

    TEST_CAMSVIO()
        :bEnque_before_start(MFALSE)
        ,bypass_imgo(MTRUE)
        ,Surce(TG_CAMSV_0)
        ,Source_Size(E_8M)
        ,TG_PixMode(ePixMode_1)
        ,m_subSample(0)
        ,m_Thread()
        ,m_threadName(NULL)
        ,m_imgo_bin(MFALSE)
        ,ring_cnt(0)
        ,bypass_tg(MFALSE)
        ,pCamsvIo(NULL)
        ,SenIdx(0)
    {
       TG_FmtSel = NSCam::eImgFmt_BAYER8;
       buf_imgo = NULL;
       buf_imgo_fh = NULL;
    }

    int     Stop_cam(void);
    int     Case_0(void);

private:
    int enque(void);
    int deque(void);
public:
    MBOOL       bEnque_before_start;
    MBOOL       bypass_imgo;

    E_INPUT     Surce;
    E_TGSZIE    Source_Size;
    E_CamPixelMode TG_PixMode;
    EImageFormat TG_FmtSel;
    MUINT32     m_subSample;

    pthread_t                                       m_Thread;
    char const*                                     m_threadName;
    MBOOL                                           m_imgo_bin;
private:
    #define BUF_DEPTH (2)

    IMEM_BUF_INFO* buf_imgo;
    IMEM_BUF_INFO* buf_imgo_fh;


    MUINT32     ring_cnt;

    PortInfo    imgo;
    STImgCrop   imgo_crop;

    PortInfo    tgi;
    MBOOL       bypass_tg;

    ICamIOPipe *pCamsvIo;


    MUINT32     SenIdx;
};

int TEST_CAMSVIO::enque(void)
{
    int ret=0;
    PortID portID;
    QBufInfo rQBufInfo;
    list<MUINT32> Input_L_mag;

    /**
        perframe control squence:
            Camsviopipe:
                1. enque imgo
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

            rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st]              = (MUINT32)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st]                = (MUINTPTR)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st]                = (MUINTPTR)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st] = (MUINT32)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]   = (MUINTPTR)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st]   = (MUINTPTR)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;

            LOG__I("##############################\n");
            LOG__I("enque_imgo_%d: size:0x%08x_0x%08x, pa:0x%08x_0x%08x, magic:0x%x, crop:%d_%d_%d_%d, rawtype:%d, ring_cnt:%d\n",\
                i,\
                rQBufInfo.vBufInfo.at(i).u4BufSize[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).m_num,\
                this->imgo_crop.x,this->imgo_crop.y,\
                this->imgo_crop.w,this->imgo_crop.h,\
                rawType, this->ring_cnt);
            LOG__I("##############################\n");
        }

        if(pCamsvIo->enqueOutBuf(portID, rQBufInfo) == MFALSE) {
            ret =1;
        }
    }

    this->ring_cnt = (this->ring_cnt + 1)%BUF_DEPTH;

    return ret;
}

int TEST_CAMSVIO::deque(void)
{
    int ret=0;
    PortID portID;
    QTimeStampBufInfo rQBufInfo;

    LOG__I("****** %s %d, TEST_CAMSVIO::deque +\n", __FUNCTION__, __LINE__);

    if(this->bypass_imgo == MFALSE) {

        portID.index = EPortIndex_IMGO;

        if(pCamsvIo->dequeOutBuf( portID, rQBufInfo) == MFALSE) {
            ret = 1;
        }

        for(UINT32 i=0; i<rQBufInfo.vBufInfo.size(); i++){
            LOG__I("##############################\n");
            LOG__I("deque_imgo_%d: size:0x%08x_0x%08x, pa:0x%08x_0x%08x, magic:0x%x, time:%lu, tg size:%dx%d,crop:%d_%d_%d_%d\n",\
                i,\
                rQBufInfo.vBufInfo.at(i).u4BufSize[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA[ePlane_1st],\
                rQBufInfo.vBufInfo.at(i).m_num,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_us,\
                rQBufInfo.vBufInfo.at(i).img_w, rQBufInfo.vBufInfo.at(i).img_h,\
                rQBufInfo.vBufInfo.at(i).crop_win.p.x, rQBufInfo.vBufInfo.at(i).crop_win.p.y,\
                rQBufInfo.vBufInfo.at(i).crop_win.s.w, rQBufInfo.vBufInfo.at(i).crop_win.s.h);
            LOG__I("##############################\n");
        }

        if (m_imgo_bin == MFALSE){
            #define SAVE_PATH           "/data/"
            char filename[256];
            struct stat st = {0};

            if (stat(SAVE_PATH, &st) == -1) {
                LOG__I("creating path: %s\n", SAVE_PATH);
                mkdir(SAVE_PATH, 0777);
            }
            else {
                LOG__I("already exist path: %s\n", SAVE_PATH);
            }

            sprintf(filename, SAVE_PATH "%s_imgo%dx%d_%d.bin", m_threadName, this->imgo_crop.w, this->imgo_crop.h, this->imgo.u4ImgWidth);
            LOG__I("CAMSV IMGO: saveBufToFile to %s\n", filename);
            if (saveBufToFile(filename,
                    reinterpret_cast<MUINT8*>(rQBufInfo.vBufInfo.at(0).u4BufVA[ePlane_1st]),
                    this->buf_imgo[0].size) == MFALSE) {
                LOG__I("CAMSV IMGO: fail saveBufToFile\n");
            }
            m_imgo_bin = MTRUE;
        }

    }

    LOG__I("****** %s %d, TEST_CAMSVIO::deque -\n", __FUNCTION__, __LINE__);

    return ret;
}

int TEST_CAMSVIO::Stop_cam(void)
{
    MINT32 ret = 0;

    for(UINT32 i=0;i<2;i++){
        if(1){//(g_pDrvCam[i]){
            if(i==0)
                g_pDrvCamsv[i] = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
            else
                g_pDrvCamsv[i] = (IspDrvImp*)IspDrvImp::createInstance(CAM_B);
            g_pDrvCamsv[i]->init("stop_cam");

            g_pDrvCamsv[i]->writeReg(0x0500,0x0);
            g_pDrvCamsv[i]->writeReg(0x0504,0x0);
            g_pDrvCamsv[i]->writeReg(0x0004,0x0);
            g_pDrvCamsv[i]->writeReg(0x0008,0x0);

            g_pDrvCamsv[i]->writeReg(0x0040,0x1);
            while(g_pDrvCamsv[i]->readReg(0x0040) != 0x2){
                LOG__I("reseting...\n");
            }
            g_pDrvCamsv[i]->writeReg(0x0040,0x4);
            g_pDrvCamsv[i]->writeReg(0x0040,0x0);

            g_pDrvCamsv[i]->uninit("stop_cam");
            g_pDrvCamsv[i]->destroyInstance();
            g_pDrvCamsv[i] = NULL;
        }
    }
    return ret;
}



int TEST_CAMSVIO::Case_0(void)
{

    int ret = 0;

    MUINT32 tg_size_w,tg_size_h;
    vector<PortInfo const*> vCamIOInPorts;
    vector<PortInfo const*> vCamIOOutPorts;
    vector<PortInfo const*> dummy;

    IMemDrv* mpImemDrv=NULL;
    MUINTPTR org_va_imgo=0,org_pa_imgo=0;
    const char Name[32] = {"case_0"};
    CAMIO_Func func;
    func.Raw = 0;
    char const * thread = this->m_threadName;
    //
    LOG__I("****** %s %d\n", thread, __LINE__);
    mpImemDrv = IMemDrv::createInstance();
    LOG__I("****** %s %d\n", thread, __LINE__);
    mpImemDrv->init();
    LOG__I("****** %s %d\n", thread, __LINE__);

    TG_SIZE(this->Source_Size, &tg_size_w, &tg_size_h);

    LOG__I("****** %s %d, tg_size_w(%d), tg_size_h(%d)\n", thread, __LINE__, tg_size_w, tg_size_h);

    /*Added subsample test: 2 frames*/
#ifdef CAMSV_SUBSAMPLE
    this->m_subSample = 1;
#endif
    for(MUINT32 i=0;i<(BUF_DEPTH* (this->m_subSample+1));i++){
        //
        LOG__I("****** %s %d, i(%d)/BUF_DEPTH(%d)/m_subSample(%d)\n", thread, __LINE__, i, BUF_DEPTH, this->m_subSample );
        if(this->bypass_imgo == MFALSE){
            if(this->buf_imgo == NULL){
                MUINT32 buf_imgo_size = sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1));
                MUINT32 buf_imgo_fh_size = sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1));
                this->buf_imgo = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_imgo_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                LOG__I("****** %s %d, buf_imgo addr(%p),info size(0x%08x)\n", thread, __LINE__, this->buf_imgo, buf_imgo_size);
                LOG__I("****** %s %d, buf_imgo_fh(%p),info size(0x%08x)\n", thread, __LINE__, this->buf_imgo_fh, buf_imgo_fh_size);
            }
            this->buf_imgo[i].useNoncache = MTRUE;
            this->buf_imgo_fh[i].useNoncache = MTRUE;
            //

            switch(this->TG_FmtSel){
                case NSCam::eImgFmt_BAYER8:
                    this->buf_imgo[i].size = tg_size_w*tg_size_h; //8bit case
                    break;
                case NSCam::eImgFmt_BAYER10:
                    this->buf_imgo[i].size = tg_size_w*tg_size_h*10/8; //10bit case
                    break;
                case NSCam::eImgFmt_BAYER12:
                    this->buf_imgo[i].size = tg_size_w*tg_size_h*12/8; //10bit case
                    break;
                case NSCam::eImgFmt_BAYER14:
                    this->buf_imgo[i].size = tg_size_w*tg_size_h*14/8; //10bit case
                    break;
                default:
                    printf("****** %s %d Incorrect Image Format(%d)\n", thread, __LINE__, this->TG_FmtSel);
                    LOG__I("****** %s %d Incorrect Image Format(%d)\n", thread, __LINE__, this->TG_FmtSel);
                    ret = 1;
                    goto EXIT;
                    break;
            }

            LOG__I("****** %s %d, buf_imgo[%d].size %d(0x%08x)\n", thread, __LINE__, i, this->buf_imgo[i].size, this->buf_imgo[i].size);

            mpImemDrv->allocVirtBuf(&this->buf_imgo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo[i]);
            LOG__I("****** %s %d, buf_imgo[%d].virtAddr 0x%08x\n", thread, __LINE__, i, this->buf_imgo[i].virtAddr);
            LOG__I("****** %s %d, buf_imgo[%d].phyAddr  0x%08x\n", thread, __LINE__, i, this->buf_imgo[i].phyAddr);
            //
            this->buf_imgo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            LOG__I("****** %s %d, buf_imgo_fh[%d].size %d(0x%08x)\n", thread, __LINE__, i, this->buf_imgo_fh[i].size, this->buf_imgo_fh[i].size);
            mpImemDrv->allocVirtBuf(&this->buf_imgo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo_fh[i]);
            LOG__I("****** %s %d, buf_imgo_fh[%d].virtAddr 0x%08x\n", thread, __LINE__, i, this->buf_imgo_fh[i].virtAddr);
            LOG__I("****** %s %d, buf_imgo_fh[%d].phyAddr  0x%08x\n", thread, __LINE__, i, this->buf_imgo_fh[i].phyAddr);
            //keep org address for release buf
            org_va_imgo = this->buf_imgo_fh[i].virtAddr;
            org_pa_imgo = this->buf_imgo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_imgo_fh[i].virtAddr = ((this->buf_imgo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_imgo_fh[i].phyAddr = ((this->buf_imgo_fh[i].phyAddr + 15) / 16 * 16);
        }
    }

    LOG__I("****** %s %d\n", thread, __LINE__);
    if((pCamsvIo = ICamIOPipe::createInstance((MINT8 const*)Name, this->Surce, ICamIOPipe::CAMSVIO)) == NULL){
        ret = 1;
        goto EXIT;
    }

    LOG__I("****** %s %d\n", thread, __LINE__);
    if(pCamsvIo->init() == MFALSE){
        pCamsvIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }

    //
    switch(this->Surce) {
    case TG_CAMSV_0:
        LOG__I("****** %s %d EPortIndex_CAMSV_0_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_0_TGI);
        this->tgi.index = EPortIndex_CAMSV_0_TGI;
        break;
    case TG_CAMSV_1:
        LOG__I("****** %s %d EPortIndex_CAMSV_1_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_1_TGI);
        this->tgi.index = EPortIndex_CAMSV_1_TGI;
        break;
    case TG_CAMSV_2:
        LOG__I("****** %s %d EPortIndex_CAMSV_2_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_2_TGI);
        this->tgi.index = EPortIndex_CAMSV_2_TGI;
        break;
    case TG_CAMSV_3:
        LOG__I("****** %s %d EPortIndex_CAMSV_3_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_3_TGI);
        this->tgi.index = EPortIndex_CAMSV_3_TGI;
        break;
    case TG_CAMSV_4:
        LOG__I("****** %s %d EPortIndex_CAMSV_4_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_4_TGI);
        this->tgi.index = EPortIndex_CAMSV_4_TGI;
        break;
    case TG_CAMSV_5:
        LOG__I("****** %s %d EPortIndex_CAMSV_5_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_5_TGI);
        this->tgi.index = EPortIndex_CAMSV_5_TGI;
        break;
    }

    this->tgi.ePxlMode       = this->TG_PixMode;
    this->tgi.eImgFmt        = this->TG_FmtSel;//NSCam::eImgFmt_BAYER10;
    this->tgi.eRawPxlID      = ERawPxlID_Gb;
    this->tgi.u4ImgWidth     = tg_size_w;
    this->tgi.u4ImgHeight    = tg_size_h;
    this->tgi.type           = EPortType_Sensor;
    this->tgi.inout          = EPortDirection_In;
    this->tgi.tgFps          = 1;
    this->tgi.crop1.x        = 0;
    this->tgi.crop1.y        = 0;
    this->tgi.crop1.floatX   = 0;
    this->tgi.crop1.floatY   = 0;
    this->tgi.crop1.w        = tg_size_w;
    this->tgi.crop1.h        = tg_size_h;
    this->tgi.tTimeClk       = 60;  //take 6mhz for example.
    vCamIOInPorts.push_back(&tgi);

    if(this->bypass_imgo == MFALSE){
        capibility CamInfo;
        tCAM_rst rst;
        E_CamPixelMode e_QueryPixMode;
        //
        this->imgo.index          = EPortIndex_CAMSV_IMGO; // jinn - update EPortIndex_IMGO
        this->imgo.ePxlMode       = this->TG_PixMode; // jinn
        this->imgo.eImgFmt        = this->TG_FmtSel;//NSCam::eImgFmt_BAYER10;
        this->imgo.u4PureRaw      = MFALSE;
        this->imgo.u4PureRawPak   = MTRUE;
        this->imgo.u4ImgWidth     = this->buf_imgo[0].size / tg_size_h;//tg_size_w;
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

        LOG__I("****** %s %d this->imgo.ePxlMode(%d)\n", thread, __LINE__, this->imgo.ePxlMode);

        e_QueryPixMode = this->imgo.ePxlMode;

        LOG__I("****** %s %d e_QueryPixMode(%d)\n", thread, __LINE__, e_QueryPixMode);

        if (CamInfo.GetCapibility(
                this->imgo.index,
                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)this->imgo.eImgFmt, this->imgo.u4ImgWidth, e_QueryPixMode),
                rst, E_CAM_UNKNOWNN) == MFALSE) {
            LOG__I("##############################\n");
            LOG__I("error:fmt(0x%x) | dma(0x%x) err\n",this->imgo.eImgFmt,this->imgo.index);
            LOG__I("##############################\n");
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
        else {
            if(rst.stride_byte[0] != this->imgo.u4Stride[ePlane_1st]){
                LOG__I("cur dma(%d) stride:0x%x, recommanded:0x%x",this->imgo.index,this->imgo.u4Stride[ePlane_1st],rst.stride_byte);
            }
        }
        this->imgo.u4Stride[ePlane_2nd] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
        this->imgo.u4Stride[ePlane_3rd] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
        this->imgo.type   = EPortType_Memory;
        this->imgo.inout  = EPortDirection_Out;
        vCamIOOutPorts.push_back(&this->imgo);
    }
    //
    LOG__I("****** %s %d, call configPipe()\n", thread, __LINE__);
    func.Bits.SUBSAMPLE = this->m_subSample;
    if(pCamsvIo->configPipe(vCamIOInPorts, vCamIOOutPorts, &func) == MFALSE){
        pCamsvIo->uninit();
        pCamsvIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }
    //
    LOG__I("****** %s %d, call enque()\n", thread, __LINE__);
    if(this->bEnque_before_start == MTRUE){
        if(this->enque()){
            LOG__I("##############################\n");
            LOG__I("error:enque fail\n");
            LOG__I("##############################\n");
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }
    //
    LOG__I("****** %s %d, call start()\n", thread, __LINE__);
    if(pCamsvIo->start() == MFALSE){
        pCamsvIo->uninit();
        pCamsvIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }

#if 0 // test SOF interrupt
    {
        Irq_t irq;
        irq.Type = Irq_t::_CLEAR_WAIT;
        irq.StatusType = Irq_t::_SIGNAL_INT_;
        irq.Status = Irq_t::_SOF_;
        irq.UserKey = 0;
        irq.Timeout = 8000;

        LOG__I("##############################\n");
        LOG__I("#     wait sof (debug)       #\n");
        LOG__I("##############################\n");
        if( pCamsvIo->irq(&irq) == MFALSE ) {
               LOG__I("##############################\n");
               LOG__I("error:wait sof fail          #\n");
               LOG__I("##############################\n");
               pCamsvIo->stop();
               pCamsvIo->uninit();
               pCamsvIo->destroyInstance();
               ret = 1;
               goto EXIT;
        }
    }
#endif
    //
    if(this->bEnque_before_start == MTRUE){
        if(this->deque()){
            LOG__I("##############################\n");
            LOG__I("# error:deque fail           #\n");
            LOG__I("##############################\n");
#if 1
                            {
                                char s[16];
                                char ch;
                                while(1) {
                                    LOG__I("##############################\n");
                                    LOG__I("# input q to exit loop       #\n");
                                    LOG__I("##############################\n");
                                    ch = (char) getchar();
                                    if( ch == 'q' || ch == 'Q' ) {
                                        break;
                                    }
                                }
                            }
#endif
            pCamsvIo->stop();
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
    }
	for (int jj=0;jj<3;jj++){
        Irq_t irq;
        irq.Type = Irq_t::_CLEAR_WAIT;
        irq.StatusType = Irq_t::_SIGNAL_INT_;
        irq.Status = Irq_t::_SOF_;
        irq.UserKey = 0;
        irq.Timeout = 8000;

        LOG__I("****** %s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", thread, __LINE__);
        LOG__I("##############################\n");
        LOG__I("#     wait sof_%d             #\n",jj);
        LOG__I("##############################\n");

        if( pCamsvIo->irq(&irq) == MFALSE ) {
            LOG__I("##############################\n");
            LOG__I("error:wait sof fail\n");
            LOG__I("##############################\n");
            pCamsvIo->stop();
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }

	if(this->deque()){
            LOG__I("##############################\n");
            LOG__I("# FALSE_PASS1_DONE PASS: No previous enqueue, OK to deque fail#\n");
            LOG__I("##############################\n");
	}else
	    LOG__I("#######FALSE_PASS1_DONE_CHECK NG(%d)#######################\n",jj);
	}

#if 0
    {
        char s[16];
        char ch;
        while(1) {
            LOG__I("##############################\n");
            LOG__I("# input q to exit loop       #\n");
            LOG__I("##############################\n");
            ch = getchar();
            if( ch == 'q' || ch == 'Q' ) {
                break;
            }
        }
    }
#endif

    //
    for(MUINT32 i=0; i<5; i++){
        Irq_t irq;
        irq.Type = Irq_t::_CLEAR_WAIT;
        irq.StatusType = Irq_t::_SIGNAL_INT_;
        irq.Status = Irq_t::_SOF_;
        irq.UserKey = 0;
        irq.Timeout = 8000;

        LOG__I("****** %s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", thread, __LINE__);
        LOG__I("##############################\n");
        LOG__I("#     wait sof_%d             #\n",i);
        LOG__I("##############################\n");

        if( pCamsvIo->irq(&irq) == MFALSE ) {
            LOG__I("##############################\n");
            LOG__I("error:wait sof fail\n");
            LOG__I("##############################\n");
            pCamsvIo->stop();
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }

        LOG__I("****** %s %d, call enque()\n", thread, __LINE__);
        if(this->enque()){
            LOG__I("##############################\n");
            LOG__I("error:enque fail\n");
            LOG__I("##############################\n");
            pCamsvIo->stop();
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }

        LOG__I("****** %s %d, call deque()\n", thread, __LINE__);
        if(this->deque()){
            LOG__I("##############################\n");
            LOG__I("error:deque fail\n");
            LOG__I("##############################\n");
            pCamsvIo->stop();
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }

#if 0
        if( i == 4 ) {
            char s[16];
            char ch;
            //CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN,0); /* 0x1A050504 */
            while(1) {
                LOG__I("##############################\n");
                LOG__I("# input q to exit loop       #\n");
                LOG__I("##############################\n");
                ch = getchar();
                if( ch == 'q' || ch == 'Q' ) {
                    break;
                }
            }
            //CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN,1); /* 0x1A050504 */
        }
#endif

    }
    LOG__I("****** %s %d <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", thread, __LINE__);

    LOG__I("****** %s %d, call pCamsvIo->stop()\n", thread, __LINE__);
    pCamsvIo->stop();
    LOG__I("****** %s %d, call pCamsvIo->uninit()\n", thread, __LINE__);
    pCamsvIo->uninit();
    LOG__I("****** %s %d, call pCamsvIo->destroyInstance()\n", thread, __LINE__);
    pCamsvIo->destroyInstance();
EXIT:
    //
    for(MUINT32 i=0;i<(BUF_DEPTH * (this->m_subSample + 1) );i++){
        if(this->bypass_imgo == MFALSE){
            mpImemDrv->freeVirtBuf(&this->buf_imgo[i]);

            this->buf_imgo_fh[i].phyAddr = org_pa_imgo;
            this->buf_imgo_fh[i].virtAddr = org_va_imgo;
            mpImemDrv->freeVirtBuf(&this->buf_imgo_fh[i]);
        }

    }
    if(this->bypass_imgo == MFALSE){
        LOG__I("****** %s %d, free buf_imgo (0x%08x)\n", thread, __LINE__, buf_imgo);
        free(buf_imgo);
        LOG__I("****** %s %d, free buf_imgo_fh (0x%08x)\n", thread, __LINE__, buf_imgo_fh);
        free(buf_imgo_fh);
    }
    LOG__I("****** %s %d, call pCamsvIo->uninit()\n", thread, __LINE__);
    mpImemDrv->uninit();
    LOG__I("****** %s %d, call pCamsvIo->destroyInstance()\n", thread, __LINE__);
    mpImemDrv->destroyInstance();

    LOG__I("****** %s %d, exit Case_0()\n", thread, __LINE__);
    return ret;
}

static void*    RunningThread(void *arg)
{
    int ret = 0;
    TEST_CAMSVIO *_this = reinterpret_cast<TEST_CAMSVIO*>(arg);
    LOG__I("****** %s %d, call Case_0()\n", __FUNCTION__, __LINE__);
    ret = _this->Case_0();
    LOG__I("****** %s %d, ret=%d\n", __FUNCTION__, __LINE__, ret);
    if(ret)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);
}

int main(int argc, char** argv)
{
    int rst = 0;
    char s[16];
    MUINT32 path, subsample = 0, dualCamsv = 0;
    MUINT32 _tg_pattern;
    MUINT32 _case0_test_mode;
    E_CamPixelMode _ePixelMode;
    TEST_CAMSVIO* pTEST_CAMSVIO;
    TEST_CAMSVIO* pTEST_CAMSVIO_1;
    char const * thread0_name = "CAMSV0\0";
    char const * thread1_name = "CAMSV1\0";
    void* thread_rst = NULL;
    (void)argc;(void)argv;

    LOG__I("##############################\n");
    LOG__I("case 1: Camsv, RAW10 IMGO; 1 pix\n");
    LOG__I("case 2: Camsv, RAW10 IMGO; 2 pixels\n");
    LOG__I("case 3: Camsv, RAW10 IMGO; 4 pixels\n");
    LOG__I("case 4: Camsv, RAW8  IMGO; 1 pix\n");
    LOG__I("case 5: Camsv, RAW8  IMGO; 2 pixels\n");
    LOG__I("case 6: Camsv, RAW8  IMGO; 4 pixels\n");
    LOG__I("select path\n");
    LOG__I("##############################\n");
    scanf("%s", s);
    path = atoi((const char*)s);
    LOG__I("select path=0x%x\n",path);

    //getchar();
    LOG__I("##############################\n");
    LOG__I("tg pattern  0: white\n");
    LOG__I("tg pattern  1: yellow\n");
    LOG__I("tg pattern  2: cyan\n");
    LOG__I("tg pattern  3: green\n");
    LOG__I("tg pattern  4: magenta\n");
    LOG__I("tg pattern  5: red\n");
    LOG__I("tg pattern  6: blue\n");
    LOG__I("tg pattern  7: black\n");
    LOG__I("tg pattern  8: horizontal gray level (Unit 1)\n");
    LOG__I("tg pattern  9: horizontal gray level (Unit 4)\n");
    LOG__I("tg pattern 10: horizontal gray level (Take 1024 pixel as one period) (only bayer)\n");
    LOG__I("tg pattern 11: vertical gray level (Unit 1)\n");
    LOG__I("tg pattern 12: static horizontal color bar\n");
    LOG__I("tg pattern 13: static vertical color bar (only bayer)\n");
    LOG__I("tg pattern 14: R,G,B,W flash every two frame (only bayer)\n");
    LOG__I("tg pattern 15: Dynamic horizontal colorbar (only bayer)\n");
    LOG__I("tg pattern\n");
    LOG__I("##############################\n");
    scanf("%s", s);
    _tg_pattern = atoi((const char*)s);
    if( _tg_pattern > 0xf ) {
        _tg_pattern = 13;
    }
    LOG__I("select pattern=0x%x(%d)\n",_tg_pattern, _tg_pattern);

    LOG__I("##############################\n");
    LOG__I("subsample: 0/1\n");
    LOG__I("##############################\n");
    scanf("%s", s);
    subsample = atoi((const char*)s);
    LOG__I("select subsample=0x%x\n",subsample);
    if (subsample > 1) {
        LOG__I("Unsupported: %d", subsample);
        return 0;
    }

    LOG__I("##############################\n");
    LOG__I("dualCamsv: 0/1\n");
    LOG__I("##############################\n");
    scanf("%s", s);
    dualCamsv = atoi((const char*)s);
    LOG__I("select dualCamsv=0x%x\n",dualCamsv);
    if (dualCamsv > 1) {
        LOG__I("Unsupported: %d", dualCamsv);
        return 0;
    }

    switch(path){
        case 1:
        case 2:
        case 3:
            switch(path){
                case 1:  _ePixelMode = ePixMode_1;break;
                case 2:  _ePixelMode = ePixMode_2;break;
                case 3:  _ePixelMode = ePixMode_4;break;
            }
            LOG__I("****** %s %d, case 1 --------------------------\n", __FUNCTION__, __LINE__);
            Seninf_Pattern(1,MFALSE,_ePixelMode,_tg_pattern);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO = new TEST_CAMSVIO(0); // sensor index 0
            pTEST_CAMSVIO->Surce = TG_CAMSV_0;
            pTEST_CAMSVIO->Source_Size = E_8M;
            pTEST_CAMSVIO->TG_PixMode = _ePixelMode;
            pTEST_CAMSVIO->TG_FmtSel = NSCam::eImgFmt_BAYER10;
            pTEST_CAMSVIO->bypass_imgo = MFALSE;
            pTEST_CAMSVIO->bEnque_before_start = MTRUE;
            pTEST_CAMSVIO->m_subSample = subsample;

            if (dualCamsv) {
                pTEST_CAMSVIO_1 = new TEST_CAMSVIO(0); // sensor index 0
                pTEST_CAMSVIO_1->Surce = TG_CAMSV_1;
                pTEST_CAMSVIO_1->Source_Size = E_8M;
                pTEST_CAMSVIO_1->TG_PixMode = _ePixelMode;
                pTEST_CAMSVIO_1->TG_FmtSel = NSCam::eImgFmt_BAYER10;
                pTEST_CAMSVIO_1->bypass_imgo = MFALSE;
                pTEST_CAMSVIO_1->bEnque_before_start = MTRUE;
                pTEST_CAMSVIO_1->m_threadName = thread1_name;

                pthread_create(&pTEST_CAMSVIO_1->m_Thread, NULL, RunningThread, pTEST_CAMSVIO_1);
            }

            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO->m_threadName = thread0_name;
            rst = pTEST_CAMSVIO->Case_0();
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);

            if (dualCamsv) {
                pthread_join(pTEST_CAMSVIO_1->m_Thread, &thread_rst);
                LOG__I("****** %s %d ----------------EOT1---------------\n", __FUNCTION__, __LINE__);
                delete pTEST_CAMSVIO_1;
            }

            delete pTEST_CAMSVIO;
            Seninf_Pattern_close();
            break;
        case 4:
        case 5:
        case 6:
            switch(path){
                case 4:  _ePixelMode = ePixMode_1;break;
                case 5:  _ePixelMode = ePixMode_2;break;
                case 6:  _ePixelMode = ePixMode_4;break;
            }
            LOG__I("****** %s %d, case 2 --------------------------\n", __FUNCTION__, __LINE__);
            Seninf_Pattern(1,MFALSE,_ePixelMode,_tg_pattern);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO = new TEST_CAMSVIO(0); // sensor index 0
            pTEST_CAMSVIO->Surce = TG_CAMSV_0;
            pTEST_CAMSVIO->Source_Size = E_8M;
            pTEST_CAMSVIO->TG_PixMode = _ePixelMode;
            pTEST_CAMSVIO->TG_FmtSel = NSCam::eImgFmt_BAYER8;
            pTEST_CAMSVIO->bypass_imgo = MFALSE;
            pTEST_CAMSVIO->bEnque_before_start = MTRUE;
            pTEST_CAMSVIO->m_subSample = subsample;

            if (dualCamsv) {
                pTEST_CAMSVIO_1 = new TEST_CAMSVIO(0); // sensor index 0
                pTEST_CAMSVIO_1->Surce = TG_CAMSV_1;
                pTEST_CAMSVIO_1->Source_Size = E_8M;
                pTEST_CAMSVIO_1->TG_PixMode = _ePixelMode;
                pTEST_CAMSVIO_1->TG_FmtSel = NSCam::eImgFmt_BAYER8;
                pTEST_CAMSVIO_1->bypass_imgo = MFALSE;
                pTEST_CAMSVIO_1->bEnque_before_start = MTRUE;
                pTEST_CAMSVIO_1->m_threadName = thread1_name;

                pthread_create(&pTEST_CAMSVIO_1->m_Thread, NULL, RunningThread, pTEST_CAMSVIO_1);
            }

            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO->m_threadName = thread0_name;
            rst = pTEST_CAMSVIO->Case_0();
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);

            if (dualCamsv) {
                pthread_join(pTEST_CAMSVIO_1->m_Thread, &thread_rst);
                LOG__I("****** %s %d ----------------EOT1---------------\n", __FUNCTION__, __LINE__);
                delete pTEST_CAMSVIO_1;
            }
            delete pTEST_CAMSVIO;
            Seninf_Pattern_close();
            break;
        case 7:
        case 8:
        case 9:
            switch(path){
                case 7:  _ePixelMode = ePixMode_1;break;
                case 8:  _ePixelMode = ePixMode_2;break;
                case 9:  _ePixelMode = ePixMode_4;break;
            }
            LOG__I("****** %s %d, case 3 --------------------------\n", __FUNCTION__, __LINE__);
            Seninf_Pattern(1,MFALSE,_ePixelMode,_tg_pattern);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO = new TEST_CAMSVIO(0); // sensor index 0
            pTEST_CAMSVIO->Surce = TG_CAMSV_0;
            pTEST_CAMSVIO->Source_Size = E_8M;
            pTEST_CAMSVIO->TG_PixMode = _ePixelMode;
            pTEST_CAMSVIO->TG_FmtSel = NSCam::eImgFmt_BAYER12;
            pTEST_CAMSVIO->bypass_imgo = MFALSE;
            pTEST_CAMSVIO->bEnque_before_start = MTRUE;
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO->m_threadName = thread0_name;
            rst = pTEST_CAMSVIO->Case_0();
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            delete pTEST_CAMSVIO;
            Seninf_Pattern_close();
            break;
        case 10:
        case 11:
        case 12:
            switch(path){
                case 10:  _ePixelMode = ePixMode_1;break;
                case 11:  _ePixelMode = ePixMode_2;break;
                case 12:  _ePixelMode = ePixMode_4;break;
            }
            LOG__I("****** %s %d, case 4 --------------------------\n", __FUNCTION__, __LINE__);
            Seninf_Pattern(1,MFALSE,_ePixelMode,_tg_pattern);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO = new TEST_CAMSVIO(0); // sensor index 0
            pTEST_CAMSVIO->Surce = TG_CAMSV_0;
            pTEST_CAMSVIO->Source_Size = E_8M;
            pTEST_CAMSVIO->TG_PixMode = _ePixelMode;
            pTEST_CAMSVIO->TG_FmtSel = NSCam::eImgFmt_BAYER14;
            pTEST_CAMSVIO->bypass_imgo = MFALSE;
            pTEST_CAMSVIO->bEnque_before_start = MTRUE;
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO->m_threadName = thread0_name;
            rst = pTEST_CAMSVIO->Case_0();
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            delete pTEST_CAMSVIO;
            Seninf_Pattern_close();
            break;

        default:
            break;
    }

    LOG__I("****** %s %d, exit\n", __FUNCTION__, __LINE__);

    return rst;
}

