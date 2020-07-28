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

#ifndef _MTK_M4U_LIB_V2_H
#define _MTK_M4U_LIB_V2_H

#include <linux/ioctl.h>
#include "m4u_lib_priv.h"

typedef int M4U_PORT_ID;
typedef int M4U_MODULE_ID_ENUM;
typedef int M4U_PORT_ID_ENUM;


#define M4U_PROT_READ   (1<<0)
#define M4U_PROT_WRITE  (1<<1)
#define M4U_PROT_CACHE  (1<<2)
#define M4U_PROT_SHARE  (1<<3)
#define M4U_PROT_SEC    (1<<4)

#define M4U_FLAGS_SEQ_ACCESS (1<<0)
#define M4U_FLAGS_FIX_MVA   (1<<1)
#define M4U_FLAGS_SEC_SHAREABLE   (1<<2)
#define M4U_FLAGS_START_FROM   (1<<3)

typedef enum
{
	RT_RANGE_HIGH_PRIORITY=0,
	SEQ_RANGE_LOW_PRIORITY=1
} M4U_RANGE_PRIORITY_ENUM;

typedef enum
{
	M4U_DMA_READ_WRITE = 0,
	M4U_DMA_READ = 1,
	M4U_DMA_WRITE = 2,
	M4U_DMA_NONE_OP = 3,

} M4U_DMA_DIR_ENUM;

typedef struct _M4U_PORT
{
	M4U_PORT_ID ePortID;		   //hardware port ID, defined in M4U_PORT_ID
	unsigned int Virtuality;
	unsigned int Security;
    unsigned int domain;            //domain : 0 1 2 3
	unsigned int Distance;
	unsigned int Direction;         //0:- 1:+
}M4U_PORT_STRUCT;

struct m4u_port_array
{
    #define M4U_PORT_ATTR_EN 		(1<<0)
    #define M4U_PORT_ATTR_VIRTUAL 	(1<<1)
    #define M4U_PORT_ATTR_SEC	 	(1<<2)
    unsigned char ports[M4U_PORT_NR];
};

typedef enum
{
	ROTATE_0=0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270,
	ROTATE_HFLIP_0,
	ROTATE_HFLIP_90,
	ROTATE_HFLIP_180,
	ROTATE_HFLIP_270
} M4U_ROTATOR_ENUM;

typedef struct _M4U_PORT_ROTATOR
{
	M4U_PORT_ID_ENUM ePortID;		   // hardware port ID, defined in M4U_PORT_ID_ENUM
	unsigned int Virtuality;
	unsigned int Security;
	// unsigned int Distance;      // will be caculated actomatically inside M4U driver
	// unsigned int Direction;
  unsigned int MVAStart;
  unsigned int BufAddr;
  unsigned int BufSize;
  M4U_ROTATOR_ENUM angle;
}M4U_PORT_STRUCT_ROTATOR;


typedef struct _M4U_MOUDLE
{
	M4U_PORT_ID port;
	unsigned long BufAddr;
	unsigned int BufSize;
	unsigned int prot;
	unsigned int MVAStart;
	unsigned int MVAEnd;
    unsigned int flags;

}M4U_MOUDLE_STRUCT;

typedef enum
{
    M4U_CACHE_CLEAN_BY_RANGE,
    M4U_CACHE_INVALID_BY_RANGE,
    M4U_CACHE_FLUSH_BY_RANGE,

    M4U_CACHE_CLEAN_ALL,
    M4U_CACHE_INVALID_ALL,
    M4U_CACHE_FLUSH_ALL,
} M4U_CACHE_SYNC_ENUM;

#define    M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM    M4U_CACHE_FLUSH_BY_RANGE
#define    M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM   M4U_CACHE_FLUSH_BY_RANGE
#define    M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM    M4U_CACHE_CLEAN_BY_RANGE
#define    M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM  M4U_CACHE_INVALID_BY_RANGE


typedef struct _M4U_CACHE
{
    M4U_PORT_ID port;
    M4U_CACHE_SYNC_ENUM eCacheSync;
    unsigned long va;
    unsigned int size;
    unsigned int mva;
}M4U_CACHE_STRUCT;

typedef enum
{
    M4U_DMA_MAP_AREA,
    M4U_DMA_UNMAP_AREA,
    M4U_DMA_FLUSH_BY_RANGE,
} M4U_DMA_TYPE;

typedef enum
{
	M4U_DMA_FROM_DEVICE,
	M4U_DMA_TO_DEVICE,
	M4U_DMA_BIDIRECTIONAL,
} M4U_DMA_DIR;

typedef struct _M4U_DMA
{
    M4U_PORT_ID port;
    M4U_DMA_TYPE eDMAType;
    M4U_DMA_DIR eDMADir;
    unsigned long va;
    unsigned int size;
    unsigned int mva;
}M4U_DMA_STRUCT;

typedef struct _M4U_MAU
{
    M4U_PORT_ID port;
    bool write;
    unsigned int mva;
    unsigned int size;
    bool enable;
    bool force;
}M4U_MAU_STRUCT;

typedef struct _M4U_TF
{
    M4U_PORT_ID port;
    bool fgEnable;
}M4U_TF_STRUCT;


enum _M4U_STATUS
{
	M4U_STATUS_OK = 0,
	M4U_STATUS_INVALID_CMD,
	M4U_STATUS_INVALID_HANDLE,
	M4U_STATUS_NO_AVAILABLE_RANGE_REGS,
	M4U_STATUS_KERNEL_FAULT,
	M4U_STATUS_MVA_OVERFLOW,
	M4U_STATUS_INVALID_PARAM
};

typedef int M4U_STATUS_ENUM;
class MTKM4UDrv
{

private:
    int mFileDescriptor;

public:
    MTKM4UDrv(void);
    ~MTKM4UDrv(void);

    int m4u_power_on(M4U_PORT_ID port);
    int m4u_power_off(M4U_PORT_ID port);

    int m4u_alloc_mva(M4U_PORT_ID port,
                  unsigned long va, unsigned int size,
                  unsigned int prot, unsigned int flags,
				  unsigned int *pMva);

    int m4u_dealloc_mva(M4U_PORT_ID port,
                            unsigned long va, unsigned int size,
                            unsigned int mva);

    M4U_STATUS_ENUM m4u_insert_wrapped_range(M4U_MODULE_ID_ENUM eModuleID,
                  M4U_PORT_ID_ENUM portID,
								  const unsigned int MVAStart,
								  const unsigned int MVAEnd); //0:disable, 1~4 is valid

    M4U_STATUS_ENUM m4u_insert_tlb_range(M4U_MODULE_ID_ENUM eModuleID,
		                          unsigned int MVAStart,
		                          const unsigned int MVAEnd,
		                          M4U_RANGE_PRIORITY_ENUM ePriority,
		                          unsigned int entryCount);

    M4U_STATUS_ENUM m4u_invalid_tlb_range(M4U_MODULE_ID_ENUM eModuleID,
		                          unsigned int MVAStart,
		                          unsigned int MVAEnd);

    M4U_STATUS_ENUM m4u_manual_insert_entry(M4U_MODULE_ID_ENUM eModuleID,
		                          unsigned int EntryMVA,
		                          bool Lock);
    M4U_STATUS_ENUM m4u_invalid_tlb_all(M4U_MODULE_ID_ENUM eModuleID);

    int m4u_config_port(M4U_PORT_STRUCT* pM4uPort);
    void m4u_port_array_init(struct m4u_port_array * port_array);
    int m4u_port_array_add(struct m4u_port_array *port_array, int port, int m4u_en, int secure);
    int m4u_config_port_array(struct m4u_port_array * port_array);

    int m4u_cache_sync(M4U_PORT_ID port,
		                          M4U_CACHE_SYNC_ENUM eCacheSync,
		                          unsigned long BufAddr,
		                          unsigned int BufSize,
		                          unsigned int mva);

    M4U_STATUS_ENUM m4u_cache_sync(M4U_MODULE_ID_ENUM eModuleID,
		                          M4U_CACHE_SYNC_ENUM eCacheSync,
		                          unsigned int BufAddr,
		                          unsigned int BufSize);

    M4U_STATUS_ENUM m4u_reset_mva_release_tlb(M4U_MODULE_ID_ENUM eModuleID);

    M4U_STATUS_ENUM m4u_cache_flush_all(M4U_MODULE_ID_ENUM eModuleID);
    M4U_STATUS_ENUM m4u_cache_flush_by_range(M4U_PORT_ID port,
            						unsigned long va,
            						unsigned int size,
            						unsigned int mva);
    M4U_STATUS_ENUM m4u_dma_map_area(M4U_PORT_ID port,
            						M4U_DMA_DIR eDMADir,
            						unsigned long va,
            						unsigned int size,
            						unsigned int mva);
    M4U_STATUS_ENUM m4u_dma_unmap_area(M4U_PORT_ID port,
            						M4U_DMA_DIR eDMADir,
            						unsigned long va,
            						unsigned int size,
            						unsigned int mva);
    bool m4u_enable_m4u_func(M4U_MODULE_ID_ENUM eModuleID);
    bool m4u_disable_m4u_func(M4U_MODULE_ID_ENUM eModuleID);
};

#endif	/* __M4U_H_ */

