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
#include <mtkcam/utils/std/ULog.h>
#include "kd_seninf.h"
#include "seninf_reg.h"
#include "iseninf_drv.h"
#include "seninf_drv.h"
#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    CAM_ULOGMD("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    CAM_ULOGME("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
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

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

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
#ifdef CONFIG_MTK_CAM_SECURE
SeninfDrv*
SeninfDrv::
getInstance(MBOOL isSecure)
{
    return SeninfDrvImp::createInstance();
}
#endif

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
    LOG_MSG("[SeninfDrv]");
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
    LOG_MSG("[~SeninfDrv]");
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::init()
{
    MBOOL result;
    KD_SENINF_REG reg;
    LOG_MSG("[init]: Entry count %d", mUser);
    Mutex::Autolock lock(mLock);
    if (mUser > 0) {
        LOG_MSG("Already inited");
        android_atomic_inc(&mUser);
        return 0;
    }
    // Open seninf driver
    mfd = open(SENINF_DEV_NAME, O_RDWR);
    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s", errno, strerror(errno));
        return -1;
    }
    if (ioctl(mfd, KDSENINFIOC_X_GET_REG_ADDR, &reg) < 0) {
       LOG_ERR("ERROR:KDSENINFIOC_X_GET_REG_ADDR");
       return -2;
    }
    // mmap seninf reg
    mpSeninfHwRegAddr = (unsigned char *) mmap(0, reg.seninf.map_length, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, reg.seninf.map_addr);
    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s", errno, strerror(errno));
        return -5;
    }
    // mipi rx analog address
    mpCSI2RxAnalogRegStartAddrAlign = (unsigned char *) mmap(0, reg.ana.map_length, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, reg.ana.map_addr);
    if (mpCSI2RxAnalogRegStartAddrAlign == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s", errno, strerror(errno));
        return -9;
    }
    /*CSI2 Base address*/
    //MMAP only support Page alignment(0x1000)
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0]  = mpCSI2RxAnalogRegStartAddrAlign;
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0A] = mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0];
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0B] = mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0] + 0x1000;
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1]  = mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0] + 0x2000;
#if 0
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1A] = mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0] + 0x2000;
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1B] = mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0] + 0x3000;
#endif
    mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2]  = mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0] + 0x4000;
    /*SenINF base address*/
    mpSeninfCtrlRegAddr[SENINF_1] = mpSeninfHwRegAddr;
    mpSeninfCtrlRegAddr[SENINF_2] = mpSeninfHwRegAddr + 0x1000;
    mpSeninfCtrlRegAddr[SENINF_3] = mpSeninfHwRegAddr + 0x2000;
    mpSeninfCtrlRegAddr[SENINF_4] = mpSeninfHwRegAddr + 0x3000;
    mpSeninfCtrlRegAddr[SENINF_5] = mpSeninfHwRegAddr + 0x4000;
    /*SenINF Mux Base address*/
    mpSeninfMuxBaseAddr[SENINF_MUX1] = mpSeninfHwRegAddr + 0x0d00;
    mpSeninfMuxBaseAddr[SENINF_MUX2] = mpSeninfHwRegAddr + 0x1d00;
    mpSeninfMuxBaseAddr[SENINF_MUX3] = mpSeninfHwRegAddr + 0x2d00;
    mpSeninfMuxBaseAddr[SENINF_MUX4] = mpSeninfHwRegAddr + 0x3d00;
    mpSeninfMuxBaseAddr[SENINF_MUX5] = mpSeninfHwRegAddr + 0x4d00;
    mpSeninfMuxBaseAddr[SENINF_MUX6] = mpSeninfHwRegAddr + 0x5d00;
    mpSeninfCSIRxConfBaseAddr[SENINF_1] = mpSeninfHwRegAddr + 0x0824;//18040800
    mpSeninfCSIRxConfBaseAddr[SENINF_2] = mpSeninfHwRegAddr + 0x1824;//18041800
    mpSeninfCSIRxConfBaseAddr[SENINF_3] = mpSeninfHwRegAddr + 0x2824;//18042800
    mpSeninfCSIRxConfBaseAddr[SENINF_4] = mpSeninfHwRegAddr + 0x3824;//18043800
    mpSeninfCSIRxConfBaseAddr[SENINF_5] = mpSeninfHwRegAddr + 0x4824;//18044800
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
    unsigned int i = 0;
    DEVINFO_S devinfo;
    fd = open("/proc/device-tree/chosen/atag,devinfo", O_RDONLY); /* v2 device node */
    if (fd < 0) { /* Use v2 device node if v1 device node is removed */
        LOG_ERR("/proc/device-tree/chosen/atag,devinfo kernel open fail, errno(%d):%s",errno,strerror(errno));
    } else {
        ret = read(fd, (void *)&devinfo, sizeof(DEVINFO_S));
        if (ret < 0) {
            LOG_ERR("Get Devinfo data fail, errno(%d):%s",errno,strerror(errno));
        } else {
            mCSI[0] = devinfo.data[103];//0x11F1018C
            mCSI[1] = devinfo.data[104];//0x11F10190
            mCSI[2] = devinfo.data[115];//0x11F101BC
            mCSI[3] = devinfo.data[116];//0x11F101C0
        }
        LOG_MSG("Efuse Data:0x1045018c= 0x%x, 0x10450190= 0x%x, 0x104501bc= 0x%x, 0x104501c0= 0x%x", mCSI[0], mCSI[1], mCSI[2], mCSI[3]);
        close(fd);
    }
#endif
    android_atomic_inc(&mUser);
    LOG_MSG("[init]: Exit count %d", mUser);
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::uninit()
{
    KD_SENINF_REG reg;
    unsigned int temp = 0;
    LOG_MSG("[uninit]: %d", mUser);
    Mutex::Autolock lock(mLock);
    if (mUser <= 0) {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUser);
    if (mUser == 0) {
        if (ioctl(mfd, KDSENINFIOC_X_GET_REG_ADDR, &reg) < 0) {
           LOG_ERR("ERROR:KDSENINFIOC_X_GET_REG_ADDR");
        }
        if ( 0 != mpSeninfHwRegAddr ) {
            if(munmap(mpSeninfHwRegAddr, reg.seninf.map_length) != 0) {
                LOG_ERR("mpSeninfHwRegAddr munmap err, %d, %s", errno, strerror(errno));
            }
            mpSeninfHwRegAddr = NULL;
        }
        if ( 0 != mpCSI2RxAnalogRegStartAddrAlign ) {
            if(munmap(mpCSI2RxAnalogRegStartAddrAlign, reg.ana.map_length) != 0){
                LOG_ERR("mpCSI2RxAnalogRegStartAddr munmap err, %d, %s", errno, strerror(errno));
            }
            mpCSI2RxAnalogRegStartAddrAlign = NULL;
        }
        LOG_MSG("[uninit]: %d, mfd(%d)", mUser, mfd);
        if (mfd > 0) {
            close(mfd);
            mfd = 0;
        }
    }
    else {
        LOG_ERR("Still users");
    }
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::configMclk(SENINF_MCLK_PARA *pmclk_para, unsigned long pcEn)
{
    MINT32 ret = 0;

    std::string str_prop("debug.seninf.Tg");
    str_prop += std::to_string(pmclk_para->sensorIdx);
    str_prop +="clk";

    LOG_MSG("[Tg%dclk]: pcEn=%lu freq=%d", pmclk_para->sensorIdx, pcEn, pmclk_para->mclkFreq);
    /*get property*/
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(str_prop.c_str(), value, "-1");
    int tgClk = atoi(value);
    if(tgClk > 0) {
        pmclk_para->mclkFreq = tgClk;
        LOG_MSG("setproperty Tg%dclk: %d", pmclk_para->sensorIdx, pmclk_para->mclkFreq);
    }
    ret = setMclk(pmclk_para->mclkIdx, pcEn, pmclk_para->mclkFreq);
    if (ret < 0) {
        LOG_MSG("setMclk fail");
        return ret;
    }
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk(MUINT8 mclkIdx, MBOOL pcEn, unsigned long freq)
{
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    sensorMclk.on   = pcEn;
    sensorMclk.freq = freq;
    sensorMclk.TG   = mclkIdx;
    /* Configure timestamp */
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) =
    SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = 1;
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_TM_STP, SENINF_TIMESTAMP_STEP);
    if (ioctl(mfd, KDSENINFIOC_X_SET_MCLK_PLL, &sensorMclk) < 0) {
       LOG_ERR("ERROR:KDSENINFIOC_X_SET_MCLK_PLL");
    }
    LOG_MSG("[setTg%dPhase]pcEn(%d), freq(%d)", mclkIdx, (MINT32)pcEn, (MINT32)freq);
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
#define SENINF_DRV_DEBUG_DELAY 1000
#ifdef CONFIG_MTK_CAM_SECURE
int SeninfDrvImp::debug(int isSecure)
#else
int SeninfDrvImp::debug()
#endif
{
    int ret = 0;
    unsigned int mmsys_clk = 0;
#ifdef CONFIG_MTK_CAM_SECURE_I2C
    int ret_sec = 0;
#endif
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    static int mipi_packet_cnt[SENINF_NUM] = {0};
    mmsys_clk = 4;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcam_ck(%d)", mmsys_clk);
    mmsys_clk = 35;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fseninf_ck(%d)", mmsys_clk);
    mmsys_clk = 11;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg_ck(%d)", mmsys_clk);
    mmsys_clk = 12;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg2_ck(%d)", mmsys_clk);
    mmsys_clk = 13;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg3_ck(%d)", mmsys_clk);
    mmsys_clk = 14;
    ret = ioctl(mfd, KDSENINFIOC_X_GET_CSI_CLK, &mmsys_clk);
    LOG_MSG("f_fcamtg4_ck(%d)", mmsys_clk);
#ifdef CONFIG_MTK_CAM_SECURE_I2C
    if(isSecure) {
        ret_sec = ioctl(mfd, KDSENINFIOC_X_SECURE_DUMP);
        LOG_MSG("DEBUG:IOCTL_SECURE_DUMP = %d", ret_sec);
    }
#endif
    SENINF_BITS(pSeninf, SENINF1_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF1_MUX_SPARE, SENINF_SPARE) & 0x37;
    SENINF_BITS(pSeninf, SENINF2_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF2_MUX_SPARE, SENINF_SPARE) & 0x37;
    SENINF_BITS(pSeninf, SENINF3_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF3_MUX_SPARE, SENINF_SPARE) & 0x37;
    SENINF_BITS(pSeninf, SENINF4_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF4_MUX_SPARE, SENINF_SPARE) & 0x37;
    SENINF_BITS(pSeninf, SENINF5_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF5_MUX_SPARE, SENINF_SPARE) & 0x37;
    SENINF_BITS(pSeninf, SENINF6_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF6_MUX_SPARE, SENINF_SPARE) & 0x37;
    usleep(SENINF_DRV_DEBUG_DELAY);
    LOG_MSG("SENINF_TOP_MUX_CTRL(0x%x) SENINF_TOP_CAM_MUX_CTRL(0x%x)",
        SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF_TOP_CAM_MUX_CTRL));
    /* sync error, crc error, frame end not sync */
    SENINF_WRITE_REG(pSeninf,SENINF1_CSI2_INT_STATUS,0xffffffff);
    usleep(SENINF_DRV_DEBUG_DELAY);
    LOG_MSG("SENINF1_CSI2_INT_STATUS(0x%x), CLR SENINF1_CSI2_INT_STATUS(0x%x)",
        SENINF_READ_REG(pSeninf, SENINF1_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS));
    /* sync error, crc error, frame end not sync */
    SENINF_WRITE_REG(pSeninf,SENINF2_CSI2_INT_STATUS,0xffffffff);
    usleep(SENINF_DRV_DEBUG_DELAY);
    LOG_MSG("SENINF2_CSI2_INT_STATUS(0x%x), CLR SENINF2_CSI2_INT_STATUS(0x%x)",
        SENINF_READ_REG(pSeninf, SENINF2_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS));
    /* sync error, crc error, frame end not sync */
    SENINF_WRITE_REG(pSeninf,SENINF3_CSI2_INT_STATUS,0xffffffff);
    usleep(SENINF_DRV_DEBUG_DELAY);
    LOG_MSG("SENINF3_CSI2_INT_STATUS(0x%x), CLR SENINF3_CSI2_INT_STATUS(0x%x)",
        SENINF_READ_REG(pSeninf, SENINF3_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS));
    /* sync error, crc error, frame end not sync */
    SENINF_WRITE_REG(pSeninf,SENINF5_CSI2_INT_STATUS,0xffffffff);
    usleep(SENINF_DRV_DEBUG_DELAY);
    LOG_MSG("SENINF5_CSI2_INT_STATUS(0x%x), CLR SENINF5_CSI2_INT_STATUS(0x%x)",
        SENINF_READ_REG(pSeninf, SENINF5_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS));
    /*Sensor Interface Control */
     LOG_MSG("SENINF1_CSI2_CTL(0x%x), SENINF2_CSI2_CTL(0x%x), SENINF3_CSI2_CTL(0x%x), SENINF5_CSI2_CTL(0x%x)",
         SENINF_READ_REG(pSeninf,SENINF1_CSI2_CTL), SENINF_READ_REG(pSeninf,SENINF2_CSI2_CTL), SENINF_READ_REG(pSeninf,SENINF3_CSI2_CTL), SENINF_READ_REG(pSeninf,SENINF5_CSI2_CTL));
    for(int i = 0; i < 3; i++) {
        mipi_packet_cnt[SENINF_1] = SENINF_READ_REG(pSeninf,SENINF1_CSI2_DBG_PORT);
        mipi_packet_cnt[SENINF_2] = SENINF_READ_REG(pSeninf,SENINF2_CSI2_DBG_PORT);
        mipi_packet_cnt[SENINF_3] = SENINF_READ_REG(pSeninf,SENINF3_CSI2_DBG_PORT);
        mipi_packet_cnt[SENINF_5] = SENINF_READ_REG(pSeninf,SENINF5_CSI2_DBG_PORT);
        LOG_MSG("SENINF1_PkCnt(0x%x), SENINF2_PkCnt(0x%x), SENINF3_PkCnt(0x%x), SENINF5_PkCnt(0x%x)",
            mipi_packet_cnt[SENINF_1], mipi_packet_cnt[SENINF_2], mipi_packet_cnt[SENINF_3], mipi_packet_cnt[SENINF_5]);
        usleep(SENINF_DRV_DEBUG_DELAY);
        /*Sensor Interface IRQ */
        if (SENINF_READ_REG(pSeninf,SENINF1_CSI2_CTL) & 0x1) {
            if (SENINF_READ_REG(pSeninf,SENINF1_CSI2_DBG_PORT) == (unsigned int)mipi_packet_cnt[SENINF_1]){
                ret = -1;
            } else if ((SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS) & 0xb0)!= 0){
                ret = -2;
            }
        }
        if (SENINF_READ_REG(pSeninf,SENINF2_CSI2_CTL) & 0x1) {
            if (SENINF_READ_REG(pSeninf,SENINF2_CSI2_DBG_PORT) == (unsigned int)mipi_packet_cnt[SENINF_2]){
                ret = -1;
            } else if ((SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS) & 0xb0)!= 0){
                ret = -2;
            }
        }
        if (SENINF_READ_REG(pSeninf,SENINF3_CSI2_CTL) & 0x1) {
            if (SENINF_READ_REG(pSeninf,SENINF3_CSI2_DBG_PORT) == (unsigned int)mipi_packet_cnt[SENINF_3]) {
                ret = -1;
            } else if ((SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS) & 0xb0)!= 0) {
                ret = -2;
            }
        }
        if (SENINF_READ_REG(pSeninf,SENINF5_CSI2_CTL) & 0x1){
            if (SENINF_READ_REG(pSeninf,SENINF5_CSI2_DBG_PORT) == (unsigned int)mipi_packet_cnt[SENINF_5]){
                ret = -1;
            } else if ((SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS) & 0xb0)!= 0) {
                ret = -2;
            }
        }
    }
    LOG_MSG("SENINF1_IRQ(0x%x), SENINF2_IRQ(0x%x), SENINF3_IRQ(0x%x), SENINF5_IRQ(0x%x), EXT_IRQ(1:0x%x, 2:0x%x, 3:0x%x, 4:0x%x)",
        SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS), SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS),
        SENINF_READ_REG(pSeninf,SENINF1_CSI2_INT_STATUS_EXT), SENINF_READ_REG(pSeninf,SENINF2_CSI2_INT_STATUS_EXT), SENINF_READ_REG(pSeninf,SENINF3_CSI2_INT_STATUS_EXT), SENINF_READ_REG(pSeninf,SENINF5_CSI2_INT_STATUS_EXT));
    /*Mux1 */
    LOG_MSG("SENINF1_MUX_CTRL(0x%x), SENINF1_MUX_INTSTA(0x%x), SENINF1_MUX_DEBUG_2(0x%x)",
         SENINF_READ_REG(pSeninf,SENINF1_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF1_MUX_DEBUG_2));
    if(SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA) & 0x1)
    {
        SENINF_WRITE_REG(pSeninf,SENINF1_MUX_INTSTA,0xffffffff);
        usleep(SENINF_DRV_DEBUG_DELAY);
        LOG_MSG("after reset overrun, SENINF1_MUX_CTRL(0x%x), SENINF1_MUX_INTSTA(0x%x), SENINF1_MUX_DEBUG_2(0x%x)",
            SENINF_READ_REG(pSeninf,SENINF1_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF1_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF1_MUX_DEBUG_2));
    }
    /*Mux2 */
    LOG_MSG("SENINF2_MUX_CTRL(0x%x), SENINF2_MUX_INTSTA(0x%x), SENINF2_MUX_DEBUG_2(0x%x)",
         SENINF_READ_REG(pSeninf,SENINF2_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF2_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF2_MUX_DEBUG_2));
    if(SENINF_READ_REG(pSeninf,SENINF2_MUX_INTSTA) & 0x1)
    {
        SENINF_WRITE_REG(pSeninf,SENINF2_MUX_INTSTA,0xffffffff);
        usleep(SENINF_DRV_DEBUG_DELAY);
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
    /*Mux5 for HDR*/
    LOG_MSG("SENINF5_MUX_CTRL(0x%x), SENINF5_MUX_INTSTA(0x%x), SENINF5_MUX_DEBUG_2(0x%x)",
         SENINF_READ_REG(pSeninf,SENINF5_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF5_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF5_MUX_DEBUG_2));
    SENINF_WRITE_REG(pSeninf,SENINF5_MUX_INTSTA,0xffffffff);
    /*Mux6  for PDAF*/
    LOG_MSG("SENINF6_MUX_CTRL(0x%x), SENINF6_MUX_INTSTA(0x%x), SENINF6_MUX_DEBUG_2(0x%x)",
         SENINF_READ_REG(pSeninf,SENINF6_MUX_CTRL), SENINF_READ_REG(pSeninf,SENINF6_MUX_INTSTA), SENINF_READ_REG(pSeninf,SENINF6_MUX_DEBUG_2));
    SENINF_WRITE_REG(pSeninf,SENINF6_MUX_INTSTA,0xffffffff);
    SENINF_BITS(pSeninf, SENINF1_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF1_MUX_SPARE, SENINF_SPARE) | 0x8;
    SENINF_BITS(pSeninf, SENINF2_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF2_MUX_SPARE, SENINF_SPARE) | 0x8;
    SENINF_BITS(pSeninf, SENINF3_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF3_MUX_SPARE, SENINF_SPARE) | 0x8;
    SENINF_BITS(pSeninf, SENINF4_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF4_MUX_SPARE, SENINF_SPARE) | 0x8;
    SENINF_BITS(pSeninf, SENINF5_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF5_MUX_SPARE, SENINF_SPARE) | 0x8;
    SENINF_BITS(pSeninf, SENINF6_MUX_SPARE, SENINF_SPARE) = SENINF_BITS(pSeninf, SENINF6_MUX_SPARE, SENINF_SPARE) | 0x8;
    LOG_MSG("ret = %d", ret);
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::sendCommand(int cmd, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    int ret = 0;
    (void)arg2;
    (void)arg3;
    Mutex::Autolock lock(mLock);//for uninit, some pointer will be set to NULL
    if (mpSeninfHwRegAddr == NULL) {
        LOG_ERR("mpSeninfHwRegAddr = NULL, seninf has been uninit, stop dump to avoid NE");
        return ret;
    }
    switch (cmd) {
    case CMD_SENINF_DEBUG_TASK:
    case CMD_SENINF_DEBUG_TASK_CAMSV:
#ifdef CONFIG_MTK_CAM_SECURE
        ret = debug(arg1);
#else
        ret = debug();
#endif
        break;
    case CMD_SENINF_GET_SENINF_ADDR:
        *(unsigned long *) arg1 = (unsigned long) mpSeninfHwRegAddr;
        break;
    case CMD_SENINF_DEBUG_PIXEL_METER:
    {
        unsigned int * meter_array = (unsigned int *)arg1;
        if(meter_array != NULL) {
            *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x3C))|= 1<<31;
                meter_array[0] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x40));
                meter_array[1] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x44));
                meter_array[2] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x48));
                meter_array[3] = *((unsigned int *)(mpSeninfMuxBaseAddr[SENINF_MUX1]+0x4c));
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
    SENINF_MUX_ENUM seninfMuXIdx, SENINF_ENUM seninfSrc
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    SENINF_WRITE_REG(pSeninf, SENINF_TOP_MUX_CTRL, (SENINF_READ_REG(pSeninf, SENINF_TOP_MUX_CTRL)&(~(0xF<<(seninfMuXIdx * 4)))|(seninfSrc&0xF)<<(seninfMuXIdx * 4)));
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfCamTGMuxCtrl(
    unsigned int targetCamTG, SENINF_MUX_ENUM muxSrc
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    SENINF_WRITE_REG(pSeninf, SENINF_TOP_CAM_MUX_CTRL, (SENINF_READ_REG(pSeninf, SENINF_TOP_CAM_MUX_CTRL)&(~(0xF<<(targetCamTG * 4)))|(muxSrc&0xF)<<(targetCamTG * 4)));
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::getSeninfTopMuxCtrl(SENINF_MUX_ENUM mux)
{
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    return (SENINF_READ_REG(pSeninf, SENINF_TOP_MUX_CTRL) & (0xF<<(mux * 4)))>>(mux * 4);
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::getSeninfCamTGMuxCtrl(unsigned int targetCam)
{
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    return (SENINF_READ_REG(pSeninf, SENINF_TOP_CAM_MUX_CTRL) & (0xF<<(targetCam * 4)))>>(targetCam * 4);
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfVC(SENINF_ENUM SenInfsel,
    unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id,
    unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id)
{
    int ret = 0;
    seninf_reg_t *pSeninf = NULL;
    pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr [SenInfsel];
    /* General Long Packet Data Types: 0x10-0x17 */
    if ((0x10 <= (vc0Id>>2) && (vc0Id>>2) <= 0x17) ||
        (0x10 <= (vc1Id>>2) && (vc1Id>>2) <= 0x17) ||
        (0x10 <= (vc2Id>>2) && (vc2Id>>2) <= 0x17) ||
        (0x10 <= (vc3Id>>2) && (vc3Id>>2) <= 0x17) ||
        (0x10 <= (vc4Id>>2) && (vc4Id>>2) <= 0x17) ||
        (0x10 <= (vc5Id>>2) && (vc5Id>>2) <= 0x17))
            SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, GENERIC_LONG_PACKET_EN) = 1;
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
    LOG_MSG("VC Data(0x%x,0x%x), VC Ctrl(0x%x,0x%x)",
            SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI), SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI_EXT),
            SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI_CTRL),SENINF_READ_REG(pSeninf,SENINF1_CSI2_DI_CTRL_EXT));
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
    seninf_mux_reg_t_base *pSeninf = (seninf_mux_reg_t_base *)mpSeninfMuxBaseAddr[mux];
    unsigned int temp = 0;
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
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL SeninfDrvImp::isMUXUsed(SENINF_MUX_ENUM mux)
{
    seninf_mux_reg_t_base *pSeninf = (seninf_mux_reg_t_base *)mpSeninfMuxBaseAddr[mux];
    return SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN);
}
int SeninfDrvImp::enableMUX(SENINF_MUX_ENUM mux)
{
    seninf_mux_reg_t_base *pSeninf = (seninf_mux_reg_t_base *)mpSeninfMuxBaseAddr[mux];
    SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN) = 1;
    return 0;
}
int SeninfDrvImp::disableMUX(SENINF_MUX_ENUM mux)
{
    seninf_mux_reg_t_base *pSeninf = (seninf_mux_reg_t_base *)mpSeninfMuxBaseAddr[mux];
    SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN) = 0;
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::Efuse(unsigned long csi_sel)
{
    int ret = 0;
    if ((mCSI[0] != 0) && (csi_sel == 0)) {
        mipi_ana_dphy_reg_t *pMipirx = (mipi_ana_dphy_reg_t *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0];
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L0P_HSRT_CODE) = (mCSI[0]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L0N_HSRT_CODE) = (mCSI[0]>>27) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L1P_HSRT_CODE) = (mCSI[0]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1A, RG_CSI1A_L1N_HSRT_CODE) = (mCSI[0]>>22) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1A, RG_CSI1A_L2P_HSRT_CODE) = (mCSI[0]>>17) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1A, RG_CSI1A_L2N_HSRT_CODE) = (mCSI[0]>>17) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L0P_HSRT_CODE) = (mCSI[0]>>12) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L0N_HSRT_CODE) = (mCSI[0]>>12) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L1P_HSRT_CODE) = (mCSI[0]>>7) & 0x1f;
        SENINF_BITS(pMipirx, MIPI_RX_ANA08_CSI1B, RG_CSI1B_L1N_HSRT_CODE) = (mCSI[0]>>7) & 0x1f;
//        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2P_HSRT_CODE) = (mCSI[0]>>2) & 0x1f;
//        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2N_HSRT_CODE) = (mCSI[0]>>2) & 0x1f;
        LOG_MSG("CSI0 MIPI_RX_ANA08_CSI0A(0x%x) MIPI_RX_ANA0C_CSI0A(0x%x) MIPI_RX_ANA08_CSI0B(0x%x) MIPI_RX_ANA0C_CSI0B(0x%x)", SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1A),
            SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1A), SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1B), SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1B));
    }
    if ((mCSI[1] != 0)&&(mCSI[2] != 0) && (csi_sel == 1)) {
        mipi_ana_dphy_reg_t *pMipirx = (mipi_ana_dphy_reg_t *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_1];
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
//        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2P_HSRT_CODE) = (mCSI[2]>>17) & 0x1f;
//        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI1B_L2N_HSRT_CODE) = (mCSI[2]>>17) & 0x1f;
        LOG_MSG("CSI1 ANA08_CSI1A(0x%x) ANA0C_CSI1A(0x%x) ANA08_CSI1B(0x%x) ANA0C_CSI1B(0x%x)", SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1A),
            SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1A), SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1B), SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1B));
    }
    if((mCSI[2] != 0)&&(mCSI[3] != 0) && (csi_sel == 2))
    {
        mipi_ana_dphy_reg_t *pMipirx = (mipi_ana_dphy_reg_t *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_2];
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
//        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI2B_L2P_HSRT_CODE) = (mCSI[3]>>20) & 0x1f;
//        SENINF_BITS(pMipirx, MIPI_RX_ANA0C_CSI1B, RG_CSI2B_L2N_HSRT_CODE) = (mCSI[3]>>20) & 0x1f;
        LOG_MSG("CSI2 ANA08_CSI2A(0x%x) ANA0C_CSI2A(0x%x) ANA08_CSI2B(0x%x) ANA0C_CSI2B(0x%x)", SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1A),
                   SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1A), SENINF_READ_REG(pMipirx, MIPI_RX_ANA08_CSI1B), SENINF_READ_REG(pMipirx, MIPI_RX_ANA0C_CSI1B));
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
int SeninfDrvImp::setTestModel(bool en, unsigned int dummypxl, unsigned int vsync, unsigned int line,unsigned int pxl)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
#if 1
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, 0x1001);
    SENINF_WRITE_REG(pSeninf, SENINF1_CTRL_EXT, 0x2);
    SENINF_WRITE_REG(pSeninf, SENINF_TOP_CTRL, 0);
    SENINF_WRITE_REG(pSeninf, SENINF_TOP_CMODEL_PAR, 1);
    SENINF_WRITE_REG(pSeninf, SENINF_TOP_MUX_CTRL, 0x3210);
    SENINF_WRITE_REG(pSeninf, SENINF_TOP_CAM_MUX_CTRL, 0x76000000);
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_TM_SIZE, 0x10001f00);
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_TM_CLK, 8);
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_TM_STP, 0);
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_TM_CTL, 0xff01c5);
    SENINF_WRITE_REG(pSeninf, SENINF1_MUX_CTRL, 0x86df1080);
#else
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = en;
    SENINF_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL) = PAD_10BIT;
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = TEST_MODEL;
    SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = 0;
    SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_NCSI2_IP_EN) = 1;
    SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_TESTMDL_IP_EN) = en;
    SENINF_WRITE_REG(pSeninf, SENINF_TG1_TM_CTL, 0x00);
    SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_EN) = en;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_VSYNC) = vsync;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_DUMMYPXL) = dummypxl;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_PAT) = 0xC;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_FMT) = 0;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_RST) = 0;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_SIZE, TM_PXL) = pxl;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_SIZE, TM_LINE) = line;
    SENINF_BITS(pSeninf, SENINF_TG1_TM_CLK, TM_CLK_CNT) = 0x8;
#endif
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
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
    seninf_reg_t *pSeninf = NULL;//(seninf_reg_t *)mpSeninfHwRegAddr;
    seninf_reg_t *pSeninf_base=(seninf_reg_t *)mpSeninfHwRegAddr;
    mipi_ana_reg_t *pMipirx = NULL;//(mipi_ana_reg_t_base *)mpCSI2RxAnalog0RegAddr;
    mipi_ana_reg_t *pMipiRx_base = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_0];
    mipi_rx_con_reg_base *pMipiRxConf = NULL;
    SENINF_CSI_INFO *pCsiInfo = pCsiMipi->pCsiInfo;
    unsigned int CalSel;
    pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[pCsiInfo->port];
    pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pCsiInfo->seninf];
    pMipiRxConf = (mipi_rx_con_reg_base *)mpSeninfCSIRxConfBaseAddr[pCsiInfo->seninf];
    if(pCsiInfo->port == CUSTOM_CFG_CSI_PORT_1) {
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, phy_seninf_lane_mux_csi1_en) = pCsiMipi->enable;
        CalSel = 1;
    } else if(pCsiInfo->port == CUSTOM_CFG_CSI_PORT_2) {
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI2, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI2, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI2, phy_seninf_lane_mux_csi2_en) = pCsiMipi->enable;
        CalSel = 2;
    } else if(pCsiInfo->port == CUSTOM_CFG_CSI_PORT_0) {
        CalSel = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = pCsiMipi->enable;
    } else if(pCsiInfo->port == CUSTOM_CFG_CSI_PORT_0A) {
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = pCsiMipi->enable;
        CalSel = 0;
    } else if(pCsiInfo->port == CUSTOM_CFG_CSI_PORT_0B) {
        CalSel = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = pCsiMipi->enable;
#if 0
    } else if(pCsiInfo->port == CUSTOM_CFG_CSI_PORT_1A) {
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, phy_seninf_lane_mux_csi1_en) = pCsiMipi->enable;
        CalSel = 1;
    } else if(pCsiInfo->port == CUSTOM_CFG_CSI_PORT_1B) {
        CalSel = 1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, DPHY_MODE) = 0x1;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_1) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, CK_SEL_2) = 0x1;//2d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI1, phy_seninf_lane_mux_csi1_en) = pCsiMipi->enable;
#endif
    } else {
        LOG_ERR("unsupported CSI configuration");//should never here, handle as SENINF_CSI2_IP_0
        CalSel = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, DPHY_MODE) = 0;
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, CK_SEL_1) = 2;//4d1c
        SENINF_BITS(pSeninf_base, SENINF_TOP_PHY_SENINF_CTL_CSI0, phy_seninf_lane_mux_csi0_en) = pCsiMipi->enable;
    }
    /*First Enable Sensor interface and select pad (0x1a04_0200)*/
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = pCsiMipi->enable;//(SENINF_CTL_BASE) |= 0x1;
    SENINF_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL) = pCsiMipi->padSel;
    if((pCsiMipi->csi_type == CSI2_1_5G) || (pCsiMipi->csi_type == CSI2_2_5G)) {// For CSI2(2.5G) support  & CPHY Support
        SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = 0; //fix for non-mipi sensor 1 ->test mode, 3 parallel sesnor
        SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_CSI2_IP_EN) = pCsiMipi->enable;//(SENINF_CTL_BASE+0x04) |= 0x40
        SENINF_BITS(pSeninf, SENINF1_CTRL_EXT, SENINF_NCSI2_IP_EN) = 0;
    }
    if(!pCsiMipi->enable) {
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL, SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL) & 0xFFFFFFE0); // disable CSI2(2.5G) first
        //disable mipi BG
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_LPF_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_LPF_EN) = 0;
        return ret;
    }
    if(pCsiMipi->csi_type != CSI2_2_5G_CPHY) //Dphy
    {
        /* set analog phy mode to DPHY */
        if(IS_CDPHY_COMBO)
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_CPHY_EN) = 0;
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
        if(IS_CDPHY_COMBO)
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_CPHY_EN) = 0;
        if (IS_4D1C) {//only 4d1c need set CSIB (MIPIRX_ANALOG_B_BASE) = 0x00001242
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L0_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L0_CKSEL) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L1_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L1_CKSEL) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L2_CKMODE_EN) = 0;
            SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L2_CKSEL) = 1;
        }

        /* byte clock invert*/
        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_CDPHY_L0_T0_BYTECK_INVERT) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_DPHY_L1_BYTECK_INVERT) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_CDPHY_L2_T1_BYTECK_INVERT) = 1;
        if (IS_4D1C) {
            SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_CDPHY_L0_T0_BYTECK_INVERT) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_DPHY_L1_BYTECK_INVERT) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_CDPHY_L2_T1_BYTECK_INVERT) = 1;
        }
        /*start ANA EQ tuning*/
        if(IS_CDPHY_COMBO) {
            SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_L0_T0AB_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
            SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_L0_T0AB_EQ_BW) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_L1_T1AB_EQ_IS) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_L1_T1AB_EQ_BW) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_L2_T1BC_EQ_IS) = 1;
            SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_L2_T1BC_EQ_BW) = 1;
#if 0 //cphy register
            if((pCsiMipi->CSI2_IP == SENINF_CSI2_IP_0A)||pCsiMipi->CSI2_IP == SENINF_CSI2_IP_0B||(pCsiMipi->CSI2_IP == SENINF_CSI2_IP_0)){
                SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_XX_T0BC_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
                SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_XX_T0BC_EQ_BW) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_XX_T0CA_EQ_IS) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_XX_T0CA_EQ_BW) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_XX_T1CA_EQ_IS) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_XX_T1CA_EQ_BW) = 1;
            }
#endif
            if (IS_4D1C) {//4d1c
                SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_L0_T0AB_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
                SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_L0_T0AB_EQ_BW) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_L1_T1AB_EQ_IS) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_L1_T1AB_EQ_BW) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_L2_T1BC_EQ_IS) = 1;
                SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_L2_T1BC_EQ_BW) = 1;
#if 0
                if (pCsiMipi->CSI2_IP == SENINF_CSI2_IP_0) {
                    SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_XX_T1CA_EQ_IS) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_XX_T1CA_EQ_BW) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_XX_T0CA_EQ_IS) = 1;/*EQ Power to Enhance Speed*/
                    SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_XX_T0CA_EQ_BW) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_XX_T0BC_EQ_IS) = 1;
                    SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_XX_T0BC_EQ_BW) = 1;
                }
#endif
            }
        }else {
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
        Efuse(CalSel);
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
        LOG_MSG("CSI offset calibration start");
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
        int status = 0;
        int i = 0;
        while(1) {
            status = SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0A);
            if((IS_CDPHY_COMBO)&&
                (status&(1<<0))&&(status&(1<<3))&&(status&(1<<5)))
                break;
            else if((status&(1<<3))&&(status&(1<<4))&&(status&(1<<5)))//CSI1,CSI2 completed status bits located at different offset
                break;
            else
                LOG_MSG("CSIA offset calibration ongoing");
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
                if((IS_CDPHY_COMBO)&&
                    (status&(1<<0))&&(status&(1<<3))&&(status&(1<<5)))
                    break;
                else if((status&(1<<3))&&(status&(1<<4))&&(status&(1<<5)))
                    break;
                else
                   LOG_MSG("CSIB offset calibration ongoing");
                i++;
                if(i>100) {
                    LOG_MSG("CSIB offset calibration timeout");
                    break;
                }
                usleep(30);
            }
        }
        LOG_MSG("CSI offset calibration end MIPI_RX_ANA48_CSI0A 0x%x, MIPI_RX_ANA48_CSI0B 0x%x",
            SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0A), SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0B));
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
        LOG_MSG("pCsiMipi->CSI2_IP %d, MIPI_RX_CON24_CSI0 0x%x", pCsiInfo->port, SENINF_READ_REG(pMipiRxConf, MIPI_RX_CON24_CSI0));
        usleep(30);
        /*D-PHY SW Delay Line calibration*/
#ifdef CSI2_TIMING_DELAY
        CSI_TimingDelayCal(0);
#endif
    }
    else /*Cphy  setting for CSI0 */
    {
        /* byte clock invert*/
        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_CDPHY_L0_T0_BYTECK_INVERT) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0A, RG_CSI0A_CDPHY_L2_T1_BYTECK_INVERT) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_CDPHY_L0_T0_BYTECK_INVERT) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANAA8_CSI0B, RG_CSI0B_CDPHY_L2_T1_BYTECK_INVERT) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_L0_T0AB_EQ_BW) = 1;/*EQ Power to Enhance Speed*/
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_XX_T0CA_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_XX_T0BC_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_L1_T1AB_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_XX_T1CA_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_L2_T1BC_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_L0_T0AB_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_XX_T0CA_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_XX_T0BC_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_L1_T1AB_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_XX_T1CA_EQ_BW) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_L2_T1BC_EQ_BW) = 1;
        /*CDR register setting*/
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x30)) = 0x06040404;
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x3c)) = 0x06040404;
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x34)) = 0x1;
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x28)) = 0x1;
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x1030)) = 0x06040404;
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x103c)) = 0x06040404;
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x1034)) = 0x1;
        *((int*)(mpCSI2RxAnaBaseAddr[pCsiInfo->port]+0x1028)) = 0x1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_CPHY_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L0_CKMODE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L0_CKSEL) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L1_CKMODE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L1_CKSEL) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L2_CKMODE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_DPHY_L2_CKSEL) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_CPHY_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L0_CKMODE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L0_CKSEL) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L1_CKMODE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L1_CKSEL) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L2_CKMODE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_DPHY_L2_CKSEL) = 0;
#ifdef CSI2_EFUSE_SET
        /*Read Efuse value : termination control registers*/
        Efuse(CalSel);
#endif
        /*Power on DPHY*/
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_CORE_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_CORE_EN) = 1;
        usleep(30);
        /*Enable LPF*/
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_LPF_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_LPF_EN) = 1;
        usleep(1);
#if 1
        /*offset calibration*/
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_L0_T0AB_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0A, RG_CSI0A_XX_T0CA_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_XX_T0BC_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0A, RG_CSI0A_L1_T1AB_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_XX_T1CA_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0A, RG_CSI0A_L2_T1BC_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_L0_T0AB_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA18_CSI0B, RG_CSI0B_XX_T0CA_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_XX_T0BC_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA1C_CSI0B, RG_CSI0B_L1_T1AB_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_XX_T1CA_EQ_OS_CAL_EN) = 1;
        SENINF_BITS(pMipirx, MIPI_RX_ANA20_CSI0B, RG_CSI0B_L2_T1BC_EQ_OS_CAL_EN) = 1;
        usleep(1);
        int status = 0;
        int i = 0;
        while(1) {
            status = SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0A);
             if((status&0x3f)==0x3f)
                break;
            i++;
            if(i>100) {
                LOG_MSG("CSIA offset calibration timeout");
                break;
            }
            usleep(30);
        }
        i = 0;
        status = 0;
        while(1) {
            status = SENINF_READ_REG(pMipirx, MIPI_RX_ANA48_CSI0B);
            if((status&0x3f)==0x3f)
                break;
            i++;
            if(i>100) {
                LOG_MSG("CSIB offset calibration timeout");
                break;
            }
            usleep(30);
        }
#endif
    }
    /*End of CSI MIPI*/
    //DPCM Enable
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DPCM, 1<<((pCsiMipi->dpcm == 0x2a) ? 15 : (pCsiMipi->dpcm & 0xF + 7)));//CSI2 must be slected when sensor clk output
    LOG_MSG("CSI2-%d (cnt:%d) LaneNum:%d NCSI2_EN:%d HeadOrder:%d dpcm:%d",
        CalSel, SENINF_SETTLE_DELAY, (int)(pCsiMipi->dlaneNum + 1),
        (int)pCsiMipi->enable, (int)pCsiMipi->dataheaderOrder, (int)pCsiMipi->dpcm);
    /*Settle delay*/
    SENINF_BITS(pSeninf, SENINF1_CSI2_LNRD_TIMING, DATA_SETTLE_PARAMETER) = SENINF_SETTLE_DELAY;
    /*CSI2 control*/
    if(pCsiMipi->csi_type != CSI2_2_5G_CPHY) {/*DPhy*/
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL, (SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL) | (pCsiMipi->dataheaderOrder<<16)|(pCsiMipi->enable<<4)|(((1<<(pCsiMipi->dlaneNum+1))-1))));
        SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, BYPASS_LANE_RESYNC) = 0;
        SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, CDPHY_SEL) = 0;
        SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, CPHY_LANE_RESYNC_CNT) = 3;//(SENINF_CSI2_BASE+0x74) = 3;
        SENINF_BITS(pSeninf, SENINF1_CSI2_MODE, csr_csi2_mode) = 0;
        SENINF_BITS(pSeninf, SENINF1_CSI2_MODE, csr_csi2_header_len) = 0;
        SENINF_BITS(pSeninf, SENINF1_CSI2_DPHY_SYNC, SYNC_SEQ_MASK_0) = 0xff00;
        SENINF_BITS(pSeninf, SENINF1_CSI2_DPHY_SYNC, SYNC_SEQ_PAT_0) = 0x001d;
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_SPARE0,SENINF_READ_REG(pSeninf, SENINF1_CSI2_SPARE0)&0xFFFFFFFE);//(SENINF_CSI2_BASE+0x20) &= 0xFFFFFFFE;
    }
    else {/*CPhy*/
        SENINF_BITS(pSeninf, SENINF1_CSI2_LNRD_TIMING, DATA_TERM_PARAMETER) = 0;
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL,(SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL)|pCsiMipi->dataheaderOrder << 16)) ;//ED_SEL
        SENINF_BITS(pSeninf, SENINF1_CSI2_MODE, csr_csi2_header_len) =
            (pCsiMipi->dlaneNum == SENSOR_MIPI_1_LANE) ? 1 :
            (pCsiMipi->dlaneNum == SENSOR_MIPI_2_LANE) ? 2 :
            (pCsiMipi->dlaneNum == SENSOR_MIPI_3_LANE) ? 4 : 5 ;
        SENINF_BITS(pSeninf, SENINF1_CSI2_CTRL_TRIO_CON, TRIO0_LPRX_EN) = pCsiMipi->dlaneNum >= SENSOR_MIPI_1_LANE;
        SENINF_BITS(pSeninf, SENINF1_CSI2_CTRL_TRIO_CON, TRIO1_LPRX_EN) = pCsiMipi->dlaneNum >= SENSOR_MIPI_2_LANE;
        SENINF_BITS(pSeninf, SENINF1_CSI2_CTRL_TRIO_CON, TRIO2_LPRX_EN) = pCsiMipi->dlaneNum >= SENSOR_MIPI_3_LANE;
        SENINF_BITS(pSeninf, SENINF1_CSI2_CTRL_TRIO_CON, TRIO3_LPRX_EN) = pCsiMipi->dlaneNum >= SENSOR_MIPI_4_LANE;
        SENINF_BITS(pSeninf, SENINF1_CSI2_MODE, csr_csi2_mode) = 0x2;
        SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, CPHY_LANE_RESYNC_CNT) = 3;
        SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, BYPASS_LANE_RESYNC) = 0;
        SENINF_BITS(pSeninf, SENINF1_CSI2_RESYNC_MERGE_CTL, CDPHY_SEL) = 1;
        SENINF_BITS(pSeninf, SENINF1_SYNC_RESYNC_CTL, SYNC_DETECTION_SEL) = 1;
        SENINF_BITS(pSeninf, SENINF1_POST_DETECT_CTL, POST_EN) = 1;
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_SPARE0, SENINF_READ_REG(pSeninf, SENINF1_CSI2_SPARE0)|0x1);
    }
    SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, CLOCK_HS_OPTION) = 0;
    SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, HSRX_DET_EN) = 0;
    SENINF_BITS(pSeninf, SENINF1_CSI2_CTL, HS_TRAIL_EN) = 1;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    unsigned int hs_trail;
    property_get("vendor.debug.seninf.hs_trail", value, "0");
    hs_trail = atoi(value);
    SENINF_BITS(pSeninf, SENINF1_CSI2_HS_TRAIL, HS_TRAIL_PARAMETER) = (hs_trail) ? hs_trail : SENINF_HS_TRAIL_PARAMETER;
    /* set debug port to output packet number */
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DGB_SEL, 0x8000001A);
    /*Enable CSI2 IRQ mask*/
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN, 0xFFFFFFFF);//turn on all interrupt
    /*write clear CSI2 IRQ*/
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_STATUS, 0xFFFFFFFF);
    /*Enable CSI2 Extend IRQ mask*/
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INT_EN_EXT, 0x0000001f);//turn on all interrupt
    //LOG_MSG("SENINF1_CSI2_CTL(0x1a04_0a00 = 0x%x",SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL));
    SENINF_BITS(pSeninf, SENINF1_CTRL, CSI2_SW_RST) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, SENINF1_CTRL, CSI2_SW_RST) = 0;
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
    mipi_ana_reg_t *pMipirx = (mipi_ana_reg_t *)mpCSI2RxAnaBaseAddr[pCsiInfo->port];
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[pCsiInfo->seninf];
    SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTL, SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTL) & 0xFFFFFFE0); // disable CSI2(2.5G) first
    //disable mipi BG
    switch (pCsiInfo->port) {
    case CUSTOM_CFG_CSI_PORT_0A:
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_LPF_EN) = 0;
        break;
    case CUSTOM_CFG_CSI_PORT_0B:
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_LPF_EN) = 0;
        break;
    default:
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0A, RG_CSI0A_BG_LPF_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_CORE_EN) = 0;
        SENINF_BITS(pMipirx, MIPI_RX_ANA00_CSI0B, RG_CSI0B_BG_LPF_EN) = 0;
        break;
    }
    return ret;
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
    seninf_mux_reg_t_base *pMux;
    SENINF_ENUM            seninf  = getCSIInfo(mipiPort)->seninf;
    seninf_reg_t          *pSeninf = (seninf_reg_t *)mpSeninfCtrlRegAddr[seninf];
    SENINF_BITS(pSeninf, SENINF1_CTRL, CSI2_SW_RST) = 1;
    usleep(1);
    SENINF_BITS(pSeninf, SENINF1_CTRL, CSI2_SW_RST) = 0;
    for(i = SENINF_MUX1; i < SENINF_MUX_NUM; i++)
        if(getSeninfTopMuxCtrl((SENINF_MUX_ENUM)i) == seninf && isMUXUsed((SENINF_MUX_ENUM)i)) {
            pMux = (seninf_mux_reg_t_base *)mpSeninfMuxBaseAddr[i];
            SENINF_BITS(pMux, SENINF1_MUX_CTRL, SENINF_MUX_SW_RST) = 1;
            usleep(1);
            SENINF_BITS(pMux, SENINF1_MUX_CTRL, SENINF_MUX_SW_RST) = 0;
        }
    LOG_MSG("reset");
    return 0;
}

