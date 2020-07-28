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
#ifndef _ICCU_CTRL_SYNC3A_H_
#define _ICCU_CTRL_SYNC3A_H_

#include "iccu_ctrl.h"
#include <debug_exif/aaa/dbg_aaa_param.h>
#include <debug_exif/cam/dbg_cam_n3d_param.h>
#include "algorithm/ccu_n3d_ae_algo_data.h"
#include <ccu_ext_interface/ccu_3actrl_extif.h>
#include "aaa/ae_param.h"
#include <debug_exif/aaa/dbg_isp_param.h>


namespace NSCcuIf {

/**
* %ICcuCtrl3ACtrl inherited from ICcuCtrl, add 3A sync specific operations\n
* 3A sync operations can be performed use ccuControl() function with following \ref uint32_t\n
* \ref MSG_TO_CCU_SET_3A_SYNC_STATE\n
* \ref MSG_TO_CCU_SYNC_AE_SETTING\n
* \sa #uint32_t
*/
class ICcuCtrl3ACtrl : public virtual ICcuCtrl
{
public:
    static ICcuCtrl3ACtrl *createInstance();
    virtual void destroyInstance(void) = 0;
    virtual MBOOL isSupportFrameSync(void) = 0;
    virtual MBOOL isSupport3ASync(void) = 0;
    virtual void getDebugInfo(N3D_AE_DEBUG_INFO_T *a_rN3dAeDebugInfo) = 0;
    
    //returns request number
    virtual uint32_t getAeDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, AE_DEBUG_INFO_T *a_rAEDebugInfo, AE_DEBUG_DATA_T *a_rAEDebugData) = 0;
    
    //returns request number
    virtual uint32_t getAeInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, AE_INFO_T *a_rAEInfo, MUINT64 *u8RealEposuretime = NULL) = 0;

    //returns request number
    virtual uint32_t getLtmDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_ltm_debug_info) = 0;
    
    //returns request number
    virtual uint32_t getLtmInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct ccu_ltm_info_isp *ltm_info) = 0;

    //returns request number
    virtual uint32_t getHlrDebugInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, NSIspExifDebug::IspExifDebugInfo_T::IspDebugInfo *ccu_hlr_debug_info) = 0;
    
    //returns request number
    virtual uint32_t getHlrInfo(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct ccu_hlr_info_isp *hlr_info) = 0;
};


/* Sample codes:

//create instance on init
ICcuCtrl3ACtrl ccuCtrl3ASync = ICcuCtrl3ACtrl::createInstance();

...
//for passing 3A sync state to CCU
Ccu3ASyncState syncState;
syncState.mode = SYNC;
syncState.feature = WIDE_TELE;
syncState.master_info.sensorIdx = XXX;
syncState.master_info.sensorDev = XXX;
ccuCtrl3ASync.ccuControl(MSG_TO_CCU_SET_3A_SYNC_STATE, &syncState, null);

...

//for requst CCU do mapping exposure settings
ccuCtrl3ASync.ccuControl(MSG_TO_CCU_SYNC_AE_SETTING, null, null);

...

//destroy instance on uninit
ICcuCtrl3ACtrl ccuCtrl3ASync = ICcuCtrl3ACtrl::destroyInstance();
*/


};  //namespace NSCcuIf

#endif