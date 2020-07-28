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
#ifndef _ISENINF_DRV_H_
#define _ISENINF_DRV_H_

#include "seninf_drv_def.h"

typedef enum {
    CMD_SENINF_GET_SENINF_ADDR,
    CMD_SENINF_DEBUG_TASK,
    CMD_SENINF_DEBUG_TASK_CAMSV,
    CMD_SENINF_DEBUG_PIXEL_METER,
    CMD_SENINF_GET_FREE_CAM_MUX,
    CMD_SENINF_SET_CAM_MUX,
    CMD_SENINF_FINALIZE_CAM_MUX,
    CMD_SENINF_SET_MUX_CROP_PIX_CTRL,
    CMD_SENINF_SYNC_REG_TO_PA,
    CMD_SENINF_SET_DFS_FOR_MUX,
    CMD_SENINF_CHECK_PIPE,
    CMD_SENINF_RESUME_FREE,
    CMD_SENINF_MAX
} CMD_SENINF;

/*******************************************************************************
*
********************************************************************************/
class SeninfDrv {
public:
    static SeninfDrv* getInstance();
#ifdef CONFIG_MTK_CAM_SECURE
    static SeninfDrv* getInstance(MBOOL isSecure);
#endif
    virtual int setSeninfCsi(void *pCsi, SENINF_SOURCE_ENUM src) = 0;
    virtual int init() = 0;
    virtual int uninit() = 0;
    virtual int configMclk(SENINF_MCLK_PARA *pmclk_para, unsigned long pcEn) = 0;
    virtual int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0) = 0;
    virtual int setSeninfTopMuxCtrl(SENINF_MUX_ENUM seninfMuXIdx, SENINF_ENUM seninfSrc) = 0;
    virtual int setSeninfCamTGMuxCtrl(unsigned int targetCamTG, SENINF_MUX_ENUM muxSrc) = 0;
    virtual int getSeninfTopMuxCtrl(SENINF_MUX_ENUM mux) = 0;
    virtual int getSeninfCamTGMuxCtrl(unsigned int targetCam) = 0;
//    virtual int setSeninfCamMuxVC(SENINF_CAM_MUX_ENUM CamMuxSel, unsigned int vc_sel, unsigned int dt_sel, unsigned int vc_en, unsigned int dt_en) = 0;
    virtual int setSeninfVC(SENINF_ENUM SenInfsel, unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id, unsigned int vc4Id, unsigned int vc5Id) = 0;
//    virtual int setSeninfMuxCrop(SENINF_ENUM SenInfsel) {return 0;}; //check
    virtual int setSeninfMuxCtrl(SENINF_MUX_ENUM mux, unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode) = 0;
    virtual MBOOL isMUXUsed(SENINF_MUX_ENUM mux) = 0;
    virtual int enableMUX(SENINF_MUX_ENUM mux) = 0;
    virtual int disableMUX(SENINF_MUX_ENUM mux) = 0;
    virtual int setMclk(MUINT8 mclkIdx, MBOOL pcEn, unsigned long mclkSel) = 0;
    virtual int setMclkIODrivingCurrent(MUINT32 mclkIdx, unsigned long ioDrivingCurrent) = 0;
//    virtual int set_camux_checker_pixel_mode(unsigned int SENINF_CAM_MUX, unsigned int pixelMode) = 0;
    virtual int setTestModel(bool en, unsigned int dummypxl,unsigned int vsync, unsigned int line, unsigned int pxl) = 0;
    virtual int getN3DDiffCnt(MUINT32 *pCnt) = 0;
    virtual SENINF_CSI_INFO* getCSIInfo(CUSTOM_CFG_CSI_PORT mipiPort) = 0;
    virtual int powerOff(void *pCsi) = 0;
    virtual int resetConfiguration(IMGSENSOR_SENSOR_IDX sensorIdx) {(void)sensorIdx;return 0;}
    virtual int setIntegration(IMGSENSOR_SENSOR_IDX sensorIdx, SENINF_MUX_ENUM mux, NSCam::MSize crop, MUINT framerate){(void)sensorIdx;(void)mux;(void)crop;(void)framerate;return 0;}
    virtual int mutexLock() = 0;
    virtual int mutexUnlock() = 0;
    virtual int reset(CUSTOM_CFG_CSI_PORT mipiPort) = 0;
#ifdef HALSENSOR_AUTO_DESKEW
    virtual int setAutoDeskew(SENINF_CSI_MIPI *pCsiMipi);
#endif
protected:
    virtual ~SeninfDrv() {};
};

#endif

