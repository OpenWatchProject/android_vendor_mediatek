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
#include <string>
#include <map>
#include <list>
#include <condition_variable>

#include <vpu_drv.h>
#include "vpu.h"
#include "vpu_platform.h"
#include <mt_iommu_port.h>

#define VPU_MVA_START         0x84000000 //0x7DA00000
#define VPU_MVA_END           0x8FF00000
#define VPU_MAX_STEP          0xffff

typedef struct vpu_buffer     vpu_buffer_t;
typedef struct vpu_algo       vpu_algo_t;
typedef struct vpu_request    vpu_request_t;
typedef struct vpu_prop_desc  vpu_prop_desc_t;
typedef struct vpu_port       vpu_port_t;
typedef enum   vpu_buf_format vpu_buf_format_t;
typedef struct vpu_power      vpu_power_t;
typedef struct vpu_status     vpu_status_t;

extern const size_t gVpuPropTypeSize[5];
extern const char *gVpuPropTypeName[5];

class VpuRequestImp;

class VpuAlgoImp : public VpuAlgo {
private:
    vpu_algo_t mNative;
    VpuPort mPorts[VPU_MAX_NUM_PORTS];
    bool    mbPortBuild;
    int     mSize;      // prop. size to be allocated
    VpuStream *mStream;

public:
    friend class VpuStreamImp;

    VpuAlgoImp(VpuStream *stream);
    virtual ~VpuAlgoImp();
    virtual bool     getProperty(const char *keyName, VpuProperty &prop);
    virtual VpuPort *getPort(const char *name);
    vpu_algo_t      *getNative() {return &mNative;}
    const char      *getName(void)       {return mNative.name;};
    void             setPropSize(int size){mSize = size;}
    VpuRequest      *acquire();
    void             release(VpuRequest *request);
    VpuStream       *getStream(void) { return mStream; };
};


class VpuRequestImp : public VpuRequest {
    typedef struct vpu_plane_buffer {
        uint32_t          share_fd;
        IApusysMem       *m;
        uint32_t          mva;
        uint32_t          offset;
    } vpu_plane_buffer_t;
    typedef struct vpu_request_buffer {
        uint8_t            plane_count;
        vpu_plane_buffer_t plane[3];
    } vpu_request_buffer_t;

private:
    apusysCmd      *mCmd = nullptr;
    IApusysMem     *mReq = nullptr;

    vpu_request_t   mNative;  // deprecated

    unsigned long   mId;
    VpuAlgoImp     *mAlgo;
    unsigned short  mBufferCount;

    IApusysMem *mSetting;

    vpu_request_buffer_t mRequestBuffer[VPU_MAX_NUM_PORTS] ;    // for ion
    VpuStream *mStream;

    // for EARA support
    VpuModuleType mModule; //acquire

    VpuEaraParam mEaraParam; //from user (NN)

    uint64_t mExecTime; //deque
    uint32_t mBandwidth;
    uint8_t mJobPriority; //deque

    uint64_t mSuggestTime; //enque

    uint64_t mFastestTime; //

public:
    friend class VpuStreamImp;
    friend class VpuAlgoImp;

    VpuRequestImp(VpuAlgoImp *algo);
    virtual ~VpuRequestImp();
    virtual bool    prepareSettBuf(int size);
    virtual void    releaseSettBuf(void);
    vpu_request_t   *getNative(){return &mNative;}
    //2.0 not support
    virtual void    addBuffer(VpuPort *port, VpuBuffer &buffer);
    virtual bool    getProperty(const char *keyName, VpuProperty &prop);
    virtual bool    setProperty(const char *keyName, const VpuProperty &prop);
    //
    virtual bool    addBuffer(VpuBuffer &buffer);
    virtual bool    getProperty(void* sett_ptr, int size);
    virtual bool    setProperty(void* sett_ptr, int size);
    virtual bool    setExtraParam(VpuExtraParam extra_param);

    VpuAlgoImp      *getAlgo(){return mAlgo;}
    void            reset() {mBufferCount = 0; mNative.buffer_count = 0; }
    int             mmapMVA();
    void            munmapMVA();
    bool            cacheSync();
    bool            cacheInvalid();

    // for EARA support
    void updateBoostValue(void);
    virtual void getInferenceInfo(struct VpuInferenceInfo &info);
    bool earaBegin(void);
    bool earaEnd(int errorStatus);
    void initEaraInfo(VpuModuleType module);
    void clearEaraInfo(void);
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

class VpuStreamImp : public VpuStream {
private:
    unsigned long mReqId;

    std::mutex mAlgosLock;
    std::map<std::string, VpuAlgo*> mAlgos;
    std::vector<VpuRequest *> mRequests;
    std::mutex mRequestMutex;
    std::condition_variable mRequestCond;

public:
    VpuStreamImp(const char* callername);
    virtual ~VpuStreamImp();
    virtual VpuAlgo     *getAlgo(const char *name);
    virtual VpuRequest  *acquire(VpuAlgo *algo, VpuModuleType module); // for EARA implementation
    virtual void        release(VpuRequest *request);
    virtual bool        runReq(VpuRequest *request, bool async);
    virtual bool        waitReq(VpuRequest *request);
    virtual bool        enque(VpuRequest *request, unsigned int core_index); // deprecated
    virtual bool        tryEnque(VpuRequest *request);  // deprecated
    virtual VpuRequest  *deque();  // deprecated
    virtual void        flush();   // flush requests => TBD
    virtual bool        setPower(VpuPower& pwr, unsigned int core_index);
    virtual bool        getVpuStatus(std::vector<VpuStatus>& vpu_status_info);  // deprecated

    virtual VpuMemBuffer    *allocMem(size_t size, uint32_t align, enum vpuMemType type);  // to apusysEngine::memAlloc
    virtual bool        freeMem(VpuMemBuffer *buf);  // to apusysEngine::memAlloc
    virtual bool        syncMem(VpuMemBuffer *buf);  // to apusysEngine::memSync

    /* 3.0 vpu dynamic algo support */
    VpuCreateAlgo createAlgo(const char *name, char *buf, uint32_t len, uint32_t core);
    bool freeAlgo(VpuCreateAlgo algoFd);
};


#endif
