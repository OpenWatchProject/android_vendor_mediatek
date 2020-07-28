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
#ifndef _SENINF_DRV_IMP_H_
#define _SENINF_DRV_IMP_H_

#include <utils/Mutex.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <mtkcam/utils/std/ULog.h>
#include "seninf_drv.h"


//-----------------------------------------------------------------------------

using namespace android;
//-----------------------------------------------------------------------------


#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_SENSOR, "[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_SENSOR, "[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    CAM_ULOGE(NSCam::Utils::ULog::MOD_DRV_SENSOR, "[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
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

#define DESKEW_INIT_WAIT_MS 30  // 30 ms, should less than 100ms
#define DPHY_HSW_DESKEW

/*******************************************************************************
*
********************************************************************************/
class SeninfDrvImp : public SeninfDrv {
public:
    static SeninfDrv* getInstance();
    virtual void destroyInstance();
//
private:
    SeninfDrvImp();
    virtual ~SeninfDrvImp();
//
public:
    virtual int init();
    virtual int uninit();
    virtual int configMclk(SENINF_MCLK_PARA *mclk_para, unsigned long pcEn);
    virtual int configSensorInput(SENINF_CONFIG_STRUCT *psensorPara, SensorDynamicInfo *psensorDynamicInfo,
            ACDK_SENSOR_INFO2_STRUCT *sensorInfo, bool enable);
	virtual int setAllMclkOnOff(unsigned long ioDrivingCurrent,bool enable);
    virtual int setMclkIODrivingCurrent(EMclkId mclkIdx, unsigned long ioDrivingCurrent);
    virtual int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0);
    virtual int calibrateSensor(SENINF_CONFIG_STRUCT *psensorPara);
	virtual int getN3DDiffCnt(MUINT32 *pCnt);
    virtual int resetConfiguration(IMGSENSOR_SENSOR_IDX sensorIdx);

private:
    int configInputForCSI(SENINF_CONFIG_STRUCT *pseninfPara, SensorDynamicInfo *psensorDynamicInfo, bool enable);
    int setMclk(EMclkId mclkIdx,unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv,
        unsigned long TimestampClk);


    int setSeninfVC(SENINF_ENUM SenInfsel, unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id,
    unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id);
    //
    int setSeninfMuxCtrl(unsigned long Muxsel, unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel,
    TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode, unsigned long CSI_TYPE);
    MBOOL isMUXUsed(SENINF_MUX_ENUM mux);
    int setSeninfCSI2(SENINF_CONFIG_STRUCT *pSeninfPara,  bool enable);
	int setSeninf1CSI2(unsigned long dataTermDelay,
						 unsigned long dataSettleDelay,
						 unsigned long clkTermDelay,
						 unsigned long vsyncType,
						 unsigned long dlaneNum,
						 unsigned long Enable,
						 unsigned long dataheaderOrder,
						 unsigned long mipi_type,
						 unsigned long HSRXDET,
						 unsigned long dpcm);
	 //
	 int setSeninf2CSI2(unsigned long dataTermDelay,
						 unsigned long dataSettleDelay,
						 unsigned long clkTermDelay,
						 unsigned long vsyncType,
						 unsigned long dlaneNum,
						 unsigned long Enable,
						 unsigned long dataheaderOrder,
						 unsigned long mipi_type,
						 unsigned long HSRXDET,
						 unsigned long dpcm);

	 //
	 virtual int setSeninf3CSI2(unsigned long dataTermDelay,
						 unsigned long dataSettleDelay,
						 unsigned long clkTermDelay,
						 unsigned long vsyncType,
						 unsigned long dlaneNum,
						 unsigned long Enable,
						 unsigned long dataheaderOrder,
						 unsigned long mipi_type,
						 unsigned long HSRXDET,
						 unsigned long dpcm);
	 //
	 int setSeninf4CSI2(unsigned long dataTermDelay,
						 unsigned long dataSettleDelay,
						 unsigned long clkTermDelay,
						 unsigned long vsyncType,
						 unsigned long dlaneNum,
						 unsigned long Enable,
						 unsigned long dataheaderOrder,
						 unsigned long mipi_type,
						 unsigned long HSRXDET,
						 unsigned long dpcm);

    //
    int setSeninf4Scam(unsigned int scamEn,
                        unsigned int clkInv,
                        unsigned int width,
                        unsigned int height,
                        unsigned int contiMode,
                        unsigned int csdNum,
                        unsigned int DDR_EN,
                        unsigned int default_delay,
                        unsigned int crcEn,
                        unsigned int timeout_cali,
                        unsigned int sof_src);
    //
    int setSeninf4Parallel(unsigned int parallelEn,  unsigned int inDataType);
	//
    int setMclk1IODrivingCurrent(unsigned long ioDrivingCurrent);
    //
    int setMclk2IODrivingCurrent(unsigned long ioDrivingCurrent);
    //
    int setMclk3IODrivingCurrent(unsigned long ioDrivingCurrent);
    int setMclk4IODrivingCurrent(unsigned long ioDrivingCurrent);

    int getISPClk();
    int csi_deskew_sw(SENINF_CONFIG_STRUCT *pseninfPara);
    void enable_deskew(bool enable, SENINF_CSI_PARA csi_para);
    int  checkError_deskew(SENINF_CSI_PARA csi_para, int fps, bool _log);
    void enableDelayCode_deskew(int lane, bool enable, SENINF_CSI_PARA csi_para, bool _log);
    void applyDelayCode_deskew(int lane, unsigned int delay_code, SENINF_CSI_PARA csi_para, bool _log);

    int getCSIpara(SENINF_CSI_PARA *pSeninfpara);
    int setTG_TM_Ctl(unsigned int seninfSrc, unsigned int TM_En, unsigned int dummypxl,unsigned int vsync,
                                        unsigned int line,unsigned int pxl);
    int setSeninfTopMuxCtrl(unsigned int seninfMuXIdx, SENINF_ENUM seninfSrc);
    int setSeninfMuxCtrl(
    unsigned long Muxsel, unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int PixelMode);
    int updateIspClk(unsigned int targetClk,  IMGSENSOR_SENSOR_IDX sensorIdx);



    volatile int mUsers;
    IMAGESENSOR_GET_SUPPORTED_ISP_CLK supported_ISP_Clks;
    mutable Mutex mLock_mmap;
    int mfd;
    int m_fdSensor;
    unsigned int *mpSeninfHwRegAddr;
    unsigned int *mpCSI2RxAnalogRegStartAddrAlign;
    unsigned int *mpSeninfCtrlRegAddr[SENINF_NUM] = {NULL};
    unsigned int *mpSeninfMuxBaseAddr[SENINF_MUX_NUM] = {NULL};
    unsigned int *mpCSI2RxAnaBaseAddr[SENINF_CSI2_IP_NUM] = {NULL};
    unsigned int *mpSeninfCSIRxConfBaseAddr[SENINF_NUM] = {NULL};
    unsigned int *mpGpioHwRegAddr;
    unsigned int mCSI[5] = {0};
    int mSeninfMuxUsed[SENINF_MUX_NUM] = {-1, -1, -1, -1, -1, -1};
    unsigned int mISPclk_tar[IMGSENSOR_SENSOR_IDX_MAX_NUM] = {0, 0, 0, 0};
    int Efuse(unsigned long csi_sel);
    //
    //IspDrv* m_pIspDrv;
#ifdef DPHY_HSW_DESKEW
    int initHSWDeskew(SENINF_CONFIG_STRUCT *pseninfPara);
    int uninitHSWDeskew(SENINF_CONFIG_STRUCT *pseninfPara);
    int enableHSWDeskew(SENINF_CONFIG_STRUCT *pseninfPara);
#endif
};

#endif // _ISP_DRV_H_
