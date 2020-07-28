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
#ifndef _IMEM_DRV_IMP_H_
#define _IMEM_DRV_IMP_H_
//-----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//
#include <mtkcam/def/common.h>
#include <imem_3a.h>

//-----------------------------------------------------------------------------
#define __ISP_USE_ION__

//-----------------------------------------------------------------------------
#include <linux/ion_drv.h>
#include <ion/ion.h>
#include <vector>
#include <map>
#include <list>
using namespace std;
//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------

#include <cutils/properties.h>              // For property_get().

//-----------------------------------------------------------------------------
typedef struct _imem_map_info_
{
    MUINT32         pAddr;
    MUINT32         size;
}stIMEM_MAP_INFO;

enum eIONCacheFlushType
{
    eIONCacheFlushType_Range = 0x1,
    eIONCacheFlushType_All   = 0x2
};
//-----------------------------------------------------------------------------
class IMem3AImp : public IMem3A
{
    public:
        IMem3AImp();
        ~IMem3AImp();
    //
    public:
        static IMem3A*  getInstance(void);
        virtual void    destroyInstance(void);
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        virtual MBOOL   reset(void);
        virtual MINT32  allocVirtBuf(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  freeVirtBuf(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  mapPhyAddr(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  unmapPhyAddr(
            IMEM_BUF_INFO* pInfo);
        virtual MINT32  cacheSyncbyRange(IMEM_CACHECTRL_ENUM ctrl,IMEM_BUF_INFO* pInfo);
        //remove later
        virtual MINT32  cacheFlushAll(void);
    //
#if defined (__ISP_USE_ION__)

//        virtual MUINT32 ion_alloc(
//            stIspIonAllocData ion_alloc_data);
//        virtual MUINT32 ion_free(
//            stIspIonHandleData handle_data);

        #define stIspIonAllocData struct ion_allocation_data
        #define stIspIonHandleData struct ion_handle_data

        MINT32 mIonDrv;
#endif

    private:
        volatile MINT32 mInitCount;
        mutable Mutex   mLock;
        map<MUINT32,stIMEM_MAP_INFO> buf_map;
        MBOOL m_3AMemLogEnable;
        //for record the buffer paddr of vido
        MUINT32 vidopAddr;
};

#endif


