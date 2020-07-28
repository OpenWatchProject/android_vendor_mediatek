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
#define LOG_TAG "ICcuCtrlSystem"

#include "ccu_ctrl.h"
#include "ccu_mgr.h"
#include "ccu_ctrl_3actrl.h"
#include <cutils/properties.h>  // For property_get().
#include "ccu_log.h"
#include "ccu_ctrl_system.h"

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

namespace NSCcuIf {

/**************************************************************************
* Globals
**************************************************************************/
CcuCtrlSystem _CcuCtrlSystemSingleton;

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

/*******************************************************************************
* Static Functions
********************************************************************************/

/*******************************************************************************
* Factory Function
********************************************************************************/
ICcuCtrlSystem *ICcuCtrlSystem::createInstance()
{
	return &_CcuCtrlSystemSingleton;
}

/*******************************************************************************
* Public Functions
********************************************************************************/
void CcuCtrlSystem::destroyInstance(void)
{
	LOG_DBG("%s\n", __FUNCTION__);
}

int CcuCtrlSystem::init(void)
{
    LOG_DBG("+:%s\n",__FUNCTION__);

    int ret = AbsCcuCtrlBase::init();
    if(ret != CCU_CTRL_SUCCEED)
    {
        return ret;
    }

    LOG_DBG("-:%s\n",__FUNCTION__);
    return CCU_CTRL_SUCCEED;
}

/*******************************************************************************
* Overridden Functions
********************************************************************************/
enum ccu_feature_type CcuCtrlSystem::_getFeatureType()
{
    return CCU_FEATURE_SYSCTRL;
}

enum ccu_tg_info CcuCtrlSystem::getCcuTgInfo()
{
    //since system control is not related to particular sensor/tg, just return 1;
    return (enum ccu_tg_info) 1;
}

};  //namespace NSCcuIf
