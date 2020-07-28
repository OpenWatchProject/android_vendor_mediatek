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
#ifndef _CCU_MGR_H_
#define _CCU_MGR_H_

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <mt_iommu_port.h>

#include "ccu_udrv.h"
#include "ccuif_compat.h"
#include "ccuif_compat_conv.h"
#include "iccu_mgr.h"
#include "ccu_platform_def.h"

namespace NSCcuIf {

struct CcuBuffer
{
    int share_fd;
    ion_user_handle_t ion_handle;
    char *va;
    unsigned int mva;
    unsigned int size;
};

struct CcuBufferList
{
    bool isValid;
    struct CcuBuffer mailboxInBuf;
    struct CcuBuffer mailboxOutBuf;
    struct CcuBuffer CtrlMsgBufs[CCU_EXT_MSG_COUNT];
};

class ICcuMgrPriv
{
public:

    static ICcuMgrPriv* createInstance();

    static void destroyInstance();

    virtual ~ICcuMgrPriv(){}

    virtual struct CcuBufferList getCcuBufferList() = 0;
};

class CcuMgr: ICcuMgrExt, ICcuMgrPriv
{
public:
    CcuMgr();
    int ccuInit();
    int ccuUninit();
    int ccuBoot();
    int ccuShutdown();
    struct CcuBufferList getCcuBufferList();

private:
    //memory/buffer
    void _checkCcuEnable();
    MBOOL _ccuAllocBuffer(int drv_h, int len, int *buf_share_fd, char **buf_va);
    MBOOL _ccuFreeBuffer(int drv_h, int len,int buf_share_fd, char *buf_va);
    MBOOL _mmapMva( int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva );
    MBOOL _munmapMva( ion_user_handle_t ion_handle );

    MBOOL _setBufferSizes();
    MBOOL _initBuffer(struct CcuBuffer *buffer);
    MBOOL _uninitBuffer(struct CcuBuffer *buffer);
    MBOOL _initControlBuffers();
    MBOOL _uninitControlBuffers();

    int m_IonDevFD;

    MBOOL m_CcuEnable;

    CcuBufferList m_BufferList;

    CcuDrvImp *m_pDrvCcu;
};

};  //namespace NSCcuIf

#endif