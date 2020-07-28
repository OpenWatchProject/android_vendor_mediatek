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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "SeninfDrv"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <mtkcam/def/common.h>
#include "kd_seninf.h"
#include "seninf_reg.h"
#include "iseninf_drv.h"
#include "seninf_drv.h"
#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    ALOGD("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    ALOGD("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    ALOGE("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#include "uvvf.h"
#if 1
#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define LOG_MSG(fmt, arg...)
#define LOG_WRN(fmt, arg...)
#define LOG_ERR(fmt, arg...)
#endif
#endif

using namespace NSCam;
#include <hw_integration.h>

/******************************************************************************
*
*******************************************************************************/
#define SENINF_DEV_NAME     "/dev/seninf"
#define IS_4D1C (pCsiInfo->port < CUSTOM_CFG_CSI_PORT_0A)
#define IS_CDPHY_COMBO (pCsiInfo->port == CUSTOM_CFG_CSI_PORT_0A ||\
                        pCsiInfo->port == CUSTOM_CFG_CSI_PORT_0B ||\
                        pCsiInfo->port == CUSTOM_CFG_CSI_PORT_0)

/*Efuse definition*/
#define CSI2_EFUSE_SET
/*define sw Offset cal*/
#define CSI2_SW_OFFSET_CAL

#define SENINF_CSI_TYPE_MIPI(port, seninf)     {port, seninf, MIPI_SENSOR}
#define SENINF_CSI_TYPE_SERIAL(port, seninf)   {port, seninf, SERIAL_SENSOR}
#define SENINF_CSI_TYPE_PARALLEL(port, seninf) {port, seninf, PARALLEL_SENSOR}

static SENINF_CSI_INFO seninfCSITypeInfo[CUSTOM_CFG_CSI_PORT_MAX_NUM] = {
    SENINF_CSI_TYPE_MIPI(CUSTOM_CFG_CSI_PORT_0,  SENINF_1),
    SENINF_CSI_TYPE_MIPI(CUSTOM_CFG_CSI_PORT_1,  SENINF_3),
    SENINF_CSI_TYPE_MIPI(CUSTOM_CFG_CSI_PORT_2,  SENINF_5),
    SENINF_CSI_TYPE_MIPI(CUSTOM_CFG_CSI_PORT_0A, SENINF_1),
    SENINF_CSI_TYPE_MIPI(CUSTOM_CFG_CSI_PORT_0B, SENINF_2),
};

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::
getInstance()
{
    return SeninfDrvImp::createInstance();
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrvImp::
createInstance()
{
    static SeninfDrvImp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::SeninfDrvImp() : SeninfDrv()
{
    LOG_MSG("[SeninfDrv] \n");
    mUser = 0;
    mfd = 0;
    mpSeninfHwRegAddr =
    mpCSI2RxAnalogRegStartAddrAlign = NULL;
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::~SeninfDrvImp()
{
    LOG_MSG("[~SeninfDrv] \n");
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::init()
{
    MBOOL result;
    KD_SENINF_REG reg;
    LOG_MSG("[init]: Entry count %d \n", mUser);
    unsigned int i;
    Mutex::Autolock lock(mLock);
    if (mUser > 0) {
        LOG_MSG("Already inited \n");
        android_atomic_inc(&mUser);
        return 0;
    }
    // Open seninf driver
    mfd = open(SENINF_DEV_NAME, O_RDWR);
    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return -1;
    }
    if (ioctl(mfd, KDSENINFIOC_X_GET_REG_ADDR, &reg) < 0) {
       LOG_ERR("ERROR:KDSENINFIOC_X_GET_REG_ADDR\n");
       return -2;
    }
    // mmap seninf reg
    mpSeninfHwRegAddr = (unsigned char *) mmap(0, reg.seninf.map_length, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, reg.seninf.map_addr);
    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -5;
    }

    // mipi rx analog address
    mpCSI2RxAnalogRegStartAddrAlign = (unsigned char *) mmap(0, reg.ana.map_length, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, reg.ana.map_addr);
    if (mpCSI2RxAnalogRegStartAddrAlign == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s \n", errno, strerror(errno));
        return -9;
    }
    /*CSI2 Base address*/
    //MMAP only support Page alignment(0x1000)
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0]  = mpCSI2RxAnalogRegStartAddrAlign; //0x11C8 0000
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A] = mpCSI2RxAnalogRegStartAddrAlign; //0x11C8 0000
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B] = mpCSI2RxAnalogRegStartAddrAlign+ 0x1000; //0x11C8 1000

    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1]  = mpCSI2RxAnalogRegStartAddrAlign + 0x4000; //0x11C8 4000
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1A] = mpCSI2RxAnalogRegStartAddrAlign + 0x4000;
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1B] = mpCSI2RxAnalogRegStartAddrAlign + 0x5000;

    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2]  = mpCSI2RxAnalogRegStartAddrAlign + 0x8000; //0x11C8 8000
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2A] = mpCSI2RxAnalogRegStartAddrAlign + 0x8000;
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2B] = mpCSI2RxAnalogRegStartAddrAlign + 0x9000;

    mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_0] = mpCSI2RxAnalogRegStartAddrAlign + 0x2000; //0x11C8 2000
    mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_1] = mpCSI2RxAnalogRegStartAddrAlign + 0x6000; //0x11C8 6000
    mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_2] = mpCSI2RxAnalogRegStartAddrAlign + 0xA000; //0x11C8 A000

    mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_0] = mpCSI2RxAnalogRegStartAddrAlign + 0x3000; //0x11C8 3000
    mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_1] = mpCSI2RxAnalogRegStartAddrAlign + 0x7000; //0x11C8 7000
    mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_2] = mpCSI2RxAnalogRegStartAddrAlign + 0xB000; //0x11C8 B000

    for(i = SENINF_1; i < SENINF_NUM; i++ ) {
        mpSeninfCtrlRegAddr[i]       = mpSeninfHwRegAddr + 0X0200 + (0x1000*i);//mpSeninfCtrlRegAddr[SENINF_1]=0x1A004200
        mpSeninfTGRegAddr[i]         = mpSeninfHwRegAddr + 0X0600 + (0x1000*i);//mpSeninfTGRegAddr[SENINF_1]=0x1A004600
        mpSeninfCSIRxConfBaseAddr[i] = mpSeninfHwRegAddr + 0x0A00 + (0x1000*i);//mpSeninfCSIRxConfBaseAddr[SENINF_1]=1A004A00
    }
    for(int k = SENINF_MUX1; k < SENINF_MUX_NUM; k++ )
        mpSeninfMuxBaseAddr[k] = mpSeninfHwRegAddr + 0x0D00 + (0x1000*k);//mpSeninfMuxBaseAddr[SENINF_MUX1]=1A004D00

    mpSeninfCamMuxRegAddr = mpSeninfHwRegAddr + 0x0400; //1A004400

    for(int j = SENINF_CAM_MUX0; j < SENINF_CAM_MUX_NUM; j++) {
        disableCamMUX((SENINF_CAM_MUX_ENUM)j);
        memset(mCamMuxInfo + i, 0, sizeof(struct CamMuxInfo));
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
            mCSI[0] = devinfo.data[103];//0x11C1018C
            mCSI[1] = devinfo.data[104];//0x11C10190
            mCSI[2] = devinfo.data[115];//0x11C101BC
            mCSI[3] = devinfo.data[116];//0x11C101C0
        }
        LOG_MSG("Efuse Data:0x11C1018C= 0x%x, 0x11C10190= 0x%x, 0x11C101BC= 0x%x, 0x11C101C0= 0x%x\n",
            mCSI[0], mCSI[1], mCSI[2], mCSI[3]);
        close(fd);
    }
#endif
    memset(&supported_ISP_Clks, 0, sizeof(IMAGESENSOR_GET_SUPPORTED_ISP_CLK));
    if (ioctl(mfd, KDSENINFIOC_GET_SUPPORTED_ISP_CLOCKS, &supported_ISP_Clks) < 0)
        LOG_ERR("KDSENINFIOC_GET_SUPPORTED_ISP_CLOCKS fail\n");
    else {
        if (supported_ISP_Clks.clklevelcnt > 0) {
            for(int i = SENINF_MUX1; i < SENINF_MUX_NUM; i++ )
               mISPclk_tar[i] = supported_ISP_Clks.clklevel[supported_ISP_Clks.clklevelcnt-1];
        }
    }

    android_atomic_inc(&mUser);
    LOG_MSG("[init]: Exit count %d \n", mUser);
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::uninit()
{
    KD_SENINF_REG reg;
    unsigned int temp = 0;
    LOG_MSG("[uninit]: %d \n", mUser);
    Mutex::Autolock lock(mLock);
    if (mUser <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUser);
    if (mUser == 0) {
        for(int i = SENINF_MUX1; i < SENINF_MUX_NUM; i++ )
            mISPclk_tar[i] = 0;

        if (ioctl(mfd, KDSENINFIOC_X_GET_REG_ADDR, &reg) < 0) {
           LOG_ERR("ERROR:KDSENINFIOC_X_GET_REG_ADDR\n");
        }
        if ( 0 != mpSeninfHwRegAddr ) {
            if(munmap(mpSeninfHwRegAddr, reg.seninf.map_length) != 0) {
                LOG_ERR("mpSeninfHwRegAddr munmap err, %d, %s \n", errno, strerror(errno));
            }
            mpSeninfHwRegAddr = NULL;
        }
        if ( 0 != mpCSI2RxAnalogRegStartAddrAlign ) {
            if(munmap(mpCSI2RxAnalogRegStartAddrAlign, reg.ana.map_length) != 0){
                LOG_ERR("mpCSI2RxAnalogRegStartAddr munmap err, %d, %s \n", errno, strerror(errno));
            }
            mpCSI2RxAnalogRegStartAddrAlign = NULL;
        }

        LOG_MSG("[uninit]: %d, mfd(%d) \n", mUser, mfd);
        if (mfd > 0) {
            close(mfd);
            mfd = 0;
        }
    }
    else {
        LOG_ERR("Still users\n");
    }
    return 0;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::configMclk(SENINF_MCLK_PARA *pmclk_para, unsigned long pcEn)
{
    MINT32 ret = 0;

    std::string str_prop("vendor.debug.seninf.Tg");
    str_prop += std::to_string(pmclk_para->sensorIdx);
    str_prop +="clk";

    LOG_MSG("[Tg%dclk]: pcEn=%d freq=%d\n", pmclk_para->sensorIdx, pcEn, pmclk_para->mclkFreq);

    /*get property*/
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(str_prop.c_str(), value, "-1");

    int tgClk = atoi(value);
    if(tgClk > 0) {
        pmclk_para->mclkFreq = tgClk;
        LOG_MSG("setproperty Tg%dclk: %d \n", pmclk_para->sensorIdx, pmclk_para->mclkFreq);
    }

    ret = setMclk(pmclk_para->mclkIdx, pcEn, pmclk_para->mclkFreq);
    if (ret < 0) {
        LOG_MSG("setMclk fail\n");
        return ret;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk(MUINT8 mclkIdx, MBOOL pcEn, unsigned long freq)
{
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    sensorMclk.on   = pcEn;
    sensorMclk.freq = freq;
    sensorMclk.TG   = mclkIdx;

    if (ioctl(mfd, KDSENINFIOC_X_SET_MCLK_PLL, &sensorMclk) < 0) {
       LOG_ERR("ERROR:KDSENINFIOC_X_SET_MCLK_PLL\n");
    }
    LOG_MSG("[setTg%dPhase]pcEn(%d), freq(%d)\n", mclkIdx, (MINT32)pcEn, (MINT32)freq);
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
#define SENINF_DRV_DEBUG_DELAY 1000
int SeninfDrvImp::debug()
{
    int ret = 0;
    unsigned int mmsys_clk = 0;

    //seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    seninf_top_REGS *pSeninf = (seninf_top_REGS *)mpSeninfHwRegAddr; //0X1A004000
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr; //0X1A004400
    seninf1_csi2_REGS *pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[0];//0x1A004A00
    seninf1_mux_REGS *pSeninf_mux; //1A00 4D00
    unsigned int i, j, temp;
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_csiA = NULL;
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_csiB = NULL;
    csi0_cphy_top_reg_REGS *pCsi0_Cphy_Top = NULL; //0X1A00 3000
    csi0_dphy_top_reg_REGS *pCsi0_Dphy_Top = NULL; //0X1A00 4000

    unsigned int mipi_packet_cnt[SENINF_NUM] = {0};
    unsigned int temp_mipi_packet_cnt[SENINF_NUM] = {0};

    mmsys_clk = 8;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcam_ck(%d)\n", mmsys_clk);
    mmsys_clk = 38;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fseninf_ck(%d)\n", mmsys_clk);
    mmsys_clk = 39;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fseninf1_ck(%d)\n", mmsys_clk);
    mmsys_clk = 40;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fseninf2_ck(%d)\n", mmsys_clk);
    mmsys_clk = 17;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg_ck(%d)\n", mmsys_clk);
    mmsys_clk = 18;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg2_ck(%d)\n", mmsys_clk);
    mmsys_clk = 19;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg3_ck(%d)\n", mmsys_clk);
    mmsys_clk = 20;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg4_ck(%d)\n", mmsys_clk);

    usleep(SENINF_DRV_DEBUG_DELAY);

    for (j = CUSTOM_CFG_CSI_PORT_0A; j < CUSTOM_CFG_CSI_PORT_MAX_NUM; j++) {
        pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[j];
        LOG_MSG("MipiRx_ANA%d: CDPHY_RX_ANA_0(0x%x) ANA_2(0x%x) ANA_4(0x%x) ANA_5(0x%x) ANA_8(0x%x)\n",
                j-3,
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_0),
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_2),
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_4),
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_5),
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_8));
        LOG_MSG("MipiRx_ANA%d:CDPHY_RX_ANA_AD_0(0x%x) AD_HS_0(0x%x) AD_HS_1(0x%x)\n",
                j-3,
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_AD_0),
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_AD_HS_0),
                SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_AD_HS_1));
    }

    for (j = CUSTOM_CFG_CSI_PORT_0; j <= CUSTOM_CFG_CSI_PORT_2; j++) {
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[j]; //0X1A00 3000
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[j]; //0X1A00 4000

        LOG_MSG("Csi%d_Dphy_Top: LANE_EN(0x%x) LANE_SELECT(0x%x) DATA_LANE0_HS(0x%x) DATA_LANE1_HS(0x%x) DATA_LANE2_HS(0x%x) DATA_LANE3_HS(0x%x)\n",
                j,
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_LANE_EN),
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT),
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE0_HS_PARAMETER),
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE1_HS_PARAMETER),
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE2_HS_PARAMETER),
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE3_HS_PARAMETER));

        LOG_MSG("Csi%d_Cphy_Top: CPHY_RX_CTRL(0x%x) CPHY_RX_DETECT_CTRL_POST(0x%x)\n",
                j,
                SENINF_READ_REG(pCsi0_Cphy_Top, CPHY_RX_CTRL),
                SENINF_READ_REG(pCsi0_Cphy_Top, CPHY_RX_DETECT_CTRL_POST));
    }

    LOG_MSG("SENINF_TOP_MUX_CTRL_0(0x%x) SENINF_TOP_MUX_CTRL_1(0x%x)\n",
        SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL_0),
        SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL_1));

    LOG_MSG("SENINF_CAM_MUX_CTRL_0(0x%x) SENINF_CAM_MUX_CTRL_1(0x%x) SENINF_CAM_MUX_CTRL_2(0x%x)\n",
        SENINF_READ_REG(pSeninf_cam_mux,SENINF_CAM_MUX_CTRL_0),
        SENINF_READ_REG(pSeninf_cam_mux,SENINF_CAM_MUX_CTRL_1),
        SENINF_READ_REG(pSeninf_cam_mux,SENINF_CAM_MUX_CTRL_2));

    /* Seninf_csi status IRQ */
    for (i = SENINF_1; i < SENINF_NUM; i++) {
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[i];//0x1A004A00
        temp = SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_IRQ_STATUS);
        SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_IRQ_STATUS, 0xffffffff);
        usleep(SENINF_DRV_DEBUG_DELAY);
        LOG_MSG("SENINF%d_CSI2_EN(0x%x) SENINF_CSI2_OPT(0x%x) SENINF_CSI2_IRQ_STATUS(0x%x), CLR SENINF_CSI2_IRQ_STATUS(0x%x)\n",
            i,
            SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_EN),
            SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_OPT),
            temp,
            SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_IRQ_STATUS));
        if ((temp & 0xD0)!= 0)
            ret = -2; //multi lanes sync error, crc error, ecc error
    }

    /* Seninf_csi packet count */
    for (j = 0; j < 3; j++) {
        for (i = SENINF_1; i < SENINF_NUM; i++) {
            pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[i];//0x1A004A00
            if (SENINF_READ_REG(pSeninf_csi2 ,SENINF_CSI2_EN) & 0x1) {
                SENINF_BITS(pSeninf_csi2, SENINF_CSI2_DBG_CTRL, rg_csi2_dbg_packet_cnt_en) = 1;
                mipi_packet_cnt[i] = SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_PACKET_CNT_STATUS);
                if (temp_mipi_packet_cnt[i] == (mipi_packet_cnt[i]&0xFFFF))
                    ret = -1;
                temp_mipi_packet_cnt[i] = mipi_packet_cnt[i]&0xFFFF;
            }
        }
        usleep(SENINF_DRV_DEBUG_DELAY);
    }
    LOG_MSG("SENINF1_PkCnt(0x%x), SENINF2_PkCnt(0x%x), SENINF3_PkCnt(0x%x), SENINF4_PkCnt(0x%x), SENINF5_PkCnt(0x%x)\n",
        mipi_packet_cnt[SENINF_1],
        mipi_packet_cnt[SENINF_2],
        mipi_packet_cnt[SENINF_3],
        mipi_packet_cnt[SENINF_4],
        mipi_packet_cnt[SENINF_5]);

    /* SENINF_MUX */
    for (int k = SENINF_MUX1; k < SENINF_MUX_NUM; k++) {
        pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[k]; //1A00 4D00
        LOG_MSG("SENINF%d_MUX_CTRL0(0x%x) SENINF%d_MUX_CTRL1(0x%x) SENINF_MUX_IRQ_STATUS(0x%x) SENINF%d_MUX_SIZE(0x%x)\n",
            k,
            SENINF_READ_REG(pSeninf_mux, SENINF_MUX_CTRL_0), //[0]seninf_mux_en
            k,
            SENINF_READ_REG(pSeninf_mux, SENINF_MUX_CTRL_1), //[3:0] source, [9:8]pixel mode
            SENINF_READ_REG(pSeninf_mux, SENINF_MUX_IRQ_STATUS), //[0]FIFO
            k,
            SENINF_READ_REG(pSeninf_mux, SENINF_MUX_SIZE));

        if(SENINF_READ_REG(pSeninf_mux, SENINF_MUX_IRQ_STATUS) & 0x1) {
            SENINF_WRITE_REG(pSeninf_mux, SENINF_MUX_IRQ_STATUS, 0xffffffff);
            usleep(SENINF_DRV_DEBUG_DELAY);
            LOG_MSG("after reset overrun, SENINF_MUX_IRQ_STATUS(0x%x) SENINF%d_MUX_SIZE(0x%x)\n",
                SENINF_READ_REG(pSeninf_mux, SENINF_MUX_IRQ_STATUS), //[0]FIFO
                k,
                SENINF_READ_REG(pSeninf_mux, SENINF_MUX_SIZE));
        }
    }

    //check SENINF_CAM_MUX size
    LOG_MSG("SENINF_CAM_MUX size: MUX0(0x%x) MUX1(0x%x) MUX2(0x%x) MUX3(0x%x) MUX4(0x%x) MUX5(0x%x)\n",
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX0_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX1_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX2_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX3_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX4_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX5_CHK_RES));
    LOG_MSG("SENINF_CAM_MUX size: MUX6(0x%x) MUX7(0x%x) MUX8(0x%x) MUX9(0x%x) MUX10(0x%x)\n",
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX6_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX7_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX8_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX9_CHK_RES),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX10_CHK_RES));

    //check VC/DT split
    LOG_MSG("VC/DT split:SENINF_CAM_MUX0_OPT(0x%x) MUX1(0x%x) MUX2(0x%x) MUX3(0x%x) MUX4(0x%x) MUX5(0x%x)\n",
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX0_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX1_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX2_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX3_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX4_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX5_OPT));
    LOG_MSG("VC/DT split:SENINF_CAM_MUX6_OPT(0x%x) MUX7(0x%x) MUX8(0x%x) MUX9(0x%x) MUX10(0x%x)\n",
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX6_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX7_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX8_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX9_OPT),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX10_OPT));

    LOG_MSG("ret = %d", ret);

    return ret;
}
MBOOL SeninfDrvImp::isCamMUXUsed(SENINF_CAM_MUX_ENUM cam_mux)
{
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr;
    LOG_MSG("cam_mux %d SENINF_CAM_MUX_EN 0x%x isCamMUXUsed %d", cam_mux, SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN), SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN)&(1>>cam_mux));
    return SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN)&(1<<cam_mux);
}
int SeninfDrvImp::enableCamMUX(SENINF_CAM_MUX_ENUM cam_mux)
{
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr;
    unsigned int temp = SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN);
    SENINF_WRITE_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN, temp|(1<<cam_mux));
    LOG_MSG("SENINF_CAM_MUX_EN 0x%x cam_mux %d", SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN), cam_mux);
    return 0;
}
int SeninfDrvImp::disableCamMUX(SENINF_CAM_MUX_ENUM cam_mux)
{
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr;
    unsigned int temp = SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN);
    SENINF_WRITE_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN, temp&(~(1<<cam_mux)));
    LOG_MSG("SENINF_CAM_MUX_EN 0x%x cam_mux %d", SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_EN), cam_mux);
    mCamMuxUsage[cam_mux] = FREE;
    return 0;

}

int SeninfDrvImp::seninfCamMUXArbitration(SENINF_CAM_MUX_ENUM searchStart, SENINF_CAM_MUX_ENUM searchEnd)
{
    int i = searchStart;
    LOG_MSG("searchStart %d searchEnd %d", searchStart, searchEnd);
    LOG_MSG("mCamMuxUsage %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
            mCamMuxUsage[SENINF_CAM_MUX0], mCamMuxUsage[SENINF_CAM_MUX1], mCamMuxUsage[SENINF_CAM_MUX2], mCamMuxUsage[SENINF_CAM_MUX3],
            mCamMuxUsage[SENINF_CAM_MUX4], mCamMuxUsage[SENINF_CAM_MUX5], mCamMuxUsage[SENINF_CAM_MUX6], mCamMuxUsage[SENINF_CAM_MUX7],
            mCamMuxUsage[SENINF_CAM_MUX8], mCamMuxUsage[SENINF_CAM_MUX9], mCamMuxUsage[SENINF_CAM_MUX10]);

    while(i <= searchEnd) {
        if(mCamMuxUsage[i] == FREE && !this->isCamMUXUsed((SENINF_CAM_MUX_ENUM)i))
            break;
        i++;
    }

    if (i > searchEnd) {
        LOG_ERR("No free CAM MUX\n");
        return CAM_TG_ERR;
    }

    mCamMuxUsage[i] = WAIT_2_FINALIZED;
    if (i >= SENINF_CAM_MUX_SV_START)//camsv
        return i - SENINF_CAM_MUX_SV_START + CAM_SV_MASK;
    return i + CAM_TG_1;
}
int SeninfDrvImp::updateIspClk(unsigned int target_clk,  SENINF_MUX_ENUM mux){
#if 0 /*for debug*/
    unsigned int ISPclk_cur = 0;
    if ((ioctl(mfd, KDSENINFIOC_GET_CUR_ISP_CLOCK, &ISPclk_cur)) < 0)
        LOG_ERR("KDSENINFIOC_GET_CUR_ISP_CLOCK fail\n");
    LOG_MSG("IspClk_cur:%d\n", ISPclk_cur);

    for(int i = SENINF_MUX1; i < SENINF_MUX_NUM; i++)
        LOG_MSG("mISPclk_tar[%d] = %d",i ,mISPclk_tar[i]);
#endif
    unsigned int original_dfs = mISPclk_tar[SENINF_MUX1];
    for (int i = SENINF_MUX1 + 1; i < SENINF_MUX_NUM; i++){
        if(mISPclk_tar[i] >= original_dfs)
            original_dfs = mISPclk_tar[i];
    }

    if (mISPclk_tar[mux] > target_clk) {
        LOG_MSG("mISPclk_tar[%d] = %d, target_clk %d",mux ,mISPclk_tar[mux], target_clk);
        return 0;
    }
    else
        mISPclk_tar[mux] = target_clk;

    unsigned int new_dfs = mISPclk_tar[SENINF_MUX1];
    for (int i = SENINF_MUX1 + 1; i < SENINF_MUX_NUM; i++){
        if(mISPclk_tar[i] >= new_dfs)
            new_dfs = mISPclk_tar[i];
    }
    LOG_MSG("target_clk %d, new_dfs %d, original_dfs %d, mux %d",target_clk , new_dfs ,original_dfs, mux);

    if (new_dfs != original_dfs) {
        if (ioctl(mfd, KDSENINFIOC_DFS_UPDATE, (unsigned int*)&new_dfs) < 0)
            LOG_ERR("Update DFS error new_dfs %d\n", new_dfs);
    }
    return 0;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::sendCommand(int cmd, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    int ret = 0;
    if (cmd != CMD_SENINF_GET_FREE_CAM_MUX &&
        cmd != CMD_SENINF_SET_CAM_MUX &&
        cmd != CMD_SENINF_SET_MUX_CROP_PIX_CTRL &&
        cmd != CMD_SENINF_SET_DFS_FOR_MUX)
        mutexLock();


    if (mpSeninfHwRegAddr == NULL) {
        LOG_ERR("mpSeninfHwRegAddr = NULL, seninf has been uninit, stop dump to avoid NE\n");

        goto sendCommand_Exit;
    }

    switch (cmd) {
    case CMD_SENINF_SET_DFS_FOR_MUX:
    {
        SeninfConfigInfo* pConfigInfo = (SeninfConfigInfo*)arg1;
        SENINF_CSI_MIPI* pCsiMipi = (SENINF_CSI_MIPI*)arg2;
        SENINF_MUX_ENUM mux = (SENINF_MUX_ENUM)pConfigInfo->seninf_mux_info;
        unsigned int traget_clk = 0;
        MUINT64 pixel_rate = IS_MUX_HW_BUFFERED(mux)
        ? pConfigInfo->grab_size_x * (pCsiMipi->pclk/(pCsiMipi->line_length - HW_BUF_EFFECT))
        : pCsiMipi->mipi_pixel_rate;

        if (pixel_rate == 0) {
            LOG_ERR("pixel_rate == 0CMD_SENINF_SET_DFS_FOR_MUX pCsiMipi->pclk %d pCsiMipi->line_length %d IS_MUX_HW_BUFFERED(mux) %d",
                    pCsiMipi->pclk, pCsiMipi->line_length, IS_MUX_HW_BUFFERED(mux));
            pixel_rate = pCsiMipi->mipi_pixel_rate;
        }
        LOG_MSG("CMD_SENINF_SET_DFS_FOR_MUX pixel rate %llu pConfigInfo->mux_pixel_mode %d", pixel_rate, pConfigInfo->mux_pixel_mode);
        for (int i = 0; i < supported_ISP_Clks.clklevelcnt; i++) {
            if (((MUINT64)supported_ISP_Clks.clklevel[i] * 1000000 * (1 << pConfigInfo->mux_pixel_mode)) > pixel_rate)
                traget_clk = supported_ISP_Clks.clklevel[i];
        }
        if (traget_clk == 0) {
            traget_clk = supported_ISP_Clks.clklevel[0];
            LOG_WRN("sensor mux will overrun, may need adjust pixel mode");
        }
        updateIspClk(traget_clk, mux);
    }
        break;
    case CMD_SENINF_SET_MUX_CROP_PIX_CTRL:
    {
        SeninfConfigInfo* pConfigInfo = (SeninfConfigInfo*)arg1;
        bool crop_enable = (bool)arg2;
        setSeninfMuxCrop(
            (SENINF_MUX_ENUM)pConfigInfo->seninf_mux_info,
            pConfigInfo->grab_start_offset,
            pConfigInfo->grab_start_offset + pConfigInfo->grab_size_x - 1,
            crop_enable);
    }
        break;
    case CMD_SENINF_FINALIZE_CAM_MUX:
    {
        SENINF_CAM_MUX_ENUM CamMux_orig = convertTgEnum2CamMux(arg1);
        SENINF_CAM_MUX_ENUM CamMux_target = convertTgEnum2CamMux(arg2);

        if (mCamMuxUsage[CamMux_target] != FREE && mCamMuxUsage[CamMux_target] != WAIT_2_FINALIZED)
            LOG_MSG("cam mux %d state not correct %d", CamMux_target, mCamMuxUsage[CamMux_target]);


        setSeninfCamMuxVC(CamMux_target,
                        mCamMuxInfo[CamMux_orig].VcInfo2.VC_ID,
                        mCamMuxInfo[CamMux_orig].VcInfo2.VC_DataType,
                        mCamMuxInfo[CamMux_orig].VcInfo2.VC_DataType != 0,
                        mCamMuxInfo[CamMux_orig].VcInfo2.VC_DataType != 0);
        setSeninfCamMuxSrc((SENINF_MUX_ENUM)mCamMuxInfo[CamMux_orig].ConfigInfo.seninf_mux_info, CamMux_target);
        setCamMuxChkPixelMode(CamMux_target, mCamMuxInfo[CamMux_orig].ConfigInfo.mux_pixel_mode);
        enableCamMUX(CamMux_target);
        //release origin
        mCamMuxUsage[CamMux_orig] = FREE;
        mCamMuxUsage[CamMux_target] = mCamMuxInfo[CamMux_orig].ConfigInfo.seninf_mux_info;
        memset(&mCamMuxInfo[CamMux_orig], 0, sizeof(CamMuxInfo));
    }
        break;
    case CMD_SENINF_GET_FREE_CAM_MUX:
    {
        int* ptg_select = (int*)arg1;
        *ptg_select = seninfCamMUXArbitration((SENINF_CAM_MUX_ENUM) arg2, (SENINF_CAM_MUX_ENUM)  arg3);
    }
        break;
    case CMD_SENINF_SET_CAM_MUX:
    {
        SeninfConfigInfo* pConfigInfo = (SeninfConfigInfo*)arg1;
        SINGLE_VC_INFO2* pVcInfo2 = (SINGLE_VC_INFO2*)arg2;
        SENINF_CAM_MUX_ENUM CamMuxSel = convertTgEnum2CamMux(pConfigInfo->tg_info);

        memcpy(&mCamMuxInfo[CamMuxSel].ConfigInfo, pConfigInfo, sizeof(SeninfConfigInfo));
        if(pVcInfo2 == NULL)
            memset(&mCamMuxInfo[CamMuxSel].VcInfo2, 0, sizeof(SINGLE_VC_INFO2));
        else
            memcpy(&mCamMuxInfo[CamMuxSel].VcInfo2, pVcInfo2, sizeof(SINGLE_VC_INFO2));
#if 0
        setSeninfCamMuxVC(CamMuxSel,
            pVcInfo2 == NULL ?0 :pVcInfo2->VC_ID,
            pVcInfo2 == NULL ?0 :pVcInfo2->VC_DataType,
            pVcInfo2 == NULL ?0 :1,
            pVcInfo2 == NULL ?0 :1);
        setSeninfCamMuxSrc((SENINF_MUX_ENUM)pConfigInfo->seninf_mux_info, CamMuxSel);
        setCamMuxChkPixelMode(CamMuxSel , pConfigInfo->mux_pixel_mode);
#endif
    }
        break;
    case CMD_SENINF_DEBUG_TASK:
    case CMD_SENINF_DEBUG_TASK_CAMSV:
        ret = debug();
        break;

    case CMD_SENINF_GET_SENINF_ADDR:
        *(unsigned long *) arg1 = (unsigned long) mpSeninfHwRegAddr;
        break;

    case CMD_SENINF_DEBUG_PIXEL_METER:
    {
        unsigned int * meter_array = (unsigned int *)arg1;
        if(meter_array != NULL) {
            seninf1_mux_REGS *pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[0]; //1A00 4D00
            SENINF_BITS(pSeninf_mux, SENINF_MUX_FRAME_SIZE_MON_CTRL, rg_seninf_mux_frame_size_mon_en) = 1;
            meter_array[0] = SENINF_READ_REG(pSeninf_mux, SENINF_MUX_FRAME_SIZE_MON_H_VALID); //1A00 4DB0
            meter_array[1] = SENINF_READ_REG(pSeninf_mux, SENINF_MUX_FRAME_SIZE_MON_H_BLANK); //1A00 4DB4
            meter_array[2] = SENINF_READ_REG(pSeninf_mux, SENINF_MUX_FRAME_SIZE_MON_V_VALID); //1A00 4DB8
            meter_array[3] = SENINF_READ_REG(pSeninf_mux, SENINF_MUX_FRAME_SIZE_MON_V_BLANK); //1A00 4DBC
        }
    }
        break;
    default:
        ret = -1;
        break;
    }

sendCommand_Exit:

    if (cmd != CMD_SENINF_GET_FREE_CAM_MUX &&
        cmd != CMD_SENINF_SET_CAM_MUX &&
        cmd != CMD_SENINF_SET_MUX_CROP_PIX_CTRL &&
        cmd != CMD_SENINF_SET_DFS_FOR_MUX)
        mutexUnlock();

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfTopMuxCtrl(
    SENINF_MUX_ENUM seninfMuXIdx, SENINF_ENUM seninfSrc
)
{
    int ret = 0;
    seninf_top_REGS *pSeninf = (seninf_top_REGS *)mpSeninfHwRegAddr; //0X1A004000

    switch (seninfMuXIdx){
    case SENINF_MUX1:
        SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL_0, rg_seninf_mux1_src_sel) = seninfSrc;
        break;
    case SENINF_MUX2:
        SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL_0, rg_seninf_mux2_src_sel) = seninfSrc;
        break;
    case SENINF_MUX3:
        SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL_0, rg_seninf_mux3_src_sel) = seninfSrc;
        break;
    case SENINF_MUX4:
        SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL_0, rg_seninf_mux4_src_sel) = seninfSrc;
        break;
    case SENINF_MUX5:
        SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL_1, rg_seninf_mux5_src_sel) = seninfSrc;
        break;
    case SENINF_MUX6:
        SENINF_BITS(pSeninf, SENINF_TOP_MUX_CTRL_1, rg_seninf_mux6_src_sel) = seninfSrc;
        break;
    default:
    break;
    }
    LOG_MSG("SENINF_TOP_MUX_CTRL_0(0x%x) SENINF_TOP_MUX_CTRL_1(0x%x)\n",
        SENINF_READ_REG(pSeninf, SENINF_TOP_MUX_CTRL_0),
    SENINF_READ_REG(pSeninf, SENINF_TOP_MUX_CTRL_1));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfCamTGMuxCtrl(
    unsigned int targetCamTG, SENINF_MUX_ENUM seninfMuxSrc
)
{
    int ret = 0;
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr; //0X1A004400

    switch (targetCamTG) {
    case SENINF_CAM_MUX0:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux0_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX1:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux1_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX2:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux2_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX3:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux3_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX4:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux4_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX5:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux5_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX6:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux6_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX7:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux7_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX8:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_2, rg_seninf_cam_mux8_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX9:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_2, rg_seninf_cam_mux9_src_sel) = seninfMuxSrc;
        break;
    case SENINF_CAM_MUX10:
        SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_2, rg_seninf_cam_mux10_src_sel) = seninfMuxSrc;
        break;
    default:
        LOG_MSG("No support SENINF_CAM_MUX%d", targetCamTG);
        break;
    }
    LOG_MSG("SENINF_CAM_MUX_CTRL_0(0x%x) SENINF_CAM_MUX_CTRL_1(0x%x) SENINF_CAM_MUX_CTRL_2(0x%x)\n",
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_0),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_1),
        SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_2));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::getSeninfTopMuxCtrl(SENINF_MUX_ENUM seninfMuXIdx)
{
    seninf_top_REGS *pSeninf = (seninf_top_REGS *)mpSeninfHwRegAddr; //0X1A004000
    unsigned int seninfSrc = 0;
    unsigned int temp0 = 0;
    unsigned int temp1 = 0;

    temp0 = SENINF_READ_REG(pSeninf, SENINF_TOP_MUX_CTRL_0);
    temp1 = SENINF_READ_REG(pSeninf, SENINF_TOP_MUX_CTRL_1);
    switch (seninfMuXIdx){
    case SENINF_MUX1:
        seninfSrc = (temp0&0xF);
        break;
    case SENINF_MUX2:
        seninfSrc = (temp0&0xF00)>>8;
        break;
    case SENINF_MUX3:
        seninfSrc = (temp0&0xF0000)>>16;
        break;
    case SENINF_MUX4:
        seninfSrc = (temp0&0xF000000)>>24;
        break;
    case SENINF_MUX5:
        seninfSrc = (temp1&0xF);
        break;
    case SENINF_MUX6:
        seninfSrc = (temp1&0xF00)>>8;
        break;
    default:
        LOG_MSG("No support SENINF_MUX%d", seninfMuXIdx);
        break;
    }

    return seninfSrc;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::getSeninfCamTGMuxCtrl(unsigned int targetCamTG)
{
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr; //0X1A004400
    unsigned int seninfMuxSrc = 0;
    unsigned int temp0 = SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_0);
    unsigned int temp1 = SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_1);
    unsigned int temp2 = SENINF_READ_REG(pSeninf_cam_mux, SENINF_CAM_MUX_CTRL_2);

    switch (targetCamTG){
    case SENINF_CAM_MUX0:
        seninfMuxSrc = (temp0&0xF);
        break;
    case SENINF_CAM_MUX1:
        seninfMuxSrc = (temp0&0xF00)>>8;
        break;
    case SENINF_CAM_MUX2:
        seninfMuxSrc = (temp0&0xF0000)>>16;
        break;
    case SENINF_CAM_MUX3:
        seninfMuxSrc = (temp0&0xF000000)>>24;
        break;
    case SENINF_CAM_MUX4:
        seninfMuxSrc = (temp1&0xF);
        break;
    case SENINF_CAM_MUX5:
        seninfMuxSrc = (temp1&0xF00)>>8;
        break;
    case SENINF_CAM_MUX6:
        seninfMuxSrc = (temp1&0xF0000)>>16;
        break;
    case SENINF_CAM_MUX7:
        seninfMuxSrc = (temp1&0xF000000)>>24;
        break;
    case SENINF_CAM_MUX8:
        seninfMuxSrc = (temp2&0xF);
        break;
    case SENINF_CAM_MUX9:
        seninfMuxSrc = (temp2&0xF00)>>8;
        break;
    case SENINF_CAM_MUX10:
        seninfMuxSrc = (temp2&0xF0000)>>16;
        break;
    default:
        LOG_MSG("No support SENINF_CAM_MUX %d", targetCamTG);
        break;
    }
    return seninfMuxSrc;

}

/*******************************************************************************
* 2rd VC/DT Split
********************************************************************************/
#if 1

int SeninfDrvImp::setSeninfCamMuxVC(SENINF_CAM_MUX_ENUM CamMuxSel,
    unsigned int vc_sel, unsigned int dt_sel, unsigned int vc_en, unsigned int dt_en)
{
    LOG_MSG("CamMuxSel %d vc_sel %d dt_sel %d, vc_en %d dt_en %d", CamMuxSel, vc_sel, dt_sel, vc_en, dt_en);

    int ret = 0;
    seninf_cam_mux_REGS *mpSeninfCamMuxVCAddr =
        (seninf_cam_mux_REGS *)(mpSeninfCamMuxRegAddr + (4 * CamMuxSel)); //0x1A00 4420

    SENINF_BITS(mpSeninfCamMuxVCAddr, SENINF_CAM_MUX0_OPT, rg_seninf_cam_mux0_vc_sel) = vc_sel;
    SENINF_BITS(mpSeninfCamMuxVCAddr, SENINF_CAM_MUX0_OPT, rg_seninf_cam_mux0_dt_sel) = dt_sel;
    SENINF_BITS(mpSeninfCamMuxVCAddr, SENINF_CAM_MUX0_OPT, rg_seninf_cam_mux0_vc_en) = vc_en;
    SENINF_BITS(mpSeninfCamMuxVCAddr, SENINF_CAM_MUX0_OPT, rg_seninf_cam_mux0_dt_en) = dt_en;

    return 0;
}

int SeninfDrvImp::setSeninfCamMuxSrc(SENINF_MUX_ENUM src, SENINF_CAM_MUX_ENUM target)
{
    LOG_MSG("cam mux target %d src %d\n", target, src);

    seninf_cam_mux_REGS *mpSeninfCamMux_base = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr;
    switch(target) {
        case SENINF_CAM_MUX0:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux0_src_sel) = src;
            break;
        case SENINF_CAM_MUX1:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux1_src_sel) = src;
            break;
        case SENINF_CAM_MUX2:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux2_src_sel) = src;
            break;
        case SENINF_CAM_MUX3:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_0, rg_seninf_cam_mux3_src_sel) = src;
            break;
        case SENINF_CAM_MUX4:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux4_src_sel) = src;
            break;
        case SENINF_CAM_MUX5:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux5_src_sel) = src;
            break;
        case SENINF_CAM_MUX6:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux6_src_sel) = src;
            break;
        case SENINF_CAM_MUX7:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_1, rg_seninf_cam_mux7_src_sel) = src;
            break;
        case SENINF_CAM_MUX8:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_2, rg_seninf_cam_mux8_src_sel) = src;
            break;
        case SENINF_CAM_MUX9:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_2, rg_seninf_cam_mux9_src_sel) = src;
            break;
        case SENINF_CAM_MUX10:
            SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX_CTRL_2, rg_seninf_cam_mux10_src_sel) = src;
            break;

        default:
            LOG_MSG("No support SENINF_CAM_MUX s: %d t: %d", src, target);
            break;
    }
    return 0;
}

#endif
SENINF_CAM_MUX_ENUM SeninfDrvImp::convertTgEnum2CamMux(int TgEnum)
{
    return (SENINF_CAM_MUX_ENUM)(TgEnum < CAM_SV_1 ?(TgEnum - CAM_TG_1) :(TgEnum - CAM_SV_MASK + CAM_TG_3));
}

int SeninfDrvImp::setSeninfVC(SENINF_ENUM SenInfsel,
    unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id,
    unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id)
{ /* seninfCSI2 stream num = 8*/
    int ret = 0;
    seninf1_csi2_REGS *pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SenInfsel];//0x1A004A00

    /* General Long Packet Data Types: 0x10-0x17 */
    if ((0x10 <= (vc0Id>>2) && (vc0Id>>2) <= 0x17) ||
        (0x10 <= (vc1Id>>2) && (vc1Id>>2) <= 0x17) ||
        (0x10 <= (vc2Id>>2) && (vc2Id>>2) <= 0x17) ||
        (0x10 <= (vc3Id>>2) && (vc3Id>>2) <= 0x17) ||
        (0x10 <= (vc4Id>>2) && (vc4Id>>2) <= 0x17) ||
        (0x10 <= (vc5Id>>2) && (vc5Id>>2) <= 0x17))
            SENINF_BITS(pSeninf_csi2, SENINF_CSI2_OPT, rg_csi2_generic_long_packet_en) = 1;//1A004A04

    if((vc0Id == 0) && (vc1Id == 0) && (vc2Id == 0) && (vc3Id == 0) && (vc4Id == 0) && (vc5Id == 0))
        return ret;

	SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_CH1_CTRL, 0); //0x1A00 4A64
	SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_CH2_CTRL, 0); //0x1A00 4A68
	SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_CH3_CTRL, 0); //0x1A00 4A6C

    if((vc0Id&0xfc) != 0) {
        //stream0 data identifier
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S0_DI_CTRL, rg_csi2_s0_dt_sel) = (vc0Id>>2);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S0_DI_CTRL, rg_csi2_s0_vc_sel) = (vc0Id&0x03);
        //data type interleave enable, 0:Disable, 1:Include, 2:Exclude
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S0_DI_CTRL, rg_csi2_s0_dt_interleave_mode) = 1;
        //virtual channel interleave enable
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S0_DI_CTRL, rg_csi2_s0_vc_interleave_en) = 1;
    } else {
        //All types of data can be propregated
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S0_DI_CTRL, rg_csi2_s0_dt_interleave_mode) = 0;
        //All types of virtual channel data can be propregated
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S0_DI_CTRL, rg_csi2_s0_vc_interleave_en) = 0;
    }
    if((vc1Id&0xfc) != 0) {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S1_DI_CTRL, rg_csi2_s1_dt_sel) = (vc1Id>>2);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S1_DI_CTRL, rg_csi2_s1_vc_sel) = (vc1Id&0x03);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S1_DI_CTRL, rg_csi2_s1_dt_interleave_mode) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S1_DI_CTRL, rg_csi2_s1_vc_interleave_en) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_CH1_CTRL, rg_csi2_ch1_s1_grp_en) = 1;
    } else {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S1_DI_CTRL, rg_csi2_s1_dt_interleave_mode) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S1_DI_CTRL, rg_csi2_s1_vc_interleave_en) = 0;
    }
    if((vc2Id&0xfc) != 0) {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S2_DI_CTRL, rg_csi2_s2_dt_sel) = (vc2Id>>2);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S2_DI_CTRL, rg_csi2_s2_vc_sel) = (vc2Id&0x03);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S2_DI_CTRL, rg_csi2_s2_dt_interleave_mode) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S2_DI_CTRL, rg_csi2_s2_vc_interleave_en) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_CH1_CTRL, rg_csi2_ch1_s2_grp_en) = 1;
    } else {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S2_DI_CTRL, rg_csi2_s2_dt_interleave_mode) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S2_DI_CTRL, rg_csi2_s2_vc_interleave_en) = 0;
    }
    if((vc3Id&0xfc) != 0) {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S3_DI_CTRL, rg_csi2_s3_dt_sel) = (vc3Id>>2);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S3_DI_CTRL, rg_csi2_s3_vc_sel) = (vc3Id&0x03);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S3_DI_CTRL, rg_csi2_s3_dt_interleave_mode) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S3_DI_CTRL, rg_csi2_s3_vc_interleave_en) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_CH1_CTRL, rg_csi2_ch1_s3_grp_en) = 1;
    } else {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S3_DI_CTRL, rg_csi2_s3_dt_interleave_mode) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S3_DI_CTRL, rg_csi2_s3_vc_interleave_en) = 0;
    }
    if((vc4Id&0xfc) != 0) {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S4_DI_CTRL, rg_csi2_s4_dt_sel) = (vc4Id>>2);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S4_DI_CTRL, rg_csi2_s4_vc_sel) = (vc4Id&0x03);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S4_DI_CTRL, rg_csi2_s4_dt_interleave_mode) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S4_DI_CTRL, rg_csi2_s4_vc_interleave_en) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_CH1_CTRL, rg_csi2_ch1_s4_grp_en) = 1;
    } else {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S4_DI_CTRL, rg_csi2_s4_dt_interleave_mode) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S4_DI_CTRL, rg_csi2_s4_vc_interleave_en) = 0;
    }
    if((vc5Id&0xfc) != 0) {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S5_DI_CTRL, rg_csi2_s5_dt_sel) = (vc5Id>>2);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S5_DI_CTRL, rg_csi2_s5_vc_sel) = (vc5Id&0x03);
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S5_DI_CTRL, rg_csi2_s5_dt_interleave_mode) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S5_DI_CTRL, rg_csi2_s5_vc_interleave_en) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_CH1_CTRL, rg_csi2_ch1_s5_grp_en) = 1;
    } else {
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S5_DI_CTRL, rg_csi2_s5_dt_interleave_mode) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_S5_DI_CTRL, rg_csi2_s5_vc_interleave_en) = 0;
    }

    LOG_MSG("CSI2 Stream0(0x%x), Stream1(0x%x), Stream2(0x%x), Stream3(0x%x), Stream4(0x%x), Stream0(0x%x), Group0(0x%x), Group1(0x%x)",
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_S0_DI_CTRL),
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_S1_DI_CTRL),
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_S2_DI_CTRL),
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_S3_DI_CTRL),
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_S4_DI_CTRL),
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_S5_DI_CTRL),
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_CH0_CTRL),
        SENINF_READ_REG(pSeninf_csi2, SENINF_CSI2_CH1_CTRL));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfMuxCtrl(
    SENINF_MUX_ENUM mux, unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int PixelMode
)
{
    int ret = 0;
    unsigned int temp = 0;
    seninf1_mux_REGS *pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[mux]; //0X1A004D00
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr; //0X1A004400

    //1A00 4D04[3:0] select source group
    if (inSrcTypeSel == TEST_MODEL)
        SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_1, rg_seninf_mux_src_sel) = 1;
    else if (inSrcTypeSel == MIPI_SENSOR)
        SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_1, rg_seninf_mux_src_sel) = 8; //group0
    else
    SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_1, rg_seninf_mux_src_sel) = 9; //group1

    SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_1, rg_seninf_mux_pix_mode_sel) = PixelMode;
    if (PixelMode > 2)
        PixelMode = 0;
//    set_camux_checker_pixel_mode(0, PixelMode);

    SENINF_BITS(pSeninf_mux, SENINF_MUX_OPT, rg_seninf_mux_hsync_pol) = hsPol;
    SENINF_BITS(pSeninf_mux, SENINF_MUX_OPT, rg_seninf_mux_vsync_pol) = vsPol;

    temp = SENINF_READ_REG(pSeninf_mux, SENINF_MUX_CTRL_0);
    SENINF_WRITE_REG(pSeninf_mux,SENINF_MUX_CTRL_0, temp|0x6);//reset
    SENINF_WRITE_REG(pSeninf_mux,SENINF_MUX_CTRL_0, temp&0xFFFFFFF9);//clear reset

    LOG_MSG("SENINF_MUX_CTRL_1(0x%x), SENINF_MUX_OPT(0x%x)",
        SENINF_READ_REG(pSeninf_mux, SENINF_MUX_CTRL_1),
        SENINF_READ_REG(pSeninf_mux, SENINF_MUX_OPT));

    return ret;
}

int SeninfDrvImp::setSeninfMuxCrop(
    SENINF_MUX_ENUM mux, unsigned int start_x, unsigned int end_x, bool enable)
{
    seninf1_mux_REGS * pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[mux]; //1A00 4D00

    SENINF_BITS(pSeninf_mux, SENINF_MUX_CROP_PIX_CTRL, rg_seninf_mux_crop_start_8pix_cnt) = start_x/8;
    SENINF_BITS(pSeninf_mux, SENINF_MUX_CROP_PIX_CTRL, rg_seninf_mux_crop_end_8pix_cnt) = start_x/8 +
                                                        (end_x - start_x + 1)/8 - 1 + (((end_x - start_x + 1) % 8) > 0 ?1 :0);
    SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_1, rg_seninf_mux_crop_en) = enable;

    LOG_MSG("SENINF_MUX_CROP_PIX_CTRL 0x%x SENINF_MUX_CTRL_1 0x%x, mux %d, start %d, end %d, enable %d \n",
        SENINF_READ_REG(pSeninf_mux, SENINF_MUX_CROP_PIX_CTRL),
        SENINF_READ_REG(pSeninf_mux, SENINF_MUX_CTRL_1),
        mux,
        start_x,
        end_x,
        enable);
    return 0;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL SeninfDrvImp::isMUXUsed(SENINF_MUX_ENUM mux)
{
    seninf1_mux_REGS * pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[mux]; //1A00 4D00

    return SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_0, seninf_mux_en);
}

int SeninfDrvImp::enableMUX(SENINF_MUX_ENUM mux)
{
    seninf1_mux_REGS * pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[mux];

    SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_0, seninf_mux_en) = 1;
    return 0;
}

int SeninfDrvImp::disableMUX(SENINF_MUX_ENUM mux)
{
    seninf1_mux_REGS * pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[mux];
    SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_0, seninf_mux_en) = 0;

    //also disable CAM_MUX with input from mux
    for(int i = SENINF_CAM_MUX0; i < SENINF_CAM_MUX_NUM; i++) {
        if (mux == this->getSeninfCamTGMuxCtrl(i))
            this->disableCamMUX((SENINF_CAM_MUX_ENUM)i);
    }
    updateIspClk(supported_ISP_Clks.clklevelcnt > 0 ?supported_ISP_Clks.clklevel[supported_ISP_Clks.clklevelcnt-1] :0, mux);
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::Efuse(unsigned long csi_sel)
{
    int ret = 0;
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana = NULL;

    if ((mCSI[0] != 0) && (csi_sel == 0)) {
        pMipiRx_ana = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A]; //0x11C8 0000
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0P_T0A_HSRT_CODE) = (mCSI[0]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0N_T0B_HSRT_CODE) = (mCSI[0]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1P_T0C_HSRT_CODE) = (mCSI[0]>>22) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1N_T1A_HSRT_CODE) = (mCSI[0]>>22) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2P_T1B_HSRT_CODE) = (mCSI[0]>>17) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2N_T1C_HSRT_CODE) = (mCSI[0]>>17) & 0x1f;
        LOG_MSG("CSI0A CDPHY_RX_ANA_2(0x%x) CDPHY_RX_ANA_3(0x%x) CDPHY_RX_ANA_4(0x%x)",
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_2),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_3),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_4));
        pMipiRx_ana = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B]; //0x11C8 1000
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0P_T0A_HSRT_CODE) = (mCSI[0]>>12) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0N_T0B_HSRT_CODE) = (mCSI[0]>>12) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1P_T0C_HSRT_CODE) = (mCSI[0]>>7) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1N_T1A_HSRT_CODE) = (mCSI[0]>>7) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2P_T1B_HSRT_CODE) = (mCSI[0]>>2) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2N_T1C_HSRT_CODE) = (mCSI[0]>>2) & 0x1f;
        LOG_MSG("CSI0B CDPHY_RX_ANA_2(0x%x) CDPHY_RX_ANA_3(0x%x) CDPHY_RX_ANA_4(0x%x)",
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_2),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_3),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_4));
    }
    if ((mCSI[1] != 0)&&(mCSI[2] != 0) && (csi_sel == 1)) {
        pMipiRx_ana = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1A]; //0x11C8 4000
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0P_T0A_HSRT_CODE) = (mCSI[1]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0N_T0B_HSRT_CODE) = (mCSI[1]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1P_T0C_HSRT_CODE) = (mCSI[1]>>22) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1N_T1A_HSRT_CODE) = (mCSI[1]>>22) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2P_T1B_HSRT_CODE) = (mCSI[1]>>17) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2N_T1C_HSRT_CODE) = (mCSI[1]>>17) & 0x1f;
        LOG_MSG("CSI1A CDPHY_RX_ANA_2(0x%x) CDPHY_RX_ANA_3(0x%x) CDPHY_RX_ANA_4(0x%x)",
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_2),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_3),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_4));
        pMipiRx_ana = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1B]; //0x11C8 5000
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0P_T0A_HSRT_CODE) = (mCSI[2]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0N_T0B_HSRT_CODE) = (mCSI[2]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1P_T0C_HSRT_CODE) = (mCSI[2]>>22) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1N_T1A_HSRT_CODE) = (mCSI[2]>>22) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2P_T1B_HSRT_CODE) = (mCSI[2]>>17) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2N_T1C_HSRT_CODE) = (mCSI[2]>>17) & 0x1f;
        LOG_MSG("CSI1B CDPHY_RX_ANA_2(0x%x) CDPHY_RX_ANA_3(0x%x) CDPHY_RX_ANA_4(0x%x)",
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_2),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_3),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_4));
    }
    if((mCSI[2] != 0)&&(mCSI[3] != 0) && (csi_sel == 2))
    {
        pMipiRx_ana = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2A]; //0x11C8 8000
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0P_T0A_HSRT_CODE) = (mCSI[2]>>12) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0N_T0B_HSRT_CODE) = (mCSI[2]>>12) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1P_T0C_HSRT_CODE) = (mCSI[2]>>7) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1N_T1A_HSRT_CODE) = (mCSI[2]>>7) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2P_T1B_HSRT_CODE) = (mCSI[2]>>2) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_4, RG_CSI0_L2N_T1C_HSRT_CODE) = (mCSI[2]>>2) & 0x1f;
        LOG_MSG("CSI2A CDPHY_RX_ANA_2(0x%x) CDPHY_RX_ANA_3(0x%x) CDPHY_RX_ANA_4(0x%x)",
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_2),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_3),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_4));
        pMipiRx_ana = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2B]; //0x11C8 9000
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0P_T0A_HSRT_CODE) = (mCSI[3]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_2, RG_CSI0_L0N_T0B_HSRT_CODE) = (mCSI[3]>>27) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1P_T0C_HSRT_CODE) = (mCSI[3]>>22) & 0x1f;
        SENINF_BITS(pMipiRx_ana, CDPHY_RX_ANA_3, RG_CSI0_L1N_T1A_HSRT_CODE) = (mCSI[3]>>22) & 0x1f;
        LOG_MSG("CSI2B CDPHY_RX_ANA_2(0x%x) CDPHY_RX_ANA_3(0x%x)",
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_2),
            SENINF_READ_REG(pMipiRx_ana, CDPHY_RX_ANA_3));
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
SENINF_CSI_INFO* SeninfDrvImp::getCSIInfo(CUSTOM_CFG_CSI_PORT mipiPort)
{
    int i;
    SENINF_CSI_INFO *pCsiTypeInfo = seninfCSITypeInfo;

    for(i=0; i<CUSTOM_CFG_CSI_PORT_MAX_NUM; i++) {
        if(seninfCSITypeInfo[i].port == mipiPort) {
            return &seninfCSITypeInfo[i];
        }
    }

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclkIODrivingCurrent(MUINT32 mclkIdx, unsigned long ioDrivingCurrent)
{
    (void)mclkIdx;
    (void)ioDrivingCurrent;
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setCamMuxChkPixelMode(SENINF_CAM_MUX_ENUM cam_mux, unsigned int pixelMode)
{
    LOG_MSG("cam mux %d chk pixelMode(%d), 0:1pixel, 1:2pixel, 2:4pixel\n",cam_mux, pixelMode); //0:1pixel, 1:2pixel, 2:4pixel
    seninf_cam_mux_REGS *mpSeninfCamMux_base = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr;
        switch(cam_mux) {
            case SENINF_CAM_MUX0:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX0_CHK_CTL_0, rg_seninf_cam_mux0_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX1:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX1_CHK_CTL_0, rg_seninf_cam_mux1_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX2:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX2_CHK_CTL_0, rg_seninf_cam_mux2_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX3:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX3_CHK_CTL_0, rg_seninf_cam_mux3_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX4:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX4_CHK_CTL_0, rg_seninf_cam_mux4_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX5:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX5_CHK_CTL_0, rg_seninf_cam_mux5_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX6:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX6_CHK_CTL_0, rg_seninf_cam_mux6_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX7:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX7_CHK_CTL_0, rg_seninf_cam_mux7_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX8:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX8_CHK_CTL_0, rg_seninf_cam_mux8_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX9:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX9_CHK_CTL_0, rg_seninf_cam_mux9_pix_mode_sel) = pixelMode;
                break;
            case SENINF_CAM_MUX10:
                SENINF_BITS(mpSeninfCamMux_base, SENINF_CAM_MUX10_CHK_CTL_0, rg_seninf_cam_mux10_pix_mode_sel) = pixelMode;
                break;

            default:
                LOG_MSG("No support SENINF_CAM_MUX cam_mux: %d pixelMode: %d", cam_mux, pixelMode);
                break;
        }
        return 0;
}


int SeninfDrvImp::setTestModel(bool en, unsigned int dummypxl, unsigned int vsync, unsigned int line,unsigned int pxl)
{
    int ret = 0;
    int i, j;
    seninf1_mux_REGS    *pSeninf_mux;
    seninf1_REGS        *pSeninf;
    SENINF_TG1_REGS     *pSeninf_tg;
    unsigned int pixelMode = pxl; //0:one pixel, 1:two pixel, 2: four pixel
    seninf_cam_mux_REGS *pSeninf_cam_mux = (seninf_cam_mux_REGS *)mpSeninfCamMuxRegAddr; //0X1A004400

    LOG_MSG("E!\n");
    if (pixelMode > 2)
        pixelMode = 0;

    SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_EN, seninf_cam_mux0_en) = 1;
    SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_EN, seninf_cam_mux1_en) = 1;
    SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_EN, seninf_cam_mux2_en) = 1;
    SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_EN, seninf_cam_mux3_en) = 1;
    SENINF_BITS(pSeninf_cam_mux, SENINF_CAM_MUX_EN, seninf_cam_mux4_en) = 1;

    for(j = SENINF_CAM_MUX0; j < SENINF_CAM_MUX_NUM; j++) //set pixel mode
        setCamMuxChkPixelMode((SENINF_CAM_MUX_ENUM)j, pixelMode);

    for(i = SENINF_1; i < SENINF_NUM; i++) {
        pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[i]; //1A00 4DG151700
        pSeninf        = (seninf1_REGS *)mpSeninfCtrlRegAddr[i]; //0x1A00 4200
        pSeninf_tg  = (SENINF_TG1_REGS *)mpSeninfTGRegAddr[i];//0x1A00 4600

        //Config seninf mux to output testmdl signal
        //SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_1, rg_seninf_mux_src_sel) = 1;
        SENINF_WRITE_REG(pSeninf_mux, SENINF_MUX_CTRL_1, 0x001f0001); //[9:8] pixel mode
        SENINF_WRITE_REG(pSeninf_mux, SENINF_MUX_CTRL_0, 0x1);//1a004d00
        SENINF_WRITE_REG(pSeninf, SENINF_TESTMDL_CTRL, 1); //1a004220
        SENINF_WRITE_REG(pSeninf, SENINF_CTRL, 1);         //1a004200
        SENINF_WRITE_REG(pSeninf_tg, TM_SIZE, 0x10001F00); //1a00460c
        SENINF_WRITE_REG(pSeninf_tg, TM_CLK, 0x7);         //1a004610
        //set pixel mode
        SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_1, rg_seninf_mux_pix_mode_sel) = pixelMode;
        //Trigger testmdl
        SENINF_WRITE_REG(pSeninf_tg, TM_CTL, 0x300161);    //1a004608
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::getN3DDiffCnt(MUINT32 *pCnt)
{ /*for hwsync*/
    int ret = 0;

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::CSI2rx_init(CUSTOM_CFG_CSI_PORT mipiPort)
{
    int i = 0;
    int is_4d1c = 0;
    csi0_cphy_top_reg_REGS *pCsi0_Cphy_Top = NULL;
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_base = NULL;

    if (mipiPort < CUSTOM_CFG_CSI_PORT_0A) {
        is_4d1c = 1;

        switch(mipiPort) {
        case CUSTOM_CFG_CSI_PORT_0:
            mipiPort = CUSTOM_CFG_CSI_PORT_0A;
            break;
        case CUSTOM_CFG_CSI_PORT_1:
            mipiPort = CUSTOM_CFG_CSI_PORT_1A;
            break;
        case CUSTOM_CFG_CSI_PORT_2:
            mipiPort = CUSTOM_CFG_CSI_PORT_2A;
            break;
        default:
            break;
        }
    }
    for(i = 0; i <= is_4d1c; i++) {
        pMipiRx_ana_base = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[mipiPort + i];
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_BG_LPRX_VTL_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_BG_LPRX_VTH_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_BG_ALP_RX_VTL_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_BG_ALP_RX_VTH_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_BG_VREF_SEL) = 0x8;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_CDPHY_EQ_DES_VREF_SEL) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_BW) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_IS) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_LATCH_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_DG0_EN) = 0x0;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_DG1_EN) = 0x0;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_SR0) = 0x0;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_SR1) = 0x0;

        //r50 termination
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_L0P_T0A_HSRT_CODE) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_L0N_T0B_HSRT_CODE) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_3, RG_CSI0_L1P_T0C_HSRT_CODE) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_3, RG_CSI0_L1N_T1A_HSRT_CODE) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_4, RG_CSI0_L2P_T1B_HSRT_CODE) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_4, RG_CSI0_L2N_T1C_HSRT_CODE) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_0, RG_CSI0_CPHY_T0_CDR_FIRST_EDGE_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_0, RG_CSI0_CPHY_T1_CDR_FIRST_EDGE_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_CPHY_T0_CDR_SELF_CAL_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_CPHY_T1_CDR_SELF_CAL_EN) = 0x1;

        LOG_MSG("mipiPort:%d CISAB:%d CDPHY_RX_ANA_0(0x%x)\n",
            mipiPort,
            i,
            SENINF_READ_REG(pMipiRx_ana_base, CDPHY_RX_ANA_0));
    }

    switch(mipiPort) {
    case CUSTOM_CFG_CSI_PORT_0:
    case CUSTOM_CFG_CSI_PORT_0A:
    case CUSTOM_CFG_CSI_PORT_0B:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_0];
        break;
    case CUSTOM_CFG_CSI_PORT_1:
    case CUSTOM_CFG_CSI_PORT_1A:
    case CUSTOM_CFG_CSI_PORT_1B:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_1];
        break;
    case CUSTOM_CFG_CSI_PORT_2:
    case CUSTOM_CFG_CSI_PORT_2A:
    case CUSTOM_CFG_CSI_PORT_2B:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_2];
        break;
    default:
        break;
    }
    SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_DETECT_CTRL_POST, rg_cphy_rx_data_valid_post_en) = 0x1; //0x11C83018

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfCsi(void *pCsi, SENINF_SOURCE_ENUM src)
{
    int ret = 0;
    switch(src) {
    case MIPI_SENSOR:
        ret = setSeninfCsiMipi((SENINF_CSI_MIPI *)pCsi);
        break;
    case SERIAL_SENSOR:
    case PARALLEL_SENSOR:
    default:
        break;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfCsiMipi(SENINF_CSI_MIPI *pCsiMipi)
{
    int ret = 0;
    int i;
    unsigned int CalSel = 0;
    SENINF_ENUM SenInfsel;
    SENINF_CSI_INFO *pCsiInfo = pCsiMipi->pCsiInfo;
    seninf1_csi2_REGS *pSeninf_csi2 = NULL;//0x1A004A00
    csi0_cphy_top_reg_REGS *pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[0]; //0X1A00 3000
    csi0_dphy_top_reg_REGS *pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[0]; //0X1A00 4000
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_base = NULL;
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A];
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B];
    seninf1_REGS *pSeninf = (seninf1_REGS *)mpSeninfCtrlRegAddr[pCsiInfo->seninf];
    seninf_top_REGS *pSeninf_top = (seninf_top_REGS *)mpSeninfHwRegAddr; //0X1A004000

    CSI2rx_init(pCsiInfo->port);

    // phy mode
    switch(pCsiInfo->port) {
    case CUSTOM_CFG_CSI_PORT_0:
        SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI0, rg_phy_seninf_mux0_cphy_mode) = 0; //4T
        break;
    case CUSTOM_CFG_CSI_PORT_0A:
    case CUSTOM_CFG_CSI_PORT_0B:
        SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI0, rg_phy_seninf_mux0_cphy_mode) = 2; //2T+2T
        break;
    case CUSTOM_CFG_CSI_PORT_1:
        SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI1, rg_phy_seninf_mux1_cphy_mode) = 0; //4T
        break;
    case CUSTOM_CFG_CSI_PORT_1A:
    case CUSTOM_CFG_CSI_PORT_1B:
        SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI1, rg_phy_seninf_mux1_cphy_mode) = 2; //2T+2T
        break;
    //case CUSTOM_CFG_CSI_PORT_2:
    default:
        break;
    }

    // port operation mode
    switch(pCsiInfo->port) {
    case CUSTOM_CFG_CSI_PORT_0:
    case CUSTOM_CFG_CSI_PORT_0A:
    case CUSTOM_CFG_CSI_PORT_0B:
        CalSel = 0;
        if (pCsiMipi->csi_type != CSI2_2_5G_CPHY) { //Dphy
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI0, phy_seninf_mux0_cphy_en) = 0;
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI0, phy_seninf_mux0_dphy_en) = 1;
        } else {
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI0, phy_seninf_mux0_dphy_en) = 0;
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI0, phy_seninf_mux0_cphy_en) = 1;
        }
        break;
    case CUSTOM_CFG_CSI_PORT_1:
    case CUSTOM_CFG_CSI_PORT_1A:
    case CUSTOM_CFG_CSI_PORT_1B:
        CalSel = 1;
        if (pCsiMipi->csi_type != CSI2_2_5G_CPHY) { //Dphy
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI1, phy_seninf_mux1_cphy_en) = 0;
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI1, phy_seninf_mux1_dphy_en) = 1;
        } else {
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI1, phy_seninf_mux1_dphy_en) = 0;
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI1, phy_seninf_mux1_cphy_en) = 1;
        }
        break;
    case CUSTOM_CFG_CSI_PORT_2:
        CalSel = 2;
        if (pCsiMipi->csi_type != CSI2_2_5G_CPHY) { //Dphy
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI2, phy_seninf_mux2_cphy_en) = 0;
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI2, phy_seninf_mux2_dphy_en) = 1;
        } else {
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI2, phy_seninf_mux2_dphy_en) = 0;
            SENINF_BITS(pSeninf_top, SENINF_TOP_PHY_CTRL_CSI2, phy_seninf_mux2_cphy_en) = 1;
        }
        break;
    default:
        break;
    }

    int laneNum =
            pCsiMipi->dlaneNum == SENSOR_MIPI_4_LANE ? 0xF :
            pCsiMipi->dlaneNum == SENSOR_MIPI_3_LANE ? 0x7 :
            pCsiMipi->dlaneNum == SENSOR_MIPI_2_LANE ? 0x3 :
            0x1;
    // configure lane/trio number
    pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_1];//0x1A004A00
    switch(pCsiInfo->port) {
    case CUSTOM_CFG_CSI_PORT_0:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_0];
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_0];
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_1];//0x1A004A00
        SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, laneNum); //4lane
        break;
    case CUSTOM_CFG_CSI_PORT_1:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_1];
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_1];
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_3];
        SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, laneNum); //4lane
        break;
    case CUSTOM_CFG_CSI_PORT_2:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_2];
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_2];
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_5];
        SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, laneNum); //4lane
        break;
    case CUSTOM_CFG_CSI_PORT_0A:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_0];
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_0];
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_1];
         SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, laneNum);
        break;
    case CUSTOM_CFG_CSI_PORT_0B:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_0];
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_0];
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_2];
        SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, laneNum);
        break;
    case CUSTOM_CFG_CSI_PORT_1A:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_1];
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_1];
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_3];
         SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, laneNum);
        break;
    case CUSTOM_CFG_CSI_PORT_1B:
        pCsi0_Cphy_Top = (csi0_cphy_top_reg_REGS *)mpCSICphyTopAddr[CUSTOM_CFG_CSI_PORT_1];
        pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[CUSTOM_CFG_CSI_PORT_1];
        pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[SENINF_4];
        SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, laneNum);
        break;
    default:
        break;
    }
    SENINF_BITS(pSeninf_csi2, SENINF_CSI2_DBG_CTRL, rg_csi2_dbg_packet_cnt_en) = 1;

    // lane/trio count
    SENINF_BITS(pSeninf_csi2, SENINF_CSI2_RESYNC_MERGE_CTRL, rg_csi2_resync_cycle_cnt_opt) = 1;
    if (pCsiMipi->csi_type != CSI2_2_5G_CPHY) { //Dphy
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_OPT, rg_csi2_cphy_sel) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_OPT, rg_csi2_spec_v2p0_sel) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_HDR_MODE_0, rg_csi2_header_mode) = 0;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_HDR_MODE_0, rg_csi2_header_len) = 0;
    } else { //Cphy
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_OPT, rg_csi2_cphy_sel) = 1;
        SENINF_BITS(pSeninf_csi2, SENINF_CSI2_HDR_MODE_0, rg_csi2_header_mode) = 2; //cphy

        switch(pCsiInfo->port) {
        case CUSTOM_CFG_CSI_PORT_0:
        case CUSTOM_CFG_CSI_PORT_1:
        case CUSTOM_CFG_CSI_PORT_2:
            SENINF_BITS(pSeninf_csi2, SENINF_CSI2_HDR_MODE_0, rg_csi2_header_len) = 4; //3trio
            break;
        case CUSTOM_CFG_CSI_PORT_0A:
        case CUSTOM_CFG_CSI_PORT_0B:
        case CUSTOM_CFG_CSI_PORT_1A:
        case CUSTOM_CFG_CSI_PORT_1B:
        case CUSTOM_CFG_CSI_PORT_2A:
        case CUSTOM_CFG_CSI_PORT_2B:
            SENINF_BITS(pSeninf_csi2, SENINF_CSI2_HDR_MODE_0, rg_csi2_header_len) = 2; //2trio
            break;
        default:
            break;
        }
    }

    //hal_csi2rx_phy_settings
    for(i = CUSTOM_CFG_CSI_PORT_0A; i <= CUSTOM_CFG_CSI_PORT_2B; i++) {
        pMipiRx_ana_base = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[i]; //0x11C8 0000
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_BG_LPRX_VTL_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_BG_LPRX_VTH_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_BG_ALP_RX_VTL_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_BG_ALP_RX_VTH_SEL) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_BG_VREF_SEL) = 0x8;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_1, RG_CSI0_CDPHY_EQ_DES_VREF_SEL) = 0x2;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_BW) = 0x3;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_IS) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_LATCH_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_DG0_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_DG1_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_SR0) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_5, RG_CSI0_CDPHY_EQ_SR1) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_0, RG_CSI0_CPHY_T0_CDR_FIRST_EDGE_EN) = 0x0;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_0, RG_CSI0_CPHY_T1_CDR_FIRST_EDGE_EN) = 0x0;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_CPHY_T0_CDR_SELF_CAL_EN) = 0x1;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_2, RG_CSI0_CPHY_T1_CDR_SELF_CAL_EN) = 0x1;

        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_6, RG_CSI0_CPHY_T0_CDR_CK_DELAY) = 0x0;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_7, RG_CSI0_CPHY_T1_CDR_CK_DELAY) = 0x0;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_6, RG_CSI0_CPHY_T0_CDR_AB_WIDTH) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_6, RG_CSI0_CPHY_T0_CDR_BC_WIDTH) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_6, RG_CSI0_CPHY_T0_CDR_CA_WIDTH) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_7, RG_CSI0_CPHY_T1_CDR_AB_WIDTH) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_7, RG_CSI0_CPHY_T1_CDR_BC_WIDTH) = 0x4;
        SENINF_BITS(pMipiRx_ana_base, CDPHY_RX_ANA_7, RG_CSI0_CPHY_T1_CDR_CA_WIDTH) = 0x4;
    }

    // enable/disable seninf csi2
    SENINF_BITS(pSeninf, SENINF_CSI2_CTRL, rg_seninf_csi2_en) = 1;//pCsiMipi->enable;//(SENINF_CTL_BASE) |= 0x1;

    // enable/disable seninf, enable after csi2, testmdl is done.
    SENINF_BITS(pSeninf, SENINF_CTRL, seninf_en) = 1;//pCsiMipi->enable;//(SENINF_CTL_BASE) |= 0x1;

    // phy setting
    if (pCsiMipi->csi_type != CSI2_2_5G_CPHY) { //Dphy
        if (IS_4D1C) {
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld0_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld1_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld2_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld3_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_lc0_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_lc1_en) = 0;

            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld3_sel) = 4;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld2_sel) = 0;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld1_sel) = 3;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld0_sel) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_lc0_sel) = 2;
        } else { //2D1C*2
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld0_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld1_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld2_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_ld3_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_lc0_en) = 1;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_EN, dphy_rx_lc1_en) = 1;

            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld3_sel) = 5;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld2_sel) = 3;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld1_sel) = 2;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_ld0_sel) = 0;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_lc1_sel) = 4;
            SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, rg_dphy_rx_lc0_sel) = 1;
        }
        SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT, dphy_rx_ck_data_mux_en) = 1;
        LOG_MSG("IS_4D1C:%d DPHY_RX_LANE_SELECT 0x%x DPHY_RX_LANE_EN 0x%x",
                IS_4D1C,
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_LANE_SELECT),
                SENINF_READ_REG(pCsi0_Dphy_Top, DPHY_RX_LANE_EN));
    } else {
        if (pCsiMipi->dlaneNum == 2) { //cphy-3trio
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr0_lprx_en) = 1;
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr1_lprx_en) = 1;
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr2_lprx_en) = 1;
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr3_lprx_en) = 0;
        } else { //cphy-2trio*2
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr0_lprx_en) = 1;
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr1_lprx_en) = 1;
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr2_lprx_en) = 1;
            SENINF_BITS(pCsi0_Cphy_Top, CPHY_RX_CTRL, cphy_rx_tr3_lprx_en) = 1;
        }
    }

	switch(pCsiInfo->port) {
	case CUSTOM_CFG_CSI_PORT_0:
	case CUSTOM_CFG_CSI_PORT_0A:
	case CUSTOM_CFG_CSI_PORT_0B:
		pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A];
		pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B];
		break;
	case CUSTOM_CFG_CSI_PORT_1:
	case CUSTOM_CFG_CSI_PORT_1A:
	case CUSTOM_CFG_CSI_PORT_1B:
		pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1A];
		pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1B];
		break;
	case CUSTOM_CFG_CSI_PORT_2:
	case CUSTOM_CFG_CSI_PORT_2A:
	case CUSTOM_CFG_CSI_PORT_2B:
		pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2A];
		pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2B];
		break;
	default:
		break;
	}

    if (pCsiMipi->csi_type != CSI2_2_5G_CPHY) { //Dphy
        // clear clk sel first
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKSEL) = 0; //1:clock lane, 0:data lane
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKSEL) = 0;
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKSEL) = 0;
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKSEL) = 0;
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKSEL) = 0;
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKSEL) = 0;
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0 ,RG_CSI0_CPHY_EN) = 0;
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0 ,RG_CSI0_CPHY_EN) = 0;
        if (IS_4D1C) {
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKSEL) = 1; //1:clock lane, 0:data lane
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKSEL) = 1;
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKSEL) = 1;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKSEL) = 1;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKSEL) = 1;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKSEL) = 1;

            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKMODE_EN) = 0; //1:clock lane, 0:data lane
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKMODE_EN) = 0;
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKMODE_EN) = 1;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKMODE_EN) = 0;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKMODE_EN) = 0;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKMODE_EN) = 0;
        } else { //2T2T
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKSEL) = 0;
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKSEL) = 0;
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKSEL) = 0;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKSEL) = 0;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKSEL) = 0;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKSEL) = 0;

            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKMODE_EN) = 0;
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKMODE_EN) = 1;
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKMODE_EN) = 0;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L0_CKMODE_EN) = 0;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L1_CKMODE_EN) = 1;
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_DPHY_L2_CKMODE_EN) = 0;
        }
    }else {
            SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0 ,RG_CSI0_CPHY_EN) = 1;
            SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0 ,RG_CSI0_CPHY_EN) = 1;
    }

#ifdef CSI2_EFUSE_SET
    /*Read Efuse value : termination control registers*/
    Efuse(CalSel);
#endif

    // receiver phy power on
    // c/d-phy eq os calibration, 1: enable 0: clear
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L0_T0AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L1_T1AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L2_T1BC_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L0_T0AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L1_T1AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L2_T1BC_EQ_OS_CAL_EN) = 0;

    // c-phy eq os calibration, 1: enable 0: clear
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T0BC_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T1CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T1CA_EQ_OS_CAL_EN) = 0;

    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_LPF_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_BG_LPF_EN) = 0;

    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_CORE_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_BG_CORE_EN) = 0;

	usleep(10);
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_CORE_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_BG_CORE_EN) = 1;

	usleep(30);
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_LPF_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_BG_LPF_EN) = 1;
    usleep(1);
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L0_T0AB_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L1_T1AB_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L2_T1BC_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L0_T0AB_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L1_T1AB_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L2_T1BC_EQ_OS_CAL_EN) = 1;

    // c-phy eq os calibration, 1: enable 0: clear
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T0BC_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T1CA_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 1;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T1CA_EQ_OS_CAL_EN) = 1;

    /*Settle delay by lane*/
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE0_HS_PARAMETER, rg_cdphy_rx_ld0_trio0_hs_prepare_parameter) = 2;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE1_HS_PARAMETER, rg_cdphy_rx_ld1_trio1_hs_prepare_parameter) = 2;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE2_HS_PARAMETER, rg_cdphy_rx_ld2_trio2_hs_prepare_parameter) = 2;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE3_HS_PARAMETER, rg_cdphy_rx_ld3_trio3_hs_prepare_parameter) = 2;

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE0_HS_PARAMETER, rg_cdphy_rx_ld0_trio0_hs_settle_parameter) = SENINF_SETTLE_DELAY_DT; //0x11c8 2020[23:16]
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE1_HS_PARAMETER, rg_cdphy_rx_ld1_trio1_hs_settle_parameter) = SENINF_SETTLE_DELAY_DT; //0x11c8 2024
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE2_HS_PARAMETER, rg_cdphy_rx_ld2_trio2_hs_settle_parameter) = SENINF_SETTLE_DELAY_DT; //0x11c8 2028
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE3_HS_PARAMETER, rg_cdphy_rx_ld3_trio3_hs_settle_parameter) = SENINF_SETTLE_DELAY_DT; //0x11c8 202C

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_CLOCK_LANE0_HS_PARAMETER, rg_dphy_rx_lc0_hs_settle_parameter) = SENINF_SETTLE_DELAY_CK; //0x11c8 2010[23:16]
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_CLOCK_LANE1_HS_PARAMETER, rg_dphy_rx_lc1_hs_settle_parameter) = SENINF_SETTLE_DELAY_CK; //0x11c8 2014

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    unsigned int hs_trail;
    property_get("vendor.debug.seninf.hs_trail", value, "0");
    hs_trail = atoi(value);

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE0_HS_PARAMETER, rg_dphy_rx_ld0_hs_trail_parameter) = (hs_trail) ? hs_trail : SENINF_HS_TRAIL_PARAMETER; //0x11c8 2020[15:8]
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE1_HS_PARAMETER, rg_dphy_rx_ld1_hs_trail_parameter) = (hs_trail) ? hs_trail : SENINF_HS_TRAIL_PARAMETER;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE2_HS_PARAMETER, rg_dphy_rx_ld2_hs_trail_parameter) = (hs_trail) ? hs_trail : SENINF_HS_TRAIL_PARAMETER;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE3_HS_PARAMETER, rg_dphy_rx_ld3_hs_trail_parameter) = (hs_trail) ? hs_trail : SENINF_HS_TRAIL_PARAMETER;

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE0_HS_PARAMETER, rg_dphy_rx_ld0_hs_trail_en) = 1; //0x11c8 2020[29]
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE1_HS_PARAMETER, rg_dphy_rx_ld1_hs_trail_en) = 1;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE2_HS_PARAMETER, rg_dphy_rx_ld2_hs_trail_en) = 1;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DATA_LANE3_HS_PARAMETER, rg_dphy_rx_ld3_hs_trail_en) = 1;
    //if (pCsiMipi->csi_type == CSI2_2_5G_CPHY) {

        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_2 ,RG_CSI0_CPHY_T0_CDR_SELF_CAL_EN) = 0;
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_2 ,RG_CSI0_CPHY_T1_CDR_SELF_CAL_EN) = 0;
        SENINF_WRITE_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_5, 0x1177);
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_2 ,RG_CSI0_CPHY_T0_CDR_SELF_CAL_EN) = 0;
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_2 ,RG_CSI0_CPHY_T1_CDR_SELF_CAL_EN) = 0;
        SENINF_WRITE_REG(pMipiRx_ana_csiB, CDPHY_RX_ANA_5, 0x1177);
    //}

    LOG_MSG("CSI2_IP:%d, csiA CDPHY_RX_ANA_0 0x%x, csiB CDPHY_RX_ANA_0 0x%x",
        pCsiInfo->port,
        SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_0),
        SENINF_READ_REG(pMipiRx_ana_csiB, CDPHY_RX_ANA_0));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::powerOff(void *pCsi)
{
    int ret = 0;
    SENINF_CSI_MIPI *pCsiMipi = (SENINF_CSI_MIPI *)pCsi;
    SENINF_CSI_INFO *pCsiInfo = pCsiMipi->pCsiInfo;
    seninf1_csi2_REGS *pSeninf_csi2 = (seninf1_csi2_REGS *)mpSeninfCSIRxConfBaseAddr[pCsiInfo->seninf];//0x1A004A00
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A];
    mipi_rx_ana_cdphy_csi0a_reg_REGS *pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B];

    SENINF_WRITE_REG(pSeninf_csi2, SENINF_CSI2_EN, 0x0); //data lane disable
    switch(pCsiInfo->port) {
    case CUSTOM_CFG_CSI_PORT_0:
        pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A];
        pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B];
        break;
    case CUSTOM_CFG_CSI_PORT_1:
        pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1A];
        pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1B];
        break;
    case CUSTOM_CFG_CSI_PORT_2:
        pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2A];
        pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2B];
        break;
    case CUSTOM_CFG_CSI_PORT_0A:
    case CUSTOM_CFG_CSI_PORT_0B:
    case CUSTOM_CFG_CSI_PORT_1A:
    case CUSTOM_CFG_CSI_PORT_1B:
    case CUSTOM_CFG_CSI_PORT_2A:
    case CUSTOM_CFG_CSI_PORT_2B:
        pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[pCsiInfo->port];
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_LPF_EN) = 0;
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_CORE_EN) = 0;
        break;
    default:
        break;
    }

    //if(pCsiInfo->port < CUSTOM_CFG_CSI_PORT_0A) {
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_LPF_EN) = 0;
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_BG_LPF_EN) = 0;
        SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_0, RG_CSI0_BG_CORE_EN) = 0;
        SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_0, RG_CSI0_BG_CORE_EN) = 0;
    //}

    // c/d-phy eq os calibration, 1: enable 0: clear
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L0_T0AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L1_T1AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_L2_T1BC_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L0_T0AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L1_T1AB_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_L2_T1BC_EQ_OS_CAL_EN) = 0;

    // c-phy eq os calibration, 1: enable 0: clear
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T0BC_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiA, CDPHY_RX_ANA_8, RG_CSI0_XX_T1CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T0CA_EQ_OS_CAL_EN) = 0;
    SENINF_BITS(pMipiRx_ana_csiB, CDPHY_RX_ANA_8, RG_CSI0_XX_T1CA_EQ_OS_CAL_EN) = 0;


    pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A];
    pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B];
    LOG_MSG("csi0A CDPHY_RX_ANA_0 0x%x, csi0B CDPHY_RX_ANA_0 0x%x",
        SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_0),
        SENINF_READ_REG(pMipiRx_ana_csiB, CDPHY_RX_ANA_0));

    pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1A];
    pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1B];
    LOG_MSG("csi1A CDPHY_RX_ANA_0 0x%x, csi1B CDPHY_RX_ANA_0 0x%x",
        SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_0),
        SENINF_READ_REG(pMipiRx_ana_csiB, CDPHY_RX_ANA_0));

    pMipiRx_ana_csiA = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2A];
    pMipiRx_ana_csiB = (mipi_rx_ana_cdphy_csi0a_reg_REGS *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2B];
    LOG_MSG("csi2A CDPHY_RX_ANA_0 0x%x, csi2B CDPHY_RX_ANA_0 0x%x",
        SENINF_READ_REG(pMipiRx_ana_csiA, CDPHY_RX_ANA_0),
        SENINF_READ_REG(pMipiRx_ana_csiB, CDPHY_RX_ANA_0));

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::resetConfiguration(IMGSENSOR_SENSOR_IDX sensorIdx)
{
    int ret = 0;
    ret = resetHwIntegration(sensorIdx);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setIntegration(IMGSENSOR_SENSOR_IDX sensorIdx, SENINF_MUX_ENUM mux,
            MSize crop, MUINT framerate)
{
    struct HW_INTEGRATION_STRUCT config;

    config.pAddr = mpSeninfHwRegAddr;
    config.cropWidth = crop.w;
    config.cropHeight = crop.h;
    config.frameRate = framerate * 10;

    return setHwIntegration(sensorIdx, mux, &config);
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::mutexLock()
{
    mLock.lock();
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::mutexUnlock()
{
    mLock.unlock();
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::reset(CUSTOM_CFG_CSI_PORT mipiPort)
{
    int i;
    seninf1_mux_REGS *pSeninf_mux;
    SENINF_ENUM   seninf  = getCSIInfo(mipiPort)->seninf;
    seninf1_REGS *pSeninf = (seninf1_REGS *)mpSeninfCtrlRegAddr[seninf];

    SENINF_BITS(pSeninf, SENINF_CSI2_CTRL, seninf_csi2_sw_rst) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, SENINF_CSI2_CTRL, seninf_csi2_sw_rst) = 0;

    for(i = SENINF_MUX1; i < SENINF_MUX_NUM; i++)
        if(getSeninfTopMuxCtrl((SENINF_MUX_ENUM)i) == seninf && isMUXUsed((SENINF_MUX_ENUM)i)) {
            pSeninf_mux = (seninf1_mux_REGS *)mpSeninfMuxBaseAddr[i];
            SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_0, seninf_mux_sw_rst) = 1;
            usleep(1);
            SENINF_BITS(pSeninf_mux, SENINF_MUX_CTRL_0, seninf_mux_sw_rst) = 0;
        }

    LOG_MSG("reset\n");

    return 0;
}

#if 0
int SeninfDrvImp::dphy_hw_deskew_en(CUSTOM_CFG_CSI_PORT mipiPort, MUINT8 enable)
{
    csi0_dphy_top_reg_REGS *pCsi0_Dphy_Top = (csi0_dphy_top_reg_REGS *)mpCSIDphyTopAddr[mipiPort]; //11c8 2000

#ifdef HAL_CSI2RX_HW_INITIAL_DESKEW
    // hw_deskew_cfg
    //int enable = 1;
    int code_unit_sel = 0;  // 1: for 6.5 Gbps
    int setup_cnt = 3;
    int hold_cnt = 3;
    int detect_cnt = 0xf;
    int cmplength = 3; // 4'b0011: 64-bits

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_CTRL, rg_dphy_rx_deskew_code_unit_sel) = code_unit_sel;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_TIMING_CTRL, rg_dphy_rx_deskew_setup_cnt) = setup_cnt; //11c8 2054
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_TIMING_CTRL, rg_dphy_rx_deskew_hold_cnt) = hold_cnt;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_TIMING_CTRL, rg_dphy_rx_deskew_detect_cnt) = detect_cnt;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_TIMING_CTRL, rg_dphy_rx_deskew_cmplength) = cmplength;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_CTRL, rg_dphy_rx_deskew_en) = enable; //11c8 2050[8], enable or disable hw deskew

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE0_CTRL, dphy_rx_deskew_l0_delay_en) = 1;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE1_CTRL, dphy_rx_deskew_l1_delay_en) = 1;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE2_CTRL, dphy_rx_deskew_l2_delay_en) = 1;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE3_CTRL, dphy_rx_deskew_l3_delay_en) = 1;
#endif

#ifdef HAL_CSI2RX_HW_PERIODIC_DESKEW
    //hal_csi2rx_dphy_hw_periodic_deskew_cfg, to configure dphy deskew delay perdoic
    //int enable = 1;
    int one_dir_only = 1;
    int delay_code_cnt = 7;

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE0_CTRL, rg_periodic_deskew_1dir_only) = one_dir_only;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE1_CTRL, rg_periodic_deskew_1dir_only) = one_dir_only;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE2_CTRL, rg_periodic_deskew_1dir_only) = one_dir_only;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE3_CTRL, rg_periodic_deskew_1dir_only) = one_dir_only;

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE0_CTRL, rg_dphy_rx_deskew_l0_periodic_delay_code_cnt) = delay_code_cnt;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE1_CTRL, rg_dphy_rx_deskew_l1_periodic_delay_code_cnt) = delay_code_cnt;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE2_CTRL, rg_dphy_rx_deskew_l2_periodic_delay_code_cnt) = delay_code_cnt;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE3_CTRL, rg_dphy_rx_deskew_l3_periodic_delay_code_cnt) = delay_code_cnt;

    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE0_CTRL, rg_dphy_rx_deskew_l0_periodic) = enable;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE1_CTRL, rg_dphy_rx_deskew_l1_periodic) = enable;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE2_CTRL, rg_dphy_rx_deskew_l2_periodic) = enable;
    SENINF_BITS(pCsi0_Dphy_Top, DPHY_RX_DESKEW_LANE3_CTRL, rg_dphy_rx_deskew_l3_periodic) = enable;

#endif

    return 0;
}
#endif
