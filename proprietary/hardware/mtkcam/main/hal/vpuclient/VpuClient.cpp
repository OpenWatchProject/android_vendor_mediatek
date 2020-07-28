/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkVpuClient_impl"
#include <mtkcam/utils/std/Log.h>
#include <system/camera_metadata.h>

#include <memory>
#include <vector>
#include <cutils/ashmem.h>

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>
#include "VpuClient.h"

#include <string> // std::string
#include <stdio.h> // ::fopen, ::fwrite, ::fclose
#include <vpu.h>                        // interface for vpu stream

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

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace vpuclient {
namespace V2_0 {
namespace implementation {

using vendor::mediatek::hardware::camera::vpuclient::V2_0::IMtkVpuBuffer;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_handle;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::hidl_memory;

//=======================================================================================
//=======================================================================================
/* TODO: remove later... */
enum {
  VPU_TEST_SUCCESS = 1 << 0,
  VPU_TEST_FAIL    = 1 << 1,
} ;

VpuStream  *g_vpu  = NULL;
std::vector<VpuStatus_t> mVecVpuStatus;

std::map<uint64_t, VpuClientRequest*> mvVpuRequestMap;

//=======================================================================================
//=======================================================================================

/******************************************************************************
 *
 ******************************************************************************/
VpuClientRequestImp::
VpuClientRequestImp()
{
	mBufferCount = 0;
}
/******************************************************************************
 *
 ******************************************************************************/
VpuClientRequestImp::
~VpuClientRequestImp()
{
}


//=======================================================================================
//=======================================================================================
Return<Status> MtkVpuClient::createInstance(const hidl_string& callername) {
Status ret=Status::OK;
    FUNCTION_LOG_START
 
    MY_LOGD("[%s](%s)",__FUNCTION__,callername.c_str());

    // 1. init vpu
    g_vpu = VpuStream::createInstance((const char*)callername.c_str());
    if (g_vpu == NULL)
    {
        MY_LOGE("fail to get vpu handle");
        ret=Status::ILLEGAL_ARGUMENT;
    }
	else {
		mvVpuRequestMap.clear();
	}
    
    FUNCTION_LOG_END
    return ret;
}

Return<uint64_t> MtkVpuClient::getAlgo(const hidl_string& stringname) {
uint64_t ret=0;
VpuAlgo *algo = NULL;

    FUNCTION_LOG_START
        
    MY_LOGD("[%s](%s)",__FUNCTION__,stringname.c_str());
    
    // 1.1 init algo of vpu
    MY_LOGD("algo_name(%s)\n",stringname.c_str());
    algo = g_vpu->getAlgo((char*)stringname.c_str());
    ret = (uint64_t)algo;
    if (algo == NULL)
    {
        MY_LOGW("fail to get vpu algo handle");
        ret=0;
    }
    MY_LOGD("(%s)->algo(%lx)",__FUNCTION__,ret);

    FUNCTION_LOG_END
    return ret;
}
    

Return<uint64_t> MtkVpuClient::acquire(uint64_t algo) {
VpuRequest *request = NULL;
    FUNCTION_LOG_START
    MY_LOGD("[%s],(%llx)",__FUNCTION__,(unsigned long long)algo);

    // 3. setup request
    // 3.1 acquire request
    request = g_vpu->acquire((VpuAlgo *)algo);
    MY_LOGD("(%s)->request(%llx)",__FUNCTION__, (unsigned long long)request);

	do {
	    vpuClientRequestMap_t::iterator iter;
	    if ((iter = mvVpuRequestMap.find((uint64_t)request)) == mvVpuRequestMap.end())    // not in Pool
	    {
	        VpuClientRequestImp *clientRequst = new VpuClientRequestImp();
	        mvVpuRequestMap.insert(vpuClientRequestPair_t((uint64_t)request, clientRequst));
	    }
	}while(0);

    FUNCTION_LOG_END
    return (uint64_t)request;
	
}

Return<Status> MtkVpuClient::addBuffer(uint64_t request, const VpuBuffer_t& vpuBuffer, const hidl_handle& hidlShareFd, const hidl_handle& hidlShareFd1, const hidl_handle& hidlShareFd2) {
Status ret=Status::OK;
VpuBuffer   buf;
    FUNCTION_LOG_START
    MY_LOGD("(%llx)",(unsigned long long)request);

#if 0
    MY_LOGD("id(%d)", vpuBuffer.port_id);
    MY_LOGD("format(0x%x)", vpuBuffer.format);
    MY_LOGD("width(%d)", vpuBuffer.width);
    MY_LOGD("height(%d)", vpuBuffer.height);
    MY_LOGD("planeCount(%d)", vpuBuffer.planeCount);
    for ( int i=0; i<3; i++ ) {    
        MY_LOGD("fd(%d)", vpuBuffer.planes[i].fd);
        MY_LOGD("offset(0x%x)", vpuBuffer.planes[i].offset);
        MY_LOGD("stride(0x%x)", vpuBuffer.planes[i].stride);
        MY_LOGD("length(0x%x)\n", vpuBuffer.planes[i].length);
    }
#endif

    // 3.2 add buffers to request
    //memcpy((void*)&buf, (void*)&vpuBuffer, sizeof(VpuBuffer));
    buf.port_id = vpuBuffer.port_id;
    buf.format = (::VpuBufferFormat)vpuBuffer.format;
    buf.width = vpuBuffer.width;
    buf.height = vpuBuffer.height;
    buf.planeCount = vpuBuffer.planeCount;
    for ( int i=0; i<3; i++ ) {
        buf.planes[i].fd = vpuBuffer.planes[i].fd;
        buf.planes[i].offset = vpuBuffer.planes[i].offset;
        buf.planes[i].stride = vpuBuffer.planes[i].stride;
        buf.planes[i].length = vpuBuffer.planes[i].length;
    }

    do {
        //MY_LOGD("[js_test]hidlShareFd->data[0](%x)",(int32_t)hidlShareFd->data[0]);
        int ion_share_fd = -1;    
        int old_ion_share_fd = -1;
		buf.planes[0].fd = 0;
		buf.planes[1].fd = 0;
		buf.planes[2].fd = 0;
        if (hidlShareFd != nullptr && (buf.planeCount >= 1) )
        {
            // !!NOTES: IMPORTANT: need to duplicate, otherwise, HwBinder will free this ion_share_fd
            old_ion_share_fd = hidlShareFd->data[0];
            MY_LOGD("old_ion_share_fd(%d)",old_ion_share_fd);
            ion_share_fd = dup(old_ion_share_fd);
            buf.planes[0].fd = ion_share_fd;
        }
        
        if (hidlShareFd1 != nullptr && (buf.planeCount >= 2) )
        {
            // !!NOTES: IMPORTANT: need to duplicate, otherwise, HwBinder will free this ion_share_fd
            old_ion_share_fd = hidlShareFd1->data[0];
            MY_LOGD("old_ion_share_fd1(%d)",old_ion_share_fd);
            ion_share_fd = dup(old_ion_share_fd);
            buf.planes[1].fd = ion_share_fd;
        }
                
        if (hidlShareFd2 != nullptr && (buf.planeCount >= 3) )
        {
            // !!NOTES: IMPORTANT: need to duplicate, otherwise, HwBinder will free this ion_share_fd
            old_ion_share_fd = hidlShareFd2->data[0];
            MY_LOGD("old_ion_share_fd2(%d)",old_ion_share_fd);
            ion_share_fd = dup(old_ion_share_fd);
            buf.planes[2].fd = ion_share_fd;
        }
    }while(0);
//
#if 0
    MY_LOGD("id(%d)", buf.port_id);
    MY_LOGD("format(0x%x)", buf.format);
    MY_LOGD("width(%d)", buf.width);
    MY_LOGD("height(%d)", buf.height);
    MY_LOGD("planeCount(%d)", buf.planeCount);
    for ( int i=0; i<3; i++ ) {    
        MY_LOGD("fd(%d)", buf.planes[i].fd);
        MY_LOGD("offset(0x%x)", buf.planes[i].offset);
        MY_LOGD("stride(0x%x)", buf.planes[i].stride);
        MY_LOGD("length(0x%x)", buf.planes[i].length);
    }
#endif

	/*
		get ion handle by fd.

		1. request no limitation
		2. 32 buffers max per request
	*/

	do {
	    vpuClientRequestMap_t::iterator iter;
	    int buf_index = 0;
	    vpu_request_buffer_t *req_buf = 0;
	    if ((iter = mvVpuRequestMap.find((uint64_t)request)) != mvVpuRequestMap.end())    // not in Pool
	    {
			buf_index = ((VpuClientRequestImp*)iter->second)->mBufferCount;
			req_buf = &((VpuClientRequestImp*)iter->second)->mRequestBuffer[buf_index];
//MY_LOGD("[js_test]buf_index(%d),buf.planeCount(%d)",buf_index,buf.planeCount);
			req_buf->plane_count = buf.planeCount;
			if(buf_index == 32)
			{
				MY_LOGE("vpu buffer number would be larger than (%d) after this\n", buf_index);
				ret=Status::ILLEGAL_ARGUMENT;
			}
			if(buf.planeCount > 3) {
				MY_LOGE("vpu wrong planeCount(%d)\n", buf.planeCount);
				ret=Status::ILLEGAL_ARGUMENT;
			} else {
				for (unsigned int i = 0; i < buf.planeCount; i++)
				{
					req_buf->plane[i].share_fd	 = buf.planes[i].fd;
//MY_LOGD("[js_test]share_fd(%d)",req_buf->plane[i].share_fd);
				}
			}

			/* buffer +1 */
			((VpuClientRequestImp*)iter->second)->mBufferCount++;
	    }
	}while(0);


    ((VpuRequest *)request)->addBuffer(buf); 

    FUNCTION_LOG_END
    return ret;
}
Return<Status> MtkVpuClient::setProperty(uint64_t request, int32_t sett, int32_t id) {
//Status ret=Status::OK;
Status ret=Status::OPERATION_NOT_SUPPORTED;
    FUNCTION_LOG_START
    MY_LOGD("[%s](%llx)(%x)(%x)",__FUNCTION__, (unsigned long long)request, sett, id);

#if 0
    // 3.3 set Property to request
    g_propertyBuf[0] = 256;
    g_propertyBuf[1] = 256;
    g_propertyBuf[2] = 3;
    g_propertyBuf[3] = 4;
    g_propertyBuf[4] = 1;
	((VpuRequest *)request)->setProperty((void*)g_propertyBuf, sizeof(g_propertyBuf));
#endif

    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkVpuClient::setPropertyMem(uint64_t request, const hidl_memory& propertyMem) {
Status ret=Status::OK;
    FUNCTION_LOG_START
    MY_LOGD("[%s](%llx)",__FUNCTION__, (unsigned long long)request);

    sp<IMemory> memoryReq = mapMemory(propertyMem);
    //MY_LOGD("[js_test](propertyMem)size=(%d), max prop size=(%d)Byte",(int)memoryReq->getSize(), 10*1024);
    char* propBuffer = new char[memoryReq->getSize()];

    void* pReg = static_cast<void*>(static_cast<void*>(memoryReq->getPointer()));
    memoryReq->update();
    const int* iReq = static_cast<const int*>(pReg);
    //MY_LOGD("[js_test](propertyMem)=0x(%x,%x,%x,%x,%x)", iReq[0],iReq[1],iReq[2],iReq[3],iReq[4]);
    memcpy((void*)propBuffer, (void*)iReq, (int)memoryReq->getSize());
    memoryReq->commit();

	((VpuRequest *)request)->setProperty((void*)propBuffer, memoryReq->getSize());

    delete[] propBuffer;

    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkVpuClient::setExtraParam(uint64_t request, const VpuExtraParam_t& extra_param) {
Status ret=Status::OK;
//Status ret=Status::OPERATION_NOT_SUPPORTED;
VpuExtraParam extraP;
    FUNCTION_LOG_START
    MY_LOGD("[%s](%llx)(%d)(%d)(%d)",__FUNCTION__, (unsigned long long)request, extra_param.opp_step, extra_param.freq_step, extra_param.bw);

	extraP.opp_step =   (uint8_t)extra_param.opp_step;
    extraP.freq_step =  (uint8_t)extra_param.freq_step;
    extraP.bw = (uint32_t)extra_param.bw;

	((VpuRequest *)request)->setExtraParam(extraP);

    FUNCTION_LOG_END
    return ret;
}

Return<void> MtkVpuClient::getVpuStatus(getVpuStatus_cb _hidl_cb) {
std::vector<VpuStatus> vpu_status;
VpuStatus_t VecVpuStatus;
bool res = false;
int i=0;
    FUNCTION_LOG_START
    MY_LOGD("[%s]",__FUNCTION__);

	res = g_vpu->getVpuStatus(vpu_status);

    //MY_LOGD("[%s]size(%d)",__FUNCTION__,vpu_status.size());
    mVecVpuStatus.clear();
	for (i = 0 ; i < (int)vpu_status.size() ; i++)
	{
		//MY_LOGD("(%d)->idx(%d), ava(%d), waitNum(%d)\n", i,
			//vpu_status[i].core_index, vpu_status[i].available,
			//vpu_status[i].waiting_requests_num);

            VecVpuStatus.core_index =(::vendor::mediatek::hardware::camera::vpuclient::V2_0::VpuCoreIndex)vpu_status[i].core_index;
            VecVpuStatus.available = vpu_status[i].available;
            VecVpuStatus.waiting_requests_num = vpu_status[i].waiting_requests_num;

            mVecVpuStatus.push_back(VecVpuStatus);
	}
    _hidl_cb(Status::OK, mVecVpuStatus);

    FUNCTION_LOG_END
    return Void();
}

Return<Status> MtkVpuClient::enque(uint64_t request, uint32_t core_index) {
Status ret=Status::OK;
    FUNCTION_LOG_START
        
    MY_LOGD("[%s](%llx)(%x)",__FUNCTION__, (unsigned long long)request, core_index);

	/*
		debug info by fd
	*/
    g_vpu->enque((VpuRequest *)request, core_index);

    FUNCTION_LOG_END
    return ret;
}


Return<uint64_t> MtkVpuClient::deque() {
VpuRequest *deque_req;
    FUNCTION_LOG_START
        
    MY_LOGD("[%s]",__FUNCTION__);
    
    deque_req = g_vpu->deque();

    FUNCTION_LOG_END
    return (uint64_t)deque_req;
}

Return<Status> MtkVpuClient::release(uint64_t request) {
Status ret=Status::OK;
    FUNCTION_LOG_START
        
    MY_LOGD("[%s](0x%llx)",__FUNCTION__, (unsigned long long)request);

    if (request){
		/*
			release ion handle
		*/
        g_vpu->release((VpuRequest*)request);

		/*
			release fd/request obj
		*/
	do {
	    vpuClientRequestMap_t::iterator iter;
	    unsigned int buf_index = 0;
	    vpu_request_buffer_t *req_buf = 0;
	    if ((iter = mvVpuRequestMap.find((uint64_t)request)) != mvVpuRequestMap.end())    // not in Pool
	    {
			buf_index = ((VpuClientRequestImp*)iter->second)->mBufferCount;
//MY_LOGD("[js_test]buf_index(%d)",buf_index);			
			for (unsigned int j = 0; j < buf_index; j++) {			
				req_buf = &((VpuClientRequestImp*)iter->second)->mRequestBuffer[j];
//MY_LOGD("[js_test]buf.planeCount(%d)",req_buf->plane_count);
				for (unsigned int i = 0; i < req_buf->plane_count; i++)
				{
//MY_LOGD("[js_test]buf.share_fd(%d)",req_buf->plane[i].share_fd);
					close(req_buf->plane[i].share_fd);
				}
			}
			/* delete request obj */
			delete iter->second;
	    }
	}while(0);
	


    }

    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkVpuClient::destroyInstance(const hidl_string& callername) {
Status ret=Status::OK;
    FUNCTION_LOG_START
    MY_LOGD("[%s])(%s)",__FUNCTION__,callername.c_str());

    // 1. deinit vpu
    if (g_vpu) {
		
		mvVpuRequestMap.clear();
		
        delete g_vpu;
        g_vpu = NULL;
    }
    
    FUNCTION_LOG_END
    return ret;
}

//=======================================================================================
//=======================================================================================








//below is for reference
//extern int VpuClientTemp_Test(int argc, char** argv);

Return<Status> MtkVpuClient::existVpuClient() {
    Status ret = Status::OK;
    FUNCTION_LOG_START
    //ret = canSupportBGService() ? Status::OK : Status::INTERNAL_ERROR;
    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkVpuClient::createVpuClient(int64_t timestamp) {
    Status ret=Status::OK;;
    FUNCTION_LOG_START
	MY_LOGD("%llx",(unsigned long long)timestamp);
    //ret = VpuClientWrap::getInstance()->createVpuClient(timestamp);
#if 0
    {
		char *argv[] = {
			()"",
			"0",	//test_algo	 = (VPU_ALGO_E)atoi(argv[1]);
			"1",	//autoTestCase = atoi(argv[2]);
			"0",	//test_pattern = atoi(argv[3]);
			"1",	//gDumpImage	 = atoi(argv[4]);
			};
		//int testResult = VpuClientTemp_Test(5, argv);
		//ret = (testResult==1) ? Status::OK : Status::METHOD_NOT_SUPPORTED;
	}
#endif
    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkVpuClient::createinstanceVpuClient(const hidl_vec<int64_t>& vpuStreamIns, const VpuBuffer_t& vpuBuffer, const hidl_handle& dstHidlShareFd) {
    Status ret=Status::OK;
    FUNCTION_LOG_START
        
    MY_LOGD("[js_test](%x)",(int32_t)vpuStreamIns[0]);
    MY_LOGD("[js_test](%x)",(int32_t)vpuStreamIns[1]);
    MY_LOGD("[js_test](%x)",(int32_t)vpuStreamIns[2]);
    MY_LOGD("[js_test]vpuBuffer.width(%x)",(int32_t)vpuBuffer.width);


    MY_LOGD("[js_test]dstHidlShareFd->data[0](%x)",(int32_t)dstHidlShareFd->data[0]);
    int ion_share_fd = -1;    
    int old_ion_share_fd = -1;
    if (dstHidlShareFd != nullptr)
    {
        // !!NOTES: IMPORTANT: need to duplicate, otherwise, HwBinder will free this ion_share_fd
        old_ion_share_fd = dstHidlShareFd->data[0];
        ion_share_fd = dup(old_ion_share_fd);
    }

    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkVpuClient::setOutputSurfaces(int64_t timestamp, const sp<vendor::mediatek::hardware::camera::vpuclient::V2_0::IMtkVpuBuffer>& bufferCB, const hidl_vec<int32_t>& msgType) {
    Status ret=Status::OK;
    FUNCTION_LOG_START
	bufferCB->setTimestamp(0,0);
	MY_LOGD("%llx,%llx",(unsigned long long)timestamp, (unsigned long long)msgType[0]);

    //ret = VpuClientWrap::getInstance()->setOutputSurfaces(timestamp, IMtkVpuBuffer::castFrom(bufferCB), msgType.size());
    FUNCTION_LOG_END
    return ret;
}

Return<Status> MtkVpuClient::destroyVpuClient(int64_t timestamp) {
    Status ret=Status::OK;
    FUNCTION_LOG_START
	MY_LOGD("%llx",(unsigned long long)timestamp);
    //ret = VpuClientWrap::getInstance()->destroyVpuClient(timestamp);
    FUNCTION_LOG_END
    return ret;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IMtkVpuClient* HIDL_FETCH_IMtkVpuClient(const char* /* name */) {
    return new MtkVpuClient();
}


}  // namespace implementation
}  // namespace V2_0
}  // namespace vpuclient
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
