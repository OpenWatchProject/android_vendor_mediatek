/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define __STDC_FORMAT_MACROS
#define LOG_TAG "SeninfDrvImp"
//
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <utils/threads.h>
#include <cutils/atomic.h>

#include <mtkcam/def/common.h>

#include <isp_reg.h>
#include "seninf_reg.h"
#include "seninf_drv_imp.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"

#include <cutils/properties.h>
#define DESKEW_LOG_IF(cond, ...)      do { if ( (cond) ) { LOG_MSG(__VA_ARGS__); } }while(0)
#define CSI_BLANKING_RATIO 10
#define CSI_DESKEW_THRESHOLD 1500000000 //1.5Gbits


#define MAX_CODE 63

#define MAX_PACKET_CNT 2
#if 1//def REAL_SENSOR
#define ERROR_CODE        0x0BA//0xFFFF6FFB// not 0x9004
#else
#define ERROR_CODE        0x38
#endif
#define CHECK_ERR_DELAY 1//us
#define isMaxCode(code)     (code == MAX_CODE)
//#define BUSY_WAIT 10000000 //1us without print log

//
/******************************************************************************
*
*******************************************************************************/
#define SENSOR_DEV_NAME     "/dev/kd_camera_hw"
#define ISP_DEV_NAME         "/dev/camera-isp"
#define DEBUG_SCAM_CALI 1
#define DEBUG_CSI_AUTO_DESKEW 1

//#define FPGA (1)//ToDo: remove after FPGA
#define SETTLE_DELAY 0x15
#define HS_TRAIL_VALUE 0x1B

#define ISP_HW_RANGE 0x2000
#define SENINF_HW_RANGE 0x8000
#define MIPIRX_HW_RANGE 0x6000
#define GPIO_HW_RANGE 0x1000
#define SCAM_CALI_TIMEOUT 0xFFFFF // 0x3D9DEA//14.5fps   0x2f72c//30fps
#define IS_4D1C (CSI2_IP < SENINF1_CSI0A)
#define IS_CDPHY_COMBO  (CSI2_IP == SENINF1_CSI0A ||\
                        CSI2_IP == SENINF2_CSI0B ||\
                        CSI2_IP == SENINF1_CSI0)



/*Efuse definition*/
#define CSI2_EFUSE_SET
/*Analog Sync Mode*/
#define ANALOG_SYNC
/*define sw Offset cal*/
#define CSI2_SW_OFFSET_CAL

bool CSI0A_Offset_Calibration = false;
bool CSI0B_Offset_Calibration = false;
bool CSI1A_Offset_Calibration = false;
bool CSI1B_Offset_Calibration = false;
bool CSI2_Offset_Calibration = false;

//
static bool ISP_CLK_GET = 0;
/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::createInstance()
{
    return SeninfDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrvImp::
getInstance()
{
    static SeninfDrvImp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
SeninfDrvImp::
destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::SeninfDrvImp() :
    SeninfDrv()
{
    LOG_MSG("[SeninfDrvImp]");

    mUsers = 0;
    mfd = 0;
    m_fdSensor = -1;
    mpCSI2RxAnalogRegStartAddrAlign = NULL;
    mpGpioHwRegAddr = NULL;
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::~SeninfDrvImp()
{
    LOG_MSG("[~SeninfDrvImp]");
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::init()  //kk test
{


    MINT32 seninf_base_addr = 0x1a040000;
    MINT32 mipiRx_analog    = 0x11c10000;  /* mipi_rx_ana_csi0*/

    MINT32 gpio_drv_base_addr   = 0x10002000;

    LOG_MSG("[init]: Entry count %d", mUsers);

    Mutex::Autolock lock(mLock_mmap);

    //
    if (mUsers > 0) {
        LOG_MSG("  Has inited");
        android_atomic_inc(&mUsers);
        return 0;
    }

    for( int k = 0; k < 3; k++){
        mfd = open(ISP_DEV_NAME, O_RDWR);
        if (mfd < 0)
            LOG_ERR("open %s fail retry %d", ISP_DEV_NAME, k);
        else
            break;
    }

    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s", errno, strerror(errno));
        return -1;
    }
    //Open sensor driver
    m_fdSensor = open(SENSOR_DEV_NAME, O_RDWR);
    if (m_fdSensor < 0) {
        LOG_ERR("[init]: error opening  %s",  strerror(errno));
        return -13;
    }
#if 0
   // reference isp_drv.cpp
   // gIspDrvObj[CAM_A].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAM_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAM_A_BASE_HW);
    mpIspHwRegAddr = (unsigned int *) mmap(0, ISP_HW_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, cam_isp_addr);
    if (mpIspHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s", errno, strerror(errno));
        return -4;
    }
#endif

    // mmap seninf reg
    mpSeninfHwRegAddr = (unsigned int *) mmap(0, SENINF_HW_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, seninf_base_addr);
    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s", errno, strerror(errno));
        return -5;
    }


    // mipi rx analog address
    //mpCSI2RxAnalogRegStartAddr = (unsigned int *) mmap(0, CAM_MIPIRX_ANALOG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, mipiRx_analog);
    mpCSI2RxAnalogRegStartAddrAlign = (unsigned int *) mmap(0, MIPIRX_HW_RANGE, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, mipiRx_analog);
    if (mpCSI2RxAnalogRegStartAddrAlign == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s", errno, strerror(errno));
        return -9;
    }



    //gpio
    mpGpioHwRegAddr = (unsigned int *) mmap(0, GPIO_HW_RANGE, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, gpio_drv_base_addr);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(6), %d, %s", errno, strerror(errno));
        return -10;
    }



    //6595 force Seninf_mux_en first to avoid standby mode w/ data output
#if 0
    //seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_EN) = 1;
#endif
    /*CSI2 Base address*/
    //MMAP only support Page alignment(0x1000)
    mpCSI2RxAnaBaseAddr[SENINF1_CSI0]  = mpCSI2RxAnalogRegStartAddrAlign;              //11E4 0000
    mpCSI2RxAnaBaseAddr[SENINF1_CSI0A] = mpCSI2RxAnaBaseAddr[SENINF1_CSI0];             //11E4 0000
    mpCSI2RxAnaBaseAddr[SENINF2_CSI0B] = mpCSI2RxAnaBaseAddr[SENINF1_CSI0]  + (0x1000/4); //11E4 1000
    mpCSI2RxAnaBaseAddr[SENINF3_CSI1]  = mpCSI2RxAnaBaseAddr[SENINF1_CSI0]  + (0x2000/4); //11E4 2000
    mpCSI2RxAnaBaseAddr[SENINF3_CSI1A] = mpCSI2RxAnaBaseAddr[SENINF1_CSI0]  + (0x2000/4); //11E4 2000
    mpCSI2RxAnaBaseAddr[SENINF4_CSI1B] = mpCSI2RxAnaBaseAddr[SENINF1_CSI0]  + (0x3000/4); //11E4 3000
    mpCSI2RxAnaBaseAddr[SENINF5_CSI2]  = mpCSI2RxAnaBaseAddr[SENINF1_CSI0]  + (0x4000/4); //11E4 4000

    /*SenINF base address*/
    mpSeninfCtrlRegAddr[SENINF_1] = mpSeninfHwRegAddr;             //18040000
    mpSeninfCtrlRegAddr[SENINF_2] = mpSeninfHwRegAddr + (0x1000/4);//18041000
    mpSeninfCtrlRegAddr[SENINF_3] = mpSeninfHwRegAddr + (0x2000/4);
    mpSeninfCtrlRegAddr[SENINF_4] = mpSeninfHwRegAddr + (0x3000/4);
    mpSeninfCtrlRegAddr[SENINF_5] = mpSeninfHwRegAddr + (0x4000/4);
    /*SenINF Mux Base address*/
    mpSeninfMuxBaseAddr[SENINF_MUX1] = mpSeninfHwRegAddr + (0x0d00/4);
    mpSeninfMuxBaseAddr[SENINF_MUX2] = mpSeninfHwRegAddr + (0x1d00/4);
    mpSeninfMuxBaseAddr[SENINF_MUX3] = mpSeninfHwRegAddr + (0x2d00/4);
    mpSeninfMuxBaseAddr[SENINF_MUX4] = mpSeninfHwRegAddr + (0x3d00/4);
    mpSeninfMuxBaseAddr[SENINF_MUX5] = mpSeninfHwRegAddr + (0x4d00/4);
    mpSeninfMuxBaseAddr[SENINF_MUX6] = mpSeninfHwRegAddr + (0x5d00/4);

    mpSeninfCSIRxConfBaseAddr[SENINF_1] = mpSeninfHwRegAddr + (0x0824/4);//18040800
    mpSeninfCSIRxConfBaseAddr[SENINF_2] = mpSeninfHwRegAddr + (0x1824/4);//18041800
    mpSeninfCSIRxConfBaseAddr[SENINF_3] = mpSeninfHwRegAddr + (0x2824/4);//18042800
    mpSeninfCSIRxConfBaseAddr[SENINF_4] = mpSeninfHwRegAddr + (0x3824/4);//18043800
    mpSeninfCSIRxConfBaseAddr[SENINF_5] = mpSeninfHwRegAddr + (0x4824/4);//18044800

    memset(&supported_ISP_Clks, 0, sizeof(IMAGESENSOR_GET_SUPPORTED_ISP_CLK));
    if (ioctl(m_fdSensor, KDIMGSENSORIOC_GET_SUPPORTED_ISP_CLOCKS, &supported_ISP_Clks) < 0)
        LOG_ERR("KDIMGSENSORIOC_GET_SUPPORTED_ISP_CLOCKS fail");
    else {
        if (supported_ISP_Clks.clklevelcnt > 0) {
            for(int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; i++ )
               mISPclk_tar[i] = supported_ISP_Clks.clklevel[supported_ISP_Clks.clklevelcnt-1];
        }
    }

#ifdef CSI2_EFUSE_SET
    mCSI[0] = 0; /* initial CSI value*/
    mCSI[1] = 0; /* initial CSI value*/
    mCSI[2] = 0; /* initial CSI value*/
    mCSI[3] = 0; /* initial CSI value*/
    mCSI[4] = 0; /* initial CSI value*/

    typedef struct {
        unsigned int entry_num;
        unsigned int data[200];
    } DEVINFO_S;
    int fd = 0;
    int ret = 0;

    DEVINFO_S devinfo;
    fd = open("/proc/device-tree/chosen/atag,devinfo", O_RDONLY); /* v2 device node */
    if (fd < 0) { /* Use v2 device node if v1 device node is removed */
        LOG_ERR("/proc/device-tree/chosen/atag,devinfo kernel open fail, errno(%d):%s",errno,strerror(errno));
    } else {
        ret = read(fd, (void *)&devinfo, sizeof(DEVINFO_S));
        if (ret < 0) {
            LOG_ERR("Get Devinfo data fail, errno(%d):%s",errno,strerror(errno));
        } else {
            mCSI[0] = devinfo.data[103];//0x11c5018c
            mCSI[1] = devinfo.data[104];//0x11c50190
            mCSI[2] = devinfo.data[115];//0x11c501bc
            mCSI[3] = devinfo.data[116];//0x11c501c0
        }
        LOG_MSG("Efuse Data:0x11c5018c= 0x%x, 0x11c50190= 0x%x, 0x11c501bc= 0x%x, 0x11c501c0= 0x%x", mCSI[0], mCSI[1], mCSI[2], mCSI[3]);
        close(fd);
    }
#endif

    android_atomic_inc(&mUsers);

    LOG_MSG("[init]: Exit count %d", mUsers);


    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::uninit()
{
    LOG_MSG("[uninit]: %d", mUsers);

    //MBOOL result;//6593


    Mutex::Autolock lock(mLock_mmap);

    if (mUsers <= 0) {
        // No more users
        return 0;
    }

    // More than one user
    android_atomic_dec(&mUsers);

    if (mUsers == 0) {
        // Last user
        mSeninfMuxUsed[SENINF_MUX1] = -1;
        mSeninfMuxUsed[SENINF_MUX2] = -1;
        for(int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; i++ )
            mISPclk_tar[i] = 0;

        setSeninf1CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2
        setSeninf2CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2
        setSeninf3CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2

        //set CMMCLK mode 0
        //*(mpGpioHwRegAddr + (0xEA0/4)) &= 0xFFF8;

        // Jessy added for debug mpIPllCon0RegAddr, to check the setting flow of mlck pll is correct
        //LOG_MSG("mpIPllCon0RegAddr %x, State: uinit", *mpIPllCon0RegAddr);

        if ( 0 != mpSeninfHwRegAddr ) {
            if(munmap(mpSeninfHwRegAddr, SENINF_HW_RANGE)!=0) {
                LOG_ERR("mpSeninfHwRegAddr munmap err, %d, %s", errno, strerror(errno));
            }
            mpSeninfHwRegAddr = NULL;
         }

        if ( 0 != mpCSI2RxAnalogRegStartAddrAlign ) {
            if(munmap(mpCSI2RxAnalogRegStartAddrAlign, MIPIRX_HW_RANGE)!=0){
                LOG_ERR("mpCSI2RxAnalogRegStartAddr munmap err, %d, %s", errno, strerror(errno));
            }
            mpCSI2RxAnalogRegStartAddrAlign = NULL;
        }

        if ( 0 != mpGpioHwRegAddr ) {
            if(munmap(mpGpioHwRegAddr, GPIO_HW_RANGE)!=0) {
                LOG_ERR("mpGpioHwRegAddr munmap err, %d, %s", errno, strerror(errno));
            }
            mpGpioHwRegAddr = NULL;
        }

         //
         LOG_MSG("[uninit]: %d, mfd(%d)", mUsers, mfd);
        //
        if (mfd > 0) {
            close(mfd);
            mfd = -1;
        }

        if (m_fdSensor > 0) {
            close(m_fdSensor);
            m_fdSensor = -1;
        }

        memset(&supported_ISP_Clks, 0, sizeof(IMAGESENSOR_GET_SUPPORTED_ISP_CLK));
    }
    else {
        LOG_ERR("  Still users");
    }

    return 0;
}

int SeninfDrvImp::configMclk(SENINF_MCLK_PARA *pmclk_para, unsigned long pcEn){
    MINT32  ret = 0;

    MINT32  clkCnt = 0;
    MINT32  csr_tg_tmp_stp = 0;

    std::string str_prop("vendor.debug.seninf.Tg");
    str_prop += std::to_string(pmclk_para->sensorIdx);
    str_prop +="clk";

    LOG_MSG("[configMclk] Tg%dclk: %d pcEn = %lu", pmclk_para->sensorIdx, pmclk_para->mclkFreq, pcEn);

    /*get property*/
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(str_prop.c_str(), value, "-1");

    int tgClk = atoi(value);

    if(tgClk > 0) {
        pmclk_para->mclkFreq = tgClk;
        LOG_MSG("[setSensorMclk] setproperty Tg%dclk: %d", pmclk_para->sensorIdx, pmclk_para->mclkFreq);
    }


    pmclk_para->mclkPLL = 1000;
    /*sub sample:csr_tg_tmp_stp */
    csr_tg_tmp_stp = 103;//depend on camtm_sel default 208Mhz;for 1M --> 208/(103+1) = (1+1) = 1M

    clkCnt = (pmclk_para->mclkFreq + (pmclk_para->mclkFreq >> 1)) / pmclk_para->mclkFreq;
    // Maximum CLKCNT is 15
    clkCnt = clkCnt > 15 ? 15 : clkCnt - 1;
    LOG_MSG("mclk%d: %d, clkCnt1: %d pclkInv %d",
            pmclk_para->sensorIdx, pmclk_para->mclkFreq, clkCnt, pmclk_para->pclkInv);

    ret = setMclk((EMclkId)pmclk_para->mclkIdx,
                pcEn, pmclk_para->mclkFreq,
                clkCnt, pmclk_para->mclkPolarityLow ? 0 : 1,
                pmclk_para->mclkFallingCnt,
                pmclk_para->mclkRisingCnt,
                pmclk_para->pclkInv,
                csr_tg_tmp_stp);
    if (ret < 0)
        LOG_ERR("setMclk fail");
    return ret;
}

int SeninfDrvImp::setMclk(EMclkId mclkIdx, unsigned long pcEn, unsigned long mclkSel,
       unsigned long clkCnt, unsigned long clkPol,
       unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv,
       unsigned long TimestampClk)
{
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int ret = 0;
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    static MUINT32 mMclkUser[eMclk_count] = {0, 0, 0, 0 };
    (void) clkCnt;
    (void) clkFallEdge;
    (void) clkRiseEdge;
    (void) padPclkInv;

    if(pcEn)
        mMclkUser[mclkIdx]++;
    else
        mMclkUser[mclkIdx]--;

    if(pcEn == 1 && mMclkUser[mclkIdx] == 1){
        sensorMclk.on = 1;
        sensorMclk.TG = mclkIdx;
        sensorMclk.freq = mclkSel;
        ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
        if (ret < 0) {
           LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
        }
    }
    else if(pcEn == 0 && mMclkUser[mclkIdx] == 0){
        sensorMclk.on = 0;
        sensorMclk.TG = mclkIdx;
        sensorMclk.freq = mclkSel;
        ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
        if (ret < 0) {
           LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
        }

    }

    SENINF_WRITE_REG(pSeninf, SENINF_TG1_TM_STP, TimestampClk);
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = 1;

    LOG_MSG("[setMclk]pcEn(%d), clkPol(%d), mMclkUser[%d](%d), TimestampClk(%d), freq(%lu), mclkIdx %d",
           (MINT32)pcEn, (MINT32)clkPol, mclkIdx, mMclkUser[mclkIdx], (MINT32)TimestampClk, mclkSel, mclkIdx);

    usleep(100);
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
#define DEBUG_DELAY 3000
#define CSI_CORRECT 0x9044
#define CSI_ERR_RESET 0xB0
int SeninfDrvImp::sendCommand(int cmd, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    int ret = 0;
    static unsigned int mipi_packet_cnt[SENINF_NUM] = {0};

    (void)arg2;
    (void)arg3;

    Mutex::Autolock lock(mLock_mmap);//for uninit, some pointer will be set to NULL
    if (mpSeninfHwRegAddr == NULL) {
        LOG_ERR("mpSeninfHwRegAddr = NULL, seninf has been uninit, stop dump to avoid NE");
        return ret;
    }

    switch (cmd) {
    case CMD_SENINF_REFRESH_DFS:
        {
            IMGSENSOR_SENSOR_IDX sensorIdx = IMGSENSOR_SENSOR_IDX_NONE;
            if (arg1 < IMGSENSOR_SENSOR_IDX_MAX_NUM && arg1 >= IMGSENSOR_SENSOR_IDX_MAIN)
                sensorIdx = (IMGSENSOR_SENSOR_IDX)arg1;
            else {
                LOG_ERR("CMD_SENINF_REFRESH_DFS wrong sensorIdx %d", sensorIdx);
                return ret;
            }
            updateIspClk(supported_ISP_Clks.clklevelcnt > 0 ?supported_ISP_Clks.clklevel[supported_ISP_Clks.clklevelcnt-1] :0, sensorIdx);
        }
        break;
    case CMD_SENINF_DEBUG_TASK:
        {
            UINT32 targetSENINF = 0xff;
            IMGSENSOR_SENSOR_IDX sensorIdx = IMGSENSOR_SENSOR_IDX_NONE;
            seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
            unsigned int temp[SENINF_NUM] = {0};
            static unsigned int mmsys_clk = 0;

            /*Please don't use meter oftenly*/
            if(ISP_CLK_GET == 0)
            {
                memset(mipi_packet_cnt, 0, SENINF_NUM* sizeof(int));
                LOG_MSG("clr SENINF1_PkCnt(0x%x), SENINF2_PkCnt(0x%x), SENINF3_PkCnt(0x%x), SENINF5_PkCnt(0x%x)",
                mipi_packet_cnt[SENINF_1], mipi_packet_cnt[SENINF_2], mipi_packet_cnt[SENINF_3], mipi_packet_cnt[SENINF_5]);
                mmsys_clk = 3;
                ret= ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_CSI_CLK,&mmsys_clk);
                ISP_CLK_GET =1;
            }

            temp[SENINF_1] = SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS);
            temp[SENINF_2] = SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS);
            temp[SENINF_3] = SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS);
            temp[SENINF_5] = SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS);


            LOG_MSG("SENINF_TOP_MUX_CTRL(0x%x) SENINF_TOP_CAM_MUX_CTRL(0x%x) ISP_clk(%d)",
                SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF_TOP_CAM_MUX_CTRL), mmsys_clk);

            if (arg1 < IMGSENSOR_SENSOR_IDX_MAX_NUM && arg1 >= IMGSENSOR_SENSOR_IDX_MAIN)
                sensorIdx = (IMGSENSOR_SENSOR_IDX)arg1;

            if (sensorIdx != IMGSENSOR_SENSOR_IDX_NONE){
                UINT32 top_mux = SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL);
                if (mSeninfMuxUsed[SENINF_MUX1] == sensorIdx) {
                    targetSENINF = ((top_mux>>0)&0xf);
                } else if  (mSeninfMuxUsed[SENINF_MUX2] == sensorIdx) {
                    targetSENINF = ((top_mux>>1)&0xf);
                } else
                    targetSENINF = 0xfe;
            }

            /*Sensor Interface Control */
            LOG_MSG("SensorIdx = %d, targetSENINF = %d, mSeninfMuxUsed[SENINF_MUX1] = %d, mSeninfMuxUsed[SENINF_MUX2] = %d",
                        sensorIdx, targetSENINF, mSeninfMuxUsed[SENINF_MUX1], mSeninfMuxUsed[SENINF_MUX2]);
            LOG_MSG("SENINF1_CSI2_CTL(0x%x), SENINF2_CSI2_CTL(0x%x), SENINF3_CSI2_CTL(0x%x), SENINF5_CSI2_CTL(0x%x)",
                SENINF_READ_REG(pSeninf,SENINF1_CSI2_CTL), SENINF_READ_REG(pSeninf,SENINF2_CSI2_CTL), SENINF_READ_REG(pSeninf,SENINF3_CSI2_CTL), SENINF_READ_REG(pSeninf,SENINF5_CSI2_CTL));
            LOG_MSG("SENINF1_CSI2_INT_STATUS(0x%x), SENINF2_CSI2_INT_STATUS(0x%x), SENINF3_CSI2_INT_STATUS(0x%x), SENINF5_CSI2_INT_STATUS(0x%x)",
                temp[SENINF_1], temp[SENINF_2], temp[SENINF_3], temp[SENINF_5]);


            /*Sensor Interface IRQ */
            if (SENINF_READ_REG(pSeninf,SENINF1_CSI2_CTL) & 0x1){

                if (temp[SENINF_1] != CSI_CORRECT){
                    /* sync error, crc error, frame end not sync */
                    SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_INT_STATUS,0xffffffff);
                    usleep(DEBUG_DELAY);
                    LOG_MSG("SENINF1_CSI2_INT_STATUS(0x%x), CLR SENINF1_CSI2_INT_STATUS(0x%x)",
                        temp[SENINF_1] , SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS));
                }
                temp[SENINF_1] = SENINF_READ_REG(pSeninf,SENINF1_CSI2_DBG_PORT);
                if (targetSENINF == SENINF_1) {
                    if ((SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS) & 0xb0)!= 0){
                        ret = -2;
                    }
                    if (mipi_packet_cnt[SENINF_1] != 0)
                        if (mipi_packet_cnt[SENINF_1] == temp[SENINF_1] )
                            ret = -1;
                }
                mipi_packet_cnt[SENINF_1] = temp[SENINF_1];
            }

            if (SENINF_READ_REG(pSeninf,SENINF2_CSI2_CTL) & 0x1){

                if (temp[SENINF_2] != CSI_CORRECT){
                    /* sync error, crc error, frame end not sync */
                    SENINF_WRITE_REG(pSeninf,SENINF2_CSI2_INT_STATUS,0xffffffff);
                    usleep(DEBUG_DELAY);
                    LOG_MSG("SENINF2_CSI2_INT_STATUS(0x%x), CLR SENINF2_CSI2_INT_STATUS(0x%x)",
                        temp[SENINF_2] , SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS));
                }
                temp[SENINF_2] = SENINF_READ_REG(pSeninf,SENINF2_CSI2_DBG_PORT);
                if (targetSENINF == SENINF_2) {
                    if ((SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS) & 0xb0)!= 0){
                        ret = -2;
                    }

                    if (mipi_packet_cnt[SENINF_2] != 0)
                        if (mipi_packet_cnt[SENINF_2] == temp[SENINF_2] )
                            ret = -1;
                }
                mipi_packet_cnt[SENINF_2] = temp[SENINF_2];
            }

            if (SENINF_READ_REG(pSeninf,SENINF3_CSI2_CTL) & 0x1){
                if (temp[SENINF_3] != CSI_CORRECT){
                    /* sync error, crc error, frame end not sync */
                    SENINF_WRITE_REG(pSeninf,SENINF3_CSI2_INT_STATUS,0xffffffff);
                    usleep(DEBUG_DELAY);
                    LOG_MSG("SENINF3_CSI2_INT_STATUS(0x%x), CLR SENINF3_CSI2_INT_STATUS(0x%x)",
                        temp[SENINF_3] , SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS));
                }

                temp[SENINF_3]  = SENINF_READ_REG(pSeninf,SENINF3_CSI2_DBG_PORT);
                if (targetSENINF == SENINF_3) {
                    if ((SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS) & 0xb0)!= 0) {
                        ret = -2;
                    }
                    if (mipi_packet_cnt[SENINF_3] != 0)
                        if (mipi_packet_cnt[SENINF_3] == temp[SENINF_3] )
                            ret = -1;
                }
                mipi_packet_cnt[SENINF_3] = temp[SENINF_3] ;
            }

            if (SENINF_READ_REG(pSeninf,SENINF5_CSI2_CTL) & 0x1){
                if (temp[SENINF_5] != CSI_CORRECT){
                    /* sync error, crc error, frame end not sync */
                    SENINF_WRITE_REG(pSeninf,SENINF5_CSI2_INT_STATUS,0xffffffff);
                    usleep(DEBUG_DELAY);
                    LOG_MSG("SENINF5_CSI2_INT_STATUS(0x%x), CLR SENINF5_CSI2_INT_STATUS(0x%x)",
                        temp[SENINF_5] , SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS));
                }

                temp[SENINF_5]  = SENINF_READ_REG(pSeninf,SENINF5_CSI2_DBG_PORT);
                if (targetSENINF == SENINF_5) {
                    if ((SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS) & 0xb0)!= 0) {
                        ret = -2;
                    }

                    if (mipi_packet_cnt[SENINF_5] != 0)
                        if (mipi_packet_cnt[SENINF_5] == temp[SENINF_5] )
                            ret = -1;
                }

                mipi_packet_cnt[SENINF_5] = temp[SENINF_5] ;
            }

            LOG_MSG("            SENINF1_PkCnt(0x%x), SENINF2_PkCnt(0x%x), SENINF3_PkCnt(0x%x), SENINF5_PkCnt(0x%x)",
                mipi_packet_cnt[SENINF_1], mipi_packet_cnt[SENINF_2], mipi_packet_cnt[SENINF_3], mipi_packet_cnt[SENINF_5]);
            usleep(DEBUG_DELAY);
            LOG_MSG("check again SENINF1_PkCnt(0x%x), SENINF2_PkCnt(0x%x), SENINF3_PkCnt(0x%x), SENINF5_PkCnt(0x%x)",
                SENINF_READ_REG(pSeninf,SENINF1_CSI2_DBG_PORT),
                SENINF_READ_REG(pSeninf,SENINF2_CSI2_DBG_PORT),
                SENINF_READ_REG(pSeninf,SENINF3_CSI2_DBG_PORT),
                SENINF_READ_REG(pSeninf,SENINF5_CSI2_DBG_PORT));

            LOG_MSG("SENINF1_IRQ(0x%x), SENINF2_IRQ(0x%x), SENINF3_IRQ(0x%x), SENINF5_IRQ(0x%x), EXT_IRQ(1:0x%x, 2:0x%x, 3:0x%x 5:0x%x)",
                SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS),
                SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS_EXT), SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS_EXT), SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS_EXT), SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS_EXT));
            /*Mux1 */
            LOG_MSG("SENINF1_MUX_CTRL(0x%x), SENINF1_MUX_INTSTA(0x%x), SENINF1_MUX_SIZE(0x%x), SENINF1_MUX_DEBUG_2(0x%x), SENINF1_MUX_DEBUG_3(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF1_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF1_MUX_SIZE), SENINF_READ_REG(pSeninf,SENINF1_MUX_DEBUG_2),SENINF_READ_REG(pSeninf,SENINF1_MUX_DEBUG_3));
            if(SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA) & 0x1)
            {
                SENINF_WRITE_REG(pSeninf,SENINF1_MUX_INTSTA,0xffffffff);
                usleep(DEBUG_DELAY);
                 LOG_MSG("after reset overrun, SENINF1_MUX_CTRL(0x%x), SENINF1_MUX_INTSTA(0x%x), SENINF1_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF1_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF1_MUX_DEBUG_2));
            }
            /*Mux2 */
            LOG_MSG("SENINF2_MUX_CTRL(0x%x), SENINF2_MUX_INTSTA(0x%x), SENINF2_MUX_SIZE(0x%x), SENINF2_MUX_DEBUG_2(0x%x), SENINF2_MUX_DEBUG_3(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF2_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF2_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF2_MUX_SIZE), SENINF_READ_REG(pSeninf,SENINF2_MUX_DEBUG_2),SENINF_READ_REG(pSeninf,SENINF2_MUX_DEBUG_3));
            if(SENINF_READ_REG(pSeninf,SENINF2_MUX_INTSTA) & 0x1)
            {
                SENINF_WRITE_REG(pSeninf,SENINF2_MUX_INTSTA,0xffffffff);
                usleep(DEBUG_DELAY);
                LOG_MSG("after reset overrun, SENINF2_MUX_CTRL(0x%x), SENINF2_MUX_INTSTA(0x%x), SENINF2_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF2_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF2_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF2_MUX_DEBUG_2));
            }
            /*Mux3 for HDR*/
            LOG_MSG("SENINF3_MUX_CTRL(0x%x), SENINF3_MUX_INTSTA(0x%x), SENINF3_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF3_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF3_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF3_MUX_DEBUG_2));

            /*Mux4  for PDAF*/
            LOG_MSG("SENINF4_MUX_CTRL(0x%x), SENINF4_MUX_INTSTA(0x%x), SENINF4_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF4_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF4_MUX_DEBUG_2));


            /*Mux5 for HDR*/
            LOG_MSG("SENINF5_MUX_CTRL(0x%x), SENINF5_MUX_INTSTA(0x%x), SENINF5_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF5_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF5_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF5_MUX_DEBUG_2));

            /*Mux6  for PDAF*/
            LOG_MSG("SENINF6_MUX_CTRL(0x%x), SENINF6_MUX_INTSTA(0x%x), SENINF6_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF6_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF6_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF6_MUX_DEBUG_2));
            LOG_MSG("ret = %d", ret);

        }
        break;

    case CMD_SENINF_DEBUG_TASK_CAMSV:
        {
	        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
            LOG_MSG("SENINF_TOP_MUX_CTRL(0x%x) SENINF_TOP_CAM_MUX_CTRL(0x%x)",
                SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF_TOP_CAM_MUX_CTRL));
	        /*Mux3 for HDR*/
	        LOG_MSG("(HDR)SENINF3_MUX_CTRL(0x%x), IRQ(0x%x), WidthHeight(0x%x), Debug(0x%x)",
                    SENINF_READ_REG(pSeninf,SENINF3_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF3_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF3_MUX_DEBUG_2), SENINF_READ_REG(pSeninf,SENINF3_MUX_DEBUG_1));
	        SENINF_WRITE_REG(pSeninf,SENINF3_MUX_INTSTA,0xffffffff);
	        /*Mux4  for PDAF*/
	        LOG_MSG("(PDAF)SENINF4_MUX_CTRL(0x%x), IRQ(0x%x), WidthHeight(0x%x), Debug(0x%x)",
                    SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF4_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF4_MUX_DEBUG_2), SENINF_READ_REG(pSeninf,SENINF4_MUX_DEBUG_1));
	        SENINF_WRITE_REG(pSeninf,SENINF4_MUX_INTSTA,0xffffffff);

             LOG_MSG("SENINF5_MUX_CTRL(0x%x), SENINF5_MUX_INTSTA(0x%x), SENINF5_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF5_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF5_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF5_MUX_DEBUG_2));
            SENINF_WRITE_REG(pSeninf,SENINF5_MUX_INTSTA,0xffffffff);
            /*Mux6  for PDAF*/
            LOG_MSG("SENINF6_MUX_CTRL(0x%x), SENINF6_MUX_INTSTA(0x%x), SENINF6_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF6_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF6_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF6_MUX_DEBUG_2));
            SENINF_WRITE_REG(pSeninf,SENINF6_MUX_INTSTA,0xffffffff);
        }
        break;

    case CMD_SENINF_GET_SENINF_ADDR:
        *(unsigned long *) arg1 = (unsigned long) mpSeninfHwRegAddr;
        break;
    case CMD_SENINF_DEBUG_PIXEL_METER:
        {
            unsigned int * meter_array = (unsigned int *)arg1;
            if(meter_array != NULL) {
                *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x3C/4))|= 1<<31;
                meter_array[0] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x40/4));
                meter_array[1] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x44/4));
                meter_array[2] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x48/4));
                meter_array[3] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x4c/4));


                //*((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x3C))&= ~(1<<31);
            }
        }
            break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfTopMuxCtrl(
    unsigned int seninfMuXIdx, SENINF_ENUM seninfSrc
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        switch (seninfMuXIdx){
        case SENINF_MUX2:
            SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL, SENINF2_MUX_SRC_SEL)= seninfSrc;
            break;
        case SENINF_MUX3:
            SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL, SENINF3_MUX_SRC_SEL)= seninfSrc;
            break;
        case SENINF_MUX4:
            SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL, SENINF4_MUX_SRC_SEL)= seninfSrc;
            break;
        case SENINF_MUX5:
            SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL, SENINF5_MUX_SRC_SEL)= seninfSrc;
            break;
        case SENINF_MUX6:
            SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL, SENINF6_MUX_SRC_SEL)= seninfSrc;
            break;
        default://SENINF_MUX1
            SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL, SENINF1_MUX_SRC_SEL)= seninfSrc;
            break;

    }

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfVC(SENINF_ENUM SenInfsel,
    unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id,
    unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id)
{
        int ret = 0;
        //unsigned int temp = 0;
        seninf_reg_t *pSeninf = NULL;
        pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr [SenInfsel];


        //0x1a04_0a3c for VC0~VC3
        SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_DI,((vc3Id)<<24)|((vc2Id)<<16)|((vc1Id)<<8)|(vc0Id));
        //0x1a04_0af0 for VC4~VC5
        SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_DI_EXT, (vc5Id << 8) | (vc4Id));

        /*Clear*/
        SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_DI_CTRL, 0x00);
        SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_DI_CTRL_EXT, 0x00);

        if((vc0Id == 0) && (vc1Id == 0) && (vc2Id == 0) && (vc3Id == 0) && (vc4Id == 0) && (vc5Id == 0))
        {
            return ret;
        }

        if((vc0Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, VC0_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, DT0_INTERLEAVING) = 1;
        }

        if((vc1Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, VC1_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, DT1_INTERLEAVING) = 1;
        }

        if((vc2Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, VC2_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, DT2_INTERLEAVING) = 1;
        }

        if((vc3Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, VC3_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL, DT3_INTERLEAVING) = 1;
        }

        if((vc4Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL_EXT, VC4_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL_EXT, DT4_INTERLEAVING) = 1;
        }

        if((vc5Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL_EXT, VC5_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_DI_CTRL_EXT, DT5_INTERLEAVING) = 1;
        }
        LOG_MSG("VC Data(0x%x,0x%x) VC Ctrl(0x%x,0x%x)",
            SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI),SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI_EXT),
            SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI_CTRL),SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI_CTRL_EXT));

        return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfMuxCtrl(
    unsigned long Muxsel, unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int PixelMode
)
{
        int ret = 0;
        seninf_mux_reg_t_base *pSeninf = NULL;
        unsigned int temp = 0;


        pSeninf = (seninf_mux_reg_t_base *)mpSeninfMuxBaseAddr[Muxsel];

        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_SRC_SEL) = inSrcTypeSel;

        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL_EXT, SENINF_SRC_SEL_EXT) = (inSrcTypeSel == TEST_MODEL) ? 0 : 1;



        if(1 == PixelMode) { /*2 Pixel*/
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL_EXT, SENINF_PIX_SEL_EXT) = 0;
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_PIX_SEL) = 1;
        }
        else if(2 == PixelMode) { /* 4 Pixel*/
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL_EXT, SENINF_PIX_SEL_EXT) = 1;
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_PIX_SEL) = 0;
        }
        else {/* 1 pixel*/
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL_EXT, SENINF_PIX_SEL_EXT) = 0;
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_PIX_SEL) = 0;

        }

        if(JPEG_FMT != inDataType) {
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FULL_WR_EN) = 2;
        }
        else {
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FULL_WR_EN) = 0;
        }

        if ((CSI2 == inSrcTypeSel)||(MIPI_SENSOR <= inSrcTypeSel)) {
            if(JPEG_FMT != inDataType) {/*Need to use Default for New design*/
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FLUSH_EN) = 0x1B;
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_PUSH_EN) = 0x1F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }
        /*Disable send fifo to cam*/
       // SENINF_BITS(pSeninf, SENINF1_MUX_SPARE, SENINF_FIFO_FULL_SEL) = 0; keep default =1

        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_HSYNC_POL) = hsPol;
        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_VSYNC_POL) = vsPol;

        temp = SENINF_READ_REG(pSeninf, SENINF1_MUX_CTRL);
        SENINF_WRITE_REG(pSeninf,SENINF1_MUX_CTRL, temp|0x3);//reset
        SENINF_WRITE_REG(pSeninf,SENINF1_MUX_CTRL, temp&0xFFFFFFFC);//clear reset
        temp = SENINF_READ_REG(pSeninf, SENINF1_MUX_CTRL_EXT);
        SENINF_WRITE_REG(pSeninf,SENINF1_MUX_CTRL_EXT, temp|0x100);//use overrun detect setting as before ECO
        return ret;

}
/*******************************************************************************
*
********************************************************************************/
#if 0
int SeninfDrvImp::setSeninf1Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;

        /*0x1a04_0200*/
        SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL) = padSel;

        SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; //[15:12]: 0:csi, 8:NCSI2

        if(inSrcTypeSel == 0)
        {
            SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = 1;
            SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_NCSI2_IP_EN) = 0;
        }
        else
        {
            SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = 0;
            SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_NCSI2_IP_EN) = 1;
        }
    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf2Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;

    /*0x1a04_1200*/
    SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF2_CTRL, PAD2CAM_DATA_SEL) = padSel;

    SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; //[15:12]: 0:csi, 8:NCSI2

    if(inSrcTypeSel == 0)
    {
        SENINF_BITS(pSeninf, SENINF2_CTRL_EXT, SENINF_CSI2_IP_EN) = 1;
        SENINF_BITS(pSeninf, SENINF2_CTRL_EXT, SENINF_NCSI2_IP_EN) = 0;
    }
    else
    {
        SENINF_BITS(pSeninf, SENINF2_CTRL_EXT, SENINF_CSI2_IP_EN) = 0;
        SENINF_BITS(pSeninf, SENINF2_CTRL_EXT, SENINF_NCSI2_IP_EN) = 1;
    }



    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf3Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;

    /*0x1a04_2200*/
    SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF3_CTRL, PAD2CAM_DATA_SEL) = padSel;

    SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; //[15:12]: 0:csi, 8:NCSI2

    if(inSrcTypeSel == 0)
    {
        SENINF_BITS(pSeninf, SENINF3_CTRL_EXT, SENINF_CSI2_IP_EN) = 1;
        SENINF_BITS(pSeninf, SENINF3_CTRL_EXT, SENINF_NCSI2_IP_EN) = 0;
    }
    else
    {
        SENINF_BITS(pSeninf, SENINF3_CTRL_EXT, SENINF_CSI2_IP_EN) = 0;
        SENINF_BITS(pSeninf, SENINF3_CTRL_EXT, SENINF_NCSI2_IP_EN) = 1;
    }

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf4Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;

    /*0x1a04_3200*/
    SENINF_BITS(pSeninf, SENINF4_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF4_CTRL, PAD2CAM_DATA_SEL) = padSel;

    SENINF_BITS(pSeninf, SENINF4_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; //[15:12]: 0:csi, 8:NCSI2

    if(inSrcTypeSel == 0)
    {
        SENINF_BITS(pSeninf, SENINF4_CTRL_EXT, SENINF_CSI2_IP_EN) = 1;
        SENINF_BITS(pSeninf, SENINF4_CTRL_EXT, SENINF_NCSI2_IP_EN) = 0;
    }
    else
    {
        SENINF_BITS(pSeninf, SENINF4_CTRL_EXT, SENINF_CSI2_IP_EN) = 0;
        SENINF_BITS(pSeninf, SENINF4_CTRL_EXT, SENINF_NCSI2_IP_EN) = 1;
    }


    return ret;

}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::CSI_TimingDelayCal(unsigned long mode)
{
    int ret = 0;

    (void)mode;
#if 0
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog0ARegAddr;
    unsigned int i = 0, CSI_CLK = 0, delay_code = 0, vref_sel = 0;
    unsigned int CSI_clk = 0;

    if(mode == 1) //Use sensor interface 2  & CSI1
    {
        pSeninf = (seninf_reg_t *)mpSeninf2HwRegAddr;
        pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog1ARegAddr;
    }
    else if(mode == 2) // Use sensor interface 3  & CSI2
    {
        pSeninf = (seninf_reg_t *)mpSeninf3HwRegAddr;
        pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog2RegAddr;
    }
    else // Use sensor interface 1  & CSI0
    {
        pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog0ARegAddr;
    }

        /*Enable sw mode*/
    SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, Delay_APPLY_MODE) = 2;/*RG mode*/
    SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_SW_RST) = 0;/* SW Reset*/
    // Enable CSI2 SW offset calibration
    /* RG_CSI0A_DPHY_L2_DELAYCAL_EN = 1'b1 */
    SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_DELAYCAL_EN) = 1;
    /*RG_CSI0A_DPHY_L2_DELAYCAL_RSTB = 1'b0*/
    SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_DELAYCAL_RSTB) = 0;
    /*RG_CSI0A_DPHY_L2_VREF_SEL[5:0] = 6'b10_0000*/
    vref_sel = 0x20;
    SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_VREF_SEL) = vref_sel;
    /*DA_CSI0A_DPHY_L2_DELAY_EN=1'b1 */
    /*DA_CSI0A_DPHY_L2_DELAY_CODE[5:0] =6'b11_1111 */
    /*DA_CSI0A_DPHY_L2_DELAY_APPLY = 1'b0 */

    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_CODE) = 0x3f;/* set to 0 first */
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;

        SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = 0x3f;/* set to 0 first */
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;

    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_CODE) = 0x3f;/* set to 0 first */
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;

        SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_CODE) = 0x3f;/* set to 0 first */
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;

    /*Apply enable*/
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_CODE) = 0x3f;/* set to 0 first */
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 1;
    usleep(1);
     SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;

    while(SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_DELAYCAL_RSTB)){}
    LOG_ERR("CSI_CLK(%d) vref_sel(0x%x)", SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_VREF_SEL), SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_VREF_SEL));

    //Step3
    for(i=0;i<5;i++)
    {
        ret= ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_CSI_CLK,&CSI_clk);
        if (ret < 0) {
            LOG_ERR("ERROR:KDIMGSENSORIOC_X_GET_CSI_CLK");
            CSI_clk = 0;
            break;
        }
        CSI_clk = CSI_clk /1000;
        if(CSI_CLK < 185)
        {
            vref_sel = 0x30;
        }
        else
        {
            vref_sel = 0x10;
        }
        SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_VREF_SEL) = vref_sel >> i;
        LOG_ERR("CSI_CLK(%d) vref_sel(0x%x)", CSI_CLK, SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_VREF_SEL));

    }
    SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_DELAYCAL_EN) = 0;
    SENINF_BITS(pMipirx, MIPI_RX_ANA10_CSI0A, RG_CSI0A_DPHY_L2_DELAYCAL_RSTB) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
#endif
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::CSI0_OffsetCal(unsigned long mode)
{
    int ret = 0;

    seninf_reg_t *pSeninf = NULL;
    mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[SENINF1_CSI0];
    unsigned int i = 0, temp = 0, out_code = 0, offset_code = 0;

    (void)mode;
#if FIXME

    if(mode == 1) //Use sensor interface 2  & CSI1
    {
        pSeninf = (seninf_reg_t *)mpSeninf2HwRegAddr;
        pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog1RegAddr;
    }
    else if(mode == 2) // Use sensor interface 3  & CSI2
    {
        pSeninf = (seninf_reg_t *)mpSeninf3HwRegAddr;
        pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog2RegAddr;
    }
    else // Use sensor interface 1  & CSI0
    {
        pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog0RegAddr;
    }

    if(((mode == 0) && (CSI0_Offset_Calibration == true))||
        ((mode == 1) && (CSI1_Offset_Calibration == true))||
        ((mode == 2) && (CSI2_Offset_Calibration == true)))
    {
        LOG_MSG("Result! CSI2 OFFSET_CAL CON00(0x%x),CON04(0x%x),CON08(0x%x)!",
        SENINF_READ_REG(pSeninf, MIPI_RX_CON00_CSI0),SENINF_READ_REG(pSeninf, MIPI_RX_CON04_CSI0),
        SENINF_READ_REG(pSeninf, MIPI_RX_CON08_CSI0));
        LOG_MSG("D2(0x%x),D0(0x%x),C0(0x%x),D1(0x%x),D3(0x%x)!",
        SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_OFFSET_CODE),SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_OFFSET_CODE),
        SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_OFFSET_CODE),SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_OFFSET_CODE),
        SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_OFFSET_CODE));
        return ret;
    }


    if(mode == 1){
        CSI1_Offset_Calibration = true;
    }
    else if(mode == 2){
        CSI2_Offset_Calibration = true;
    }
    else {
        CSI0_Offset_Calibration = true;
    }

    // Enable CSI2 SW offset calibration
    SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 1; // 0x1a04_0838[0]= 1'b1;
    SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CAL_MODE) = 1;  // 0x1a04_0838[1]= 1'b1;

    SENINF_WRITE_REG(pSeninf, MIPI_RX_CON3C_CSI0, 0x1541); // 0x1a04_083c[31:0]=32'h1541;

    //D2 calibration
    //RG_CSI0_LNR0_HSRX_CAL_EN
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_CAL_EN) = 1; // 0x1a04_0800
    //out_code =  AD_CSI_CDPHY_L0_T0AB_HSAMP_OS_OUT value
    usleep(1);
    out_code = SENINF_BITS(pMipirx, MIPI_RX_ANA50_CSI0A, AD_CSI_CDPHY_L0_T0AB_HSAMP_OS_OUT);
    if(out_code == 1)
        offset_code = 0x10;
    else
        offset_code = 0x00;

    for(i = 0 ; i < 15 ; i++){

        SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_OFFSET_CODE)= i + offset_code;
        usleep(1);
        if(out_code != SENINF_BITS(pMipirx, MIPI_RX_ANA50_CSI0A, AD_CSI_CDPHY_L0_T0AB_HSAMP_OS_OUT))
            break;
    }
    /*set offset code*/
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_OFFSET_CODE)= i + offset_code- 1;
    /*disable hsrx cal*/
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_CAL_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_CAL_APPLY) = 1;
    //LOG_MSG("SW calibration D2(0x%x)!",SENINF_READ_REG(pSeninf, MIPI_RX_CON00_CSI0));

    //D0 calibration
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_OFFSET_CODE)= 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_CAL_EN) = 1; // 0x1a04_0800
    usleep(1);
    out_code = SENINF_BITS(pMipirx, MIPI_RX_ANA54_CSI0A, AD_CSI_CDPHY_L1_T1AB_HSAMP_OS_OUT);
    if(out_code == 1)
        offset_code = 0x10;
    else
        offset_code = 0x00;

    for(i = 0 ; i < 15 ; i++){

        SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_OFFSET_CODE)= i + offset_code;
        usleep(1);
        if(out_code != SENINF_BITS(pMipirx, MIPI_RX_ANA54_CSI0A, AD_CSI_CDPHY_L1_T1AB_HSAMP_OS_OUT))
            break;
    }
    /*set offset code*/
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_OFFSET_CODE)= i + offset_code - 1;
    /*disable hsrx cal*/
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_CAL_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_CAL_APPLY) = 1;
    //LOG_MSG("SW calibration D0(0x%x)!",SENINF_READ_REG(pSeninf, MIPI_RX_CON00_CSI0));

    //C calibration
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_OFFSET_CODE) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_CAL_EN) = 1; // 0x1a04_0800
    usleep(1);
    out_code = SENINF_BITS(pMipirx, MIPI_RX_ANA58_CSI0A, AD_CSI_CDPHY_L2_T1BC_HSAMP_OS_OUT);
    if(out_code == 1)
        offset_code = 0x10;
    else
        offset_code = 0x00;

    for(i = 0 ; i < 15 ; i++){

        SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_OFFSET_CODE)= i + offset_code;
        usleep(1);
        if(out_code != SENINF_BITS(pMipirx, MIPI_RX_ANA58_CSI0A, AD_CSI_CDPHY_L2_T1BC_HSAMP_OS_OUT))
            break;
    }
    /*set offset code*/
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_OFFSET_CODE)= i + offset_code - 1;
    /*disable hsrx cal*/
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_CAL_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_CAL_APPLY) = 1;
    //LOG_MSG("SW calibration C(0x%x)!",SENINF_READ_REG(pSeninf, MIPI_RX_CON04_CSI0));
    //4-4-2  Use sensor interface 3  & CSI2
    if(mode == 3)
    {
        // Disable CSI2 SW offset calibration
        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 0; // 0x1a04_0838[0]= 1'b1;
        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CAL_MODE) = 1;  // 0x1a04_0838[1]= 1'b1;
        LOG_MSG("CSI2 only support two lane!");
        LOG_MSG("D2(0x%x),D0(0x%x),C0(0x%x),D1(0x%x),D3(0x%x)!",
        SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_OFFSET_CODE),SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_OFFSET_CODE),
        SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_OFFSET_CODE),SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_OFFSET_CODE),
        SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_OFFSET_CODE));
        return ret;
    }

    //D1 calibration
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_OFFSET_CODE) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_CAL_EN) = 1; // 0x1a04_0800
    usleep(1);
    out_code = SENINF_BITS(pMipirx, MIPI_RX_ANA50_CSI0B, AD_CSI_CDPHY_L0_T0AB_HSAMP_OS_OUT);
    if(out_code == 1)
        offset_code = 0x10;
    else
        offset_code = 0x00;

    for(i = 0 ; i < 15 ; i++){

        SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_OFFSET_CODE)= i + offset_code;
        usleep(1);
        if(out_code != SENINF_BITS(pMipirx, MIPI_RX_ANA50_CSI0B, AD_CSI_CDPHY_L0_T0AB_HSAMP_OS_OUT))
            break;
    }
    /*set offset code*/
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_OFFSET_CODE)= i + offset_code - 1;
    /*disable hsrx cal*/
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_CAL_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_CAL_APPLY) = 1;
    //LOG_MSG("SW calibration D1(0x%x)!",SENINF_READ_REG(pSeninf, MIPI_RX_CON04_CSI0));

    //D3 calibration
    SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_OFFSET_CODE)=0;
    SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_CAL_EN) = 1; // 0x1a04_0800
    usleep(1);
    out_code = SENINF_BITS(pMipirx, MIPI_RX_ANA54_CSI0B, AD_CSI_CDPHY_L1_T1AB_HSAMP_OS_OUT);
    if(out_code == 1)
        offset_code = 0x10;
    else
        offset_code = 0x00;

    for(i = 0 ; i < 15 ; i++){

        SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_OFFSET_CODE)= i + offset_code;
        usleep(1);
        if(out_code != SENINF_BITS(pMipirx, MIPI_RX_ANA54_CSI0B, AD_CSI_CDPHY_L1_T1AB_HSAMP_OS_OUT))
            break;
    }
    /*set offset code*/
    SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_OFFSET_CODE)= i + offset_code - 1;
    /*disable hsrx cal*/
    SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_CAL_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_CAL_APPLY) = 1;
    //LOG_MSG("SW calibration D3(0x%x)!",SENINF_READ_REG(pSeninf, MIPI_RX_CON08_CSI0));
    // Disable CSI2 SW offset calibration
    SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 0; // 0x1a04_0838[0]= 1'b1;
    SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CAL_MODE) = 1;  // 0x1a04_0838[1]= 1'b1;

    LOG_MSG("CSI2 OFFSET_CAL CON00(0x%x),CON04(0x%x),CON08(0x%x)!",
    SENINF_READ_REG(pSeninf, MIPI_RX_CON00_CSI0),SENINF_READ_REG(pSeninf, MIPI_RX_CON04_CSI0),
    SENINF_READ_REG(pSeninf, MIPI_RX_CON08_CSI0));
    LOG_MSG("D2(0x%x),D0(0x%x),C0(0x%x),D1(0x%x),D3(0x%x)!",
    SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR0_HSRX_OFFSET_CODE),SENINF_BITS(pSeninf, MIPI_RX_CON00_CSI0, RG_CSI0_LNR3_HSRX_OFFSET_CODE),
    SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR5_HSRX_OFFSET_CODE),SENINF_BITS(pSeninf, MIPI_RX_CON04_CSI0, RG_CSI0_LNR6_HSRX_OFFSET_CODE),
    SENINF_BITS(pSeninf, MIPI_RX_CON08_CSI0, RG_CSI0_LNR9_HSRX_OFFSET_CODE));
#endif

    return ret;
}
#endif
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::Efuse(unsigned long csi_sel)
{
    int ret = 0;
     if ((mCSI[0] != 0) && (csi_sel == 0)) {
        mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[SENINF1_CSI0];

        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0A, RG_CSI0A_L0P_T0A_HSRT_CODE) = (mCSI[0]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0A, RG_CSI0A_L0N_T0B_HSRT_CODE) = (mCSI[0]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0A, RG_CSI0A_L1P_T0C_HSRT_CODE) = (mCSI[0]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0A, RG_CSI0A_L1N_T1A_HSRT_CODE) = (mCSI[0]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI0A, RG_CSI0A_L2P_T1B_HSRT_CODE) = (mCSI[0]>>17) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI0A, RG_CSI0A_L2N_T1C_HSRT_CODE) = (mCSI[0]>>17) & 0x1f;

        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0B, RG_CSI0B_L0P_T0A_HSRT_CODE) = (mCSI[0]>>12) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0B, RG_CSI0B_L0N_T0B_HSRT_CODE) = (mCSI[0]>>12) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0B, RG_CSI0B_L1P_T0C_HSRT_CODE) = (mCSI[0]>>7) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI0B, RG_CSI0B_L1N_T1A_HSRT_CODE) = (mCSI[0]>>7) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI0B, RG_CSI0B_L2P_T1B_HSRT_CODE) = (mCSI[0]>>2) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI0B, RG_CSI0B_L2N_T1C_HSRT_CODE) = (mCSI[0]>>2) & 0x1f;


        LOG_MSG("CSI0 MIPI_RX_ANA08_CSI0A(0x%x) MIPI_RX_ANA0C_CSI0A(0x%x) MIPI_RX_ANA08_CSI0B(0x%x) MIPI_RX_ANA0C_CSI0B(0x%x)", SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI0A),
            SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI0A), SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI0B), SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI0B));
    }

    if ((mCSI[1] != 0)&&(mCSI[2] != 0) && (csi_sel == 1))
    {
        mipi_ana_dphy_reg_t *pMipirx = (mipi_ana_dphy_reg_t *)mpCSI2RxAnaBaseAddr[SENINF3_CSI1];
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L0P_HSRT_CODE) = (mCSI[1]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L0N_HSRT_CODE) = (mCSI[1]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L1P_HSRT_CODE) = (mCSI[1]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L1N_HSRT_CODE) = (mCSI[1]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1A, RG_CSI1A_L2P_HSRT_CODE) = (mCSI[1]>>17) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1A, RG_CSI1A_L2N_HSRT_CODE) = (mCSI[1]>>17) & 0x1f;

        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L0P_HSRT_CODE) = (mCSI[2]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L0N_HSRT_CODE) = (mCSI[2]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L1P_HSRT_CODE) = (mCSI[2]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L1N_HSRT_CODE) = (mCSI[2]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2P_HSRT_CODE) = (mCSI[2]>>17) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2N_HSRT_CODE) = (mCSI[2]>>17) & 0x1f;

        LOG_MSG("CSI1 ANA08_CSI1A(0x%x) ANA0C_CSI1A(0x%x) ANA08_CSI1B(0x%x) ANA0C_CSI1B(0x%x)", SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1A),
            SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1A), SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1B), SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1B));
    }

    if((mCSI[2] != 0)&&(mCSI[3] != 0) && (csi_sel == 2))
    {
        mipi_ana_dphy_reg_t *pMipirx = (mipi_ana_dphy_reg_t *)mpCSI2RxAnaBaseAddr[SENINF5_CSI2];

        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L0P_HSRT_CODE) = (mCSI[2]>>12) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L0N_HSRT_CODE) = (mCSI[2]>>12) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L1P_HSRT_CODE) = (mCSI[2]>>7) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L1N_HSRT_CODE) = (mCSI[2]>>7) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1A, RG_CSI1A_L2P_HSRT_CODE) = (mCSI[2]>>2) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1A, RG_CSI1A_L2N_HSRT_CODE) = (mCSI[2]>>2) & 0x1f;

        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L0P_HSRT_CODE) = (mCSI[3]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L0N_HSRT_CODE) = (mCSI[3]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L1P_HSRT_CODE) = (mCSI[3]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L1N_HSRT_CODE) = (mCSI[3]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2P_HSRT_CODE) = (mCSI[3]>>20) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2N_HSRT_CODE) = (mCSI[3]>>20) & 0x1f;

        LOG_MSG("CSI2 ANA08_CSI2A(0x%x) ANA0C_CSI2A(0x%x) ANA08_CSI2B(0x%x) ANA0C_CSI2B(0x%x)", SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1A),
                   SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1A), SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1B), SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1B));

    }

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf1CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlaneNum, unsigned long Enable,
    unsigned long dataheaderOrder, unsigned long mipi_type,
    unsigned long HSRXDE, unsigned long dpcm
)
{

    int ret = 0;
//please use setSeninfCSI2; this only support disable
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[SENINF1_CSI0];
    unsigned int temp = 0;

    (void)dataTermDelay;
    (void)dataSettleDelay;
    (void)clkTermDelay;
    (void)vsyncType;
    (void)dlaneNum;
    (void)dataheaderOrder;
    (void)mipi_type;
    (void)HSRXDE;
    (void)dpcm;

    //GPIO config & MIPI RX config, CSI2 enable
    if(Enable == 1) {

    }
    else {
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first
         temp = SENINF_READ_REG(pSeninf, SENINF2_CSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF2_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first
        SENINF_BITS(pSeninf, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = 0;

        //disable mipi BG
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_LPF_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_LPF_EN) = 0;

    }
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf2CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlaneNum, unsigned long Enable,
    unsigned long dataheaderOrder, unsigned long mipi_type,
    unsigned long HSRXDE, unsigned long dpcm
)
{
     int ret = 0;

     seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
     mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[SENINF1_CSI0];
     unsigned int temp = 0;

     (void)dataTermDelay;
     (void)dataSettleDelay;
     (void)clkTermDelay;
     (void)vsyncType;
     (void)dlaneNum;
     (void)dataheaderOrder;
     (void)mipi_type;
     (void)HSRXDE;
     (void)dpcm;

     //GPIO config & MIPI RX config, CSI2 enable
     if(Enable == 1) {

     }
     else {

         // disable CSI2
         temp = SENINF_READ_REG(pSeninf, SENINF3_CSI2_CTL);
         SENINF_WRITE_REG(pSeninf, SENINF3_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first

         temp = SENINF_READ_REG(pSeninf, SENINF4_CSI2_CTL);
         SENINF_WRITE_REG(pSeninf, SENINF4_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first
         SENINF_BITS(pSeninf, SENINF_TOP_PHY_SENINF_CTL_CSI1, phy_seninf_lane_mux_csi1_en) = 0;

         //disable mipi BG
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI1A, RG_CSI1A_BG_CORE_EN) = 0;
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI1B, RG_CSI1B_BG_CORE_EN) = 0;
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI1A, RG_CSI1A_BG_LPF_EN) = 0;
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI1B, RG_CSI1B_BG_LPF_EN) = 0;


     }
     return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf3CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlaneNum, unsigned long Enable,
    unsigned long dataheaderOrder, unsigned long mipi_type,
    unsigned long HSRXDE, unsigned long dpcm
)
{
    int ret = 0;

    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[SENINF1_CSI0];
    unsigned int temp = 0;

    (void)dataTermDelay;
    (void)dataSettleDelay;
    (void)clkTermDelay;
    (void)vsyncType;
    (void)dataheaderOrder;
    (void)mipi_type;
    (void)HSRXDE;
    (void)dpcm;

    if(dlaneNum >2 )
    {
        LOG_ERR("CSI2-2 Only support 2Lane !");

    }
    //GPIO config & MIPI RX config, CSI2 enable
    if(Enable == 1) {

    } else{
        temp = SENINF_READ_REG(pSeninf, SENINF5_CSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF5_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first
        SENINF_BITS(pSeninf, SENINF_TOP_PHY_SENINF_CTL_CSI2, phy_seninf_lane_mux_csi2_en) = 0;

        //disable mipi BG
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2A, RG_CSI2A_BG_CORE_EN) = 0;
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2B, RG_CSI2B_BG_CORE_EN) = 0;
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2A, RG_CSI2A_BG_LPF_EN) = 0;
         SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2B, RG_CSI2B_BG_LPF_EN) = 0;

    }
    return ret;

}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf4CSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlaneNum, unsigned long Enable,
    unsigned long dataheaderOrder, unsigned long mipi_type,
    unsigned long HSRXDE, unsigned long dpcm
)
{
    int ret = 0;

    (void)dataTermDelay;
    (void)dataSettleDelay;
    (void)clkTermDelay;
    (void)vsyncType;
    (void)dlaneNum;
    (void)Enable;
    (void)dataheaderOrder;
    (void)mipi_type;
    (void)HSRXDE;
    (void)dpcm;
#if 0//only 3 CSI
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnalog0ARegAddr;
    unsigned int temp = 0;
    unsigned int dpcm_set = 0;
    unsigned int Isp_clk = 0;
    unsigned int  msettleDelay = 0;

    //GPIO config & MIPI RX config, CSI2 enable
    if(Enable == 1) {
    }
    else {
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF4_CSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF4_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first

        //disable mipi BG
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2A, RG_CSI2A_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2A, RG_CSI2A_BG_LPF_EN) = 0;

        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2B, RG_CSI2B_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI2B, RG_CSI2B_BG_LPF_EN) = 0;
    }

#endif
    return ret;

}

int SeninfDrvImp::getCSIpara(SENINF_CSI_PARA *pSeninfpara){


    if(pSeninfpara->mipiPad == NSCamCustomSensor::EMipiPort_CSI0){
        pSeninfpara->seninfSrc = SENINF_1;
        pSeninfpara->CSI2_IP  = SENINF1_CSI0;
    }
    else if(pSeninfpara->mipiPad == NSCamCustomSensor::EMipiPort_CSI1){
        pSeninfpara->seninfSrc = SENINF_3;
        pSeninfpara->CSI2_IP  = SENINF3_CSI1;
    }
    else if(pSeninfpara->mipiPad == NSCamCustomSensor::EMipiPort_CSI2){
        pSeninfpara->seninfSrc = SENINF_5;
        pSeninfpara->CSI2_IP  = SENINF5_CSI2;
    }
    else if(pSeninfpara->mipiPad == NSCamCustomSensor::EMipiPort_CSI0A){
        pSeninfpara->seninfSrc = SENINF_1;
        pSeninfpara->CSI2_IP  = SENINF1_CSI0A;
    }
    else if(pSeninfpara->mipiPad == NSCamCustomSensor::EMipiPort_CSI0B){
        pSeninfpara->seninfSrc = SENINF_2;
        pSeninfpara->CSI2_IP  = SENINF2_CSI0B;
#if 0
    }
    else if(csiIndex == NSCamCustomSensor::EMipiPort_CSI1A){
        pSeninfpara->seninfSrc = SENINF_3;
        pSeninfpara->CSI2_IP  = SENINF3_CSI1A;
    }
    else if(csiIndex == NSCamCustomSensor::EMipiPort_CSI1B){
        pSeninfpara->seninfSrc = SENINF_4;
        pSeninfpara->CSI2_IP  = SENINF4_CSI1B;
#endif
    } else {
        pSeninfpara->seninfSrc = SENINF_1;
        pSeninfpara->CSI2_IP  = SENINF1_CSI0;
        LOG_ERR("camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp", pSeninfpara->mipiPad);
    }
    return 0;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfCSI2(SENINF_CONFIG_STRUCT *pSeninfPara, bool enable)
{
    int ret = 0;
    seninf_reg_t *pSeninf = NULL;//(seninf_reg_t *)mpSeninfHwRegAddr;
    seninf_reg_t *pSeninf_base=(seninf_reg_t *)mpSeninfHwRegAddr;
    mipi_ana_reg_t *pMipirx = NULL;//(mipi_ana_reg_t_base *)mpCSI2RxAnalog0RegAddr;
    mipi_ana_reg_t *pMipiRx_base = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[SENINF1_CSI0];
    mipi_rx_con_reg_base *pMipiRxConf = NULL;
    unsigned int temp = 0, Isp_clk = 0, msettleDelay = 0;
    unsigned int dataTermDelay, dataSettleDelay, clkTermDelay, vsyncType, dlaneNum;
    unsigned int dataheaderOrder, mipi_type, HSRXDE, dpcm;
    unsigned int padSel, inSrcTypeSel, CsiSel,mipiskew;
    int i = 0;
    /*ISP clock get from metter*/
    ISP_CLK_GET = 0;

    padSel          = pSeninfPara->padSel;
    inSrcTypeSel    = pSeninfPara->inSrcTypeSel;
    dataTermDelay   = pSeninfPara->csi_para.dataTermDelay;
    dataSettleDelay = pSeninfPara->csi_para.dataSettleDelay;
    clkTermDelay    = pSeninfPara->csi_para.clkTermDelay;
    vsyncType       = pSeninfPara->csi_para.vsyncType;
    dlaneNum        = pSeninfPara->csi_para.dlaneNum;
    dataheaderOrder = pSeninfPara->csi_para.dataHeaderOrder;
    mipi_type       = pSeninfPara->csi_para.mipi_type;
    HSRXDE          = pSeninfPara->csi_para.HSRXDE;
    dpcm            = pSeninfPara->csi_para.dpcm;
    mipiskew        = pSeninfPara->csi_para.mipi_deskew;
    pSeninf         = (seninf_reg_t *)mpSeninfHwRegAddr;
    SENINF_CSI2_IP_ENUM CSI2_IP = pSeninfPara->csi_para.CSI2_IP;

    pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pSeninfPara->csi_para.seninfSrc];
    pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[CSI2_IP];
    pMipiRxConf = (mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[pSeninfPara->csi_para.seninfSrc];

    if(CSI2_IP == SENINF3_CSI1) //Use sensor interface 2  & CSI1
    {

        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, phy_seninf_lane_mux_csi1_en) = enable;
        CsiSel = 1;
    }
    else if(CSI2_IP == SENINF5_CSI2) // Use sensor interface 4  & CSI2
    {
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI2, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI2, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI2, phy_seninf_lane_mux_csi2_en) = enable;
        CsiSel = 2;
    }
    else if(CSI2_IP == SENINF1_CSI0)
    {
        CsiSel = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = enable;
    }
    else if(CSI2_IP == SENINF1_CSI0A)
    {

        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = enable;
        CsiSel = 0;
    }
    else if(CSI2_IP == SENINF2_CSI0B)
    {
        CsiSel = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = enable;
    }
    else if(CSI2_IP == SENINF3_CSI1A)
    {

        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, phy_seninf_lane_mux_csi1_en) = enable;
        CsiSel = 1;
    }
    else if(CSI2_IP == SENINF4_CSI1B)
    {
        CsiSel = 1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, phy_seninf_lane_mux_csi1_en) = enable;
    }
    else {
        LOG_ERR("unsupported CSI configuration");//should never here, handle as SENINF1_CSI0

        CsiSel = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = enable;
    }

    LOG_MSG("mipi_type %d CSI2_IP %d", mipi_type, CSI2_IP);


/*
MIPI_OPHY_NCSI2 = 0,
MIPI_OPHY_CSI2  = 1,
MIPI_CPHY       = 2,
*/
    /*First Enable Sensor interface and select pad (0x1a04_0200)*/
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = enable;//(SENINF_CTL_BASE) |= 0x1;
    SENINF_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL) = padSel;
    if((mipi_type == CSI2_1_5G) || (mipi_type == CSI2_2_5G))// For CSI2(2.5G) support  & CPHY Support
    {
        SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = 0; //fix for non-mipi sensor 1 ->test mode, 3 parallel sesnor
        SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = enable;//(SENINF_CTL_BASE+0x04) |= 0x40
        SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_NCSI2_IP_EN) = 0;
    }


    /*GPIO config & MIPI RX config, CSI2 enable*/
    if(enable == 1) {
       // if((seninf->CSI2_IP == SENINF1_CSI0) || (seninf->CSI2_IP == SENINF3_CSI1) ||(seninf->CSI2_IP == SENINF5_CSI2))
        {
            if(mipi_type != CSI2_2_5G_CPHY) //Dphy
            {

                /* set analog phy mode to DPHY */

                if (IS_4D1C) {//4D1C (MIPIRX_ANALOG_A_BASE) = 0x00001A42
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L0_CKMODE_EN) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L0_CKSEL) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L1_CKMODE_EN) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L1_CKSEL) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L2_CKMODE_EN) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L2_CKSEL) = 1;
                } else {//(MIPIRX_ANALOG_BASE) = 0x102;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L0_CKMODE_EN) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L0_CKSEL) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L1_CKMODE_EN) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L1_CKSEL) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L2_CKMODE_EN) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L2_CKSEL) = 0;
                }

                if (IS_4D1C) {//only 4d1c need set CSIB (MIPIRX_ANALOG_B_BASE) = 0x00001242
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L0_CKMODE_EN) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L0_CKSEL) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L1_CKMODE_EN) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L1_CKSEL) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L2_CKMODE_EN) = 0;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L2_CKSEL) = 1;
                }

                /* byte clock invert*/
                if(IS_CDPHY_COMBO) {
                    SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_CDPHY_L0_T0_BYTECK_INVERT) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_DPHY_L1_BYTECK_INVERT) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_CDPHY_L2_T1_BYTECK_INVERT) = 1;
                    if (IS_4D1C) {
                        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_CDPHY_L0_T0_BYTECK_INVERT) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_DPHY_L1_BYTECK_INVERT) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_CDPHY_L2_T1_BYTECK_INVERT) = 1;
                    }
                } else {
                    mipi_ana_dphy_reg_t *pDphy_ana_base = (mipi_ana_dphy_reg_t *)pMipirx;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANAA8_CSI1A, RG_CSI1A_DPHY_L0_BYTECK_INVERT) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANAA8_CSI1A, RG_CSI1A_DPHY_L1_BYTECK_INVERT) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANAA8_CSI1A, RG_CSI1A_DPHY_L2_BYTECK_INVERT) = 1;
                    if (IS_4D1C) {
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANAA8_CSI1B, RG_CSI1B_DPHY_L0_BYTECK_INVERT) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANAA8_CSI1B, RG_CSI1B_DPHY_L0_BYTECK_INVERT) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANAA8_CSI1B, RG_CSI1B_DPHY_L0_BYTECK_INVERT) = 1;
                    }

                }

                /*start ANA EQ tuning*/
                if(IS_CDPHY_COMBO) {
                    SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_L0_T0AB_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
                    SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_L0_T0AB_EQ_BW) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_L1_T1AB_EQ_IS) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_L1_T1AB_EQ_BW) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_L2_T1BC_EQ_IS) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_L2_T1BC_EQ_BW) = 1;
                    if (IS_4D1C) {//4d1c
                        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_L0_T0AB_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
                        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_L0_T0AB_EQ_BW) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_L1_T1AB_EQ_IS) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_L1_T1AB_EQ_BW) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_L2_T1BC_EQ_IS) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_L2_T1BC_EQ_BW) = 1;
                    }
                } else {
                    mipi_ana_dphy_reg_t *pDphy_ana_base = (mipi_ana_dphy_reg_t *)pMipirx;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1A, RG_CSI1A_L0_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1A, RG_CSI1A_L0_EQ_BW) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1A, RG_CSI1A_L1_EQ_IS) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1A, RG_CSI1A_L1_EQ_BW) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA1C_CSI1A, RG_CSI1A_L2_EQ_IS) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA1C_CSI1A, RG_CSI1A_L2_EQ_BW) = 1;
                    if (IS_4D1C) {//4d1c
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1B, RG_CSI1B_L0_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1B, RG_CSI1B_L0_EQ_BW) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1B, RG_CSI1B_L1_EQ_IS) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1B, RG_CSI1B_L1_EQ_BW) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA1C_CSI1B, RG_CSI1B_L2_EQ_IS) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA1C_CSI1B, RG_CSI1B_L2_EQ_BW) = 1;
                    }
                }
                /*end ANA EQ tuning*/
                SENINF_WRITE_REG(pMipiRx_base, MIPI_RX_ANA40_CSI0A, 0x90); // MIPIRX_ANALOG_0A_BASE+0x40) =0x0090;



                SENINF_BITS(pMipirx, MIPI_RX_ANA24_CSI0A, RG_CSI0A_RESERVE) = 0x40;//(MIPIRX_ANALOG_A_BASE+0x24) |=0x40000000;
                if (IS_4D1C)
                    SENINF_BITS(pMipirx, MIPI_RX_ANA24_CSI0B, RG_CSI0B_RESERVE) = 0x40;//(MIPIRX_ANALOG_B_BASE+0x24) |=0x40000000;

                SENINF_BITS(pMipirx, MIPI_RX_WRAPPER80_CSI0A, CSR_CSI_RST_MODE) = 0;//(MIPIRX_ANALOG_BASE+0x80) &=0xF0FFFFFF;
                if (IS_4D1C)
                    SENINF_BITS(pMipirx, MIPI_RX_WRAPPER80_CSI0B, CSR_CSI_RST_MODE) = 0;//(MIPIRX_ANALOG_B_BASE+0x80) &=0xF0FFFFFF;


#ifdef CSI2_EFUSE_SET
                /*Read Efuse value : termination control registers*/
                Efuse(CsiSel);
#endif
                /*ANA power on*/
                SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_CORE_EN) = 1;//(MIPIRX_ANALOG_A_BASE) |= 0x8
                if (IS_4D1C)
                    SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_CORE_EN) = 1;
                usleep(30);

                SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_LPF_EN) = 1;//(MIPIRX_ANALOG_B_BASE) |= 0x4
                if (IS_4D1C)
                   SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_LPF_EN) = 1;
                usleep(1);

#ifdef CSI2_SW_OFFSET_CAL
               /*ANA offset calibration
                        (MIPIRX_ANALOG_A_BASE+0x18) |=(0x1<<0);
                        (MIPIRX_ANALOG_A_BASE+0x1C) |=(0x1<<16);
                        (MIPIRX_ANALOG_A_BASE+0x20) |=(0x1<<16);
                        */

                if (IS_CDPHY_COMBO) {
                    SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_L0_T0AB_EQ_OS_CAL_EN) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_L1_T1AB_EQ_OS_CAL_EN) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_L2_T1BC_EQ_OS_CAL_EN) = 1;
                    if (IS_4D1C) {
                        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_L0_T0AB_EQ_OS_CAL_EN) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_L1_T1AB_EQ_OS_CAL_EN) = 1;
                        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_L2_T1BC_EQ_OS_CAL_EN) = 1;
                    }
                } else {
                    mipi_ana_dphy_reg_t *pDphy_ana_base = (mipi_ana_dphy_reg_t *)pMipirx;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1A, RG_CSI1A_L0_EQ_OS_CAL_EN) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1A, RG_CSI1A_L1_EQ_OS_CAL_EN) = 1;
                    SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA1C_CSI1A, RG_CSI1A_L2_EQ_OS_CAL_EN) = 1;
                    if (IS_4D1C) {
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1B, RG_CSI1B_L0_EQ_OS_CAL_EN) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA18_CSI1B, RG_CSI1B_L1_EQ_OS_CAL_EN) = 1;
                        SENINF_BITS(pDphy_ana_base, MIPI_RX_ANA1C_CSI1B, RG_CSI1B_L2_EQ_OS_CAL_EN) = 1;
                    }
                }
                usleep(1);

                int status =0;
                while(1) {
                    status = SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0A);
                    if((IS_CDPHY_COMBO) && (status&0x29))
                        break;
                    else if(status&0x38)//CSI1,CSI2 completed status bits located at different offset
                        break;
                    else
                        LOG_MSG("CSIA offset calibration ongoing %x",status);
                    i++;
                    if(i>100) {
                        LOG_MSG("CSIA offset calibration timeout");
                        break;
                    }
                    usleep(30);
                }

                if (IS_4D1C) {
                    i = 0;
                    status = 0;
                    while(1) {
                        status = SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0B);
                        if((IS_CDPHY_COMBO) && (status&0x29))
                            break;
                        else if(status&0x38)//CSI1,CSI2 completed status bits located at different offset
                            break;

                        else
                           LOG_MSG("CSIB offset calibration ongoing %x",status);
                        i++;
                        if(i>100) {
                            LOG_MSG("CSIB offset calibration timeout");
                            break;
                        }
                        usleep(30);
                    }
                }
                LOG_MSG("CSI%d offset calibration end MIPI_RX_ANA48_CSI%dA 0x%x, MIPI_RX_ANA48_CSI%dB 0x%x", CsiSel, CsiSel,
                    SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0A),CsiSel, SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0B));
#endif

                if( IS_4D1C ){//4d1c (MIPIRX_CONFIG_CSI_BASE) =0xC9000000;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN0_MUX) = 1;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN1_MUX) = 2;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN2_MUX) = 0;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN3_MUX) = 3;
                }
                else {//2d1c (MIPIRX_CONFIG_CSI_BASE) =0xE4000000;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN0_MUX) = 0;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN1_MUX) = 1;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN2_MUX) = 2;
                    SENINF_BITS(pMipiRxConf, MIPI_RX_CON24_CSI0, CSI0_BIST_LN3_MUX) = 3;
                }

                usleep(30);
                /*D-PHY SW Delay Line calibration*/
#ifdef CSI2_TIMING_DELAY
                CSI_TimingDelayCal(0);
#endif
            }
            else /*Cphy  setting for CSI0 */
            {
                LOG_ERR("CPHY not supported!!!");

            }

        }

        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.seninf.settle_delay_manual", value, "0");
        int settle_delay_manual = atoi(value);
        if(settle_delay_manual) {
            property_get("vendor.debug.seninf.settle_delay", value, "85");
            int settleDelay_user = atoi(value);
            /*End of CSI MIPI*/
            if(settleDelay_user != 85)
                msettleDelay = settleDelay_user;
            else if(dataSettleDelay != 85)
                msettleDelay = dataSettleDelay;
            else
                msettleDelay = SETTLE_DELAY;
        } else
            msettleDelay = SETTLE_DELAY;
        //DPCM Enable
        switch(dpcm)
        {
            case 0x30:
                temp = 1<<7;
                break;
            case 0x31:
                temp = 1<<8;
                break;
            case 0x32:
                temp = 1<<9;
                break;
            case 0x33:
                temp = 1<<10;
                break;
            case 0x34:
                temp = 1<<11;
                break;
            case 0x35:
                temp = 1<<12;
                break;
            case 0x36:
                temp = 1<<13;
                break;
            case 0x37:
                temp = 1<<14;
                break;
            case 0x2a:
                temp = 1<<15;
                break;
            default :
                temp = 0x00;
                break;
        }
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DPCM, temp);//CSI2 must be slected when sensor clk output

        LOG_MSG("CSI2-%d TermDelay:%d SettleDelay:%d(cnt:%d) TermDelay:%d Vsync:%d LaneNum:%d NCSI2_EN:%d HeadOrder:%d ISP_clk:%d dpcm:%d skew:%d",
            CsiSel, (int) dataTermDelay, (int) dataSettleDelay, (int)msettleDelay, (int)clkTermDelay, (int)vsyncType, (int)(dlaneNum+1),
            (int)enable, (int)dataheaderOrder, (int)Isp_clk,(int)dpcm,(int)mipiskew);

            /*Settle delay*/
            temp = (msettleDelay&0xFF)<<8;
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_LNRD_TIMING, temp);
            /*CSI2 control*/
            if(mipi_type != CSI2_2_5G_CPHY) /*DPhy*/
            {
                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, CLOCK_HS_OPTION) = 0;
                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, HSRX_DET_EN) = 0;
#if 0
#ifdef ANALOG_SYNC
                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, SYNC_DET_EN) = 0;
                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, REF_SYNC_DET_EN) = 1;
#else
                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, ASYNC_FIFO_RST_SCH) = 2;
                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, SYNC_DET_EN) = 1;
                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, REF_SYNC_DET_EN) = 0;
#endif
#endif
                temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL);
                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL,(temp|(dataheaderOrder<<16)|(enable<<4)|(((1<<(dlaneNum+1))-1)))) ;

                SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, HS_TRAIL_EN) = 1;
                SENINF_BITS(pSeninf, SENINF1_CSI2_HS_TRAIL, HS_TRAIL_PARAMETER) = HS_TRAIL_VALUE;

                SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, BYPASS_LANE_RESYNC) = 0;
                SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, CDPHY_SEL) = 0;
                SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, CPHY_LANE_RESYNC_CNT) = 3;//(SENINF_CSI2_BASE+0x74) = 3;
                SENINF_BITS(pSeninf, SENINF1_CSI2_MODE, csr_csi2_mode) = 0;
                SENINF_BITS(pSeninf, SENINF1_CSI2_MODE, csr_csi2_header_len) = 0;

                //
                SENINF_BITS(pSeninf, SENINF1_CSI2_DPHY_SYNC, SYNC_SEQ_MASK_0) = 0xff00;
                SENINF_BITS(pSeninf, SENINF1_CSI2_DPHY_SYNC, SYNC_SEQ_PAT_0) = 0x001d;
                //

                temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_SPARE0);

                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_SPARE0,temp&0xFFFFFFFE);//(SENINF_CSI2_BASE+0x20) &= 0xFFFFFFFE;

                }
            else /*CPhy*/
            {
                LOG_ERR("CPHY not supported!!!");

            }
#if 0//need fine tune deskew
            if(mipiskew)
            {
                /*Enable HW Auto deskew function*/
                /* deskew Lane number control : need open by [dlaneNum]*/
                /*Need alway enable *DELAY_EN whed deskew funciton on*/
                if(dlaneNum == 2)
                {
                    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 0;
                    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 0;
                    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 1;
                }
                else
                {
                    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 1;
                }


                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_ENABLE) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_CSI2_RST_ENABLE) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_IP_SEL) = 0;
                // delay mode: 0:hw, 1:sw
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, Delay_APPLY_MODE) = 0;
                // deskew delay apply mode
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_DELAY_APPLY_MODE) = 5;
                // deskew triggle mode
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_TRIGGER_MODE) = 4;
                // EXPECTED_SYNC_CODE
                //SENINF_WRITE_BITS(pSeninf, MIPI_RX_CONB4_CSI0, SYNC_CODE_MASK,0xffff);
                //SENINF_WRITE_BITS(pSeninf, MIPI_RX_CONB4_CSI0, EXPECTED_SYNC_CODE, 0xffff);
                SENINF_WRITE_REG(pSeninf, MIPI_RX_CONB4_CSI0,0xffffffff);
                // Time out
                //SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_TIME_OUT_EN) = 1;
                //SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_SETUP_TIME) = 0xff;
                SENINF_WRITE_REG(pSeninf, MIPI_RX_CONB8_CSI0,0x1ff0000);
                // deskew detect mode
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_DETECTION_MODE) = 0;
                // deskew ACC mode
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_ACC_MODE) = 0;
                // deskew IRQ enable
                //SENINF_BITS(pSeninf, MIPI_RX_CONC0_CSI0, DESKEW_INTERRUPT_W1C_EN) = 1;
                //SENINF_BITS(pSeninf, MIPI_RX_CONC0_CSI0, DESKEW_INTERRUPT_ENABLE) = 0x0f;
                SENINF_WRITE_REG(pSeninf, MIPI_RX_CONC0_CSI0,0x800000ff);
                // deskew Lane number(choose calibrartoin lane number)
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_LANE_NUMBER) = 0;
                //Skew pattern period ~= 2^12 UI
                SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_SETUP_TIME) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_HOLD_TIME) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_DETECTION_CNT) = 4;
                //DESKEW_DELAY_LENGTH
                SENINF_BITS(pSeninf, MIPI_RX_COND0_CSI0, DESKEW_DELAY_LENGTH) = 0x0f;
                /*0: done, 1: timeout, 2: ffff detection, 3: sw delay apply successfully(sw mode used)*/
                /*5: b8 detection*/
                LOG_MSG("CSI2 HW Deskew IRQ = 0x%x",SENINF_READ_REG(pSeninf, MIPI_RX_CONC4_CSI0));
            }
            else
            {
                SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 0;
            }
#endif
             /* set debug port to output packet number */
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DGB_SEL,0x8000001A);
            /*Enable CSI2 IRQ mask*/
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN,0xFFFFFFFF);//turn on all interrupt
            /*write clear CSI2 IRQ*/
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_STATUS,0xFFFFFFFF);
            /*Enable CSI2 Extend IRQ mask*/
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN_EXT,0x0000001f);//turn on all interrupt
            //LOG_MSG("SENINF1_CSI2_CTL(0x1a04_0a00 = 0x%x",SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL));
    }
    else {
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first
        //disable mipi BG
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_LPF_EN) = 0;
        if (IS_4D1C) {
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_CORE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_LPF_EN) = 0;
        }
    }

    return ret;

}
/*******************************************************************************
*
********************************************************************************/
#if 0
int SeninfDrvImp::setSeninfCSI2_CPHY(SENINF_CSI_PARA *seninf)
{
    int ret = 0;

    (void)seninf;
#if 0
    seninf_reg_t_base *pSeninf = NULL;//(seninf_reg_t *)mpSeninfHwRegAddr;
    mipi_ana_reg_t_base *pMipirx = NULL;//(mipi_ana_reg_t_base *)mpCSI2RxAnalog0RegAddr;
    mipi_ana_reg_t *pMipirx_all = NULL;//(mipi_ana_reg_t *)mpCSI2RxAnalog0RegAddr;
    unsigned int temp = 0, dpcm_set = 0, Isp_clk = 0, msettleDelay = 0;
    unsigned int dataTermDelay, dataSettleDelay, clkTermDelay, vsyncType, dlaneNum, Enable;
    unsigned int dataheaderOrder, mipi_type, HSRXDE, dpcm;
    unsigned int padSel, inSrcTypeSel, CalSel,mipiskew;

    padSel          = seninf->padSel;
    inSrcTypeSel    = seninf->inSrcTypeSel;
    dataTermDelay   = seninf->dataTermDelay;
    dataSettleDelay = seninf->dataSettleDelay;
    clkTermDelay    = seninf->clkTermDelay;
    vsyncType       = seninf->vsyncType;
    dlaneNum        = seninf->dlaneNum;
    Enable          = seninf->Enable;
    dataheaderOrder = seninf->dataheaderOrder;
    mipi_type       = seninf->mipi_type;
    HSRXDE          = seninf->HSRXDE;
    dpcm            = seninf->dpcm;
    mipiskew        = seninf->MIPIDeskew;

    if(seninf->CSI2_IP == SENINF2_CSI1) //Use sensor interface 2  & CSI1
    {
        pSeninf = (seninf_reg_t_base *)mpSeninf2HwRegAddr;
        pMipirx = (mipi_ana_reg_t_base *)mpCSI2RxAnalog1RegAddr;
        CalSel = 1;
    }
    else if(seninf->CSI2_IP == SENINF4_CSI2) // Use sensor interface 4  & CSI2
    {
        pSeninf = (seninf_reg_t_base *)mpSeninf4HwRegAddr;
        pMipirx_all = (mipi_ana_reg_t *)mpCSI2RxAnalog0RegAddr;
        CalSel = 2;
    }
    else // Use sensor interface 1  & CSI0
    {
        pSeninf = (seninf_reg_t_base *)mpSeninfHwRegAddr;
        pMipirx = (mipi_ana_reg_t_base *)mpCSI2RxAnalog0RegAddr;
        CalSel = 0;
    }

    /*First Enable Sensor interface and select pad (0x1a04_0200)*/
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL) = padSel;
    //For CSI2(2.5G) support
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = 0; //[15:12]: 0:csi, 8:NCSI2
    SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = 1;
    SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_NCSI2_IP_EN) = 0;

    /*GPIO config & MIPI RX config, CSI2 enable*/
    if(Enable == 1) {
        if((seninf->CSI2_IP == SENINF1_CSI0) || (seninf->CSI2_IP == SENINF2_CSI1))
        {
            /*Cphy select*/
            SENINF_BITS(pMipirx, MIPI_RX_WRAPPER80_CSI0A, CSR_CSI_MODE) = 2;
            /*CSI2: Enable mipi lane  0x10217000 */
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_CPHY_EN) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L0_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L0_CKSEL) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L1_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L1_CKSEL) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L2_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L2_CKSEL) = 0;

            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_CPHY_EN) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L0_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L0_CKSEL) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L1_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L1_CKSEL) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L2_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L2_CKSEL) = 0;
            //for Cphy only Digital sync
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L0_BYPASS_SYNC) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L1_BYPASS_SYNC) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_DPHY_L2_BYPASS_SYNC) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L0_BYPASS_SYNC) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L1_BYPASS_SYNC) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_DPHY_L2_BYPASS_SYNC) = 1;

            /*Adjust performance setting*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA04_CSI0A, RG_CSI_BG_HSDET_VTH_SEL) = 6;/*HSDET Level Adjustment*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA04_CSI0A, RG_CSI_BG_HSDET_VTL_SEL) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI_L0_T0AB_HSAMP_BW) = 2;/*EQ Power to Enhance Speed*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI_XX_T0CA_HSAMP_BW) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI_XX_T0BC_HSAMP_BW) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI_L1_T1AB_HSAMP_BW) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI_XX_T1CA_HSAMP_BW) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI_L2_T1BC_HSAMP_BW) = 2;

            SENINF_BITS(pMipirx, MIPI_RX_ANA04_CSI0B, RG_CSI_BG_HSDET_VTH_SEL) = 6;/*HSDET Level Adjustment*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA04_CSI0B, RG_CSI_BG_HSDET_VTL_SEL) = 2;/*HSDET Level Adjustment*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI_L0_T0AB_HSAMP_BW) = 2;/*EQ Power to Enhance Speed*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI_XX_T0CA_HSAMP_BW) = 2;/*EQ Power to Enhance Speed*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI_XX_T0BC_HSAMP_BW) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI_L1_T1AB_HSAMP_BW) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI_XX_T1CA_HSAMP_BW) = 2;
            SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI_L2_T1BC_HSAMP_BW) = 2;
            /*CDR register setting*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0A, RG_CSI_CPHY_T0_CDR_AB_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0A, RG_CSI_CPHY_T0_CDR_BC_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0A, RG_CSI_CPHY_T0_CDR_CA_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0A, RG_CSI_CPHY_T0_CDR_INIT_CODE) = 8;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0A, RG_CSI_CPHY_T0_CDR_EARLY_CODE) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA2C_CSI0A, RG_CSI_CPHY_T0_CDR_LATE_CODE) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA2C_CSI0A, RG_CSI_CPHY_T0_SYNC_INIT_SEL) = 1;

            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0A, RG_CSI_CPHY_T1_CDR_AB_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0A, RG_CSI_CPHY_T1_CDR_BC_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0A, RG_CSI_CPHY_T1_CDR_CA_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0A, RG_CSI_CPHY_T1_CDR_INIT_CODE) = 8;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0A, RG_CSI_CPHY_T1_CDR_EARLY_CODE) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA38_CSI0A, RG_CSI_CPHY_T1_CDR_LATE_CODE) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA38_CSI0A, RG_CSI_CPHY_T1_SYNC_INIT_SEL) = 1;

            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0B, RG_CSI_CPHY_T0_CDR_AB_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0B, RG_CSI_CPHY_T0_CDR_BC_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0B, RG_CSI_CPHY_T0_CDR_CA_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0B, RG_CSI_CPHY_T0_CDR_INIT_CODE) = 8;
            SENINF_BITS(pMipirx, MIPI_RX_ANA28_CSI0B, RG_CSI_CPHY_T0_CDR_EARLY_CODE) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA2C_CSI0B, RG_CSI_CPHY_T0_CDR_LATE_CODE) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA2C_CSI0B, RG_CSI_CPHY_T0_SYNC_INIT_SEL) = 1;

            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0B, RG_CSI_CPHY_T1_CDR_AB_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0B, RG_CSI_CPHY_T1_CDR_BC_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0B, RG_CSI_CPHY_T1_CDR_CA_WIDTH) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0B, RG_CSI_CPHY_T1_CDR_INIT_CODE) = 8;
            SENINF_BITS(pMipirx, MIPI_RX_ANA34_CSI0B, RG_CSI_CPHY_T1_CDR_EARLY_CODE) = 4;
            SENINF_BITS(pMipirx, MIPI_RX_ANA38_CSI0B, RG_CSI_CPHY_T1_CDR_LATE_CODE) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA38_CSI0B, RG_CSI_CPHY_T1_SYNC_INIT_SEL) = 1;
#ifdef CSI2_EFUSE_SET
            /*Read Efuse value : termination control registers*/
            Efuse(CalSel);
#endif
            /*Power on DPHY*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_BG_CORE_EN) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_BG_CORE_EN) = 1;
            usleep(30);
            /*D-PHY SW Delay Line calibration*/
#ifdef CSI2_TIMING_DELAY
            CSI_TimingDelayCal(0);
#endif
            /*Enable LPF*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_BG_LPF_EN) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_BG_LPF_EN) = 1;
            usleep(1);
        }
        else
        {
            /*CSI2(0x10217800): Enable mipi lane*/
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_CPHY_EN) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L0_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L0_CKSEL) = 0;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L1_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L1_CKSEL) = 0;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L2_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L2_CKSEL) = 0;
            //for Cphy only Digital sync
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L0_BYPASS_SYNC) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L1_BYPASS_SYNC) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_DPHY_L2_BYPASS_SYNC) = 1;
            /* Warning : 2D/1C need swap data/clock lane position,  2:  for CPHY*/
            SENINF_BITS(pMipirx_all, MIPI_RX_WRAPPER80_CSI2, CSR_CSI_MODE) = 2;
            /*Adjust performance setting*/
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA04_CSI2, RG_CSI_BG_HSDET_VTH_SEL) = 6;/*HSDET Level Adjustment*/
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA04_CSI2, RG_CSI_BG_HSDET_VTL_SEL) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA18_CSI2, RG_CSI_L0_T0AB_HSAMP_BW) = 1;/*EQ Power to Enhance Speed*/
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA18_CSI2, RG_CSI_XX_T0CA_HSAMP_BW) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA1C_CSI2, RG_CSI_XX_T0BC_HSAMP_BW) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA1C_CSI2, RG_CSI_L1_T1AB_HSAMP_BW) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA20_CSI2, RG_CSI_XX_T1CA_HSAMP_BW) = 1;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA20_CSI2, RG_CSI_L2_T1BC_HSAMP_BW) = 1;
#ifdef CSI2_EFUSE_SET
            /*Read Efuse value : termination control registers*/
            Efuse(CalSel);
#endif
            /*Power on DPHY*/
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_BG_CORE_EN) = 1;
            usleep(30);
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_BG_LPF_EN) = 1;
            usleep(1);
        }

        /*End of CSI MIPI*/
        // ISP CLK = 286 , settel delay count =  (x ns *  286 M )/1000
        msettleDelay = dataSettleDelay * 286 /1000;
        //DPCM Enable
        switch(dpcm)
        {
            case 0x30:
                dpcm_set = 1<<7;
                break;
            case 0x31:
                dpcm_set = 1<<8;
                break;
            case 0x32:
                dpcm_set = 1<<9;
                break;
            case 0x33:
                dpcm_set = 1<<10;
                break;
            case 0x34:
                dpcm_set = 1<<11;
                break;
            case 0x35:
                dpcm_set = 1<<12;
                break;
            case 0x36:
                dpcm_set = 1<<13;
                break;
            case 0x37:
                dpcm_set = 1<<14;
                break;
            case 0x2a:
                dpcm_set = 1<<15;
                break;
            default :
                dpcm_set = 0x00;
                break;
        }

        LOG_MSG("CSI2-%d TermDelay:%d SettleDelay:%d(cnt:%d) TermDelay:%d Vsync:%d LaneNum:%d NCSI2_EN:%d HeadOrder:%d ISP_clk:%d dpcm:%d skew:%d",
            CalSel, (int) dataTermDelay, (int) dataSettleDelay, (int)msettleDelay, (int)clkTermDelay, (int)vsyncType, (int)(dlaneNum+1),
            (int)Enable, (int)dataheaderOrder, (int)Isp_clk,(int)dpcm,(int)mipiskew);
        {
            /*SW offset calibration */
            //CSI2(2.5g) offset calibration
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 1;
#ifdef CSI2_SW_OFFSET_CAL
            CSI0_OffsetCal(CalSel);
#else
            // CSI2 offset calibration
            SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 1; // 0x15048338[0]= 1'b1;
            temp = SENINF_READ_REG(pSeninf, MIPI_RX_CON3C_CSI0);
            SENINF_WRITE_REG(pSeninf, MIPI_RX_CON3C_CSI0, 0x1541); // 0x1504833C[31:0]=32'h1541;

            SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_HW_CAL_START) = 1;//MIPI_RX_HW_CAL_START

            LOG_MSG("CSI2-0(2.5G) calibration start,MIPIType=%d,HSRXDE=%d",(int)mipi_type, (int)HSRXDE);

            usleep(500);
            if(!(( SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI0)& 0x10001) && (SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI0) & 0x101))){
             LOG_ERR("CSI2-0(2.5G) calibration failed!, NCSI2Config Reg 0x44=0x%x, 0x48=0x%x",SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI0),SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI0));
             //ret = -1;
            }
            // Disable SW control mode
            SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 0; // 0x15008338[0]= 1'b0;
#endif
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 0;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 0;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 0;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 0;
            SENINF_BITS(pSeninf, SENINF1_CSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 0;

            LOG_MSG("CSI2-%d(2.5G) calibration end !",CalSel);

            /*Settle delay*/
            temp = (msettleDelay&0xFF)<<8;
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_LNRD_TIMING, temp);
            /*CSI2 control*/
            SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, HSRX_DET_EN) = 0;
            SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, CLOCK_HS_OPTION) = 1;
            //for Cphy only Digital sync
            SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, ASYNC_FIFO_RST_SCH) = 2;
            SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, SYNC_DET_EN) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, REF_SYNC_DET_EN) = 0;

            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL,(temp|(dataheaderOrder<<16))) ;
            /* TRIO0/1/2 */
            if(dlaneNum == 0){
                /*TRIO LPRX Enable 0x1A04_0A7c*/
                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL_TRIO_CON,0x01);
                /*CSI2 Mode 0x1A04_0AE8*/
                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_MODE,0x102) ;
            }
            else if (dlaneNum == 1){
                /*TRIO LPRX Enable 0x1A04_0A7c*/
                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL_TRIO_CON,0x05);
                /*CSI2 Mode 0x1A04_0AE8*/
                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_MODE,0x202) ;
            }
            else{
                /*TRIO LPRX Enable 0x1A04_0A7c*/
                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL_TRIO_CON,0x15);
                /*CSI2 Mode 0x1A04_0AE8*/
                SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_MODE,0x402) ;
            }
            //0x1A040A74
            SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, LANE_MERGE_INPUT_SEL) = 1;
            SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, CPHY_LANE_RESYNC_CNT) = 5;
            //0x1A040AA8
            SENINF_BITS(pSeninf, SENINF1_SYNC_RESYNC_CTL, SYNC_DETECTION_SEL) = 1;
            //0x1A040AAC
            SENINF_BITS(pSeninf, SENINF1_POST_DETECT_CTL, POST_EN) = 1;
            if(mipiskew)
            {
                /*Enable HW Auto deskew function*/
                /* deskew Lane number control : need open by [dlaneNum]*/
                /*Need alway enable *DELAY_EN whed deskew funciton on*/
                if(dlaneNum == 2)
                {
                    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 0;
                    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 0;
                    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 1;
                }
                else
                {
                    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 1;
                    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 1;
                }


                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_ENABLE) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_CSI2_RST_ENABLE) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_IP_SEL) = 0;
                // delay mode: 0:hw, 1:sw
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, Delay_APPLY_MODE) = 0;
                // deskew delay apply mode
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_DELAY_APPLY_MODE) = 5;
                // deskew triggle mode
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_TRIGGER_MODE) = 4;
                // EXPECTED_SYNC_CODE
                //SENINF_WRITE_BITS(pSeninf, MIPI_RX_CONB4_CSI0, SYNC_CODE_MASK,0xffff);
                //SENINF_WRITE_BITS(pSeninf, MIPI_RX_CONB4_CSI0, EXPECTED_SYNC_CODE, 0xffff);
                SENINF_WRITE_REG(pSeninf, MIPI_RX_CONB4_CSI0,0xffffffff);
                // Time out
                //SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_TIME_OUT_EN) = 1;
                //SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_SETUP_TIME) = 0xff;
                SENINF_WRITE_REG(pSeninf, MIPI_RX_CONB8_CSI0,0x1ff0000);
                // deskew detect mode
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_DETECTION_MODE) = 0;
                // deskew ACC mode
                SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_ACC_MODE) = 0;
                // deskew IRQ enable
                //SENINF_BITS(pSeninf, MIPI_RX_CONC0_CSI0, DESKEW_INTERRUPT_W1C_EN) = 1;
                //SENINF_BITS(pSeninf, MIPI_RX_CONC0_CSI0, DESKEW_INTERRUPT_ENABLE) = 0x0f;
                SENINF_WRITE_REG(pSeninf, MIPI_RX_CONC0_CSI0,0x800000ff);
                // deskew Lane number(choose calibrartoin lane number)
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_LANE_NUMBER) = 0;
                //Skew pattern period ~= 2^12 UI
                SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_SETUP_TIME) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONB8_CSI0, DESKEW_HOLD_TIME) = 1;
                SENINF_BITS(pSeninf, MIPI_RX_CONBC_CSI0, DESKEW_DETECTION_CNT) = 4;
                //DESKEW_DELAY_LENGTH
                SENINF_BITS(pSeninf, MIPI_RX_COND0_CSI0, DESKEW_DELAY_LENGTH) = 0x0f;
                /*0: done, 1: timeout, 2: ffff detection, 3: sw delay apply successfully(sw mode used)*/
                /*5: b8 detection*/
                LOG_MSG("CSI2 HW Deskew IRQ = 0x%x",SENINF_READ_REG(pSeninf, MIPI_RX_CONC4_CSI0));
            }
            else
            {
                SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 0;
                SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 0;
            }
             /* set debug port to output packet number */
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DGB_SEL,0x8000001A);
            /*Enable CSI2 IRQ mask*/
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN,0x9FFFBFFF);//turn on all interrupt
            /*write clear CSI2 IRQ*/
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_STATUS,0x9FFFBFFF);
            /*Enable CSI2 Extend IRQ mask*/
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN_EXT,0x0000001f);//turn on all interrupt
            //LOG_MSG("SENINF1_CSI2_CTL(0x1a04_0a00 = 0x%x",SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL));
        }
    }
    else {
        // disable NCSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(1.5G) first
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first
        if((seninf->CSI2_IP == SENINF1_CSI0) || (seninf->CSI2_IP == SENINF2_CSI1))
        {
            //disable mipi BG
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_BG_CORE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_BG_CORE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI_BG_LPF_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI_BG_LPF_EN) = 0;
        }
        else
        {
            //disable mipi BG
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_BG_CORE_EN) = 0;
            SENINF_BITS(pMipirx_all, MIPI_RX_ANA00_CSI2, RG_CSI_BG_LPF_EN) = 0;
        }
    }
#endif
    return ret;

}
#endif
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf4Scam(
    unsigned int scamEn, unsigned int clkInv,
    unsigned int width, unsigned int height,
    unsigned int contiMode, unsigned int csdNum,
    unsigned int DDR_EN,unsigned int default_delay,
    unsigned int crcEn,unsigned int timeout_cali,
    unsigned int sof_src
)
{
    int ret = 0;

    (void)scamEn;
    (void)clkInv;
    (void)width;
    (void)height;
    (void)contiMode;
    (void)csdNum;
    (void)DDR_EN;
    (void)default_delay;
    (void)crcEn;
    (void)timeout_cali;
    (void)sof_src;
#if 0
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    IMGSENSOR_GPIO_STRUCT mpgpio;
    int data_channel;
    LOG_MSG("Scam clkInv(%d),width(%d),height(%d),csdNum(%d),DDR_EN(%d), best_delay_value(%d), default_delay (%d)",clkInv,width,height,csdNum+1,DDR_EN,best_delay_value,default_delay);
	LOG_MSG("Scam crcEn(%d),timeout_cali(%d),sof_src(%d)",crcEn,timeout_cali,sof_src);

    /* set GPIO config */
    if(scamEn == 1) {
        mpgpio.GpioEnable = 1;
        mpgpio.SensroInterfaceType = SENSORIF_SERIAL;
        mpgpio.SensorIndataformat = DATA_YUV422_FMT;
        ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
        if (ret < 0) {
           LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO");
        }

        //GPI
        temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x50/4));

        temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x50/4));
    }
    else {
         mpgpio.GpioEnable = 0;
         mpgpio.SensroInterfaceType = SENSORIF_SERIAL;
         mpgpio.SensorIndataformat = DATA_YUV422_FMT;
         ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
         if (ret < 0) {
            LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO");
         }
        temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x50/4));

         temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x50/4));
    }

if(scamEn == 1) {
        /**/
        if(DDR_EN == 0){

            switch(csdNum) {
                case 0: /* 1 channel*/
                    data_channel = 0;
                    break;
                case 1: /* 2 channel*/
                    data_channel = 1;
                    break;
                case 2:/* 3 channel*/
                    data_channel = 3;
                    break;
                case 3:/* 4 channel*/
                    data_channel = 2;
                    break;
                default:
                    data_channel = 0;
                    break;
            }
        }else {
                    switch(csdNum) {
                case 0: /* 2 channel*/
                    data_channel = 1;
                    break;
                case 1: /* 4 channel*/
                    data_channel = 2;
                    break;
                case 2:/* 6 channel*/
                    data_channel = 4;
                    break;
                case 3:/* 8 channel*/
                    data_channel = 5;
                    break;
                default:
                    data_channel = 0;
                    break;
            }
        }

		if(sof_src != 1){//sof_src should always be 1 for scam
			LOG_MSG("!!! scam sof_src =%d",sof_src);
			isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
			ISP_BITS(pisp, CAM_TG_SEN_MODE, SOF_SRC) = sof_src;
		}
        /*Reset for enable scam*/
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x10000);
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00);

        /*Sensor Interface control select to Serial camera*/
        temp = SENINF_READ_REG(pSeninf, SENINF4_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF4_CTRL,((temp&0xFFFF0FFF)|0x4000));

        /*SCAM control */
        temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);
        temp &= 0x50000000;
        SENINF_WRITE_REG(pSeninf,SCAM1_CFG, temp|((data_channel&0x7)<<24)|(contiMode<<17)|(clkInv<<12)|(1<<8)|(1));
        SENINF_WRITE_REG(pSeninf,SCAM1_SIZE, ((height&0xFFF)<<16)|(width&0xFFF));
        if(width > 0xFFF || height > 0xFFF) {
            LOG_ERR("scam size incorrect, it must be smaller than 0xfff !!");
        }

        SENINF_WRITE_REG(pSeninf, SCAM1_CFG2, 1);
		/*Enable SCAM*/
        SENINF_WRITE_REG(pSeninf, SCAM1_LINE_ID_START, 0x00);

        /*DDR Mode */
        if(DDR_EN != 1) {
			SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x01);
        } else {

            temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);
            temp &= ~0x00000006;// clear clock 1&2 inverse flag

            SENINF_WRITE_REG(pSeninf,SCAM1_DDR_CTRL, temp|((csdNum)<<28)|(DDR_EN<<7));
            if((best_delay_value == -1)&&(default_delay < 64)){
                if(default_delay < 32){//0~31 negative
                    best_delay_value = default_delay;
                    Positive_Calibration = false;
                } else {//0~31 postive
                    best_delay_value = default_delay - 32;
                    Positive_Calibration = true;
                }
            }

            if(best_delay_value != -1) {
                LOG_MSG("already calibration  best_delay_value %d",best_delay_value);
            } else {
    //calibration session start
    			if(timeout_cali == 0){
    				SENINF_WRITE_REG(pSeninf,SCAM1_TIME_OUT,SCAM_CALI_TIMEOUT);//
    			} else {
    				SENINF_WRITE_REG(pSeninf,SCAM1_TIME_OUT,timeout_cali);
    			}
    			temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);

    			temp = temp|1<<21|1<<22|1<<7|1<<1;//format=yuv, in9, crc_on,calibration mode,int7,int1
    			temp = temp|0x6000;//timeout counter & int8

    			SENINF_WRITE_REG(pSeninf,SCAM1_CFG,temp);

    			int boundary1[2];
    			int boundary2[2];
    			int valid_num[2];

    			int boundary1_max[2];
    			int boundary2_max[2];
    			int valid_num_max[2];
    			int calibration_result[2];
    			int delay_value =0;
    			int j,i=0;
    			int test_round =0;
    			bool	delay_flag[2][32];
    			temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);
    			temp |=0x4;
    			temp &=~0x2;
    			SENINF_WRITE_REG(pSeninf,SCAM1_DDR_CTRL,temp);

    			temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);
    			temp = temp|0x6000;//timeout counter & int8
    			SENINF_WRITE_REG(pSeninf,SCAM1_CFG,temp);//enable time out

    			for( test_round=0;test_round<2;test_round++){
    				for (delay_value = 0; delay_value < 32 ; delay_value++){
    					SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00010000);//SET_PAD2CAM_RST
    					SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00000000); //SET_PAD2CAM_CLEAR_RST;

    					temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);
    					temp  = temp &(~0xFFFFF00);//clear all delay value
    					temp  = temp | (delay_value<<8)|(delay_value<<13)|(delay_value<<18)|(delay_value<<23);
    					SENINF_WRITE_REG(pSeninf,SCAM1_DDR_CTRL,temp);

#if DEBUG_SCAM_CALI
                         temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);
                         LOG_MSG("delay_value %d,SCAM1_DDR_CTRL  0x%08x",delay_value,temp);
               			 temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);
                         LOG_MSG("SCAM1_CFG  0x%08x",temp);
#endif
                        SENINF_WRITE_REG(pSeninf ,SCAM1_CON, 0x00000001); //SET_PAD2CAM_ENA;
                        while(1){
                            temp = SENINF_READ_REG(pSeninf, SCAM1_INT);
                           if(temp&(1<<8)){//timeout error 0x100
    							delay_flag[test_round][delay_value] = false;
    							SENINF_WRITE_REG(pSeninf ,SCAM1_INFO4, 0);//clear all counter of time out, crc error or correct
    							SENINF_WRITE_REG(pSeninf ,SCAM1_INFO6, 0);
                                break;
                            }
                            if(temp&(1<<1)){//crc error 0x002
    							delay_flag[test_round][delay_value] = false;
    							SENINF_WRITE_REG(pSeninf ,SCAM1_INFO4, 0);
    							SENINF_WRITE_REG(pSeninf ,SCAM1_INFO6, 0);
                                break;
                            }

                            if(crcEn&&(temp&(1<<7))){//crc correct //0x8
                                if(((SENINF_READ_REG(pSeninf, SCAM1_INFO4)>>28)&0xf)==0xf){
                                    delay_flag[test_round][delay_value] = true;
                                     SENINF_WRITE_REG(pSeninf ,SCAM1_INFO4, 0);
                                     SENINF_WRITE_REG(pSeninf ,SCAM1_INFO6, 0);
                                    break;//delay code works
                                }
                            }
    						if(!crcEn&&(temp&(1<<9))){//correct packet
    							 if(((SENINF_READ_REG(pSeninf, SCAM1_INFO6)>>4)&0xf)==0xf){
    	                                delay_flag[test_round][delay_value] = true;
    	                                 SENINF_WRITE_REG(pSeninf ,SCAM1_INFO4, 0);
    	                                 SENINF_WRITE_REG(pSeninf ,SCAM1_INFO6, 0);
    	                                break;//delay code works
    	                            }
    						}

                       }
#if DEBUG_SCAM_CALI
    					LOG_MSG("delay_value %d SCAM1_INT  0x%08x",delay_value,temp);
#endif

    				}
    				temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);//try reverse clk
    				temp |=0x2;
    				temp &=~0x4;
    				SENINF_WRITE_REG(pSeninf,SCAM1_DDR_CTRL,temp);
    			}
    			for (j=0; j<2; j++)
    			{
    				boundary1[j] = -1;
    				boundary2[j] = -1;
    				valid_num[j] = 0;
    				calibration_result[j] = 0;
    				boundary1_max[j]=-1;
    				boundary2_max[j]=-1;
    				valid_num_max[j]=0;
    			}

    			for (j=0; j<2; j++)
    			{
    			   for (i=0;i<32; i++)
    			   {
    				  if (delay_flag[j][i] == true)
    					  valid_num[j]++;
    			       if ((delay_flag[j][i] == true)&&((i==0)||(delay_flag[j][i-1] == false))&&(boundary1[j]==-1)) // 0001111111
    			           boundary1[j] = i;

    					if ((delay_flag[j][i] == true)&&((i==31)||(delay_flag[j][i+1] == false))&&(boundary2[j]==-1)) {//1111000
    						boundary2[j] = i;
    						if(valid_num[j]>2) {
    							boundary1_max[j] = boundary1[j] ;
    							boundary2_max[j] = boundary2[j] ;
    							valid_num_max[j] = valid_num[j] ;
    							break;
    						} else {
    							boundary1[j] = -1;
    							boundary2[j] = -1;
    							valid_num[j] = 0;
    						}
    					}

    			   }
    			   if(valid_num_max[j]==0){// exception case, no session length >2,
    					boundary1[j] = -1;
    					boundary2[j] = -1;
    					valid_num[j] = 0;
    					calibration_result[j] = 0;
    					boundary1_max[j]=-1;
    					boundary2_max[j]=-1;
    					valid_num_max[j]=0;
    					for (i=0;i<32; i++)
    					{
    						if (delay_flag[j][i] == true)
    							valid_num[j]++;
    						if ((delay_flag[j][i] == true)&&((i==0)||(delay_flag[j][i-1] == false))&&(boundary1[j]==-1)) // 0001111111
    							boundary1[j] = i;

    						if ((delay_flag[j][i] == true)&&((i==31)||(delay_flag[j][i+1] == false))&&(boundary2[j]==-1)) {//1111000
    							boundary2[j] = i;

    							boundary1_max[j] = boundary1[j] ;
    							boundary2_max[j] = boundary2[j] ;
    							valid_num_max[j] = valid_num[j] ;
    							break;
    						}
    					}
    			   }


    			   if ((boundary1_max[j] != -1)&&(boundary2_max[j] != -1))
    			   {
    			       if (boundary1_max[j] > boundary2_max[j])
    				   		calibration_result[j] = (boundary2[j])/2;//11111000100
    			       else
    			            calibration_result[j] = (boundary2_max[j] + boundary1_max[j])/2;//00011111100
    			   }
    			   else if ((boundary1_max[j] == -1)&&(boundary2_max[j] != -1))
    			   { //11111000
    			       calibration_result[j] = boundary2_max[j]/2;
    			   }
    			   else if((boundary1_max[j] != -1)&&(boundary2_max[j] == -1))
    			   {//000011111
    			       calibration_result[j]= (boundary1_max[j]+31)/2;
    			   }
    			   else //((boundary1[j] == 0)&&(boundary2[j] == 0))
    			   {
    			       calibration_result[j]= 0;
    			   }

    			}
#if DEBUG_SCAM_CALI
                 LOG_MSG("boundary1_max[0] %d, boundary2_max[0] %d",boundary1_max[0],boundary2_max[0]);
                 LOG_MSG("boundary1_max[1] %d, boundary2_max[1] %d",boundary1_max[1],boundary2_max[1]);
                 LOG_MSG("valid_num_max[0] %d, valid_num_max[1] %d",valid_num_max[0],valid_num_max[1]);
#endif
                if((boundary1_max[0]==-1)&&(boundary1_max[1]==-1))
                {
                    LOG_MSG("cant find delay code in both side %d");
                    Positive_Calibration = true;
    				best_delay_value = 0;
                }else {
                    if(boundary1_max[0]==-1){
                        Positive_Calibration = false;
                        best_delay_value = calibration_result[1];
                    } else if(boundary1_max[1]==-1){
                        Positive_Calibration = true;
        				best_delay_value = calibration_result[0];
                    } else if(boundary1_max[1] < boundary1_max[0]){
        				Positive_Calibration = false;
        				best_delay_value = calibration_result[1];
                    } else {
        				Positive_Calibration = true;
        				best_delay_value = calibration_result[0];
                    }
                }


#if DEBUG_SCAM_CALI
    			LOG_MSG("calibration_result[0] %d, calibration_result[1] %d Positive_Calibration %d",calibration_result[0], calibration_result[1],Positive_Calibration);
#endif
                if(best_delay_value<0)
                   best_delay_value = 0;
            }
////calibration sesson end

			SENINF_WRITE_REG(pSeninf ,SCAM1_CON, 0x00010000);//SET_PAD2CAM_RST
			SENINF_WRITE_REG(pSeninf ,SCAM1_CON, 0x00000000); //SET_PAD2CAM_CLEAR_RST;

			temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);
			temp  = temp &(~0xFFFFF00);
			temp  = temp | (best_delay_value<<8)|(best_delay_value<<13)|(best_delay_value<<18)|(best_delay_value<<23);

			if(Positive_Calibration) {
				temp |=0x4;
				temp &=~0x2;
			} else {
				temp |=0x2;
				temp &=~0x4;
			}
			SENINF_WRITE_REG(pSeninf,SCAM1_DDR_CTRL,temp);

			temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);//clear all flag about calibration
			temp =temp &( ~0x6000);
			temp = temp & (~(1<<21));
			temp = temp & (~(1<<22));
			temp = temp & (~(1<<7));
			temp = temp & (~(1<<1));
			SENINF_WRITE_REG(pSeninf,SCAM1_CFG,temp);


			SENINF_WRITE_REG(pSeninf ,SCAM1_CON, 0x00000001); //SET_PAD2CAM_ENA;

			temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);
			LOG_MSG("best_delay_value %d after cali SCAM1_DDR_CTRL 0x%08x",best_delay_value,temp);
			temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);
			LOG_MSG("after cali SCAM1_CFG 0x%08x",temp);
        }

    }
    else
    {
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00);
    }

    temp = SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL);
    SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp|0x3);//reset
    SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp&0xFFFFFFFC);//clear reset
#endif
    return ret;

}


int SeninfDrvImp::setSeninf4Parallel(unsigned int parallelEn,unsigned int inDataType)
{
    int ret = 0;
    (void)parallelEn;
    (void)inDataType;
#if 0
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    IMGSENSOR_GPIO_STRUCT mpgpio;

    if(parallelEn == 1) {
        if ((inDataType == RAW_8BIT_FMT)||(inDataType == YUV422_FMT)||(inDataType == RGB565_MIPI_FMT)||
            (inDataType == RGB888_MIPI_FMT)||(inDataType == JPEG_FMT)||(inDataType == RAW_10BIT_FMT)) {

            mpgpio.GpioEnable = 1;
            mpgpio.SensroInterfaceType = SENSORIF_PARALLEL;
            mpgpio.SensorIndataformat = (INDATA_FORMAT_ENUM)inDataType;
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
            if (ret < 0) {
               LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO");
            }

            //GPI
            temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x4C/4));
            temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x50/4));

            temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x4C/4));
            temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x50/4));

        }

        else {
            LOG_ERR("parallel interface only support 8bit/10bit !");
        }


        temp = SENINF_READ_REG(pSeninf, SENINF4_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF4_CTRL,((temp&0xFFFF0FFF)|0x3000));


         temp = SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL);
        SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp|0x3);//reset
        SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp&0xFFFFFFFC);//clear reset
    }
    else {

         mpgpio.GpioEnable = 0;
         mpgpio.SensroInterfaceType = SENSORIF_PARALLEL;
         mpgpio.SensorIndataformat = (INDATA_FORMAT_ENUM)inDataType;
         ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
         if (ret < 0) {
            LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO");
         }

         temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x50/4));

         temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x50/4));

    }
#endif
    return ret;
}

int SeninfDrvImp::setAllMclkOnOff(unsigned long ioDrivingCurrent, bool enable){


    if(enable) {
        setMclkIODrivingCurrent(eMclk_1, ioDrivingCurrent);
        setMclkIODrivingCurrent(eMclk_2, ioDrivingCurrent);
        setMclkIODrivingCurrent(eMclk_3, ioDrivingCurrent);
        setMclkIODrivingCurrent(eMclk_4, ioDrivingCurrent);
        setMclk(eMclk_1, 1, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_2, 1, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_3, 1, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_4, 1, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);

    } else {
        setMclk(eMclk_1, 0, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_2, 0, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_3, 0, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_4, 0, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);

    }
    return 0;
}

int SeninfDrvImp::setMclkIODrivingCurrent(EMclkId mclkIdx, unsigned long ioDrivingCurrent){
    if (mclkIdx == eMclk_1)
        return setMclk1IODrivingCurrent(ioDrivingCurrent);
    else if (mclkIdx == eMclk_2)
        return setMclk2IODrivingCurrent(ioDrivingCurrent);
    else if (mclkIdx == eMclk_3)
        return setMclk3IODrivingCurrent(ioDrivingCurrent);
    else if (mclkIdx == eMclk_4)
        return setMclk4IODrivingCurrent(ioDrivingCurrent);
    else
        LOG_ERR("never here");
    return -1;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk1IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;
    unsigned int *pCAMIODrvRegAddr = NULL;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.seninf.mclk1", value, "255");
    int inputCurrent = atoi(value);
    if (inputCurrent<8)
        ioDrivingCurrent = inputCurrent;

    pCAMIODrvRegAddr = mpGpioHwRegAddr+ (0x800/4);

    if(pCAMIODrvRegAddr != NULL) {
        *(pCAMIODrvRegAddr) &= (~(0x7<<0));
        *(pCAMIODrvRegAddr) |= (ioDrivingCurrent<<0); // [2:0] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
    }
    LOG_MSG("DrivingIdx(%d), Reg(0x%08x)", (int) ioDrivingCurrent, (int) (*(pCAMIODrvRegAddr)));

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk2IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;
    unsigned int *pCAMIODrvRegAddr = NULL;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.seninf.mclk2", value, "255");
    int inputCurrent = atoi(value);
    if (inputCurrent<8)
        ioDrivingCurrent = inputCurrent;

    pCAMIODrvRegAddr = mpGpioHwRegAddr+(0x800/4);

    if(pCAMIODrvRegAddr != NULL) {
        *(pCAMIODrvRegAddr) &= (~(0x7<<3));
        *(pCAMIODrvRegAddr) |= (ioDrivingCurrent<<3); // [5:3] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
    }
    LOG_MSG("DrivingIdx(%d), Reg(0x%08x)", (int) ioDrivingCurrent, (int) (*(pCAMIODrvRegAddr)));

    return ret;
}
int SeninfDrvImp::setMclk3IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;
    unsigned int *pCAMIODrvRegAddr = NULL;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.seninf.mclk3", value, "255");
    int inputCurrent = atoi(value);
    if (inputCurrent<8)
        ioDrivingCurrent = inputCurrent;

    pCAMIODrvRegAddr = mpGpioHwRegAddr+ (0xc00/4);

    if(pCAMIODrvRegAddr != NULL) {
        *(pCAMIODrvRegAddr) &= (~(0x7<<9));
        *(pCAMIODrvRegAddr) |= (ioDrivingCurrent<<9); // [9:11] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
    }
    LOG_MSG("DrivingIdx(%d), Reg(0x%08x)", (int) ioDrivingCurrent, (int) (*(pCAMIODrvRegAddr)));

    return ret;
}

int SeninfDrvImp::setMclk4IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;

    unsigned int *pCAMIODrvRegAddr = NULL;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.seninf.mclk4", value, "255");
    int inputCurrent = atoi(value);
    if (inputCurrent<8)
        ioDrivingCurrent = inputCurrent;

    pCAMIODrvRegAddr = mpGpioHwRegAddr+(0x600/4);

    if(pCAMIODrvRegAddr != NULL) {
        *(pCAMIODrvRegAddr) &= (~(0x7<<3));
        *(pCAMIODrvRegAddr) |= (ioDrivingCurrent<<3); // [5:3] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
    }
    LOG_MSG("DrivingIdx(%d), Reg(0x%08x)", (int) ioDrivingCurrent, (int) (*(pCAMIODrvRegAddr)));

    return ret;
}


int SeninfDrvImp::setTG_TM_Ctl(unsigned int seninfSrc, unsigned int TM_En, unsigned int dummypxl,unsigned int vsync,
                                       unsigned int line, unsigned int pxl)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_REG(pSeninf,SENINF_TG1_TM_CTL,0x00);
    if(seninfSrc == SENINF_1)
    {
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_SIZE, TM_LINE) = line;
    }
    else if(seninfSrc == SENINF_2)
    {
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_SIZE, TM_LINE) = line;
    }
    else if(seninfSrc == SENINF_3)
    {
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_SIZE, TM_LINE) = line;
    }
    else if(seninfSrc == SENINF_4)
    {
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_SIZE, TM_LINE) = line;
    }
    else
    {
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_EN) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_EN) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_EN) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_EN) = 0;
    }
    return ret;
}

#if 0
int SeninfDrvImp::setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount,
            unsigned long startLine, unsigned long startPixel, unsigned long  flashPol)
{
    int ret = 0;
    cam_reg_t *pisp = (cam_reg_t *) mpIspHwRegAddr;


    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN) = 0x0;

    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_POL) = flashPol;
    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_END_FRM) = endFrame;
    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_STARTPNT) = startPoint;

    ISP_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT_NO) = unitCount;
    ISP_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT) = lineUnit;

    ISP_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_PXL) =  startPixel;
    ISP_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_LINE) =  startLine;

    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN) = 0x1;

    return ret;

}


int SeninfDrvImp::setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel)
{
    int ret = 0;
    cam_reg_t *pisp = (cam_reg_t *) mpIspHwRegAddr;

    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN) = 0x0;

    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_CONT_FRM) = contiFrm;
    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_START_FRM) = startFrame;
    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_STARTPNT) = 0x0;
    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_TRIG_SRC) = 0x0;

    ISP_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT_NO) = unitCount;
    ISP_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT) = lineUnit;

    ISP_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_PXL) = startPixel;
    ISP_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_LINE) =  startLine;

    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN) = 0x1;

    return ret;
}

int SeninfDrvImp::setFlashEn(bool flashEn)
{
    int ret = 0;
    cam_reg_t *pisp = (cam_reg_t *) mpIspHwRegAddr;

    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_EN) = flashEn;

    return ret;

}


int SeninfDrvImp::setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    (void)vsPol;
    (void)hsPol;

    if ((hsStart > 4095) || (hsEnd > 4095))
    {
        LOG_ERR("CCIR656 HSTART or HEND value err");
        ret = -1;
    }

    //SENINF_BITS(pSeninf, CCIR656_CTL, CCIR656_VS_POL) = vsPol;
    //SENINF_BITS(pSeninf, CCIR656_CTL, CCIR656_HS_POL) = hsPol;
    //SENINF_BITS(pSeninf, CCIR656_H, CCIR656_HS_END) = hsEnd;
    //SENINF_BITS(pSeninf, CCIR656_H, CCIR656_HS_START) = hsStart;

    return ret;
}


int SeninfDrvImp::setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode, unsigned long diffCntEn, unsigned long diffCntThr)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    (void)n3dEn;
    (void)i2c1En;
    (void)i2c2En;
    (void)n3dMode;
    (void)diffCntEn;
    (void)diffCntThr;

    //SENINF_BITS(pSeninf, N3D_CTL, N3D_EN) = n3dEn;
    //SENINF_BITS(pSeninf, N3D_CTL, I2C1_EN) = i2c1En;
    //SENINF_BITS(pSeninf, N3D_CTL, I2C2_EN) = i2c2En;
    //SENINF_BITS(pSeninf, N3D_CTL, MODE) = n3dMode;

    return ret;
}


int SeninfDrvImp::setN3DI2CPos(unsigned long n3dPos)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    (void)n3dPos;

    //SENINF_BITS(pSeninf, N3D_POS, N3D_POS) = n3dPos;

    return ret;
}


int SeninfDrvImp::setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    (void)i2c1TrigOn;
    (void)i2c2TrigOn;

    //SENINF_BITS(pSeninf, N3D_TRIG, I2CA_TRIG) = i2c1TrigOn;
    //SENINF_BITS(pSeninf, N3D_TRIG, I2CB_TRIG) = i2c2TrigOn;

    return ret;

}
#endif

int SeninfDrvImp::getN3DDiffCnt(MUINT32 *pCnt)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

   *pCnt = SENINF_READ_REG(pSeninf,SENINF_N3D_A_DIFF_CNT);
   *(pCnt+1) = SENINF_READ_REG(pSeninf,SENINF_N3D_A_CNT0);
   *(pCnt+2) = SENINF_READ_REG(pSeninf,SENINF_N3D_A_CNT1);
   *(pCnt+3) = SENINF_READ_REG(pSeninf,SENINF_N3D_A_DBG);

    return ret;

}
#if 0
int SeninfDrvImp::checkSeninf1Input()
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;
#if 0
    temp = SENINF_READ_REG(pSeninf,SENINF1_DEBUG_4);
    LOG_MSG("[checkSeninf1Input]:size = 0x%x",temp);
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;

    if( (tempW >= tg1GrabWidth) && (tempH >= tg1GrabHeight)  ) {
        ret = 0;
    }
    else {
        ret = 1;
    }
#endif
    return ret;

}

int SeninfDrvImp::checkSeninf2Input()
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;
#if 0
    temp = SENINF_READ_REG(pSeninf,SENINF2_DEBUG_4);
    LOG_MSG("[checkSeninf2Input]:size = 0x%x",temp);
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;

    if( (tempW >= tg2GrabWidth) && (tempH >= tg2GrabHeight)  ) {
        ret = 0;
    }
    else {
        ret = 1;
    }
#endif
    return ret;

}
#endif
int SeninfDrvImp::updateIspClk(unsigned int target_clk,  IMGSENSOR_SENSOR_IDX sensorIdx){
#if 0 /*for debug*/
    unsigned int ISPclk_cur = 0;
    if ((ioctl(m_fdSensor, KDIMGSENSORIOC_GET_CUR_ISP_CLOCK, &ISPclk_cur)) < 0)
        LOG_ERR("KDIMGSENSORIOC_GET_CUR_ISP_CLOCK fail");
    LOG_MSG("IspClk_cur:%d", ISPclk_cur);

    for(int i = IMGSENSOR_SENSOR_IDX_MIN_NUM; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; i++)
        LOG_MSG("mISPclk_tar[%d] = %d",i ,mISPclk_tar[i]);
#endif

    unsigned int original_dfs = mISPclk_tar[IMGSENSOR_SENSOR_IDX_MIN_NUM];
    for(int i = IMGSENSOR_SENSOR_IDX_MIN_NUM + 1; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; i++){
        if(mISPclk_tar[i] >= original_dfs)
            original_dfs = mISPclk_tar[i];
    }

    mISPclk_tar[sensorIdx] = target_clk;

    unsigned int new_dfs = mISPclk_tar[IMGSENSOR_SENSOR_IDX_MIN_NUM];
    for(int i = IMGSENSOR_SENSOR_IDX_MIN_NUM + 1; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; i++){
        if(mISPclk_tar[i] >= new_dfs)
            new_dfs = mISPclk_tar[i];
    }
    LOG_MSG("target_clk %d, new_dfs %d, original_dfs %d, sensorIdx %d",target_clk , new_dfs ,original_dfs, sensorIdx);

    if (new_dfs != original_dfs) {
        if (ioctl(m_fdSensor, KDIMGSENSORIOC_DFS_UPDATE, (unsigned int*)&new_dfs) < 0)
            LOG_ERR("Update DFS error new_dfs %d", new_dfs);
    }
    return 0;
}

int SeninfDrvImp::configInputForCSI(SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo, bool enable) {
    int ret = 0;
    SENINF_TOP_P1_ENUM SENINF_TOP_TG_selected = SENINF_TOP_TG1;
    SENINF_MUX_ENUM SENINF_MUX_selected = SENINF_MUX1;
    unsigned long TG_SEL = CAM_TG_NONE;
    SENSOR_VC_INFO_STRUCT *pVcInfo = NULL;
    Mutex::Autolock lock(mLock_mmap);
    {
        if(mSeninfMuxUsed[SENINF_MUX1] == -1 || mSeninfMuxUsed[SENINF_MUX1] == pseninfPara->sensor_idx) {
            mSeninfMuxUsed[SENINF_MUX1] = pseninfPara->sensor_idx;
            SENINF_TOP_TG_selected = SENINF_TOP_TG1;
            SENINF_MUX_selected = SENINF_MUX1;
            TG_SEL = CAM_TG_1;
        } else if(mSeninfMuxUsed[SENINF_MUX2] == -1 || mSeninfMuxUsed[SENINF_MUX2] == pseninfPara->sensor_idx) {
            mSeninfMuxUsed[SENINF_MUX2] = pseninfPara->sensor_idx;
            SENINF_TOP_TG_selected = SENINF_TOP_TG2;
            SENINF_MUX_selected = SENINF_MUX2;
            TG_SEL = CAM_TG_2;
        } else {
            LOG_ERR("no availabe mux mSeninfMuxUsed[SENINF_MUX1] = %d, mSeninfMuxUsed[SENINF_MUX2] = %d",
                mSeninfMuxUsed[SENINF_MUX1], mSeninfMuxUsed[SENINF_MUX2]);
            mSeninfMuxUsed[SENINF_MUX1] = pseninfPara->sensor_idx;
            SENINF_TOP_TG_selected = SENINF_TOP_TG1;
            SENINF_MUX_selected = SENINF_MUX1;
            TG_SEL = CAM_TG_1;
        }
    }


    if(enable) {
        ret = this->setSeninfMuxCtrl(SENINF_MUX_selected, 0, 0,
                         MIPI_SENSOR, pseninfPara->inDataType, psensorDynamicInfo->pixelMode);
        ret = this->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, pseninfPara->csi_para.seninfSrc);
        psensorDynamicInfo->TgInfo = TG_SEL;

        if(pseninfPara->HDRMode == HDR_CAMSV){
            psensorDynamicInfo->HDRPixelMode = HALSENSOR_CAMSV_PIXEL_MODE;
            pVcInfo = &(pseninfPara->csi_para.vcInfo);
            ret = this->setSeninfMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_MUX3 :SENINF_MUX5), 0, 0,
                                            VIRTUAL_CHANNEL_1, pseninfPara->inDataType, psensorDynamicInfo->HDRPixelMode);
            LOG_MSG("HDR VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                    pVcInfo->VC0_ID,pVcInfo->VC0_DataType,pVcInfo->VC1_ID,pVcInfo->VC1_DataType,
                    pVcInfo->VC2_ID,pVcInfo->VC2_DataType,pVcInfo->VC3_ID,pVcInfo->VC3_DataType);
            ret = this->setSeninfVC(pseninfPara->csi_para.seninfSrc,
                                    (pVcInfo->VC0_DataType<<2)|(pVcInfo->VC0_ID&0x03),
                                    (pVcInfo->VC1_DataType<<2)|(pVcInfo->VC1_ID&0x03),
                                    (pVcInfo->VC2_DataType<<2)|(pVcInfo->VC2_ID&0x03),
                                    (pVcInfo->VC3_DataType<<2)|(pVcInfo->VC3_ID&0x03), 0, 0);

            ret = this->setSeninfTopMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_TOP_SV1 :SENINF_TOP_SV3), pseninfPara->csi_para.seninfSrc);
            psensorDynamicInfo->HDRInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_1 :CAM_SV_3);
        }

        if (pseninfPara->PDAFMode == PDAF_SUPPORT_CAMSV ||
            pseninfPara->PDAFMode == PDAF_SUPPORT_CAMSV_LEGACY ||
            pseninfPara->PDAFMode == PDAF_SUPPORT_CAMSV_DUALPD ){
            psensorDynamicInfo->PDAFPixelMode= HALSENSOR_CAMSV_PIXEL_MODE;
            pVcInfo = &(pseninfPara->csi_para.vcInfo);

           //seninf muxer4 for  PDAF using virtual channel 2
           ret = this->setSeninfMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_MUX4 :SENINF_MUX6), 0, 0,
                                               VIRTUAL_CHANNEL_2, pseninfPara->inDataType, psensorDynamicInfo->PDAFPixelMode);
           // Sensor interface for VC
           LOG_MSG("PDAF VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                    pVcInfo->VC0_ID, pVcInfo->VC0_DataType, pVcInfo->VC1_ID, pVcInfo->VC1_DataType,
                    pVcInfo->VC2_ID, pVcInfo->VC2_DataType, pVcInfo->VC3_ID, pVcInfo->VC3_DataType);
            ret = this->setSeninfVC(pseninfPara->csi_para.seninfSrc,
                                    (pVcInfo->VC0_DataType<<2)|(pVcInfo->VC0_ID&0x03),
                                    (pVcInfo->VC1_DataType<<2)|(pVcInfo->VC1_ID&0x03),
                                    (pVcInfo->VC2_DataType<<2)|(pVcInfo->VC2_ID&0x03),
                                    (pVcInfo->VC3_DataType<<2)|(pVcInfo->VC3_ID&0x03), 0, 0);

           ret = this->setSeninfTopMuxCtrl((TG_SEL == CAM_TG_1 ?SENINF_TOP_SV2 :SENINF_TOP_SV4), pseninfPara->csi_para.seninfSrc);
           psensorDynamicInfo->PDAFInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_2 :CAM_SV_4);
        }
    }

    LOG_MSG("sensor index = %d, psensorDynamicInfo->TgInfo = %d; mSeninfMuxUsed[SENINF_MUX1] =%d, mSeninfMuxUsed[SENINF_MUX2] = %d",
                pseninfPara->sensor_idx, psensorDynamicInfo->TgInfo, mSeninfMuxUsed[SENINF_MUX1], mSeninfMuxUsed[SENINF_MUX2]);
    ret = setSeninfCSI2(pseninfPara,  enable);



	/*set mmdvfs */
    if (pseninfPara->csi_para.mipi_pixel_rate != 0) {
        unsigned int traget_clk = 0;
        for (int i = 0; i < supported_ISP_Clks.clklevelcnt; i++) {
            /*LOG_MSG("clklevel[%d]:%d ", i, supported_ISP_Clks.clklevel[i]);*/
            if (((MUINT64)supported_ISP_Clks.clklevel[i]*1000000*(1<<HALSENSOR_CAM_PIXEL_MODE)) > pseninfPara->csi_para.mipi_pixel_rate)
                traget_clk = supported_ISP_Clks.clklevel[i];
        }
        LOG_MSG("clklevelcnt:%d, traget_clk:%d, mipi pixel rate %" PRIu64,
        supported_ISP_Clks.clklevelcnt, traget_clk, pseninfPara->csi_para.mipi_pixel_rate);

        updateIspClk(traget_clk, pseninfPara->sensor_idx);

    } else
        LOG_MSG("sensor does not provide mipi_pixel_rate");

    return ret;

}


int SeninfDrvImp::configSensorInput(SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo,
        ACDK_SENSOR_INFO2_STRUCT *sensorInfo, bool enable) {

    (void) sensorInfo;

    psensorDynamicInfo->TgInfo   =
    psensorDynamicInfo->HDRInfo  =
    psensorDynamicInfo->PDAFInfo = CAM_TG_NONE;

    //Fixed pixel mode
    psensorDynamicInfo->pixelMode     = HALSENSOR_CAM_PIXEL_MODE;
   // psensorDynamicInfo->HDRPixelMode  =
  //  psensorDynamicInfo->PDAFPixelMode = HALSENSOR_CAMSV_PIXEL_MODE;

    psensorDynamicInfo->TgCLKInfo = 1000;
    switch(pseninfPara->inSrcTypeSel) {
        case MIPI_SENSOR:
            getCSIpara(&(pseninfPara->csi_para));
            configInputForCSI(pseninfPara, psensorDynamicInfo, enable);
            break;
        default:
            LOG_MSG("unsupported inSrcTypeSel %d", pseninfPara->inSrcTypeSel);
            break;
    }
    return 0;
}
int SeninfDrvImp::calibrateSensor(SENINF_CONFIG_STRUCT *pseninfPara){



    if (pseninfPara->inSrcTypeSel == MIPI_SENSOR){
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.seninf.deskew.enable", value, "1");
        if(atoi(value) == 0)
            return 0;
        MUINT64 pixel_rate = 0;
        int bpp=10;
        MUINT64 csi_skew_threshold = CSI_DESKEW_THRESHOLD;
        switch (pseninfPara->inDataType){
            case RAW_8BIT_FMT:
                bpp = 8;
            break;
            case RAW_12BIT_FMT:
                bpp = 12;
            break;
            default:
                bpp =10;
            break;
        }
        if(pseninfPara->csi_para.mipi_pixel_rate == 0) {
            pixel_rate= (MUINT64)pseninfPara->cropWidth * pseninfPara->cropHeight * (pseninfPara->frameRate/10);
            LOG_MSG("pixel_rate %" PRIu64", threshold %" PRIu64", h %d, w %d, fps %d,pdaf mode %d",
                     pixel_rate, csi_skew_threshold, pseninfPara->cropHeight, pseninfPara->cropWidth, pseninfPara->frameRate, pseninfPara->PDAFMode);

            pseninfPara->csi_para.mipi_pixel_rate = (pixel_rate * bpp *(100 + CSI_BLANKING_RATIO))/
                                                                        ((pseninfPara->csi_para.dlaneNum+1)* 100);
            pseninfPara->csi_para.mipi_pixel_rate *= pseninfPara->PDAFMode == PDAF_SUPPORT_RAW_DUALPD ?2 :1;
        } else {
            pixel_rate = pseninfPara->csi_para.mipi_pixel_rate;
            LOG_MSG("pixel_rate from driver %" PRIu64", threshold %" PRIu64", h %d, w %d, fps %d,pdaf mode %d",
                     pixel_rate, csi_skew_threshold, pseninfPara->cropHeight, pseninfPara->cropWidth, pseninfPara->frameRate, pseninfPara->PDAFMode);
            pseninfPara->csi_para.mipi_pixel_rate = ((pixel_rate * bpp)/(pseninfPara->csi_para.dlaneNum+1));
        }

        LOG_MSG("mipi_pixel_rate %" PRIu64" CSI_DESKEW_THRESHOLD %" PRIu64,
                        pseninfPara->csi_para.mipi_pixel_rate, csi_skew_threshold);

        if(pseninfPara->csi_para.mipi_pixel_rate > csi_skew_threshold){

#ifdef DPHY_HSW_DESKEW
            property_get("vendor.debug.seninf.deskew.mode", value, "1");
            if (pseninfPara->csi_para.mipi_deskew && atoi(value)) {
                int ret = 0;
                initHSWDeskew(pseninfPara);
                ret = enableHSWDeskew(pseninfPara);
                if (ret) {
                    uninitHSWDeskew(pseninfPara);
                }
            }
#else
            return csi_deskew_sw(pseninfPara);
#endif
        }
    }
    return 0;
}

int SeninfDrvImp::resetConfiguration(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    (void) sensorIdx;

    return 0;
}

int SeninfDrvImp::csi_deskew_sw(SENINF_CONFIG_STRUCT *pseninfPara){
    unsigned int lane_num = pseninfPara->csi_para.dlaneNum+1;
    int fps = pseninfPara->frameRate;
    int error_case= 0;
    int begin = 0;
    int end = 0;
    unsigned int i, j = 0 ;
    bool print_log = false;
    int ret =0 ;
    int csi_irq_en_flag = 0;
    unsigned int delay_code=0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.debug.seninf.deskew.log", value, "0");
    print_log = (atoi(value) > 0);

    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pseninfPara->csi_para.seninfSrc];


    LOG_MSG( "auto deskew start fps %d", fps);
    fps/=10;//fps was passed in with 10 base

    for(i = 0; i<=4; i++){
        enableDelayCode_deskew(i, false, pseninfPara->csi_para, false);
        applyDelayCode_deskew(i, 0, pseninfPara->csi_para, false);
    }

    csi_irq_en_flag = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_EN);

    DESKEW_LOG_IF(print_log, "autoDeskew_SW start condition: CSI2_IRQ_SET=0x%x CSI2_IRQ_STA=0x%x CSI2_PACKET=0x%x SENINF1_CSI2_DGB_SEL=0x%x",
        csi_irq_en_flag,
        SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS),
        SENINF_READ_REG(pSeninf, SENINF1_CSI2_DBG_PORT),
        SENINF_READ_REG(pSeninf, SENINF1_CSI2_DGB_SEL));

    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN, 0xFFFFFFFF);

    enable_deskew(true, pseninfPara->csi_para);

    for (i =0;i<lane_num;i++){
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_STATUS, 0xFFFFFFFF);
        DESKEW_LOG_IF(print_log, "======auto deskew lane %d start, clear all errors 0x%x=====", i, SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS));
        begin = 0;
        end = 0;
        error_case =0;
        enableDelayCode_deskew(i, true, pseninfPara->csi_para, print_log);
        applyDelayCode_deskew(i, 0, pseninfPara->csi_para, print_log);
        if (checkError_deskew(pseninfPara->csi_para, fps, print_log) == 1) {

            for (delay_code=1; delay_code <= MAX_CODE; delay_code++) {
                applyDelayCode_deskew(i, delay_code, pseninfPara->csi_para, print_log);
                if(checkError_deskew(pseninfPara->csi_para, fps, print_log) == 2){
                    break;
                } else {
                    if(isMaxCode(delay_code)){
                        error_case = 2;//delay code == 63
                    }
                }
            }
            if(error_case ==0) {
                delay_code+=1;
                for(;delay_code<=MAX_CODE;delay_code++) {
                    applyDelayCode_deskew(i, delay_code, pseninfPara->csi_para, print_log);
                    if( checkError_deskew(pseninfPara->csi_para, fps, print_log) == 1){
                        begin= delay_code;
                        break;
                    } else {
                        if(isMaxCode(delay_code)){
                            error_case = 3;
                        }
                    }

                }
            }
            if(error_case == 0) {
                delay_code+=1;
                for(;delay_code<=MAX_CODE;delay_code++) {
                    applyDelayCode_deskew(i, delay_code, pseninfPara->csi_para, print_log);
                    if(checkError_deskew(pseninfPara->csi_para, fps, print_log) == 2){
                        end = delay_code - 1;
                        break;
                    } else {
                        if(isMaxCode(delay_code)){
                            error_case = 4;
                        }
                    }

                }
            }
        } else {
            error_case = 1;
        }
        if(error_case !=0){
            LOG_ERR("deskew fail lane %d, error case =%d",i ,error_case);
            for(j = 0; j<=i; j++) {
                applyDelayCode_deskew(j, 0, pseninfPara->csi_para, false);
                enableDelayCode_deskew(j, false, pseninfPara->csi_para, false);
            }
            enable_deskew(false, pseninfPara->csi_para);
            ret = -1;
            break;
        } else {
            LOG_MSG("deskew result lane %d, begin =%d, end= %d",i ,begin, end);
            applyDelayCode_deskew(i,((begin+end)/2), pseninfPara->csi_para, print_log);

        }

    }

    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN, csi_irq_en_flag);//restore irq flag
    LOG_MSG( "auto deskew end");

    return ret;


}


void SeninfDrvImp::enable_deskew(bool enable, SENINF_CSI_PARA csi_para){

    mipi_rx_con_reg_base *pRxConReg=(mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[csi_para.seninfSrc];
    SENINF_WRITE_REG(pRxConReg, MIPI_RX_CONB0_CSI0, enable ?0x8 :0);
    if (csi_para.CSI2_IP == SENINF4_CSI1B) {//SW work around
        pRxConReg = (mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[SENINF_2];
        SENINF_WRITE_REG(pRxConReg, MIPI_RX_CONB0_CSI0, enable ?0x8 :0);
    }
}
void SeninfDrvImp::enableDelayCode_deskew(int lane, bool enable, SENINF_CSI_PARA csi_para, bool _log){
    mipi_rx_con_reg_base *pRxConReg=(mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[csi_para.seninfSrc];
    switch(lane) {
    case 1:
        SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = enable;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON90_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON90_CSI0));
        break;
    case 2:
        SENINF_BITS(pRxConReg, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = enable;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON94_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON94_CSI0));
        break;
    case 3:
        SENINF_BITS(pRxConReg, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = enable;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON98_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON98_CSI0));
        break;
    default:
        SENINF_BITS(pRxConReg, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = enable;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON8C_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON8C_CSI0));
        break;
    }
    if (csi_para.CSI2_IP == SENINF4_CSI1B) {
        if(lane == 1){
            pRxConReg=(mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[SENINF_2];
            SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = enable;
            DESKEW_LOG_IF(_log, "SENINF1_CSI1B MIPI_RX_CON90_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON90_CSI0));
        }
    }

}

/*1 ok, 2 fail*/
int SeninfDrvImp::checkError_deskew(SENINF_CSI_PARA csi_para, int fps, bool _log){
    int STATUS = 0;
    unsigned int currPacket = 0;
    int i,k = 0;
    int check_error_delay = 0;
    int busy_wait_cnt = 0;
    int wait_packet_times = 0;//(((1000/fps)+1)*1000)/CHECK_ERR_DELAY;//(34000/CHECK_ERR_DELAY);
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[csi_para.seninfSrc];
    char value[PROPERTY_VALUE_MAX] = {'\0'};


    property_get("vendor.debug.seninf.deskew.delay", value, "0");
    check_error_delay = atoi(value);
    if (check_error_delay == 0)
        check_error_delay = CHECK_ERR_DELAY;

    property_get("vendor.debug.seninf.deskew.busywait", value, "0");
    busy_wait_cnt = atoi(value);

    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_STATUS, 0xFFFFFFFF);

    DESKEW_LOG_IF(_log,"csi_para.seninfSrc %d SENINF1_CSI2_INT_STATUS 0x%x",
            csi_para.seninfSrc,
            SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS));
    for(i=0;i<MAX_PACKET_CNT;i++){//wait packet change for MAX_PACKET_CNT to get stable result
        wait_packet_times = (((1000/fps)+1)*1000)/CHECK_ERR_DELAY;
        wait_packet_times = wait_packet_times/10;
        currPacket = SENINF_READ_REG(pSeninf, SENINF1_CSI2_DBG_PORT);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_STATUS, 0xFFFFFFFF);
        while(currPacket == SENINF_READ_REG(pSeninf, SENINF1_CSI2_DBG_PORT)) {
            wait_packet_times--;
            if(wait_packet_times == 0){
            //                DESKEW_LOG_IF(_log,"!!!CSI error, pcket cnt stops for a long time !!! and CSI2_IRQ_STA 0x%x",
            //                SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS));
                DESKEW_LOG_IF(1,"!!!CSI error, pcket cnt stops for a long time !!! and CSI2_IRQ_STA 0x%x",
                SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS));
                return 2;
            }
            if(busy_wait_cnt){
                for(k=0; k<busy_wait_cnt;){
                    if(k == 0)
                         DESKEW_LOG_IF(_log,"k=0");
                    if(k == (busy_wait_cnt-1))
                         DESKEW_LOG_IF(_log,"k=busy_wait_cnt-1");
                    k+=1;
                }
            } else
                usleep(check_error_delay);
        }

        DESKEW_LOG_IF(_log,"pcket cnt changed!!! and CSI2_IRQ_STA 0x%x k%d",
            SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS),k);

    }
    STATUS = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS);
    if( (STATUS & ERROR_CODE) !=0){
        DESKEW_LOG_IF(_log,"!!!CSI error2, CSI2_IRQ_STA = 0x%x",STATUS);
        return 2;
    }
    return 1;
}

void SeninfDrvImp::applyDelayCode_deskew(int lane, unsigned int delay_code, SENINF_CSI_PARA csi_para, bool _log){
    mipi_rx_con_reg_base *pRxConReg=(mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[csi_para.seninfSrc];
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[csi_para.seninfSrc];
    DESKEW_LOG_IF(_log, "lane %d, delay code =%d", lane,delay_code);

    switch(lane) {
    case 1:
        //SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = 0;
        SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = delay_code;
        SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
        SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON90_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON90_CSI0));
        break;
    case 2:
        SENINF_BITS(pRxConReg, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_CODE) = delay_code;
        SENINF_BITS(pRxConReg, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 1;
        SENINF_BITS(pRxConReg, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON94_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON94_CSI0));
        break;
    case 3:
        SENINF_BITS(pRxConReg, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_CODE) = delay_code;
        SENINF_BITS(pRxConReg, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 1;
        SENINF_BITS(pRxConReg, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON98_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON98_CSI0));
        break;
    default:

        SENINF_BITS(pRxConReg, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_CODE) = delay_code;
        SENINF_BITS(pRxConReg, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 1;
        SENINF_BITS(pRxConReg, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
        DESKEW_LOG_IF(_log, "MIPI_RX_CON8C_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON8C_CSI0));
        break;
    }
    if (csi_para.CSI2_IP == SENINF4_CSI1B) {
        if(lane == 1){
            pRxConReg=(mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[SENINF_2];
            SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = delay_code;
            SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
            SENINF_BITS(pRxConReg, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;

            DESKEW_LOG_IF(_log, "SENINF1_CSI1B MIPI_RX_CON90_CSI0 = 0x%x", SENINF_READ_REG(pRxConReg, MIPI_RX_CON90_CSI0));
        }
    }
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_STATUS, 0xFFFFFFFF);
}



int SeninfDrvImp::getISPClk(){
    int ret =0;
    unsigned int Isp_clk = 0;
    ret= ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_ISP_CLK,&Isp_clk);
    if (ret < 0) {
        LOG_ERR("ERROR:KDIMGSENSORIOC_X_GET_ISP_CLK");
        Isp_clk = 0;
    }
    LOG_MSG("pixelModeArrange Isp_clk= %d",Isp_clk);
    return Isp_clk;
}

#ifdef DPHY_HSW_DESKEW
int SeninfDrvImp::enableHSWDeskew(SENINF_CONFIG_STRUCT *pseninfPara)
{
    int ret = 0, i = 0 , k=0 , m=0;
    UINT32 u4Buf = 0;
    UINT32 skew_code_buf = 0;
    UINT32 skew_code_pos = 0;
    UINT32 skew_code_target = 0;

    UINT32 skew_start[4] = { 0, 0, 0, 0};
    UINT32 skew_end[4] = { 0, 0, 0, 0};
    UINT32 skew_applied[4] = { 0, 0, 0, 0};
    UINT32 cal_skew_start[4] = { 0, 0, 0, 0};
    UINT32 cal_skew_end[4] = { 0, 0, 0, 0};
    UINT32 cal_skew_applied[4] = { 0, 0, 0, 0};
    UINT32 skew_data[4][4] = { { 0, 0, 0, 0}, { 0, 0, 0, 0}, { 0, 0, 0, 0}, { 0, 0, 0, 0} };
    UINT32 eye_open_target = 5;

    bool print_log = true;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pseninfPara->csi_para.seninfSrc];

    // waiting for de-skew result
    for (i=0 ; i<DESKEW_INIT_WAIT_MS ; i++) {
        usleep(1000);
        u4Buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONC4_CSI0) & 0xffff;
        if (u4Buf) {
            break;
        }
    }

    if (u4Buf) {
        //get hw de-skew raw data
        for (i = 0 ; i < 4 ; i++) {
            for (k = 0 ; k < 4 ; k++) {
                SENINF_READ_REG(pSeninf, MIPI_RX_CONC8_CSI0) &= ~0xff;
                SENINF_READ_REG(pSeninf, MIPI_RX_CONC8_CSI0) |= ((i*4)<<4) + k + 2;
                skew_data[i][k] = SENINF_READ_REG(pSeninf, MIPI_RX_CONCC_CSI0);
            }
        }

        //record hw de-skew results
        skew_applied[0] = (SENINF_READ_REG(pSeninf, MIPI_RX_CON7C_CSI0) & 0xff) >> 2;
        skew_applied[1] = (SENINF_READ_REG(pSeninf, MIPI_RX_CON80_CSI0) & 0xff) >> 2;
        skew_applied[2] = (SENINF_READ_REG(pSeninf, MIPI_RX_CON84_CSI0) & 0xff) >> 2;
        skew_applied[3] = (SENINF_READ_REG(pSeninf, MIPI_RX_CON88_CSI0) & 0xff) >> 2;

        SENINF_READ_REG(pSeninf, MIPI_RX_CONC8_CSI0) &= ~0xff;
        SENINF_READ_REG(pSeninf, MIPI_RX_CONC8_CSI0) |= 0x8;

        //get start/end result
        for (i=0;i<4;i++) {
            SENINF_READ_REG(pSeninf, MIPI_RX_CONC8_CSI0) &= ~0xC0;
            SENINF_READ_REG(pSeninf, MIPI_RX_CONC8_CSI0) |= (i<<6);
            skew_start[i] = (SENINF_READ_REG(pSeninf, MIPI_RX_CONCC_CSI0) >> 8) & 0x3f;
            skew_end[i] = (SENINF_READ_REG(pSeninf, MIPI_RX_CONCC_CSI0) >> 16) & 0x3f;
            //skew_applied[i] = (skew_start[i] + skew_end[i])/2;
        }

        // clear hw desk irq status
        SENINF_READ_REG(pSeninf, MIPI_RX_CONC4_CSI0) = 0xffff;
    } else {
        LOG_MSG("Deskew error!");
        return -1;
    }

    LOG_MSG("u4Buf = %x", u4Buf);

    int cnt = 0;
    while (u4Buf) {
        u4Buf &= (u4Buf - 1);
        cnt++;
    }

    //work on hw de-skew results
    if (cnt == pseninfPara->csi_para.dlaneNum + 1) {
        for (i = 0 ; i < 4 ; i++) {
            cal_skew_start[i] = 0;
            cal_skew_end[i] = 0;
            cal_skew_applied[i] = 0;
            eye_open_target = 11;
            skew_code_buf = 0;
            skew_code_target = 0x99;
            skew_code_pos = 0;

            for (k = 0 ; k < 4 ; k++, skew_code_pos += 1) {
                for (m = 30 ; m > 1 ; m -= 2, skew_code_pos += 1) {
                    skew_code_buf = (skew_data[i][k] >> m) & 0x3;

                    if (skew_code_buf == 0x1 || skew_code_buf == 0x2) {
                        if (skew_code_target != skew_code_buf) {
                            if (skew_code_target != 0x99) {
                               eye_open_target = 5;
                            }

                            if ((cal_skew_end[i] - cal_skew_start[i] + 1) >= eye_open_target && cal_skew_start[i] != 0) {
                                 cal_skew_applied[i] = (cal_skew_end[i] + cal_skew_start[i] ) >> 1;
                                 break;
                            }

                            skew_code_target = skew_code_buf;
                            cal_skew_start[i] = skew_code_pos;
                        }
                    } else {
                        if ((cal_skew_end[i] - cal_skew_start[i] + 1) >= eye_open_target && cal_skew_start[i] != 0) {
                            cal_skew_applied[i] = (cal_skew_end[i] + cal_skew_start[i]) >> 1;
                            break;
                        } else {
                            eye_open_target = 5;
                            cal_skew_start[i] = 0;
                            cal_skew_end[i] = 0;
                            cal_skew_applied[i] = 0;
                            skew_code_buf = 0;
                            skew_code_target = 0x99;
                        }
                    }

                    cal_skew_end[i] = skew_code_pos;
                }

                if (cal_skew_applied[i] > 0) {
                    break;
                }
            }
        }

        enable_deskew(true, pseninfPara->csi_para);

        for (i = 0 ; i < pseninfPara->csi_para.dlaneNum + 1; i++) {
            if (cal_skew_applied[i] > 0 && cal_skew_applied[i] < 0x2f) {
                enableDelayCode_deskew(i, true, pseninfPara->csi_para, print_log);
                applyDelayCode_deskew(i, cal_skew_applied[i], pseninfPara->csi_para, print_log);
            }
        }
    } else {
        LOG_MSG("Deskew lane error!");

        for (i = 0 ; i < pseninfPara->csi_para.dlaneNum + 1; i++) {
            LOG_MSG(" lane %d skew data: %x %x %x %x",\
                    i, skew_data[i][0],skew_data[i][1],skew_data[i][2],skew_data[i][3]);\
        }

        return -1;
    }

    if (print_log) {
        LOG_MSG("lane0(skew_start = 0x%x, skew_applied = 0x%x, skew_end = 0x%x), lane1(skew_start = 0x%x, skew_applied = 0x%x, skew_end = 0x%x), lane2(skew_start = 0x%x, skew_applied = 0x%x, skew_end = 0x%x), lane3(skew_start = 0x%x, skew_applied = 0x%x, skew_end = 0x%x)",\
                skew_start[0], skew_applied[0], skew_end[0], \
                skew_start[1], skew_applied[1], skew_end[1], \
                skew_start[2], skew_applied[2], skew_end[2], \
                skew_start[3], skew_applied[3], skew_end[3]);

        LOG_MSG("lane0(cal_skew_start = 0x%x, cal_skew_applied = 0x%x, cal_skew_end = 0x%x), lane1(cal_skew_start = 0x%x, cal_skew_applied = 0x%x, cal_skew_end = 0x%x), lane2(cal_skew_start = 0x%x, cal_skew_applied = 0x%x, cal_skew_end = 0x%x), lane3(cal_skew_start = 0x%x, cal_skew_applied = 0x%x, cal_skew_end = 0x%x)",\
                cal_skew_start[0],cal_skew_applied[0],cal_skew_end[0], \
                cal_skew_start[1],cal_skew_applied[1],cal_skew_end[1], \
                cal_skew_start[2],cal_skew_applied[2],cal_skew_end[2], \
                cal_skew_start[3],cal_skew_applied[3],cal_skew_end[3]);
    }

    return ret;
}

int SeninfDrvImp::initHSWDeskew(SENINF_CONFIG_STRUCT *pSeninfPara)
{
    int ret = 0;
    UINT32 buf, data;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pSeninfPara->csi_para.seninfSrc];

    buf = 0;
    data = 0;

    if(pSeninfPara->csi_para.CSI2_IP < SENINF1_CSI0A){
        //RG_CSI0_LNRD#_HSRX_DELAY_EN 1' b1 (per-lane)
        buf = SENINF_READ_REG(pSeninf, MIPI_RX_CON8C_CSI0);
        data = 0x1;
        SENINF_READ_REG(pSeninf, MIPI_RX_CON8C_CSI0) = buf&~0x1 | data; // lane 0 HSRX_DELAY_EN

        buf = SENINF_READ_REG(pSeninf, MIPI_RX_CON90_CSI0);
        data = 0x1;
        SENINF_READ_REG(pSeninf, MIPI_RX_CON90_CSI0) = buf&~0x1 | data; // lane 1 HSRX_DELAY_EN

        buf = SENINF_READ_REG(pSeninf, MIPI_RX_CON94_CSI0);
        data = 0x1;
        SENINF_READ_REG(pSeninf, MIPI_RX_CON94_CSI0) = buf&~0x1 | data; // lane 2 HSRX_DELAY_EN

        buf = SENINF_READ_REG(pSeninf, MIPI_RX_CON98_CSI0);
        data = 0x1;
        SENINF_READ_REG(pSeninf, MIPI_RX_CON98_CSI0) = buf&~0x1 | data; // lane 3 HSRX_DELAY_EN
    } else {
        buf = SENINF_READ_REG(pSeninf, MIPI_RX_CON8C_CSI0);
        data = 0x1;
        SENINF_READ_REG(pSeninf, MIPI_RX_CON8C_CSI0) = buf&~0x1 | data; // lane 0 HSRX_DELAY_EN

        buf = SENINF_READ_REG(pSeninf, MIPI_RX_CON90_CSI0);
        data = 0x1;
        SENINF_READ_REG(pSeninf, MIPI_RX_CON90_CSI0) = buf&~0x1 | data; // lane 1 HSRX_DELAY_EN
    }

    //DESKEW_DELAY_LENGTH
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_COND0_CSI0);
    data = 0x3f; // <-- DESKEW_DELAY_LENGTH
    SENINF_READ_REG(pSeninf, MIPI_RX_COND0_CSI0) = buf&~0x3f | data;

    //DESKEW_INITIAL_SETUP
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_COND0_CSI0);
    data = 0x5; // <-- DESKEW_INITIAL_SETUP
    SENINF_READ_REG(pSeninf, MIPI_RX_COND0_CSI0) = buf&~(0xf<<8) | (data<<8);

    //DESKEW_SETUP_TIME
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONB8_CSI0);
    data = 0x5; // <-- DESKEW_SETUP_TIME
    SENINF_READ_REG(pSeninf, MIPI_RX_CONB8_CSI0) = buf&~0xf | (data);

    //DESKEW_DETECTION_CNT
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONBC_CSI0);
    data = 0x4; // <-- DESKEW_DETECTION_CNT
    SENINF_READ_REG(pSeninf, MIPI_RX_CONBC_CSI0) = buf&~(0x7f<<8) | (data<<8);

    //SYNC_SEQ_MASK_1
    buf = SENINF_READ_REG(pSeninf, SENINF1_CSI2_DESKEW_SYNC);
    data = 0x0;  // <-- SYNC_SEQ_MASK_1
    SENINF_READ_REG(pSeninf, SENINF1_CSI2_DESKEW_SYNC) = buf&~(0xffff<<0) | (data<<0);

    //DESKEW_IP_SEL
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONB0_CSI0);
    data = 0x0;  // <-- DESKEW_IP_SEL
    SENINF_READ_REG(pSeninf, MIPI_RX_CONB0_CSI0) = buf&~(0x1<<30) | (data<<30);

    //DESKEW_ENABLE
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONB0_CSI0);
    data = 0x1;  // <-- DESKEW_IP_EN
    SENINF_READ_REG(pSeninf, MIPI_RX_CONB0_CSI0) = buf&~(0x1<<31) | (data<<31);

    //DESKEW_INTERRUPT_ENABLE
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONC0_CSI0);
    data = 0xffff;  // <-- DESKEW_INTERRUPT_ENABLE
    SENINF_READ_REG(pSeninf, MIPI_RX_CONC0_CSI0) = buf&~(0xffff<<0) | (data<<0);

#ifdef DPHY_HSW_DESKEW
    buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONB0_CSI0);
    data = 0x1;  // <-- Enable whole range scan
    SENINF_READ_REG(pSeninf, MIPI_RX_CONB0_CSI0) = buf&~(0xf<<12) | (data<<12);

    buf = SENINF_READ_REG(pSeninf, MIPI_RX_CONBC_CSI0);
    data = 0x6; // <-- DESKEW_DETECTION_CNT
    SENINF_READ_REG(pSeninf, MIPI_RX_CONBC_CSI0) = buf&~(0x7f<<8) | (data<<8);
#endif

    // reset and enable hw deskew
    SENINF_READ_REG(pSeninf, MIPI_RX_CONC4_CSI0) &= ~0x80000000;
    SENINF_READ_REG(pSeninf, MIPI_RX_CONC4_CSI0) |= 0x80000000;

    return ret;
}

int SeninfDrvImp::uninitHSWDeskew(SENINF_CONFIG_STRUCT *pSeninfPara)
{
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pSeninfPara->csi_para.seninfSrc];
    int i;

    SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_ENABLE) = 0x1;
    SENINF_BITS(pSeninf, MIPI_RX_CONC0_CSI0, DESKEW_INTERRUPT_ENABLE) = 0x16;

    enable_deskew(true, pSeninfPara->csi_para);

    for (i = 0 ; i < pSeninfPara->csi_para.dlaneNum + 1; i++) {
        enableDelayCode_deskew(i, true, pSeninfPara->csi_para, true);
        applyDelayCode_deskew(i, 0, pSeninfPara->csi_para, true);
    }

    return 0;
}
#endif
