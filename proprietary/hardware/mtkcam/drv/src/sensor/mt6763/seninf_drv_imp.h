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
#include "seninf_drv.h"


//-----------------------------------------------------------------------------

using namespace android;
//-----------------------------------------------------------------------------


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
	 virtual int setMclk(EMclkId mclkIdx,unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv,
        unsigned long TimestampClk);
    //
    int setMclk1(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv,
        unsigned long TimestampClk);
    //
    int setMclk2(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv,
        unsigned long TimestampClk);
    //
    virtual int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0);
    //
    virtual int setSeninfTopMuxCtrl(unsigned int seninfMuXIdx, SENINF_ENUM seninfSrc);
	//
    virtual int setSeninfVC(SENINF_ENUM SenInfsel, unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id,
    unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id);
    //
    virtual int setSeninfMuxCtrl(unsigned long Muxsel, unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel,
    TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode, unsigned long CSI_TYPE);
    //
    virtual int setSeninf1Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
    //
    virtual int setSeninf2Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
    //
    virtual int setSeninf3Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
    //
    virtual int setSeninf4Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
	//
    virtual int setSeninf1CSI2(unsigned long dataTermDelay,
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
    virtual int setSeninf2CSI2(unsigned long dataTermDelay,
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
	virtual int setSeninf4CSI2(unsigned long dataTermDelay,
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
    virtual int setSeninfCSI2(seninf_csi_para *seninf);
    virtual int setSeninfCSI2_CPHY(seninf_csi_para *seninf);
    //
    virtual int setSeninf4Scam(unsigned int scamEn,
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
    virtual int setSeninf4Parallel(unsigned int parallelEn,  unsigned int inDataType);
	//
	virtual int setAllMclkOnOff(unsigned long ioDrivingCurrent,bool enable);
    //
    virtual int setMclkIODrivingCurrent(EMclkId mclkIdx, unsigned long ioDrivingCurrent);
    int setMclk1IODrivingCurrent(unsigned long ioDrivingCurrent);
    //
    int setMclk2IODrivingCurrent(unsigned long ioDrivingCurrent);
    //
    virtual int setTG1_TM_Ctl(unsigned int seninfSrc, unsigned int TM_En, unsigned int dummypxl,unsigned int vsync,
                                    unsigned int line,unsigned int pxl);

    virtual int setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel, unsigned long  flashPol);
    //
    virtual int setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel);
    //
    virtual int setFlashEn(bool flashEn) ;
    //
    virtual int setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd);
    //
    virtual int setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode, unsigned long diffCntEn, unsigned long diffCntThr);
    //
    virtual int setN3DI2CPos(unsigned long n3dPos);
    //
    virtual int setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn);
    //
    virtual int getN3DDiffCnt(MUINT32 *pCnt);
    //
    virtual int checkSeninf1Input();
    //
    virtual int checkSeninf2Input();
    //
    virtual int autoDeskewCalibrationSeninf(EMipiPort csi_sel, unsigned int lane_num, bool is_sw_deskew );
	//
    virtual int CSI0_OffsetCal(unsigned long mode);
	//
    virtual int CSI_TimingDelayCal(unsigned long mode);
    virtual int getISPClk();
	virtual int getCSIpara(EMipiPort csiIndex ,seninf_csi_para *pSeninfpara);

    //ToDo :remove
    //virtual int setPdnRst(int camera, bool on);

private:
    //
    //IspDrv *m_pIspDrv;//6593
    volatile int mUsers;
    mutable Mutex mLock;
    int mfd;
    int m_fdSensor;
    unsigned int *mpIspHwRegAddr;
    unsigned int *mpSeninfHwRegAddr;
    unsigned int *mpCSI2RxAnalogRegStartAddrAlign;
    unsigned int *mpSeninfCtrlRegAddr[SENINF_NUM] = {NULL};
    unsigned int *mpSeninfMuxBaseAddr[SENINF_MUX_NUM] = {NULL};
    unsigned int *mpCSI2RxAnaBaseAddr[SENINF_CSI2_IP_NUM] = {NULL};
    unsigned int *mpSeninfCSIRxConfBaseAddr[SENINF_NUM] = {NULL};
    unsigned int *mpGpioHwRegAddr;
    unsigned long mDevice;
	unsigned int mCSI[5] = {0};

   	int autoDeskew_SW(EMipiPort csi_sel, unsigned int lane_num);
	int Efuse(unsigned long csi_sel);
    //
    //IspDrv* m_pIspDrv;
};

#endif // _ISP_DRV_H_
