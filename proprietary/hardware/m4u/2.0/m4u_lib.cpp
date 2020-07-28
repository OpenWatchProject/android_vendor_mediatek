/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
/*#include <cutils/log.h>*/
#include <log/log.h>
#include "m4u_lib_v2.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "stdlib.h"
#include <cutils/properties.h>

#undef LOG_TAG
#define LOG_TAG "M4U_L"

#define MTKM4UMSG
#ifdef MTKM4UMSG
  #define M4UMSG(...) \
        do { \
            ALOGE(__VA_ARGS__); \
        } while (0)
#else
  #define M4UMSG(...)
#endif

#ifdef M4U_DBG
  #define M4UINFO(...) \
        do { \
            ALOGI(__VA_ARGS__); \
        }while (0)
#else
  #define M4UINFO(...)
#endif

//#define __DUMP_BACKTRACE_ON_ERROR__
#ifdef __DUMP_BACKTRACE_ON_ERROR__
extern "C" void rtt_dump_caller_backtrace(const char* tag);
extern "C" int rtt_dump_backtrace(pid_t pid, pid_t tid, const char* file_path);
static void m4u_dump_backtrace(void)
{
    char name[35] = "/data/m4u_dump";
    //sprintf(name, "/data/m4u_dump_%d_%d.txt", getpid(), gettid());
    M4UMSG("m4u dump back trace when error============> \n");
    rtt_dump_backtrace(getpid(), gettid(), name);

    FILE *fp = fopen(name, "r");
    if(fp)
    {
        char tmp[101] = "";
        int cnt=0;
        while( (fgets(tmp, 100, fp) != NULL) && (cnt<500) )
        {
            cnt++;
            M4UMSG("[%d]: %s", cnt, tmp);
        }
       fclose(fp);
    }
    //unlink(name);
}
#else
static void m4u_dump_backtrace(void)
{
}
#endif


/******************************************************************************
*
*******************************************************************************/
MTKM4UDrv::MTKM4UDrv(void)
{

    mFileDescriptor = -1;

    mFileDescriptor = open("/proc/m4u", O_RDONLY);
    if(mFileDescriptor<0)
    {
        mFileDescriptor = -1;
        M4UMSG("Open file failed mFileDescriptor=%d, error=%d :%s", mFileDescriptor, errno, strerror(errno));
        m4u_dump_backtrace();
    }
}

MTKM4UDrv::~MTKM4UDrv(void)
{
    int fd;
    fd = mFileDescriptor;
    mFileDescriptor = -1;
    if(-1!=fd)
    {
    	close(fd);
    }
}


/**
 * @brief :
 * @param
 * @return
 */
int MTKM4UDrv::m4u_power_on(M4U_PORT_ID port)
{

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_power_on failed \n");
        m4u_dump_backtrace();
        return -1;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_POWER_ON, &port) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_POWER_ON fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief :
 * @param
 * @return
 */
int MTKM4UDrv::m4u_power_off(M4U_PORT_ID port)
{

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_power_off failed \n");
        m4u_dump_backtrace();
        return -1;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_POWER_OFF, &port) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_POWER_OFF fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}


int MTKM4UDrv::m4u_alloc_mva(M4U_PORT_ID port,
                  unsigned long va, unsigned int size,
                  unsigned int prot, unsigned int flags,
				  unsigned int *pMva)
{
    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.port = port;
    m4u_module.BufAddr = va;
    m4u_module.BufSize = size;
    m4u_module.prot = prot;
    m4u_module.flags = flags;

    m4u_module.MVAStart = *pMva;

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_alloc_mva failed fd=%d\n", mFileDescriptor);
        m4u_dump_backtrace();
        return -1;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_ALLOC_MVA, &m4u_module) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_ALLOC_MVA fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        *pMva = 0;
	    M4UMSG("ioctl MTK_M4U_T_ALLOC_MVA fail:va = %lx,size=%x,port=%d\n ", va, size, port);

        return -1;
    }
    else
    {
        *pMva = m4u_module.MVAStart;
		/*M4UINFO("ioctl MTK_M4U_T_ALLOC_MVA :va = 0x%lx,size = 0x%x,mva = 0x%x,port = %d\n ", va, size, *pMva, port);*/
        return 0;
    }
}


int MTKM4UDrv::m4u_dealloc_mva(M4U_PORT_ID port,
                            unsigned long va, unsigned int size,
                            unsigned int mva)
{
    M4U_MOUDLE_STRUCT m4u_module;
    m4u_module.port = port;
    m4u_module.MVAStart = mva;

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_dealloc_mva failed \n");
        m4u_dump_backtrace();
        return -1;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_DEALLOC_MVA, &m4u_module) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_DEALLOC_MVA fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
		M4UMSG("ioctl MTK_M4U_T_DEALLOC_MVA fail:va = 0x%lx,size = 0x%x,mva = 0x%x,port = %d\n ", va, size, mva, port);

        return -1;
    }
    else
    {
    	/*M4UINFO("ioctl MTK_M4U_T_DEALLOC_MVA :va = 0x%lx,size =0x%x,mva = 0x%x, port=%d\n ", va, size, mva, port);*/
        return 0;
    }
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_insert_wrapped_range(M4U_MODULE_ID_ENUM eModuleID,
                                  M4U_PORT_ID_ENUM portID,
								  const unsigned int MVAStart,
								  const unsigned int MVAEnd)
{
	ALOGV("error!! use m4u_insert_wrapped_range(%d, %d, %d, %d)!!\n", eModuleID, portID, MVAStart, MVAEnd);
	return 0;
}
M4U_STATUS_ENUM MTKM4UDrv::m4u_insert_tlb_range(M4U_MODULE_ID_ENUM eModuleID,
										unsigned int MVAStart,
										const unsigned int MVAEnd,
										M4U_RANGE_PRIORITY_ENUM ePriority,
										unsigned int entryCount)
{
	ALOGV("error!! use m4u_insert_tlb_range(%d, %d, %d, %d, %d)!!\n", 
		   eModuleID, MVAStart, MVAEnd, ePriority, entryCount);
	return 0;
}
M4U_STATUS_ENUM MTKM4UDrv::m4u_invalid_tlb_range(M4U_MODULE_ID_ENUM eModuleID,
										  unsigned int MVAStart,
										  unsigned int MVAEnd)
{
	ALOGV("error!! use m4u_invalid_tlb_range(%d, %d, %d)!!\n", eModuleID, MVAStart, MVAEnd);
	return 0;
}
M4U_STATUS_ENUM MTKM4UDrv::m4u_invalid_tlb_all(M4U_MODULE_ID_ENUM eModuleID)
{
	ALOGV("error!! use m4u_invalid_tlb_all(%d)!!\n", eModuleID);
	return 0;
}
M4U_STATUS_ENUM MTKM4UDrv::m4u_manual_insert_entry(M4U_MODULE_ID_ENUM eModuleID,
									unsigned int EntryMVA,
									bool Lock)
{
	ALOGV("error!! use m4u_manual_insert_entry(%d, %d, %d)!!\n", eModuleID, EntryMVA, Lock);
	return 0;
}

///> native
int MTKM4UDrv::m4u_config_port(M4U_PORT_STRUCT* pM4uPort)
{

    if(NULL==pM4uPort)
    {
        M4UMSG("m4u_config_port failed, input M4U_PORT_STRUCT* is null! \n");
        m4u_dump_backtrace();
        return -1;
    }

    if(mFileDescriptor<0)
    {
        M4UMSG("m4u_config_port failed \n");
        m4u_dump_backtrace();
        return -1;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_CONFIG_PORT, pM4uPort) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_CONFIG_PORT fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}

void MTKM4UDrv::m4u_port_array_init(struct m4u_port_array * port_array)
{
    memset(port_array, 0, sizeof(struct m4u_port_array));
}

int MTKM4UDrv::m4u_port_array_add(struct m4u_port_array *port_array,
    int port, int m4u_en, int secure)
{
    if(port>=M4U_PORT_NR)
    {
        M4UMSG("error: port_array_add, port=%d, v(%d), s(%d)\n", port, m4u_en, secure);
        return -1;
    }
    port_array->ports[port] = M4U_PORT_ATTR_EN;
    if(m4u_en)
        port_array->ports[port] |= M4U_PORT_ATTR_VIRTUAL;
    if(secure)
        port_array->ports[port] |= M4U_PORT_ATTR_SEC;
    return 0;
}

int MTKM4UDrv::m4u_config_port_array(struct m4u_port_array * port_array)
{
    if(mFileDescriptor<0)
    {
        M4UMSG("m4u_config_port failed \n");
        m4u_dump_backtrace();
        return -1;
    }
    if(ioctl(mFileDescriptor, MTK_M4U_T_CONFIG_PORT_ARRAY, port_array) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_CONFIG_PORT fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_cache_sync(M4U_MODULE_ID_ENUM eModuleID,
	                                    M4U_CACHE_SYNC_ENUM eCacheSync,
		                                  unsigned int BufAddr,
		                                  unsigned int BufSize)
{
    M4UMSG("error!! use old m4u_cache_sync interface(%d, %d, %d, %d)!!\n", eModuleID, eCacheSync, BufAddr, BufSize);
    return -1;
}

int MTKM4UDrv::m4u_cache_sync(M4U_PORT_ID port,
	                                    M4U_CACHE_SYNC_ENUM eCacheSync,
		                                  unsigned long va,
		                                  unsigned int size,
		                                  unsigned int mva)
{
    M4U_CACHE_STRUCT m4u_cache;

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_cache_sync failed \n");
        m4u_dump_backtrace();
        return -1;
    }

    m4u_cache.port = port;
    m4u_cache.eCacheSync = eCacheSync;
    m4u_cache.va = va;
    m4u_cache.size = size;
    m4u_cache.mva = mva;

    if(ioctl(mFileDescriptor, MTK_M4U_T_CACHE_SYNC, &m4u_cache) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_CACHE_SYNC fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}


int MTKM4UDrv::m4u_cache_flush_all(M4U_PORT_ID port)
{

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_cache_sync port %d failed\n", port);
        m4u_dump_backtrace();
        return -1;
    }

    if(ioctl(mFileDescriptor, MTK_M4U_T_CACHE_FLUSH_ALL, NULL) < 0)
    {
        M4UMSG(" ioctl MTK_M4U_T_CACHE_FLUSH_ALL fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_cache_flush_by_range(M4U_PORT_ID port,
										unsigned long va,
										unsigned int size,
										unsigned int mva)
{
    M4U_DMA_STRUCT m4u_dma;

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_cache_flush_by_range failed \n");
        m4u_dump_backtrace();
        return -1;
    }

    m4u_dma.port = port;
    m4u_dma.eDMAType = M4U_DMA_FLUSH_BY_RANGE;
    m4u_dma.va = va;
    m4u_dma.size = size;
    m4u_dma.mva = mva;

    if(ioctl(mFileDescriptor, MTK_M4U_T_DMA_OP, &m4u_dma) < 0)
    {
        M4UMSG("ioctl MTK_M4U_T_DMA_MAP_AREA fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_dma_map_area(M4U_PORT_ID port,
										M4U_DMA_DIR eDMADir,
										unsigned long va,
										unsigned int size,
										unsigned int mva)
{
    M4U_DMA_STRUCT m4u_dma;

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_cache_sync failed \n");
        m4u_dump_backtrace();
        return -1;
    }

    m4u_dma.port = port;
    m4u_dma.eDMAType = M4U_DMA_MAP_AREA;
    m4u_dma.eDMADir = eDMADir;
    m4u_dma.va = va;
    m4u_dma.size = size;
    m4u_dma.mva = mva;

    if(ioctl(mFileDescriptor, MTK_M4U_T_DMA_OP, &m4u_dma) < 0)
    {
        M4UMSG("ioctl MTK_M4U_T_DMA_MAP_AREA fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}

M4U_STATUS_ENUM MTKM4UDrv::m4u_dma_unmap_area(M4U_PORT_ID port,
										M4U_DMA_DIR eDMADir,
										unsigned long va,
										unsigned int size,
										unsigned int mva)
{
    M4U_DMA_STRUCT m4u_dma;

    if(mFileDescriptor<0 )
    {
        M4UMSG("m4u_cache_sync failed \n");
        m4u_dump_backtrace();
        return -1;
    }

    m4u_dma.port = port;
    m4u_dma.eDMAType = M4U_DMA_UNMAP_AREA;
    m4u_dma.eDMADir = eDMADir;
    m4u_dma.va = va;
    m4u_dma.size = size;
    m4u_dma.mva = mva;

    if(ioctl(mFileDescriptor, MTK_M4U_T_DMA_OP, &m4u_dma) < 0)
    {
        M4UMSG("ioctl MTK_M4U_T_DMA_UNMAP_AREA fail! %d, %s \n", errno, strerror(errno));
        m4u_dump_backtrace();
        return -1;
    }
    else
    {
        return 0;
    }
}


bool MTKM4UDrv::m4u_enable_m4u_func(M4U_MODULE_ID_ENUM eModuleID)
{
	ALOGV("m4u_enable_m4u_func %d\n", eModuleID);
	return 0;
}

bool MTKM4UDrv::m4u_disable_m4u_func(M4U_MODULE_ID_ENUM eModuleID)
{
	ALOGV("m4u_disable_m4u_func %d\n", eModuleID);
	return 0;
}

