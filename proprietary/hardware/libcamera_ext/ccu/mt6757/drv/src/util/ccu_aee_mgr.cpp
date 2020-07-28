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
#define LOG_TAG "ccuaee"

#include <dlfcn.h>
#include "ccu_aee_mgr.h"
#include "ccu_log.h"

#define LIB_AED "libaed.so"

EXTERN_DBG_LOG_VARIABLE(ccu_drv);

void * CcuAeeMgrDl::_dlHandle;
bool CcuAeeMgrDl::_aeeSysExpInitDone = false;
AEE_SYSTEM_EXCEPTION_FUNC CcuAeeMgrDl::AeeSystemException;
/*******************************************************************************
*
********************************************************************************/
void CcuAeeMgrDl::CcuAeeMgrDlInit()
{
    void *func = nullptr;

    _aeeSysExpInitDone = false;

    _dlHandle = dlopen(LIB_AED, RTLD_NOW);
    if(_dlHandle == nullptr)
    {
        LOG_ERR("dlopen LIB_AED failed.\n");
        return;
    }
    LOG_DBG_MUST("aed dl handle: %p\n", _dlHandle);

    func = dlsym(_dlHandle, "aee_system_exception");

    AeeSystemException = (AEE_SYSTEM_EXCEPTION_FUNC)(func);
    if (AeeSystemException == nullptr) 
    {
       LOG_ERR("dlopen aee_system_exception failed.:%p\n", func);
       dlclose(_dlHandle);
       return;
    }
    LOG_DBG_MUST("AeeSystemException func ptr: %p\n", AeeSystemException);

    _aeeSysExpInitDone = true;
}

void CcuAeeMgrDl::CcuAeeMgrDlUninit()
{
    if(_dlHandle != nullptr)
    {
        dlclose(_dlHandle);
    }
}

bool CcuAeeMgrDl::IsCcuAeeSysExpInitDone()
{
    return _aeeSysExpInitDone;
}