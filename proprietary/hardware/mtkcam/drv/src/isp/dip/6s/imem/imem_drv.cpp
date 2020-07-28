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

#define LOG_TAG "Drv/IMem"
//
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
#include <mt_iommu_port.h>

#include "camera_dip.h"
#include <utils/Log.h>
//
//
#define ISP_DRV_DEV_NAME  "/dev/camera-dip"
//
#define _ION_DEVICE_NAME_ "/dev/ion"
//
//#define IMEM_DRV_DELAY usleep(50000);
#define IMEM_DRV_DELAY

//add define for EP
#ifndef M4U_PORT_L9_IMG_IMGI_D1_MDP
#define M4U_PORT_L9_IMG_IMGI_D1_MDP 1
#endif

//-----------------------------------------------------------------------------
DipIMemDrv* DipIMemDrv::createInstance()
{
    DBG_LOG_CONFIG(drv, imem_drv);
    return DipIMemDrvImp::getInstance();
}
//-----------------------------------------------------------------------------
DipIMemDrvImp::DipIMemDrvImp()
    :mIonDrv(0),mInitCount(0),vidopAddr(0)
{
    IMEM_DBG("getpid[0x%08x],gettid[0x%08x] ", getpid() ,gettid());
}
//-----------------------------------------------------------------------------
DipIMemDrvImp::~DipIMemDrvImp()
{
    IMEM_DBG("");
}
//-----------------------------------------------------------------------------
DipIMemDrv* DipIMemDrvImp::getInstance(void)
{
    static DipIMemDrvImp singleton;
    IMEM_DBG("singleton[0x%lx],getpid[0x%08x],gettid[0x%08x] ",(unsigned long)(&singleton),getpid() ,gettid());
    IMEM_DRV_DELAY
    return &singleton;
}
//-----------------------------------------------------------------------------
void DipIMemDrvImp::destroyInstance(void)
{
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
///Flow concept: we do increase global and local count first, then judge we have to initial m4uDrv/ion_dev and m4uPort according
///              local count and global count repectively
MBOOL DipIMemDrvImp::init(void)
{
    vidopAddr=0;
    MBOOL Result = MTRUE;
    IMEM_INF("DIP mCount(%d)", mInitCount);
    //
    Mutex::Autolock lock(mLock);
    //
    IMEM_DRV_DELAY
    android_atomic_inc(&mInitCount);
    //IMEM_DBG("#flag3# mInitCount(%d),mInitCount>0 and run w\o _use_kernel_ref_cnt_\n",mInitCount);
    //////////////////////////////////////////
    //init. buf_map
    //erase all
    buf_map.clear();
    //


#if defined (__ISP_USE_PMEM__)
    //
#elif defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
    if(mInitCount==1)
    {
   #if defined (__ISP_USE_ION__)
        mIonDrv = mt_ion_open("DipIMemDrvImp.cpp");
        if (mIonDrv < 0)
        {
            IMEM_ERR("ion device open FAIL ");
            return MFALSE;
        }
        IMEM_INF("open ion id(%d).\n", mIonDrv);
   #endif

        return MTRUE;
    }//match if local count
#endif

    //
    if(!Result)
    {
    }
    return Result;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
///Flow concept: we do decrease global and local count first, then judge we have to uninit m4uDrv/ion_dev and m4uPort according
///              local count and global count repectively
MBOOL DipIMemDrvImp::uninit(void)
{
    MBOOL Result = MTRUE;

    //
    IMEM_INF("mCount(%d)", mInitCount);
    //
    Mutex::Autolock lock(mLock);
    //
    ///IMEM_DBG("mInitCount(%d)",mInitCount);
    // More than one user
    android_atomic_dec(&mInitCount);

#if defined (__ISP_USE_PMEM__)
    //
#elif defined (__ISP_USE_ION__)
    //////////////////////////////////////////////////////
    // we delete m4udrv and close ion device when local count is 1,
    // and unconfig m4v ports when global count is 1
    if ( mInitCount <= 0 )
    {
#if defined (__ISP_USE_ION__)
        // we have to handle local ion drv here
        // if process A open ionID, then process B open ionID before process A call DipImemDrv_uninit,
        // process A would not do close ionID.
        if (mIonDrv)
        {
            IMEM_INF("close ion id(%d).\n", mIonDrv);
            ion_close(mIonDrv);
        }
#endif
    }
#endif



    return Result;
}
//-----------------------------------------------------------------------------
MBOOL DipIMemDrvImp::reset(void)
{
    IMEM_DBG("");
    //erase all
    buf_map.clear();
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MINT32 DipIMemDrvImp::allocVirtBuf(
    IMEM_BUF_INFO* pInfo)
{

    if(pInfo->size <= 0)
    {
        LOG_ERR("size 0!");
        return -1;
    }
#if defined (__ISP_USE_PMEM__)
    IMEM_DBG("__ISP_USE_PMEM__");
    //
    pInfo->virtAddr= (MUINTPTR) ::pmem_alloc_sync(pInfo->size, &pInfo->memID);
    //
    IMEM_DBG("memID[0x%x]",pInfo->memID);
#elif defined (__ISP_USE_ION__)
    IMEM_DBG("__ISP_USE_ION__");
    //
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;

    int ion_prot_flags = pInfo->useNoncache ? 0 : (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);

    //ion_prot_flags = 0;//marked: it should decide by pInfo->useNoncache
    if(ion_prot_flags != 0)
        IMEM_DBG("imem_alloc: cached");

#if 1 //ndef USING_MTK_LDVT   // Not using LDVT.
    //a. Allocate a buffer
    IMEM_DBG("ion_alloc \n");

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
	IMEM_DBG("ion_share \n");
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
        IMEM_ERR("Cannot map ion buffer. memID(0x%x)/size(0x%x)/va(0x%lx)",pInfo->memID,pInfo->size,(unsigned long)pInfo->virtAddr);
        return -1;
    }

    //
    IMEM_DBG("ionFd[0x%x]",pInfo->memID);
#endif
    IMEM_DBG("mID[0x%x]/size[0x%x]/VA[0x%lx] \n",pInfo->memID,pInfo->size,(unsigned long)pInfo->virtAddr);
    return 0;
}
//-----------------------------------------------------------------------------
MINT32 DipIMemDrvImp::freeVirtBuf(
    IMEM_BUF_INFO* pInfo)
{
    IMEM_DBG("mID[0x%x]/size[0x%x]/VA[0x%lx]/PA[0x%lx]",pInfo->memID,pInfo->size,(unsigned long)pInfo->virtAddr,(unsigned long)pInfo->phyAddr);
    IMEM_DRV_DELAY

#if defined (__ISP_USE_PMEM__)
    IMEM_DBG("pmem ID[0x%x]",pInfo->memID);
    //
    if( pInfo->virtAddr != 0 )
    {
        ::pmem_free(
            (MUINT8*)(pInfo->virtAddr),
            pInfo->size,
            pInfo->memID);
    }
#else
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
        IMEM_DBG("ionFd[0x%x]",pInfo->memID);
        //a. get handle of ION_IOC_SHARE from fd_data.fd
        IonBufFd = pInfo->memID;
        if(ion_import(
            mIonDrv,
            IonBufFd,
            &pIonHandle))
        {
            IMEM_ERR("ion_import fail, memID(0x%x)",IonBufFd);
            return -1;
        }
        //free for IMPORT ref cnt
        if(ion_free(
            mIonDrv,
            pIonHandle))
        {
            IMEM_ERR("ion_free fail");
            return -1;
        }
        ion_munmap(mIonDrv,(char*)pInfo->virtAddr, pInfo->size);
        ion_share_close(mIonDrv,pInfo->memID);

        //d. pair of ION_IOC_ALLOC
        if(ion_free(
            mIonDrv,
            pIonHandle))
        {
            IMEM_ERR("ion_free fail");
            return -1;
        }
    }
#else
    else{
        IMEM_ERR("ERROR:mem ID(0x%x)",pInfo->memID);
        return -1;
    }
#endif

#endif

    return 0;
}
//-----------------------------------------------------------------------------
MINT32  DipIMemDrvImp::mapPhyAddr(
    IMEM_BUF_INFO* pInfo)
{
    stIMEM_MAP_INFO map_info;

    //IMEM_DBG("memID[0x%x]/size[0x%x]/vAddr[0x%x],S/C(%d/%d)",pInfo->memID,pInfo->size,pInfo->virtAddr,pInfo->bufSecu,pInfo->bufCohe);
    IMEM_DRV_DELAY

    //check mapping
    if ( 0 ==  buf_map.count(pInfo->virtAddr) ) {
        IMEM_DBG(" NO Mapped");
        //
#if     defined(__ISP_USE_PMEM__)
            pInfo->phyAddr = (MUINTPTR)::pmem_get_phys(pInfo->memID);
#else
        if ( IMEM_MIN_ION_FD > pInfo->memID ) {
            //
            LOG_ERR("don't support m4u mapPhyAddr api, memID!!(%d)",pInfo->memID);
        }
#if defined(__ISP_USE_ION__)
        else /*if ( BUF_TYPE_ION == pInfo->type )*/  {
            //
            ion_user_handle_t pIonHandle;
            MINT32 IonBufFd;
            MINT32 err;
            //MINT32 ret = 0;
            //struct ion_handle_data handle_data;
            //struct ion_custom_data custom_data;
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
                IMEM_ERR("ion_import fail, memID(0x%x)",IonBufFd);
                return -1;
            }
            //
            mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
            mm_data.config_buffer_param.handle = (ion_user_handle_t)pIonHandle;//allocation_data.handle;
            mm_data.config_buffer_param.eModuleID = M4U_PORT_L9_IMG_IMGI_D1_MDP;    //define in mt_iommu_port.h
            mm_data.config_buffer_param.security = pInfo->bufSecu;
            mm_data.config_buffer_param.coherent = pInfo->bufCohe;

            err = ion_custom_ioctl(mIonDrv, ION_CMD_MULTIMEDIA, &mm_data);
            if(err == (-ION_ERROR_CONFIG_LOCKED))
            {
                IMEM_DBG("IOCTL[ION_IOC_CUSTOM] Double config after map phy address");
            }
            else if(err != 0)
            {
                IMEM_ERR("IOCTL[ION_IOC_CUSTOM] ION_CMD_MULTIMEDIA Config Buffer failed!");
                return -1;
            }
            //
            sys_data.sys_cmd = ION_SYS_GET_PHYS;
            sys_data.get_phys_param.handle = (ion_user_handle_t)pIonHandle;  //allocation_data.handle;
            if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM, &sys_data))
            {
                IMEM_ERR("IOCTL[ION_IOC_CUSTOM] Config Buffer failed!");
                return -1;
            }
            //
            pInfo->phyAddr = (MUINTPTR)(sys_data.get_phys_param.phy_addr);

            //free for IMPORT ref cnt
            if(ion_free(
                mIonDrv,
                pIonHandle))
            {
                IMEM_ERR("ion_free fail");
                return -1;
            }
            //
            IMEM_DBG("Physical address=0x%08X len=0x%lX", sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);

        }
#else
        else {
            IMEM_ERR("ERROR:mem ID(%d)",pInfo->memID);
            return -1;
        }
#endif

#endif
        //
        map_info.pAddr = (MUINT32)(pInfo->phyAddr);
        map_info.size = pInfo->size;
        //ION mapping should be fast enough,so for ION mapping everytime.
        //buf_map.insert(pair<int, stIMEM_MAP_INFO>(pInfo->virtAddr, map_info));


        //buf_map.insert(map<int, int> :: value_type(virtAddr, phyAddr)) ;
        //buf_map[virtAddr] = phyAddr;
    }
    else {
        IMEM_DBG(" Already Mapped");
        map_info = buf_map[pInfo->virtAddr];
        pInfo->phyAddr = (MUINTPTR)(map_info.pAddr);
    }
    //
    IMEM_DBG("mID(0x%x),size(0x%x),VA(0x%lx),PA(0x%lx),S/C(%d/%d)",pInfo->memID,pInfo->size,(unsigned long)pInfo->virtAddr,(unsigned long)pInfo->phyAddr,pInfo->bufSecu,pInfo->bufCohe);

    return 0;
}
//-----------------------------------------------------------------------------
MINT32  DipIMemDrvImp::unmapPhyAddr(
    IMEM_BUF_INFO* pInfo)
{
    stIMEM_MAP_INFO map_info;

    //mapping
#if 0
    if ( 0 !=  buf_map.count(pInfo->virtAddr) ) {
        //
        map_info = buf_map[pInfo->virtAddr];
#else
    if ( 1 ) {
        //
        map_info.size = pInfo->size;
        map_info.pAddr = (MUINT32)(pInfo->phyAddr);
#endif
        //
        IMEM_DBG("[IMEM_BUFFER] - unmap<memID(0x%x),size(0x%x),virtAddr(0x%lx),phyAddr(0x%lx)>",pInfo->memID,pInfo->size,(unsigned long)pInfo->virtAddr,(unsigned long)pInfo->phyAddr);
        IMEM_DRV_DELAY
        //
#if     defined(__ISP_USE_PMEM__)
        //NOT support for PMEM
        IMEM_DBG("BUF_TYPE_PMEM: free by virtMem free");
#else
        //
        if ( IMEM_MIN_ION_FD > pInfo->memID ) {
            LOG_ERR("don't support m4u unmapPhyAddr api, memID!!(%d)",pInfo->memID);
        }

        else
        {//using ion
        }
#endif
        //
        buf_map.erase(pInfo->virtAddr);

    }

    return 0;
}
//-----------------------------------------------------------------------------
MINT32 DipIMemDrvImp::cacheFlushAll(
    void)
{
    IMEM_ERR("DO NOT SUPPORT cache flush all, plz use cacheSyncbyRange");
    return 0;
}
//-----------------------------------------------------------------------------
MINT32 DipIMemDrvImp::cacheSyncbyRange(IMEM_CACHECTRL_ENUM ctrl,IMEM_BUF_INFO* pInfo)
{
#if defined (__ISP_USE_ION__)

    MINT32 err=0;
    //MUINT32 phyAddr = (MUINT32)(pInfo->phyAddr);
    IMEM_DBG("+, c/m/va/sz/pa(%d/0x%x/0x%lx/0x%x/0x%lx)",ctrl,pInfo->memID,(unsigned long)pInfo->virtAddr,pInfo->size,(unsigned long)pInfo->phyAddr);

    if( IMEM_MIN_ION_FD > pInfo->memID)
    {
        LOG_ERR("m4u don't support cachesync api, memID!!(%d)",pInfo->memID);
    }
    else
    {
        //a. get handle of ION_IOC_SHARE from IonBufFd and increase handle ref count
        ion_user_handle_t pIonHandle;
        MINT32 IonBufFd;
        IonBufFd = pInfo->memID;
        if(ion_import(mIonDrv,IonBufFd,&pIonHandle))
        {
            IMEM_ERR("ion_import fail, memId(0x%x)",IonBufFd);
            return -1;
        }

        //b. cache sync by range
        struct ion_sys_data sys_data;
        sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
        sys_data.cache_sync_param.va = (void*)pInfo->virtAddr;
        sys_data.cache_sync_param.size = pInfo->size;
        sys_data.cache_sync_param.handle=(ion_user_handle_t)pIonHandle;
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
                LOG_ERR("ERR cmd(%d)",ctrl);
                break;
        }
        if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM,&sys_data))
        {
            IMEM_ERR("CAN NOT DO SYNC, memID/(0x%x)",pInfo->memID);
            //decrease handle ref count if cache fail
            if(ion_free(mIonDrv,pIonHandle))
            {
                IMEM_ERR("ion_free fail");
                return -1;
            }
            return -1;
        }

        //c. decrease handle ref count
        if(ion_free(mIonDrv,pIonHandle))
        {
            IMEM_ERR("ion_free fail");
            return -1;
        }
    }
    IMEM_DBG("-");
    return err;
#endif
    return 0;
}
//-----------------------------------------------------------------------------

MINT32 DipIMemDrvImp::initSecureM4U(void)
{
	int m4u_fd;

#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_SEC_INIT _IOW(MTK_M4U_MAGICNO, 50, int)

	m4u_fd = open("/proc/m4u", O_RDONLY);
	if (m4u_fd == -1)
	{
		IMEM_ERR("open /proc/m4u failed! errno=%d, %s\n", errno, strerror(errno));
		goto FAIL;
	}

	IMEM_INF("%s +", __FUNCTION__);
	if (ioctl(m4u_fd, MTK_M4U_T_SEC_INIT, NULL))
	{
		IMEM_ERR("m4u ioctl sec_init failed! errno=%d, %s\n", errno, strerror(errno));
		goto FAIL;
	}
	else
	{
		IMEM_INF("m4u sec_init sucess\n");
	}
	IMEM_INF("%s -", __FUNCTION__);

	close(m4u_fd);
	return 0;

FAIL:
	// close(m4u_fd); coverity --> if not open, must not be closed
	return -1;
}
//-----------------------------------------------------------------------------
MINT32 DipIMemDrvImp::allocSecureBuf(
    IMEM_BUF_INFO* pInfo)
{
    if(pInfo->size <= 0)
    {
        IMEM_ERR("size 0!");
        return -1;
    }

    //int g_ion_fd;
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;
    //ion_user_handle_t g_ion_handle;
    struct ion_sys_data sys_data;
    uint32_t size = pInfo->size;
    uint32_t align = 0;//16;
    //uint32_t sec_handle;

    //g_ion_fd = ion_open();
    //if(g_ion_fd < 0) {
    //    LOG_ERR("ion_open failed!");
    //    return -1;
    //}
    if(ion_alloc(mIonDrv, size, align, ION_HEAP_MULTIMEDIA_TYPE_2D_FR_MASK, 0, &pIonHandle)) {
        IMEM_ERR("ion_alloc failed!");
        return -1;
    }

    if(ion_share(mIonDrv,pIonHandle,&IonBufFd)) {
        IMEM_ERR("ion_share fail");
        return -1;
    }

    pInfo->memID = IonBufFd; // Tianshu suggest to keep this fd

    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = pIonHandle;
    if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM, &sys_data)) {
        IMEM_ERR("ion_custom_ioctl failed!");
    } else {
        IMEM_DBG("Secure memory allocated OK, handle : 0x%08X, size: %lu", sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);
        pInfo->phyAddr = sys_data.get_phys_param.phy_addr;
        pInfo->virtAddr = sys_data.get_phys_param.phy_addr;
        pInfo->secHandle = (MUINT32)sys_data.get_phys_param.phy_addr;
    }

    return 0;
}
//-----------------------------------------------------------------------------
MINT32 DipIMemDrvImp::freeSecureBuf(
    IMEM_BUF_INFO* pInfo)
{

    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;
    IMEM_DBG("ionFd[0x%x]",pInfo->memID);
    //a. get handle of ION_IOC_SHARE from fd_data.fd
    IonBufFd = pInfo->memID;
    if(ion_import(mIonDrv,IonBufFd,&pIonHandle))
    {
        IMEM_ERR("ion_import fail, memID(0x%x)",IonBufFd);
        return -1;
    }
    //free for IMPORT ref cnt
    if(ion_free(mIonDrv,pIonHandle))
    {
        IMEM_ERR("ion_free fail");
        return -1;
    }
    else
    {
        IMEM_INF("Secure memory free OK, handle: 0x%08X\n", (uint32_t)pIonHandle);
    }
    //ion_munmap(mIonDrv,(char*)pInfo->virtAddr, pInfo->size);
    ion_share_close(mIonDrv,pInfo->memID);

    //d. pair of ION_IOC_ALLOC
    //if(ion_free(mIonDrv,pIonHandle))
    //{
    //    IMEM_ERR("ion_free fail");
    //    return -1;
    //}

    return 0;
}

//-----------------------------------------------------------------------------
MINT32 DipIMemDrvImp::setBufferdbgName(
    char const* bufferName,
    IMEM_BUF_INFO* pInfo)
{
    ion_user_handle_t pIonHandle;
    struct ion_mm_data data;
    int ret = 0;

    if(ion_import(
        mIonDrv,
        pInfo->memID,
        &pIonHandle))
    {
        IMEM_ERR("ion_import fail, dev(%d), memID(0x%x)", mIonDrv, pInfo->memID);
        return -1;
    }

    data.mm_cmd = ION_MM_SET_DEBUG_INFO;
    data.buf_debug_info_param.handle = pIonHandle;
    ::strncpy(data.buf_debug_info_param.dbg_name, bufferName, strlen(bufferName));
    data.buf_debug_info_param.dbg_name[strlen(bufferName)]='\0';
    
    data.buf_debug_info_param.value1 = 0;
    data.buf_debug_info_param.value2 = 0;
    data.buf_debug_info_param.value3 = 0;
    data.buf_debug_info_param.value4 = 0;

    ret = ion_custom_ioctl(mIonDrv, ION_CMD_MULTIMEDIA, &data);
    if(ret == (-ION_ERROR_CONFIG_LOCKED))
    {
        IMEM_WRN("ion_custom_ioctl Double config after map phy address, ionDev/memID(%d/0x%x), handle(0x%lx)",
            mIonDrv, pInfo->memID, (unsigned long)pIonHandle);
    }
    else if(ret != 0) {
        IMEM_ERR("ion_custom_ioctl MULTIMEDIA failed,  err(%d), ionDev/memID(%d/0x%x), handle(0x%lx)!",
            ret, mIonDrv, pInfo->memID, (unsigned long)pIonHandle);
    }

    //free for IMPORT ref cnt
    if(ion_free(
        mIonDrv,
        pIonHandle))
    {
        IMEM_ERR("ion_free fail, dev(%d), memID(0x%x), handle(0x%lx)", mIonDrv, pInfo->memID, (unsigned long)pIonHandle);
        ret = -1;
    }
    //
    
    return ret;
}