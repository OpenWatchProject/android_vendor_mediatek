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
using namespace NSCam;

//#define SENINF_SETTLE_DELAY_DT 0x15 // data lane hs settle, base on 208 MHz csi ck
#define SENINF_SETTLE_DELAY_DT 0x20     // data lane hs settle, base on 312 MHz csi ck
//#define SENINF_SETTLE_DELAY_DT 0x28     // data lane hs settle, base on 393 MHz csi ck
//#define SENINF_SETTLE_DELAY_DT 0x35     // data lane hs settle, base on 525 MHz csi ck

//#define SENINF_SETTLE_DELAY_CK 0x9 // clock lane hs settle, base on 208 MHz csi ck
#define SENINF_SETTLE_DELAY_CK 0xD  // clock lane hs settle, base on 312 MHz csi ck
//#define SENINF_SETTLE_DELAY_CK 0x10 // clock lane hs settle, base on 393 MHz csi ck
//#define SENINF_SETTLE_DELAY_CK 0x15 // clock lane hs settle, base on 525 MHz csi ck

//#define SENINF_HS_TRAIL_PARAMETER 0x8 //SENINF_CLK_208M
#define SENINF_HS_TRAIL_PARAMETER 0x4 //SENINF_CLK_312M

#define FREE  -1
#define WAIT_2_FINALIZED 0XFF


struct CamMuxInfo
{
    SeninfConfigInfo ConfigInfo;
    SINGLE_VC_INFO2 VcInfo2;
};


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
    virtual int setSeninfTopMuxCtrl(SENINF_MUX_ENUM mux, SENINF_ENUM seninfSrc);
    virtual int setSeninfCamTGMuxCtrl(unsigned int targetCamTG, SENINF_MUX_ENUM seninfMuxSrc);
    virtual int getSeninfTopMuxCtrl(SENINF_MUX_ENUM mux);
    virtual int getSeninfCamTGMuxCtrl(unsigned int targetCamTG);
//    int setSeninfCamMuxVC(SENINF_CAM_MUX_ENUM CamMuxSel, unsigned int vc_sel, unsigned int dt_sel, unsigned int vc_en, unsigned int dt_en);
    int setSeninfVC(SENINF_ENUM SenInfsel, unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id);
    virtual int setSeninfMuxCtrl(SENINF_MUX_ENUM mux, unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode);
    virtual MBOOL isMUXUsed(SENINF_MUX_ENUM mux);
    virtual int enableMUX(SENINF_MUX_ENUM mux);
    virtual int disableMUX(SENINF_MUX_ENUM mux);
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
    int setAutoDeskew(SENINF_CSI_MIPI *pCsiMipi);

protected:
    SeninfDrvImp();
    ~SeninfDrvImp();
    virtual int debug();
    virtual MBOOL isCamMUXUsed(SENINF_CAM_MUX_ENUM cam_mux);
    virtual int enableCamMUX(SENINF_CAM_MUX_ENUM cam_mux);
    virtual int disableCamMUX(SENINF_CAM_MUX_ENUM cam_mux);
    virtual int setSeninfCamMuxSrc(SENINF_MUX_ENUM src, SENINF_CAM_MUX_ENUM target);

    volatile int mUser;
    mutable Mutex mLock;
    int mfd;
    unsigned char *mpSeninfHwRegAddr;
    unsigned char *mpCSI2RxAnalogRegStartAddrAlign;
    unsigned char *mpSeninfCtrlRegAddr[SENINF_NUM] = {NULL};
    unsigned char *mpSeninfTGRegAddr[SENINF_NUM] = {NULL};
    unsigned char *mpSeninfMuxBaseAddr[SENINF_MUX_NUM] = {NULL};
    unsigned char *mpCSI2RxAnaBaseAddr[CUSTOM_CFG_CSI_PORT_MAX_NUM] = {NULL};
    unsigned char *mpCSICphyTopAddr[3] = {NULL};
    unsigned char *mpCSIDphyTopAddr[3] = {NULL};
    unsigned char *mpSeninfCSIRxConfBaseAddr[SENINF_NUM] = {NULL};
    unsigned char *mpSeninfCamMuxRegAddr;
    unsigned int mCSI[5] = {0};

    int mCamMuxUsage[SENINF_CAM_MUX_NUM] = { FREE, FREE, FREE, FREE, FREE, FREE, FREE, FREE, FREE, FREE, FREE};
    CamMuxInfo mCamMuxInfo[SENINF_CAM_MUX_NUM];
    unsigned int mISPclk_tar[SENINF_MUX_NUM] = {0, 0, 0, 0, 0, 0};
    IMAGESENSOR_GET_SUPPORTED_ISP_CLK supported_ISP_Clks;

    int autoDeskew_SW(CUSTOM_CFG_CSI_PORT csi_sel, unsigned int lane_num);
    int seninfCamMUXArbitration(SENINF_CAM_MUX_ENUM searchStart, SENINF_CAM_MUX_ENUM searchEnd);
    int Efuse(unsigned long csi_sel);
    SENINF_CAM_MUX_ENUM convertTgEnum2CamMux(int TgEnum);
    int setSeninfCamMuxVC(SENINF_CAM_MUX_ENUM CamMuxSel, unsigned int vc_sel, unsigned int dt_sel, unsigned int vc_en, unsigned int dt_en);
    int setCamMuxChkPixelMode(SENINF_CAM_MUX_ENUM cam_mux, unsigned int pixelMode);
    virtual int setSeninfMuxCrop(SENINF_MUX_ENUM mux, unsigned int start_x, unsigned int end_y, bool enable);
    void hal_csirx_dphy_init(CUSTOM_CFG_CSI_PORT mipiPort);
    void hal_csirx_cphy_init(CUSTOM_CFG_CSI_PORT mipiPort);
    void hal_csirx_phyA_init(CUSTOM_CFG_CSI_PORT mipiPort);
    void hal_csirx_phy_init(CUSTOM_CFG_CSI_PORT mipiPort);
    void hal_csirx_seninf_csi2_setting(SENINF_CSI_MIPI *pCsiMipi);
    void hal_csirx_seninf_setting(SENINF_CSI_MIPI *pCsiMipi);
    void hal_csirx_seninf_top_setting(SENINF_CSI_MIPI *pCsiMipi);
    void hal_csirx_phyA_power_on(CUSTOM_CFG_CSI_PORT mipiPort, bool enable);
    void hal_csirx_phyA_setting(SENINF_CSI_MIPI *pCsiMipi);
    void hal_csirx_dphy_setting(SENINF_CSI_MIPI *pCsiMipi);
    void hal_csirx_cphy_setting(SENINF_CSI_MIPI *pCsiMipi);
    void hal_csirx_phy_setting(SENINF_CSI_MIPI *pCsiMipi);
    int setSeninfCsiMipi(SENINF_CSI_MIPI *pCsiMipi);
    int updateIspClk(unsigned int target_clk,  SENINF_MUX_ENUM mux);
    void hal_csirx_dphy_get_dbg_value(CUSTOM_CFG_CSI_PORT mipiPort, unsigned int dbg_mux, unsigned int lane_number, unsigned int *value);
    void hal_csirx_dphy_dbg_deskew_result(CUSTOM_CFG_CSI_PORT mipiPort, unsigned int *pResult, unsigned int deskew_lane);
    void hal_csirx_ana_get_dbg_value(CUSTOM_CFG_CSI_PORT mipiPort, unsigned int dbg_mux, unsigned int *value_a, unsigned int *value_b);
    void hal_csirx_dphy_dbg_deskew_ana_delay_code(CUSTOM_CFG_CSI_PORT mipiPort, unsigned int deskew_lane);
    int hal_csirx_work_on_deskew_result(unsigned int *deskew_data, unsigned int *pResult);
    void hal_csirx_dphy_dbg_deskew_raw_data(CUSTOM_CFG_CSI_PORT mipiPort, unsigned int *pResult, unsigned int deskew_lane);
    int dphy_hw_deskew_en(CUSTOM_CFG_CSI_PORT mipiPort, MUINT8 enable);

};

#endif

