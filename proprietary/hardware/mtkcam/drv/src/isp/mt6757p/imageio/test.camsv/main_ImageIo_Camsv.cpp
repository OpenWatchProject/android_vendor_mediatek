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
#include <ispio_utility.h>
#include <android/log.h>

/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <Cam_Notify_datatype.h>
#include <utils/Thread.h>

#undef LOG_TAG
#define LOG_TAG "Test_ImageIo_Camsv"

#define  LOG__I(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOG__D(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOG__D(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

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

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        printf("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            printf("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
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
            printf("#####################################\n");
            printf("error:unsupported size:0x%x\n",size);
            printf("#####################################\n");
            LOG__I("#####################################\n");
            LOG__I("error:unsupported size:0x%x\n",size);
            LOG__I("#####################################\n");
            *pSize_w = -1;
            *pSize_h = -1;
            return 1;
            break;
    }
    return 0;
}


//pix_mode 1: 1pix, 2: 2pix, 4:4pix
//#define PIXEL_MODE_4

static int Seninf_Pattern(MUINT32 cnt, MBOOL bYUV, EPxlMode epixel_mode, MUINT32 _tg_pattern)
{
    MINT32 ret = 0;

    g_pDrvCamsv[0] = (IspDrvImp*)IspDrvImp::createInstance(CAMSV_0);
    g_pDrvCamsv[0]->init("test pattern");
    g_pDrvCamsv[0]->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only

    if(cnt>1){
        g_pDrvCamsv[1] = (IspDrvImp*)IspDrvImp::createInstance(CAMSV_1);
        g_pDrvCamsv[1]->init("test pattern");
        g_pDrvCamsv[1]->setRWMode(ISP_DRV_RW_IOCTL);//specail control for start seninf , for test code only
    }
    printf("#####################################\n");
    printf("SENINF start in TestPattern Mode !!!![is YUV = %d]\n",bYUV);
    printf("pattern size :%d x %d with %d enabled pattern!!!!\n",0x1f00,0xfa0,cnt);
    printf("pix_mode: %d\n", 1<<epixel_mode);
    printf("#####################################\n");
    LOG__I("#####################################\n");
    LOG__I("SENINF start in TestPattern Mode !!!![is YUV = %d]\n",bYUV);
    LOG__I("pattern size :%d x %d with %d enabled pattern!!!!\n",0x1f00,0xfa0,cnt);
    LOG__I("pix_mode: %d\n", 1<<epixel_mode);
    LOG__I("#####################################\n");


    for(MUINT32 i=0;i< cnt;i++){
        printf("%d TM open\n",i+1);
        printf("#####################################\n");

        static MUINT32 mPowerOnTM_2[][2] = {
#ifdef EVEREST_BITFILE_SENINF
            {0x15040000, 0x00000C00}, //SENINF_TOP_CTRL, Rocky_SENINF_TOP_CODA.xls
            {0x15040010, 0x76543010}, //SENINF_TOP_CTRL, MUX SEL
            {0x15040200, 0x00001001},
            {0x15040204, 0x00000002}, // FPGA set 0x00000000
            {0x15040d00, 0x96DF1080},
            {0x15040d04, 0x8000007F},
            {0x15040d0c, 0x00000000},
            {0x15040d2c, 0x000E2000},
            {0x15040d3c, 0x00000000},
            {0x15040d38, 0x00000000},
            {0x15040608, 0x000404C1}, // TG1_TM_CTL
                                    // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
            {0x1504060c, 0x1F001F00}, // TG1_TM_SIZE [28:16] line number, [12:0] Pixel number
            {0x15040610, 0x00000000}, // TG1_TM_CLK
            {0x15040614, 0x00000001}, // TG1_TM_STP always setting timestamp dividor to 1 for test only
#else
        //no change
        {0x1A040000, 0x00000C00}, //SENINF_TOP_CTR
	    {0x1A040008, 0x76542210},	/*SENINF_TOP_MUX_CTRL to select from SENINF0~3*/
#ifdef PIXEL_MODE_4
				/*SENINF1_CTRL_EXT: no 4-pixel in mt6757/mt6755*/
#endif
        /*1. SENINF_TG is in every SENINF.
             If we want to get data from SENINF3_TG_TM, must enable SENINF3
          2. In using seninf's test model case, SENINF*_MUX clock source is from SENINF*.
             If we use SENINF3_MUX, SENINF3 must be enabled.
        */
        {0x1A040900, 0x00001001}, //SENINF3_CTL: Test Model + SENINF_EN
        {0x1A040920, 0x96DF1080}, //SENINF3_MUX_CTRL
        {0x1A040924, 0x8000007F}, //SENINF3_MUX_INTEN(RST)
        {0x1A04092c, 0x00000000}, //SENINF3_MUX_SIZE(RST)
        {0x1A04094c, 0x000E2000}, //SENINF3_MUX_SPARE(RST)
        {0x1A040958, 0x00000000}, //SENINF3_MUX_CROP(RST)

        {0x1A040A08, 0x000404C1}, // TG3_TM_CTL
                                // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
        {0x1A040A0c, 0x190012C0}, // TG3_TM_SIZE:Ju 0x0FA01F00
        {0x1A040A10, 0x00000000}, // TG3_TM_CLK
        {0x1A040A14, 0x00000001}, // TG3_TM_STP always setting timestamp dividor to 1 for test only

        //for PIP Camsv
        {0x1A040D00, 0x00001001}, //SENINF4_CTL: Test Model + SENINF_EN
        {0x1A040D20, 0x96DF1080}, //SENINF4_MUX_CTRL
        {0x1A040D24, 0x8000007F}, //SENINF4_MUX_INTEN(RST)
        {0x1A040D2c, 0x00000000}, //SENINF4_MUX_SIZE(RST)
        {0x1A040D4c, 0x000E2000}, //SENINF4_MUX_SPARE(RST)
        {0x1A040D58, 0x00000000}, //SENINF4_MUX_CROP(RST)
#endif
        };

        for(MUINT32 j=0;j<(sizeof(mPowerOnTM_2)/(sizeof(MUINT32)*2)); j++){
            if( mPowerOnTM_2[j][0]==0x1A040208 ) {
                mPowerOnTM_2[j][1] &= 0xFFFFFF0F;
                mPowerOnTM_2[j][1] |= (_tg_pattern<<4);
            }
            g_pDrvCamsv[i]->writeReg(mPowerOnTM_2[j][0]- 0x1A040000 ,mPowerOnTM_2[j][1]);
        }

        // yuv format
        if(1 == bYUV)
        {
            g_pDrvCamsv[i]->writeReg(0x1A040208 - 0x1A040000 , g_pDrvCamsv[i]->readReg(0x1A040208 - 0x1A040000 ) | 0x4);
        }

        // pixel mode
        switch(epixel_mode)
        {
            case ePxlMode_One_:
                g_pDrvCamsv[i]->writeReg(0x1A040920 - 0x1A040000 , g_pDrvCamsv[i]->readReg(0x1A040920 - 0x1A040000) & ~0x100);
#ifdef PIXEL_MODE_4
                g_pDrvCamsv[i]->writeReg(0x1A040D3C - 0x1A040000 , g_pDrvCamsv[i]->readReg(0x1A040D3C - 0x1A040000) & ~0x10);
#endif
                break;
            case ePxlMode_Two_:
                g_pDrvCamsv[i]->writeReg(0x1A040920 - 0x1A040000 , g_pDrvCamsv[i]->readReg(0x1A040920 - 0x1A040000) | 0x100);
#ifdef PIXEL_MODE_4
               g_pDrvCamsv[i]->writeReg(0x1A040D3C - 0x1A040000 , g_pDrvCamsv[i]->readReg(0x1A040D3C - 0x1A040000) & ~0x10); // jinn update
#endif
                break;
#ifdef PIXEL_MODE_4
            case ePxlMode_Four_:
                g_pDrvCamsv[i]->writeReg(0x1A040920 - 0x1A040000 , g_pDrvCamsv[i]->readReg(0x1A040920 - 0x1A040000) & ~0x100);
               g_pDrvCamsv[i]->writeReg(0x1A040D3C - 0x1A040000 , g_pDrvCamsv[i]->readReg(0x1A040D3C - 0x1A040000) | 0x10);
#endif
                break;
            default:
                printf("#####################################\n");
                printf("error:unsupported pix_mode:%d\n",1<<epixel_mode);
                printf("#####################################\n");
        }
        printf("****** %s %d, Reg(0x1A040000)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040000 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040010)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040010 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040200)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040200 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040204)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040204 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040120)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040120 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040124)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040124 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A04012C)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A04012C - 0x1A040000));
        printf("****** %s %d, Reg(0x1A04014C)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A04014C - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040158)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040158 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040D3C)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040D3C - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040600)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040600 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040604)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040604 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040208)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040208 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A04060c)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A04060C - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040610)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040610 - 0x1A040000));
        printf("****** %s %d, Reg(0x1A040614)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040614 - 0x1A040000));

        LOG__I("****** %s %d, Reg(0x1A040000)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040000 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040010)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040010 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040200)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040200 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040204)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040204 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040120)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040120 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040124)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040124 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A04012C)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A04012C - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A04014C)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A04014C - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040158)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040158 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040D3C)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040D3C - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040600)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040600 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040604)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040604 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040208)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040208 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A04060c)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A04060C - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040610)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040610 - 0x1A040000));
        LOG__I("****** %s %d, Reg(0x1A040614)=0x%08x\n", __FUNCTION__, __LINE__, g_pDrvCamsv[i]->readReg(0x1A040614 - 0x1A040000));



    }
    g_pDrvCamsv[0]->setRWMode(ISP_DRV_RW_MMAP);
    if(g_pDrvCamsv[1]) {
        g_pDrvCamsv[1]->setRWMode(ISP_DRV_RW_MMAP);
    }
    return ret;
}

static int Seninf_Pattern_close(void)
{
    printf("############################\n");
    printf("Seninf_Pattern_close\n");
    printf("############################\n");
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
        ,Surce(ICamIOPipe::TG_CAMSV_0)
        ,Source_Size(E_8M)
        ,TG_PixMode(ePxlMode_One_)
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
        ,Surce(ICamIOPipe::TG_CAMSV_0)
        ,Source_Size(E_8M)
        ,TG_PixMode(ePxlMode_One_)
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

    ICamIOPipe::E_INPUT Surce;
    E_TGSZIE    Source_Size;
    EPxlMode    TG_PixMode;
    EImageFormat TG_FmtSel;
    MUINT32     m_subSample;

    pthread_t   m_Thread;
    char const* m_threadName;
    MBOOL       m_imgo_bin;
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

            rQBufInfo.vBufInfo[i].u4BufSize[0]           = (MUINT32)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].u4BufVA[0]             = (MUINTPTR)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].u4BufPA[0]             = (MUINTPTR)this->buf_imgo[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize = (MUINT32)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].size;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA   = (MUINTPTR)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA   = (MUINTPTR)this->buf_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;

            printf("##############################\n");
            printf("enque_imgo_%d: size:0x%08x_0x%08x, pa:0x%08x_0x%08x, magic:0x%x, crop:%d_%d_%d_%d, rawtype:%d, ring_cnt:%d\n",\
                i,\
                rQBufInfo.vBufInfo.at(i).u4BufSize[0],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize,\
                (MUINT32)rQBufInfo.vBufInfo.at(i).u4BufPA[0],\
                (MUINT32)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                this->imgo_crop.x,this->imgo_crop.y,\
                this->imgo_crop.w,this->imgo_crop.h,\
                rawType, this->ring_cnt);
            printf("##############################\n");
            LOG__I("##############################\n");
            LOG__I("enque_imgo_%d: size:0x%08x_0x%08x, pa:0x%08x_0x%08x, magic:0x%x, crop:%d_%d_%d_%d, rawtype:%d, ring_cnt:%d\n",\
                i,\
                rQBufInfo.vBufInfo.at(i).u4BufSize[0],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize,\
                (MUINT32)rQBufInfo.vBufInfo.at(i).u4BufPA[0],\
                (MUINT32)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                this->imgo_crop.x,this->imgo_crop.y,\
                this->imgo_crop.w,this->imgo_crop.h,\
                rawType, this->ring_cnt);
            LOG__I("##############################\n");
        }
        //pCamsvIo->sendCommand(EPIPECmd_SET_IMGO, (MINTPTR)&Input_L, (MINTPTR)&Input2_L,0);

        if(pCamsvIo->enqueOutBuf(portID, rQBufInfo) == MFALSE) {
            ret =1;
        }
    }

    this->ring_cnt = (this->ring_cnt + 1)%BUF_DEPTH;


    //magic number & tuning
    //pCamIo->sendCommand(EPIPECmd_SET_P1_UPDATE,MINTPTR arg1,MINTPTR arg2,MINTPTR arg3);
    return ret;
}

int TEST_CAMSVIO::deque(void)
{
    int ret=0;
    PortID portID;
    QTimeStampBufInfo rQBufInfo;

    printf("****** %s %d, TEST_CAMSVIO::deque +\n", __FUNCTION__, __LINE__);
    LOG__I("****** %s %d, TEST_CAMSVIO::deque +\n", __FUNCTION__, __LINE__);

    if(this->bypass_imgo == MFALSE) {

        portID.index = EPortIndex_IMGO;

        if(pCamsvIo->dequeOutBuf( portID, rQBufInfo) == MFALSE) {
            ret = 1;
        }

        for(UINT32 i=0; i<rQBufInfo.vBufInfo.size(); i++){
            printf("##############################\n");
            printf("deque_imgo_%d: size:0x%08x_0x%08x, pa:0x%08x_0x%08x, magic:0x%x, time:%d_%d, tg size:%dx%d,crop:%d_%d_%d_%d\n",\
                i,\
                rQBufInfo.vBufInfo.at(i).u4BufSize[0],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize,\
                (MUINT32)rQBufInfo.vBufInfo.at(i).u4BufPA[0],\
                (MUINT32)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_sec,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_us,\
                rQBufInfo.vBufInfo.at(i).img_w, rQBufInfo.vBufInfo.at(i).img_h,\
                rQBufInfo.vBufInfo.at(i).crop_win.p.x, rQBufInfo.vBufInfo.at(i).crop_win.p.y,\
                rQBufInfo.vBufInfo.at(i).crop_win.s.w, rQBufInfo.vBufInfo.at(i).crop_win.s.h);
            printf("##############################\n");
            LOG__I("##############################\n");
            LOG__I("deque_imgo_%d: size:0x%08x_0x%08x, pa:0x%08x_0x%08x, magic:0x%x, time:%d_%d, tg size:%dx%d,crop:%d_%d_%d_%d\n",\
                i,\
                rQBufInfo.vBufInfo.at(i).u4BufSize[0],\
                rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize,\
                (MUINT32)rQBufInfo.vBufInfo.at(i).u4BufPA[0],\
                (MUINT32)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA,\
                rQBufInfo.vBufInfo.at(i).m_num,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_sec,\
                rQBufInfo.vBufInfo.at(i).i4TimeStamp_us,\
                rQBufInfo.vBufInfo.at(i).img_w, rQBufInfo.vBufInfo.at(i).img_h,\
                rQBufInfo.vBufInfo.at(i).crop_win.p.x, rQBufInfo.vBufInfo.at(i).crop_win.p.y,\
                rQBufInfo.vBufInfo.at(i).crop_win.s.w, rQBufInfo.vBufInfo.at(i).crop_win.s.h);
            LOG__I("##############################\n");
        }

        if (m_imgo_bin == MFALSE){
            char filename[256];
            printf("CAMSV IMGO: saveBufToFile \n");
            sprintf(filename, "/data/%s_imgo%dx%d.bin", m_threadName, this->imgo.u4ImgWidth,this->imgo.u4ImgHeight);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(rQBufInfo.vBufInfo.at(0).u4BufVA[0]), this->imgo.u4ImgWidth *this->imgo.u4ImgHeight * 2);
            m_imgo_bin = MTRUE;
        }

    }

    printf("****** %s %d, TEST_CAMSVIO::deque -\n", __FUNCTION__, __LINE__);
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
                printf("reseting...\n");
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
    ISP_QUERY_RST QueryRst;
    MUINTPTR org_va_imgo=0,org_pa_imgo=0;
    const char Name[32] = {"case_0"};
    CAMIO_Func func;
    func.Raw = 0;
    char const * thread = this->m_threadName;
    //
    printf("****** %s %d\n", thread, __LINE__);
    LOG__I("****** %s %d\n", thread, __LINE__);
    mpImemDrv = IMemDrv::createInstance();
    printf("****** %s %d\n", thread, __LINE__);
    LOG__I("****** %s %d\n", thread, __LINE__);
    mpImemDrv->init();
    printf("****** %s %d\n", thread, __LINE__);
    LOG__I("****** %s %d\n", thread, __LINE__);

    TG_SIZE(this->Source_Size, &tg_size_w, &tg_size_h);

    printf("****** %s %d, tg_size_w(%d), tg_size_h(%d)\n", thread, __LINE__, tg_size_w, tg_size_h);
    LOG__I("****** %s %d, tg_size_w(%d), tg_size_h(%d)\n", thread, __LINE__, tg_size_w, tg_size_h);

    /*Added subsample test: 2 frames*/
    for(MUINT32 i=0;i<(BUF_DEPTH* (this->m_subSample+1));i++){
        //
        printf("****** %s %d, i(%d)/BUF_DEPTH(%d)/m_subSample(%d)\n", thread, __LINE__, i, BUF_DEPTH, this->m_subSample );
        LOG__I("****** %s %d, i(%d)/BUF_DEPTH(%d)/m_subSample(%d)\n", thread, __LINE__, i, BUF_DEPTH, this->m_subSample );
        if(this->bypass_imgo == MFALSE){
            if(this->buf_imgo == NULL){
                MUINT32 buf_imgo_size = sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1));
                MUINT32 buf_imgo_fh_size = sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1));
                this->buf_imgo = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                this->buf_imgo_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(BUF_DEPTH* (this->m_subSample+1)));
                printf("****** %s %d, buf_imgo addr(%p),info size(0x%08x)\n", thread, __LINE__, this->buf_imgo, buf_imgo_size);
                printf("****** %s %d, buf_imgo_fh(%p),info size(0x%08x)\n", thread, __LINE__, this->buf_imgo_fh, buf_imgo_fh_size);
                LOG__I("****** %s %d, buf_imgo addr(%p),info size(0x%08x)\n", thread, __LINE__, this->buf_imgo, buf_imgo_size);
                LOG__I("****** %s %d, buf_imgo_fh(%p),info size(0x%08x)\n", thread, __LINE__, this->buf_imgo_fh, buf_imgo_fh_size);
            }
            this->buf_imgo[i].useNoncache = MTRUE;
            this->buf_imgo_fh[i].useNoncache = MTRUE;
            //
            if( this->TG_FmtSel == NSCam::eImgFmt_BAYER10 ) {
                this->buf_imgo[i].size = tg_size_w*tg_size_h*10/8; //10bit case
            }
            else if( this->TG_FmtSel == NSCam::eImgFmt_BAYER8 ) {
                this->buf_imgo[i].size = tg_size_w*tg_size_h; //8bit case
            }

            printf("****** %s %d, buf_imgo[%d].size %d(0x%08x)\n", thread, __LINE__, i, this->buf_imgo[i].size, this->buf_imgo[i].size);
            LOG__I("****** %s %d, buf_imgo[%d].size %d(0x%08x)\n", thread, __LINE__, i, this->buf_imgo[i].size, this->buf_imgo[i].size);

            //printf("****** %s %d, buf_imgo[%d].virtAddr 0x%08x\n", thread, __LINE__, i, this->buf_imgo[i].virtAddr);
            //printf("****** %s %d, buf_imgo[%d].phyAddr  0x%08x\n", thread, __LINE__, i, this->buf_imgo[i].phyAddr);
            mpImemDrv->allocVirtBuf(&this->buf_imgo[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo[i]);
            printf("****** %s %d, buf_imgo[%d].virtAddr 0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo[i].virtAddr);
            printf("****** %s %d, buf_imgo[%d].phyAddr  0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo[i].phyAddr);
            LOG__I("****** %s %d, buf_imgo[%d].virtAddr 0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo[i].virtAddr);
            LOG__I("****** %s %d, buf_imgo[%d].phyAddr  0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo[i].phyAddr);
            //
            this->buf_imgo_fh[i].size = sizeof(MUINT32)*(16 + 2);   //+2 for 64-bit alignment shift
            printf("****** %s %d, buf_imgo_fh[%d].size %d(0x%08x)\n", thread, __LINE__, i, this->buf_imgo_fh[i].size, this->buf_imgo_fh[i].size);
            LOG__I("****** %s %d, buf_imgo_fh[%d].size %d(0x%08x)\n", thread, __LINE__, i, this->buf_imgo_fh[i].size, this->buf_imgo_fh[i].size);
            mpImemDrv->allocVirtBuf(&this->buf_imgo_fh[i]);
            mpImemDrv->mapPhyAddr(&this->buf_imgo_fh[i]);
            printf("****** %s %d, buf_imgo_fh[%d].virtAddr 0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo_fh[i].virtAddr);
            printf("****** %s %d, buf_imgo_fh[%d].phyAddr  0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo_fh[i].phyAddr);
            LOG__I("****** %s %d, buf_imgo_fh[%d].virtAddr 0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo_fh[i].virtAddr);
            LOG__I("****** %s %d, buf_imgo_fh[%d].phyAddr  0x%08x\n", thread, __LINE__, i, (MUINT32)this->buf_imgo_fh[i].phyAddr);
            //keep org address for release buf
            org_va_imgo = this->buf_imgo_fh[i].virtAddr;
            org_pa_imgo = this->buf_imgo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            this->buf_imgo_fh[i].virtAddr = ((this->buf_imgo_fh[i].virtAddr + 15) / 16 * 16);
            this->buf_imgo_fh[i].phyAddr = ((this->buf_imgo_fh[i].phyAddr + 15) / 16 * 16);
        }
    }

    printf("****** %s %d\n", thread, __LINE__);
    LOG__I("****** %s %d\n", thread, __LINE__);
    if((pCamsvIo = ICamIOPipe::createInstance((MINT8 const*)Name, this->Surce, ICamIOPipe::CAMSVIO)) == NULL){
        ret = 1;
        goto EXIT;
    }

    printf("****** %s %d\n", thread, __LINE__);
    LOG__I("****** %s %d\n", thread, __LINE__);
    if(pCamsvIo->init() == MFALSE){
        pCamsvIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }

    //
    switch(this->Surce) {
        case ICamIOPipe::TG_CAMSV_0:
            printf("****** %s %d EPortIndex_CAMSV_0_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_0_TGI);
            LOG__I("****** %s %d EPortIndex_CAMSV_0_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_0_TGI);
            this->tgi.index = EPortIndex_CAMSV_0_TGI;
            break;
        case ICamIOPipe::TG_CAMSV_1:
            printf("****** %s %d EPortIndex_CAMSV_1_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_1_TGI);
            LOG__I("****** %s %d EPortIndex_CAMSV_1_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_1_TGI);
            this->tgi.index = EPortIndex_CAMSV_1_TGI;
            break;
        case ICamIOPipe::TG_CAMSV_2:
            printf("****** %s %d EPortIndex_CAMSV_2_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_2_TGI);
            LOG__I("****** %s %d EPortIndex_CAMSV_2_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_2_TGI);
            this->tgi.index = EPortIndex_CAMSV_2_TGI;
            break;
        case ICamIOPipe::TG_CAMSV_3:
            printf("****** %s %d EPortIndex_CAMSV_3_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_3_TGI);
            LOG__I("****** %s %d EPortIndex_CAMSV_3_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_3_TGI);
            this->tgi.index = EPortIndex_CAMSV_3_TGI;
            break;
        case ICamIOPipe::TG_CAMSV_4:
            printf("****** %s %d EPortIndex_CAMSV_4_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_4_TGI);
            LOG__I("****** %s %d EPortIndex_CAMSV_4_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_4_TGI);
            this->tgi.index = EPortIndex_CAMSV_4_TGI;
            break;
        case ICamIOPipe::TG_CAMSV_5:
            printf("****** %s %d EPortIndex_CAMSV_5_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_5_TGI);
            LOG__I("****** %s %d EPortIndex_CAMSV_5_TGI(%d)\n", thread, __LINE__, EPortIndex_CAMSV_5_TGI);
            this->tgi.index = EPortIndex_CAMSV_5_TGI;
            break;
        default:
            printf("****** %s %d Wrong EPortIndex_CAMSV_X_TGI(%d)\n", thread, __LINE__, this->Surce);
            LOG__I("****** %s %d Wrong EPortIndex_CAMSV_X_TGI(%d)\n", thread, __LINE__, this->Surce);
            ret = 1;
            goto EXIT;
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
        //
        this->imgo.index          = EPortIndex_CAMSV_IMGO; // jinn - update EPortIndex_IMGO
        this->imgo.ePxlMode       = this->TG_PixMode; // jinn
        this->imgo.eImgFmt        = this->TG_FmtSel;//NSCam::eImgFmt_BAYER10;
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
        this->imgo.u4Stride[ESTRIDE_1ST_PLANE] = this->buf_imgo[0].size / tg_size_h;

        //prepare for enque if needed
        this->imgo_crop.x = this->imgo_crop.y = this->imgo_crop.floatX = this->imgo_crop.floatY = 0;
        this->imgo_crop.w = tg_size_w;
        this->imgo_crop.h = tg_size_h;

        printf("****** %s %d this->imgo.ePxlMode(%d)\n", thread, __LINE__, this->imgo.ePxlMode);
        LOG__I("****** %s %d this->imgo.ePxlMode(%d)\n", thread, __LINE__, this->imgo.ePxlMode);

        E_ISP_PIXMODE e_QueryPixMode = ISP_QUERY_1_PIX_MODE;

        switch( this->imgo.ePxlMode ) {
            case ePxlMode_One_:  e_QueryPixMode = ISP_QUERY_1_PIX_MODE;break;
            case ePxlMode_Two_:  e_QueryPixMode = ISP_QUERY_2_PIX_MODE;break;
            case ePxlMode_Four_: e_QueryPixMode = ISP_QUERY_4_PIX_MODE;break;
            default: break;
        }

        printf("****** %s %d e_QueryPixMode(%d)\n", thread, __LINE__, e_QueryPixMode);
        LOG__I("****** %s %d e_QueryPixMode(%d)\n", thread, __LINE__, e_QueryPixMode);

        if(ISP_QuerySize(this->imgo.index,ISP_QUERY_STRIDE_BYTE,this->imgo.eImgFmt,this->imgo.u4ImgWidth,QueryRst, e_QueryPixMode) == 0){
            printf("##############################\n");
            printf("error:fmt(0x%x) | dma(0x%x) err\n",this->imgo.eImgFmt,this->imgo.index);
            printf("##############################\n");
            LOG__I("##############################\n");
            LOG__I("error:fmt(0x%x) | dma(0x%x) err\n",this->imgo.eImgFmt,this->imgo.index);
            LOG__I("##############################\n");
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }
        else {
            if(QueryRst.stride_byte != this->imgo.u4Stride[ESTRIDE_1ST_PLANE]){
                printf("cur dma(%d) stride:0x%x, recommanded:0x%x",this->imgo.index,this->imgo.u4Stride[ESTRIDE_1ST_PLANE],QueryRst.stride_byte);
                LOG__I("cur dma(%d) stride:0x%x, recommanded:0x%x",this->imgo.index,this->imgo.u4Stride[ESTRIDE_1ST_PLANE],QueryRst.stride_byte);
            }
        }
        this->imgo.u4Stride[ESTRIDE_2ND_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
        this->imgo.u4Stride[ESTRIDE_3RD_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
        this->imgo.type   = EPortType_Memory;
        this->imgo.inout  = EPortDirection_Out;
        vCamIOOutPorts.push_back(&this->imgo);
    }
    //
    printf("****** %s %d, call configPipe()\n", thread, __LINE__);
    LOG__I("****** %s %d, call configPipe()\n", thread, __LINE__);
    func.Bits.SUBSAMPLE = this->m_subSample;
    if(pCamsvIo->configPipe(vCamIOInPorts,vCamIOOutPorts,func) == MFALSE){
        pCamsvIo->uninit();
        pCamsvIo->destroyInstance();
        ret = 1;
        goto EXIT;
    }
    //
    printf("****** %s %d, call enque()\n", thread, __LINE__);
    LOG__I("****** %s %d, call enque()\n", thread, __LINE__);
    if(this->bEnque_before_start == MTRUE){
        if(this->enque()){
            printf("##############################\n");
            printf("error:enque fail\n");
            printf("##############################\n");
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
    printf("****** %s %d, call start()\n", thread, __LINE__);
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

        printf("##############################\n");
        printf("#     wait sof (debug)       #\n");
        printf("##############################\n");
        LOG__I("##############################\n");
        LOG__I("#     wait sof (debug)       #\n");
        LOG__I("##############################\n");
        if( pCamsvIo->irq(&irq) == MFALSE ) {

               //printf("****** %s %d stamp = 0x%08x\n", thread, __LINE__, g_pDrvCamsv[0]->readReg(0x15050570 - 0x15040000));

               printf("##############################\n");
               printf("error:wait sof fail          #\n");
               printf("##############################\n");
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
        printf("****** %s %d, call deque()\n", thread, __LINE__);
        if(this->deque()){
            printf("##############################\n");
            printf("# error:deque fail           #\n");
            printf("##############################\n");
            LOG__I("##############################\n");
            LOG__I("# error:deque fail           #\n");
            LOG__I("##############################\n");
#if 1
                            {
                                char s[16];
                                char ch;
                                while(1) {
                                    printf("##############################\n");
                                    printf("# input q to exit loop       #\n");
                                    printf("##############################\n");
                                    LOG__I("##############################\n");
                                    LOG__I("# input q to exit loop       #\n");
                                    LOG__I("##############################\n");
                                    ch = (char)getchar();
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
/*FALSE_PASS1_DONE*/
	printf("#########FALSE_PASS1_DONE_CHECK STARTS#####################\n");
	for (int jj=0;jj<3;jj++){
        Irq_t irq;
        irq.Type = Irq_t::_CLEAR_WAIT;
        irq.StatusType = Irq_t::_SIGNAL_INT_;
        irq.Status = Irq_t::_SOF_;
        irq.UserKey = 0;
        irq.Timeout = 8000;

        printf("****** %s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", thread, __LINE__);
        printf("##############################\n");
        printf("#     wait sof_%d             #\n",jj);
        printf("##############################\n");
        LOG__I("****** %s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", thread, __LINE__);
        LOG__I("##############################\n");
        LOG__I("#     wait sof_%d             #\n",jj);
        LOG__I("##############################\n");

        if( pCamsvIo->irq(&irq) == MFALSE ) {
            printf("##############################\n");
            printf("error:wait sof fail\n");
            printf("##############################\n");
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
            printf("##############################\n");
            printf("# FALSE_PASS1_DONE PASS(%d): No previous enqueue, OK to deque fail#\n",jj);
            printf("##############################\n");
            LOG__I("##############################\n");
            LOG__I("# FALSE_PASS1_DONE PASS: No previous enqueue, OK to deque fail#\n");
            LOG__I("##############################\n");
	}else
	    printf("#######FALSE_PASS1_DONE_CHECK NG(%d)#######################\n",jj);
	}

#if 0
    {
        char s[16];
        char ch;
        while(1) {
            printf("##############################\n");
            printf("# input q to exit loop       #\n");
            printf("##############################\n");
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

        printf("****** %s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", thread, __LINE__);
        printf("##############################\n");
        printf("#     wait sof_%d             #\n",i);
        printf("##############################\n");
        LOG__I("****** %s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", thread, __LINE__);
        LOG__I("##############################\n");
        LOG__I("#     wait sof_%d             #\n",i);
        LOG__I("##############################\n");

        if( pCamsvIo->irq(&irq) == MFALSE ) {
            printf("##############################\n");
            printf("error:wait sof fail\n");
            printf("##############################\n");
            LOG__I("##############################\n");
            LOG__I("error:wait sof fail\n");
            LOG__I("##############################\n");
            pCamsvIo->stop();
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }

        printf("****** %s %d, call enque()\n", thread, __LINE__);
        printf("****** %s %d, call enque()\n", thread, __LINE__);
        if(this->enque()){
            printf("##############################\n");
            printf("error:enque fail\n");
            printf("##############################\n");
            LOG__I("##############################\n");
            LOG__I("error:enque fail\n");
            LOG__I("##############################\n");
            pCamsvIo->stop();
            pCamsvIo->uninit();
            pCamsvIo->destroyInstance();
            ret = 1;
            goto EXIT;
        }

        printf("****** %s %d, call deque()\n", thread, __LINE__);
        printf("****** %s %d, call deque()\n", thread, __LINE__);
        if(this->deque()){
            printf("##############################\n");
            printf("error:deque fail\n");
            printf("##############################\n");
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
                printf("##############################\n");
                printf("# input q to exit loop       #\n");
                printf("##############################\n");
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
    printf("****** %s %d <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", thread, __LINE__);
    LOG__I("****** %s %d <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", thread, __LINE__);

    printf("****** %s %d, call pCamsvIo->stop()\n", thread, __LINE__);
    LOG__I("****** %s %d, call pCamsvIo->stop()\n", thread, __LINE__);
    pCamsvIo->stop();
    printf("****** %s %d, call pCamsvIo->uninit()\n", thread, __LINE__);
    LOG__I("****** %s %d, call pCamsvIo->uninit()\n", thread, __LINE__);
    pCamsvIo->uninit();
    printf("****** %s %d, call pCamsvIo->destroyInstance()\n", thread, __LINE__);
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
        printf("****** %s %d, free buf_imgo (0x%p)\n", thread, __LINE__,(void*)buf_imgo);
        LOG__I("****** %s %d, free buf_imgo (0x%p)\n", thread, __LINE__,(void*)buf_imgo);
        free(buf_imgo);
        printf("****** %s %d, free buf_imgo_fh (0x%p)\n", thread, __LINE__, (void*)buf_imgo_fh);
        LOG__I("****** %s %d, free buf_imgo_fh (0x%p)\n", thread, __LINE__, (void*)buf_imgo_fh);
        free(buf_imgo_fh);
    }
    printf("****** %s %d, call pCamsvIo->uninit()\n", thread, __LINE__);
    LOG__I("****** %s %d, call pCamsvIo->uninit()\n", thread, __LINE__);
    mpImemDrv->uninit();
    printf("****** %s %d, call pCamsvIo->destroyInstance()\n", thread, __LINE__);
    LOG__I("****** %s %d, call pCamsvIo->destroyInstance()\n", thread, __LINE__);
    mpImemDrv->destroyInstance();

    printf("****** %s %d, exit Case_0()\n", thread, __LINE__);
    LOG__I("****** %s %d, exit Case_0()\n", thread, __LINE__);
    return ret;
}

static void*    RunningThread(void *arg)
{
    int ret = 0;
    TEST_CAMSVIO *_this = reinterpret_cast<TEST_CAMSVIO*>(arg);
    printf("****** %s %d, call Case_0()\n", __FUNCTION__, __LINE__);
    LOG__I("****** %s %d, call Case_0()\n", __FUNCTION__, __LINE__);
    ret = _this->Case_0();
    printf("****** %s %d, ret=%d\n", __FUNCTION__, __LINE__, ret);
    LOG__I("****** %s %d, ret=%d\n", __FUNCTION__, __LINE__, ret);
    if(ret)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);
}

int Test_ImageIo_Camsv(int argc, char** argv)
{
    int rst = 0;
    char s[16];
    MUINT32 path = 1;
    MUINT32 _tg_pattern, _subsample, _dual;
    MUINT32 _case0_test_mode;
    EPxlMode _ePixelMode;
    TEST_CAMSVIO* pTEST_CAMSVIO;
    TEST_CAMSVIO* pTEST_CAMSVIO_1;
    char const * thread0_name = "CAMSV0\0";
    char const * thread1_name = "CAMSV1\0";
    void* thread_rst = NULL;
    (void)argc;(void)argv;

    printf("****** %s %d, enter\n", __FUNCTION__, __LINE__);

    getchar();
    printf("##############################\n");
    printf("case 1: Camsv, RAW10 IMGO; 1 pix\n");
    printf("case 2: Camsv, RAW10 IMGO; 2 pixels\n");
    printf("case 3: Camsv, RAW10 IMGO; 4 pixels\n");
    printf("case 4: Camsv, RAW8  IMGO; 1 pix\n");
    printf("case 5: Camsv, RAW8  IMGO; 2 pixels\n");
    printf("case 6: Camsv, RAW8  IMGO; 4 pixels\n");
    printf("select path\n");
    printf("##############################\n");
    //gets(s);
    fgets(s,sizeof(s),stdin);
    path = atoi((const char*)s);
    printf("select path=0x%x\n",path);

    //getchar();
    printf("##############################\n");
    printf("tg pattern  0: white\n");
    printf("tg pattern  1: yellow\n");
    printf("tg pattern  2: cyan\n");
    printf("tg pattern  3: green\n");
    printf("tg pattern  4: magenta\n");
    printf("tg pattern  5: red\n");
    printf("tg pattern  6: blue\n");
    printf("tg pattern  7: black\n");
    printf("tg pattern  8: horizontal gray level (Unit 1)\n");
    printf("tg pattern  9: horizontal gray level (Unit 4)\n");
    printf("tg pattern 10: horizontal gray level (Take 1024 pixel as one period) (only bayer)\n");
    printf("tg pattern 11: vertical gray level (Unit 1)\n");
    printf("tg pattern 12: static horizontal color bar\n");
    printf("tg pattern 13: static vertical color bar (only bayer)\n");
    printf("tg pattern 14: R,G,B,W flash every two frame (only bayer)\n");
    printf("tg pattern 15: Dynamic horizontal colorbar (only bayer)\n");
    printf("tg pattern\n");
    printf("##############################\n");
    fgets(s,2,stdin);
    _tg_pattern = atoi((const char*)s);
    if( _tg_pattern > 0xf ) {
        _tg_pattern = 13;
    }
    printf("select pattern=0x%x(%d)\n",_tg_pattern, _tg_pattern);

    //
    printf("##############################\n");
    printf("subsample, default is 1 (no-subsample), only can be 1, 2, 4\n");
    printf("##############################\n");
    fgets(s,1,stdin);
    _subsample = atoi((const char*)s);
    if( _subsample > 4 ) {
        _subsample = 4;
    } else if(_subsample > 2 && _subsample % 2 == 1){
        _subsample = 2;
    } else if(_subsample < 1){
        _subsample = 1;
    }
    printf("select subsample=0x%x(%d)\n",_subsample, _subsample);
    _subsample -= 1;//configPipe CAMSV should be 0,1,3

    //
    printf("##############################\n");
    printf("Single Camsv or PIP camsv\n");
    printf("0: Single Camsv\n");
    printf("1: PIP camsv\n");
    printf("##############################\n");
    fgets(s,1,stdin);
    _dual = atoi((const char*)s);
    if( _dual > 1 ) {
        _dual = 1;
    }
    printf("select dualCamsv=0x%x(%d)\n",_dual, _dual);

    switch(path){
        case 1:
        case 2:
        case 3:
            switch(path){
                case 1:  _ePixelMode = ePxlMode_One_;break;
                case 2:  _ePixelMode = ePxlMode_Two_;break;
                case 3:  _ePixelMode = ePxlMode_Four_;break;
                default: _ePixelMode = ePxlMode_One_;
            }
            printf("****** %s %d, case 1 --------------------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d, case 1 --------------------------\n", __FUNCTION__, __LINE__);
            Seninf_Pattern(1,MFALSE,_ePixelMode,_tg_pattern);
            printf("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO = new TEST_CAMSVIO(0); // sensor index 0
            pTEST_CAMSVIO->Surce = ICamIOPipe::TG_CAMSV_0;
            pTEST_CAMSVIO->Source_Size = E_8M;
            pTEST_CAMSVIO->TG_PixMode = _ePixelMode;
            pTEST_CAMSVIO->TG_FmtSel = NSCam::eImgFmt_BAYER10;
            pTEST_CAMSVIO->bypass_imgo = MFALSE;
            pTEST_CAMSVIO->bEnque_before_start = MTRUE;
            pTEST_CAMSVIO->m_subSample = _subsample;

            if(_dual){
                pTEST_CAMSVIO_1 = new TEST_CAMSVIO(0); // sensor index 0
                pTEST_CAMSVIO_1->Surce = ICamIOPipe::TG_CAMSV_1;
                pTEST_CAMSVIO_1->Source_Size = E_8M;
                pTEST_CAMSVIO_1->TG_PixMode = _ePixelMode;
                pTEST_CAMSVIO_1->TG_FmtSel = NSCam::eImgFmt_BAYER10;
                pTEST_CAMSVIO_1->bypass_imgo = MFALSE;
                pTEST_CAMSVIO_1->bEnque_before_start = MTRUE;
                pTEST_CAMSVIO_1->m_threadName = thread1_name;
                pTEST_CAMSVIO_1->m_subSample = _subsample;

                pthread_create(&pTEST_CAMSVIO_1->m_Thread, NULL, RunningThread, pTEST_CAMSVIO_1);
            }
            printf("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO->m_threadName = thread0_name;
            rst = pTEST_CAMSVIO->Case_0();
            printf("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);

            if(_dual){
                pthread_join(pTEST_CAMSVIO_1->m_Thread, &thread_rst);
                printf("****** %s %d ----------------EOT1---------------\n", __FUNCTION__, __LINE__);
                delete pTEST_CAMSVIO_1;
            }

            delete pTEST_CAMSVIO;
            Seninf_Pattern_close();
            break;
        case 4:
        case 5:
        case 6:
            switch(path){
                case 4:  _ePixelMode = ePxlMode_One_;break;
                case 5:  _ePixelMode = ePxlMode_Two_;break;
                case 6:  _ePixelMode = ePxlMode_Four_;break;
                default: _ePixelMode = ePxlMode_One_;
            }
            printf("****** %s %d, case 2 --------------------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d, case 2 --------------------------\n", __FUNCTION__, __LINE__);
            Seninf_Pattern(1,MFALSE,_ePixelMode,_tg_pattern);
            printf("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO = new TEST_CAMSVIO(0); // sensor index 0
            pTEST_CAMSVIO->Surce = ICamIOPipe::TG_CAMSV_0;
            pTEST_CAMSVIO->Source_Size = E_8M;
            pTEST_CAMSVIO->TG_PixMode = _ePixelMode;
            pTEST_CAMSVIO->TG_FmtSel = NSCam::eImgFmt_BAYER8;
            pTEST_CAMSVIO->bypass_imgo = MFALSE;
            pTEST_CAMSVIO->bEnque_before_start = MTRUE;
            pTEST_CAMSVIO->m_subSample = _subsample;

            if(_dual){
                pTEST_CAMSVIO_1 = new TEST_CAMSVIO(0); // sensor index 0
                pTEST_CAMSVIO_1->Surce = ICamIOPipe::TG_CAMSV_1;
                pTEST_CAMSVIO_1->Source_Size = E_8M;
                pTEST_CAMSVIO_1->TG_PixMode = _ePixelMode;
                pTEST_CAMSVIO_1->TG_FmtSel = NSCam::eImgFmt_BAYER8;
                pTEST_CAMSVIO_1->bypass_imgo = MFALSE;
                pTEST_CAMSVIO_1->bEnque_before_start = MTRUE;
                pTEST_CAMSVIO_1->m_threadName = thread1_name;
                pTEST_CAMSVIO_1->m_subSample = _subsample;

                pthread_create(&pTEST_CAMSVIO_1->m_Thread, NULL, RunningThread, pTEST_CAMSVIO_1);
            }
            printf("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);
            pTEST_CAMSVIO->m_threadName = thread0_name;
            rst = pTEST_CAMSVIO->Case_0();
            printf("****** %s %d ----------------EOT0--------------\n", __FUNCTION__, __LINE__);
            LOG__I("****** %s %d ----------------------------------\n", __FUNCTION__, __LINE__);

            if(_dual){
                pthread_join(pTEST_CAMSVIO_1->m_Thread, &thread_rst);
                printf("****** %s %d ----------------EOT1---------------\n", __FUNCTION__, __LINE__);
                delete pTEST_CAMSVIO_1;
            }

            delete pTEST_CAMSVIO;
            Seninf_Pattern_close();
            break;
        default:
            break;
    }

    printf("****** %s %d, exit\n", __FUNCTION__, __LINE__);
    LOG__I("****** %s %d, exit\n", __FUNCTION__, __LINE__);

    return rst;
}

