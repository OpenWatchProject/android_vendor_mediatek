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

//
/******************************************************************************
*
*******************************************************************************/
#define SENSOR_DEV_NAME     "/dev/kd_camera_hw"
#define ISP_DEV_NAME         "/dev/camera-isp"
#define DEBUG_SCAM_CALI 1
#define DEBUG_CSI_AUTO_DESKEW 1
#define CSI_BLANKING_RATIO 10
#define CSI_DESKEW_THRESHOLD 1610612736 //1.5*1024*1024*1024

//#define FPGA (1)//ToDo: remove after FPGA
#define SETTLE_DELAY 0x15
#define CAM_ISP_RANGE 0xA000
#define SENINF_HW_RANGE 0x4000
#define MIPIRX_HW_RANGE 0x20000
#define GPIO_HW_RANGE 0x1000
#define SCAM_CALI_TIMEOUT 0xFFFFF // 0x3D9DEA//14.5fps   0x2f72c//30fps

int best_delay_value = -1;//for scam
bool Positive_Calibration = false;



/*Efuse definition*/
#define CSI2_EFUSE_SET
/*Analog Sync Mode*/
#define ANALOG_SYNC
/*define sw Offset cal*/
#define CSI2_SW_OFFSET_CAL

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
    mpIspHwRegAddr = NULL;
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
    MINT32 cam_isp_addr = 0x15000000;
    MINT32 seninf_base_addr = 0x15008000;
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

    // Open isp driver
#if USE_ISP_OPEN_FD
    m_pIspDrv = IspDrv::createInstance();
    if (m_pIspDrv == NULL)
    {
        LOG_ERR("IspDrv::createInstance() fail");
        return -1;
    }
    mfd = m_pIspDrv->isp_fd_open("seninf_drv");
    LOG_MSG("IspDrv mfd(%d)", mfd);
#else
    mfd = open(ISP_DEV_NAME, O_RDWR);
#endif
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
#if 1
   // reference isp_drv.cpp
   // gIspDrvObj[CAM_A].m_pIspHwRegAddr = (MUINT32 *) mmap(0, CAM_BASE_RANGE, (PROT_READ | PROT_WRITE | PROT_NOCACHE), MAP_SHARED, this->m_Fd, CAM_A_BASE_HW);
    mpIspHwRegAddr = (unsigned int *) mmap(0, CAM_ISP_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, cam_isp_addr);
    if (mpIspHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s", errno, strerror(errno));
        return -4;
    }
#else
    mpIspHwRegAddr = NULL;
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
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_EN) = 1;

    /*CSI2 Base address*/
    //MMAP only support Page alignment(0x1000)
    mpCSI2RxAnaBaseAddr[SENINF1_CSI0] = mpCSI2RxAnalogRegStartAddrAlign;              //11C1 0000
    mpCSI2RxAnaBaseAddr[SENINF2_CSI1] = mpCSI2RxAnaBaseAddr[SENINF1_CSI0] + (0x10000/4); //11C2 0000

    /*SenINF base address*/
    mpSeninfCtrlRegAddr[SENINF_1] = mpSeninfHwRegAddr;            //15008000
    mpSeninfCtrlRegAddr[SENINF_2] = mpSeninfHwRegAddr + (0x400/4);//15008400
    mpSeninfCtrlRegAddr[SENINF_3] = mpSeninfHwRegAddr + (0x800/4);
    mpSeninfCtrlRegAddr[SENINF_4] = mpSeninfHwRegAddr + (0xC00/4);
    /*SenINF Mux Base address*/
    mpSeninfMuxBaseAddr[SENINF_MUX1] = mpSeninfHwRegAddr + (0x0120/4);
    mpSeninfMuxBaseAddr[SENINF_MUX2] = mpSeninfHwRegAddr + (0x0520/4);
    mpSeninfMuxBaseAddr[SENINF_MUX3] = mpSeninfHwRegAddr + (0x0920/4);
    mpSeninfMuxBaseAddr[SENINF_MUX4] = mpSeninfHwRegAddr + (0x0d20/4);

    mpSeninfCSIRxConfBaseAddr[SENINF_1] = mpSeninfHwRegAddr + (0x0300/4);//18040800
    mpSeninfCSIRxConfBaseAddr[SENINF_2] = mpSeninfHwRegAddr + (0x0700/4);//18041800

    mpIspHwRegBaseAddr[SENINF_TOP_TG1] = mpIspHwRegAddr;
    mpIspHwRegBaseAddr[SENINF_TOP_TG2] = mpIspHwRegAddr + (0x2000/4);
    mpIspHwRegBaseAddr[SENINF_TOP_SV1] = mpIspHwRegAddr;
    mpIspHwRegBaseAddr[SENINF_TOP_SV2] = mpIspHwRegAddr + (0x800/4);

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
            mCSI[0] = devinfo.data[100];//0x11c00180
            mCSI[1] = devinfo.data[101];//0x11c00184
        }
        LOG_MSG("Efuse Data:0x11c00180= 0x%x, 0x11c00184= 0x%x", mCSI[0], mCSI[1]);
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
        mSeninfMuxUsed[SENINF_MUX3] = -1;
        mSeninfMuxUsed[SENINF_MUX4] = -1;

        setSeninf1CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2
        setSeninf2CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2
        //setSeninf3CSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2

        //set CMMCLK mode 0
        //*(mpGpioHwRegAddr + (0xEA0/4)) &= 0xFFF8;

        // Jessy added for debug mpIPllCon0RegAddr, to check the setting flow of mlck pll is correct
        //LOG_MSG("mpIPllCon0RegAddr %x, State: uinit", *mpIPllCon0RegAddr);

#if 1
        if ( 0 != mpIspHwRegAddr ) {
            if(munmap(mpIspHwRegAddr, CAM_ISP_RANGE)!=0){
                LOG_ERR("mpIspHwRegAddr munmap err, %d, %s", errno, strerror(errno));
            }
            mpIspHwRegAddr = NULL;
        }
#else
        mpIspHwRegAddr = NULL;

#endif

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

    /* Target: 2*(11+1)*(1/mclkPLL) = 1Mhz */
    pmclk_para->mclkPLL = 1000;
    /*sub sample:csr_tg_tmp_stp */
    if(pmclk_para->mclkFreq == 24)
        csr_tg_tmp_stp = 11;/*unit :Khz. 1000kHz = 1Mhz */
    else /*mclk1 == 26000*/
        csr_tg_tmp_stp = 12;/*unit :Khz. 1000kHz = 1Mhz */

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
    if (mclkIdx == eMclk_1)
        return setMclk1(pcEn, mclkSel, clkCnt, clkPol, clkFallEdge, clkRiseEdge, padPclkInv, TimestampClk);
    else if (mclkIdx == eMclk_2)
        return setMclk2(pcEn, mclkSel, clkCnt, clkPol, clkFallEdge, clkRiseEdge, padPclkInv, TimestampClk);
    else if (mclkIdx == eMclk_3)
        return setMclk3(pcEn, mclkSel, clkCnt, clkPol, clkFallEdge, clkRiseEdge, padPclkInv, TimestampClk);
    else
        LOG_ERR("never here");
    return -1;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk1(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv, unsigned long TimestampClk
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    static MUINT32 mMclk1User = 0;
    if(1 == pcEn)
        mMclk1User++;
    else
        mMclk1User--;

#if 1
    if(pcEn == 1 && mMclk1User == 1){
        sensorMclk.on = 1;
        sensorMclk.TG = 0;
        sensorMclk.freq = mclkSel;
        ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
        if (ret < 0) {
           LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
        }


        SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, PCEN) = pcEn;
    }
    else if(pcEn == 0 && mMclk1User == 0){
        sensorMclk.on = 0;
        sensorMclk.TG = 0;
        sensorMclk.freq = mclkSel;
        ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
        if (ret < 0) {
           LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
        }
        SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, PCEN) = pcEn;
    }
 #endif
    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 0)? (clkCnt+1)>>1 : 0;//avoid setting larger than clkCnt
    /* 0x1A040604 */
    //Seninf Top pclk clear gating
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN) = 1;
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN) = 1;

    SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKRS) = clkRiseEdge;
    SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL) = clkFallEdge;
    SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT) = clkCnt;


    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKFL_POL) = (clkCnt & 0x1) ? 0 : 1;
    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKPOL) = clkPol;

    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, TGCLK_SEL) = 1;//force PLL due to ISP engine clock dynamic spread
    //SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, ADCLK_EN) = 1;//FPGA experiment
    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, PAD_PCLK_INV) = padPclkInv;
    LOG_MSG("[setMclk1]pcEn(%d), clkPol(%d), mMclk1User(%d), TimestampClk(%d), SENINF_TG1_PH_CNT(0x%x), mclkSel %lu",
        (MINT32)pcEn, (MINT32)clkPol, mMclk1User, (MINT32)TimestampClk, SENINF_READ_REG(pSeninf,SENINF_TG1_PH_CNT), mclkSel);

#if(defined(FPGA))
    //JR //ToDo: remove
    *(mpSeninfHwRegAddr+0x200/4) = 0xa0000001;
    *(mpSeninfHwRegAddr+0x204/4) = 0x00030002; // 12/2 = 6Mhz
    //*(mpSeninfHwRegAddr+0x204/4) = 0x000010001; // 12/1 = 12Mhz
    *(mpSeninfHwRegAddr+0x100/4) |= 0x1000;
#endif
    usleep(100);



    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk2(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv, unsigned long TimestampClk
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    static MUINT32 mMclk2User = 0;
    if(1 == pcEn)
        mMclk2User++;
    else
        mMclk2User--;

#if 1
        if(pcEn == 1 && mMclk2User == 1){
            sensorMclk.on = 1;
            sensorMclk.TG = 1;
            sensorMclk.freq = mclkSel;

            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
                LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
            }
            SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN) = pcEn;
        }
        else if(pcEn == 0 && mMclk2User == 0) {
            sensorMclk.on = 0;
            sensorMclk.TG = 1;
            sensorMclk.freq = mclkSel;

            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
               LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
            }
            SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN) = pcEn;
        }
 #endif

    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 0)? (clkCnt+1)>>1 : 0;//avoid setting larger than clkCnt
    /* 0x1A041604 */
    //Seninf Top pclk clear gating
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN) = 1;
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN) = 1;

    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKCNT) = clkCnt;       // Sensor master clock falling edge control
    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKRS ) = clkRiseEdge;  // Sensor master clock rising edge control
    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKFL ) = clkFallEdge;//fpga


    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKFL_POL   ) = (clkCnt & 0x1) ? 0 : 1;    // Sensor clock falling edge polarity
    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKPOL      ) = clkPol;    // Sensor master clock polarity control

    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, TGCLK_SEL   ) = 1;//force PLL due to ISP engine clock dynamic spread
    //SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, ADCLK_EN    ) = 1;    // Enable sensor master clock (mclk) output to sensor. Note that to set sensor master clock driving setting,
    //SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN        ) = pcEn;    // TG phase counter enable control
    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PAD_PCLK_INV) = padPclkInv;    // Pixel clock inverse in PAD side
    //
    //Enable timestamp counter ( TM need enable Sensor interface 1)
    //TM Enable 1.turn on sensor interface1"seninf_en"  2. seninf_tg_ph_cnt  bit[1:0]  set to 2'b01
    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, TGCLK_SEL) = 1;

    LOG_MSG("[setMclk2]pcEn(%d), clkPol(%d), mMclk2User(%d), TimestampClk(%d), SENINF_TG2_PH_CNT(0x%x), mclkSel %lu",
        (MINT32)pcEn, (MINT32)clkPol, mMclk2User, (MINT32)TimestampClk, SENINF_READ_REG(pSeninf,SENINF_TG2_PH_CNT), mclkSel);

#if(defined(FPGA))
    //JR //ToDo: remove
    *(mpSeninfHwRegAddr+0x600/4) = 0xa0000001;
    *(mpSeninfHwRegAddr+0x604/4) = 0x00010001; // 12/2 = 6Mhz
    //*(mpSeninfHwRegAddr+0x204/4) = 0x000000000; // 12/1 = 12Mhz
    *(mpSeninfHwRegAddr+0x100/4) |= 0x1000;

#endif
    usleep(100);


    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk3(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv, unsigned long TimestampClk
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    static MUINT32 mMclk3User = 0;
    if(1 == pcEn)
        mMclk3User++;
    else
        mMclk3User--;

#if 1
        if(pcEn == 1 && mMclk3User == 1){
            sensorMclk.on = 1;
            sensorMclk.TG = 2;
            sensorMclk.freq = mclkSel;

            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
                LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
            }
            SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN) = pcEn;
        }
        else if(pcEn == 0 && mMclk3User == 0) {
            sensorMclk.on = 0;
            sensorMclk.TG = 2;
            sensorMclk.freq = mclkSel;

            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
               LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL");
            }
            SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN) = pcEn;
        }
 #endif

    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 0)? (clkCnt+1)>>1 : 0;//avoid setting larger than clkCnt
    /* 0x1A041604 */
    //Seninf Top pclk clear gating
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN) = 1;
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN) = 1;

    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKCNT) = clkCnt;       // Sensor master clock falling edge control
    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKRS ) = clkRiseEdge;  // Sensor master clock rising edge control
    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKFL ) = clkFallEdge;//fpga


    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKFL_POL   ) = (clkCnt & 0x1) ? 0 : 1;    // Sensor clock falling edge polarity
    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKPOL      ) = clkPol;    // Sensor master clock polarity control

    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, TGCLK_SEL   ) = 1;//force PLL due to ISP engine clock dynamic spread
    //SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, ADCLK_EN    ) = 1;    // Enable sensor master clock (mclk) output to sensor. Note that to set sensor master clock driving setting,
    //SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN        ) = pcEn;    // TG phase counter enable control
    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PAD_PCLK_INV) = padPclkInv;    // Pixel clock inverse in PAD side
    //
    //Enable timestamp counter ( TM need enable Sensor interface 1)
    //TM Enable 1.turn on sensor interface1"seninf_en"  2. seninf_tg_ph_cnt  bit[1:0]  set to 2'b01
    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, TGCLK_SEL) = 1;

    LOG_MSG("[setMclk3]pcEn(%d), clkPol(%d), mMclk3User(%d), TimestampClk(%d), SENINF_TG2_PH_CNT(0x%x), mclkSel %lu",
        (MINT32)pcEn, (MINT32)clkPol, mMclk3User, (MINT32)TimestampClk, SENINF_READ_REG(pSeninf,SENINF_TG2_PH_CNT), mclkSel);

    usleep(100);


    return ret;
}

/*******************************************************************************
*
********************************************************************************/
#define DEBUG_DELAY 3000
int SeninfDrvImp::sendCommand(int cmd, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    int ret = 0;
    static int mipi_packet_cnt[SENINF_NUM] = {0};

    (void)arg2;
    (void)arg3;

    Mutex::Autolock lock(mLock_mmap);//for uninit, some pointer will be set to NULL
    if (mpSeninfHwRegAddr == NULL) {
        LOG_ERR("mpSeninfHwRegAddr = NULL, seninf has been uninit, stop dump to avoid NE");
        return ret;
    }

    switch (cmd) {
    case CMD_SENINF_DEBUG_TASK:
        {
            seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
            unsigned int temp1 = 0;
            unsigned int temp2 = 0;
            unsigned int mmsys_clk = 0;

            /*Read clear*/
            //SENINF_BITS(pSeninf, SENINF1_CSI2_INT_EN, INT_WCLR_EN) = 0;
            //SENINF_BITS(pSeninf, SENINF2_CSI2_INT_EN, INT_WCLR_EN) = 0;
            //SENINF_BITS(pSeninf, SENINF4_CSI2_INT_EN, INT_WCLR_EN) = 0;
            /*Please don't use meter oftenly*/
            if(ISP_CLK_GET == 0)
            {
                mmsys_clk = 3;
                ret= ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_CSI_CLK,&mmsys_clk);
                ISP_CLK_GET =1;
            }

            temp1 = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS);
            temp2 = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS);

            LOG_MSG("SENINF_TOP_MUX_CTRL(0x%x) ISP_clk(%d)",
                SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL), mmsys_clk);

            if (temp1 != 0x1004){
                /* sync error, crc error, frame end not sync */
                SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0xffffffff);
                usleep(DEBUG_DELAY);
                LOG_MSG("SENINF1_NCSI2_INT_STATUS(0x%x), CLR SENINF1_NCSI2_INT_STATUS(0x%x)",
                    temp1, SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS));
            }

            if (temp2 != 0x1004){
                /* sync error, crc error, frame end not sync */
                SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_STATUS,0xffffffff);
                usleep(DEBUG_DELAY);
                LOG_MSG("SENINF2_NCSI2_INT_STATUS(0x%x), CLR SENINF2_NCSI2_INT_STATUS(0x%x)",
                    temp2, SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS));
            }

            /*Sensor Interface Control */
             LOG_MSG("SENINF1_NCSI2_CTL(0x%x), SENINF2_NCSI2_CTL(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF1_NCSI2_CTL), SENINF_READ_REG(pSeninf,SENINF2_NCSI2_CTL));
            /*Sensor Interface IRQ */

            if (SENINF_READ_REG(pSeninf,SENINF1_NCSI2_CTL) & 0x1){
                temp1 = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
                if (temp1 != (unsigned int)mipi_packet_cnt[SENINF_1]){
                    ret = -1;
                }
                else if ((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS) & 0xff0)!= 0){
                    ret = -2;
                }
                mipi_packet_cnt[SENINF_1] = temp1;
            }

            if (SENINF_READ_REG(pSeninf,SENINF2_NCSI2_CTL) & 0x1){
                temp2 = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT);
                if (temp2 != (unsigned int)mipi_packet_cnt[SENINF_2]) {
                    ret = -1;
                }
                else if ((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS) & 0xff0)!= 0) {
                    ret = -2;
                }
                mipi_packet_cnt[SENINF_2] = temp2;
            }

            LOG_MSG("SENINF1_PkCnt(0x%x), SENINF2_PkCnt(0x%x)",
                mipi_packet_cnt[SENINF_1], mipi_packet_cnt[SENINF_2]);

            LOG_MSG("SENINF1_IRQ(0x%x), SENINF2_IRQ(0x%x)",
                SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS));
            /*Mux1 */
            LOG_MSG("SENINF1_MUX_CTRL(0x%x), SENINF1_MUX_INTSTA(0x%x), SENINF1_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF1_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF1_MUX_DEBUG_2));
            if(SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA) & 0x1)
            {
                SENINF_WRITE_REG(pSeninf,SENINF1_MUX_INTSTA,0xffffffff);
                usleep(DEBUG_DELAY);
                 LOG_MSG("after reset overrun, SENINF1_MUX_CTRL(0x%x), SENINF1_MUX_INTSTA(0x%x), SENINF1_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF1_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF1_MUX_DEBUG_2));
            }
            /*Mux2 */
            LOG_MSG("SENINF2_MUX_CTRL(0x%x), SENINF2_MUX_INTSTA(0x%x), SENINF2_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF2_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF2_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF2_MUX_DEBUG_2));
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
            SENINF_WRITE_REG(pSeninf,SENINF3_MUX_INTSTA,0xffffffff);
            /*Mux4  for PDAF*/
            LOG_MSG("SENINF4_MUX_CTRL(0x%x), SENINF4_MUX_INTSTA(0x%x), SENINF4_MUX_DEBUG_2(0x%x)",
                 SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF4_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF4_MUX_DEBUG_2));
            SENINF_WRITE_REG(pSeninf,SENINF4_MUX_INTSTA,0xffffffff);

            /*Scam*/
            LOG_MSG("SCAM1_CFG(0x%x), SCAM1_CON(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_CFG), SENINF_READ_REG(pSeninf,SCAM1_CON));
            LOG_MSG("SCAM1_INT(0x%x), SCAM1_SIZE(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_INT), SENINF_READ_REG(pSeninf,SCAM1_SIZE));
            LOG_MSG("SCAM1_CFG2(0x%x), SCAM1_INFO0(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_CFG2), SENINF_READ_REG(pSeninf,SCAM1_INFO0));
            LOG_MSG("SCAM1_INFO1(0x%x), SCAM1_INFO2(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_INFO1), SENINF_READ_REG(pSeninf,SCAM1_INFO2));
            LOG_MSG("SCAM1_INFO3(0x%x), SCAM1_INFO4(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_INFO3), SENINF_READ_REG(pSeninf,SCAM1_INFO4));
            LOG_MSG("SCAM1_INFO5(0x%x), SCAM1_INFO6(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_INFO5), SENINF_READ_REG(pSeninf,SCAM1_INFO6));
            LOG_MSG("SCAM1_DDR_CTRL(0x%x), SCAM1_TIME_OUT(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_DDR_CTRL), SENINF_READ_REG(pSeninf,SCAM1_TIME_OUT));
            LOG_MSG("SCAM1_LINE_ID_START(0x%x)",
                    SENINF_READ_REG(pSeninf,SCAM1_LINE_ID_START));
        }
        break;

    case CMD_SENINF_DEBUG_TASK_CAMSV:
        {
	        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
            LOG_MSG("SENINF_TOP_MUX_CTRL(0x%x)", SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL));
	        /*Mux3 for HDR*/
	        LOG_MSG("(HDR)SENINF3_CTRL(0x%x), IRQ(0x%x), WidthHeight(0x%x), Debug(0x%x)",
                    SENINF_READ_REG(pSeninf,SENINF3_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF3_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF3_MUX_DEBUG_2), SENINF_READ_REG(pSeninf,SENINF3_MUX_DEBUG_1));
	        SENINF_WRITE_REG(pSeninf,SENINF3_MUX_INTSTA,0xffffffff);
	        /*Mux4  for PDAF*/
	        LOG_MSG("(PDAF)SENINF4_CTRL(0x%x), IRQ(0x%x), WidthHeight(0x%x), Debug(0x%x)",
                    SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF4_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF4_MUX_DEBUG_2), SENINF_READ_REG(pSeninf,SENINF4_MUX_DEBUG_1));
	        SENINF_WRITE_REG(pSeninf,SENINF4_MUX_INTSTA,0xffffffff);
        }
        break;

    case CMD_SENINF_GET_SENINF_ADDR:
        *(unsigned long *) arg1 = (unsigned long) mpSeninfHwRegAddr;
        break;

    case CMD_SENINF_SET_DUAL_CAM_MODE:
        setDualCamMode(*(bool *) arg1);
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
        SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DI,((vc3Id)<<24)|((vc2Id)<<16)|((vc1Id)<<8)|(vc0Id));

        /*Clear*/
        SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DI_CTRL, 0x00);

        if((vc0Id == 0) && (vc1Id == 0) && (vc2Id == 0) && (vc3Id == 0) && (vc4Id == 0) && (vc5Id == 0))
        {
            return ret;
        }

        if((vc0Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, VC0_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, DT0_INTERLEAVING) = 1;
        }

        if((vc1Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, VC1_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, DT1_INTERLEAVING) = 1;
        }

        if((vc2Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, VC2_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, DT2_INTERLEAVING) = 1;
        }

        if((vc3Id&0xfc) != 0)
        {
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, VC3_INTERLEAVING) = 1;
            SENINF_BITS(pSeninf, SENINF1_NCSI2_DI_CTRL, DT3_INTERLEAVING) = 1;
        }

        LOG_MSG("VC Data(0x%x)",SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DI));
        LOG_MSG("VC Ctrl(0x%x)",SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DI_CTRL));

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

        if(1 == PixelMode) { /*2 Pixel*/
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_PIX_SEL) = 1;
        }
        else if(2 == PixelMode) { /* 4 Pixel*/
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_PIX_SEL) = 0;
        }
        else {/* 1 pixel*/
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

    return ret;

}
#endif
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf4Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    /*0x1a04_3200*/
    SENINF_BITS(pSeninf, SENINF4_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF4_CTRL, PAD2CAM_DATA_SEL) = padSel;

    SENINF_BITS(pSeninf, SENINF4_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; //[15:12]: 0:csi, 8:NCSI2

    return ret;

}
#if 0
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
#if 1
    (void) mode;
    return 0;
#else
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
#endif
}
#endif
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::Efuse(unsigned long csi_sel)
{
    int ret = 0;
    unsigned int temp = 0;
    unsigned int *mpCSI2RxAnalogRegAddr;

    if ((mCSI[0] != 0) && (mCSI[1] != 0) && (csi_sel == 0)) {
        mpCSI2RxAnalogRegAddr = mpCSI2RxAnaBaseAddr[0];

        temp = *(mpCSI2RxAnalogRegAddr);
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[0]&0x00000f00)<<4), mpCSI2RxAnalogRegAddr);
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[0]&0x000000f0)<<8), mpCSI2RxAnalogRegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[0]&0x0000000f)<<12), mpCSI2RxAnalogRegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[1]&0x000f0000)>>4), mpCSI2RxAnalogRegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[1]&0x0000f000)), mpCSI2RxAnalogRegAddr + (0x10/4));

        LOG_MSG("CSI-0 EFUSE 0x00=0x%x, 0x04=0x%x,0x08=0x%x, 0x0c=0x%x, 0x10=0x%x",
                *(mpCSI2RxAnalogRegAddr),
                *(mpCSI2RxAnalogRegAddr + (0x04/4)),
                *(mpCSI2RxAnalogRegAddr + (0x08/4)),
                *(mpCSI2RxAnalogRegAddr + (0x0c/4)),
                *(mpCSI2RxAnalogRegAddr + (0x10/4)));
    } else if ((mCSI[1] != 0) && (csi_sel == 1)) {
        mpCSI2RxAnalogRegAddr = mpCSI2RxAnaBaseAddr[1];

        temp = *(mpCSI2RxAnalogRegAddr );
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[1]&0x0f000000)>>12), mpCSI2RxAnalogRegAddr);
        temp = *(mpCSI2RxAnalogRegAddr + (0x04/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[1]&0x00f00000)>>8), mpCSI2RxAnalogRegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x08/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[1]&0x000f0000)>>4), mpCSI2RxAnalogRegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x0C/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[1]&0x0000f000)), mpCSI2RxAnalogRegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalogRegAddr + (0x10/4));
        mt65xx_reg_writel((temp&(~0xf000))|((mCSI[1]&0x00000f00)<<4), mpCSI2RxAnalogRegAddr + (0x10/4));

        LOG_MSG("CSI-1 EFUSE 0x00=0x%x, 0x04=0x%x,0x08=0x%x, 0x0c=0x%x, 0x10=0x%x",
                *(mpCSI2RxAnalogRegAddr),
                *(mpCSI2RxAnalogRegAddr + (0x04/4)),
                *(mpCSI2RxAnalogRegAddr + (0x08/4)),
                *(mpCSI2RxAnalogRegAddr + (0x0c/4)),
                *(mpCSI2RxAnalogRegAddr + (0x10/4)));
    } else {
        LOG_MSG("No EFUSE data for CSI-%lu", csi_sel);
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
    //unsigned int *mpCSI2RxAnalog0RegAddr = mpSeninfCSIRxConfBaseAddr[0];
    unsigned int *mpCSI2RxAnalog0RegAddr = mpCSI2RxAnaBaseAddr[0];
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
        // disable NCSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2 first
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFE0); // disable CSI2 first

        //disable mipi BG
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalog0RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x10/4));

        // disable mipi pin
        temp = *(mpCSI2RxAnalog0RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//clock lane input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//data lane 0 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//data lane 1 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//data lane 2 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//data lane 3 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x10/4));
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
     //unsigned int *mpCSI2RxAnalog0RegAddr = mpSeninfCSIRxConfBaseAddr[1];
     unsigned int *mpCSI2RxAnalog0RegAddr = mpCSI2RxAnaBaseAddr[1];
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
        // disable NCSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2 first
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFE0); // disable CSI2 first

        //disable mipi BG
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalog0RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x10/4));

        // disable mipi pin
        temp = *(mpCSI2RxAnalog0RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//clock lane input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//data lane 0 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//data lane 1 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//data lane 2 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//data lane 3 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x10/4));
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
    //unsigned int *mpCSI2RxAnalog0RegAddr = mpSeninfCSIRxConfBaseAddr[0];
    unsigned int *mpCSI2RxAnalog0RegAddr = mpCSI2RxAnaBaseAddr[0];
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
        // disable NCSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2 first
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFE0); // disable CSI2 first

        //disable mipi BG
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalog0RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x10/4));

        // disable mipi pin
        temp = *(mpCSI2RxAnalog0RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//clock lane input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//data lane 0 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//data lane 1 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//data lane 2 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//data lane 3 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x10/4));
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
        pSeninfpara->seninfSrc = SENINF_2;
        pSeninfpara->CSI2_IP  = SENINF2_CSI1;
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
    unsigned int *mpCSI2RxAnalog0RegAddr;
    unsigned int temp = 0, Isp_clk = 0, msettleDelay = 0;
    unsigned int dataTermDelay, dataSettleDelay, clkTermDelay, vsyncType, dlaneNum;
    unsigned int dataheaderOrder, mipi_type, dpcm;
    unsigned int padSel, inSrcTypeSel, CalSel,mipiskew;
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
    dpcm            = pSeninfPara->csi_para.dpcm;
    mipiskew        = pSeninfPara->csi_para.mipi_deskew;
    pSeninf         = (seninf_reg_t *)mpSeninfHwRegAddr;
    SENINF_CSI2_IP_ENUM CSI2_IP = pSeninfPara->csi_para.CSI2_IP;

    pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pSeninfPara->csi_para.seninfSrc];
    mpCSI2RxAnalog0RegAddr = mpCSI2RxAnaBaseAddr[pSeninfPara->csi_para.seninfSrc];

    if(CSI2_IP == SENINF2_CSI1) //Use sensor interface 2  & CSI1
    {
        CalSel = 1;
    }
    else if(CSI2_IP == SENINF1_CSI0)
    {
        CalSel = 0;
    }
    else {
        LOG_ERR("unsupported CSI configuration");//should never here, handle as SENINF1_CSI0

        CalSel = 0;
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
#if 0
    if((mipi_type == CSI2_1_5G) || (mipi_type == CSI2_2_5G))// For CSI2(2.5G) support  & CPHY Support
    {
        SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = 0; //fix for non-mipi sensor 1 ->test mode, 3 parallel sesnor
    }
#endif


    /*GPIO config & MIPI RX config, CSI2 enable*/
    if(enable == 1) {
        // enable mipi lane
        temp = *(mpCSI2RxAnalog0RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
        mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog0RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
        mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog0RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//clock lane input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//data lane 0 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//data lane 1 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0c/4));//data lane 2 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//data lane 3 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x10/4));

        // CSI Power On Timing
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_FAST_CHARGE
        mt65xx_reg_writel(temp|0x00000010, mpCSI2RxAnalog0RegAddr + (0x24/4));
        usleep(30);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_FAST_CHARGE
        mt65xx_reg_writel(temp&(~0x00000010), mpCSI2RxAnalog0RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x20/4));//bit0:RG_CSI0_LDO_CORE_EN,bit1:RG_CSI0_LNRD_HSRX_BCLK_INVERT
        mt65xx_reg_writel(temp|0x00000003, mpCSI2RxAnalog0RegAddr + (0x20/4));
        usleep(1);
        temp = *(mpCSI2RxAnalog0RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x10/4));
        //CSI efuse calibration
        //0x10206190 [7:4]gJ0x10217004[15:12]  (CSI0 lane0)
        //0x10206190 [3:0]gJ0x10217008[15:12]  (CSI0 lane1)
        //0x1020618C [31:28]gJ0x1021700C[15:12]  (CSI0 lane2)
        //0x1020618C [27:24]gJ0x10217010[15:12]  (CSI0 lane3)
        //0x1020618C [23:20]gJ0x10217000[15:12]  (CSI0 laneC)

        //0x1020618C [19:16]gJ0x10218004[15:12]  (CSI1 lane0)
        //0x1020618C [15:12]gJ0x10218008[15:12]  (CSI1 lane1)
        //0x1020618C [11:8]gJ0x1021800C[15:12]  (CSI1 lane2)
        //0x1020618C [7:4]gJ0x10218010[15:12]  (CSI1 lane3)
        //0x1020618C [3:0]gJ0x10218000[15:12]  (CSI1 laneC)

#ifdef CSI2_EFUSE_SET
        /*Read Efuse value : termination control registers*/
        Efuse(CalSel);
#endif

        //CSI Offset calibration
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 1;

        // CSI2 offset calibration
        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 1; // 0x15008338[0]= 1'b1;
        temp = SENINF_READ_REG(pSeninf, MIPI_RX_CON3C_CSI0);
        SENINF_WRITE_REG(pSeninf, MIPI_RX_CON3C_CSI0, 0x1541); // 0x1500833C[31:0]=32'h1541;

        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_HW_CAL_START) = 1;//MIPI_RX_HW_CAL_START

        LOG_MSG("CSI-0 calibration start,MIPIType=%d,HSRXDE=1",(int)mipi_type);

        usleep(500);
        if(!(( SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI0)& 0x10001) && (SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI0) & 0x101))){
            LOG_ERR("CSI-0 calibration failed!, NCSI2Config Reg 0x44=0x%x, 0x48=0x%x",SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI0),SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI0));
            //ret = -1;
        }
        // disable SW control mode
        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 0; // 0x15008338[0]= 1'b0;

        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 0;

        LOG_MSG("CSI-0 calibration end !");

        msettleDelay = SETTLE_DELAY;//fix count


        LOG_MSG("TermDelay:%d SettleDelay:%d ClkTermDelay:%d Vsync:%d lane_num:%d NCSI2_en:%d HeaderOrder:%d SettleDelay(cnt):%d ISP_clk:%d dpcm:%d",
                (int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlaneNum,
                (int) enable, (int)dataheaderOrder, (int)msettleDelay, (int)Isp_clk,(int)dpcm);

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
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_DPCM, temp);//CSI2 must be slected when sensor clk output

        if(mipi_type == 1) // For CSI2
        {
            //0x8100
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, temp&0xFFFF0FFF);//CSI2 must be slected when sensor clk output
            //0x8364
            temp = (msettleDelay&0xFF)<<16;
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DELAY, temp);
            //0x8360
            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL,(temp|(1<<5)|(enable<<4)|(((1<<(dlaneNum+1))-1)))) ;
            //0x8368
            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTEN);
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTEN,temp|0x000F);//turn on all interrupt
        }
        else // for NCSI2
        {
            LOG_MSG("Setting for NCSI2");

            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,((temp&0xFFFF0FFF)|0x8000));//nCSI2 must be slected when sensor clk output
            //temp = SENINF_READ_REG(pSeninf, SENINF1_MUX_CTRL);

            //temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFEEF); // disable CSI2 first & disable HSRX_DET_EN (use settle delay setting instead auto)
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFE7F); // disable CSI2 first & disable HSRX_DET_EN (use settle delay setting instead auto)

            // Force manual mode
            LOG_MSG("**disable HSRX_DET_EN");
            // disable HSRX_DET_EN  bit7:HSRX_DET_EN
            temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&(~0x00000080));

            temp = (msettleDelay&0xFF)<<8;  //Fixed@145ns/4ns(ISP clock)
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, temp);
            temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL,(temp|(1<<26)|(dataheaderOrder<<16)|(enable<<4)|(((1<<(dlaneNum+1))-1)))) ;
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_INT_EN,0x80007FFF);//turn on all interrupt
        }

        //turn on virtual channel
        //temp = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DI_CTRL);
        //SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DI_CTRL,temp|0x1);
        // temp solution : force enable clk lane / data lane
        //temp = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_HSRX_DBG);
        //SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_HSRX_DBG,temp|0x1f);

        // Workaround to trigger
        LOG_MSG("Workaround to trigger");
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_HSRX_DBG, 0x10) ;

    }
    else {
        // disable NCSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2 first
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFE0); // disable CSI2 first

        //disable mipi BG
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalog0RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x10/4));

        // disable mipi pin
        temp = *(mpCSI2RxAnalog0RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//clock lane input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//data lane 0 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//data lane 1 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//data lane 2 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//data lane 3 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x10/4));
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
            SENINF_BITS(pSeninf, SENINF1_NCSI2_CTL, HSRX_DET_EN) = 0;
            SENINF_BITS(pSeninf, SENINF1_NCSI2_CTL, CLOCK_HS_OPTION) = 1;
            //for Cphy only Digital sync
            SENINF_BITS(pSeninf, SENINF1_NCSI2_CTL, ASYNC_FIFO_RST_SCH) = 2;
            SENINF_BITS(pSeninf, SENINF1_NCSI2_CTL, SYNC_DET_EN) = 1;
            SENINF_BITS(pSeninf, SENINF1_NCSI2_CTL, REF_SYNC_DET_EN) = 0;

            temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL,(temp|(dataheaderOrder<<16))) ;
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
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_INT_STATUS,0x9FFFBFFF);
            //LOG_MSG("SENINF1_NCSI2_CTL(0x1a04_0a00 = 0x%x",SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL));
        }
    }
    else {
        // disable NCSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(1.5G) first
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2(2.5G) first
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
    unsigned int DDR_EN, TG_FORMAT_ENUM inDataType
)
{
    int ret = 0;

#if 0
    (void)scamEn;
    (void)clkInv;
    (void)width;
    (void)height;
    (void)contiMode;
    (void)csdNum;
    (void)DDR_EN;
#else
    unsigned int *mpCSI2RxAnalog1RegAddr = mpCSI2RxAnaBaseAddr[SENINF2_CSI1];

    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    IMGSENSOR_GPIO_STRUCT mpgpio;
    int data_channel;
    LOG_MSG("Scam clkInv(%d),width(%d),height(%d),csdNum(%d),DDR_EN(%d)",clkInv,width,height,csdNum+1,DDR_EN);

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

        //temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        //mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x4C/4));
        //temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        //mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x50/4));
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

        //temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
        //mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x4C/4));
        //temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
        //mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x50/4));
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

        /*Reset for enable scam*/
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x10000);
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00);

        /*Sensor Interface control select to Serial camera*/
        temp = SENINF_READ_REG(pSeninf, SENINF4_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF4_CTRL,((temp&0xFFFF0FFF)|0x4000));

        /*SCAM control */
        temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);
        temp &= 0x10000000;
        if (inDataType == JPEG_FMT) {
            // bit[30:29] remind b'00
            LOG_MSG("inDataType = JPEG_FMT");
        } else if (inDataType == RAW_8BIT_FMT) {
            // bit[30:29] set to b'01
            temp |= 0x20000000;
            LOG_MSG("inDataType = RAW_8BIT_FMT");
        } else if (inDataType == YUV422_FMT) {
            // bit[30:29] set to b'10
            temp |= 0x40000000;
            LOG_MSG("inDataType = YUV422_FMT");
        } else if (inDataType == RGB565_MIPI_FMT) {
            // bit[30:29] set to b'10
            temp |= 0x40000000;
            LOG_MSG("inDataType = RGB565_MIPI_FMT");
        } else {
            // set to default
            temp |= 0x40000000;
            LOG_MSG("Unsupported inDataType = %d", inDataType);
        }
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
        }

    }
    else
    {
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00);
    }

    temp = SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL);
    SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp|0x3);//reset
    SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp&0xFFFFFFFC);//clear reset

    LOG_MSG("SCAM1_CFG(0x%x), SCAM1_CON(0x%x)",
            SENINF_READ_REG(pSeninf,SCAM1_CFG), SENINF_READ_REG(pSeninf,SCAM1_CON));
    LOG_MSG("SCAM1_INT(0x%x), SCAM1_SIZE(0x%x)",
            SENINF_READ_REG(pSeninf,SCAM1_INT), SENINF_READ_REG(pSeninf,SCAM1_SIZE));
#endif
    return ret;

}


int SeninfDrvImp::setSeninf4Parallel(unsigned int parallelEn,unsigned int inDataType)
{
    int ret = 0;
#if 0
    (void)parallelEn;
    (void)inDataType;
#else
    unsigned int *mpCSI2RxAnalog1RegAddr = mpCSI2RxAnaBaseAddr[SENINF2_CSI1];
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

            //temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
            //mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x4C/4));
            //temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
            //mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x50/4));

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

         //temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         //mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x4C/4));
         //temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         //mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x50/4));

    }
#endif
    return ret;
}

int SeninfDrvImp::setAllMclkOnOff(unsigned long ioDrivingCurrent, bool enable){
    if(enable) {
        setMclkIODrivingCurrent(eMclk_1, ioDrivingCurrent);
        setMclkIODrivingCurrent(eMclk_2, ioDrivingCurrent);
        //setMclkIODrivingCurrent(eMclk_3, ioDrivingCurrent);
        setMclk(eMclk_1, 1, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_2, 1, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        //setMclk(eMclk_3, 1, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
    } else {
        setMclk(eMclk_1, 0, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        setMclk(eMclk_2, 0, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
        //setMclk(eMclk_3, 0, MCLK_24MHZ, 1, 0, 1, 0, 0, 0);
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
    if (inputCurrent<4)
        ioDrivingCurrent = inputCurrent;

    pCAMIODrvRegAddr = mpGpioHwRegAddr + (0xA00 / 4);

    if(pCAMIODrvRegAddr != NULL) {
        *(pCAMIODrvRegAddr) &= (~(0x3<<0));
        *(pCAMIODrvRegAddr) |= (ioDrivingCurrent<<0); // [1:0] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
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
    if (inputCurrent<4)
        ioDrivingCurrent = inputCurrent;

    pCAMIODrvRegAddr = mpGpioHwRegAddr + (0xC00 / 4);

    if(pCAMIODrvRegAddr != NULL) {
        *(pCAMIODrvRegAddr) &= (~(0x3<<22));
        *(pCAMIODrvRegAddr) |= (ioDrivingCurrent<<22); // [23:22] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
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

    pCAMIODrvRegAddr = mpGpioHwRegAddr + (0xA00 / 4);

    if(pCAMIODrvRegAddr != NULL) {
        *(pCAMIODrvRegAddr) &= (~(0x7<<24));
        *(pCAMIODrvRegAddr) |= (ioDrivingCurrent<<24); // [23:21] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
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
    // seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    (void) pCnt;
    // *pCnt = SENINF_READ_REG(pSeninf,SENINF_N3D_A_DIFF_CNT);
    // *(pCnt+1) = SENINF_READ_REG(pSeninf,SENINF_N3D_A_CNT0);
    // *(pCnt+2) = SENINF_READ_REG(pSeninf,SENINF_N3D_A_CNT1);
    // *(pCnt+3) = SENINF_READ_REG(pSeninf,SENINF_N3D_A_DBG);

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
int SeninfDrvImp::autoDeskew_SW(EMipiPort csi_sel, unsigned int lane_num){

	int ret =0 ;
    //SENINF_CSI_PARA csi_para;
    (void)lane_num;
    (void) csi_sel;

 //   getCSIpara(csi_sel, &csi_para);
    LOG_MSG("autoDeskew_SW start");
    return ret;

}

void SeninfDrvImp::getSeninfMux(SENINF_CONFIG_STRUCT *pseninfPara, SENINF_TOP_P1_ENUM *pSENINF_TOP_TG_selected,
        SENINF_MUX_ENUM *pSENINF_MUX_selected, unsigned long *pTG_SEL) {

    Mutex::Autolock lock(mLock_mmap);

    if(isDualCamMode() && pseninfPara->sensor_idx == IMGSENSOR_SENSOR_IDX_MAIN2
            && (mSeninfMuxUsed[SENINF_MUX3] == -1
                || mSeninfMuxUsed[SENINF_MUX3] == pseninfPara->sensor_idx)) {
        // to camsv1
        mSeninfMuxUsed[SENINF_MUX3] = pseninfPara->sensor_idx;
        *pSENINF_TOP_TG_selected = SENINF_TOP_SV1;
        *pSENINF_MUX_selected = SENINF_MUX3;
        *pTG_SEL = CAM_SV_1;
    } else if(mSeninfMuxUsed[SENINF_MUX1] == -1 || mSeninfMuxUsed[SENINF_MUX1] == pseninfPara->sensor_idx) {
        // to tg1
        mSeninfMuxUsed[SENINF_MUX1] = pseninfPara->sensor_idx;
        *pSENINF_TOP_TG_selected = SENINF_TOP_TG1;
        *pSENINF_MUX_selected = SENINF_MUX1;
        *pTG_SEL = CAM_TG_1;
    } else {
        LOG_ERR("no availabe mux mSeninfMuxUsed[SENINF_MUX1] = %d, mSeninfMuxUsed[SENINF_MUX2] = %d",
                mSeninfMuxUsed[SENINF_MUX1], mSeninfMuxUsed[SENINF_MUX2]);
        mSeninfMuxUsed[SENINF_MUX1] = pseninfPara->sensor_idx;
        *pSENINF_TOP_TG_selected = SENINF_TOP_TG1;
        *pSENINF_MUX_selected = SENINF_MUX1;
        *pTG_SEL = CAM_TG_1;
    }
}

int SeninfDrvImp::configTg(SENINF_TOP_P1_ENUM selected, SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo) {
    int ret = 0;

    if (selected == SENINF_TOP_SV1) {
        ret = this->setSVGrabRange(selected, pseninfPara->u4PixelX0,
                pseninfPara->u4PixelX1, pseninfPara->u4PixelY0, pseninfPara->u4PixelY1);
        ret = this->setSVCfg(selected, (TG_FORMAT_ENUM)pseninfPara->inDataType,
                (SENSOR_DATA_BITS_ENUM)pseninfPara->senInLsb, psensorDynamicInfo->pixelMode,
                pseninfPara->inSrcTypeSel == SERIAL_SENSOR ? (pseninfPara->scam_para).SCAM_SOF_src : 0);
        ret = this->setSVViewFinderMode(selected, pseninfPara->u1IsContinuous? 0 : 1 );
    } else {
        ret = this->setTgGrabRange(selected, pseninfPara->u4PixelX0,
                pseninfPara->u4PixelX1, pseninfPara->u4PixelY0, pseninfPara->u4PixelY1);
        ret = this->setTgViewFinderMode(selected, pseninfPara->u1IsContinuous ? 0 : 1);
        ret = this->setTgCfg(selected, (TG_FORMAT_ENUM)pseninfPara->inDataType,
                (SENSOR_DATA_BITS_ENUM)pseninfPara->senInLsb, psensorDynamicInfo->pixelMode,
                pseninfPara->inSrcTypeSel == SERIAL_SENSOR ? (pseninfPara->scam_para).SCAM_SOF_src : 0);
    }

    return ret;
}

int SeninfDrvImp::configInputForCSI(SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo, bool enable) {
    int ret = 0;
    SENINF_TOP_P1_ENUM SENINF_TOP_TG_selected = SENINF_TOP_TG1;
    SENINF_MUX_ENUM SENINF_MUX_selected = SENINF_MUX1;
    unsigned long TG_SEL = CAM_TG_NONE;
    SENSOR_VC_INFO_STRUCT *pVcInfo = NULL;

    this->getSeninfMux(pseninfPara, &SENINF_TOP_TG_selected, &SENINF_MUX_selected, &TG_SEL);

    if(enable) {
        ret = this->configTg(SENINF_TOP_TG_selected, pseninfPara, psensorDynamicInfo);

        ret = this->setSeninfMuxCtrl(SENINF_MUX_selected, 0, 0,
                         MIPI_SENSOR, pseninfPara->inDataType, psensorDynamicInfo->pixelMode);
        ret = this->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, pseninfPara->csi_para.seninfSrc);
        psensorDynamicInfo->TgInfo = TG_SEL;

        if(pseninfPara->HDRMode == HDR_CAMSV){
            psensorDynamicInfo->HDRPixelMode = HALSENSOR_CAMSV_PIXEL_MODE;
            pVcInfo = &(pseninfPara->csi_para.vcInfo);
            ret = this->setSeninfMuxCtrl(SENINF_MUX3, 0, 0,
                                            VIRTUAL_CHANNEL_1, pseninfPara->inDataType, psensorDynamicInfo->HDRPixelMode);

            // CAM SV1 Tg Grab
            ret = this->setSVGrabRange(SENINF_TOP_SV1,
                    0x00, convertByteToPixel(pVcInfo->VC1_DataType, pVcInfo->VC1_SIZEH),
                    0x00, pVcInfo->VC1_SIZEV);
            ret = this->setSVCfg(SENINF_TOP_SV1, (TG_FORMAT_ENUM)pseninfPara->inDataType,
                    (SENSOR_DATA_BITS_ENUM)pseninfPara->senInLsb, psensorDynamicInfo->HDRPixelMode, 0);
            ret = this->setSVViewFinderMode(SENINF_TOP_SV1, pseninfPara->u1IsContinuous? 0 : 1 );

            LOG_MSG("HDR VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                    pVcInfo->VC0_ID,pVcInfo->VC0_DataType,pVcInfo->VC1_ID,pVcInfo->VC1_DataType,
                    pVcInfo->VC2_ID,pVcInfo->VC2_DataType,pVcInfo->VC3_ID,pVcInfo->VC3_DataType);
            ret = this->setSeninfVC(pseninfPara->csi_para.seninfSrc,
                                    (pVcInfo->VC0_DataType<<2)|(pVcInfo->VC0_ID&0x03),
                                    (pVcInfo->VC1_DataType<<2)|(pVcInfo->VC1_ID&0x03),
                                    (pVcInfo->VC2_DataType<<2)|(pVcInfo->VC2_ID&0x03),
                                    (pVcInfo->VC3_DataType<<2)|(pVcInfo->VC3_ID&0x03), 0, 0);

            ret = this->setSeninfTopMuxCtrl(SENINF_TOP_SV1, pseninfPara->csi_para.seninfSrc);
            psensorDynamicInfo->HDRInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_1 :CAM_SV_3);
        }

        if (pseninfPara->PDAFMode == PDAF_SUPPORT_CAMSV ||
            pseninfPara->PDAFMode == PDAF_SUPPORT_CAMSV_LEGACY ||
            pseninfPara->PDAFMode == PDAF_SUPPORT_CAMSV_DUALPD ){
            psensorDynamicInfo->PDAFPixelMode= HALSENSOR_CAMSV_PIXEL_MODE;
            pVcInfo = &(pseninfPara->csi_para.vcInfo);

           //seninf muxer4 for  PDAF using virtual channel 2
           ret = this->setSeninfMuxCtrl(SENINF_MUX4, 0, 0,
                                               VIRTUAL_CHANNEL_2, pseninfPara->inDataType, psensorDynamicInfo->PDAFPixelMode);

           // CAM SV2 Tg Grab
           ret = this->setSVGrabRange(SENINF_TOP_SV2,
                   0x00, convertByteToPixel(pVcInfo->VC2_DataType, pVcInfo->VC2_SIZEH),
                   0x00, pVcInfo->VC2_SIZEV);
           ret = this->setSVCfg(SENINF_TOP_SV2, (TG_FORMAT_ENUM)pseninfPara->inDataType,
                   (SENSOR_DATA_BITS_ENUM)pseninfPara->senInLsb, psensorDynamicInfo->PDAFPixelMode, 0);
           ret = this->setSVViewFinderMode(SENINF_TOP_SV2, pseninfPara->u1IsContinuous? 0 : 1 );

           // Sensor interface for VC
           LOG_MSG("PDAF VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                    pVcInfo->VC0_ID, pVcInfo->VC0_DataType, pVcInfo->VC1_ID, pVcInfo->VC1_DataType,
                    pVcInfo->VC2_ID, pVcInfo->VC2_DataType, pVcInfo->VC3_ID, pVcInfo->VC3_DataType);
            ret = this->setSeninfVC(pseninfPara->csi_para.seninfSrc,
                                    (pVcInfo->VC0_DataType<<2)|(pVcInfo->VC0_ID&0x03),
                                    (pVcInfo->VC1_DataType<<2)|(pVcInfo->VC1_ID&0x03),
                                    (pVcInfo->VC2_DataType<<2)|(pVcInfo->VC2_ID&0x03),
                                    (pVcInfo->VC3_DataType<<2)|(pVcInfo->VC3_ID&0x03), 0, 0);

           ret = this->setSeninfTopMuxCtrl(SENINF_TOP_SV2, pseninfPara->csi_para.seninfSrc);
           psensorDynamicInfo->PDAFInfo = (TG_SEL == CAM_TG_1 ?CAM_SV_2 :CAM_SV_4);
        }
    }

    LOG_MSG("sensor index = %d, psensorDynamicInfo->TgInfo = %d; mSeninfMuxUsed[SENINF_MUX1] =%d, mSeninfMuxUsed[SENINF_MUX2] = %d",
                pseninfPara->sensor_idx, psensorDynamicInfo->TgInfo, mSeninfMuxUsed[SENINF_MUX1], mSeninfMuxUsed[SENINF_MUX2]);
    ret = setSeninfCSI2(pseninfPara,  enable);
    return ret;

}

int SeninfDrvImp::configInputForSerial(SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo, bool enable) {
    int ret = 0;
    SENINF_TOP_P1_ENUM SENINF_TOP_TG_selected = SENINF_TOP_TG1;
    SENINF_MUX_ENUM SENINF_MUX_selected = SENINF_MUX1;
    unsigned long TG_SEL = CAM_TG_NONE;

    this->getSeninfMux(pseninfPara, &SENINF_TOP_TG_selected, &SENINF_MUX_selected, &TG_SEL);

    if(enable) {
        ret = this->configTg(SENINF_TOP_TG_selected, pseninfPara, psensorDynamicInfo);

        ret = this->setSeninfMuxCtrl(SENINF_MUX_selected, 0, 0,
                         SERIAL_SENSOR, pseninfPara->inDataType, psensorDynamicInfo->pixelMode);
        ret = this->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, SENINF_4);
        psensorDynamicInfo->TgInfo = TG_SEL;

        ret = this->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)pseninfPara->padSel, (SENINF_SOURCE_ENUM)pseninfPara->inSrcTypeSel);
    }

    ret = this->setSeninf4Scam(enable, (pseninfPara->scam_para).SCAM_CLK_INV,
            pseninfPara->cropWidth, pseninfPara->cropHeight, 1, (pseninfPara->scam_para).SCAM_DataNumber,
            (pseninfPara->scam_para).SCAM_DDR_En, (TG_FORMAT_ENUM)pseninfPara->inDataType);

    LOG_MSG("sensor index = %d, psensorDynamicInfo->TgInfo = %d; mSeninfMuxUsed[SENINF_MUX1] =%d, mSeninfMuxUsed[SENINF_MUX2] = %d",
                pseninfPara->sensor_idx, psensorDynamicInfo->TgInfo, mSeninfMuxUsed[SENINF_MUX1], mSeninfMuxUsed[SENINF_MUX2]);
    return ret;
}

int SeninfDrvImp::configInputForParallel(SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo, bool enable) {
    int ret = 0;
    SENINF_TOP_P1_ENUM SENINF_TOP_TG_selected = SENINF_TOP_TG1;
    SENINF_MUX_ENUM SENINF_MUX_selected = SENINF_MUX1;
    unsigned long TG_SEL = CAM_TG_NONE;

    this->getSeninfMux(pseninfPara, &SENINF_TOP_TG_selected, &SENINF_MUX_selected, &TG_SEL);

    if(enable) {

        ret = this->configTg(SENINF_TOP_TG_selected, pseninfPara, psensorDynamicInfo);

        ret = this->setSeninfMuxCtrl(SENINF_MUX_selected,
                         pseninfPara->parallel_para.u1HsyncPol ? 0 : 1,
                         pseninfPara->parallel_para.u1VsyncPol ? 0 : 1,
                         PARALLEL_SENSOR, pseninfPara->inDataType, psensorDynamicInfo->pixelMode);
        ret = this->setSeninfTopMuxCtrl(SENINF_TOP_TG_selected, SENINF_4);
        psensorDynamicInfo->TgInfo = TG_SEL;

        ret = this->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)pseninfPara->padSel, (SENINF_SOURCE_ENUM)pseninfPara->inSrcTypeSel);
    }

    ret = this->setSeninf4Parallel(enable, pseninfPara->inDataType);

    LOG_MSG("sensor index = %d, psensorDynamicInfo->TgInfo = %d; mSeninfMuxUsed[SENINF_MUX1] =%d, mSeninfMuxUsed[SENINF_MUX2] = %d",
                pseninfPara->sensor_idx, psensorDynamicInfo->TgInfo, mSeninfMuxUsed[SENINF_MUX1], mSeninfMuxUsed[SENINF_MUX2]);
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

    /* Target: 2*(11+1)*(1/24M) = 1Mhz */
    psensorDynamicInfo->TgCLKInfo = 1000;
    switch(pseninfPara->inSrcTypeSel) {
        case MIPI_SENSOR:
            getCSIpara(&(pseninfPara->csi_para));
            configInputForCSI(pseninfPara, psensorDynamicInfo, enable);
            break;
        case SERIAL_SENSOR:
            configInputForSerial(pseninfPara, psensorDynamicInfo, enable);
            break;
        case PARALLEL_SENSOR:
            configInputForParallel(pseninfPara, psensorDynamicInfo, enable);
            break;
        default:
            LOG_MSG("unsupported inSrcTypeSel %d", pseninfPara->inSrcTypeSel);
            break;
    }
    return 0;
}
int SeninfDrvImp::calibrateSensor(SENINF_CONFIG_STRUCT *pseninfPara){
    switch(pseninfPara->inSrcTypeSel) {
        case MIPI_SENSOR:
            calibrateMipiSensor(pseninfPara);
            break;
        case SERIAL_SENSOR:
            calibrateSerialSensor(pseninfPara);
            break;
        case PARALLEL_SENSOR:
            break;
        default:
            LOG_MSG("unsupported inSrcTypeSel %d", pseninfPara->inSrcTypeSel);
            break;
    }
    return 0;
}

int SeninfDrvImp::calibrateMipiSensor(SENINF_CONFIG_STRUCT *pseninfPara)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MUINT64 csi_skew_treshold = CSI_DESKEW_THRESHOLD;

    property_get("vendor.debug.seninf.deskew.enable", value, "1");
    if(atoi(value) == 0)
        return 0;

    if(pseninfPara->csi_para.mipi_pixel_rate == 0) {
        int bpp = 10;
        MUINT64 pixel_rate = ((MUINT64)pseninfPara->cropWidth) * pseninfPara->cropHeight * (pseninfPara->frameRate / 10);
        LOG_MSG("pixel_rate %llu, threshold %llu, h %d, w %d, fps %d,pdaf mode %d",
                pixel_rate, csi_skew_treshold, pseninfPara->cropHeight, pseninfPara->cropWidth,
                pseninfPara->frameRate, pseninfPara->PDAFMode);
        switch (pseninfPara->inDataType){
            case RAW_8BIT_FMT:
                bpp = 8;
                break;
                break;
            case RAW_12BIT_FMT:
                bpp = 12;
                break;
            default:
                bpp =10;
                break;
        }
        pseninfPara->csi_para.mipi_pixel_rate = (pixel_rate * bpp *(100 + CSI_BLANKING_RATIO)) /
            ((pseninfPara->csi_para.dlaneNum + 1) * 100);
        pseninfPara->csi_para.mipi_pixel_rate *= pseninfPara->PDAFMode == PDAF_SUPPORT_RAW_DUALPD ? 2 : 1;
    }

    LOG_MSG("mipi_pixel_rate %llu CSI_DESKEW_THRESHOLD %llu",
            pseninfPara->csi_para.mipi_pixel_rate, csi_skew_treshold);

    if(pseninfPara->csi_para.mipi_pixel_rate > csi_skew_treshold){
        return csi_deskew_sw(pseninfPara);
    }

    return 0;
}

int SeninfDrvImp::resetConfiguration(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    (void) sensorIdx;

    return 0;
}

int SeninfDrvImp::csi_deskew_sw(SENINF_CONFIG_STRUCT *pseninfPara)
{
    int ret = 0;
    unsigned int temp = 0;
    MUINT32 min_lane_code=0;
    MUINT8 lane0_code=0,lane1_code=0,lane2_code=0,lane3_code=0,i=0;
    MUINT8 clk_code=0;
    MUINT32 currPacket = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *) mpSeninfCtrlRegAddr[pseninfPara->csi_para.seninfSrc];
    unsigned int *mpCSI2RxAnalog0RegAddr = mpCSI2RxAnaBaseAddr[pseninfPara->csi_para.seninfSrc];;

    LOG_MSG("autoDeskewCalibration start");

    temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//disable clock lane delay
    mt65xx_reg_writel(temp&0xFFF7FFFF, mpCSI2RxAnalog0RegAddr + (0x00));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//disable data lane 2 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//disable data lane 3 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));


    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DGB_SEL,0x12);//set debug port to output packet number
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_EN,0x80007FFF);//set interrupt enable //@write clear?

    //@add check default if any interrup error exist, if yes, debug and fix it first. if no, continue calibration
    //@add print ecc & crc error status
    if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0){
        LOG_ERR("autoDeskewCalibration Line %d, default input has error, please check it",__LINE__);
    }
    LOG_MSG("autoDeskewCalibration start interupt status = 0x%x",SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS));
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);

    //Fix Clock lane
    //set lane 0
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//enable data lane 0 delay
    mt65xx_reg_writel(temp|0x00800000, mpCSI2RxAnalog0RegAddr + (0x04/4));

    for(i=0; i<=0xF; i++) {
        lane0_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        //@add read interrupt status to clear
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x04/4));
        //usleep(5);
         while(currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT)){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
    usleep(5);

    //set lane 1
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//enable data lane 1 delay
    mt65xx_reg_writel(temp|0x00800000, mpCSI2RxAnalog0RegAddr + (0x08/4));

    for(i=0; i<=0xF; i++) {
        lane1_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x08/4));
         //usleep(5);
         while(currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT)){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
    usleep(5);

    //set lane 2
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//enable data lane 2 delay
    mt65xx_reg_writel(temp|0x00800000, mpCSI2RxAnalog0RegAddr + (0x0C/4));

    for(i=0; i<=0xF; i++) {
        lane2_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x0C/4));
         //usleep(5);
         while(currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT)){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//disable data lane 2 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
    usleep(5);

    //set lane 3
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//enable data lane 3 delay
    mt65xx_reg_writel(temp|0x00800000, mpCSI2RxAnalog0RegAddr + (0x10/4));

    for(i=0; i<=0xF; i++) {
        lane3_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x10/4));
         //usleep(5);
         while(currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT)){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//disable data lane 3 delay
    mt65xx_reg_writel(temp&0xFF7FFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));

    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d",lane0_code,lane1_code,lane2_code,lane3_code);

    //find minimum data lane code
    min_lane_code = lane0_code;
    if(min_lane_code > lane1_code) {
        min_lane_code = lane1_code;
    }
    if(min_lane_code > lane2_code) {
        min_lane_code = lane2_code;
    }
    if(min_lane_code > lane3_code) {
        min_lane_code = lane3_code;
    }
    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d, minimum = %d",lane0_code,lane1_code,lane2_code,lane3_code,min_lane_code);


    //Fix Data lane
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//set to 0 first
    mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//set to 0 first
    mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//set to 0 first
    mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//set to 0 first
    mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));
    temp = *(mpCSI2RxAnalog0RegAddr);//enable clock lane delay
    mt65xx_reg_writel(temp|0x00080000, mpCSI2RxAnalog0RegAddr );

    LOG_MSG("autoDeskewCalibration start test 5");

    for(i=0; i<=0xF; i++) {
        clk_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFF0FFFFF, mpCSI2RxAnalog0RegAddr );
        temp = *(mpCSI2RxAnalog0RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<20), mpCSI2RxAnalog0RegAddr );
        while(currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT)){};
         //usleep(5);
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    LOG_MSG("autoDeskewCalibration clk_code = %d",clk_code);

    if(clk_code < min_lane_code) {
        lane0_code = lane0_code -((min_lane_code+clk_code)>>1);
        lane1_code = lane1_code -((min_lane_code+clk_code)>>1);
        lane2_code = lane2_code -((min_lane_code+clk_code)>>1);
        lane3_code = lane3_code -((min_lane_code+clk_code)>>1);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//set to 0 first
        mt65xx_reg_writel(temp&0xF0FFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<24), mpCSI2RxAnalog0RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog0RegAddr);//clk code = 0
        mt65xx_reg_writel(temp&0xFF0FFFFF, mpCSI2RxAnalog0RegAddr );
    }
    else {
        //data code keeps at DC[n]-min(DC[n])
        clk_code = (clk_code - min_lane_code)>>1;
        temp = *(mpCSI2RxAnalog0RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFF0FFFFF, mpCSI2RxAnalog0RegAddr );
        temp = *(mpCSI2RxAnalog0RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<20), mpCSI2RxAnalog0RegAddr );
    }
    LOG_MSG("autoDeskewCalibration clk_code = %d, min_lane_code = %d",clk_code,min_lane_code);



    LOG_MSG("autoDeskewCalibration end");
    return ret;
}

int SeninfDrvImp::calibrateSerialSensor(SENINF_CONFIG_STRUCT *pseninfPara)
{
    unsigned int temp = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int DDR_EN = (pseninfPara->scam_para).SCAM_DDR_En;
    unsigned int default_delay = (pseninfPara->scam_para).SCAM_DEFAULT_DELAY;
    unsigned int timeout_cali = (pseninfPara->scam_para).SCAM_Timout_Cali;
    unsigned int csdNum = (pseninfPara->scam_para).SCAM_DataNumber;
    unsigned int crcEn = (pseninfPara->scam_para).SCAM_CRC_En;

    if(DDR_EN == 1) {
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

            temp = temp|1<<21|1<<22|1<<7|1<<1;//crc_on,calibration mode,int7,int1
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
                LOG_MSG("cant find delay code in both side");
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

    return 0;
}

int SeninfDrvImp::autoDeskewCalibrationSeninf(EMipiPort csi_sel, unsigned int lane_num, bool is_sw_deskew )
{
    int ret = 0;
    if (is_sw_deskew) {
        ret = autoDeskew_SW(csi_sel, lane_num);
    } else {
        LOG_ERR("HW deksew was not supporeted");
        ret = -1;
    }

#if 0
    unsigned int temp = 0;
    unsigned int min_lane_code=0;
    unsigned char lane0_code=0,lane1_code=0,lane2_code=0,lane3_code=0,i=0,j=0;
    unsigned char clk_code=0;
    unsigned int currPacket = 0;

    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("autoDeskewCalibration start");

    if(csi_sel == 1) //Use sensor interface 2   & CSI1
    {
        pSeninf = (seninf_reg_t *)(mpSeninf2HwRegAddr);
    }
    else // Use sensor interface 1  & CSI0
    {
        pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    }

    //CSI2 offset calibration
    /*Disable "LNRD0_HSRX_DELAY_EN" for D0/D1/D2/D3/C0*/
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 0;

    /*Enable sw mode*/
    SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, Delay_APPLY_MODE) = 2;/*RG mode*/
    SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, DESKEW_SW_RST) = 0;/* SW Reset*/


    /* set debug port to output packet number */
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DGB_SEL,0x8000001A);
    /* set interrupt enable, @write clear? */
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN,0x1FFFBFFF);
    SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_INT_STATUS,0x1FFFBFFF);
    usleep(1000);
    //@add check default if any interrup error exist, if yes, debug and fix it first. if no, continue calibration
    //@add print ecc & crc error status
    if((SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0){
        LOG_ERR("autoDeskewCalibration Line , default input has error, please check it");
    }
   // temp = 0;
    //while(*(volatile kal_uint32 *)(0x1a041a14)!=0x9004)
    //{
    //   temp++;
    //}
    //for(i=0; i < 0xfF; i++)
    //{
    //LOG_MSG("IRQ = 0x%x",SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
    //}
    //set interrupt enable, @write clear?
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN,0x00007FFF);

    //set lane 0
    LOG_MSG("[step 1]D0");
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 1;
    /*Apply enable*/
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_CODE) = 0;/* set to 0 first */
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
    usleep(1);
    for(i=1; i < 0x3F; i++)
    {
        lane0_code = i;
        currPacket = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT);
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_CODE) = lane0_code;
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
        usleep(1);
        while(currPacket == SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT)){};/*Do until package number increase*/
        //SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS);
        //SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS);
        while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) == 0){};/*Need*/

        LOG_MSG("MIPI_RX_CON7C_CSI0 = 0x%x, IRQ = 0x%x",SENINF_READ_REG(pSeninf, MIPI_RX_CON7C_CSI0), temp);

        if(( temp&0xFB8)!= 0) {
           LOG_MSG("lane0_code = %d, IRQ = 0x%d",lane0_code,SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
           usleep(1000);
           //if(((SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS))&0xFB8)!= 0) {//double confirm error happen
               break;
           //}
        }

    }

    //SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_CODE) = 0;/* set to 0 first */
    //SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
    //SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 1;
    //SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
    //CTP_Wait_msec(100);

    lane0_code = lane0_code;
    /* disable data lane 0 delay */
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 0;
    /*Apply enable*/
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;

    while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) != 4){};/*Need*/


    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d",lane0_code,lane1_code,lane2_code,lane3_code);

     //set lane 1
     LOG_MSG("[step 2]D1");
     SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 1;
     /*Apply enable*/
     SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = 0;/* set to 0 first */
     SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
     SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
     SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
     usleep(1);
     for(i=1; i<=0x3F; i++)
     {
         lane1_code = i;
         currPacket = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT);
         SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = lane1_code;
         SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
         SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
         usleep(1);
         SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
         usleep(1);
         while(currPacket == SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT)){};/*Do until package number increase*/
         //SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS);
         //SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS);
         while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) == 0){};/*Need*/

         LOG_MSG("MIPI_RX_CON80_CSI0 = 0x%x, IRQ = 0x%x",SENINF_READ_REG(pSeninf, MIPI_RX_CON80_CSI0), temp);

        if(( temp&0xFB8)!= 0) {
         LOG_MSG("lane1_code = %d, IRQ = 0x%d",lane1_code,SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
         usleep(1000);
         //if(((SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS))&0xFB8)!= 0) {//double confirm error happen
             break;
         //}
        }


     }
     //SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = 0;/* set to 0 first */
     //SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
     //SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
     //SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
     //CTP_Wait_msec(10);

     lane1_code = lane1_code;

     /* disable data lane 0 delay */
     SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 0;
     /*Apply enable*/
     SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;

     while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) != 4){};/*Need*/

     LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d",lane0_code,lane1_code,lane2_code,lane3_code);

     //set lane 2
     LOG_MSG("[step 3]D2");
     SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 1;
     /*Apply enable*/
     SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_CODE) = 0;/* set to 0 first */
     SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
     SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 1;
     SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
     usleep(1);
     for(i=1; i<=0x3F; i++)
     {
         lane2_code = i;
         currPacket = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT);
         SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_CODE) = lane2_code;
         SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
         SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 1;
         usleep(1);
         SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
         usleep(1);
         while(currPacket == SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT)){};/*Do until package number increase*/
         //SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS);
         //SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS);
         while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) == 0){};/*Need*/
         LOG_MSG("MIPI_RX_CON84_CSI0 = 0x%x, IRQ = 0x%x",SENINF_READ_REG(pSeninf, MIPI_RX_CON84_CSI0), temp);

        if(( temp&0xFB8)!= 0) {
         LOG_MSG("lane2_code = %d, IRQ = 0x%d",lane2_code,SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
         usleep(1000);
         //if(((SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS))&0xFB8)!= 0) {//double confirm error happen
             break;
         //}
        }


     }

     //SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_CODE) = 0;/* set to 0 first */
     //SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
     //SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 1;
     //SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
     //CTP_Wait_msec(10);

     lane2_code = lane2_code;
     /* disable data lane 0 delay */
     SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 0;
     /*Apply enable*/
     SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;

     while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) != 4){};/*Need*/

     LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d",lane0_code,lane1_code,lane2_code,lane3_code);

     //set lane 3
     LOG_MSG("[step 4]D3");
     SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 1;
     /*Apply enable*/
     SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_CODE) = 0;/* set to 0 first */
     SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
     SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 1;
     SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
     usleep(1);
     for(i=1; i<=0x3F; i++)
     {
         lane3_code = i;
         currPacket = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT);
         SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_CODE) = lane3_code;
         SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
         SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 1;
         usleep(1);
         SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
         usleep(1);
         while(currPacket == SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT)){};/*Do until package number increase*/
         //LOG_MSG("IRQ = 0x%d",SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
         //LOG_MSG("IRQ = 0x%d",SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
         while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) == 0){};/*Need*/
         LOG_MSG("lane3_code = %d, MIPI_RX_CON88_CSI0 = 0x%x, IRQ = 0x%x",lane3_code,SENINF_READ_REG(pSeninf, MIPI_RX_CON88_CSI0), temp);


          if(( temp&0xFB8)!= 0) {
             LOG_MSG("lane3_code = %d, IRQ = 0x%d",lane3_code,SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
             usleep(1000);
             //if(((SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS))&0xFB8)!= 0) {//double confirm error happen
                 break;
             //}
          }


     }
     //SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_CODE) = 0;/* set to 0 first */
     //SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
     //SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 1;
     //SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
     //CTP_Wait_msec(10);

     lane3_code = lane3_code;

     /* disable data lane 0 delay */
     SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 0;
     /*Apply enable*/
     SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;

     while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) != 4){};/*Need*/

     LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d",lane0_code,lane1_code,lane2_code,lane3_code);


    //find minimum data lane code
    min_lane_code = lane0_code;
    if(min_lane_code > lane1_code) {
        min_lane_code = lane1_code;
    }
    if(min_lane_code > lane2_code) {
        min_lane_code = lane2_code;
    }
    if(min_lane_code > lane3_code) {
        min_lane_code = lane3_code;
    }
    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d, minimum = %d",lane0_code,lane1_code,lane2_code,lane3_code,min_lane_code);
    /*Enable "LNRD0_HSRX_DELAY_EN" for D0/D1/D2/D3/C0*/
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_EN) = 1;
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_EN) = 1;

    /*set delay code for L0*/
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_CODE) = (lane0_code-min_lane_code)&0x3F;
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
    /*set delay code for L1*/
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = (lane1_code-min_lane_code)&0x3F;
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
    /*set delay code for L2*/
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_CODE) = (lane2_code-min_lane_code)&0x3F;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
    /*set delay code for L3*/
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_CODE) =(lane3_code-min_lane_code)&0x3F;
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;

    while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) != 4){};/*Need*/


    LOG_MSG("[step 5]AutoDeskewCalibration start for Clk Lane");
    /*Enable clk lane*/
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_EN) = 1;
    usleep(1);
    /*Apply enable*/
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_CODE) = 0;/* set to 0 first */
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
    for(i=0; i<=0x3F; i++)
    {
        clk_code = i;
        currPacket = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT);
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_CODE) = clk_code;
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
        usleep(1);
        while(currPacket == SENINF_READ_REG(pSeninf, SENINF1_NCSI2_DBG_PORT)){};/*Do until package number increase*/

        while((temp=SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS)) == 0){};/*Need*/

         LOG_MSG("MIPI_RX_CONA0_CSI0 = 0x%x, IRQ = 0x%x",SENINF_READ_REG(pSeninf, MIPI_RX_CONA0_CSI0), temp);


          if(( temp&0xFB8)!= 0) {
             LOG_MSG("clk_code = %d, IRQ = 0x%d",clk_code,SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS));
             usleep(1000);
             //if(((SENINF_READ_REG(pSeninf, SENINF1_NCSI2_INT_STATUS))&0xFB8)!= 0) {//double confirm error happen
                 break;
             //}
          }
    }

    clk_code = clk_code;

    if(clk_code < min_lane_code) {
        lane0_code = lane0_code -((min_lane_code+clk_code)>>1);
        lane1_code = lane1_code -((min_lane_code+clk_code)>>1);
        lane2_code = lane2_code -((min_lane_code+clk_code)>>1);
        lane3_code = lane3_code -((min_lane_code+clk_code)>>1);
        clk_code = 0;

        /*set delay code for L0*/
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_CODE) = lane0_code&0x3F;
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CON8C_CSI0, RG_CSI0_LNRD0_HSRX_DELAY_APPLY) = 0;
        /*set delay code for L1*/
        SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_CODE) = lane1_code&0x3F;
        SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CON90_CSI0, RG_CSI0_LNRD1_HSRX_DELAY_APPLY) = 0;
        /*set delay code for L2*/
        SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_CODE) = lane2_code&0x3F;
        SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CON94_CSI0, RG_CSI0_LNRD2_HSRX_DELAY_APPLY) = 0;
        /*set delay code for L3*/
        SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_CODE) = lane3_code&0x3F;
        SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CON98_CSI0, RG_CSI0_LNRD3_HSRX_DELAY_APPLY) = 0;
        /*set delay code for Clock lane*/
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_CODE) = clk_code&0x3F;
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;

        LOG_MSG("Clk_code < min_lane_code");

    }
    else
    {
        //data code keeps at DC[n]-min(DC[n])
        clk_code = (clk_code - min_lane_code)>>1;
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_CODE) = clk_code&0x3F;
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 1;
        usleep(1);
        SENINF_BITS(pSeninf, MIPI_RX_CONA0_CSI0, RG_CSI0_LNRC_HSRX_DELAY_APPLY) = 0;
        LOG_MSG("Clk_code > min_lane_code");
    }
    LOG_MSG("data0 = %d, data1 = %d, data2 = %d, data3 = %d, minimum = %d, clk_code = %d",lane0_code,lane1_code,lane2_code,lane3_code,min_lane_code,clk_code);


    /*Set to default : HW mode*/
    //SENINF_BITS(pSeninf, MIPI_RX_CONB0_CSI0, Delay_APPLY_MODE) = 0;/*HW mode*/

    LOG_MSG("autoDeskewCalibration clk_code = %d, min_lane_code = %d",clk_code,min_lane_code);
    LOG_MSG("autoDeskewCalibration end");
#endif
    return ret;
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

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTgGrabRange(
    unsigned int tgSel, unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    int tg1GrabWidth = 0;
    int tg1GrabHeight = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegBaseAddr[tgSel];

    tg1GrabWidth = pixelEnd - pixelStart;
    tg1GrabHeight = lineEnd - lineStart;
    LOG_MSG("[setTgGrabRange](%u) TG grab width = 0x%x, TG grab height = 0x%x", tgSel, tg1GrabWidth, tg1GrabHeight);

    // TG Grab Win Setting
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_E) = pixelEnd;
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_S) = pixelStart;
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_E) = lineEnd;
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_S) = lineStart;

    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
int SeninfDrvImp::setTgViewFinderMode(
    unsigned int tgSel, unsigned long spMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegBaseAddr[tgSel];

    LOG_MSG("[setTgViewFinderMode](%u) sp mode = %lu", tgSel, spMode);

    ISP_BITS(pisp, CAM_TG_SEN_MODE, CMOS_EN) = 1;
    ISP_BITS(pisp, CAM_TG_SEN_MODE, SOT_MODE) = 1;

    ISP_BITS(pisp, CAM_TG_VF_CON, SPDELAY_MODE) = 1;
    ISP_BITS(pisp, CAM_TG_VF_CON, SINGLE_MODE) = spMode;
    //ISP_BITS(pisp, CAM_TG_VF_CON, SP_DELAY) = spDelay;

    return ret;
}

int SeninfDrvImp::setTgCfg(
    unsigned int tgSel, TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb,
    unsigned int twoPxlMode, unsigned int sof_Src
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegBaseAddr[tgSel];

    (void) senInLsb;

    LOG_MSG("[setTgCfg](%u) data type = %d, pixel mode = %u, sof_src = %u", tgSel, inDataType, twoPxlMode, sof_Src);

    if(1 == twoPxlMode) {
        ISP_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS) = 1;
        ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1, TWO_PIX) = 1;
    }
    else {
        ISP_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS) = 0;
        ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1, TWO_PIX) = 0;
    }

    //JPG sensor
    if ( JPEG_FMT != inDataType) {
        ISP_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN) = 0;
    }
    else {
        ISP_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN) = 1;
    }

    ISP_BITS(pisp, CAM_TG_PATH_CFG, SEN_IN_LSB) = 0x0;//no matter what kind of format, set 0
    ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1, TG1_FMT) = inDataType;

    ISP_BITS(pisp, CAM_TG_SEN_MODE, SOF_SRC) = sof_Src;

    //for Scam sof should trigger by Vsync falling edge (set to 1)to get more time for cmdQ in DDR mode
    return ret;

}

int SeninfDrvImp::setSVGrabRange(
    unsigned int camsvSel, unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    int svGrabWidth = 0;
    int svGrabHeight = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegBaseAddr[camsvSel];

    svGrabWidth = pixelEnd - pixelStart;
    svGrabHeight = lineEnd - lineStart;
    LOG_MSG("[setSVGrabRange](%u) CAMSV TG grab width = 0x%x, TG grab height = 0x%x", camsvSel, svGrabWidth, svGrabHeight);

    // TG Grab Win Setting
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_PXL, PXL_E) = pixelEnd;
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_PXL, PXL_S) = pixelStart;
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_LIN, LIN_E) = lineEnd;
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_LIN, LIN_S) = lineStart;

    return ret;
}

int SeninfDrvImp::setSVCfg(
    unsigned int camsvSel, TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb,
    unsigned int twoPxlMode, unsigned int sof_Src
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegBaseAddr[camsvSel];

    (void) senInLsb;

    LOG_MSG("[setSVCfg](%u) data type = %d, pixel mode = %u, sof_src = %u", camsvSel, inDataType, twoPxlMode, sof_Src);

#if 1
    if(1 == twoPxlMode) {
        ISP_BITS(pisp, CAMSV_TG_SEN_MODE, DBL_DATA_BUS) = 1;
        ISP_BITS(pisp, CAMSV_FMT_SEL, IMGO_BUS_SIZE) = 3;
    }
    else {
        ISP_BITS(pisp, CAMSV_TG_SEN_MODE, DBL_DATA_BUS) = 0;
        ISP_BITS(pisp, CAMSV_FMT_SEL, IMGO_BUS_SIZE) = 1;
    }

    //JPG sensor
    if ( JPEG_FMT == inDataType) {
        ISP_BITS(pisp, CAMSV_FMT_SEL, IMGO_FORMAT) = 2;
        ISP_BITS(pisp, CAMSV_TG_PATH_CFG, JPGINF_EN) = 1;
    }
    else if(YUV422_FMT == inDataType) {
        ISP_BITS(pisp, CAMSV_FMT_SEL, IMGO_FORMAT) = 1;
        ISP_BITS(pisp, CAMSV_TG_PATH_CFG, JPGINF_EN) = 1;
    }
    else {
        ISP_BITS(pisp, CAMSV_FMT_SEL, IMGO_FORMAT) = 0;
        ISP_BITS(pisp, CAMSV_TG_PATH_CFG, JPGINF_EN) = 1;
    }
#endif
    ISP_BITS(pisp, CAMSV_TG_PATH_CFG, SEN_IN_LSB) = 0x0;
    ISP_BITS(pisp, CAMSV_FMT_SEL, TG1_FMT) = inDataType;

    //for Scam sof should trigger by Vsync falling edge (set to 1)to get more time for cmdQ in DDR mode
    ISP_BITS(pisp, CAMSV_TG_SEN_MODE, SOF_SRC) = sof_Src;

    return ret;

}

int SeninfDrvImp::setSVViewFinderMode(
    unsigned int camsvSel, unsigned long spMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegBaseAddr[camsvSel];

    LOG_MSG("[setSVViewFinderMode](%u) sp mode = %lu", camsvSel, spMode);

    ISP_BITS(pisp, CAMSV_TG_SEN_MODE, CMOS_EN) = 1;
    ISP_BITS(pisp, CAMSV_TG_SEN_MODE, SOT_MODE) = 1;

    ISP_BITS(pisp, CAMSV_TG_VF_CON, SPDELAY_MODE) = 1;
    ISP_BITS(pisp, CAMSV_TG_VF_CON, SINGLE_MODE) = spMode;
    //ISP_BITS(pisp, CAMSV_TG_VF_CON, SP_DELAY) = spDelay;

    return ret;
}

unsigned long SeninfDrvImp::convertByteToPixel(
        unsigned int vcDataType, unsigned long sizeInBytes
)
{
    unsigned long ret = sizeInBytes;

    LOG_MSG("data type = 0x%x", vcDataType);
    if (vcDataType == 0x2B) {
        // Raw10 type
        ret = sizeInBytes * 8 / 10;
        LOG_MSG("Raw10 format. Convert from %lu to %lu", sizeInBytes, ret);
    }

    return ret;
}

bool SeninfDrvImp::isDualCamMode()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int inputValue = 0;

    property_get("vendor.debug.seninf.dualcam.camsv", value, "0");
    inputValue = atoi(value);

    if (inputValue > 0)
        return true;
    else
        return mDualCam;
}

void SeninfDrvImp::setDualCamMode(bool enableDualCam)
{
    LOG_MSG("[setDualCamMode] %d", enableDualCam);
    mDualCam = enableDualCam;
}

