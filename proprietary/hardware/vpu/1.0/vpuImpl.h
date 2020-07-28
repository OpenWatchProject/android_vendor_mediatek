/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_VPU_IMPL_H
#define _MTK_VPU_IMPL_H
#include <stdio.h>
#include <stdlib.h>

#include <linux/ioctl.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <mutex>
#include<string>
#include<map>
#include<list>

#include <vpu_drv.h>
#include <vpu.h>
#include <vpu_platform.h>

#include<mt_iommu_port.h>

//#define VPU_DUMP_ALGO_INFO

#define VPU_MVA_START         0x60000000
#define VPU_MVA_END           0x7FFFFFFF
#define VPU_MAX_STEP          0xffff

typedef struct vpu_buffer     vpu_buffer_t;
typedef struct vpu_algo       vpu_algo_t;
typedef struct vpu_request    vpu_request_t;
typedef struct vpu_prop_desc  vpu_prop_desc_t;
typedef struct vpu_port       vpu_port_t;
typedef enum   vpu_buf_format vpu_buf_format_t;
typedef struct vpu_power      vpu_power_t;

const size_t gVpuPropTypeSize[5] = {
        [eTypeChar]     = sizeof(uint8_t),
        [eTypeInt32]    = sizeof(int32_t),
        [eTypeFloat]    = sizeof(float),
        [eTypeInt64]    = sizeof(int64_t),
        [eTypeDouble]   = sizeof(double),
};

const char *gVpuPropTypeName[5] = {
        [eTypeChar]     = "byte",
        [eTypeInt32]    = "int32",
        [eTypeFloat]    = "float",
        [eTypeInt64]    = "int64",
        [eTypeDouble]   = "double",
};

class VpuRequestImp;
typedef std::list<VpuRequestImp*>         vpuRequestList_t;
typedef std::map <std::string, VpuAlgo*>  vpuAlgoMap_t;
typedef std::pair<std::string, VpuAlgo*>  vpuAlgoPair_t;
typedef std::map <VpuStream*, int>        vpuStepMap_t;
typedef std::pair<VpuStream*, int>        vpuStepPair_t;
class VpuAlgoImp : public VpuAlgo {

private:
    vpu_algo_t mNative;
    VpuPort mPorts[VPU_MAX_NUM_PORTS];
    bool    mbPortBuild;
    int     mSize;      // prop. size to be allocated

    std::mutex mRequestMutex;
    vpuRequestList_t mAvaiableRequest;
    vpuRequestList_t mInUseRequest;

public:
    VpuAlgoImp();
    virtual ~VpuAlgoImp();
    virtual bool     getProperty(const char *keyName, VpuProperty &prop);
    virtual VpuPort *getPort(const char *name);
    vpu_algo_t      *getNative() {return &mNative;}
    unsigned int     getId()     {return mNative.id;}

    void             setPropSize(int size){mSize = size;}
    VpuRequest      *acquire();
    void             release(VpuRequest *request);
};


class VpuRequestImp : public VpuRequest {
    typedef struct vpu_plane_buffer {
        uint32_t          share_fd;
        ion_user_handle_t ion_handle;
        uint32_t          mva;
        uint32_t          offset;
    } vpu_plane_buffer_t;
    typedef struct vpu_request_buffer {
        uint8_t            plane_count;
        vpu_plane_buffer_t plane[3];
    } vpu_request_buffer_t;

    typedef struct vpu_setting_buffer {
        uint32_t          share_fd;
        ion_user_handle_t ion_handle;
        uint32_t          mva;
        uint64_t          va;
    } vpu_setting_buffer_t;
private:
    vpu_request_t   mNative;
    VpuAlgoImp     *mAlgo;
    unsigned short  mBufferCount;
    vpu_setting_buffer_t mSetting;
    vpu_request_buffer_t mRequestBuffer[VPU_MAX_NUM_PORTS] ;    // for ion

public:
    VpuRequestImp(VpuAlgoImp *algo, int size);
    virtual ~VpuRequestImp();
    virtual void    addBuffer(VpuPort *port, VpuBuffer &buffer);
    vpu_request_t  *getNative(){return &mNative;}
    virtual bool    getProperty(const char *keyName, VpuProperty &prop);
    virtual bool    setProperty(const char *keyName, const VpuProperty &prop);
	/* not support in 1.0 */
	virtual void 	addBuffer(VpuBuffer &buffer);
	virtual bool	setProperty(void* sett_ptr, int size);
	virtual bool	setExtraParam(VpuExtraParam extra_param);
	/* *** */

    VpuAlgoImp     *getAlgo(){return mAlgo;}
    void            reset() {mBufferCount = 0; mNative.buffer_count = 0;}
    void            mmapMVA();
    void            munmapMVA();

};


class VpuUtil {
public:
    VpuUtil();
    ~VpuUtil();
    static VpuUtil *getInstance();
    uint32_t mapPhyAddr(int fd);

private:
    std::mutex mMapPhyMutex;
};

class VpuAlgoPool {
public:
    VpuAlgoPool();
    ~VpuAlgoPool();
    VpuAlgo    *getAlgo(std::string name, int fd);
    static VpuAlgoPool *getInstance();

private:
    std::map<std::string, VpuAlgo*> mvVpuAlgoMap;
};

class VpuStreamImp : public VpuStream {
private:
    int mFd;
    std::vector<VpuRequest *> mRequests;
    std::mutex mRequestMutex;
    std::condition_variable mRequestCond;
public:
    VpuStreamImp();
    virtual ~VpuStreamImp();
    virtual VpuAlgo    *getAlgo(char *name);
    virtual VpuRequest *acquire(VpuAlgo *algo);
    virtual void        release(VpuRequest *request);
    virtual bool        enque(VpuRequest *request, unsigned int core_index);
    virtual VpuRequest *deque();
    virtual void        flush();
    virtual bool        setPower(VpuPower& pwr, unsigned int core_index);
	/* not support in 1.0 */
	virtual bool        getVpuStatus(std::vector<VpuStatus>& vpu_status_info);
	/* *** */
};


#endif
