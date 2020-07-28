/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#ifndef __VPU_BUFFER_H__
#define __VPU_BUFFER_H__

/*
    vpuMemAllocator -----> vpuIonAllocator --\
          |           |--> vpuDmaAllocator ---\
          |           |--> vpuUserAllocator ---\
          |           |--> extend...            \
          |-------------------------------------------> vpuMemBlock
*/

#include <vector>
#include <vpu.h> //enum vpuMemType

#if 0
#define VPU_BUFFER_DEBUG(x, ...) printf("[debug] "x, __VA_ARGS__);
#define VPU_BUFFER_INFO(x, ...) printf("[Info] "x, __VA_ARGS__);
#define VPU_BUFFER_WARN(x, ...) printf("[Warn] "x, __VA_ARGS__);
#define VPU_BUFFER_ERR(x, ...) printf("[Error] "x, __VA_ARGS__);
#else
#include <utils/Trace.h>
#include <cutils/log.h>
#define VPU_BUFFER_DEBUG(x, arg...) ALOGD("[vpuBuffer] " x, ##arg);
#define VPU_BUFFER_INFO(x, arg...) ALOGI("[vpuBuffer] " x, ##arg);
#define VPU_BUFFER_WARN(x, arg...) ALOGW("[vpuBuffer] " x, ##arg);
#define VPU_BUFFER_ERR(x, arg...) ALOGE("[vpuBuffer] " x, ##arg);
#endif

#define LINE_TAG VPU_BUFFER_WARN("%s %d \n",__func__, __LINE__);

/* ion */
typedef struct {
	uint64_t va;
	uint32_t mva;
	uint32_t mvaLen;

	ion_user_handle_t handle;
	int sharedFd;
}vpuMemIonHandle;

/* dma */
typedef struct {
	uint64_t va;
	uint64_t iova;

	int fd;
}vpuMemDmaHandle;

/* malloc */
typedef struct {
	uint64_t va;
}vpuMemMallocHandle;

class ionAllocator {
private:
	int mDevFd;
public:
	ionAllocator();
	~ionAllocator();
	bool allocMem(uint32_t len, uint32_t align, vpuMemIonHandle &hnd);
	bool importMem(int fd, uint32_t len, vpuMemIonHandle &hnd);
	bool freeMem(vpuMemIonHandle &hnd);
	bool syncMem(vpuMemIonHandle &hnd);
};

class dmaAllocator {
private:
public:
	dmaAllocator();
	~dmaAllocator();
	bool allocMem(uint32_t len, uint32_t align, vpuMemDmaHandle &hnd);
	bool importMem(int fd, uint32_t len, vpuMemDmaHandle &hnd);
	bool freeMem(vpuMemDmaHandle &hnd);
	bool syncMem(vpuMemDmaHandle &hnd);
};

class userAllocator {
private:

public:
	userAllocator();
	~userAllocator();
	bool allocMem(uint32_t len, uint32_t align, vpuMemMallocHandle &hnd);
	bool importMem(int fd, uint32_t len, vpuMemMallocHandle &hnd);
	bool freeMem(vpuMemMallocHandle &hnd);
	bool syncMem(vpuMemMallocHandle &hnd);
};

class vpuMemAllocator {
private:
	std::vector<VpuMemBuffer *> mAllocList;
	ionAllocator mIonAllocator;
	dmaAllocator mDmaAllocator;
	userAllocator mUserAllocator;

	bool allocHandle(VpuMemBuffer* buf);

public:
	vpuMemAllocator();
	~vpuMemAllocator();

	VpuMemBuffer* allocMem(uint32_t len, uint32_t align, enum vpuMemType type);
	VpuMemBuffer* importMem(int fd, uint32_t len, enum vpuMemType type);
	bool freeMem(VpuMemBuffer* buf);
	bool syncMem(VpuMemBuffer* buf);

	uint64_t queryVa(uint64_t pa);
	uint64_t queryPa(uint64_t va);
};


#endif
