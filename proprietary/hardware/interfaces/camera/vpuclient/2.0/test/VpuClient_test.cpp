/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "mmsdk/CallbackClient"
#include <cutils/log.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <semaphore.h>

#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion

#include <vpu.h>                        // interface for vpu stream

#include <cutils/properties.h>  // For property_get().
#include <ctime>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include <system/camera.h>
#define MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE 0x00000015 // REMOVE ME!!
//
#define CAM_LOGV ALOGV
#define CAM_LOGD ALOGD
#define CAM_LOGI ALOGI
#define CAM_LOGW ALOGW
#define CAM_LOGE ALOGE
#define CAM_LOGA ALOGA
#define CAM_LOGF ALOGF
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define FUNCTION_LOG_START          MY_LOGD_IF(1<=1, " +");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=1, " -");
/******************************************************************************
 *
 ******************************************************************************/

#include <vendor/mediatek/hardware/camera/vpuclient/2.0/IMtkVpuClient.h>
#include <vendor/mediatek/hardware/camera/vpuclient/2.0/types.h>
#include <android/hardware/camera/common/1.0/types.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>

using namespace ::android;
using namespace ::android::hardware::camera::common::V1_0;
using namespace vendor::mediatek::hardware::camera::vpuclient::V2_0;

using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_handle;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::hidl_memory;
using ::android::hardware::Return;

/******************************************************************************
 *
 ******************************************************************************/
typedef struct d2d_k3_property_structure {
	int           param1;
	int           param2;
	int           param3;
	int           param4;
	int           param5;
} d2d_k3_property_structure_t;

/******************************************************************************
*
******************************************************************************/
#include <utils/Vector.h>

#ifndef ION_HEAP_MULTIMEDIA_MASK
#define ION_HEAP_TYPE_MULTIMEDIA 10
#define ION_HEAP_MULTIMEDIA_MASK (1 << ION_HEAP_TYPE_MULTIMEDIA)
#endif

int32_t g_IonDriFD = -1;
int g_logEnabled = 0;//::property_get_int32(DEBUG_LOMO_LOG_ENABLED, 0);
int g_effectBufferSize;

struct IonInfo {
    int mIonDevFd;;
    int32_t mIonFD;
    int mSize;
    void *mVirtualAddr;
    ion_user_handle_t mIonAllocHandle;

    native_handle_t *pNativeHandle;
    hidl_handle hidlIonShareFd;

    IonInfo() {
        mIonDevFd = -1; // this one is ionDriFD, created somewhere else
        mIonFD = -1;
        mSize = 0;
        mVirtualAddr = NULL;
        mIonAllocHandle = -1;

        pNativeHandle = nullptr;
        hidlIonShareFd = nullptr;
        if (g_logEnabled != 0) {
            ALOGD("[IonInfo-%s] Info constructor done!", __func__);
        }
    }

    ~IonInfo() {
        if (pNativeHandle != nullptr) {
            native_handle_close(pNativeHandle);
            native_handle_delete(pNativeHandle);
            pNativeHandle = NULL;
        }
        hidlIonShareFd = nullptr;

        if (mVirtualAddr != NULL) {
            munmap(mVirtualAddr, mSize);
        }
        if (mIonFD != -1) {
            close(mIonFD);
            mIonFD = -1;
        }

        if (mIonAllocHandle != -1) {
            ion_free(mIonDevFd, mIonAllocHandle);
            mIonAllocHandle = -1;
        }
        mIonDevFd = -1; // delete somewhere else

        hidlIonShareFd = nullptr;
        if (g_logEnabled != 0) {
            ALOGD("[IonInfo-%s] Info destructor done!", __func__);
        }
    }
};

struct EffectBuffer {
    void *m_bffer;
    bool m_dirty;
    int m_effectId;
    int m_id;

    IonInfo *m_pIonInfo;

    EffectBuffer() {
        m_pIonInfo = NULL;
    }

    ~EffectBuffer() {
        // pNativeHandle & pHidlIonShareFd should be destryoed in IonInfo
        if (g_logEnabled != 0) {
            ALOGD("[%s] m_pIonInfo = %p", __func__, m_pIonInfo);
        }
        m_pIonInfo = NULL;
    }
};
Vector<EffectBuffer *> g_EffectBuffers;

Vector<IonInfo *> g_IonInfos;




IonInfo* allocateIon(int length, int32_t ionDriFD) {
    if (g_logEnabled != 0) {
        ALOGD("[%s] +", __func__);
    }
    int ion_handle = ionDriFD;
    ion_user_handle_t ion_allocHandle;
    int32_t ionFD;
    int alloc_ret = ion_alloc(ion_handle, length, 1, ION_HEAP_MULTIMEDIA_MASK, 0, &ion_allocHandle);
    if (alloc_ret) {
        ALOGE("[%s] Ion allocate failed, alloc_ret:%d, return", __func__, alloc_ret);
    }
    int share_ret = ion_share(ion_handle, ion_allocHandle, &ionFD);
    if (share_ret) {
        ALOGE("[%s] Ion share failed, share_ret:%d, return", __func__, share_ret);
    }
    void *virtualAddr = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, ionFD, 0);
    if (virtualAddr == MAP_FAILED) {
        ALOGE("[%s] mmap failed fd = %d, addr = 0x%p, len = %zu, prot = %d, flags = %d, share_fd = %d, 0x%p: %s\n",
            __func__, ion_handle, (void *)0, (size_t)length, PROT_READ | PROT_WRITE, MAP_SHARED,
            ionFD, (void *)0, strerror(errno));
    }
    IonInfo *info = new IonInfo();
    info->mIonDevFd= ion_handle;
    info->mIonAllocHandle = ion_allocHandle;
    info->mIonFD = ionFD;
    info->mVirtualAddr = virtualAddr;
    info->mSize = length;
    if (g_logEnabled != 0) {
        ALOGD("[%s] new IonInfo and init", __func__);
    }
    info->pNativeHandle = nullptr;
    if (g_logEnabled != 0) {
        ALOGD("[%s] info->hidlIonShareFd.mHandle=%p", __func__, info->hidlIonShareFd.getNativeHandle());
    }
    info->hidlIonShareFd = nullptr;

    if (g_logEnabled != 0) {
        ALOGD("[%s] -", __func__);
    }
    return info;
}

void destroyIon(IonInfo * info) {
    if (g_logEnabled != 0) {
        ALOGD("[%s] +", __func__);
    }
    delete info;
    if (g_logEnabled != 0) {
        ALOGD("[%s] -", __func__);
    }
}
/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char **argv) {

	printf("VPU HIDL(109)(%d/%lx)\n",argc,(unsigned long)argv);


/****************************************************************************************
*   initialize image in/out ion buffer
****************************************************************************************/
    g_IonDriFD = ion_open();
    if (g_IonDriFD < 0) {
        ALOGE("[%s] Open ion driver failed, return", __func__);
        return 0;
    }

    g_effectBufferSize = 1920*1080;
    for (int i = 0; i < 3; i++) {
        IonInfo *ionInfo = allocateIon(g_effectBufferSize, g_IonDriFD);
        g_IonInfos.push(ionInfo);

        EffectBuffer *buffer = new EffectBuffer();
        buffer->m_bffer = ionInfo->mVirtualAddr;
        buffer->m_dirty = true;
        buffer->m_effectId = -1;
        buffer->m_id = ionInfo->mIonFD;
        printf("m_id(%d)\n",buffer->m_id);
        buffer->m_pIonInfo = ionInfo;
        buffer->m_pIonInfo->pNativeHandle = nullptr;
        buffer->m_pIonInfo->hidlIonShareFd = nullptr;

        g_EffectBuffers.push(buffer);
    }

/****************************************************************************************
*   VPU StreamDrv Operations vis HIDL IFs
****************************************************************************************/
    VpuBuffer_t vpuBuffer;
    vpuBuffer.width = 0x888;

    VpuExtraParam_t extra_param;
    extra_param.opp_step = 0xff;
    extra_param.freq_step = 0xff;
    extra_param.bw = 0x0;

    VpuRequest_t request_hidl;
    request_hidl.width = 0xAAA;
    
    hidl_string userName("vpu_hidl_ilus");
    hidl_string algoName("vpu_flo_d2d_ksample");

    unsigned char* pVPUBuf[3];

    uint64_t algo;
    uint64_t request;
    uint64_t deqRequest;
    /*
        *   get VPU HIDL Client Service
        */
    sp<IMtkVpuClient> pVpuClient = IMtkVpuClient::tryGetService("internal/0");
    if (pVpuClient != NULL) {

    	pVpuClient->createInstance(userName);   /*  create VPU Stream Driver Instance   */
        printf("00:createInstance\n");
        
    	algo = pVpuClient->getAlgo(algoName);   /*  check VPU algorithm library exist     */
        printf("10:getAlgo(%llx)\n",(unsigned long long)algo);
        
    	request = pVpuClient->acquire(algo);    /*  get request of VPU algorithm          */
        printf("20:acquire(%llx))\n",(unsigned long long)request);

        for (int i = 0; i < 3; i++) {
            EffectBuffer *buffer =  g_EffectBuffers.itemAt(i);
            
            pVPUBuf[i] = (unsigned char*)buffer->m_bffer;
            if (buffer->m_pIonInfo->pNativeHandle== nullptr)
            {
                buffer->m_pIonInfo->pNativeHandle = native_handle_create(1, 0);
                buffer->m_pIonInfo->pNativeHandle->data[0] = buffer->m_id;
                buffer->m_pIonInfo->hidlIonShareFd = buffer->m_pIonInfo->pNativeHandle;

                vpuBuffer.format            = (::vendor::mediatek::hardware::camera::vpuclient::V2_0::VpuBufferFormat)eFormatImageNV21;
                vpuBuffer.width             = 1920;
                vpuBuffer.height            = 1080;
                vpuBuffer.planeCount        = 1;
                vpuBuffer.planes[0].offset  = 0;
                vpuBuffer.planes[0].stride  = 1920;
                vpuBuffer.planes[0].length  = g_effectBufferSize;
                vpuBuffer.port_id           = i;            
                vpuBuffer.planes[0].fd      = (unsigned int)0xFF;

                Return<Status> ret = 
                    pVpuClient->addBuffer(      /*  add VPU buffers                         */
                        request, vpuBuffer, buffer->m_pIonInfo->hidlIonShareFd, 0, 0);  
                
                if (!ret.isOk()) {
                    printf("[%s] isOk return false when addBuffer", __func__ );
                }
            }
            else {
                //TODO...
            }
        }
        printf("30:add buffer\n");

//-----------------------------------------------------------------------------
// REMOVE:initialized buffer data for test
pVPUBuf[0][0] = 0x07;
pVPUBuf[0][1] = 0x0A;
pVPUBuf[0][2] = 0x05;

pVPUBuf[1][0] = 0xFF;
pVPUBuf[1][1] = 0xFF;
pVPUBuf[1][2] = 0xFF;

pVPUBuf[2][0] = 0xCC;
pVPUBuf[2][1] = 0xCC;
pVPUBuf[2][2] = 0xCC;
printf("img1_va(%x/%x/%x)\n",pVPUBuf[0][0],pVPUBuf[0][1],pVPUBuf[0][2]);
printf("img2_va(%x/%x/%x)\n",pVPUBuf[1][0],pVPUBuf[1][1],pVPUBuf[1][2]);
printf("img3_va(%x/%x/%x)\n",pVPUBuf[2][0],pVPUBuf[2][1],pVPUBuf[2][2]);
//-----------------------------------------------------------------------------

        do {
            //
            d2d_k3_property_structure_t property;
            property.param1 = 256;
            property.param2 = 256;
            property.param3 = 3;
            property.param4 = 4;
            property.param5 = 1;
            //            
            sp<IAllocator> ashmem = IAllocator::getService("ashmem");
            if (ashmem == NULL) {
                printf("[MtkCodecService] failed to get IAllocator HW service");
                break;
            }
            ashmem->allocate(2048,
                            [&](bool success, const hidl_memory &memory_out) {
                if (success == true) {
                    printf("[MtkCodecService] Get HIDL output memory success %d.",(int)2048);
                    sp<IMemory> memory = mapMemory(memory_out);
                    int32_t * data = static_cast<int32_t *>(static_cast<void*>(memory->getPointer()));
                    memory->update();
                    memcpy((void*)data, (void*)&property, sizeof(d2d_k3_property_structure_t));
                    memory->commit();
                    pVpuClient->setPropertyMem(request, memory_out); /*  add VPU settings                         */
                    printf("45:setPropertyMem\n");
                }
            });
        } while (0);

    	pVpuClient->setExtraParam(request, extra_param);
        printf("50:setExtraParam\n");

        do {
            ::android::hardware::camera::common::V1_0::Status err = 
                ::android::hardware::camera::common::V1_0::Status::OK;
            std::vector<VpuStatus_t> vecVpuStatus;
            pVpuClient->getVpuStatus([&err, &vecVpuStatus]( /*  add VPU status                              */
            ::android::hardware::camera::common::V1_0::Status idStatus,
            const hidl_vec<VpuStatus_t>& vVpuStatus) {
                err = idStatus;
                printf("mkdbg: getVpuStatus CB: size: %zd", vVpuStatus.size());
                if (err == Status::OK) {
                    for (size_t i = 0; i < vVpuStatus.size(); i++) {
                        vecVpuStatus.push_back(vVpuStatus[i]);
                    }
                }});

            for (int i = 0 ; i < (int)vecVpuStatus.size() ; i++)
            {
                printf("(%d)->idx(%d), ava(%d), waitNum(%d)\n", i,
                    vecVpuStatus[i].core_index, vecVpuStatus[i].available,
                    vecVpuStatus[i].waiting_requests_num);
            }

        }while(0);
        printf("55:getVpuStatus\n");

    	pVpuClient->enque(request, 0x1);        /*  enque VPU job                               */
        printf("60:enque\n");
        
    	deqRequest = pVpuClient->deque();             /*  deque VPU job                               */
        if (request == deqRequest) {  printf("deque succeed\n");    }
        else                       {  printf("deque fail\n");       }
        printf("70:deque\n");
        
    	pVpuClient->release(request);           /*  release VPU request                        */
        printf("80:release\n");
        
    	pVpuClient->destroyInstance(userName);  /*  release VPU Stream Driver Instance   */
        printf("90:destroyInstance\n");

    } else {
		printf("ng1\n");
    }

    


/****************************************************************************************
*   relase ion buffers
****************************************************************************************/
//-----------------------------------------------------------------------------
// REMOVE:check out data
printf("img1_va(%x/%x/%x)\n",pVPUBuf[0][0],pVPUBuf[0][1],pVPUBuf[0][2]);
printf("img2_va(%x/%x/%x)\n",pVPUBuf[1][0],pVPUBuf[1][1],pVPUBuf[1][2]);
printf("img3_va(%x/%x/%x)\n",pVPUBuf[2][0],pVPUBuf[2][1],pVPUBuf[2][2]);
//-----------------------------------------------------------------------------

    while (!g_EffectBuffers.isEmpty()) {
        EffectBuffer *buffer = g_EffectBuffers.top();
        g_EffectBuffers.pop();
        delete buffer;
    }

    while (!g_IonInfos.isEmpty()) {
        IonInfo *info = g_IonInfos.top();
        destroyIon(info);
        info = NULL;
        g_IonInfos.pop();
    }

    ion_close(g_IonDriFD);
    ALOGD("[%s] Close ion driver, driver FD:%d", __func__, g_IonDriFD);
    g_IonDriFD = -1;

/****************************************************************************************
*
****************************************************************************************/

    return 0;
}




