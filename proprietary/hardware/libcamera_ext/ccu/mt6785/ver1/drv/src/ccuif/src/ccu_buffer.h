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
#ifndef _CCU_BUFFER_H_
#define _CCU_BUFFER_H_

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
    
    void reset();
    bool init(int ionDevFd, bool cached, uint32_t lowerBound=CCU_CTRL_BUFS_LOWER_BOUND, uint32_t upperBound=CCU_CTRL_BUFS_UPPER_BOUND);
    bool uninit();

private:
    int m_IonDevFD;
    bool _ccuAllocBuffer(int drv_h, int len, int *buf_share_fd, char **buf_va, bool cached);
    bool _ccuFreeBuffer(int drv_h, int len,int buf_share_fd, char *buf_va);
    bool _mmapMva( int buf_share_fd, ion_user_handle_t *p_ion_handle, unsigned int *mva, 
        unsigned int start_addr, unsigned int end_addr);
    bool _munmapMva( ion_user_handle_t ion_handle );
};

enum eIONCacheFlushType
{
    eIONCacheFlushType_Range = 0x1,
    eIONCacheFlushType_All   = 0x2
};



struct CcuBufferList
{
public:
    bool isValid;
    struct CcuBuffer mailboxInBuf;
    struct CcuBuffer mailboxOutBuf;
    struct CcuBuffer CtrlMsgBufs[CCU_EXT_MSG_COUNT];

    CcuBufferList();
    virtual ~CcuBufferList() {}
    virtual bool init(int ionDevFd);
    virtual bool uninit();
    int m_IonDevFD;

protected:
    enum ccu_feature_type m_featureType;
    bool _initControlBuffers();
    bool _uninitControlBuffers();
    MINT32 doIonCacheFlush(MINT32 memID,eIONCacheFlushType /*flushtype*/);

};

struct CcuAfBufferList : CcuBufferList
{
public:
    struct CcuBuffer AF_Buf[2][3];
    struct CcuBuffer AF_Reg[2][3];

    bool init(int ionDevFd);
    bool uninit();

private:
    bool _initAFBuffers();
    bool _uninitAFBuffers();
};


struct CcuAeBufferList : CcuBufferList
{};

struct Ccu3ASyncBufferList : CcuBufferList
{};

};  //namespace NSCcuIf

#endif