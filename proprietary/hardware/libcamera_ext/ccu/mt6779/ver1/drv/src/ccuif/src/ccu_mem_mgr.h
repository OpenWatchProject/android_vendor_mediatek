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
#ifndef _CCU_MEMMGR_H_
#define _CCU_MEMMGR_H_

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

#include "ccu_udrv.h"
#include "ccuif_compat.h"
#include "ccu_cmd_param_info.h"
#include "iccu_mgr.h"
#include "ccu_platform_def.h"
#include "ccu_buffer.h"

namespace NSCcuIf {

struct CcuMemHandle
{
    int shareFd;
    ion_user_handle_t ionHandle;
    char *va;
    unsigned int align_mva;
    unsigned int mva;
    unsigned int size;
    unsigned int occupiedSize;
    bool cached;
};

class CcuMemMgr
{
public:
    static CcuMemMgr *getInstance();
    CcuMemMgr();
    //~CcuMemMgr();

    bool init();
    bool uninit();
    bool reset();
    //allocate all memory needed by CCU from Android system
    bool allocateSystemMem();
    //deallocate all memory needed by CCU from Android system
    bool deallocateSystemMem();
    //split memories pre-allocated in allocateSystemMem(),
    //and assign to the CcuBuffer in param, including va, mva, cached
    bool allocateCcuBuffer(struct CcuBuffer *buffer);
    bool dumpAllocationTable();
    int getIonDevFd();
    ion_user_handle_t getIonHandle();
    unsigned int getAlignMva(enum CCU_BUFFER_TYPE);
    bool importMemoriesToKernel();

private:
    int m_ionDevFd = 0;
    struct CcuMemHandle m_memHandle[CCU_BUFTYPE_MAX] = {0};
    int m_ccuBufferRecIdx = 0;
    struct CcuBuffer *m_ccuBufferRecs[CCU_BUFFER_COUNT_CAPACITY] = {0};
    android::Mutex _allocBufMutex;

    bool _ccuAllocMem(struct CcuMemHandle *memHandle, int size, bool cached);
    bool _ccuDeallocMem(struct CcuMemHandle *memHandle);
    bool _ccuAllocMva(struct CcuMemHandle *memHandle, unsigned int startAddr, unsigned int endAddr);
    bool _ccuDeallocMva(struct CcuMemHandle *memHandle);
};

}
#endif