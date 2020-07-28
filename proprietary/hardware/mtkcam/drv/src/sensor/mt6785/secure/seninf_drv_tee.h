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

#ifndef _SENINF_DRV_TEE_H_
#define _SENINF_DRV_TEE_H_

#include "seninf_drv.h"
#include "kd_imgsensor_CA_TA_cmd.h"
#include "seninf_tee_reg.h"

class SeninfDrvTeeImp final : public SeninfDrvImp {
public:
    static SeninfDrv* createInstance();
    int init() override;
    int uninit() override;
    int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0) override;
    int setSeninfTopMuxCtrl(SENINF_MUX_ENUM mux, SENINF_ENUM seninfSrc) override;
    int setSeninfCamTGMuxCtrl(unsigned int targetCamTG, SENINF_MUX_ENUM muxSrc) override;
    int getSeninfTopMuxCtrl(SENINF_MUX_ENUM mux) override;
    int getSeninfCamTGMuxCtrl(unsigned int targetCam) override;
    int setSeninfMuxCtrl(SENINF_MUX_ENUM mux, unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode) override;
    MBOOL isMUXUsed(SENINF_MUX_ENUM mux) override;
    int enableMUX(SENINF_MUX_ENUM mux) override;
    int disableMUX(SENINF_MUX_ENUM mux) override;
    int reset(CUSTOM_CFG_CSI_PORT mipiPort) override;

private:
    volatile int   mUser;
    mutable Mutex  mLock;
    SENINF_TEE_REG mTeeReg;
    void           *mpCa;
    seninf_ca_open_session_t  *seninf_ca_open_session;
    seninf_ca_close_session_t *seninf_ca_close_session;
    seninf_ca_sync_to_pa_t    *seninf_ca_sync_to_pa;
    seninf_ca_sync_to_va_t    *seninf_ca_sync_to_va;

    /*design for 1st devapc lock*/
    seninf_ca_checkpipe_t     *seninf_ca_checkpipe;
    seninf_ca_free_t          *seninf_ca_free;
    uint32_t seninf_write_flag;

    int debug() override;
    MBOOL isCamMUXUsed(SENINF_CAM_MUX_ENUM cam_mux) override;
    int enableCamMUX(SENINF_CAM_MUX_ENUM cam_mux) override;
    int disableCamMUX(SENINF_CAM_MUX_ENUM cam_mux) override;
    int setSeninfCamMuxSrc(SENINF_MUX_ENUM src, SENINF_CAM_MUX_ENUM target) override;
    int setSeninfMuxCrop(SENINF_MUX_ENUM mux, unsigned int start_x, unsigned int end_y, bool enable) override;
    SeninfDrvTeeImp();
    ~SeninfDrvTeeImp();
};

#endif

