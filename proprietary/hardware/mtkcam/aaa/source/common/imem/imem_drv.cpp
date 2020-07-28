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

#define LOG_TAG "3A/IMem"
//
#include <mtkcam/utils/std/Log.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <ion.h>
#include <linux/mtk_ion.h>
//
#include "imem_drv_imp.h"

#include "camera_isp.h"
#include <utils/Log.h>
//
//
#define ISP_DRV_DEV_NAME  "/dev/camera-isp"
//
#define _ION_DEVICE_NAME_ "/dev/ion"
//
//#define IMEM_DRV_DELAY usleep(50000);
#define IMEM_DRV_DELAY

#define GET_PROP(prop, init, val)\
{\
    val = property_get_int32(prop, (init));\
}
//-----------------------------------------------------------------------------
IMem3A* IMem3A::createInstance()
{
    //DBG_LOG_CONFIG(drv, imem_drv);
    return IMem3AImp::getInstance();
}
//-----------------------------------------------------------------------------
IMem3AImp::IMem3AImp()
    : mIonDrv(0)
    , mInitCount(0)
    , m_3AMemLogEnable(0)
    , vidopAddr(0)
{
    CAM_LOGD("getpid[0x%08x],gettid[0x%08x] ", getpid() ,gettid());
}
//-----------------------------------------------------------------------------
IMem3AImp::~IMem3AImp()
{
    CAM_LOGD_IF( m_3AMemLogEnable,"");
}
//-----------------------------------------------------------------------------
IMem3A* IMem3AImp::getInstance(void)
{
    static IMem3AImp singleton;
    //CAM_LOGD("singleton[0x%08x],getpid[0x%08x],gettid[0x%08x] ",&singleton,getpid() ,gettid());
    IMEM_DRV_DELAY
    return &singleton;
}
//-----------------------------------------------------------------------------
void IMem3AImp::destroyInstance(void)
{
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
///Flow concept: we do increase global and local count first, then judge we have to initial m4uDrv/ion_dev and m4uPort according
///              local count and global count repectively
MBOOL IMem3AImp::init(void)
{
    GET_PROP("vendor.debug.camera_imem.log", 0, m_3AMemLogEnable);
    vidopAddr=0;
    MBOOL Result = MTRUE;
    CAM_LOGD_IF( m_3AMemLogEnable,"mCount(%d)", mInitCount);
    //
    Mutex::Autolock lock(mLock);
    //
    IMEM_DRV_DELAY
    android_atomic_inc(&mInitCount);
    //CAM_LOGD_IF( m_3AMemLogEnable,"#flag3# mInitCount(%d),mInitCount>0 and run w\o _use_kernel_ref_cnt_\n",mInitCount);
    //////////////////////////////////////////
    //init. buf_map
    //erase all
    buf_map.clear();
    //


#if defined (__ISP_USE_ION__)
    if(mInitCount==1)
    {

        //////////////////////////////////////////////////////
        // we initial m4udrv and open ion device when local count is 1,
        // and config m4v ports when global count is 1

   #if defined (__ISP_USE_ION__)
        mIonDrv = mt_ion_open("imem_drv.cpp");
        if (mIonDrv < 0)
        {
            CAM_LOGE("ion device open FAIL ");
            return MFALSE;
        }
        CAM_LOGD_IF( m_3AMemLogEnable,"open ion id(%d).\n", mIonDrv);
   #endif
#if 1
        CAM_LOGD_IF( m_3AMemLogEnable,"Skip m4u_config_port\n");
        return MTRUE;
#endif

#else
        CAM_LOGE("########################\n");
        CAM_LOGE("########################\n");
        CAM_LOGE("need to review M4U PORT ID\n");
        CAM_LOGE("########################\n");
        CAM_LOGE("########################\n");
        return MFALSE;
#endif
    }//match if local count

    return Result;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
///Flow concept: we do decrease global and local count first, then judge we have to uninit m4uDrv/ion_dev and m4uPort according
///              local count and global count repectively
MBOOL IMem3AImp::uninit(void)
{
    MBOOL Result = MTRUE;

    //
    CAM_LOGD_IF( m_3AMemLogEnable,"mCount(%d)", mInitCount);
    //
    Mutex::Autolock lock(mLock);
    //
    ///CAM_LOGD_IF( m_3AMemLogEnable,"mInitCount(%d)",mInitCount);
    // More than one user
    android_atomic_dec(&mInitCount);


    //////////////////////////////////////////////////////
    // we delete m4udrv and close ion device when local count is 1,
    // and unconfig m4v ports when global count is 1
    if ( mInitCount <= 0 )
    {
#if defined (__ISP_USE_ION__)
        // we have to handle local ion drv here
        // if process A open ionID, then process B open ionID before process A call IMem3A_uninit,
        // process A would not do close ionID.
        if (mIonDrv)
        {
            CAM_LOGD_IF( m_3AMemLogEnable,"close ion id(%d).\n", mIonDrv);
            ion_close(mIonDrv);
        }
#endif
    }



    return Result;
}
//-----------------------------------------------------------------------------
MBOOL IMem3AImp::reset(void)
{
    CAM_LOGD_IF( m_3AMemLogEnable,"");
    //erase all
    buf_map.clear();
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MINT32 IMem3AImp::allocVirtBuf(
    IMEM_BUF_INFO* pInfo)
{

    if(pInfo->size <= 0)
    {
        CAM_LOGE("size 0!");
        return -1;
    }
#if defined (__ISP_USE_ION__)
    CAM_LOGD_IF( m_3AMemLogEnable,"__ISP_USE_ION__");
    //
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;

    int ion_prot_flags = pInfo->useNoncache ? 0 : (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);

    //ion_prot_flags = 0;//marked: it should decide by pInfo->useNoncache
    if(ion_prot_flags != 0)
        CAM_LOGD_IF( m_3AMemLogEnable,"imem_alloc: cached");

#if 1 //ndef USING_MTK_LDVT   // Not using LDVT.
    //a. Allocate a buffer
	CAM_LOGD_IF( m_3AMemLogEnable,"ion_alloc \n");

    if(ion_alloc(
        mIonDrv,
        pInfo->size,
        0, //32 //alignment
        ION_HEAP_MULTIMEDIA_MASK,
        ion_prot_flags,
        &pIonHandle))
#else
	    printf("ion_alloc \n");
    //a. Allocate a buffer
    if(ion_alloc(
        mIonDrv,
        pInfo->size,
        0, //32 //alignment
        ION_HEAP_CARVEOUT_MASK,
		ion_prot_flags,
        &pIonHandle))
#endif
    {
        printf("ion_alloc fail, size(0x%x)",pInfo->size);
        return -1;
    }
	CAM_LOGD_IF( m_3AMemLogEnable,"ion_share \n");
    //b. Map a new fd for client.
    if(ion_share(
        mIonDrv,
        pIonHandle,
        &IonBufFd))
    {
        printf("ion_share fail");
        return -1;
    }
    pInfo->memID = (MINT32)IonBufFd; // Tianshu suggest to keep this fd
    //c. Map FD to a virtual space.
    pInfo->virtAddr = (MUINTPTR)ion_mmap(mIonDrv,NULL, pInfo->size, PROT_READ|PROT_WRITE, MAP_SHARED, IonBufFd, 0);
    if (!pInfo->virtAddr)
    {
        CAM_LOGE("Cannot map ion buffer. memID(0x%x)/size(0x%x)/va(0x%x)",pInfo->memID,pInfo->size,pInfo->virtAddr);
        return -1;
    }

    //
    CAM_LOGD_IF( m_3AMemLogEnable,"ionFd[0x%x]",pInfo->memID);
#endif
    CAM_LOGD_IF( m_3AMemLogEnable,"mID[0x%x]/size[0x%x]/VA[0x%u] \n",pInfo->memID,pInfo->size,pInfo->virtAddr);
    return 0;
}
//-----------------------------------------------------------------------------
MINT32 IMem3AImp::freeVirtBuf(
    IMEM_BUF_INFO* pInfo)
{
    CAM_LOGD_IF( m_3AMemLogEnable,"mID[0x%x]/size[0x%x]/VA[0x%x]/PA[0x%x]",pInfo->memID,pInfo->size,pInfo->virtAddr,pInfo->phyAddr);
    IMEM_DRV_DELAY


    if ( IMEM_MIN_ION_FD > pInfo->memID ) {
        if ( pInfo->virtAddr != 0 ) {
            ::free((MUINT8 *)pInfo->virtAddr);
        }

    }
    //
#if defined (__ISP_USE_ION__)
    //
    else{
        ion_user_handle_t pIonHandle;
        MINT32 IonBufFd;
        CAM_LOGD_IF( m_3AMemLogEnable,"ionFd[0x%x]",pInfo->memID);
        //a. get handle of ION_IOC_SHARE from fd_data.fd
        IonBufFd = pInfo->memID;
        if(ion_import(
            mIonDrv,
            IonBufFd,
            &pIonHandle))
        {
            CAM_LOGE("ion_import fail, memID(0x%x)",IonBufFd);
            return -1;
        }
        //free for IMPORT ref cnt
        if(ion_free(
            mIonDrv,
            pIonHandle))
        {
            CAM_LOGE("ion_free fail");
            return -1;
        }
        ion_munmap(mIonDrv,(char*)pInfo->virtAddr, pInfo->size);
        ion_share_close(mIonDrv,pInfo->memID);

        //d. pair of ION_IOC_ALLOC
        if(ion_free(
            mIonDrv,
            pIonHandle))
        {
            CAM_LOGE("ion_free fail");
            return -1;
        }
        pInfo->virtAddr = NULL;
    }
#else
    else{
        CAM_LOGE("ERROR:mem ID(0x%x)",pInfo->memID);
        return -1;
    }
#endif

    return 0;
}
//-----------------------------------------------------------------------------
MINT32  IMem3AImp::mapPhyAddr(
    IMEM_BUF_INFO* pInfo)
{
#if defined (__ISP_USE_ION__)
        ion_user_handle_t pIonHandle;
        MINT32 IonBufFd;
        MINT32 err;
        //struct ion_handle_data handle_data;
        struct ion_mm_data mm_data;
        struct ion_sys_data sys_data;
        //struct ion_fd_data fd_data;
        //
        //a. get handle of ION_IOC_SHARE from IonBufFd
        IonBufFd = pInfo->memID;
        if(ion_import(
            mIonDrv,
            IonBufFd,
            &pIonHandle))
        {
            CAM_LOGE("ion_import fail, memID(0x%x)",IonBufFd);
            return -1;
        }
        //
        mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
        mm_data.config_buffer_param.handle = (ion_user_handle_t)pIonHandle;//allocation_data.handle;
        mm_data.config_buffer_param.eModuleID = 1;    // 1;
        mm_data.config_buffer_param.security = pInfo->bufSecu;
        mm_data.config_buffer_param.coherent = pInfo->bufCohe;

        err = ion_custom_ioctl(mIonDrv, ION_CMD_MULTIMEDIA, &mm_data);
        if(err == (-ION_ERROR_CONFIG_LOCKED))
        {
            CAM_LOGD_IF( m_3AMemLogEnable,"IOCTL[ION_IOC_CUSTOM] Double config after map phy address");
        }
        else if(err != 0)
        {
            CAM_LOGE("IOCTL[ION_IOC_CUSTOM] ION_CMD_MULTIMEDIA Config Buffer failed!");
            return -1;
        }
        //
        sys_data.sys_cmd = ION_SYS_GET_PHYS;
        sys_data.get_phys_param.handle = (ion_user_handle_t)pIonHandle;  //allocation_data.handle;
        if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM, &sys_data))
        {
            CAM_LOGE("IOCTL[ION_IOC_CUSTOM] Config Buffer failed!");
            return -1;
        }
        //
        pInfo->phyAddr = (MUINTPTR)(sys_data.get_phys_param.phy_addr);

        //free for IMPORT ref cnt
        if(ion_free(
            mIonDrv,
            pIonHandle))
        {
            CAM_LOGE("ion_free fail");
            return -1;
        }
        //
        //CAM_LOGD_IF( m_3AMemLogEnable,"Physical address=0x%08X len=0x%X", sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);

    //
    CAM_LOGD_IF( m_3AMemLogEnable,"mID(0x%x),size(0x%x),VA(x%x),PA(0x%x),S/C(%d/%d)",pInfo->memID,pInfo->size,pInfo->virtAddr,pInfo->phyAddr,pInfo->bufSecu,pInfo->bufCohe);

    return 0;

#else
    CAM_LOGE("ERROR:mem ID(%d)",pInfo->memID);
    return -1;
#endif
}
//-----------------------------------------------------------------------------
MINT32  IMem3AImp::unmapPhyAddr(
    IMEM_BUF_INFO* /*pInfo*/)
{
    return 0;
}
//-----------------------------------------------------------------------------
MINT32 IMem3AImp::cacheFlushAll(
    void)
{
    CAM_LOGE("DO NOT SUPPORT cache flush all, plz use cacheSyncbyRange");
    return 0;
}
//-----------------------------------------------------------------------------
MINT32 IMem3AImp::cacheSyncbyRange(IMEM_CACHECTRL_ENUM ctrl,IMEM_BUF_INFO* pInfo)
{
#if defined (__ISP_USE_ION__)

    //flush all if the buffer size is larger than 50M (suggested by K)
    if((ctrl == IMEM_CACHECTRL_ENUM_FLUSH) && (pInfo->size >= 0x3200000))
    {
        //mpM4UDrv->m4u_cache_flush_all(M4U_PORT_IMGI);
        //CAM_LOGD_IF( m_3AMemLogEnable,"+ Flush all, c/m/va/sz/pa(%d/0x%x/0x%x/0x%x/0x%x)",ctrl,pInfo->memID,pInfo->virtAddr,pInfo->size,pInfo->phyAddr);
        //return 0;
    }
    MINT32 err=0;
    CAM_LOGD_IF( m_3AMemLogEnable,"+, c/m/va/sz/pa(%d/0x%x/0x%x/0x%x/0x%x)",ctrl,pInfo->memID,pInfo->virtAddr,pInfo->size,pInfo->phyAddr);


    //a. get handle of ION_IOC_SHARE from IonBufFd and increase handle ref count
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;
    IonBufFd = pInfo->memID;
    if(ion_import(mIonDrv,IonBufFd,&pIonHandle))
    {
        CAM_LOGE("ion_import fail, memId(0x%x)",IonBufFd);
        return -1;
    }

    //b. cache sync by range
    struct ion_sys_data sys_data;
    sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle=(ion_user_handle_t)pIonHandle;
    sys_data.cache_sync_param.va    = (void *)pInfo->virtAddr;
    sys_data.cache_sync_param.size  = pInfo->size;
    switch(ctrl)
    {
        case IMEM_CACHECTRL_ENUM_FLUSH:
            sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_BY_RANGE;
            break;
        case IMEM_CACHECTRL_ENUM_INVALID:
            sys_data.cache_sync_param.sync_type=ION_CACHE_INVALID_BY_RANGE;
            break;
        case IMEM_CACHECTRL_ENUM_INVALID_ALL:
            sys_data.cache_sync_param.sync_type=ION_CACHE_INVALID_ALL;
            break;
        default:
            CAM_LOGE("ERR cmd(%d)",ctrl);
            break;
    }
    if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM,&sys_data))
    {
        CAM_LOGE("CAN NOT DO SYNC, memID/(0x%x)",pInfo->memID);
        //decrease handle ref count if cache fail
        if(ion_free(mIonDrv,pIonHandle))
        {
            CAM_LOGE("ion_free fail");
            return -1;
        }
        return -1;
    }

    //c. decrease handle ref count
    if(ion_free(mIonDrv,pIonHandle))
    {
        CAM_LOGE("ion_free fail");
        return -1;
    }

    CAM_LOGD_IF( m_3AMemLogEnable,"-");
    return err;
#endif
    return 0;
}
//-----------------------------------------------------------------------------
