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

/**
    this module support only CCU + DIP only.
    other module like FDVT CCUSV UNI... etc. need to re-pack its owner module via inheriting this module
*/
#ifndef _CCU_DRV_IPC_H_
#define _CCU_DRV_IPC_H_

#include "utils/Mutex.h"    // android mutex
#include "ccu_drv.h"
#include "ccu_ext_interface/ccu_ext_interface.h"

#include <semaphore.h>

//For ION functions
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

//For AEE function
#include <dlfcn.h>
#include "aee.h"
#include "ccu_aee_mgr.h"
#include "kd_ccu_i2c.h"

class CcuDrvIpc
{
    public:
        ~CcuDrvIpc(void){}
        CcuDrvIpc(void);
        MBOOL init(struct shared_buf_map *sb_map_ptr, void *ccuDmBase, void *ccuCtrlBase);
        MBOOL sendCmd(struct ccu_msg *msg);
        MBOOL getIpcIOBufAddr(void **ipcInDataPtr, void **ipcOutDataPtr, MUINT32 *ipcInDataAddrCcu, MUINT32 *ipcOutDataAddrCcu);

    private:
        MBOOL m_initialized;
        volatile MUINT32 *m_ccuIntTriggerPtr;
        volatile struct ap2ccu_ipc_t *m_ccuIpcPtr;
        void *m_ipcInDataPtr;
        void *m_ipcOutDataPtr;
        MUINT32 m_ipcInDataAddrCcu;
        MUINT32 m_ipcOutDataAddrCcu;
};

#endif  // _CCU_DRV_H_
