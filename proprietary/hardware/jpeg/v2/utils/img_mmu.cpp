/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

//#include <linux/ion.h>
#include <linux/ion_drv.h>
#include <ion.h>

#include "img_mmu.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "imgMmu"

//M4U_CLNTMOD_VENC    = 0,	//0
//
//M4U_CLNTMOD_VDEC       ,
//M4U_CLNTMOD_ROT        ,
//M4U_CLNTMOD_OVL        ,
//M4U_CLNTMOD_WDMA       ,
//M4U_CLNTMOD_RDMA       , //5
//
//M4U_CLNTMOD_CMDQ       ,
//M4U_CLNTMOD_DBI        ,
//M4U_CLNTMOD_G2D        ,
//M4U_CLNTMOD_JPGDEC     ,
//M4U_CLNTMOD_JPGENC     , //10
//
//M4U_CLNTMOD_VIP        ,
//M4U_CLNTMOD_DISP       ,
//M4U_CLNTMOD_VID        ,
//M4U_CLNTMOD_GDMA       ,
//M4U_CLNTMOD_IMG        , //15
//
//M4U_CLNTMOD_LSCI       ,
//M4U_CLNTMOD_FLKI       ,
//M4U_CLNTMOD_LCEI       ,
//M4U_CLNTMOD_LCSO       ,
//M4U_CLNTMOD_ESFKO      , //20
//
//M4U_CLNTMOD_AAO        ,
//
//M4U_CLNTMOD_AUDIO      ,
//
//
//M4U_CLNTMOD_UNKNOWN,
//M4U_CLNTMOD_MAX


#ifdef MTK_M4U_SUPPORT
bool imgMmu_create(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID)
{

   if(*ppM4uDrv == NULL)
    *ppM4uDrv = new MTKM4UDrv();

   //JPG_DBG("::[CREATE] enable m4u %p, ID %d, L:%d,  JD(9),JE(10),GD(14),IMG(15),VD(1)!! \n", *ppM4uDrv, mID, __LINE__);
//   (*ppM4uDrv)->m4u_enable_m4u_func(mID);
   //(*ppM4uDrv)->m4u_disable_m4u_func(mID);

   return true ;
}


bool imgMmu_cfg_port_mci(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, M4U_PORT_ID_ENUM pID)
{
    if(pM4uDrv == NULL){
       JPG_ERR("::[CONFIG] m4u is %p, mID %d, L:%d!! \n", pM4uDrv, mID, __LINE__);
       return false;
    }
    M4U_PORT_STRUCT m4uPort;
    m4uPort.ePortID = pID;
    m4uPort.Virtuality = 1;
    m4uPort.Security = 1;
    m4uPort.domain = 3;
    m4uPort.Distance = 1;
    m4uPort.Direction = 0;
    if(M4U_STATUS_OK != pM4uDrv->m4u_config_port(&m4uPort))
    {
        JPG_ERR("Can not config m4u port\n");
        return false;
    }
   return true ;
}


bool imgMmu_cfg_port(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, M4U_PORT_ID_ENUM pID)
{
    if(pM4uDrv == NULL){
       JPG_ERR("::[CONFIG] m4u is %p, mID %d, L:%d!! \n", pM4uDrv, mID, __LINE__);
       return false;
    }

    M4U_PORT_STRUCT m4uPort;

    m4uPort.ePortID = pID;
    m4uPort.Virtuality = 1;
    m4uPort.Security = 0;
    m4uPort.domain = 3;
    m4uPort.Distance = 1;
    m4uPort.Direction = 0;

    if(M4U_STATUS_OK != pM4uDrv->m4u_config_port(&m4uPort))
    {
        JPG_ERR("Can not config m4u port\n");
        //(*ppM4uDrv)->m4u_invalid_tlb_range(mID, *pa, *pa + size - 1);
        //(*ppM4uDrv)->m4u_dealloc_mva(mID, va, size, *pa);
        //delete *ppM4uDrv;
        //*ppM4uDrv = NULL;
        return false;
    }

   return true ;
}


bool imgMmu_reset(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID)
{
    if(*ppM4uDrv == NULL){
       JPG_ERR("::[RESET] m4u is %p, mID %d, L:%d!! \n", *ppM4uDrv, mID, __LINE__);
       return false;
    }
    return true;
}


bool imgMmu_alloc_pa_mci(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 size, JUINT32 *pa)
{
   unsigned int prot = M4U_PROT_READ | M4U_PROT_WRITE;

    if(*ppM4uDrv == NULL || size == 0 || va == 0){
       JPG_ERR("::[ALLOC] m4u is %p, mID %d, size %x, va %lx, L:%d!! \n", *ppM4uDrv, mID, size, (unsigned long)va,__LINE__);
       return false;
    }
    {
      //security = 1;  //cache_coherent = 1;
        prot |= M4U_PROT_SEC|M4U_PROT_CACHE;
    }

    if(M4U_STATUS_OK != (*ppM4uDrv)->m4u_alloc_mva(mID, (unsigned long)va, size, prot, M4U_FLAGS_SEQ_ACCESS, pa) )
    {
        JPG_ERR("::[ALLOC] mID %d alloc m4u va %lx, size %x, L:%d!! \n",  mID, (unsigned long)va, size, __LINE__);
        delete *ppM4uDrv;
        *ppM4uDrv = NULL;
        *pa = 0;
        return false;
    }

    JPG_ERR("::[ALLOC] mID %d alloc m4u va %lx, size %x, mva %x, L:%d!! \n",  mID, (unsigned long)va, size, *pa,__LINE__);
//    M4U_PORT_STRUCT m4uPort;
//
//    m4uPort.ePortID = pID;
//    m4uPort.Virtuality = 1;
//    m4uPort.Security = 0;
//    m4uPort.domain = 3;
//    m4uPort.Distance = 1;
//    m4uPort.Direction = 0;
//
//    if(M4U_STATUS_OK != (*ppM4uDrv)->m4u_config_port(&m4uPort))
//    {
//        JPG_ERR("Can not config m4u port\n");
//        (*ppM4uDrv)->m4u_invalid_tlb_range(mID, *pa, *pa + size - 1);
//        (*ppM4uDrv)->m4u_dealloc_mva(mID, va, size, *pa);
//        delete *ppM4uDrv;
//        *ppM4uDrv = NULL;
//        return false;
//    }
   return true ;
}


bool imgMmu_alloc_pa(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 size, JUINT32 *pa)
{
   unsigned int prot = M4U_PROT_READ | M4U_PROT_WRITE;

    if(*ppM4uDrv == NULL || size == 0 || va == 0){
       JPG_ERR("::[ALLOC] m4u is %p, mID %d, size %x, va %lx, L:%d!! \n", *ppM4uDrv, mID, size, (unsigned long)va,__LINE__);
       return false;
    }
//    if(flag & IMGMMU_MASK_MCI){
//      security = 1;
//      cache_coherent = 1;
//    }
    if(M4U_STATUS_OK != (*ppM4uDrv)->m4u_alloc_mva(mID, (unsigned long)va, size, prot, M4U_FLAGS_SEQ_ACCESS, pa) )
    {
        JPG_ERR("::[ALLOC] mID %d alloc m4u va %lx, size %x, L:%d!! \n",  mID, (unsigned long)va, size, __LINE__);
        delete *ppM4uDrv;
        *ppM4uDrv = NULL;
        *pa = 0;
        return false;
    }

    JPG_DBG("::[ALLOC] mID %d alloc m4u va %lx, size %x, mva %x, L:%d!!\n",  mID, (unsigned long)va, size, *pa,__LINE__);

    return true ;
}


bool imgMmu_sync(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 mva, JUINT32 size, JUINT32 isWrite )
{
  if(pM4uDrv == NULL){
     JPG_ERR("::[SYNC] m4u is %p, mID %d, L:%d!! \n", pM4uDrv, mID, __LINE__);
     return false;
  }

#ifdef ION_M4U_USE_MAP_API
  if( isWrite ) // modify by K, need to modify correct mva for last param.
  {
      if (-1 == pM4uDrv->m4u_dma_map_area(mID, M4U_DMA_FROM_DEVICE, (unsigned long)va, size, mva))
      return false;
  }
  else
  {
      if (-1 == pM4uDrv->m4u_dma_map_area(mID, M4U_DMA_TO_DEVICE, (unsigned long)va, size, mva))
      return false;
  }
#else
  if( isWrite ) // modify by K, need to modify correct mva for last param.
  {
      if (-1 == pM4uDrv->m4u_cache_sync(mID, M4U_CACHE_INVALID_BY_RANGE, (unsigned long)va, size, mva))
          return false;
  }
  else
  {
      if (-1 == pM4uDrv->m4u_cache_sync(mID, M4U_CACHE_FLUSH_BY_RANGE, (unsigned long)va, size, mva))
          return false;
  }
#endif
  return true;
}


void imgMmu_dealloc_pa(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, void* va, JUINT32 size, JUINT32 *pa)
{

      //JPG_DBG("::[DEALLOC] mID %d alloc m4u va %x, size %x, mva %x, L:%d!! \n",  mID, va, size, *pa, __LINE__);
      if(pM4uDrv == NULL){
         JPG_ERR("::[DEALLOC] m4u is %p, mID %d, L:%d!! \n", pM4uDrv, mID, __LINE__);
         return ;
      }

      if(*pa != 0){
//        pM4uDrv->m4u_invalid_tlb_range(mID, *pa, *pa + size - 1);
        pM4uDrv->m4u_dealloc_mva(mID, (unsigned long)va, size, *pa);
        *pa = 0;
      }
}


//for ION flow, only map virtual pa tlb
bool imgMmu_pa_map_tlb(MTKM4UDrv **ppM4uDrv, M4U_MODULE_ID_ENUM mID, JUINT32 size, JUINT32 *pa)
{
    if(*ppM4uDrv == NULL || size == 0 ){
       JPG_ERR("::[MAP] m4u is %p, mID %d, size %x, L:%d!! \n", *ppM4uDrv, mID, size,__LINE__);
       return false;
    }
    //if(mID == M4U_CLNTMOD_JPGENC) mID = M4U_CLNTMOD_IMG ;
    return true ;
}


//for ION flow, only unmap virtual pa tlb
void imgMmu_pa_unmap_tlb(MTKM4UDrv *pM4uDrv, M4U_MODULE_ID_ENUM mID, JUINT32 size, JUINT32 *pa)
{
    if( pM4uDrv == NULL){
        JPG_ERR("::[UNMAP] m4u is %p, mID %d, L:%d!! \n", pM4uDrv, mID, __LINE__);
        return ;
    }

    if(*pa != 0){
        //if(mID == M4U_CLNTMOD_JPGENC) mID = M4U_CLNTMOD_IMG ;
        //pM4uDrv->m4u_invalid_tlb_range(mID, *pa, *pa + size - 1);
        *pa = 0;
    }
}
#endif


bool imgIon_open(JINT32 *ionFD )
{

    *ionFD = mt_ion_open("img_mmu.cpp");  //open("/dev/ion", O_RDONLY);
    if( *ionFD < 0)
    {
      JPG_ERR("IMG_ION: open fd fail!!\n");
      return false;
    }

    return true;
}


bool imgIon_freeVA(void **va, JUINT32 size)
{
    if(*va != NULL)
        munmap(*va, size);
    *va = NULL;

    return true;
}


bool imgIon_close(JINT32 ionFD)
{
    close(ionFD);
    return true;
}


bool imgIon_getPA(JINT32 ionFD, JINT32 bufFD, M4U_MODULE_ID_ENUM mID, void *va, JUINT32 size, JINT32 *bufHandle, JUINT32 *pa)
{
    // get handler
    ion_fd_data fd_data;
    fd_data.fd = bufFD;
    int err = 0;

    if(ioctl(ionFD, ION_IOC_IMPORT, &fd_data))
    {
      JPG_ERR("IMG_ION: IMPORT fail!!\n");
      return false;
    }

    *bufHandle = fd_data.handle;

    // config module ID to get physical Address
    ion_custom_data custom_data;
    ion_mm_data mm_data;

    custom_data.cmd = ION_CMD_MULTIMEDIA;
    custom_data.arg = (unsigned long)&mm_data;

    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = fd_data.handle;
    mm_data.config_buffer_param.eModuleID = mID;
    mm_data.config_buffer_param.security = 0;
    mm_data.config_buffer_param.coherent = 0;

    err = ioctl(ionFD, ION_IOC_CUSTOM, &custom_data) ;

    if(err != 0 && err != (-ION_ERROR_CONFIG_LOCKED) )
    {
      JPG_ERR("IMG_ION: CONFIG module fail (err_no = %x)!!\n", err);
      return false;
    }

    // get physical address
    ion_sys_data    sys_data;

    custom_data.cmd = ION_CMD_SYSTEM;
    custom_data.arg = (unsigned long)&sys_data;

    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = fd_data.handle;

    if(ioctl(ionFD, ION_IOC_CUSTOM, &custom_data))
    {
      JPG_ERR("IMG_ION: get physical fail!!\n");
      return false;
    }
    *pa = sys_data.get_phys_param.phy_addr;

    JPG_DBG("::[ION_PA] ionFD %d, bufFD %d, mID %d, va %lx, size %x, pa %x, L:%d!!\n", ionFD,bufFD, mID, (unsigned long)va, size, *pa, __LINE__);

    return true ;
}


bool imgIon_getVA(JINT32 bufFD, JUINT32 size, void **va)
{
    //JPG_DBG("::[ION_VA] bufFD %d, va %x, size %x, L:%d!!\n", bufFD,*va, size, __LINE__);

    // get virtual address
    *va = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, bufFD, 0);
    if((*va == NULL) || (*va == (void*)(-1)))
    {
      JPG_ERR("IMG_ION: MMAP fail, fd %d, va %lx, size %x!!\n", bufFD, (unsigned long)*va, size);
      *va = NULL;
      return false ;
    }

    return true ;
}

#if 0 //new VA interface if needed
bool imgIon_getVA(JINT32 ionDevFD,JINT32 bufFD, JUINT32 size, unsigned long *va)
{

    // get virtual address
    *va = (unsigned long)ion_mmap(ionDevFD,NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, bufFD, 0);
    if((*va == NULL) || (*va == (unsigned long)(void *)-1))
    {
        JPG_ERR("IMG_ION: UNMMAP fail, fd %d, size %x!!\n", bufFD, size);
        return false ;
    }

    return true ;
}

bool imgIon_unMapVA(JINT32 ionDevFD, JUINT32 size, unsigned long va)
{
    int ret;
    // get virtual address
    if (va == 0)
    {
        JPG_ERR("IMG_ION: imgIon_unMapVA fail,invalid VA!!\n");
        return false;
    }
    ret = ion_munmap(ionDevFD, (void *)va, size);
    if(ret < 0)
    {
        JPG_ERR("IMG_ION: ion_munmap fail\n");
        return false ;
    }

    return true ;
}
#endif


bool imgIon_sync(JINT32 ionFD, JINT32 bufHandle, JUINT32 isWrite)
{
#if 0
#ifdef ION_M4U_USE_MAP_API
    if (isWrite == 1)
        ion_dma_map_area(ionFD, bufHandle, ION_DMA_FROM_DEVICE);
    else
        ion_dma_map_area(ionFD, bufHandle, ION_DMA_TO_DEVICE);
#else
    ion_custom_data custom_data;
    ion_sys_data    sys_data;

    custom_data.cmd = ION_CMD_SYSTEM;
    custom_data.arg = (unsigned long)&sys_data;
    sys_data.sys_cmd = ION_SYS_CACHE_SYNC;
    //bobule workaround pdk build error, needing review
    sys_data.cache_sync_param.handle = bufHandle;
    sys_data.cache_sync_param.sync_type = ION_CACHE_FLUSH_ALL;//ION_CACHE_FLUSH_BY_RANGE;

    //JPG_DBG("::[ION_SYNC] ionFD %d, handle %p, L:%d!!\n", ionFD, bufHandle,  __LINE__);

    ioctl(ionFD, ION_IOC_CUSTOM, &custom_data);
#endif
#endif
    return true;
}

#ifdef ION_M4U_USE_MAP_API
bool imgIon_syncDone(JINT32 ionFD, JINT32 bufHandle, JUINT32 isWrite)
{
#if 0
    if (isWrite == 1)
        ion_dma_unmap_area(ionFD, bufHandle, ION_DMA_FROM_DEVICE);
    else
        ion_dma_unmap_area(ionFD, bufHandle, ION_DMA_TO_DEVICE);
#endif
    return true;
}
#endif

