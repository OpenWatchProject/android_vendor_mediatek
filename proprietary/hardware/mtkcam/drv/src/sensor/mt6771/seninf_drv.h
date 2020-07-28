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
#ifndef _SENINF_DRV_BASE_H_
#define _SENINF_DRV_BASE_H_
#include <utils/Mutex.h>
#include <camera_custom_imgsensor_cfg.h>
#include "iseninf_drv.h"
using namespace android;
using namespace NSCamCustomSensor;
#define SENINF_CLK_208M
#if defined(SENINF_CLK_208M)
#define SENINF_TIMESTAMP_STEP     0x67
#define SENINF_SETTLE_DELAY       0x15
#define SENINF_HS_TRAIL_PARAMETER 0x8
#elif defined(SENINF_CLK_312M)
#define SENINF_TIMESTAMP_STEP     0x9b
#define SENINF_SETTLE_DELAY       0x20
#define SENINF_HS_TRAIL_PARAMETER 0xa
#else
#define SENINF_TIMESTAMP_STEP     0x67
#define SENINF_SETTLE_DELAY       0x15
#define SENINF_HS_TRAIL_PARAMETER 0x8
#endif
/*******************************************************************************
*
********************************************************************************/
class SeninfDrvImp : public SeninfDrv {
public:
    static SeninfDrv* createInstance();
    int init();
    int uninit();
    int configMclk(SENINF_MCLK_PARA *pmclk_para, unsigned long pcEn);
    int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0);
    int setSeninfTopMuxCtrl(SENINF_MUX_ENUM mux, SENINF_ENUM seninfSrc);
    int setSeninfCamTGMuxCtrl(unsigned int targetCamTG, SENINF_MUX_ENUM muxSrc);
    int getSeninfTopMuxCtrl(SENINF_MUX_ENUM mux);
    int getSeninfCamTGMuxCtrl(unsigned int targetCam);
    int setSeninfVC(SENINF_ENUM SenInfsel, unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id);
    int setSeninfMuxCtrl(SENINF_MUX_ENUM mux, unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode);
    MBOOL isMUXUsed(SENINF_MUX_ENUM mux);
    int enableMUX(SENINF_MUX_ENUM mux);
    int disableMUX(SENINF_MUX_ENUM mux);
    int setMclk(MUINT8 mclkIdx, MBOOL pcEn, unsigned long mclkSel);
    int setMclkIODrivingCurrent(MUINT32 mclkIdx, unsigned long ioDrivingCurrent);
    int setTestModel(bool en, unsigned int dummypxl,unsigned int vsync, unsigned int line,unsigned int pxl);
    int getN3DDiffCnt(MUINT32 *pCnt);
    int autoDeskewCalibrationSeninf(CUSTOM_CFG_CSI_PORT csi_sel, unsigned int lane_num, bool is_sw_deskew);
    SENINF_CSI_INFO* getCSIInfo(CUSTOM_CFG_CSI_PORT csi2IP);
    int setSeninfCsi(void *pCsi, SENINF_SOURCE_ENUM src);
    int powerOff(void *pCsi);
    int mutexLock();
    int mutexUnlock();
    int reset(CUSTOM_CFG_CSI_PORT mipiPort);
private:
    SeninfDrvImp();
    ~SeninfDrvImp();
    int setSeninfCsiMipi(SENINF_CSI_MIPI *pCsiMipi);
#ifdef CONFIG_MTK_CAM_SECURE
    int debug(int isSecure);
#else
    int debug();
#endif
private:
    volatile int mUser;
    mutable Mutex mLock;
    int mfd;
    unsigned char *mpSeninfHwRegAddr;
    unsigned char *mpCSI2RxAnalogRegStartAddrAlign;
    unsigned char *mpSeninfCtrlRegAddr[SENINF_NUM] = {NULL};
    unsigned char *mpSeninfMuxBaseAddr[SENINF_MUX_NUM] = {NULL};
    unsigned char *mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_MAX_NUM] = {NULL};
    unsigned char *mpSeninfCSIRxConfBaseAddr[SENINF_NUM] = {NULL};
    unsigned int mCSI[5] = {0};
    int autoDeskew_SW(CUSTOM_CFG_CSI_PORT csi_sel, unsigned int lane_num);
    int Efuse(unsigned long csi_sel);
};
#endif

