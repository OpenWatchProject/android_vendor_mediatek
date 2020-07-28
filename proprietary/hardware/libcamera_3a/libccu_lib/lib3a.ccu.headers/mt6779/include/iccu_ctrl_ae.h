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
#ifndef _ICCU_CTRL_AE_H_
#define _ICCU_CTRL_AE_H_

#include "iccu_ctrl.h"
#include "aaa/ae_param.h"
#include "camera_custom_3a_nvram.h"
#include <debug_exif/aaa/dbg_aaa_param.h>

namespace NSCcuIf {

struct CcuAeHdrBufferSet
{
	void *va;
	uint32_t mva;
};

/**
* ICcuCtrlAe inherited from ICcuCtrl, add AE specific operations
*/
class ICcuCtrlAe : public virtual ICcuCtrl
{
public:
    static ICcuCtrlAe *getInstance(MUINT32 sensorIdx, ESensorDev_T sensorDev);

    /*CcuCtrlAe will fill va & mva in bufferSets, the array length is decided by CCU_HDR_DATA_BUF_CNT in ccu_ae_extif.h*/
    static bool getHdrDataBuffer(MUINT32 sensorIdx, ESensorDev_T sensorDev, struct CcuAeHdrBufferSet *bufferSets);

    //returns request number
    virtual uint32_t getDebugInfo(AE_DEBUG_INFO_T *a_rAEDebugInfo, AE_DEBUG_DATA_T *a_rAEDebugData = NULL) = 0;
    
    //returns request number
    virtual uint32_t getAeInfoForIsp(AE_INFO_T *a_rAEInfo) = 0;

    virtual bool getCcuAeOutput(struct ccu_ae_output *output) = 0;

    virtual bool getCcuAeStat(CCU_AE_STAT *output) = 0;
};

};  //namespace NSCcuIf

#endif