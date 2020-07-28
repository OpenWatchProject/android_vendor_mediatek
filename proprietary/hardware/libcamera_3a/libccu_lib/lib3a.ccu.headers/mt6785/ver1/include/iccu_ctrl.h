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
#ifndef _ICCU_CTRL_H_
#define _ICCU_CTRL_H_

#include <isp_tuning.h>
#include "ccu_ext_interface/ccu_control_extif.h"
#include "ccu_ext_interface/ccu_sensor_extif.h"

using namespace NSIspTuning;

namespace NSCcuIf {

enum CcuCtrlStatusCode
{
    CCU_CTRL_SUCCEED = 0,
    CCU_CTRL_ERR_GENERAL,
};

//ICcuCtrl exports basic ccu control operations
class ICcuCtrl
{
public:
    
    virtual ~ICcuCtrl(void){}

    virtual int init(MUINT32 sensorIdx, ESensorDev_T sensorDev) = 0;

    virtual int uninit() = 0;

    virtual int ccuControl(ccu_msg_id msgId, void *inDataPtr, void *outDataPtr) = 0;
};

};  //namespace NSCcuIf
#endif

/** \fn virtual int NSCcuIf::ICcuCtrl::ccuControl(ccu_msg_id msgId, void *inDataPtr, void *outDataPtr) = 0;
 *  \brief Send command to CCU and get result if need
 *  \param[in] msgId message id to tell CCU which command to execute, see \ref ccu_msg_id for available commands
 *  \param[in] inDataPtr input data pointer, or simply a integer value, actual data type varies with msgId
 *  \param[out] outDataPtr output data pointer, actual data type varies with msgId
 *  \return 0 if success
*/